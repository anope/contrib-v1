#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: ns_masset.c v1.0.0 19-10-2006 n00bie $"
#define MASS_SET_SYNTAX "Syntax: \2MASSET [KILL|KILLQUICK|SECURE|AUTOOP] {ON | OFF}\2"

/* -----------------------------------------------------------------------------------
 * Name		: ns_masset.c [NickServ Mass Set]
 * Author	: n00bie
 * Version	: 1.0.0
 * Date		: 19th October, 2006
 * -----------------------------------------------------------------------------------
 * Supported: Anope-1.7.16, 1.7.17
 * -----------------------------------------------------------------------------------
 * Description:
 *
 * This module provides mass commands for various NickServ settings like KILL, 
 * KILL QUICK, SECURE and AUTOOP. Using this module or commands ALL REGISTERED NICK 
 * on the network will be affected, so use the provided commands wisely and
 * respect your network users settings ;b
 * -----------------------------------------------------------------------------------
 * Providing commands:
 *
 * /msg NickServ MASSET KILL {ON | OFF}
 * /msg NickServ MASSET KILLQUICK {ON | OFF}
 * /msg NickServ MASSET SECURE {ON | OFF}
 * /msg NickServ MASSET AUTOOP {ON | OFF}
 * /msg NickServ HELP MASSET 
 * -----------------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 1, or (at your option) any later version.
 * -----------------------------------------------------------------------------------
 * This module have no configurable option.
 */

void myNickServHelp(User *u);
int myNickServHelpFull(User *u);
int do_mass_set(User *u);
int AnopeInit(int argc, char **argv)
{
	Command *c;
	int status = 0;
	c = createCommand("MASSET", do_mass_set, is_services_admin, -1, -1, -1, -1, -1);
	status = moduleAddCommand(NICKSERV, c, MOD_HEAD);
	moduleAddHelp(c, myNickServHelpFull);
	moduleSetNickHelp(myNickServHelp);
	if (status != MOD_ERR_OK) {
		return MOD_STOP;
	} else {
		alog("%s: Successfully loaded module.", s_NickServ);
		alog("%s: New command: \2/msg NickServ HELP MASSET\2", s_NickServ);
	}
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("%s: ns_masset: Module Unloaded.", s_NickServ);
}

void myNickServHelp(User *u)
{
	if (is_services_admin(u)) {
		notice(s_NickServ, u->nick, "    MASSET     Set various settings for all registered users");
	}
}

int myNickServHelpFull(User *u)
{
	if (is_services_admin(u)) {
		notice(s_NickServ, u->nick, MASS_SET_SYNTAX);
		notice(s_NickServ, u->nick, " ");
		notice(s_NickServ, u->nick, "\2KILL\2 - Turns the automatic protection option for");
		notice(s_NickServ, u->nick, "        all registered nick ON or OFF.");
		notice(s_NickServ, u->nick, "\2KILLQUICK\2 - Turns the automatic protection option");
		notice(s_NickServ, u->nick, "        for all registered nick ON or OFF. All users");
		notice(s_NickServ, u->nick, "        will be given only 20 seconds to change nicks");
		notice(s_NickServ, u->nick, "        instead of the usual 60 seconds.");
		notice(s_NickServ, u->nick, "\2SECURE\2 - Turns NickServ's security features ON or OFF");
		notice(s_NickServ, u->nick, "        for all registered nick.");
		notice(s_NickServ, u->nick, "\2AUTOOP\2 - Turns NickServ's auto op features ON or OFF");
		notice(s_NickServ, u->nick, "        for all registered nick.");
		notice(s_NickServ, u->nick, " ");
		notice(s_NickServ, u->nick, "Limited to \2Services Admins\2.");
	} else {
		notice(s_NickServ, u->nick, "No help available for \2masset\2.");
	}
	return MOD_CONT;
}

