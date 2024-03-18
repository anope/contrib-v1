#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: ns_noexpire.c v2.0.0 14-03-2008 n00bie $"
#define NOEXPIRE_SYNTAX "Syntax: \2SET NOEXPIRE {ON | OFF}\2"

/* -----------------------------------------------------------------------------------
 * Name		: ns_noexpire.c
 * Author	: n00bie
 * Version	: 2.0.1
 * Date		: 16th January, 2007
 * Updated	: 14th March, 2008
 * -----------------------------------------------------------------------------------
 * Tested	: Anope-1.7.21 (1341)
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
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS for
 * a PARTICULAR PURPOSE. THIS MODULE IS DISTRIBUTED 'AS IS'. NO WARRANTY OF ANY
 * KIND IS EXPRESSED OR IMPLIED. YOU USE AT YOUR OWN RISK. THE MODULE AUTHOR WILL 
 * NOT BE RESPONSIBLE FOR DATA LOSS, DAMAGES, LOSS OF PROFITS OR ANY OTHER KIND OF 
 * LOSS WHILE USING OR MISUSING THIS MODULE. 
 *
 * See the GNU General Public License for more details.
 * -----------------------------------------------------------------------------------
 * Changelog:
 * v1.0.0 - First initial release.
 * v1.0.1 - Fixed unfree'd memory (thanks Viper)
 * v2.0.0 - Fixed a syntax bug (crash bug?) when trying to unset 
 *          URL, EMAIL, GREET & ICQ (thanks to Fr3d for reporting)
* -----------------------------------------------------------------------------------
 * This module have no configurable option.
 *
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
	notice(s_NickServ, u->nick, "Syntax: \2SET option parameters\2");
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
	notice(s_NickServ, u->nick, "               \2/msg %s LIST\2", s_NickServ);
	notice(s_NickServ, u->nick, "HIDE       Hide certain pieces of nickname information");
	notice(s_NickServ, u->nick, "MSG        Change the communication method of Services");
	notice(s_NickServ, u->nick, "AUTOOP     Should services op you automatically");
	notice(s_NickServ, u->nick, "NOEXPIRE   Prevent your nickname from expiring");
	notice(s_NickServ, u->nick, " ");
	notice(s_NickServ, u->nick, "In order to use this command, you must first identify");
	notice(s_NickServ, u->nick, "with your password (\2/msg %s HELP IDENTIFY\2 for more", s_NickServ);
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
	if (!param
        && (!cmd
            || (stricmp(cmd, "URL") != 0 && stricmp(cmd, "EMAIL") != 0
                && stricmp(cmd, "GREET") != 0
                && stricmp(cmd, "ICQ") != 0))) {
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
