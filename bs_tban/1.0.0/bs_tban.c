/**
 * Simple module to make all botserv badword kicks timed.
 **/
#include "module.h"

/**
 * Some basic module constants
 **/
#define AUTHOR "Rob"
#define VERSION "1.0"
#define DEFAULT_DB_NAME "bs_tban.db"
#define LANG_NUM_STRINGS    6

#define BSTBAN_SYNTAX       0
#define BSTBAN_HELP         1
#define BSTBAN_SET          2
#define BSTBAN_OFF          3
#define BSTBAN_INVALID_TIME 4
#define BSTBAN_HELP_DETAIL  5


/*************************************************************************/
/* Globals */
/*************************************************************************/
char *BSTBanDBName = NULL;

/*************************************************************************/
/* Function definitions */
/*************************************************************************/
/**
 * Local functions called directly from this module
 **/
int loadData(void);
int loadConfig(void);
void addLangauges(void);

/**
 * Local callback functions
 **/
int delBan(int argc, char **argv);

/**
 * Functions to deal with commands from users.
 **/
int hndlAutoUnban(User *u);

/**
 * Events automatically called from anope's core
 **/
int hndlBotBan(int argc, char **argv);
int hndlSaveData(int argc, char **argv);
int hndlBackupData(int argc, char **argv);
int hndlReload(int argc, char **argv);

/**
 * Help functions to make our commands appear in the help menus
 **/
int botservHelp(User * u);
void mainBotservHelp(User * u);


/*************************************************************************/
/* AnopeInit, called automatically by Anope. */
/*************************************************************************/
int AnopeInit(int argc, char **argv) {
    EvtHook *hook;
    Command *c;
    int ret = MOD_CONT;

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);

    loadConfig();

    c = createCommand("autounban", hndlAutoUnban, NULL, -1, -1, -1, -1, -1);
    moduleAddHelp(c, botservHelp);
    if(moduleAddCommand(BOTSERV, c, MOD_HEAD) != MOD_ERR_OK) {
        alog("bs_tban: Unable to create command /bs autounban, module will unload");
	ret = MOD_STOP;
    }

    hook = createEventHook(EVENT_BOT_BAN, hndlBotBan);
    if(moduleAddEventHook(hook)!=MOD_ERR_OK) {
        alog("bs_tban: Unable to hook event EVENT_BOT_BAN, module will unload");
	ret = MOD_STOP;
    }

    hook = createEventHook(EVENT_DB_SAVING, hndlSaveData);
    if(moduleAddEventHook(hook)!=MOD_ERR_OK) {
        alog("bs_tban: Unable to hook to event EVENT_DB_SAVING, module will unload");
	ret = MOD_STOP;
    }

    hook = createEventHook(EVENT_DB_BACKUP, hndlBackupData);
    if(moduleAddEventHook(hook) != MOD_ERR_OK) {
        alog("bs_tban: Unable to hook to event EVENT_DB_BACKUP, module will unload");
        ret = MOD_STOP;
    }

    hook = createEventHook(EVENT_RELOAD, hndlReload);
    if(moduleAddEventHook(hook) != MOD_ERR_OK) {
       alog("bs_tban: Unable to hook to event EVENT_RELOAD, module will unload");
       ret = MOD_STOP;
    }

    loadData();
    addLangauges();

    return ret;
}

/**
 * The module is unloading, save the data and free up veriables.
 **/ 
void AnopeFini(void) {
    char *av[1];
    av[0] = sstrdup(EVENT_START);
    hndlSaveData(1,av);
    free(av[0]);

    if(BSTBanDBName) {
        free(BSTBanDBName);
    }
}

/*************************************************************************/
/* Events automatically called from anope's core */
/*************************************************************************/
/**
 * When the user has been banned this function will be called, it should setup
 * the callback with the correct params so that delBan can effectivally remove
 * the ban.  Checks for "should we monitor this channel" will need to be made
 * before any callback is added.
 *  EVENT_BOT_BAN
 *      A BotServ bot has banned a user, e.g. kickers.
 *	av[0]  The nick of the user banned.
 *	av[1]  The Channel the user was banned from.
 *	av[2]  The mask that was banned.
 **/
