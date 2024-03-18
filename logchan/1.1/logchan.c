#include "module.h"
#include "logchan.h"

#define AUTHOR "Liber"
#define VERSION "1.0"
 
/* AnopeInit, run on load */
int AnopeInit(int argc, char **argv)
{

    LogChanInit(); //turn on log channel
    BindEvents(); // bind to our events
    
    /* give info to module */
    moduleAddAuthor(AUTHOR);       /* Add the author information  */
    moduleAddVersion(VERSION);     /* Add the version information */
    moduleSetType(THIRD);          /* Flag as Third party module  */
    alog("\002MODULE\002 [LogChan] Loaded Successfully");
    return MOD_CONT;
}

/* Run on unload */ 
void AnopeFini(void) 
{
	alog("\002MODULE\002 Logchan unloaded successfully"); 	
}
                   
int BindEvents(void)
{
	EvtHook *hook;
	Message *message; 
	int status = 0;

	/* /bs bot add */
	hook = createEventHook(EVENT_BOT_CREATE, log_botadd); 
	status = moduleAddEventHook(hook);
		if (status != MOD_ERR_OK)
		{
			alog("Error binding to event: EVENT_BOT_CREATE [%d]", status);
			return MOD_STOP;
		}
	/* /bs bot del */
	hook = createEventHook(EVENT_BOT_DEL, log_botdel); 
	status = moduleAddEventHook(hook); 
		if (status != MOD_ERR_OK) 
		{
			alog("Error binding to event: EVENT_BOT_DEL [%d]", status); 
			return MOD_STOP; 
		}
	/* UMODE (set by user) */
	message = createMessage("UMODE2", logchan_umode); 
	status = moduleAddMessage(message, MOD_HEAD);
		if (status != MOD_ERR_OK) 
		{
			alog("Error binding to message: UMODE2 [%d]", status);
			return MOD_STOP;
		}
	/* Server Notices (SNO) */
	message = createMessage("SENDSNO", log_sno); 
	status = moduleAddMessage(message, MOD_HEAD); 
		if (status != MOD_ERR_OK) 
		{
			alog("Error binding to message SENDSNO: %d", status); 
			return MOD_STOP; 
		}
	/* UMODE (Binding to "|" for Unreal tokens) */ 
	message = createMessage("|", logchan_umode);
	status = moduleAddMessage(message, MOD_HEAD);
		if (status != MOD_ERR_OK)
		{
			alog("Error binding to UMODE token [%d]", status);
			return MOD_STOP; 
		}
	/* AWAY */
	message = createMessage("AWAY", log_away);
	status = moduleAddMessage(message, MOD_HEAD);
		if (status != MOD_ERR_OK) 
		{
			alog("Error binding to message: AWAY [%d]", status);
			return MOD_STOP;
		}
	/* Nick Change */
	message = createMessage("NICK", log_nick_change); 
	status = moduleAddMessage(message, MOD_HEAD); 
		if (status != MOD_ERR_OK) 
		{
			alog("Error binding to NICK message: %d", status); 
			return MOD_STOP;
		}
	/* Log User Connections */
	hook = createEventHook(EVENT_NEWNICK, log_newnick); 
	status = moduleAddEventHook(hook); 
		if (status != MOD_ERR_OK) 
		{
			alog("Error binding to EVENT_NEWNICK: %d", status); 
			return MOD_CONT; 
		}
	/* Log User Disconnections */
	message = createMessage("QUIT", log_user_logoff); 
	status = moduleAddMessage(message, MOD_HEAD); 
		if (status != MOD_ERR_OK) 
		{
			alog("Error binding to message QUIT: %d", status); 
			return MOD_STOP; 
		}
	/* Log User Kills */ 
	message = createMessage("KILL", log_user_kill); 
	status = moduleAddMessage(message, MOD_HEAD); 
		if (status != MOD_ERR_OK) 
		{
			alog("Error binding to message KILL: %d", status); 
			return MOD_STOP; 
		}
	/* GlobOps */
	message = createMessage("GLOBOPS", log_globops); 
	status = moduleAddMessage(message, MOD_HEAD); 
	/* AWAY (Binding to "6" due to not all ircds supporting "AWAY") */
	message = createMessage("6", log_away); 
		if (status != MOD_ERR_OK) {
			alog("Error binding to message: AWAY (Token 6) [%d]", status); 
			return MOD_STOP; 
		}
	/* TKL updates (GLINE, GZLINE) */ 
	message = createMessage("TKL", log_tkl); 
	status = moduleAddMessage(message, MOD_HEAD); 
		if (status != MOD_ERR_OK) {
			alog("Error binding to message: TKL [%d]", status); 
		}
	/* BOT UNASSIGN */
	hook = createEventHook(EVENT_BOT_UNASSIGN, log_botUnassign);
	status = moduleAddEventHook(hook);
 		if (status != MOD_ERR_OK)
 		{
 			alog("Error binding to event: EVENT_BOT_UNASSIGN [%d]", status);
 			return MOD_STOP;
 		}

	/* TOPIC UPDATED */
	hook = createEventHook(EVENT_TOPIC_UPDATED, log_topic);
	status = moduleAddEventHook(hook);
		if (status != MOD_ERR_OK)
		{
			alog("Error binding to event: EVENT_TOPIC_UPDATED [%d]", status);
			return MOD_STOP;
		}

	/* BOT BAN */
	hook = createEventHook(EVENT_BOT_BAN, log_botBan);
	status = moduleAddEventHook(hook);
		if (status != MOD_ERR_OK)
		{
			alog("Error binding to event: EVENT_BOT_BAN [%d]", status);
			return MOD_STOP;
		}
	/* Bot Kick */
	hook = createEventHook(EVENT_BOT_KICK, log_botKick); 
	status = moduleAddEventHook(hook); 
		if (status != MOD_ERR_OK) 
		{
			alog("Error binding to event: EVENT_BOT_KICK [%d]", status); 
			return MOD_STOP; 
		}
	/* CHANNEL KICK */
	message = createMessage("KICK", log_kicks);
	status = moduleAddMessage(message, MOD_HEAD);
		if (status != MOD_ERR_OK)
		{
			alog("Error binding to message KICK: %d", status);
			return MOD_STOP;
		}
	/* Channel Modes */
	message = createMessage("MODE", log_chan_mode); 
	status = moduleAddMessage(message, MOD_HEAD); 
		if (status != MOD_ERR_OK) 
		{
			alog("Error binding to message MODE: %d", status); 
			return MOD_STOP; 
		}
	/* BOT ASSIGN */
	hook = createEventHook(EVENT_BOT_ASSIGN, log_botAssign);
	status = moduleAddEventHook(hook);
		if (status != MOD_ERR_OK)
		{
			alog("Error binding to event: EVENT_BOT_ASSIGN [%d]", status);
			return MOD_STOP;
		}

	/* JOIN */
	hook = createEventHook(EVENT_JOIN_CHANNEL, log_joins);
	status = moduleAddEventHook(hook);
		if (status != MOD_ERR_OK)
		{
			alog("Error binding to event: EVENT_JOIN_CHANNEL [%d]", status);
			return MOD_STOP;
		}

	/* PART */
	hook = createEventHook(EVENT_PART_CHANNEL, log_parts);
	status = moduleAddEventHook(hook);
		if (status != MOD_ERR_OK)
		{
			alog("Error binding to event: EVENT_PART_CHANNEL [%d]", status);
			return MOD_STOP;
		}
	/* SHUTDOWN */
	hook = createEventHook(EVENT_SHUTDOWN, log_shutdown);
	status = moduleAddEventHook(hook);
	    if (status != MOD_ERR_OK)
	    {
	        alog("Error binding to event: EVENT_SHUTDOWN [%d]", status);
	        return MOD_STOP;
	    }
	/* RESTART */ 	                                                    
	hook = createEventHook(EVENT_RESTART, log_restart);
	status = moduleAddEventHook(hook);
		if (status != MOD_ERR_OK)
		{
			alog("Error binding to event: EVENT_RESTART [%d]", status);
			return MOD_STOP;
		}
	/* EVENT_DEFCON_LEVEL */
	hook = createEventHook(EVENT_DEFCON_LEVEL, log_defcon); 
	status = moduleAddEventHook(hook); 
		if (status != MOD_ERR_OK) 
		{
			alog("Error binding to event: EVENT_DEFCON_LEVEL [%d]", status); 
			return MOD_STOP; 
		}
	return MOD_CONT; 
}

