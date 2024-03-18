#include "module.h"
#define AUTHOR "TheBingsta"
#define VERSION "$Id: cs_autoOP.c v2.0.0 17-01-2009 TheBingsta $"

/* ------------------------------------------------------------------------------------
 * Name		: cs_autoop.c
 * Author	: TheBingsta [ed@expertec.co.uk]
 * Version	: 1.0.1
 * Date		: 10th August, 2010
 * Update	: 
 * ------------------------------------------------------------------------------------
 * Description:
 *
 * Enable/disable automatic channel operating of users on a channel.
 * ------------------------------------------------------------------------------------
 * Providing Commands:
 *
 * /msg ChanServ HELP AUTOOP
 * /msg ChanServ AUTOOP #channel {REG|ALL|STATUS|OFF}
 *
 * REG		- Will autoop all registered (identified) users.
 * ALL		- Will autoop all users.
 * STATUS	- Display autoop status for the given channel.
 * OFF		- Disable autoop.
 * ------------------------------------------------------------------------------------
 * Changelog:
 *
 * v1.0.0	- First initial release
 * v2.0.0	- • Major code update.
 *			  • Added STATUS option on the command.
 *			  • Autovoice database will now be backed up :P
 *			  • Changed access level 5 or AOPs to SOP's (level 10) and above.
 *			  • The module now respects nickserv AUTOOP settings. If a user have autoop
 *			    settings turned off, he will not be voiced.
 *			  • Fixed ChanServ autooping all REG nickname whether its identified or not;
 *			    instead voice ONLY if they are identified to services. (Security issues)
 *			  • Fixed ChanServ oping all/reg users on the channel on services reboot.
 *			  • Fixed ChanServ oping a user even if the user have channel access.
 *			    That was pretty annoying!
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
 * Suggestions, comments, bugs - feel free :)
 *
 * This module have 1 optional configuration.
 *
 * Copy/paste below on services.conf

# CSAOPDBName [OPTIONAL]
# Module: cs_autoop
# Use the given filename as database to store the avoice data.
# If not given, the default "autovoice.db" will be used.
#
CSAOPDBName "cs_autoOP.db"

*/

#define DEFAULT_DB_NAME "autoOP.db"
#define LANG_NUM_STRINGS		9
#define CS_AUTOOP_SYNTAX		0
#define CS_AUTOOP_HELP		1
#define CS_AUTOOP_HELP_FULL	2
#define CS_AUTOOP_STATUS_REG	3
#define CS_AUTOOP_STATUS_ALL	4
#define CS_AUTOOP_STATUS_OFF	5
#define CS_AUTOOP_SET_REG	6
#define CS_AUTOOP_SET_ALL	7
#define CS_AUTOOP_SET_OFF	8

char *CSAOPDBName = NULL;
int do_aop(User *u);
int mSaveData(int argc, char **argv);
int mBackupData(int argc, char **argv);
int mEventJoin(int argc, char **argv);
int mLoadData(int argc, char **argv);
int myChanServAOPHelp(User *u);
void myChanServHelp(User *u);
void mAddLanguages(void);
void mLoadConfig(void);
void csav_save_db(void);
int AnopeInit(int argc, char **argv)
{
    Command *c = NULL;
    EvtHook *hook = NULL;
    int status = 0;
	if (!moduleMinVersion(1,7,21,1341)) {
		alog("cs_autoOP0: Your version of Anope isn't supported. This module require Anope-1.7.21 (1341) or later. Please upgrade to a newer release.");
		return MOD_STOP;
	}
    hook = createEventHook(EVENT_DB_SAVING, mSaveData);
    status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("cs_autoOP: Cannot hook to EVENT_DB_SAVING.");
		return MOD_STOP;
	}
	hook = createEventHook(EVENT_DB_BACKUP, mBackupData);
    status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("cs_autoOP: Cannot hook to EVENT_DB_BACKUP.");
		return MOD_STOP;
	}
    hook = createEventHook(EVENT_JOIN_CHANNEL, mEventJoin);
    status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("cs_autoOP: Cannot hook to EVENT_JOIN_CHANNEL.");
		return MOD_STOP;
	}
    c = createCommand("AUTOOP", do_aop, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, myChanServHelp);
    moduleSetChanHelp(myChanServHelp);
    moduleAddCommand(CHANSERV, c, MOD_HEAD);
	if (status != MOD_ERR_OK) {
		alog("cs_autoOP: Something isn't init right!");
		return MOD_STOP;
	} else {
		alog("cs_autoOP: New command: \2/msg %s HELP AUTOVOICE\2", s_ChanServ);
		alog("cs_autoOP: Module loaded successfully.");
	}
	mLoadConfig();
	mAddLanguages();
	moduleAddCallback("cs_autoOP: mLoadData", time(NULL) + 2, mLoadData, 0, NULL);
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    return MOD_CONT;
}

