#include "module.h"
#include "services.h"

#define AUTHOR "MaienM"
#define VERSION "1.02"

/* Chanserv Multiple Founder Support. */

// Forward declarations.
int OnIdent(int argc, char **argv);
int OnJoin(int argc, char **argv);
void DoModes(char *nick, char *chan, int force);
int PrivMsg(char *source, int argc, char **argv);
int FounderCommand(User *u);
int FounderHelp(User *u);
void FounderHelpShort(User *u);

int AnopeInit(int argc, char **argv)
{
	Command *c;
	EvtHook *h;
	Message *m;
	int status = 0;

	c = createCommand("FOUNDER", FounderCommand, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, FounderHelp);
	moduleSetChanHelp(FounderHelpShort);
	status = moduleAddCommand(CHANSERV, c, MOD_TAIL);

	if (status != MOD_ERR_OK) return MOD_STOP;

	m = createMessage("PRIVMSG", PrivMsg);
    	status = moduleAddMessage(m, MOD_HEAD);

	if (status != MOD_ERR_OK) return MOD_STOP;

	h = createEventHook(EVENT_JOIN_CHANNEL, OnJoin);
    	status = moduleAddEventHook(h);

	if (status != MOD_ERR_OK) return MOD_STOP;

	h = createEventHook(EVENT_NICK_IDENTIFY, OnIdent);
    	status = moduleAddEventHook(h);

	if (status != MOD_ERR_OK) return MOD_STOP;

	alog("cs_mutlifounder: Successfully loaded module.");
	alog("cs_multifounder: \2/msg %s HELP Founder\2", s_ChanServ);
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void) {}

char *replace(const char *src, const char *from, const char *to)
{
    size_t size    = strlen(src) + 1;
    char *value = malloc(size);
    char *dst = value;
    char *match;

    if (value != NULL)
    {
        for (match = strstr(src, from); match; match = strstr(src, from))
        {
            size_t count = match - src;
            char *temp;
            size += strlen(to) - strlen(from);
            temp = realloc(value, size);
            if (temp == NULL)
            {
               free(value);
               return NULL;
            }
            dst = temp + (dst - value);
            value = temp;
            memmove(dst, src, count);
            src += count;
            dst += count;
            memmove(dst, to, strlen(to));
            src += strlen(from);
            dst += strlen(to);
        }
        strcpy(dst, src);
    }
    return value;
}

char *ConvString(const int org, User *u)
{
	char *string = getstring(u->na, org);
	string = replace(string, "VOP", "FOUNDER");
	
	// Remove part from one spot before "|" to "".
	
	return string;
}

int FounderHelp(User *u) 
{
	notice_user(s_ChanServ, u, "Syntax: \2FOUNDER \037channel\037 ADD \037nick\037\2");
	notice_user(s_ChanServ, u, "        \2FOUNDER \037channel\037 DEL \037nick\037\2");
	notice_user(s_ChanServ, u, "        \2FOUNDER \037channel\037 LIST\2");
	return MOD_CONT;
}

void FounderHelpShort(User *u) 
{
	notice_user(s_ChanServ, u, "    FOUNDER    Add/Remove additional channel founders.");
}

int FounderCommand(User *u)
{
	char *chan = strtok(NULL, " ");
    	char *cmd = strtok(NULL, " ");
    	char *nick = strtok(NULL, " ");	
	int i;

	ChannelInfo *ci;
    	NickAlias *na;
	ChanAccess *access;

	ci = cs_findchan(chan);
	na = findnick(nick);

	if (!ci) {
        	notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    	} else if (ci->flags & CI_VERBOTEN) {
        	notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
	} else if (!na && stricmp(cmd, "LIST"))	{
		notice_user(s_ChanServ, u, ConvString(CHAN_VOP_NICKS_ONLY, u), nick);
	} else if ((stricmp(cmd, "LIST")) && (na->status & NS_VERBOTEN)) {
		notice_lang(s_ChanServ, u, NICK_X_FORBIDDEN, nick);
	} else if ((get_access_level(ci, u->na) < ACCESS_FOUNDER) && // access level check should never be needed, any founder SHOULD have  
		   (ci->founder != u->na->nc)) {	 	       //the ci->founder flag set if they reach this point.
       	 	notice_lang(s_ChanServ, u, PERMISSION_DENIED);
	}

	else if (stricmp(cmd, "ADD") == 0) 
	{
		access = get_access_entry(na->nc, ci);
		access->level = ACCESS_FOUNDER;
		notice_user(s_ChanServ, u, ConvString(CHAN_VOP_ADDED, u), na->nc->display, chan);
	} 

	else if (stricmp(cmd, "DEL") == 0)
	{
		access = get_access_entry(na->nc, ci);
		
		if (!access) {
			notice_user(s_ChanServ, u, ConvString(CHAN_VOP_NOT_FOUND, u), nick, chan);
		} else {
			access->level = 0; // THIS IS NOT THE WAY ITS SUPPOSED TO BE DONE (but it works, kinda....)!!!!!
			notice_user(s_ChanServ, u, ConvString(CHAN_VOP_DELETED, u), na->nc->display, chan);
		}
	}

	else if (stricmp(cmd, "LIST") == 0)
	{
		notice_user(s_ChanServ, u, ConvString(CHAN_VOP_LIST_HEADER, u), chan);
		for (access = ci->access, i=0; i<ci->accesscount; access++, i++) 
		{
            		if (access->level == ACCESS_FOUNDER) 
			{
				notice_user(s_ChanServ, u, "  %s", access->nc->display);
			}
		}
	}

	return MOD_CONT;
}

int OnJoin(int argc, char **argv)
{
	if (!stricmp(argv[0], EVENT_START)) DoModes(argv[1], argv[2], 0);
	return MOD_CONT;
}

int OnIdent(int argc, char **argv)
{
	User *u = finduser(argv[0]);
	if (!u) return MOD_CONT;

	struct u_chanlist *uc;
		
	for (uc = u->chans; uc; uc = uc->next) DoModes(u->nick, uc->chan->name, 1);

	return MOD_CONT;
}

void DoModes(char *nick, char *chan, int forcemodes)
{
	User *u;
    	Channel *c = NULL;

	c = findchan(chan);
	if (!c || !c->ci) return; // check if channel is registerd.
	if (!(u = finduser(nick))) return; // check if username is registered.
        if (!nick_identified(u)) return; // check if user is identified.

	if (get_access_level(c->ci, u->na) >= ACCESS_FOUNDER && c->ci->founder != u->na->nc)
	{
		c->ci->founder = u->na->nc;
		if (forcemodes) chan_set_correct_modes(u, c, 1);
	}
}

int PrivMsg(char *source, int argc, char **argv)
{
	User *u;
	char chan[31] = "\0";
    	int i=0;
	char *string = argv[1];

	if ((stricmp(argv[0],"chanserv") != 0) && 
	    (stricmp(argv[0],"chanserv@services.mmfail.irc.su") != 0)) return MOD_CONT; // check is PRIVMSG is sent to chanserv.

	if (!(u = finduser(source))) return MOD_CONT; // check if user is registered.

    	while (string != NULL && string[0] != ' ') // remove string till first space character.
		string++;
   	string++; // remove space character.

    	while (string[i] != ' ' && i < strlen(string)) // find position of next space character (and thus the length of the channelname).
        	i++;
    	strncat(chan, string, i); // copy channelname from string to chan.

	DoModes(source, chan, 0); // pass onto the mode function.

	return MOD_CONT;
}