/* LogChanInit, enable the log channel and disable LogUsers */
void LogChanInit(void)
{
	if (!LogChannel) 
		LogChannel = "#services"; 
	if (!logchan)
		logchan = 1;
	LogUsers = 0; 
}

/* log_botUnassign, log /bs unassign with nick, channel, etc */
int log_botUnassign(int argc, char **argv)
{
	BotInfo *bi; // bot struct
	Channel *ci; // channel struct

	if (!(bi = findbot(argv[1])) | !(ci = findchan(argv[0])))
	{
		return MOD_CONT;
	}
	alog("\002%s\002: %s (%s@%s \"%s\") has been unassigned from %s", s_BotServ, bi->nick, bi->user, bi->host, bi->real, ci->name);
	return MOD_CONT;
}

/* log_defcon, log change of defcon level */
int log_defcon(int argc, char **argv) {
	alog("\002DEFCON\002 Defcon level has been changed to %s", argv[0]); 
	return MOD_CONT; 
}

/* log_botAssign, log for /bs assign */
int log_botAssign(int argc, char **argv)
{
	BotInfo *bi; //bot struct
	Channel *ci; //channel struct

		if (!(bi = findbot(argv[1])) | !(ci = findchan(argv[0])))
		{
			return MOD_CONT;
		}
	alog("\002%s\002: %s (%s@%s \"%s\") has been assigned to %s", s_BotServ, bi->nick, bi->user, bi->host, bi->real, ci->name);
	return MOD_CONT;
}

