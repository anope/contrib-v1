/***************************************************************************************/
/* Anope Module : ircd_clan_info.c : v1.x                                              */
/* Scott Seufert - katsklaw@ircmojo.net                                                */
/*                                                                                     */
/* Anope (c) 2000-2002 Anope.org                                                       */
/*                                                                                     */
/* This program is free software; you can redistribute it and/or modify it under the   */
/* terms of the GNU General Public License as published by the Free Software           */
/* Foundation; either version 1, or (at your option) any later version.                */
/*                                                                                     */
/*  This program is distributed in the hope that it will be useful, but WITHOUT ANY    */
/*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A    */
/*  PARTICULAR PURPOSE.  See the GNU General Public License for more details.          */
/*                                                                                     */
/***************************************************************************************/
#include <module.h>

#define AUTHOR "katsklaw"
#define VERSION "1.0.0"

/* Default database name */
#define DEFAULT_DB_NAME "clan_info.db"

/* Multi-language stuff */
#define LANG_NUM_STRINGS   10

#define clan_SYNTAX        0
#define clan_ADD_SUCCESS   1
#define clan_DEL_SUCCESS   2
#define Cclan_SYNTAX       3
#define Cclan_ADD_SUCCESS  4
#define Cclan_DEL_SUCCESS  5
#define clan_HELP          6
#define Cclan_HELP         7
#define clan_HELP_CMD      8
#define Cclan_HELP_CMD     9

/*************************************************************************/

char *clanDBName = NULL;

int myAddCommNickInfo(User * u);
int myAddCommChanInfo(User * u);
int myNickInfo(User * u);
int myChanInfo(User * u);

int mNickHelp(User * u);
int mChanHelp(User * u);
void mMainChanHelp(User * u);
void mMainNickHelp(User * u);
void m_AddLanguages(void);

int mLoadData(void);
int mSaveData(int argc, char **argv);
int mBackupData(int argc, char **argv);
int mLoadConfig();
int mEventReload(int argc, char **argv);

/*************************************************************************/

/**
 * AnopeInit is called when the module is loaded
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv)
{
    Command *c;
    EvtHook *hook = NULL;

    int status;

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);

    alog("clan_info: Loading configuration directives...");
    if (mLoadConfig()) {
        return MOD_STOP;
    }

    c = createCommand("CLAN", myAddCommNickInfo, is_oper, -1, -1, -1, -1, -1);
    moduleAddHelp(c, mNickHelp);
    status = moduleAddCommand(NICKSERV, c, MOD_HEAD);

    c = createCommand("INFO", myNickInfo, NULL, -1, -1, -1, -1, -1);
    status = moduleAddCommand(NICKSERV, c, MOD_TAIL);

    c = createCommand("CLAN", myAddCommChanInfo, is_oper, -1, -1, -1, -1, -1);
    moduleAddHelp(c, mChanHelp);
    status = moduleAddCommand(CHANSERV, c, MOD_HEAD);

    c = createCommand("INFO", myChanInfo, NULL, -1, -1, -1, -1, -1);
    status = moduleAddCommand(CHANSERV, c, MOD_TAIL);

    hook = createEventHook(EVENT_DB_SAVING, mSaveData);
    status = moduleAddEventHook(hook);

    hook = createEventHook(EVENT_DB_BACKUP, mBackupData);
    status = moduleAddEventHook(hook);

    hook = createEventHook(EVENT_RELOAD, mEventReload);
    status = moduleAddEventHook(hook);

    moduleSetNickHelp(mMainNickHelp);
    moduleSetChanHelp(mMainChanHelp);

    mLoadData();
    m_AddLanguages();

    return MOD_CONT;
}

/**
 * Unload the module
 **/
void AnopeFini(void)
{
    char *av[1];

    av[0] = sstrdup(EVENT_START);
    mSaveData(1, av);
    free(av[0]);

    if (clanDBName)
        free(clanDBName);
}

/*************************************************************************/

/**
 * Provide the user interface to add/remove/update oper information
 * about a nick.
 * We are going to assume that anyone who gets this far is an oper;
 * the createCommand should have handled this checking for us and its 
 * tedious / a waste to do it twice.
 * @param u The user who executed this command
 * @return MOD_CONT if we want to process other commands in this command
 * stack, MOD_STOP if we dont
 **/
