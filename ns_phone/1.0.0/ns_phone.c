/***************************************************************************************/
/* Anope Module : ns_phone : v1.x                                                      */
/* Scott Seufert - katsklaw@ircmojo.net                                                */
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

#define DEFAULT_DB_NAME "phone.db"

/* Multi-language stuff */

#define LANG_NUM_STRINGS   5
#define phone_SYNTAX        0
#define phone_ADD_SUCCESS   1
#define phone_DEL_SUCCESS   2
#define phone_HELP          3
#define phone_HELP_CMD      4

char *phoneDBName = NULL;
int myAddPhone(User * u);
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
    alog("ns_phone: Loading configuration directives...");

    if (mLoadConfig()) {
        return MOD_STOP;
    }

    c = createCommand("PHONE", myAddPhone, NULL, -1, -1, -1, -1, -1);
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

    if (phoneDBName)
        free(phoneDBName);
}

int myAddPhone(User * u)
{
    char *text = NULL;
    char *cmd = NULL;
    char *phone = NULL;
    NickAlias *na = NULL;

    /* Get the last buffer anope recived */

    text = moduleGetLastBuffer();
    if (text) {
        cmd = myStrGetToken(text, ' ', 0);
        phone = myStrGetTokenRemainder(text, ' ', 1);
        if (cmd && phone) {
            if (strcasecmp(cmd, "ADD") == 0) {
                    /* ok we've found the user */
                    if ((na = findnick(u->nick))) {
                        /* Add the module data to the user */
                        moduleAddData(&na->nc->moduleData, "phone", phone);
                        moduleNoticeLang(s_NickServ, u,
                                         phone_ADD_SUCCESS, u->nick);
                       /* NickCore not found! */
                    } else {
                        notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED,
                                   u->nick);
                }
            } else if (strcasecmp(cmd, "DEL") == 0) {

                /* ok we've found the user */
                if ((na = findnick(u->nick))) {
                   moduleDelData(&na->nc->moduleData, "phone");
                   moduleNoticeLang(s_NickServ, u,
                   phone_DEL_SUCCESS, u->nick);

                    /* NickCore not found! */

                } else {
                    notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED,
                                u->nick);
                }

                /* another syntax error! */

            } else {
                moduleNoticeLang(s_NickServ, u, phone_SYNTAX);

            }

            free(cmd);
            free(phone);

            /* Syntax error */

        } else if (cmd) {
            moduleNoticeLang(s_NickServ, u, phone_SYNTAX);
            free(cmd);

            /* Syntax error */

        } else {
            moduleNoticeLang(s_NickServ, u, phone_SYNTAX);
        }
    }

    return MOD_CONT;
}


int myNickInfo(User * u)
{
    char *text = NULL;
    char *nick = NULL;
    char *phone = NULL;
    NickAlias *na = NULL;

        /* Get the last buffer anope recived */

        text = moduleGetLastBuffer();
	if (is_oper(u)) {
	        if (text) {
        	    nick = myStrGetToken(text, ' ', 0);
       		   	if (nick) {

                /* ok we've found the user */
                if ((na = findnick(nick))) {

                    /* If we have any info on this user */
                   if ((phone = moduleGetData(&na->nc->moduleData, "phone"))) {
                        notice_user(s_NickServ, u, "      Phone: %s", phone);
						free(phone);

                    }

                    /* NickCore not found! */

                } else {

                    /* we dont care! */

                }

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
    if ((in = fopen(phoneDBName, "r")) == NULL) {
        alog("ns_phone: WARNING: can not open the database file! (it might not exist, this is not fatal)");
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
                                moduleAddData(&na->nc->moduleData, "phone", info);
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
            if ((out = fopen(phoneDBName, "w")) == NULL) {
                alog("ns_phone: ERROR: can not open the database file!");
                anope_cmd_global(s_OperServ,
                                 "ns_phone: ERROR: can not open the database file!");
                ret = 1;
            } else {
                for (i = 0; i < 1024; i++) {
                    for (nc = nclists[i]; nc; nc = nc->next) {

                        /* If we have any info on this user */
                        if ((info = moduleGetData(&nc->moduleData, "phone"))) {
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
	ModuleDatabaseBackup(phoneDBName);
	return MOD_CONT;
}

int mLoadConfig(void)
{
    char *tmp = NULL;

    Directive directivas[] = {
        {"phoneDBName", {{PARAM_STRING, PARAM_RELOAD, &tmp}}},
    };

    Directive *d = &directivas[0];
    moduleGetConfigDirective(d);

    if (phoneDBName)
        free(phoneDBName);

    if (tmp) {
        phoneDBName = tmp;
    } else {
        phoneDBName = sstrdup(DEFAULT_DB_NAME);
        alog("ns_phone: phoneDBName is not defined in Services configuration file, using default %s", phoneDBName);
    }
    alog("ns_phone: Directive phoneDBName loaded (%s)...", phoneDBName);
    return 0;
}

int mEventReload(int argc, char **argv)
{
    int ret = 0;
    if (argc >= 1) {
        if (!stricmp(argv[0], EVENT_START)) {
            alog("ns_phone: Reloading configuration directives...");
            ret = mLoadConfig();
        } else {
            /* Nothing for now */
        }
    }

    if (ret)
        alog("ns_phone: ERROR: An error has occured while reloading the configuration file");
    return MOD_CONT;
}

void m_AddLanguages(void)
{
    char *langtable_en_us[] = {

        /* SYNTAX */
        "Syntax: PHONE [ADD|DEL] number <provider>",
        /* ADD_SUCCESS */
        "Phone number has been added to nick %s",
        /* DEL_SUCCESS */
        "Phone has been removed from nick %s",
        /* HELP */
        "Syntax:PHON[ADD|DELnumbe <provider>\n"
            "Add or Delete a phone number to your nick\n"
            "This will show up when any oper /ns info nick's the user.",
        /* HELP_CMD */
        "    PHONE         Add / Del a phone number to you nick",
    };
    moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}

int mNickHelp(User * u)
{
	moduleNoticeLang(s_NickServ, u, phone_HELP);
	return MOD_CONT;
}

void mMainNickHelp(User * u)
{
	moduleNoticeLang(s_NickServ, u, phone_HELP_CMD);
}

/* EOF */
