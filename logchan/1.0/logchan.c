#include "module.h"

#define AUTHOR "Liber"
#define VERSION "1.0"

/* 
 * Version Log: *
 * 0.1: Logged only joins/parts *
 * 0.2: Added kick, ban, /bs assign and /bs unassign *
 * 0.3: Added services restart, services shutdown and /away *
 * 1.0: Added usermode logging *
*/

/* 
 * Thanks to Trysten, for helping me with the usermodes logging *
 * Thanks to iZach, for testing my module on his network *
 * Thanks to the original author of log_jpk, who first inspired me to write this *
 * Thanks to Rob, for helping me debug my code *
*/ 

/* Our functions */ 
/* JOINS PARTS KICKS RELOAD RESTART SHUTDOWN AWAY RELOAD*/
int log_joins(int argc, char **argv); // EVENT_JOIN
int log_parts(int argc, char **argv); // EVENT_PART
int log_kicks(int argc, char **argv); // EVENT_KICK
int log_topic(int argc, char **argv); // EVENT_TOPIC_UPDATED
int log_restart(int argc, char **argv); // EVENT_RESTART
int log_shutdown(int argc, char **argv); // EVENT_SHUTDOWN
int log_away(char *source, int argc, char **argv); // EVENT_AWAY
int log_defcon(int argc, char **argv); // EVENT_DEFCON_LEVEL

/* BOTSERV */
int log_botadd(int argc, char **argv); // EVENT_BOT_CREATE
int log_botdel(int argc, char **argv); // EVENT_BOT_DEL
int log_botAssign(int argc, char **argv); // EVENT_BOT_ASSIGN
int log_botUnassign(int argc, char **argv); // EVENT_BOT_UNASSIGN
int log_botBan(int argc, char **argv); // EVENT_BOT_BAN

/* UMODE Logging */
int logchan_umode(char *source, int ac, char **av);
void my_set_umode(User * user, int ac, char **av);

/* Miscellaneous */
void LogChanInit(void); // Enable LogChannel
int BindEvents(void); // Bind to all events


/* AnopeInit, run on load */
int AnopeInit(int argc, char **argv)
{

    LogChanInit(); //turn on log channel
    BindEvents(); // bind to our events
    
    /* give info to module */
    moduleAddAuthor(AUTHOR);       /* Add the author information  */
    moduleAddVersion(VERSION);     /* Add the version information */
    moduleSetType(THIRD);          /* Flag as Third party module  */
    alog("\002Module\002: [LogChan] Loaded Successfully");
    return MOD_CONT;
}

/* Run on unload */ 
void AnopeFini(void) 
{

}

