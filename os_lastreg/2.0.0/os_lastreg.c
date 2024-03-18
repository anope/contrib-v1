#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: os_lastreg.c v2.0.0 04-02-2008 n00bie $"

/*
*******************************************************************************************
** Module	: os_lastreg.c
** Author	: n00bie (n00bie@rediffmail.com)
** Version	: 2.0.0
** Release	: 26th August, 2007
** Update	: 4th Feb, 2008
*******************************************************************************************
** Description:
**
** This module will retrieve the last nickname and channel registered. This command
** if given with a 'nick' param will display the last registered nickname and the
** registered time. Likewise if given with the 'channel' param will display the last
** registered channel, registered time and the founder of the channel if available.
*******************************************************************************************
** Providing Command:
**
** /msg OperServ HELP LASTREG
** /msg OperServ LASTREG nick|chan
**
** Tested: 1.7.21
*******************************************************************************************
** Changelog:
** v1.0.0 - First Public Release
** v1.0.1 - Added support for Global notice on logon.
** v1.0.2 - Now closed the db when not used :s
** v2.0.0 - • Fixed a crash bug on empty database
**          • Added configuration directive
**			• Added multi-language support
*******************************************************************************************
** This program is free software; you can redistribute it and/or modify it under the
** terms of the GNU General Public License as published by the Free Software
** Foundation; either version 1, or (at your option) any later version. 
**
** This program is distributed in the hope that it will be useful, but WITHOUT ANY
** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS for
** a PARTICULAR PURPOSE. THIS MODULE IS DISTRIBUTED 'AS IS'. NO WARRANTY OF ANY
** KIND IS EXPRESSED OR IMPLIED. YOU USE AT YOUR OWN RISK. THE MODULE AUTHOR WILL 
** NOT BE RESPONSIBLE FOR DATA LOSS, DAMAGES, LOSS OF PROFITS OR ANY OTHER KIND OF 
** LOSS WHILE USING OR MISUSING THIS MODULE. 
*******************************************************************************************
** NOTE:
**
** This module is my first module using a seperate database, so don't think that this
** module is intelligent enough to retrieve the last registered nickname/channel at the 
** time of loading this module. It will ONLY retrieve if a new nickname/channel is
** registered after loading or using this module ;-)
********************************************************************************************
** Module suggested by ice
** This module have 1 configurable option.
*/

/*
Copy/paste below on services.conf

# LastRegLogon [OPTIONAL]
# Module: os_lastreg
#
# Define this if you want Global to display the
# last registered nickname and channel when
# a users connect.

LastRegLogon

# End of Configuration
*/

#define LANG_NUM_STRINGS			13
#define LASTREG_HELP				0
#define LASTREG_SYNTAX				1
#define LASTREG_HELP_CMD			2
#define LASTREG_ERROR_NICK			3
#define LASTREG_ERROR_CHAN			4
#define LASTREG_NICKNAME			5
#define LASTREG_NICKNAME_REGTIME	6
#define LASTREG_CHANNEL				7
#define LASTREG_CHANNEL_FOUNDER		8
#define LASTREG_CHANNEL_REGTIME		9
#define LASTREG_NOINFO				10
#define LASTREG_GLOBAL_NICK			11
#define LASTREG_GLOBAL_CHAN			12
#define LastRegNSDB "ns_lastreg.db"
#define LastRegCSDB "cs_lastreg.db"
int LastRegLogon;
int do_lastreg(User *u);
int do_nreg(int argc, char **argv);
int do_creg(int argc, char **argv);
int do_mglobal(int argc, char **argv);
int do_reload(int argc, char **argv);
int mLoadConfig(void);
int myOperServLastRegHelp(User *u); 
void myOperServHelp(User *u);
void mAddLanguages(void);
int AnopeInit(int argc, char **argv)
{
	Command *c;
	EvtHook *hook; 
	int status = 0;
	hook = createEventHook(EVENT_NICK_REGISTERED, do_nreg);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("os_lastreg: Something isn't init right.");
		return MOD_STOP;
	} else {
		alog("os_lastreg: Successfully hooked to EVENT_NICK_REGISTERED");
	}
	hook = createEventHook(EVENT_CHAN_REGISTERED, do_creg);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("os_lastreg: Something isn't init right.");
		return MOD_STOP;
	} else {
		alog("os_lastreg: Successfully hooked to EVENT_CHAN_REGISTERED");
	}
	hook = createEventHook(EVENT_NEWNICK, do_mglobal);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("os_lastreg: Something isn't init right.");
		return MOD_STOP;
	} else {
		alog("os_lastreg: Successfully hooked to EVENT_NEWNICK");
	}
	hook = createEventHook(EVENT_RELOAD, do_reload);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("os_lastreg: Something isn't init right.");
		return MOD_STOP;
	} else {
		alog("os_lastreg: Successfully hooked to EVENT_RELOAD");
	}
	c = createCommand("LASTREG", do_lastreg, is_services_oper, -1, -1, -1, -1, -1);
	moduleAddOperHelp(c, myOperServLastRegHelp);
	moduleAddAdminHelp(c, myOperServLastRegHelp);
	moduleAddRootHelp(c, myOperServLastRegHelp);
	moduleSetOperHelp(myOperServHelp);
	status = moduleAddCommand(OPERSERV, c, MOD_TAIL);
	if (status != MOD_ERR_OK) {
		alog("os_lastreg: Something isn't init right.");
		return MOD_STOP;
	} else {
		alog("os_lastreg: \2/msg %s HELP LASTREG\2", s_OperServ);
		alog("os_lastreg: Successfully loaded module.");
	}
	mLoadConfig();
	mAddLanguages();
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("os_lastreg: Module unloaded.");
}

