#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: massnotice.c v2.0.0 03-02-2008 n00bie $"

/**********************************************************************
 * Name		: massnotice.c
 * Author	: n00bie [n00bie@rediffmail.com]
 * Version	: v2.0.0
 * Date		: 29th Nov, 2007
 * Update	: 3rd Feb, 2008
 **********************************************************************
 * Description:
 *
 * This module will make services clients (ChanServ, NickServ,
 * BotServ, HostServ, HelpServ, OperServ, MemoServ) sends a mass notice
 * to all users on the network.
 **********************************************************************
 * Providing Commands:
 *
 * /msg ChanServ MNOTICE [message]
 * /msg NickServ MNOTICE [message]
 * /msg BotServ  MNOTICE [message]
 * /msg HostServ MNOTICE [message]
 * /msg HelpServ MNOTICE [message]
 * /msg OperServ MNOTICE [message]
 * /msg MemoServ MNOTICE [message]
 * AND
 * /msg ChanServ HELP MNOTICE 
 * etc..
 *
 * NOTE: Commands is Limited to Services Admins.
 ***********************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 ***********************************************************************
 * This module have no configurable option.
 ***********************************************************************
 * Changelog:
 * v1.0.0	- First Public Release.
 * v2.0.0	- 
 *				• Minor Code Update
 *				• Added Multi-language support
 *				• Added MemoServ
 ***********************************************************************
 */

#define LANG_NUM_STRINGS	10
#define MNOTICE_BS_HELP		0
#define MNOTICE_CS_HELP		1
#define MNOTICE_HE_HELP		2
#define	MNOTICE_HS_HELP		3
#define	MNOTICE_NS_HELP		4
#define	MNOTICE_OS_HELP		5
#define	MNOTICE_MS_HELP		6
#define MNOTICE_SYNTAX		7
#define MNOTICE_HELP		8
#define MNOTICE_SENT		9