/* log_botadd, log for /bs bot add */
int log_botadd(int argc, char **argv)
{
	BotInfo *bi; //bot struct
		if (!(bi = findbot(argv[0]))) 
		{ 
			return MOD_CONT; 
		}
	
	alog("\002%s\002: %s!%s@%s (%s) created", s_BotServ, bi->nick, bi->user, bi->host, bi->real); 
	return MOD_CONT; 
}

/* log_botdel, log for /bs bot del */
int log_botdel(int argc, char **argv)
{
	BotInfo *bi; 
		if (!(bi = findbot(argv[0]))) 
		{
			return MOD_CONT; 
		}
	alog("\002%s\002: %s!%s@%s (%s) deleted", s_BotServ, bi->nick, bi->user, bi->host, bi->real); 
	return MOD_CONT;
}

/* log joins, from EVENT_JOIN_CHANNEL */
int log_joins(int argc, char **argv)
{
	User *u;

	u = finduser(argv[1]);


	if (!argv[1] || !argv[2])
	{
		return MOD_CONT;
	}

	if (stricmp(argv[0], EVENT_STOP) == 0)
	{
		alog("\002JOIN\002 %s (%s@%s) [%s] joins %s", argv[1], u->vident, u->chost, u->hostip, argv[2]);
	}

	return MOD_CONT;
}

/* log parts, from EVENT_PART_CHANNEL */
int log_parts(int argc, char **argv)
{
	int checkPart;
	User *u;
	u = finduser(argv[1]);

	checkPart = 0;

    if (!argv[1] || !argv[2])
    {
    	return MOD_CONT;
    }

    if (argc == 4)
    {
    	checkPart = 1;
    }

    if (stricmp(argv[0], EVENT_STOP) == 0)
    {
    	alog("\002PART\002 %s (%s@%s) [%s] parts %s (%s)", argv[1], u->vident, u->chost, u->hostip, argv[2], (checkPart == 1 ? argv[3] : ""));
    }

	return MOD_CONT;
}

/* log kicks, "KICK" message */ 
/* :Liber KICK #Liber test :test */ 
/* source: Liber, argv[0] = chan, argv[1] = victim, argv[2] reason */
int log_kicks(char *source, int argc, char **argv)
{
	User *u; /* kicker */ 
	User *u2; /* victim */ 
		
	if (!(u = finduser(source)) || !(u2 = finduser(argv[1]))) {
		return MOD_CONT;
	}

	alog("\002KICK\002 %s (%s@%s) was kicked from %s by %s!%s@%s [%s]", 
		u2->nick, u2->username, u2->chost, argv[0], u->nick, u->username, u->chost, argv[2]); 
	return MOD_CONT;
}


int log_botBan(int argc, char **argv)
{
	User *u;
	Channel *c;
	ChannelInfo *ci; 
	BotInfo *bi; 

		if (!(u = finduser(argv[0])) || !(c = findchan(argv[1])))
		{
			return MOD_CONT;
		}
	ci = c->ci; 
		
		if (!(bi = findbot(ci->bi->nick))) 
		{
			return MOD_CONT; 
		}

	alog("\002%s\002: %s (%s@%s) was banned from %s", bi->nick, u->nick, u->vident, u->chost, ci->name);
	return MOD_CONT;
}
// argv[0] = victim, argv[1] = channel, argv[2] = reason 
int log_botKick(int argc, char **argv) 
{
	User *u; 
	Channel *c; 
	ChannelInfo *ci; 
	BotInfo *bi; 

		if (!(u = finduser(argv[0])) || !(c = findchan(argv[1]))) 
		{
			return MOD_CONT; 
		}
	ci = c->ci; 
	bi = ci->bi; 

		if (!(bi = findbot(ci->bi->nick))) 
		{
			return MOD_CONT; 
		}

	alog("\002KICK\002 %s (%s@%s) was kicked from %s by %s!%s@%s [%s]", 
		u->nick, u->vident, u->chost, ci->name, bi->nick, bi->user, bi->host, argv[2]); 
	return MOD_CONT; 
}

		