void AnopeFini(void)
{
    csav_save_db();
	if (CSAOPDBName)
		free(CSAOPDBName);
	alog("cs_autoOP: Module unloaded.");
}

int do_aop(User *u)
{
	char *buf = moduleGetLastBuffer();
    char *chan = myStrGetToken(buf, ' ', 0);
    char *option = myStrGetTokenRemainder(buf, ' ', 1);
	char *data;
    ChannelInfo *ci;
    if (!chan) {
		moduleNoticeLang(s_ChanServ, u, CS_AUTOOP_SYNTAX);
		return MOD_CONT;
    }
    if ((ci = cs_findchan(chan))) {
        if (!check_access(u, ci, CA_PROTECT)) {
            notice_lang(s_ChanServ, u, PERMISSION_DENIED);
			if (chan) free(chan);
			if (option) free(option);
            return MOD_CONT;
        }
		if (!option) {
			if ((data = moduleGetData(&ci->moduleData, "aop"))) {
				if (stricmp(data, "REG") == 0) {
					moduleNoticeLang(s_ChanServ, u, CS_AUTOOP_STATUS_REG, chan);
				} else if (stricmp(data, "ALL") == 0) {
					moduleNoticeLang(s_ChanServ, u, CS_AUTOOP_STATUS_ALL, chan);
				}
				free(data);
			} else {
				moduleNoticeLang(s_ChanServ, u, CS_AUTOOP_STATUS_OFF, chan);
			}
		} else {
			if (stricmp(option, "REG") == 0) {
				moduleAddData(&ci->moduleData, "aop", "REG");
				moduleNoticeLang(s_ChanServ, u, CS_AUTOOP_SET_REG, chan);
			} else if (stricmp(option, "ALL") == 0) {
				moduleAddData(&ci->moduleData, "aop", "ALL");
				moduleNoticeLang(s_ChanServ, u, CS_AUTOOP_SET_ALL, chan);
			} else if (stricmp(option, "OFF") == 0) {
				moduleDelData(&ci->moduleData, "aop");
				moduleNoticeLang(s_ChanServ, u, CS_AUTOOP_SET_OFF, chan);
			} else if (stricmp(option, "STATUS") == 0) {
				if ((data = moduleGetData(&ci->moduleData, "aop"))) {
					if (stricmp(data, "REG") == 0) {
						moduleNoticeLang(s_ChanServ, u, CS_AUTOOP_STATUS_REG, chan);
					} else if (stricmp(data, "ALL") == 0) {
						moduleNoticeLang(s_ChanServ, u, CS_AUTOOP_STATUS_ALL, chan);
					}
					free(data);
				} else {
					moduleNoticeLang(s_ChanServ, u, CS_AUTOOP_STATUS_OFF, chan);
				}
			} else {
				moduleNoticeLang(s_ChanServ, u, CS_AUTOOP_SYNTAX);
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
				if ((data = moduleGetData(&ci->moduleData, "aop"))) {
                    if (stricmp(data, "REG") == 0) {
						if ((na = findnick(u->nick))) {
							if (!check_access(na->u, ci, CA_AUTOOP) && nick_identified(u) && 
								!(na->nc->flags & NI_AUTOOP)) {
									anope_cmd_mode(whosends(ci), ci->name, "+o %s", na->nick);
									do_cmode(ci->c, u, CUS_OP);
							}
						}
					} else if (stricmp(data, "ALL") == 0) {
						if (!check_access(u, ci, CA_AUTOOP)) {
							anope_cmd_mode(whosends(ci), ci->name, "+o %s", u->nick);
							do_cmode(ci->c, u, CUS_OP);
						}
					}
					free(data);
				}
			}
		}
    }
    return MOD_CONT;
}

int mLoadData(int argc, char **argv)
{
    FILE *fp;
	char *filename;
    char *type = NULL;
    char *name = NULL;
    ChannelInfo *ci;
    char buffer[1024];
	if (CSAOPDBName) {
		filename = CSAOPDBName;
	} else {
		filename = DEFAULT_DB_NAME;
	}
	fp = fopen(filename, "r");
	if (!fp) {
		if (debug)
			alog("cs_autovoice: Unable to open database ('%s') for reading!", filename);
		return MOD_CONT;
	}
	while (fgets(buffer, 1024, fp)) {
		type = myStrGetToken(buffer, ' ', 0);
		name = myStrGetToken(buffer, ' ', 1);
		if (type) {
			if (name) {
				if ((ci = cs_findchan(name))) {
					if (stricmp(type, "REG") == 0) {
						moduleAddData(&ci->moduleData, "aop", "REG");
					} else if (stricmp(type, "ALL") == 0) {
						moduleAddData(&ci->moduleData, "aop", "ALL");
					} else {
						alog("cs_autoOP: Possible invalid entry [%s] in .db file, ignoring...", name);
					}
				}
				free(name);
			}
			free(type);
		}
	}
	fclose(fp);
	return MOD_CONT;
}

