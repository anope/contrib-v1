/*----------------------------------------------------------------------------*\
|                                                                              |
|  Module:        NickServ Lock Nick Registration (ns_locknickreg)             |
|  Author:        lnx85 <lnx85@simosnap.org>                                   |
|  Created:       July 9, 2008                                                 |
|  Version:       0.1                                                          |
|  Description:   Module blocks registration of nicks matching a config list.  |
|                 Useful to block registration of certain nicks, for example   |
|                 applet guest nicks like Anon|XXXX or Java|XXXX.              |
|                 It can also be used to protect nicks starting with admins    |
|                 nick (i.e. lnx85* prevent registration of all nicks similar  |
|                 such as lnx85`afk, lnx85`out and much more...)               |
|                                                                              |
|  For more information contact me in #developers on SimosNap IRC Network      |
|  irc://irc.simosnap.com/developers                                           |
|                                                                              |
+------------------------------------------------------------------------------+
| ChangeLog                                                                    |
+------------------------------------------------------------------------------+
|                                                                              |
| Version 0.1 (July 9, 2008) - Initial version                                 |
|                                                                              |
+------------------------------------------------------------------------------+
| Configuration Information                                                    |
+------------------------------------------------------------------------------+
|                                                                              |
|  This module requires the following configuration values to be added to      |
|  services.conf before the module will load.                                  |
|                                                                              |
|  The values are listed below along with an explanation of what each one      |
|  does.                                                                       |
|                                                                              |
|  - LockNickRegNicks "list of nick (wildcards allowed) goes here"             |
|                                                                              |
|  - LockNickRegMessage1 "Message goes here"                                   |
|    This will be the first line of the message sent to users when their       |
|    nickname registration is denied.                                          |
|                                                                              |
|  - LockNickRegMessage2 "Message goes here"                                   |
|    This will be the second line of the message sent to users when their      |
|    nickname registration is denied. Set to "" to have no second line.        |
|                                                                              |
|  - LockNickRegMessage3 "Message goes here"                                   |
|    This will be the third line of the message sent to users when their       |
|    nickname registration is denied. Set to "" to have no third line.         |
|                                                                              |
\*----------------------------------------------------------------------------*/
 
#include "module.h"
#define AUTHOR "lnx85 (lnx85@simosnap.org)"
#define VERSION "0.1"
#define MODNAME "ns_locknickreg"
 
// -----------------------------------------------------------------------------
//        PLEASE DO NOT EDIT ANYTHING BELOW HERE - MODULE CODE BEGINS   
// -----------------------------------------------------------------------------

char *LockNickRegNicks = NULL;
char *LockNickRegMessage1 = NULL;
char *LockNickRegMessage2 = NULL;
char *LockNickRegMessage3 = NULL;
char **LockNickRegList;
int LockNickRegNum = 0;


// -----------------------------------------------------------------------------
//                                 PROTOTYPES
// -----------------------------------------------------------------------------

int AnopeInit(int argc, char **argv);
void AnopeFini();
int LockNickReg_Register(User *u);
int LockNickReg_CheckNick(User *u);
int LockNickReg_Configured();
int LockNickReg_Reload(int argc, char **argv);
int LockNickReg_LoadConfig();

// -----------------------------------------------------------------------------
//                                  FUNCTIONS
// -----------------------------------------------------------------------------

/* Anope initialization event */

int AnopeInit(int argc, char **argv) {
    Command *c = NULL; EvtHook *hook = NULL; User *user = NULL;
	
	if (argc == 1) user = finduser(argv[0]);     
    
    c = createCommand("REGISTER", LockNickReg_Register, NULL, -1, -1, -1, -1, -1);
    moduleAddCommand(NICKSERV, c, MOD_HEAD);
    
    hook = createEventHook(EVENT_RELOAD, LockNickReg_Reload);
    moduleAddEventHook(hook);
    
    if (!LockNickReg_LoadConfig()) {
        if (user) notice(s_OperServ, user->nick, "ERROR: Missing configuration "
			"options in services.conf - Please see the logs for details");
        return MOD_STOP;
    }
                
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    moduleMinVersion(1, 7, 9, 0);
    alog("[%s] Module Loaded Successfully!", MODNAME);
    
    return MOD_CONT;
}

// -----------------------------------------------------------------------------

/* Anope finalization - unloads the module */

void AnopeFini() {
	int i;

	if (LockNickRegNicks) free(LockNickRegNicks);
    if (LockNickRegMessage1) free(LockNickRegMessage1);
    if (LockNickRegMessage2) free(LockNickRegMessage2);
    if (LockNickRegMessage3) free(LockNickRegMessage3);       

	if (LockNickRegNum) {
		for (i = 0; i < LockNickRegNum; i++) {
			free(LockNickRegList[i]);
		}
	}
	alog("[%s] Module Unloaded Successfully!", MODNAME);
}