int myAddCommNickInfo(User * u)
{
    char *text = NULL;
    char *cmd = NULL;
    char *nick = NULL;
    char *info = NULL;
    NickAlias *na = NULL;

    /* Get the last buffer anope recived */
    text = moduleGetLastBuffer();
    if (text) {
        cmd = myStrGetToken(text, ' ', 0);
        nick = myStrGetToken(text, ' ', 1);
        info = myStrGetTokenRemainder(text, ' ', 2);
        if (cmd && nick) {
            if (strcasecmp(cmd, "ADD") == 0) {
                /* Syntax error, again! */
                if (info) {
                    /* ok we've found the user */
                    if ((na = findnick(nick))) {
                        /* Add the module data to the user */
                        moduleAddData(&na->nc->moduleData, "info", info);
                        moduleNoticeLang(s_NickServ, u,
                                         clan_ADD_SUCCESS, nick);
                        /* NickCore not found! */
                    } else {
                        notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED,
                                    nick);
                    }
                    free(info);
                }
            } else if (strcasecmp(cmd, "DEL") == 0) {
                /* ok we've found the user */
                if ((na = findnick(nick))) {
                    moduleDelData(&na->nc->moduleData, "info");
                    moduleNoticeLang(s_NickServ, u,
                                     clan_DEL_SUCCESS, nick);
                    /* NickCore not found! */
                } else {
                    notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED,
                                nick);
                }
                /* another syntax error! */
            } else {
                moduleNoticeLang(s_NickServ, u, clan_SYNTAX);
            }
            free(cmd);
            free(nick);
            /* Syntax error */
        } else if (cmd) {
            moduleNoticeLang(s_NickServ, u, clan_SYNTAX);
            free(cmd);
            /* Syntax error */
        } else {
            moduleNoticeLang(s_NickServ, u, clan_SYNTAX);
        }
    }
    return MOD_CONT;
}

/**
 * Provide the user interface to add/remove/update oper information
 * about a channel.
 * We are going to assume that anyone who gets this far is an oper; 
 * the createCommand should have handled this checking for us and 
 * its tedious / a waste to do it twice.
 * @param u The user who executed this command
 * @return MOD_CONT if we want to process other commands in this command
 * stack, MOD_STOP if we dont
 **/
int myAddCommChanInfo(User * u)
{
    char *text = NULL;
    char *cmd = NULL;
    char *chan = NULL;
    char *info = NULL;
    ChannelInfo *ci = NULL;

    /* Get the last buffer anope recived */
    text = moduleGetLastBuffer();
    if (text) {
        cmd = myStrGetToken(text, ' ', 0);
        chan = myStrGetToken(text, ' ', 1);
        info = myStrGetTokenRemainder(text, ' ', 2);
        if (cmd && chan) {
            if (strcasecmp(cmd, "ADD") == 0) {
                if (info) {
                    if ((ci = cs_findchan(chan))) {
                        /* Add the module data to the channel */
                        moduleAddData(&ci->moduleData, "info", info);
                        moduleNoticeLang(s_ChanServ, u,
                                         Cclan_ADD_SUCCESS, chan);
                        /* ChanInfo */
                    } else {
                        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED,
                                    chan);
                    }
                    free(info);
                }
            } else if (strcasecmp(cmd, "DEL") == 0) {
                if ((ci = cs_findchan(chan))) {
                    /* Del the module data from the channel */
                    moduleDelData(&ci->moduleData, "info");
                    moduleNoticeLang(s_ChanServ, u,
                                     Cclan_DEL_SUCCESS, chan);
                    /* ChanInfo */
                } else {
                    notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED,
                                chan);
                }
                /* another syntax error! */
            } else {
                moduleNoticeLang(s_ChanServ, u, Cclan_SYNTAX);
            }
            free(cmd);
            free(chan);
            /* Syntax error */
        } else if (cmd) {
            moduleNoticeLang(s_ChanServ, u, Cclan_SYNTAX);
            free(cmd);
            /* Syntax error */
        } else {
            moduleNoticeLang(s_ChanServ, u, Cclan_SYNTAX);
        }
    }
    return MOD_CONT;
}

/*************************************************************************/

/**
 * Called after a user does a /msg nickserv info [nick] 
 * @param u The user who requested info
 * @return MOD_CONT to continue processing commands or MOD_STOP to stop
 **/