void csav_save_db(void)
{
    ChannelInfo *ci;
    int i = 0;
    char *data;
	char *filename;
    FILE *fp;

	if (CSAOPDBName) {
		filename = CSAOPDBName;
	} else {
		filename = DEFAULT_DB_NAME;
	}
	fp = fopen(filename, "w");
	if (!fp) {
		alog("cs_autoOP: Unable to open database ('%s') for writing!", filename);
		return;
	} else {
		for (i = 0; i < 256; i++) {
            for (ci = chanlists[i]; ci; ci = ci->next) {
                if ((data = moduleGetData(&ci->moduleData, "aop"))) {
                    if (stricmp(data, "REG") == 0) {
                        fprintf(fp, "%s %s \n", "REG", ci->name);
                    } else if (stricmp(data, "ALL") == 0) {
                        fprintf(fp, "%s %s \n", "ALL", ci->name);
                    } else {
                        alog("cs_autoOP: Invalid aop setting for channel [%s] not saving this record!", ci->name);
                    }
                    free(data);
                }
            }
        }        
    }
	fclose(fp);    
}

int mSaveData(int argc, char **argv)
{
	if ((argc >= 1) && (stricmp(argv[0], EVENT_START) == 0)) {
		csav_save_db();
	}
	return MOD_CONT;
}

int mBackupData(int argc, char **argv)
{
	if ((argc >= 1) && (stricmp(argv[0], EVENT_START) == 0)) {
		if (CSAOPDBName) {
			alog("Backing up database %s...", CSAOPDBName);
			ModuleDatabaseBackup(CSAOPDBName);
		} else {
			alog("Backing up database %s...", DEFAULT_DB_NAME);
			ModuleDatabaseBackup(DEFAULT_DB_NAME);
		}
	}
	return MOD_CONT;
}

void mLoadConfig(void)
{
	int i;
    char *tmp = NULL;
    Directive confvalues[][1] = {
		{{"CSAOPDBName", {{PARAM_STRING, PARAM_RELOAD, &tmp}}}},
	};
    for (i = 0; i < 1; i++)
		moduleGetConfigDirective(confvalues[i]);
    if (tmp) {
        if (CSAOPDBName)
			free(CSAOPDBName);
		CSAOPDBName = sstrdup(tmp);
    } else {
        CSAOPDBName = sstrdup(DEFAULT_DB_NAME);
    }
}

int myChanServAopHelp(User *u)
{
    moduleNoticeLang(s_ChanServ, u, CS_AUTOOP_SYNTAX);

}

void myChanServHelp(User *u)
{
    moduleNoticeLang(s_ChanServ, u, CS_AUTOOP_SYNTAX);
	    anope_cmd_notice(s_ChanServ, u->nick, " ");
	moduleNoticeLang(s_ChanServ, u, CS_AUTOOP_HELP_FULL);
    return MOD_CONT;
}

void add_cs_help_cmd(User *u)
{
moduleNoticeLang(s_ChanServ, u, CS_AUTOOP_HELP);
 }
void mAddLanguages(void)
{
	/* English (US) */
	char *langtable_en_us[] = {
		/* CS_AUTOOP_SYNTAX */
		"Syntax: \2AUTOOP #channel {REG|ALL|STATUS|OFF}\2",
		/* CS_AUTOOP_HELP */
		"    AUTOOP  Enable/disable auto-oping of users",
		/* CS_AUTOOP_HELP_FULL */
		"Enable or disable automatic channel oping of users\n"
		"upon joining a channel. The \2REG\2 option will\n"
		"voice only registered users whereas the \2ALL\2 option\n"
		"will voice all users. \2OFF\2 option will disable\n"
		"auto channel operator on the channel. \2STATUS\2 option will\n"
		"return autoOP status for the channel.\n"
		" \n"
		"By default, limited to SOPs or those with level 10\n"
		"and above on the channel access list.",
		/* CS_AUTOOP_STATUS_REG */
		"AutoOP option for %s is currently set to \2registered\2 users.",
		/* CS_AUTOOP_STATUS_ALL */
		"AutoOP option for %s is currently set to \2all\2 users.",
		/* CS_AUTOOP_STATUS_OFF */
		"AutoOP option for %s is currently \2OFF\2.",
		/* CS_AUTOOP_SET_REG */
		"AutoOP option for %s set to \2REGISTERED\2 users.",
		/* CS_AUTOOP_SET_ALL */
		"AutoOP option for %s set to \2ALL\2.",
		/* CS_AUTOOP_SET_OFF */
		"AutoOP option for %s is now \2OFF\2."
	};
	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}

/* EOF */