/*
***********************************************************************************************
***********************************************************************************************
***********************************************************************************************
*/
                       
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
	/* AWAY */
	message = createMessage("AWAY", log_away);
	status = moduleAddMessage(message, MOD_HEAD);
		if (status != MOD_ERR_OK) 
		{
			alog("Error binding to message: AWAY [%d]", status);
			return MOD_STOP;
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

	/* CHANNEL KICK */
	hook = createEventHook(EVENT_CHAN_KICK, log_kicks);
	status = moduleAddEventHook(hook);
		if (status != MOD_ERR_OK)
		{
			alog("Error binding to event: EVENT_CHAN_KICK [%d]", status);
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

/*
***********************************************************************************************
***********************************************************************************************
***********************************************************************************************
*/

/* LogChanInit, enable the log channel  */
void LogChanInit(void)
{
	logchan = 1;
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
	alog("\002DefCon\002: Defcon level has been changed to %s", argv[0]); 
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
		alog("\002Join\002: %s (%s@%s => %s@%s) joins %s", argv[1], u->username, u->hostip, u->vident, u->vhost, argv[2]);
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
    	alog("\002Part\002: %s (%s@%s => %s@%s) parts %s (%s)", argv[1], u->username, u->hostip, u->vident, u->vhost, argv[2], (checkPart == 1 ? argv[3] : ""));
    }

	return MOD_CONT;
}

/* log kicks, EVENT_CHAN_KICK */
int log_kicks(int argc, char **argv)
{
	User *u; //user struct
	Channel *ci; //channel struct
		if (!(u = finduser(argv[0])) | !(ci = findchan(argv[1])))
		{
			return MOD_CONT;
		}

	alog("\002Kick\002: %s (%s@%s => %s@%s) was kicked from %s", u->nick, u->username, u->hostip, u->vident, u->vhost, ci->name);
	return MOD_CONT;
}


int log_botBan(int argc, char **argv)
{
	User *u;
	Channel *ci;

		if (!(u = finduser(argv[0])) || !(ci = findchan(argv[1])))
		{
			return MOD_CONT;
		}

	alog("\002%s\002: %s (%s@%s => %s@%s) was banned from %s", s_BotServ, u->nick, u->username, u->hostip, u->vident, u->vhost, ci->name);
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

	alog("\002Topic\002: %s (%s@%s => %s@%s) changed the topic on %s", u->nick, u->username, u->hostip, u->vident, u->vhost, ci->name);
	return MOD_CONT;
}

int log_shutdown(int argc, char **argv)
{
	alog("\002Shutdown\002: Services Shutting Down"); 
	return MOD_CONT; 
}

int log_restart(int argc, char **argv)
{
	alog("\002Restart\002: Services Restarting"); 
	return MOD_CONT; 
}

/*********************************************************************************************/
/*********************************************************************************************/
/*********************************************************************************************/
/*** UMODE2 ***/
/*********************************************************************************************/
/*********************************************************************************************/
/*********************************************************************************************/


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
    
    // alog("\002UserMode\002: %s (%s@%s => %s@%s) set to %s", user->nick, user->username, user->hostip, user->vident, user->vhost, modes);
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
					alog("\002UserMode\002: %s (%s@%s => %s@%s) is now a Bot (+B)", user->nick, user->username, user->hostip, user->vident, user->vhost); 
				}
				else if (myadd == 0) 
				{
					alog("\002UserMode\002: %s (%s@%s => %s@%s) is no longer a Bot (-B)", user->nick, user->username, user->hostip, user->vident, user->vhost); 
				} 
			break; 
			case 'O': /* Local IRCop */
				if (myadd == 1) 
				{
					alog("\002UserMode\002: %s (%s@%s => %s@%s) is now a Local IRC Operator (+O)", user->nick, user->username, user->hostip, user->vident, user->vhost);	
				}
				else if (myadd == 0) 
				{
					alog("\002UserMode\002: %s (%s@%s => %s@%s) is no longer a Local IRC Operator (-O)", user->nick, user->username, user->hostip, user->vident, user->vhost);
				} 
			break; 
			case 'o': /* Global IRCop */
				if (myadd == 1) 
				{
					alog("\002UserMode\002: %s (%s@%s => %s@%s) is now a Global IRC Operator (+o)", user->nick, user->username, user->hostip, user->vident, user->vhost);
				}
				else if (myadd == 0) 
				{
					alog("\002UserMode\002: %s (%s@%s => %s@%s) is no longer a Global IRC Operator (-o)", user->nick, user->username, user->hostip, user->vident, user->vhost);
				}
			break; 
			case 'a': /* Services Admin */
				if (myadd == 1) 
				{
					alog("\002UserMode\002: %s (%s@%s => %s@%s) is now a Services Administrator (+a)", user->nick, user->username, user->hostip, user->vident, user->vhost);
				}
				else if (myadd == 0) 
				{
					alog("\002UserMode\002: %s (%s@%s => %s@%s) is no longer a Services Administrator (-a)", user->nick, user->username, user->hostip, user->vident, user->vhost);
				}
			break; 
			case 'A': /* Server Admin */
				if (myadd == 1) 
				{
					alog("\002UserMode\002: %s (%s@%s => %s@%s) is now a Server Administrator (+A)", user->nick, user->username, user->hostip, user->vident, user->vhost);
				}
				else if (myadd == 0) 
				{
					alog("\002UserMode\002: %s (%s@%s => %s@%s) is no longer a Services Administrator (-a)", user->nick, user->username, user->hostip, user->vident, user->vhost);
				} 
			break; 
			case 'N': /* Network Admin */ 
				if (myadd == 1) 
				{
					alog("\002UserMode\002: %s (%s@%s => %s@%s) is now a Network Administrator (+N)", user->nick, user->username, user->hostip, user->vident, user->vhost);	
				}
				else if (myadd == 0) 
				{
					alog("\002UserMode\002: %s (%s@%s => %s@%s) is no longer a Network Administrator (-N)", user->nick, user->username, user->hostip, user->vident, user->vhost);
				} 
			break; 
			case 'C': /* Co Admin */
				if (myadd == 1) 
				{
					alog("\002UserMode\002: %s (%s@%s => %s@%s) is now a Co Administrator (+C)", user->nick, user->username, user->hostip, user->vident, user->vhost);
				}
				else if (myadd == 0) 
				{
					alog("\002UserMode\002: %s (%s@%s => %s@%s) is no longer a Co Administrator (-C)", user->nick, user->username, user->hostip, user->vident, user->vhost);
				} 
			break; 
        }
	}
}

int log_away(char *source, int argc, char **argv) 
{
	User *u; 
		if (!(u = finduser(source))) { return MOD_CONT; } /* sanity */
		
		// if (argv[0]) away, else returning.
		if (argv[0]) { alog("\002Away\002: %s (%s@%s) is now marked as away", u->nick, u->vident, u->vhost); } 
		else { alog("\002Away\002: %s (%s@%s) is no longer marked as away", u->nick, u->vident, u->vhost); }
	return MOD_CONT; 
}