int log_topic(int argc, char **argv)
{
	User *u;
	Channel *ci;

		if (!(ci = findchan(argv[0])) || !(argv[1]) || !(u = finduser(ci->topic_setter)))
		{
			return MOD_CONT;
		}

	alog("\002TOPIC\002 %s (%s@%s) changed the topic on %s", u->nick, u->vident, u->chost, ci->name);
	return MOD_CONT;
}

int log_shutdown(int argc, char **argv)
{
	alog("\002SHUTDOWN\002 Services Shutting Down"); 
	return MOD_CONT; 
}

int log_restart(int argc, char **argv)
{
	alog("\002RESTART\002 Services Restarting"); 
	return MOD_CONT; 
}

int logchan_umode(char *source, int ac, char **av)
{
	User *user;
    if (*av[0] == '#' || *av[0] == '&') 
    {
	    /* channel modes */ 
	    return MOD_CONT;
    } 
    else 
    { 
        user = finduser(source);
    	if (!user) 
    	{
            return MOD_CONT;
    	}
    	else 
    	{
           	my_set_umode(user, ac - 1, &av[0]);  
        }
    }        

    return MOD_CONT;
}
 
void my_set_umode(User * user, int ac, char **av)
{
    int myadd = 1;                /* 1 if adding modes, 0 if deleting */
    char *modes = av[0];
 
    ac--;
 
    if (debug)
    {
        alog("debug: Changing mode for %s to %s", user->nick, modes);
    }
    
    while (*modes) 
    {
        switch (*modes++) 
        {
        	case '+':
	            myadd = 1;
	            break; 
			case '-':
	            myadd = 0;
	            break; 
			case 'B': /* BOT */ 
				if (myadd == 1)
				{ 
					alog("\002USERMODE\002 %s (%s@%s => %s@%s) is now a Bot (+B)", user->nick, user->username, user->hostip, user->vident, user->vhost); 
				}
				else if (myadd == 0) 
				{
					alog("\002USERMODE\002 %s (%s@%s => %s@%s) is no longer a Bot (-B)", user->nick, user->username, user->hostip, user->vident, user->vhost); 
				} 
			break; 
		}
	}
}

int log_away(char *source, int argc, char **argv) 
{
	User *u; 
		if (!(u = finduser(source))) { 
			return MOD_CONT; 
		} 
		
		if (argc > 0) { alog("\002AWAY\002 %s (%s@%s) [%s] is now marked as away (%s)", u->nick, u->vident, u->chost, u->hostip, argv[0]); } 
		else { alog("\002AWAY\002 %s (%s@%s) [%s] is no longer marked as away", u->nick, u->vident, u->chost, u->hostip); }
	return MOD_CONT; 
}

int log_nick_change(char *source, int argc, char **argv) 
{
	User *u; 
	if (!strcmp(source, "") || !strcmp(argv[0], ""))
		return MOD_CONT; 
	
	if (!(u = finduser(source)))
		return MOD_CONT; 

	if (argc <= 3) 
	{
		alog("\002NICK\002 %s (%s@%s => %s@%s) is now known as %s", source, u->username, u->hostip, u->vident, u->chost, argv[0]);
	} 
	return MOD_CONT;
}

int log_newnick(int argc, char **argv) 
{
	User *u; 
	if (!strcmp(argv[0],"")) 
	{
		return MOD_CONT;
	}

	if (!(u = finduser(argv[0]))) 
	{
		return MOD_CONT; 
	}	
	alog("\002CONNECT\002 %s (%s@%s, %s) [%s] has connected to the network (%s)", 
			u->nick, u->username, u->host, u->realname, u->hostip, u->server->name); 
	return MOD_CONT; 
}


int log_user_logoff(char *source, int argc, char **argv) 
{
	User *u; 
	if (!(u = finduser(source))) 
	{
		return MOD_CONT; 
	}
		
	alog("\002QUIT\002 %s (%s@%s, %s) [%s] left the network (%s) [%s]", u->nick, u->username, u->host, u->realname, u->hostip, u->server->name, argv[0]); 
	return MOD_CONT; 
} 
	