void myOperServHelp(User *u)
{
	if (is_services_oper(u)) {
		moduleNoticeLang(s_OperServ, u, LASTREG_HELP);
	}
}

int myOperServLastRegHelp(User *u)
{
	moduleNoticeLang(s_OperServ, u, LASTREG_SYNTAX);
	anope_cmd_notice(s_OperServ, u->nick, " ");
	moduleNoticeLang(s_OperServ, u, LASTREG_HELP_CMD);
	return MOD_CONT;
}

int do_nreg(int argc, char **argv)
{
	User *u;
	int ret = 0;
	FILE *fp;
	if ((fp = fopen(LastRegNSDB, "w")) == NULL) {
		alog("os_lastreg: ERROR: can not open the database file!");
		if (WallOSGlobal) {
			anope_cmd_global(s_OperServ, "os_lastreg: ERROR: can not open the database file \2LastRegNSDB\2!");
		}
		ret = 1;
	} else {
		if ((u = finduser(argv[0]))) {
			fprintf(fp, "%s", u->nick);
		}
		fclose(fp);
	}
    return ret;
}

int do_creg(int argc, char **argv)
{
	int ret = 0;
	ChannelInfo *ci;
	FILE *fp;
	if ((fp = fopen(LastRegCSDB, "w")) == NULL) {
		alog("os_lastreg: ERROR: can not open the database file!");
		if (WallOSGlobal) {
			anope_cmd_global(s_OperServ, "os_lastreg: ERROR: can not open the database file \2LastRegCSDB\2!");
		}
		ret = 1;
	} else {
		if ((ci = cs_findchan(argv[0]))) {
			fprintf(fp, "%s", ci->name);
		}
		fclose(fp);
	}
    return ret;
}

