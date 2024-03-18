#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: cs_autovoice.c v1.0.0 23-02-2008 n00bie $"

/* ------------------------------------------------------------------------------------
 * Name		: cs_autovoice.c
 * Author	: n00bie [n00bie@rediffmail.com]
 * Version	: 1.0.0
 * Date		: 23rd February, 2008
 * ------------------------------------------------------------------------------------
 * Description:
 *
 * Enable/disable automatic voicing of users on a channel.
 * ------------------------------------------------------------------------------------
 * Providing Commands:
 *
 * /msg ChanServ HELP AUTOVOICE
 * /msg ChanServ AUTOVOICE #channel {REG | ALL | OFF}
 *
 * REG - Will autovoice all registered users.
 * ALL - Will autovoice all users.
 * OFF - Disable autovoicing.
 * ------------------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * ------------------------------------------------------------------------------------
 * Module made possible using modifications made from Rob's (cs_avoice) module.
 * This module have 1 optional configuration.
 *
 * Copy/paste below on services.conf

# CSAvoiceDBName <database> [OPTIONAL]
# Module: cs_autovoice
#	Use the given filename as database to store the avoice data.
#	If not given, the default "autovoice.db" will be used.
#
CSAVoiceDBName "cs_autovoice.db"

*/

#define DEFAULT_DB_NAME "autovoice.db"
#define LANG_NUM_STRINGS		9
#define CS_AUTOVOICE_SYNTAX		0
#define CS_AUTOVOICE_HELP		1
#define CS_AUTOVOICE_HELP_FULL	2
#define CS_AUTOVOICE_STATUS_REG	3
#define CS_AUTOVOICE_STATUS_ALL	4
#define CS_AUTOVOICE_STATUS_OFF	5
#define CS_AUTOVOICE_SET_REG	6
#define CS_AUTOVOICE_SET_ALL	7
#define CS_AUTOVOICE_SET_OFF	8

char *CSAVoiceDBName;
int do_avoice(User *u);
int mSaveData(int argc, char **argv);
int mLoadData(void);
int mEventJoin(int argc, char **argv);
int mLoadConfig(int argc, char **argv);
int myChanServAvoiceHelp(User *u);
void myChanServHelp(User *u);
void mAddLanguages(void);
int AnopeInit(int argc, char **argv)
{
    Command *c = NULL;
    EvtHook *hook = NULL;
    int status = 0;
    CSAVoiceDBName = NULL;
	if (mLoadConfig(0, NULL)) {
		return MOD_STOP;
	}
	hook = createEventHook(EVENT_RELOAD, mLoadConfig);
    status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("cs_autovoice: Cannot hook to EVENT_RELOAD.");
		return MOD_STOP;
	}
    hook = createEventHook(EVENT_DB_SAVING, mSaveData);
    status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("cs_autovoice: Cannot hook to EVENT_DB_SAVING.");
		return MOD_STOP;
	}
    hook = createEventHook(EVENT_JOIN_CHANNEL, mEventJoin);
    status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("cs_autovoice: Cannot hook to EVENT_JOIN_CHANNEL.");
		return MOD_STOP;
	}
    c = createCommand("AUTOVOICE", do_avoice, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, myChanServAvoiceHelp);
    moduleSetChanHelp(myChanServHelp);
    moduleAddCommand(CHANSERV, c, MOD_HEAD);
	if (status != MOD_ERR_OK) {
		alog("cs_autovoice: Something isn't init right!");
		return MOD_STOP;
	} else {
		alog("cs_autovoice: New command: \2/msg %s HELP AUTOVOICE\2", s_ChanServ);
		alog("cs_autovoice: Module loaded successfully.");
	}
	mLoadData();
	mAddLanguages();
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    return MOD_CONT;
}

void AnopeFini(void)
{
    char *av[1];
    av[0] = sstrdup(EVENT_START);
    mSaveData(1, av);
    free(av[0]);
	if (CSAVoiceDBName) {
        free(CSAVoiceDBName);
	}
	alog("cs_autovoice: Module unloaded.");
}