int hndlBotBan(int argc, char **argv) {
    char *cb[2];
    ChannelInfo *ci;
    char *timeout = NULL;
    int my_time = -1;

    if(argc >= 3 && ( ci = cs_findchan(argv[1])) )  {
        if( (timeout = moduleGetData(&ci->moduleData, "timeout")) ) {
	    if( (my_time = dotime(timeout) ) > 0) {
                cb[0] = ci->name;
                cb[1] = argv[2];
                moduleAddCallback("tban", time(NULL) + my_time, delBan, 2, cb); 
	    }
	}
    }

    return MOD_CONT;
}

/**
 * Save any relevant module data as/when anope decides to call save
 *    EVENT_DB_SAVING
 *      This event is emitted when the databases are being saved.
 *      av[0]  EVENT_START or EVENT_STOP, to indicate if it's emitted before
 *             or after the saving routines have been run.
 **/
int hndlSaveData(int argc, char **argv) {
    ChannelInfo *ci = NULL;
    int i = 0;
    int ret = 0;
    FILE *out;
    char *m_time;
    if(argc >= 1) {
        if (!stricmp(argv[0], EVENT_START)) {
            if ((out = fopen(BSTBanDBName, "w")) == NULL) {
	        alog("bs_tban: Unable to open database file, not saved");
		anope_cmd_global(s_OperServ,"bs_tban: Unable to open database file, not saved");
		ret = 1;
	    } else {
                for (i = 0; i < 256; i++) {
		    for (ci = chanlists[i]; ci; ci = ci->next) {
		        if ((m_time=moduleGetData(&ci->moduleData, "timeout"))) {
			    fprintf(out, "%s %s\n",ci->name,m_time);
			    free(m_time);
			}
		    }
		}
		fclose(out);
	    }
	}
    } 
    return MOD_CONT;
}

/**
 * Ensure we backup our database when anope calls the normal backup routines.
 *    EVENT_DB_BACKUP
 *      This event is emitted when the databases are backed up.
 *      av[0]  EVENT_START when the backup commences, and EVENT_STOP when it
 *             finishes.
 **/
int hndlBackupData(int argc, char **argv) {
    ModuleDatabaseBackup(BSTBanDBName);
    return MOD_CONT;
}

/**
 * Reload any config changes that have taken place when anope is reloaded.
 *    EVENT_RELOAD
 *      This event is emitted after the configuration file has been reloaded.
 *      av[0]  Always EVENT_START.
 **/
int hndlReload(int argc, char **argv) {
    int ret = MOD_CONT;
    if(argc >= 1) {
        if (!stricmp(argv[0], EVENT_START)) {
	    alog("bs_tban: Reloading config settings...");
	    ret = loadConfig();
	}
    }
    return ret;
}

/*************************************************************************/
/* Local callback functions */
/*************************************************************************/
/**
 * Remove the ban after the specified number of seconds have passed.  This
 * function will automatically be called by anope's core thanks to the callback
 * setup in hndlBotBan, it should pass in all the info we need to effectivally
 * remove the ban.
 **/
int delBan(int argc, char **argv) {
    char *av[3];
    Channel *c;
    
    if( argc>=2 && (c=findchan(argv[0])) ) {
        av[0] = sstrdup("-b");
        av[1] = argv[1];
        anope_cmd_mode(whosends(c->ci), c->name, "-b %s", av[1]);
        chan_set_modes(s_ChanServ, c, 2, av, 1);
        free(av[0]);
    } else {
        alog("bs_tban: Unable to remove ban, perhaps the channel vanished?");
    }
    return MOD_CONT;
}

/*************************************************************************/
/* Functions to deal with commands from users. */
/*************************************************************************/
int hndlAutoUnban(User *u) {
    char *text = NULL;
    char *channel = NULL;
    char *timeout = NULL;
    int m_time = -1;
    ChannelInfo *ci = NULL;

    text = moduleGetLastBuffer();

    if(text) {
        channel = myStrGetToken(text, ' ', 0);
	timeout = myStrGetTokenRemainder(text, ' ', 1);
	if(channel) {
	    if(timeout) {
	        if ((ci = cs_findchan(channel))) {
		    if((m_time = dotime(timeout)) >= 0) { /* Its a valid time */
                        if(m_time==0) {
                            moduleDelData(&ci->moduleData, "timeout");
                            moduleNoticeLang(s_BotServ, u, BSTBAN_OFF, s_BotServ, channel);
                        } else {
                            moduleAddData(&ci->moduleData, "timeout", timeout);
                            moduleNoticeLang(s_BotServ, u, BSTBAN_SET, s_BotServ, m_time, channel);
                        }
		    } else {
                        moduleNoticeLang(s_BotServ, u, BSTBAN_INVALID_TIME);
		    }
		} else {
		    notice_lang(s_BotServ, u, CHAN_X_NOT_REGISTERED, channel);
		}
                free(timeout);
	    } else {
                moduleNoticeLang(s_BotServ, u, BSTBAN_SYNTAX);
	    }
            free(channel);
	} else {
            moduleNoticeLang(s_BotServ, u, BSTBAN_SYNTAX);
	}
    } else {
        moduleNoticeLang(s_BotServ, u, BSTBAN_SYNTAX);
    }
    return MOD_CONT;
}