int do_lastreg(User *u) 
{
	int ret = 0;
	char line[40];
	char *text = moduleGetLastBuffer();
	char *cmd = myStrGetToken(text, ' ', 0);
	char buf[BUFSIZE];
    struct tm *tm;
	FILE *fp;
	NickAlias *na;
	ChannelInfo *ci;
	if (!cmd) {
		moduleNoticeLang(s_OperServ, u, LASTREG_SYNTAX);
	} else if (stricmp(cmd, "NICK") == 0) {
		if ((fp = fopen(LastRegNSDB, "r")) == NULL) {
			moduleNoticeLang(s_OperServ, u, LASTREG_ERROR_NICK);
			ret = 1;
		} else {
			while (!feof(fp)) {
				fgets(line, 40, fp);
				moduleNoticeLang(s_OperServ, u, LASTREG_NICKNAME, line);
				if ((na = findnick(line))) {
					tm = localtime(&na->time_registered);
					strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
					moduleNoticeLang(s_OperServ, u, LASTREG_NICKNAME_REGTIME, buf);
				} else {
					moduleNoticeLang(s_OperServ, u, LASTREG_NOINFO, line);
				}
			}
			fclose(fp);
		}
	} else if (stricmp(cmd, "CHAN") == 0) {
		if ((fp = fopen(LastRegCSDB, "r")) == NULL) {
			moduleNoticeLang(s_OperServ, u, LASTREG_ERROR_CHAN);
			ret = 1;
		} else {
			while (!feof(fp)) {
				fgets(line, 40, fp);
				moduleNoticeLang(s_OperServ, u, LASTREG_CHANNEL, line);
				if ((ci = cs_findchan(line))) {
					tm = localtime(&ci->time_registered);
					strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
					moduleNoticeLang(s_OperServ, u, LASTREG_CHANNEL_FOUNDER, ci->founder->display);
					moduleNoticeLang(s_OperServ, u, LASTREG_CHANNEL_REGTIME, buf);
				} else {
					moduleNoticeLang(s_OperServ, u, LASTREG_NOINFO, line);
				}
			}
			fclose(fp);
		}
	} else {
		moduleNoticeLang(s_OperServ, u, LASTREG_SYNTAX);
	}
	if (cmd)
		free(cmd);
	return ret;
}

int do_mglobal(int argc, char **argv)
{
	int ret = 0;
	User *u;
	char line[40];
	FILE *fp1, *fp2;
	if (LastRegLogon) {
		if ((u = finduser(argv[0]))) {
			if ((fp1 = fopen(LastRegNSDB, "r")) == NULL) {
				ret = 1;
			} else {
				while (!feof(fp1)) {
					fgets(line, 40, fp1);
					moduleNoticeLang(s_GlobalNoticer, u, LASTREG_GLOBAL_NICK, line);
				}
				fclose(fp1);
			}
			if ((fp2 = fopen(LastRegCSDB, "r")) == NULL) {
				ret = 1;
			} else {
				while (!feof(fp2)) {
					fgets(line, 40, fp2);
					moduleNoticeLang(s_GlobalNoticer, u, LASTREG_GLOBAL_CHAN, line);
				}
				fclose(fp2);
			}
		}
	}
	return ret;
}

int mLoadConfig(void)
{
    Directive directivas[] = {
        {"LastRegLogon", {{PARAM_SET, PARAM_RELOAD, &LastRegLogon}}},
    };
    Directive *d = &directivas[0];
    moduleGetConfigDirective(d);
    return MOD_CONT;
}

int do_reload(int argc, char **argv)
{
	mLoadConfig();
	return MOD_CONT;
}

void mAddLanguages(void)
{
    char *langtable_en_us[] = {
		/* LASTREG_HELP */
		"    LASTREG     See last registered nickname or channel",
		/* LASTREG_SYNTAX */
		"Syntax: \2LASTREG \037nick\037|\037chan\037\2",
		/* LASTREG_HELP_CMD */
		"This command if given with a \2nick\2 param will display\n"
		"the last registered \2nickname\2 and the registered time.\n"
		"Likewise if given with the \2channel\2 param will display\n"
		"the last registered \2channel\2, registered time and the\n"
		"founder of the channel if available.",
		/* LASTREG_ERROR_NICK */
		"Cannot retrieve the last registered nickname.",
		/* LASTREG_ERROR_CHAN */
		"Cannot retrieve the last registered channel.",
		/* LASTREG_NICKNAME */
		"The last nickname registered was: \2%s\2",
		/* LASTREG_NICKNAME_REGTIME */
		"Time registered: %s",
		/* LASTREG_CHANNEL */
		"The last channel registered was: %s",
		/* LASTREG_CHANNEL_FOUNDER */
		"Registered by: %s",
		/* LASTREG_CHANNEL_REGTIME */
		"Time registered: %s",
		/* LASTREG_NOINFO */
		"Currently no information for \2%s\2 is available. Perhaps it might have been dropped.",
		/* LASTREG_GLOBAL_NICK */
		"The last nickname registered is: \2%s\2",
		/* LASTREG_GLOBAL_CHAN */
		"The last registered channel is: %s"
	};
	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}

/* EOF */