int cs_mnotice(User *u);
int ns_mnotice(User *u);
int hs_mnotice(User *u);
int bs_mnotice(User *u);
int os_mnotice(User *u);
int he_mnotice(User *u);
int ms_mnotice(User *u);
int cs_help_mnotice(User *u);
int ns_help_mnotice(User *u);
int hs_help_mnotice(User *u);
int bs_help_mnotice(User *u);
int os_help_mnotice(User *u);
int he_help_mnotice(User *u);
int ms_help_mnotice(User *u);
void myChanServHelp(User *u);
void myNickServHelp(User *u);
void myHostServHelp(User *u);
void myBotServHelp(User *u);
void myOperServHelp(User *u);
void myHelpServHelp(User *u);
void myMemoServHelp(User *u);
void doAddLanguages(void);
int AnopeInit(int argc, char **argv)
{
	Command *c;
	int status = 0;
	c = createCommand("MNOTICE", cs_mnotice, is_services_admin, -1, -1, -1, -1, -1);
	moduleAddAdminHelp(c, cs_help_mnotice);
	moduleAddRootHelp(c, cs_help_mnotice);
	alog("%s: Added command 'MNOTICE'", s_ChanServ);
	status = moduleAddCommand(CHANSERV, c, MOD_HEAD);
	moduleSetChanHelp(myChanServHelp);
	c = createCommand("MNOTICE", ns_mnotice, is_services_admin, -1, -1, -1, -1, -1);
	moduleAddAdminHelp(c, ns_help_mnotice);
	moduleAddRootHelp(c, ns_help_mnotice);
	alog("%s: Added command 'MNOTICE'", s_NickServ);
	status = moduleAddCommand(NICKSERV, c, MOD_HEAD);
	moduleSetNickHelp(myNickServHelp);
	c = createCommand("MNOTICE", hs_mnotice, is_services_admin, -1, -1, -1, -1, -1);
	moduleAddAdminHelp(c, hs_help_mnotice);
	moduleAddRootHelp(c, hs_help_mnotice);
	alog("%s: Added command 'MNOTICE'", s_HostServ);
	status = moduleAddCommand(HOSTSERV, c, MOD_HEAD);
	moduleSetHostHelp(myHostServHelp);
	c = createCommand("MNOTICE", bs_mnotice, is_services_admin, -1, -1, -1, -1, -1);
	moduleAddAdminHelp(c, bs_help_mnotice);
	moduleAddRootHelp(c, bs_help_mnotice);
	alog("%s: Added command 'MNOTICE'", s_BotServ);
	status = moduleAddCommand(BOTSERV, c, MOD_HEAD);
	moduleSetBotHelp(myBotServHelp);
	c = createCommand("MNOTICE", os_mnotice, is_services_admin, -1, -1, -1, -1, -1);
	moduleAddAdminHelp(c, os_help_mnotice);
	moduleAddRootHelp(c, os_help_mnotice);
	alog("%s: Added command 'MNOTICE'", s_OperServ);
	status = moduleAddCommand(OPERSERV, c, MOD_HEAD);
	moduleSetOperHelp(myOperServHelp);
	c = createCommand("MNOTICE", he_mnotice, is_services_admin, -1, -1, -1, -1, -1);
	moduleAddAdminHelp(c, he_help_mnotice);
	moduleAddRootHelp(c, he_help_mnotice);
	alog("%s: Added command 'MNOTICE'", s_HelpServ);
	status = moduleAddCommand(HELPSERV, c, MOD_HEAD);
	moduleSetHelpHelp(myHelpServHelp);
	c = createCommand("MNOTICE", ms_mnotice, is_services_admin, -1, -1, -1, -1, -1);
	moduleAddAdminHelp(c, ms_help_mnotice);
	moduleAddRootHelp(c, ms_help_mnotice);
	alog("%s: Added command 'MNOTICE'", s_MemoServ);
	status = moduleAddCommand(MEMOSERV, c, MOD_TAIL);
	moduleSetMemoHelp(myMemoServHelp);
	doAddLanguages();
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	alog("massnotice%s: Successfully loaded module.", MODULE_EXT);
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("massnotice%s: Module unloaded.", MODULE_EXT);
}

void myChanServHelp(User *u)
{
	if (is_services_admin(u)) {
		moduleNoticeLang(s_ChanServ, u, MNOTICE_CS_HELP);
	}
}

void myNickServHelp(User *u)
{
	if (is_services_admin(u)) {
		moduleNoticeLang(s_NickServ, u, MNOTICE_NS_HELP);
	}
}

void myHostServHelp(User *u)
{
	if (is_services_admin(u)) {
		moduleNoticeLang(s_HostServ, u, MNOTICE_HS_HELP);
	}
}

void myBotServHelp(User *u)
{
	if (is_services_admin(u)) {
		moduleNoticeLang(s_BotServ, u, MNOTICE_BS_HELP);
	}
}

void myOperServHelp(User *u)
{
	if (is_services_admin(u)) {
		moduleNoticeLang(s_OperServ, u, MNOTICE_OS_HELP);
	}
}

void myHelpServHelp(User *u)
{
	if (is_services_admin(u)) {
		moduleNoticeLang(s_HelpServ, u, MNOTICE_HE_HELP, s_HelpServ);
	}
}

void myMemoServHelp(User *u)
{
	if (is_services_admin(u)) {
		moduleNoticeLang(s_MemoServ, u, MNOTICE_MS_HELP);
	}
}

int cs_help_mnotice(User *u)
{
	moduleNoticeLang(s_ChanServ, u, MNOTICE_SYNTAX);
	moduleNoticeLang(s_ChanServ, u, MNOTICE_HELP, s_ChanServ);
	return MOD_CONT;
}

int ns_help_mnotice(User *u)
{
	moduleNoticeLang(s_NickServ, u, MNOTICE_SYNTAX);
	moduleNoticeLang(s_NickServ, u, MNOTICE_HELP, s_NickServ);
	return MOD_CONT;
}