// -----------------------------------------------------------------------------

/* LockNickReg_Register - Handles the Register command */

int LockNickReg_Register(User *u) {
    int status;
    
    if (!u || !u->nick) {
        return MOD_STOP;
    } else if (readonly) {
        notice_lang(s_NickServ, u, NICK_REGISTRATION_DISABLED);
        return MOD_STOP;
    } else if (checkDefCon(DEFCON_NO_NEW_NICKS)) {
        notice_lang(s_NickServ, u, OPER_DEFCON_DENIED);
        return MOD_STOP;
    } else if (u->na) {
        if (u->na->status & NS_VERBOTEN) {
            alog("%s: %s@%s tried to register FORBIDden nick %s", s_NickServ, 
				u->username, common_get_vhost(u), u->nick);
            notice_lang(s_NickServ, u, NICK_CANNOT_BE_REGISTERED, u->nick);
        } else {
			notice_lang(s_NickServ, u, NICK_ALREADY_REGISTERED, u->nick);
        }
        return MOD_STOP;
    }
    
    status = LockNickReg_CheckNick(u);
    
    if (status != MOD_CONT) {
        if (stricmp(LockNickRegMessage1, "") != 0)
            notice(s_NickServ, u->nick, LockNickRegMessage1);
        if (stricmp(LockNickRegMessage2, "") != 0)
            notice(s_NickServ, u->nick, LockNickRegMessage2);
        if (stricmp(LockNickRegMessage3, "") != 0)
            notice(s_NickServ, u->nick, LockNickRegMessage3);
    }
    
    return status;
}

// -----------------------------------------------------------------------------

/* LockNickReg_CheckNick - Check if nick matches some masks in config file */

int LockNickReg_CheckNick(User *u)
{
    int retval = MOD_STOP, i;
	char *token;
    
    if (!LockNickReg_Configured()) {
        alog("[%s] ERROR: There are missing values in services.conf - Please "
			"configure these before using this module", MODNAME);
        return MOD_CONT;
    }

	retval = MOD_CONT;

	for (i = 0; i < LockNickRegNum; i++) {
		if (match_wild_nocase(LockNickRegList[i], u->nick)) {
			retval = MOD_STOP;
		}
	}
    
    return retval;
}

int LockNickReg_Configured() {
    return (LockNickRegNicks && LockNickRegMessage1 && LockNickRegMessage2 && 
		LockNickRegMessage3);
}
        
// -----------------------------------------------------------------------------

/* LockNickReg_Reload - Handles the OperServ Reload event */

int LockNickReg_Reload(int argc, char **argv) {
    LockNickReg_LoadConfig();
    return MOD_CONT;
}

// -----------------------------------------------------------------------------

/* LockNickReg_LoadConfig - Loads the modules configuration directives */

int LockNickReg_LoadConfig() {                             
	int i, count; char *s;
    Directive directives[] = {
        {"LockNickRegNicks", {{PARAM_STRING, PARAM_RELOAD, &LockNickRegNicks}}},
        {"LockNickRegMessage1", {{PARAM_STRING, PARAM_RELOAD, &LockNickRegMessage1}}},
        {"LockNickRegMessage2", {{PARAM_STRING, PARAM_RELOAD, &LockNickRegMessage2}}},
        {"LockNickRegMessage3", {{PARAM_STRING, PARAM_RELOAD, &LockNickRegMessage3}}}
    };

	count = 4;
    for (i = 0; i < count; i++) {
        if (!&directives[i]) break;
        moduleGetConfigDirective(&directives[i]);      
        if (!(*(char **) (&directives[i])->params[0].ptr)) {
            alog("[%s] ERROR: Missing configuration option '%s' - Please read"
				" %s.c before compiling", MODNAME, (&directives[i])->name, MODNAME);
            return false;
        }            
    }

	if (LockNickRegNum) {
		for (i = 0; i < LockNickRegNum; i++) {
			free(LockNickRegList[i]);
		}
	}

	if (LockNickRegNicks) {
		LockNickRegNum = 0;
        s = strtok(LockNickRegNicks, " ");
        do {
            if (s) {
                LockNickRegNum++;
                LockNickRegList = realloc(LockNickRegList, sizeof(char *) * LockNickRegNum);
                LockNickRegList[LockNickRegNum - 1] = sstrdup(s);
            }
        } while ((s = strtok(NULL, " ")));
	}
	
	if (debug) alog("[%s] debug: Successfully loaded %d configuration options!", 
		MODNAME, count);
    return true;
}

// -----------------------------------------------------------------------------
