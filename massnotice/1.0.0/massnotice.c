#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: massnotice.c v1.0.0 29/11/2006 n00bie $"

/**********************************************************************
 * Name		: massnotice.c
 * Author	: n00bie
 * Version	: v1.0.0
 * Date		: 29/11/2006
 **********************************************************************
 * Description:
 *
 * This module will make services clients (ChanServ, NickServ,
 * BotServ, HostServ, HelpServ, OperServ) sends a mass notice
 * to all users on the network.
 **********************************************************************
 * Providing Commands:
 *
 * /ChanServ MNOTICE [message]
 * /NickServ MNOTICE [message]
 * /BotServ  MNOTICE [message]
 * /HostServ MNOTICE [message]
 * /HelpServ MNOTICE [message]
 * /OperServ MNOTICE [message]
 * AND
 * /ChanServ HELP MNOTICE 
 * etc..
 *
 * NOTE: Commands is Limited to Services Admins.
 ***********************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 ***********************************************************************
 * This module have no configurable option.
 **********************************************************************/

int cs_mnotice(User *u);
int ns_mnotice(User *u);
int hs_mnotice(User *u);
int bs_mnotice(User *u);
int os_mnotice(User *u);
int he_mnotice(User *u);
int cs_help_mnotice(User *u);
int ns_help_mnotice(User *u);
int hs_help_mnotice(User *u);
int bs_help_mnotice(User *u);
int os_help_mnotice(User *u);
int he_help_mnotice(User *u);
void myChanServHelp(User *u);
void myNickServHelp(User *u);
void myHostServHelp(User *u);
void myBotServHelp(User *u);
void myOperServHelp(User *u);
void myHelpServHelp(User *u);

int AnopeInit(int argc, char **argv)
{
	Command *c;
	c = createCommand("MNOTICE", cs_mnotice, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, cs_help_mnotice);
	alog("%s: Added command 'MNOTICE'", s_ChanServ);
	moduleAddCommand(CHANSERV, c, MOD_HEAD);
	moduleSetChanHelp(myChanServHelp);
	c = createCommand("MNOTICE", ns_mnotice, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, ns_help_mnotice);
	alog("%s: Added command 'MNOTICE'", s_NickServ);
	moduleAddCommand(NICKSERV, c, MOD_HEAD);
	moduleSetNickHelp(myNickServHelp);
	c = createCommand("MNOTICE", hs_mnotice, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, hs_help_mnotice);
	alog("%s: Added command 'MNOTICE'", s_HostServ);
	moduleAddCommand(HOSTSERV, c, MOD_HEAD);
	moduleSetHostHelp(myHostServHelp);
	c = createCommand("MNOTICE", bs_mnotice, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, bs_help_mnotice);
	alog("%s: Added command 'MNOTICE'", s_BotServ);
	moduleAddCommand(BOTSERV, c, MOD_HEAD);
	moduleSetBotHelp(myBotServHelp);
	c = createCommand("MNOTICE", os_mnotice, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, os_help_mnotice);
	alog("%s: Added command 'MNOTICE'", s_OperServ);
	moduleAddCommand(OPERSERV, c, MOD_HEAD);
	moduleSetOperHelp(myOperServHelp);
	c = createCommand("MNOTICE", he_mnotice, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, he_help_mnotice);
	alog("%s: Added command 'MNOTICE'", s_HelpServ);
	moduleAddCommand(HELPSERV, c, MOD_HEAD);
	moduleSetHelpHelp(myHelpServHelp);
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
		notice(s_ChanServ, u->nick, "    MNOTICE    Send mass notice to all users");
	}
}

void myNickServHelp(User *u)
{
	if (is_services_admin(u)) {
		notice(s_NickServ, u->nick, "    MNOTICE    Send mass notice to all users");
	}
}

void myHostServHelp(User *u)
{
	if (is_services_admin(u)) {
		notice(s_HostServ, u->nick, "    MNOTICE     Send mass notice to all users");
	}
}

void myBotServHelp(User *u)
{
	if (is_services_admin(u)) {
		notice(s_BotServ, u->nick, "    MNOTICE        Send mass notice to all users");
	}
}

void myOperServHelp(User *u)
{
	if (is_services_admin(u)) {
		notice(s_OperServ, u->nick, "    MNOTICE     Send mass notice to all users");
	}
}

void myHelpServHelp(User *u)
{
	if (is_services_admin(u)) {
		notice(s_HelpServ, u->nick, "     MNOTICE    Send mass notice to all users");
	}
}

int cs_help_mnotice(User *u)
{
	if (is_services_admin(u)) {
		notice(s_ChanServ, u->nick, "Syntax: \2MNOTICE \037message\037\2");
		notice(s_ChanServ, u->nick, " ");
		notice(s_ChanServ, u->nick, "This command will send a mass notice");
		notice(s_ChanServ, u->nick, "to all users on the network.");
		notice(s_ChanServ, u->nick, " ");
		notice(s_ChanServ, u->nick, "Limited to \2Services Admin\2");
	}
	return MOD_CONT;
}

int ns_help_mnotice(User *u)
{
	if (is_services_admin(u)) {
		notice(s_NickServ, u->nick, "Syntax: \2MNOTICE \037message\037\2");
		notice(s_NickServ, u->nick, " ");
		notice(s_NickServ, u->nick, "This command will send a mass notice");
		notice(s_NickServ, u->nick, "to all users on the network.");
		notice(s_NickServ, u->nick, " ");
		notice(s_NickServ, u->nick, "Limited to \2Services Admin\2");
	}
	return MOD_CONT;
}

