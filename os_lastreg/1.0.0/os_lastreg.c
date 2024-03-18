#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: os_lastreg.c v1.0.1 26-08-2007 n00bie $"

/*
*******************************************************************************************
** Module	: os_lastreg.c
** Author	: n00bie (n00bie@rediffmail.com)
** Version	: 1.0.0
** Release	: 26th August, 2007
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
** Tested: 1.7.18, 1.7.19
** Changelog:
** v1.0.0 - First Public Release
** v1.0.1 - Added support for Global notice on logon.
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
**
** See the GNU General Public License for more details.
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

/********* Start of Configuration *********
** Do you want Global to notice them the
** last registered nick and channel when
** they connect.
**
** 1 = Yes
** 0 = No
*/

#define GLOBAL_NOTICE_ON_LOGON 1

/********* End of Configuration *********
** Don't touch anything beyond this
** unless you know what you are doing.
*/

#define LastRegNSDB "ns_lastreg.db"
#define LastRegCSDB "cs_lastreg.db"

int do_lastreg(User *u);
int do_nreg(int argc, char **argv);
int do_creg(int argc, char **argv);
int do_mglobal(int argc, char **argv);
int myOperServLastRegHelp(User *u); 
void myOperServHelp(User *u);
int AnopeInit(int argc, char **argv)
{
	Command *c;
	EvtHook *hook; 
	int status = 0;
	hook = createEventHook(EVENT_NICK_REGISTERED, do_nreg);
	status = moduleAddEventHook(hook);
	hook = createEventHook(EVENT_CHAN_REGISTERED, do_creg);
	status = moduleAddEventHook(hook);
	hook = createEventHook(EVENT_NEWNICK, do_mglobal);
	status = moduleAddEventHook(hook);
	c = createCommand("LASTREG", do_lastreg, is_services_oper, -1, -1, -1, -1, -1);
	moduleAddHelp(c, myOperServLastRegHelp);
	moduleSetOperHelp(myOperServHelp);
	status = moduleAddCommand(OPERSERV, c, MOD_TAIL);
	if (status != MOD_ERR_OK) {
		alog("os_lastreg: Something isn't init right.");
		return MOD_STOP;
	} else {
		alog("os_lastreg: \2/msg %s HELP LASTREG\2", s_OperServ);
		alog("os_lastreg: Successfully loaded module.");
	}
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
	anope_cmd_notice(s_OperServ, u->nick, "    LASTREG     See last registered nickname or channel");
}

int myOperServLastRegHelp(User *u)
{
	anope_cmd_notice(s_OperServ, u->nick, "Syntax: \2LASTREG \037nick\037|\037chan\037\2");
	anope_cmd_notice(s_OperServ, u->nick, " ");
	anope_cmd_notice(s_OperServ, u->nick, "This command if given with a \2nick\2 param will display");
	anope_cmd_notice(s_OperServ, u->nick, "the last registered \2nickname\2 and the registered time.");
	anope_cmd_notice(s_OperServ, u->nick, "Likewise if given with the \2channel\2 param will display");
	anope_cmd_notice(s_OperServ, u->nick, "the last registered \2channel\2, registered time and the");
	anope_cmd_notice(s_OperServ, u->nick, "founder of the channel if available.");
	anope_cmd_notice(s_OperServ, u->nick, " ");
	anope_cmd_notice(s_OperServ, u->nick, "Limited to \2Services Operator\2.");
	return MOD_CONT;
}

int do_nreg(int argc, char **argv)
{
	User *u;
	FILE *fp;
	fp = fopen(LastRegNSDB, "w");
	if(fp != NULL) {
		if ((u = finduser(argv[0]))) {
			fprintf(fp, "%s", u->nick);
		}
	} else {
		alog("os_lastreg: An error occured while opening %s", LastRegNSDB);
	}
	fclose(fp);
    return MOD_CONT;
}

int do_creg(int argc, char **argv)
{
	ChannelInfo *ci;
	FILE *fp;
	fp = fopen(LastRegCSDB, "w");
	if(fp != NULL) {
		if ((ci = cs_findchan(argv[0]))) {
			fprintf(fp, "%s", ci->name);
		}
	} else {
		alog("os_lastreg: An error occured while opening %s", LastRegCSDB);
	}
	fclose(fp);
    return MOD_CONT;
}

int do_lastreg(User *u) 
{
	char line[40];
	char *mbuf = moduleGetLastBuffer();
	char *cmd = myStrGetToken(mbuf, ' ', 0);
	char buf[BUFSIZE];
    struct tm *tm;
	FILE *fp;
	NickAlias *na;
	ChannelInfo *ci;
	if (!cmd) {
		notice_user(s_OperServ, u, "Syntax: \2LASTREG \037nick\037|\037chan\037\2");
	} else if (stricmp(cmd, "NICK") == 0) {
		fp = fopen(LastRegNSDB, "r");
		if (fp == NULL) {
			notice_user(s_OperServ, u, "Cannot retrieve the last registered nickname.");
		} else {
			while (!feof(fp)) {
				fgets(line, 40, fp);
				notice_user(s_OperServ, u, "The last nickname registered was: \2%s\2", line);
				if ((na = findnick(line))) {
					tm = localtime(&na->time_registered);
					strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
					notice_user(s_OperServ, u, "Time registered: %s", buf);
				} else {
					notice_user(s_OperServ, u, "Currently no information for \2%s\2 is available. It might have been dropped.", line);
				}
			}
		}
	} else if (stricmp(cmd, "CHAN") == 0) {
		fp = fopen(LastRegCSDB, "r");
		if (fp == NULL) {
			notice_user(s_OperServ, u, "Cannot retrieve the last channel registered.");
		} else {
			while (!feof(fp)) {
				fgets(line, 40, fp);
				notice_user(s_OperServ, u, "The last channel registered was: %s", line);
				if ((ci = cs_findchan(line))) {
					tm = localtime(&ci->time_registered);
					strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
					notice_user(s_OperServ, u, "Registered by: %s", ci->founder->display);
					notice_user(s_OperServ, u, "Time registered: %s", buf);
				} else {
					notice_user(s_OperServ, u, "Currently no information for '%s' is available. It might have been dropped.", line);
				}
			}
		}		
	} else {
		notice_user(s_OperServ, u, "Syntax: \2LASTREG \037nick\037|\037chan\037\2");
	}
	if (cmd)
		free(cmd);
	return MOD_CONT;
}

int do_mglobal(int argc, char **argv)
{
	User *u;
	char line[40];
	FILE *fp1, *fp2;
	if (GLOBAL_NOTICE_ON_LOGON >= 1) {
		if ((u = finduser(argv[0]))) {
			fp1 = fopen(LastRegNSDB, "r");
			if (fp1 != NULL) {
				while (!feof(fp1)) {
					fgets(line, 40, fp1);
					notice_user(s_GlobalNoticer, u, "The last nickname registered is: \2%s\2", line);
				}
			}
			fp2 = fopen(LastRegCSDB, "r");
			if (fp2 != NULL) {
				while (!feof(fp2)) {
					fgets(line, 40, fp2);
					notice_user(s_GlobalNoticer, u, "The last registered channel is: %s", line);
				}
			}
		}
	}
	return MOD_CONT;
}
/* EOF */
