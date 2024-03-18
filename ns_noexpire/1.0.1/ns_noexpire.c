#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: ns_noexpire.c v1.0.1 23-01-2007 n00bie $"
#define NOEXPIRE_SYNTAX "Syntax: \2SET NOEXPIRE {ON | OFF}\2"

/* -----------------------------------------------------------------------------------
 * Name		: ns_noexpire.c
 * Author	: n00bie
 * Version	: 1.0.1
 * Date		: 16th Jan, 2007
 * Updated	: 23th Jan, 2007
 * -----------------------------------------------------------------------------------
 * Tested	: Anope-1.7.18 (1214)
 * -----------------------------------------------------------------------------------
 * Description:
 *
 * This module provides NickServ NOEXPIRE settings for normal registered users.
 * Ideas and requested by don_will
 * -----------------------------------------------------------------------------------
 * Providing commands:
 *
 * /msg NickServ SET NOEXPIRE {ON | OFF}
 * /msg NickServ HELP SET NOEXPIRE
 * -----------------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 1, or (at your option) any later version.
 * -----------------------------------------------------------------------------------
 * This module have no configurable option.
 *
 * Thanks to: Viper, for informing me about the mem leak :)
 */

int do_mset(User *u);
int myNickServHelpSet(User *u);
int myNickServHelpSetNoExpire(User *u);
int AnopeInit(int argc, char **argv)
{
	Command *c;
	int status = 0;
	c = createCommand("SET", do_mset, NULL, -1, -1, -1, -1, -1);
	status = moduleAddCommand(NICKSERV, c, MOD_HEAD);
	moduleAddHelp(c, myNickServHelpSet);
    c = createCommand("SET NOEXPIRE", do_mset, NULL, -1, -1, -1, -1, -1);
	status = moduleAddCommand(NICKSERV, c, MOD_HEAD);
	moduleAddHelp(c, myNickServHelpSetNoExpire);
	if (status != MOD_ERR_OK) {
		return MOD_STOP;
	} else {
		alog("ns_noexpire: Successfully loaded module.");
		alog("ns_noexpire: \2/msg %s HELP SET NOEXPIRE\2", s_NickServ);
	}
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("ns_noexpire: Module Unloaded.");
}

int myNickServHelpSet(User *u)
{
	notice(s_NickServ, u->nick, "Syntax: SET option parameters");
	notice(s_NickServ, u->nick, " ");
	notice(s_NickServ, u->nick, "Sets various nickname options.  option can be one of:");
	notice(s_NickServ, u->nick, " ");
	notice(s_NickServ, u->nick, "DISPLAY    Set the display of your group in Services");
	notice(s_NickServ, u->nick, "PASSWORD   Set your nickname password");
	notice(s_NickServ, u->nick, "LANGUAGE   Set the language Services will use when");
	notice(s_NickServ, u->nick, "               sending messages to you");
	notice(s_NickServ, u->nick, "URL        Associate a URL with your nickname");
	notice(s_NickServ, u->nick, "EMAIL      Associate an E-mail address with your nickname");
	notice(s_NickServ, u->nick, "ICQ        Associate an ICQ number with your nickname");
	notice(s_NickServ, u->nick, "GREET      Associate a greet message with your nickname");
	notice(s_NickServ, u->nick, "KILL       Turn protection on or off");
	notice(s_NickServ, u->nick, "SECURE     Turn nickname security on or off");
	notice(s_NickServ, u->nick, "PRIVATE    Prevent your nickname from appearing in a");
	notice(s_NickServ, u->nick, "               /msg NickServ LIST");
	notice(s_NickServ, u->nick, "HIDE       Hide certain pieces of nickname information");
	notice(s_NickServ, u->nick, "MSG        Change the communication method of Services");
	notice(s_NickServ, u->nick, "AUTOOP     Should services op you automatically");
	notice(s_NickServ, u->nick, "NOEXPIRE   Prevent your nickname from expiring");
	notice(s_NickServ, u->nick, " ");
	notice(s_NickServ, u->nick, "In order to use this command, you must first identify");
	notice(s_NickServ, u->nick, "with your password (/msg NickServ HELP IDENTIFY for more");
	notice(s_NickServ, u->nick, "information).");
	return MOD_STOP;
}

int myNickServHelpSetNoExpire(User *u)
{
	notice(s_NickServ, u->nick, NOEXPIRE_SYNTAX);
	notice(s_NickServ, u->nick, " ");
	notice(s_NickServ, u->nick, "Sets whether your nickname will expire. Setting");
	notice(s_NickServ, u->nick, "this to \2ON\2 prevents your nickname from expiring.");
	return MOD_STOP;
}

int do_mset(User *u)
{
	char *buf = moduleGetLastBuffer();
	char *cmd = myStrGetToken(buf, ' ', 0);
	char *param = myStrGetToken(buf, ' ', 1);
	NickAlias *na = u->na;
	if (readonly) {
		notice_lang(s_NickServ, u, NICK_SET_DISABLED);
		if (cmd) free(cmd);
		if (param) free(param);
		return MOD_STOP;
	}
	if (!cmd || !param) {
		syntax_error(s_NickServ, u, "SET", NICK_SET_SYNTAX);
		if (cmd) free(cmd);
		if (param) free(param);
		return MOD_STOP;
	} else if (!na) {
		if (cmd) free(cmd);
		if (param) free(param);
		return MOD_CONT;
	} else if (na->status & NS_VERBOTEN) {
		if (cmd) free(cmd);
		if (param) free(param);
		return MOD_CONT;
	} else if (na->nc->flags & NI_SUSPENDED) {
		if (cmd) free(cmd);
		if (param) free(param);
		return MOD_CONT;
	} else if (!nick_identified(u)) {
		if (cmd) free(cmd);
		if (param) free(param);
		return MOD_CONT;
	} else if (stricmp(cmd, "NOEXPIRE") == 0) {
		if (stricmp(param, "ON") == 0) {
			na->status |= NS_NO_EXPIRE;
			notice(s_NickServ, u->nick, "Nick %s \2will not\2 expired.", na->nick);
		} else if (stricmp(param, "OFF") == 0) {
			na->status &= ~NS_NO_EXPIRE;
			notice(s_NickServ, u->nick, "Nick %s \2will\2 expire.", na->nick);
		} else {
			notice(s_NickServ, u->nick, NOEXPIRE_SYNTAX);
		}
		free(cmd);
		free(param);
		return MOD_STOP;
	}
	return MOD_CONT;
}

/* EOF */