int do_mass_set(User *u)
{
	char *buf = moduleGetLastBuffer();
	char *option = myStrGetToken(buf, ' ', 0);
	char *param = myStrGetToken(buf, ' ', 1);
	int i, count = 0;
	NickCore *nc, *next;

	if (!is_services_admin(u)) {
		return MOD_CONT;
	}
	if (!option || !param) {
		notice(s_NickServ, u->nick, MASS_SET_SYNTAX);
	} else if (stricmp(option, "KILL") == 0) {
		if (stricmp(param, "ON") == 0) {
			for (i = 0; i < 1024; i++) {
				for (nc = nclists[i]; nc; nc = next) {
					next = nc->next;
					count++;
					nc->flags |= NI_KILLPROTECT;
					nc->flags &= ~(NI_KILL_QUICK | NI_KILL_IMMED);
				}
			}
			notice(s_NickServ, u->nick, "Successfully enabled \2protection\2 for \2%d\2 users.", count);
		} else if (stricmp(param, "OFF") == 0) {
			for (i = 0; i < 1024; i++) {
				for (nc = nclists[i]; nc; nc = next) {
					next = nc->next;
					count++;
					nc->flags &= ~(NI_KILLPROTECT | NI_KILL_QUICK | NI_KILL_IMMED);
				}
			}
			notice(s_NickServ, u->nick, "Successfully disabled \2protection\2 for \2%d\2 users.", count);
		} else {
			notice(s_NickServ, u->nick, "Syntax: \2/msg %s MASSET KILL {ON | OFF}\2", s_NickServ);
		}
	} else if (stricmp(option, "KILLQUICK") == 0) {
		if (stricmp(param, "ON") == 0) {
			for (i = 0; i < 1024; i++) {
				for (nc = nclists[i]; nc; nc = next) {
					next = nc->next;
					count++;
					nc->flags |= NI_KILLPROTECT | NI_KILL_QUICK;
					nc->flags &= ~NI_KILL_IMMED;
				}
			}
			notice(s_NickServ, u->nick, "Successfully enabled \2quick kill protection\2 for \2%d\2 users.", count);
		} else if (stricmp(param, "OFF") == 0) {
			for (i = 0; i < 1024; i++) {
				for (nc = nclists[i]; nc; nc = next) {
					next = nc->next;
					count++;
					nc->flags |= NI_KILLPROTECT;
					nc->flags &= ~(NI_KILL_QUICK | NI_KILL_IMMED);
				}
			}
			notice(s_NickServ, u->nick, "Successfully disabled \2quick kill protection\2 for \2%d\2 users.", count);
		} else {
			notice(s_NickServ, u->nick, "Syntax: \2/msg %s MASSET KILLQUICK {ON | OFF}\2", s_NickServ);
		}
	} else if (stricmp(option, "SECURE") == 0) {
		if (stricmp(param, "ON") == 0) {
			for (i = 0; i < 1024; i++) {
				for (nc = nclists[i]; nc; nc = next) {
					next = nc->next;
					count++;
					nc->flags |= NI_SECURE;
				}
			}
			notice(s_NickServ, u->nick, "Successfully enabled \2security feature\2 for \2%d\2 users.", count);
		} else if (stricmp(param, "OFF") == 0) {
			notice(s_NickServ, u->nick, "Sorry, \2security feature\2 cannot be disabled.");
			notice(s_NickServ, u->nick, "Don't you want your network secure?");
		} else {
			notice(s_NickServ, u->nick, "Syntax: \2/msg %s MASSET SECURE {ON | OFF}\2", s_NickServ);
		}
	} else if (stricmp(option, "AUTOOP") == 0) {
		if (stricmp(param, "ON") == 0) {
			for (i = 0; i < 1024; i++) {
				for (nc = nclists[i]; nc; nc = next) {
					next = nc->next;
					count++;
					nc->flags &= ~NI_AUTOOP;
				}
			}
			notice(s_NickServ, u->nick, "Successfully enabled \2autoop\2 for \2%d\2 users.", count);
		} else if (stricmp(param, "OFF") == 0) {
			for (i = 0; i < 1024; i++) {
				for (nc = nclists[i]; nc; nc = next) {
					next = nc->next;
					count++;
					nc->flags |= NI_AUTOOP;
				}
			}
			notice(s_NickServ, u->nick, "Successfully disabled \2autoop\2 for \2%d\2 users.", count);
		} else {
			notice(s_NickServ, u->nick, "Syntax: \2/msg %s MASSET AUTOOP {ON | OFF}\2", s_NickServ);
		}
	} else {
		notice(s_NickServ, u->nick, "Unknown MASSET option \2%s\2.", option);
		notice(s_NickServ, u->nick, "See /msg %s HELP MASSET for valid options.", s_NickServ);
	}
	if (option)
		free(option);
	if (param)
		free(param);
	return MOD_CONT;
}

/* EOF */