int do_avoice(User *u)
{
	char *buf = moduleGetLastBuffer();
    char *chan = myStrGetToken(buf, ' ', 0);
    char *option = myStrGetTokenRemainder(buf, ' ', 1);
	char *data;
    ChannelInfo *ci;
    if (!chan) {
		moduleNoticeLang(s_ChanServ, u, CS_AUTOVOICE_SYNTAX);
		return MOD_CONT;
    }
    if ((ci = cs_findchan(chan))) {
        if (!check_access(u, ci, CA_BAN)) {
            notice_lang(s_ChanServ, u, ACCESS_DENIED);
			if (chan)
				free(chan);
			if (option)
				free(option);
            return MOD_CONT;
        }
		if (!option) {
			if ((data = moduleGetData(&ci->moduleData, "avoice"))) {
				if (stricmp(data, "REG") == 0) {
					moduleNoticeLang(s_ChanServ, u, CS_AUTOVOICE_STATUS_REG, chan);
				} else if (stricmp(data, "ALL") == 0) {
					moduleNoticeLang(s_ChanServ, u, CS_AUTOVOICE_STATUS_ALL, chan);
				}
				free(data);
			} else {
				moduleNoticeLang(s_ChanServ, u, CS_AUTOVOICE_STATUS_OFF, chan);
			}
		} else {
			if (stricmp(option, "REG") == 0) {
				moduleAddData(&ci->moduleData, "avoice", "REG");
				moduleNoticeLang(s_ChanServ, u, CS_AUTOVOICE_SET_REG, chan);
			} else if (stricmp(option, "ALL") == 0) {
				moduleAddData(&ci->moduleData, "avoice", "ALL");
				moduleNoticeLang(s_ChanServ, u, CS_AUTOVOICE_SET_ALL, chan);
			} else if (stricmp(option, "OFF") == 0) {
				moduleDelData(&ci->moduleData, "avoice");
				moduleNoticeLang(s_ChanServ, u, CS_AUTOVOICE_SET_OFF, chan);
			} else {
				moduleNoticeLang(s_ChanServ, u, CS_AUTOVOICE_SYNTAX);
			}
		}
	} else {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    }
	if (chan)
		free(chan);
	if (option)
		free(option);
    return MOD_CONT;
}