int bs_help_mnotice(User *u)
{
	if (is_services_admin(u)) {
		notice(s_BotServ, u->nick, "Syntax: \2MNOTICE \037message\037\2");
		notice(s_BotServ, u->nick, " ");
		notice(s_BotServ, u->nick, "This command will send a mass notice");
		notice(s_BotServ, u->nick, "to all users on the network.");
		notice(s_BotServ, u->nick, " ");
		notice(s_BotServ, u->nick, "Limited to \2Services Admin\2");
	}
	return MOD_CONT;
}

int os_help_mnotice(User *u)
{
	if (is_services_admin(u)) {
		notice(s_OperServ, u->nick, "Syntax: \2MNOTICE \037message\037\2");
		notice(s_OperServ, u->nick, " ");
		notice(s_OperServ, u->nick, "This command will send a mass notice");
		notice(s_OperServ, u->nick, "to all users on the network.");
		notice(s_OperServ, u->nick, " ");
		notice(s_OperServ, u->nick, "Limited to \2Services Admin\2");
	}
	return MOD_CONT;
}

int he_help_mnotice(User *u)
{
	if (is_services_admin(u)) {
		notice(s_HelpServ, u->nick, "Syntax: \2MNOTICE \037message\037\2");
		notice(s_HelpServ, u->nick, " ");
		notice(s_HelpServ, u->nick, "This command will send a mass notice");
		notice(s_HelpServ, u->nick, "to all users on the network.");
		notice(s_HelpServ, u->nick, " ");
		notice(s_HelpServ, u->nick, "Limited to \2Services Admin\2");
	}
	return MOD_CONT;
}

int hs_help_mnotice(User *u)
{
	if (is_services_admin(u)) {
		notice(s_HostServ, u->nick, "Syntax: \2MNOTICE \037message\037\2");
		notice(s_HostServ, u->nick, " ");
		notice(s_HostServ, u->nick, "This command will send a mass notice");
		notice(s_HostServ, u->nick, "to all users on the network.");
		notice(s_HostServ, u->nick, " ");
		notice(s_HostServ, u->nick, "Limited to \2Services Admin\2");
	}
	return MOD_CONT;
}

int cs_mnotice(User *u)
{
	int i, count = 0;
	User *u2;
	char *text = moduleGetLastBuffer();
	if (is_services_admin(u)) {
		if (!text) {
			notice(s_ChanServ, u->nick, "Syntax: \2MNOTICE \037message\037\2");
		} else {
			for (i = 0; i < 1024; i++) {
				for (u2 = userlist[i]; u2; u2 = u2->next) {
					notice(s_ChanServ, u2->nick, "%s", text);
					count++;
				}
			}
			notice(s_ChanServ, u->nick, "Successfully sent a mass notice to \2%d\2 users.", count);
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
	char *text = moduleGetLastBuffer();
	if (is_services_admin(u)) {
		if (!text) {
			notice(s_NickServ, u->nick, "Syntax: \2MNOTICE \037message\037\2");
		} else {
			for (i = 0; i < 1024; i++) {
				for (u2 = userlist[i]; u2; u2 = u2->next) {
					notice(s_NickServ, u2->nick, "%s", text);
					count++;
				}
			}
			notice(s_NickServ, u->nick, "Successfully sent a mass notice to \2%d\2 users.", count);
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
	char *text = moduleGetLastBuffer();
	if (is_services_admin(u)) {
		if (!text) {
			notice(s_BotServ, u->nick, "Syntax: \2MNOTICE \037message\037\2");
		} else {
			for (i = 0; i < 1024; i++) {
				for (u2 = userlist[i]; u2; u2 = u2->next) {
					notice(s_BotServ, u2->nick, "%s", text);
					count++;
				}
			}
			notice(s_BotServ, u->nick, "Successfully sent a mass notice to \2%d\2 users.", count);
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
	char *text = moduleGetLastBuffer();
	if (is_services_admin(u)) {
		if (!text) {
			notice(s_OperServ, u->nick, "Syntax: \2MNOTICE \037message\037\2");
		} else {
			for (i = 0; i < 1024; i++) {
				for (u2 = userlist[i]; u2; u2 = u2->next) {
					notice(s_OperServ, u2->nick, "%s", text);
					count++;
				}
			}
			notice(s_OperServ, u->nick, "Successfully sent a mass notice to \2%d\2 users.", count);
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
	char *text = moduleGetLastBuffer();
	if (is_services_admin(u)) {
		if (!text) {
			notice(s_HelpServ, u->nick, "Syntax: \2MNOTICE \037message\037\2");
		} else {
			for (i = 0; i < 1024; i++) {
				for (u2 = userlist[i]; u2; u2 = u2->next) {
					notice(s_HelpServ, u2->nick, "%s", text);
					count++;
				}
			}
			notice(s_HelpServ, u->nick, "Successfully sent a mass notice to \2%d\2 users.", count);
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
	char *text = moduleGetLastBuffer();
	if (is_services_admin(u)) {
		if (!text) {
			notice(s_HostServ, u->nick, "Syntax: \2MNOTICE \037message\037\2");
		} else {
			for (i = 0; i < 1024; i++) {
				for (u2 = userlist[i]; u2; u2 = u2->next) {
					notice(s_HostServ, u2->nick, "%s", text);
					count++;
				}
			}
			notice(s_HostServ, u->nick, "Successfully sent a mass notice to \2%d\2 users.", count);
		}
	}
	if (text)
		free(text);
	return MOD_CONT;
}

/* EOF */

