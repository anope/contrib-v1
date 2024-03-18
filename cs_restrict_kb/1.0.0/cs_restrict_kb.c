#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: cs_restrict_kb.c v1.0.0 14-01-2007 n00bie $"

/* ------------------------------------------------------------------------------------
 * Name		: cs_restrict_kb.c
 * Author	: n00bie
 * Version	: 1.0.0
 * Date		: 14th Jan, 2007
 * ------------------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 1, or (at your option) any later version.
 * ------------------------------------------------------------------------------------
 * Tested	: Anope-1.7.18
 *
 * This module have been suggested by zach on Anope's forum.
 * http://forum.anope.org/viewthread.php?tid=1196
 * ------------------------------------------------------------------------------------
 * Description:
 *
 * This module will restrict the used of ChanServ KICK/BAN command by normal users
 * against Services Operator and above.
 * ------------------------------------------------------------------------------------
 * This module have no configurable option.
 */

int doKick(User *u);
int doBan(User *u);
int AnopeInit(int argc, char **argv)
{
	Command *c;
	int status = 0;
	c = createCommand("KICK", doKick, NULL, -1, -1, -1, -1, -1);
	status = moduleAddCommand(CHANSERV, c, MOD_HEAD);
	c = createCommand("BAN", doBan, NULL, -1, -1, -1, -1, -1);
	status = moduleAddCommand(CHANSERV, c, MOD_HEAD);
	if (status != MOD_ERR_OK) {
		return MOD_STOP;
	} else {
		alog("%s: Success loading module.", s_ChanServ);
	}
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("%s: cs_restrict_kb: Module unloaded.", s_ChanServ);
}

int doKick(User *u)
{
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	char *params = myStrGetToken(buf, ' ', 1);
	User *u2;
	if (!(u2 = finduser(params))) {
		notice_lang(s_ChanServ, u, NICK_X_NOT_IN_USE, params);
	} else if (is_services_oper(u2)) {
		notice_lang(s_ChanServ, u, PERMISSION_DENIED);
		free(chan);
		free(params);
		return MOD_STOP;
	}
	if (chan)
		free(chan);
	if (params)
		free(params);
	return MOD_CONT;
}

int doBan(User *u)
{
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	char *params = myStrGetToken(buf, ' ', 1);
	User *u2;
	if (!(u2 = finduser(params))) {
		notice_lang(s_ChanServ, u, NICK_X_NOT_IN_USE, params);
	} else if (is_services_oper(u2)) {
		notice_lang(s_ChanServ, u, PERMISSION_DENIED);
		free(chan);
		free(params);
		return MOD_STOP;
	}
	if (chan)
		free(chan);
	if (params)
		free(params);
	return MOD_CONT;
}

/* EOF */