int mEventJoin(int argc, char **argv)
{
    ChannelInfo *ci;
    User *u;
	NickAlias *na;
	char *data = NULL;
	if (argc != 3) {
        return MOD_CONT;
	}
	if (!(u = finduser(argv[1]))) {
		return MOD_CONT;
	} 
    if (strcmp(argv[0], EVENT_STOP) == 0) {
        if (u) {
			if ((ci = cs_findchan(argv[2]))) {
				if ((data = moduleGetData(&ci->moduleData, "avoice"))) {
                    if (stricmp(data, "REG") == 0) {
						if (na = findnick(u->nick)) {
							anope_cmd_mode(whosends(ci), ci->name, "+v %s", u->nick);
							chan_set_user_status(ci->c, u, CUS_VOICE);
						}
					} else if (stricmp(data, "ALL") == 0) {
						anope_cmd_mode(whosends(ci), ci->name, "+v %s", u->nick);
						chan_set_user_status(ci->c, u, CUS_VOICE);
					}
					free(data);
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
    ChannelInfo *ci;
    char buffer[2000];
	if ((in = fopen(CSAVoiceDBName, "r")) == NULL) {
		ret = 1;
    } else {
		while (!feof(in)) {
            fgets(buffer, 1500, in);
			type = myStrGetToken(buffer, ' ', 0);
			name = myStrGetToken(buffer, ' ', 1);
            if (type) {
                if (name) {
                    if ((ci = cs_findchan(name))) {
                        if (stricmp(type, "REG") == 0) {
                            moduleAddData(&ci->moduleData, "avoice", "REG");
                        } else if (stricmp(type, "ALL") == 0) {
                            moduleAddData(&ci->moduleData, "avoice", "ALL");
                        } else {
                            alog("cs_autovoice: Possible invalid entry [%s] in .db file, ignoring...", name);
                        }
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
    ChannelInfo *ci;
    int i = 0;
    int ret = 0;
    char *data;
    FILE *out;

    if ((out = fopen(CSAVoiceDBName, "w")) == NULL) {
        alog("cs_autovoice: Can't open the database file!");
        ret = 1;
    } else {
        for (i = 0; i < 256; i++) {
            for (ci = chanlists[i]; ci; ci = ci->next) {
                if ((data = moduleGetData(&ci->moduleData, "avoice"))) {
                    if (stricmp(data, "REG") == 0) {
                        fprintf(out, "%s %s \n", "REG", ci->name);
                    } else if (stricmp(data, "ALL") == 0) {
                        fprintf(out, "%s %s \n", "ALL", ci->name);
                    } else {
                        alog("cs_autovoice: Invalid avoice setting for channel [%s] not saving this record!", ci->name);
                    }
                    free(data);
                }
            }
        }
        fclose(out);
    }
    moduleAddCallback("cs_autovoice: save me", time(NULL) + UpdateTimeout, mSaveData, 0, NULL);
    return ret;
}

int mLoadConfig(int argc, char **argv)
{
    char *tmp = NULL;
    Directive d[] = {
		{"CSAVoiceDBName", {{PARAM_STRING, PARAM_RELOAD, &tmp}}},
	};
    moduleGetConfigDirective(d);
	if (CSAVoiceDBName) {
		free(CSAVoiceDBName);
	}
    if (tmp) {
        CSAVoiceDBName = tmp;
    } else {
        CSAVoiceDBName = sstrdup(DEFAULT_DB_NAME);
        alog("cs_autovoice: \2CSAVoiceDBName\2 is not defined in Services configuration file, using default database: '%s'", CSAVoiceDBName);
    }
    if (!CSAVoiceDBName) {
        alog("cs_autovoice: FATAL: Can't read required configuration directives!");
        return MOD_STOP;
    }
    return MOD_CONT;
}

int myChanServAvoiceHelp(User *u)
{
    moduleNoticeLang(s_ChanServ, u, CS_AUTOVOICE_SYNTAX);
    anope_cmd_notice(s_ChanServ, u->nick, " ");
	moduleNoticeLang(s_ChanServ, u, CS_AUTOVOICE_HELP_FULL);
    return MOD_CONT;
}

void myChanServHelp(User *u)
{
    moduleNoticeLang(s_ChanServ, u, CS_AUTOVOICE_HELP);
}

void mAddLanguages(void)
{
	/* English (US) */
	char *langtable_en_us[] = {
		/* CS_AUTOVOICE_SYNTAX */
		"Syntax: \2AUTOVOICE #channel {REG|ALL|OFF}\2",
		/* CS_AUTOVOICE_HELP */
		"    AUTOVOICE  Enable/disable auto-voicing of users",
		/* CS_AUTOVOICE_HELP_FULL */
		"Enable or disable automatic voicing of users\n"
		"upon joining a channel. The \2REG\2 option will\n"
		"voice only registered users whereas the \2ALL\2 option\n"
		"will voice all users. \2OFF\2 option will disable\n"
		"auto voicing on the channel.",
		/* CS_AUTOVOICE_STATUS_REG */
		"Autovoice option for %s is currently set to \2registered\2 users.",
		/* CS_AUTOVOICE_STATUS_ALL */
		"Autovoice option for %s is currently set to \2all\2 users.",
		/* CS_AUTOVOICE_STATUS_OFF */
		"Autovoice option for %s is currently \2OFF\2.",
		/* CS_AUTOVOICE_SET_REG */
		"Autovoice option for %s set to \2REG\2.",
		/* CS_AUTOVOICE_SET_ALL */
		"Autovoice option for %s set to \2ALL\2.",
		/* CS_AUTOVOICE_SET_OFF */
		"Autovoice option for %s is now \2OFF\2."
	};
	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}

/* EOF */