int bs_help_mnotice(User *u)
{
	moduleNoticeLang(s_BotServ, u, MNOTICE_SYNTAX);
	moduleNoticeLang(s_BotServ, u, MNOTICE_HELP, s_BotServ);
	return MOD_CONT;
}

int os_help_mnotice(User *u)
{
	moduleNoticeLang(s_OperServ, u, MNOTICE_SYNTAX);
	moduleNoticeLang(s_OperServ, u, MNOTICE_HELP, s_OperServ);
	return MOD_CONT;
}

int he_help_mnotice(User *u)
{
	moduleNoticeLang(s_HelpServ, u, MNOTICE_SYNTAX);
	moduleNoticeLang(s_HelpServ, u, MNOTICE_HELP, s_HelpServ);
	return MOD_CONT;
}

int hs_help_mnotice(User *u)
{
	moduleNoticeLang(s_HostServ, u, MNOTICE_SYNTAX);
	moduleNoticeLang(s_HostServ, u, MNOTICE_HELP, s_HostServ);
	return MOD_CONT;
}

int ms_help_mnotice(User *u)
{
	moduleNoticeLang(s_MemoServ, u, MNOTICE_SYNTAX);
	moduleNoticeLang(s_MemoServ, u, MNOTICE_HELP, s_MemoServ);
	return MOD_CONT;
}

int cs_mnotice(User *u)
{
	int i, count = 0;
	User *u2;
	char *buf = moduleGetLastBuffer();
	char *text = myStrGetTokenRemainder(buf, ' ', 0);
	if (is_services_admin(u)) {
		if (!text) {
			moduleNoticeLang(s_ChanServ, u, MNOTICE_SYNTAX);
		} else {
			for (i = 0; i < 1024; i++) {
				for (u2 = userlist[i]; u2; u2 = u2->next) {
					anope_cmd_notice(s_ChanServ, u2->nick, "%s", text);
					count++;
				}
			}
			moduleNoticeLang(s_ChanServ, u, MNOTICE_SENT, count);
		}
	}
	if (text)
		free(text);
	return MOD_CONT;
}

int ns_mnotice(User *u)
{
	int i, count = 0;
	User *u2;
	char *buf = moduleGetLastBuffer();
	char *text = myStrGetTokenRemainder(buf, ' ', 0);
	if (is_services_admin(u)) {
		if (!text) {
			moduleNoticeLang(s_NickServ, u, MNOTICE_SYNTAX);
		} else {
			for (i = 0; i < 1024; i++) {
				for (u2 = userlist[i]; u2; u2 = u2->next) {
					anope_cmd_notice(s_NickServ, u2->nick, "%s", text);
					count++;
				}
			}
			moduleNoticeLang(s_NickServ, u, MNOTICE_SENT, count);
		}
	}
	if (text)
		free(text);
	return MOD_CONT;
}

int bs_mnotice(User *u)
{
	int i, count = 0;
	User *u2;
	char *buf = moduleGetLastBuffer();
	char *text = myStrGetTokenRemainder(buf, ' ', 0);
	if (is_services_admin(u)) {
		if (!text) {
			moduleNoticeLang(s_BotServ, u, MNOTICE_SYNTAX);
		} else {
			for (i = 0; i < 1024; i++) {
				for (u2 = userlist[i]; u2; u2 = u2->next) {
					anope_cmd_notice(s_BotServ, u2->nick, "%s", text);
					count++;
				}
			}
			moduleNoticeLang(s_BotServ, u, MNOTICE_SENT, count);
		}
	}
	if (text)
		free(text);
	return MOD_CONT;
}

int os_mnotice(User *u)
{
	int i, count = 0;
	User *u2;
	char *buf = moduleGetLastBuffer();
	char *text = myStrGetTokenRemainder(buf, ' ', 0);
	if (is_services_admin(u)) {
		if (!text) {
			moduleNoticeLang(s_OperServ, u, MNOTICE_SYNTAX);
		} else {
			for (i = 0; i < 1024; i++) {
				for (u2 = userlist[i]; u2; u2 = u2->next) {
					anope_cmd_notice(s_OperServ, u2->nick, "%s", text);
					count++;
				}
			}
			moduleNoticeLang(s_OperServ, u, MNOTICE_SENT, count);
		}
	}
	if (text)
		free(text);
	return MOD_CONT;
}