int myNickInfo(User * u)
{
    char *text = NULL;
    char *nick = NULL;
	char *info = NULL;
    NickAlias *na = NULL;

        /* Get the last buffer anope recived */
        text = moduleGetLastBuffer();
        if (text) {
            nick = myStrGetToken(text, ' ', 0);
            if (nick) {
                /* ok we've found the user */
                if ((na = findnick(nick))) {
                    /* If we have any info on this user */
                    if ((info = moduleGetData(&na->nc->moduleData, "info"))) {
                        notice_user(s_NickServ, u, "      Clan: %s", info);
						free(info);
                    }
                    /* NickCore not found! */
                } else {
                    /* we dont care! */
                }
                free(nick);
            }
        }
    return MOD_CONT;
}

/**
 * Called after a user does a /msg chanserv info chan 
 * @param u The user who requested info
 * @return MOD_CONT to continue processing commands or MOD_STOP to stop
 **/
int myChanInfo(User * u)
{
    char *text = NULL;
    char *chan = NULL;
	char *info = NULL;
    ChannelInfo *ci = NULL;

        /* Get the last buffer anope recived */
        text = moduleGetLastBuffer();
        if (text) {
            chan = myStrGetToken(text, ' ', 0);
            if (chan) {
                if ((ci = cs_findchan(chan))) {
                    /* If we have any info on this channel */
                    if ((info = moduleGetData(&ci->moduleData, "info"))) {
                        notice_user(s_ChanServ, u, "      Clan: %s", info);
						free(info);
                    }
                }
                free(chan);
            }
        }
    return MOD_CONT;
}

/*************************************************************************/

/** 
 * Load data from the db file, and populate our clanInfo lines
 * @return 0 for success
 **/
int mLoadData(void)
{
    int ret = 0;
    FILE *in;

    char *type = NULL;
    char *name = NULL;
    char *info = NULL;
    int len = 0;

    ChannelInfo *ci = NULL;
    NickAlias *na = NULL;

    /* will _never_ be this big thanks to the 512 limit of a message */
    char buffer[2000];
    if ((in = fopen(clanDBName, "r")) == NULL) {
        alog("ircd_clan_info: WARNING: can not open the database file! (it might not exist, this is not fatal)");
        ret = 1;
    } else {
        while (fgets(buffer, 1500, in)) {
            type = myStrGetToken(buffer, ' ', 0);
            name = myStrGetToken(buffer, ' ', 1);
            info = myStrGetTokenRemainder(buffer, ' ', 2);
            if (type) {
                if (name) {
                    if (info) {
                        len = strlen(info);
                        /* Take the \n from the end of the line */
                        info[len - 1] = '\0';
                        if (stricmp(type, "C") == 0) {
                            if ((ci = cs_findchan(name))) {
                                moduleAddData(&ci->moduleData, "info",
                                              info);
                            }
                        } else if (stricmp(type, "N") == 0) {
                            if ((na = findnick(name))) {
                                moduleAddData(&na->nc->moduleData, "info",
                                              info);
                            }
                        }
                        free(info);
                    }
                    free(name);
                }
                free(type);
            }
        }
    }
    return ret;
}

/** 
 * Save all our data to our db file
 * First walk through the nick CORE list, and any nick core which has
 * oper info attached to it, write to the file.
 * Next do the same again for ChannelInfos
 * @return 0 for success
 **/
int mSaveData(int argc, char **argv)
{
    ChannelInfo *ci = NULL;
    NickCore *nc = NULL;
    int i = 0;
    int ret = 0;
    FILE *out;
    char *info = NULL;

    if (argc >= 1) {
        if (!stricmp(argv[0], EVENT_START)) {
            if ((out = fopen(clanDBName, "w")) == NULL) {
                alog("ircd_clan_info: ERROR: can not open the database file!");
                anope_cmd_global(s_OperServ,
                                 "ircd_clan_info: ERROR: can not open the database file!");
                ret = 1;
            } else {
                for (i = 0; i < 1024; i++) {
                    for (nc = nclists[i]; nc; nc = nc->next) {
                        /* If we have any info on this user */
                        if ((info = moduleGetData(&nc->moduleData, "info"))) {
                            fprintf(out, "N %s %s\n", nc->display, info);
							free(info);
                        }
                    }
                }


                for (i = 0; i < 256; i++) {
                    for (ci = chanlists[i]; ci; ci = ci->next) {
                        /* If we have any info on this channel */
                        if ((info = moduleGetData(&ci->moduleData, "info"))) {
                            fprintf(out, "C %s %s\n", ci->name, info);
							free(info);
                        }
                    }
                }
                fclose(out);
            }
        } else {
            ret = 0;
        }
    }

    return ret;
}