/* [May 12 17:40:05.138148 2009] debug: Received: :Liber KILL test :ZetaIRC-449ECCFD.ziber.org!Liber (test) */
int log_user_kill(char *source, int argc, char **argv) 
{
	User *u;
	User *u2;

	/* Sanity */ 
	if (!(u = finduser(source)) || !(u2 = finduser(argv[0])))
	{
		return MOD_CONT; 
	}

	char *reason = myStrGetTokenRemainder(argv[1], ' ', 1);

	alog("\002KILL\002 %s (%s@%s) was killed by %s!%s@%s [%s]", u2->nick, u2->username, u2->host, u->nick, u->username, u->host, reason); 
	return MOD_CONT; 
}
/* :alpha.zetairc.net TKL + Z * 127.0.0.1 Liber!Liber@liber-ipv6.net 0 1250265581 :A test ban for services. */ 
/* -- argv[0] = + or - */ 
/* -- argv[1] = (Z|G|z|F) [gzline, gline, zline, spamfilter] */
/* -- argv[2] = [if its a gline/gzline, ident of ban (*), otherwise the target of a spamfilter */ 
/* -- argv[3] = (if spamfilter, [g|Z|z] (gline, gzline, zline) | (if ban, target ip/host) */ 
/* -- argv[4] = source of ban [nick!user@host] */ 
/* -- argv[5] = expiration time, or 0. (NULL if removal OF BAN) */ 
/* -- argv[6] = time set (NULL if removal OF BAN) */ 
/* -- argv[7] = [if spamfilter [if removal, the regex] [if addition, expiration time (ex: 30m)] ], [if ban, reason] (NULL IF REMOVAL OF BAN)*/ 
/* -- argv[8] = [if spamfilter addition, reason] (NULL IF REMOVAL OF ANYTHING) */ 
/* -- argv[9] = [if spamfilter addition, regex to match] (NULL IF REMOVAL OF ANYTHING) */   
int log_tkl(char *source, int argc, char **argv) {
	if (!strcmp(argv[0],"+")) {
		if (!strcmp(argv[1], "Z")) {
			alog("\002GZLINE\002 Added for %s@%s by %s [%s]", argv[2], argv[3], argv[4], argv[7]); 
		} 
		if (!strcmp(argv[1], "G")) {
			alog("\002GLINE\002 Added for %s@%s by %s [%s]", argv[2], argv[3], argv[4], argv[7]);
		} 
		if (!strcmp(argv[1], "z")) {
			alog("\002ZLINE\002 Added for %s@%s by %s [%s]", argv[2], argv[3], argv[4], argv[7]);
		}
	}
	if (!strcmp(argv[0],"-")) {
		if (!strcmp(argv[1], "Z")) {
			alog("\002GZLINE\002 Removed by %s for %s@%s", argv[4], argv[2], argv[3]); 
		}
		if (!strcmp(argv[1], "G")) {
			alog("\002GLINE\002 Removed by %s for %s@%s", argv[4], argv[2], argv[3]);
		}
		if (!strcmp(argv[1], "z")) {
			alog("\002GLINE\002 Removed by %s for %s@%s", argv[4], argv[2], argv[3]);
		}
	}
	
	return MOD_CONT; 
}

		
/* :crypt.hub.zetairc.net SENDSNO o :Ziber (Liber@ZetaIRC-449ECCFD.ziber.org) [liber] is now a network administrator (N) */
/* source = server, argv[0] = "o", argv[1] = message */ 
int log_sno(char *source, int argc, char **argv) 
{
	alog("\002NOTICE\002 %s [%s]", argv[1], source);  
	return MOD_CONT; 
}

/* :Liber MODE #services -N */
int log_chan_mode(char *source, int argc, char **argv)
{
	User *u;
	if (!(u = finduser(source))) 
	{
		return MOD_CONT; 
	}
	
	if (argc == 2) 
	{
		alog("\002MODE\002 %s %s by %s (%s@%s)", argv[0], argv[1], u->nick, u->username, u->chost); 
	} 
	if (argc == 3)
	{
		alog("\002MODE\002 %s %s %s by %s (%s@%s)", argv[0], argv[1], argv[2], u->nick, u->username, u->chost);
	}
	return MOD_CONT; 
} 

/* :alpha.zetairc.net GLOBOPS :Received SQUIT test.zetairc.net from Liber[alpha.ziber.org] (test.) */
int log_globops(char *source, int argc, char **argv) 
{

	if (!argv[0]) /* something is odd... bail */ 
		return MOD_CONT; 

	alog("\002GLOBOPS\002 %s", argv[0]); 
	return MOD_CONT; 
}