int he_mnotice(User *u)
{
	int i, count = 0;
	User *u2;
	char *buf = moduleGetLastBuffer();
	char *text = myStrGetTokenRemainder(buf, ' ', 0);
	if (is_services_admin(u)) {
		if (!text) {
			moduleNoticeLang(s_HelpServ, u, MNOTICE_SYNTAX);
		} else {
			for (i = 0; i < 1024; i++) {
				for (u2 = userlist[i]; u2; u2 = u2->next) {
					anope_cmd_notice(s_HelpServ, u2->nick, "%s", text);
					count++;
				}
			}
			moduleNoticeLang(s_HelpServ, u, MNOTICE_SENT, count);
		}
	}
	if (text)
		free(text);
	return MOD_CONT;
}

int hs_mnotice(User *u)
{
	int i, count = 0;
	User *u2;
	char *buf = moduleGetLastBuffer();
	char *text = myStrGetTokenRemainder(buf, ' ', 0);
	if (is_services_admin(u)) {
		if (!text) {
			moduleNoticeLang(s_HostServ, u, MNOTICE_SYNTAX);
		} else {
			for (i = 0; i < 1024; i++) {
				for (u2 = userlist[i]; u2; u2 = u2->next) {
					anope_cmd_notice(s_HostServ, u2->nick, "%s", text);
					count++;
				}
			}
			moduleNoticeLang(s_HostServ, u, MNOTICE_SENT, count);
		}
	}
	if (text)
		free(text);
	return MOD_CONT;
}

int ms_mnotice(User *u)
{
	int i, count = 0;
	User *u2;
	char *buf = moduleGetLastBuffer();
	char *text = myStrGetTokenRemainder(buf, ' ', 0);
	if (is_services_admin(u)) {
		if (!text) {
			moduleNoticeLang(s_MemoServ, u, MNOTICE_SYNTAX);
		} else {
			for (i = 0; i < 1024; i++) {
				for (u2 = userlist[i]; u2; u2 = u2->next) {
					anope_cmd_notice(s_MemoServ, u2->nick, "%s", text);
					count++;
				}
			}
			moduleNoticeLang(s_MemoServ, u, MNOTICE_SENT, count);
		}
	}
	if (text)
		free(text);
	return MOD_CONT;
}

void doAddLanguages(void)
{
    char *langtable_en_us[] = {
		/*
		Reasons why i used different help headers
		is that i like the text format to align nicely.
		One single space or extra spaces can make the looks
		very ugly ;b
		*/

		/* MNOTICE_BS_HELP */
		"    MNOTICE        Send mass notice to all users",
		/* MNOTICE_CS_HELP */
		"    MNOTICE    Send mass notice to all users",
		/* MNOTICE_HE_HELP */
		"\2/msg %s MNOTICE\2\n"
		"     for sending notice to all users on the network",
		/* MNOTICE_HS_HELP */
		"    MNOTICE     Send mass notice to all users",
		/* MNOTICE_NS_HELP */
		"    MNOTICE    Send mass notice to all users",
		/* MNOTICE_OS_HELP */
		"    MNOTICE     Send mass notice to all users",
		/* MNOTICE_MS_HELP */
		"    MNOTICE  Send mass notice to all users",
		/* MNOTICE_SYNTAX */
		"Syntax: \2MNOTICE \037message\037\2\n"
		" ",
		/* MNOTICE_HELP */
		"This command will send a mass notice\n"
		"to all users on the network using \2%s\2.",
		/* MNOTICE_SENT */
		"Successfully sent mass notice to \2%d\2 users."
	};
	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}

/* EOF */
