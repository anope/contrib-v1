/***************************************************************************************/
/* Anope Module : ns_facebook : v1.0 (21/02/2013)                                      */
/* Contact info : westor7@gmail.com                                                    */
/* CODE HAS BEEN COPYIED FROM NS_PHONE, THANKS!                                        */
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

#define AUTHOR "westor"
#define VERSION "1.0"

/* Default database name */

#define DEFAULT_DB_NAME "facebook.db"

/* Multi-language stuff */

#define LANG_NUM_STRINGS   5
#define facebook_SYNTAX    0
#define facebook_ADD_SUCCESS   1
#define facebook_DEL_SUCCESS   2
#define facebook_HELP          3
#define facebook_HELP_CMD      4

char *facebookDBName = NULL;
int myAddFacebook(User * u);
int myNickInfo(User * u);

int mNickHelp(User * u);
void mMainNickHelp(User * u);
void m_AddLanguages(void);

int mLoadData(void);
int mSaveData(int argc, char **argv);
int mBackupData(int argc, char **argv);
int mLoadConfig();
int mEventReload(int argc, char **argv);

int AnopeInit(int argc, char **argv)
{
    Command *c;
    EvtHook *hook = NULL;

    int status;

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    alog("ns_facebook: Loading configuration directives...");

    if (mLoadConfig()) {
        return MOD_STOP;
    }

    c = createCommand("FACEBOOK", myAddFacebook, NULL, -1, -1, -1, -1, -1);
    moduleAddHelp(c, mNickHelp);
    status = moduleAddCommand(NICKSERV, c, MOD_HEAD);

    c = createCommand("INFO", myNickInfo, NULL, -1, -1, -1, -1, -1);
    status = moduleAddCommand(NICKSERV, c, MOD_TAIL);

    hook = createEventHook(EVENT_DB_SAVING, mSaveData);
    status = moduleAddEventHook(hook);

    hook = createEventHook(EVENT_DB_BACKUP, mBackupData);
    status = moduleAddEventHook(hook);



    hook = createEventHook(EVENT_RELOAD, mEventReload);
    status = moduleAddEventHook(hook);

    moduleSetNickHelp(mMainNickHelp);

    mLoadData();
    m_AddLanguages();

    return MOD_CONT;
}

void AnopeFini(void)
{
    char *av[1];

    av[0] = sstrdup(EVENT_START);
    mSaveData(1, av);
    free(av[0]);

    if (facebookDBName)
        free(facebookDBName);
}

int myAddFacebook(User * u)
{
    char *text = NULL;
    char *cmd = NULL;
    char *facebook = NULL;
    NickAlias *na = NULL;

    /* Get the last buffer anope recived */

    text = moduleGetLastBuffer();
    if (text) {
        cmd = myStrGetToken(text, ' ', 0);
        facebook = myStrGetTokenRemainder(text, ' ', 1);
        if (cmd && facebook) {
            if (strcasecmp(cmd, "ADD") == 0) {
                    /* ok we've found the user */
                    if ((na = findnick(u->nick))) {
                        /* Add the module data to the user */
                        moduleAddData(&na->nc->moduleData, "facebook", facebook);
                        moduleNoticeLang(s_NickServ, u,
                                         facebook_ADD_SUCCESS, u->nick);
                       /* NickCore not found! */
                    } else {
                        notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED,
                                   u->nick);
                }
            } else if (strcasecmp(cmd, "DEL") == 0) {

                /* ok we've found the user */
                if ((na = findnick(u->nick))) {
                   moduleDelData(&na->nc->moduleData, "facebook");
                   moduleNoticeLang(s_NickServ, u,
                   facebook_DEL_SUCCESS, u->nick);

                    /* NickCore not found! */

                } else {
                    notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED,
                                u->nick);
                }

                /* another syntax error! */

            } else {
                moduleNoticeLang(s_NickServ, u, facebook_SYNTAX);

            }

            free(cmd);
            free(facebook);

            /* Syntax error */

        } else if (cmd) {
            moduleNoticeLang(s_NickServ, u, facebook_SYNTAX);
            free(cmd);

            /* Syntax error */

        } else {
            moduleNoticeLang(s_NickServ, u, facebook_SYNTAX);
        }
    }

    return MOD_CONT;
}