/** 
 * Backup our databases using the commands provided by Anope
 * @return MOD_CONT
 **/
int mBackupData(int argc, char **argv)
{
	ModuleDatabaseBackup(clanDBName);
	
	return MOD_CONT;
}

/** 
 * Load the configuration directives from Services configuration file.
 * @return 0 for success
 **/
int mLoadConfig(void)
{
    char *tmp = NULL;

    Directive directivas[] = {
        {"clanDBName", {{PARAM_STRING, PARAM_RELOAD, &tmp}}},
    };

    Directive *d = &directivas[0];
    moduleGetConfigDirective(d);

    if (clanDBName)
        free(clanDBName);

    if (tmp) {
        clanDBName = tmp;
    } else {
        clanDBName = sstrdup(DEFAULT_DB_NAME);
        alog("ircd_clan_info: clanDBName is not defined in Services configuration file, using default %s", clanDBName);
    }

    alog("ircd_clan_info: Directive clanDBName loaded (%s)...", clanDBName);

    return 0;
}

/** 
 * Manage the RELOAD EVENT
 * @return MOD_CONT
 **/
int mEventReload(int argc, char **argv)
{
    int ret = 0;
    if (argc >= 1) {
        if (!stricmp(argv[0], EVENT_START)) {
            alog("ircd_clan_info: Reloading configuration directives...");
            ret = mLoadConfig();
        } else {
            /* Nothing for now */
        }
    }

    if (ret)
        alog("ircd_clan_info.c: ERROR: An error has occured while reloading the configuration file");

    return MOD_CONT;
}

/*************************************************************************/

/**
 * manages the multilanguage stuff
 **/
void m_AddLanguages(void)
{
    char *langtable_en_us[] = {
        /* clan_SYNTAX */
        "Syntax: clan [ADD|DEL] nick <info>",
        /* clan_ADD_SUCCESS */
        "clanInfo line has been added to nick %s",
        /* clan_DEL_SUCCESS */
        "clanInfo line has been removed from nick %s",
        /* Cclan_SYNTAX */
        "Syntax: clan [ADD|DEL] chan <info>",
        /* Cclan_ADD_SUCCESS */
        "clanInfo line has been added to channel %s",
        /* Cclan_DEL_SUCCESS */
        "clanInfo line has been removed from channel %s",
        /* clan_HELP */
        "Syntax: clan [ADD|DEL] nick <info>\n"
            "Add or Delete Oper information for the given nick\n"
            "This will show up when any oper /ns info nick's the user.\n"
            "and can be used for 'tagging' users etc....",
        /* Cclan_HELP */
        "Syntax: clan [ADD|DEL] chan <info>\n"
            "Add or Delete Oper information for the given channel\n"
            "This will show up when any oper /cs info's the channel.\n"
            "and can be used for 'tagging' channels etc....",
        /* clan_HELP_CMD */
        "    CLAN         Add / Del an clanInfo line to a nick",
        /* Cclan_HELP_CMD */
        "    CLAN         Add / Del an clanInfo line to a channel"
    };

    moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}

/*************************************************************************/

int mNickHelp(User * u)
{
    if (is_oper(u)) {
        moduleNoticeLang(s_NickServ, u, clan_HELP);
    } else {
        notice_lang(s_NickServ, u, NO_HELP_AVAILABLE, "clan");
    }
    return MOD_CONT;
}

int mChanHelp(User * u)
{
    if (is_oper(u)) {
        moduleNoticeLang(s_ChanServ, u, Cclan_HELP);
    } else {
        notice_lang(s_ChanServ, u, NO_HELP_AVAILABLE, "clan");
    }
    return MOD_CONT;
}

/* This help will be added to the main NickServ list */
void mMainNickHelp(User * u)
{
    if (is_oper(u)) {
        moduleNoticeLang(s_NickServ, u, clan_HELP_CMD);
    }
}

/* This help will be added to the main NickServ list */
void mMainChanHelp(User * u)
{
    if (is_oper(u)) {
        moduleNoticeLang(s_ChanServ, u, Cclan_HELP_CMD);
    }
}

/*************************************************************************/

/* EOF */
