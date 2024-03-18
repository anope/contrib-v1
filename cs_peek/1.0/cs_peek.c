/*
	cs_peek.c
	
	Allows channel operators to display information about a channel.
	irc operators are able to override and peek any
	registered channel, along with users with autoop in "OVERRIDE_CHAN". 
	
	peek overrides are logged.
	
	command/concept stolen from srvx, all credit goes to its developers.
	
	if it's broken or there's something wrong, fix it yourself.
	
	katlyn (katlyn@swiftirc.net)

*/

#include "module.h"

#define AUTHOR "Katlyn"
#define VERSION "1.0"
#define OVERRIDE_CHAN "#irchelp"

#define SYNTAX_CS_PEEK 0
#define MORE_CS_PEEK 1
#define DESC_CS_PEEK 2
#define STATUS_CS_PEEK 3
#define TOPIC_CS_PEEK 4
#define MODES_CS_PEEK 5
#define USERS_CS_PEEK 6
#define OPS_CS_PEEK 7
#define CHAN_SUSPENDED 8
#define CHAN_FORBIDDEN 9
#define CHAN_EMPTY 10
#define PEEK_NOACCESS 11

void help_get_langs();
int check_useraccess(User *u, ChannelInfo *target);
int chanserv_peek(User *u);
int cs_help_peek(User *u);

int AnopeInit(int argc, char **argv) 
{
	Command *c;

	c = createCommand("PEEK", chanserv_peek, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(CHANSERV, c, MOD_HEAD);
	moduleAddHelp(c, cs_help_peek);

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	help_get_langs();

	alog("[cs_peek] Successfully loaded module.");
	
	return MOD_CONT;
}

void AnopeFini(void) 
{
	alog("[cs_peek] Unloaded module.");
}

int cs_help_peek(User *u) 
{
	moduleNoticeLang(s_ChanServ, u, SYNTAX_CS_PEEK);
	notice_user(s_ChanServ, u, " ");
	moduleNoticeLang(s_ChanServ, u, DESC_CS_PEEK);
	
	return 1;
}
void help_get_langs() 
{
	char *text[] = 
	{
		"Syntax: \2PEEK \037channel\037\2",
		"\2/msg %s HELP PEEK\2 for more information.",
		"Displays the current topic, modes, and ops of the specified channel.",
		"\002%s\002 Status:",
		"\002Topic:\002          %s",
		"\002Modes:\002          +%s",
		"\002Total users:\002    %d",
		"\002Ops\02:",	
		"\002%s\002 is currently suspended.",
		"\002%s\002 is currently forbidden.",
		"\002%s\002 is empty.",
		"You lack sufficient access in %s to use this command.",
	};
		
	moduleInsertLanguage(LANG_EN_US, 12, text);

}

int check_useraccess(User *u, ChannelInfo *target)
{
	ChannelInfo *ci;
	ci = cs_findchan(OVERRIDE_CHAN);
	
	if(!nick_identified(u))
	{
		return 0;
	}
	
	if(check_access(u, target, CA_AUTOOP))
	{
		return 1;
	}
	
	if(check_access(u, ci, CA_AUTOOP))
	{
		return 2;
	}
	
	if(is_services_oper(u))
	{
		return 2;
	}

	return 0;
}


int chanserv_peek(User *u)
{
	char *buffer = moduleGetLastBuffer();
	char *channelname;
	struct c_userlist *cu;
	Channel *c;
	channelname = myStrGetToken(buffer, ' ', 0);
	ChannelInfo *target;
	
	if(!channelname)
	{
		moduleNoticeLang(s_ChanServ, u, SYNTAX_CS_PEEK);
		moduleNoticeLang(s_ChanServ, u, MORE_CS_PEEK);
		free(channelname);
		return MOD_STOP;
	}
	
	if(!(target = cs_findchan(channelname)))
	{
		notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, channelname);
		free(channelname);
		return MOD_STOP;
	}
	
	if((check_useraccess(u, target) == 1) || (check_useraccess(u, target) == 2))
	{
		if (check_useraccess(u, target) == 2)
		{
			anope_cmd_global(s_OperServ, "\002[staff override]\002 CS_PEEK by %s on channel %s", u->nick, channelname);
			alog("[cs_peek staff override] %s on %s", u->nick, channelname);
		}
		
		if (target->flags & CI_SUSPENDED)
		{
			moduleNoticeLang(s_ChanServ, u, CHAN_SUSPENDED, channelname);
			free(channelname);
			return MOD_STOP;
		}
		
		if(target->flags & CI_VERBOTEN)
		{
			moduleNoticeLang(s_ChanServ, u, CHAN_FORBIDDEN, channelname);
			free(channelname);
			return MOD_STOP;
		}
		
		if(c = findchan(channelname))
		{
			moduleNoticeLang(s_ChanServ, u, STATUS_CS_PEEK, channelname);
			moduleNoticeLang(s_ChanServ, u, TOPIC_CS_PEEK, target->last_topic);
			moduleNoticeLang(s_ChanServ, u, MODES_CS_PEEK, chan_get_modes(c, 1, 1));
			moduleNoticeLang(s_ChanServ, u, USERS_CS_PEEK, c->usercount);
			moduleNoticeLang(s_ChanServ, u, OPS_CS_PEEK);
		
			for (cu = c->users; cu; cu = cu->next)
			{
				if (chan_has_user_status(c, cu->user, CUS_OP))
				{
					notice(s_ChanServ, u->nick, "%s", cu->user->nick);
				}
			}
		}
		else
		{
			moduleNoticeLang(s_ChanServ, u, CHAN_EMPTY, channelname);
		}
	}
	else
	{
		moduleNoticeLang(s_ChanServ, u, PEEK_NOACCESS, channelname);
		free(channelname);
		return MOD_STOP;
	}
	
	free(channelname);
	
	return MOD_CONT;
}