/*************************************************************************/
/* Local functions called directly from this module */
/*************************************************************************/
/**
 * Load data stored in our database.
 * @return 0 on success, none-zero if an error occured.
 **/
int loadData(void) {
    int ret = 0;
    FILE *in;
    char *name;
    char *m_time;
    char buffer[2000];
    ChannelInfo *ci = NULL;
    if( (in = fopen(BSTBanDBName, "r"))==NULL) {
        alog("bs_tban: Unable to open DB file [%s]",BSTBanDBName);
	ret = 1;
    } else {
        while(fgets(buffer,1500,in)) {
            name = myStrGetToken(buffer, ' ', 0);
	    m_time = myStrGetTokenRemainder(buffer, ' ', 1);
	    if(name) {
                if(m_time) {
		    if ((ci = cs_findchan(name))) {
                        moduleAddData(&ci->moduleData,"timeout",m_time);
		    } else {
                        alog("bs_tban: Invalid entry in db file, removing");
		    }
                    free(m_time);
		} else {
                    alog("bs_tban: Invalid entry in db file, removing");
		}
		free(name);
	    } else {
                alog("bs_tban: Invalid entry in db file, removing");
	    }
	}
	fclose(in);
    }
    return ret;
}

/**
 * Load any config settings we might have, e.g. database name
 * @return 0 on success, none-zero to indicate an error
 **/
int loadConfig(void) {
    char *tmp = NULL;
    Directive directives[] = {
        {"BSTBanDBName", {{PARAM_STRING, PARAM_RELOAD, &tmp}}},
    };
    Directive *d = &directives[0];
    moduleGetConfigDirective(d);
    if(BSTBanDBName) {
        free(BSTBanDBName);
    }
    if(tmp) {
        BSTBanDBName = tmp;
    } else {
        BSTBanDBName = sstrdup(DEFAULT_DB_NAME);
    }
    return MOD_CONT;
}

/**
 * Add our various langauge tables to anope, this will allow us to
 * respond to uses in thier native language (assuming we know how to!)
 **/
void addLangauges(void) {
    char *langtable_en_us[] = {
        /* BSTBAN_SYNTAX */
        "Syntax: AUTOUNBAN #channel timeout",
	/* BSTBAN_HELP */
	"    AUTOUNBAN  Automatically remove bans set by botserv auto-kicks",
	/* BSTBAN_SET */
	"%s bans will be auto removed in %d seconds from %s.",
	/* BSTBAN_OFF */
	"%s bans will no longer be removed from %s",
	/* BSTBAN_INVALID_TIME */
	"Invalid time, please use seconds or normal time format, e.g. 5m",
	/* BSTBAN_HELP_DETAIL */
	"Automatically removes any bans placed by botserv after the given\n"
	"timeout is reached."
    };
    moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}

/*************************************************************************/
/* Help functions to make our commands appear in the help menus */
/*************************************************************************/
/**
 * Function to provide basic help on the controling command
 * @param u The user who requested help
 * @return MOD_CONT to carry on processing other modules, MOD_STOP to stop.
 **/
int botservHelp(User * u) {
    moduleNoticeLang(s_BotServ, u, BSTBAN_SYNTAX);
    notice(s_BotServ, u->nick, "");
    moduleNoticeLang(s_BotServ, u, BSTBAN_HELP_DETAIL);
    return MOD_CONT;
}

/**
 * Ensure our command is added to the main help menu.
 * @param u The user who requested help
 **/
void mainBotservHelp(User * u) {
    moduleNoticeLang(s_BotServ, u, BSTBAN_HELP);
}

/*************************************************************************/
/* EOF */
/*************************************************************************/