int myNickInfo(User * u)
{
    char *text = NULL;
    char *nick = NULL;
    char *facebook = NULL;
    NickAlias *na = NULL;

        /* Get the last buffer anope recived */

        text = moduleGetLastBuffer();
	        if (text) {
        	    nick = myStrGetToken(text, ' ', 0);
       		   	if (nick) {
                /* ok we've found the user */
                if ((na = findnick(nick))) {
                    /* If we have any info on this user */
                   if ((facebook = moduleGetData(&na->nc->moduleData, "facebook"))) {
                        notice_user(s_NickServ, u, "    Facebook Page: http://www.fb.me/%s", facebook);
						free(facebook);
                    }

                    /* NickCore not found! */

                } else {

                    /* we dont care! */
            }

        }

    }

    return MOD_CONT;

}


int mLoadData(void)
{
    int ret = 0;
    FILE *in;

    char *type = NULL;
    char *name = NULL;
    char *info = NULL;

    int len = 0;

    NickAlias *na = NULL;



    /* will _never_ be this big thanks to the 512 limit of a message */

    char buffer[2000];
    if ((in = fopen(facebookDBName, "r")) == NULL) {
        alog("ns_facebook: WARNING: can not open the database file! (it might not exist, this is not fatal)");
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
                        if (stricmp(type, "N") == 0) {
                            if ((na = findnick(name))) {
                                moduleAddData(&na->nc->moduleData, "facebook", info);
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

int mSaveData(int argc, char **argv)
{
    NickCore *nc = NULL;
    int i = 0;
    int ret = 0;
    FILE *out;

    char *info = NULL;
    char *info2 = NULL;

    if (argc >= 1) {
        if (!stricmp(argv[0], EVENT_START)) {
            if ((out = fopen(facebookDBName, "w")) == NULL) {
                alog("ns_facebook: ERROR: can not open the database file!");
                anope_cmd_global(s_OperServ,
                                 "ns_facebook: ERROR: can not open the database file!");
                ret = 1;
            } else {
                for (i = 0; i < 1024; i++) {
                    for (nc = nclists[i]; nc; nc = nc->next) {

                        /* If we have any info on this user */
                        if ((info = moduleGetData(&nc->moduleData, "facebook"))) {
                            fprintf(out, "N %s %s\n", nc->display, info);
							free(info);
                        }
                        if ((info2 = moduleGetData(&nc->moduleData, "provider"))) {
                            fprintf(out, "N %s %s\n", nc->display, info2);
							free(info2);
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

int mBackupData(int argc, char **argv)
{
	ModuleDatabaseBackup(facebookDBName);
	return MOD_CONT;
}

int mLoadConfig(void)
{
    char *tmp = NULL;

    Directive directivas[] = {
        {"facebookDBName", {{PARAM_STRING, PARAM_RELOAD, &tmp}}},
    };

    Directive *d = &directivas[0];
    moduleGetConfigDirective(d);

    if (facebookDBName)
        free(facebookDBName);

    if (tmp) {
        facebookDBName = tmp;
    } else {
        facebookDBName = sstrdup(DEFAULT_DB_NAME);
        alog("ns_facebook: facebookDBName is not defined in Services configuration file, using default %s", facebookDBName);
    }
    alog("ns_facebook: Directive facebookDBName loaded (%s)...", facebookDBName);
    return 0;
}

int mEventReload(int argc, char **argv)
{
    int ret = 0;
    if (argc >= 1) {
        if (!stricmp(argv[0], EVENT_START)) {
            alog("ns_facebook: Reloading configuration directives...");
            ret = mLoadConfig();
        } else {
            /* Nothing for now */
        }
    }

    if (ret)
        alog("ns_facebook: ERROR: An error has occured while reloading the configuration file");
    return MOD_CONT;
}

void m_AddLanguages(void)
{
    char *langtable_en_us[] = {

        /* SYNTAX */
        "Syntax: FACEBOOK [ADD|DEL] <page>/n"
		"Example: /msg NickServ FACEBOOK ADD westor7",
        /* ADD_SUCCESS */
        "Facebook page has been added to nick %s",
        /* DEL_SUCCESS */
        "Facebook page has been removed from nick %s",
        /* HELP */
        "Syntax: FACEBOOK [ADD|DEL] <page>\n"
            "Add or Delete a facebook page to your nick\n"
            "This will show up when any user use /ns info nick to an user./n"
			"EXAMPLE: /msg NickServ FACEBOOK ADD westor7",
        /* HELP_CMD */
        "    FACEBOOK      Add / Del a facebook page to you nick",
    };
    moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}

int mNickHelp(User * u)
{
	moduleNoticeLang(s_NickServ, u, facebook_HELP);
	return MOD_CONT;
}

void mMainNickHelp(User * u)
{
	moduleNoticeLang(s_NickServ, u, facebook_HELP_CMD);
}

/* EOF */
