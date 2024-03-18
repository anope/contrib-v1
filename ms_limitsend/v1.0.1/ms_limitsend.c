#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: ms_limitsend.c v1.0.1, 13-10-2006 n00bie $"

/* -----------------------------------------------------------------------------------
 * Name		: ms_limitsend.c
 * Author	: n00bie
 * Version	: 1.0.1
 * Date		: 10th Sept, 2006
 * Update	: 13th Oct, 2006
 * -----------------------------------------------------------------------------------
 * Description:
 *
 * Using this module, only users who are on the channel access list (HalfOp and above)
 * will be able to send memos to the channel.
 * -----------------------------------------------------------------------------------
 * Tested: Anope-1.7.15, 1.7.16
 *
 * Thanks:
 * Jobe1986 - for giving this module a name (ms_limitsend.c) ^^
 * Viper - for always willing to help ;)
 * Camti - for testing the module
 * -----------------------------------------------------------------------------------
 * Change log:
 * Ver 1.0.0 - First public release
 * Ver 1.0.1 - Minor Code Update
 * -----------------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 1, or (at your option) any later version.
 * -----------------------------------------------------------------------------------
 * This module have no configurable option.
 */

int m_do_send(User *u);
int AnopeInit(int argc, char **argv)
{
    Command *c;
	int status = 0;
    c = createCommand("SEND", m_do_send, NULL, -1, -1, -1, -1, -1);
    status = moduleAddCommand(MEMOSERV, c, MOD_HEAD);
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
	if (status != MOD_ERR_OK) {
		return MOD_STOP;
	} else {
		alog("%s: ms_limitsend: Successfully loaded module.", s_MemoServ);
	}
    return MOD_CONT;
}

void AnopeFini(void)
{
	alog("%s: ms_limitsend: Module unloaded.", s_MemoServ);
}

int m_do_send(User *u)
{
	ChannelInfo *ci;
	char *buf = moduleGetLastBuffer();
	char *target = myStrGetToken(buf, ' ', 0);
	char *text = myStrGetToken(buf, ' ', 1);
	int z = 0;

	if (!target || !text) {
		notice(s_MemoServ, u->nick, "Syntax: \2SEND {\037nick\037 | \037channel\037} \037memo-text\037\2");
		notice(s_MemoServ, u->nick, "\2/msg %s HELP SEND\2 for more information.", s_MemoServ);
	} else if (target[0] == '#') {
		if (!text) {
			notice(s_MemoServ, u->nick, "Syntax: \2SEND #Channel \037memo-text\037\2");
		} else if (!(ci = cs_findchan(target))) {
			notice_lang(s_MemoServ, u, CHAN_X_NOT_REGISTERED, target);
		} else if ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED)) {
			notice_lang(s_MemoServ, u, CHAN_X_FORBIDDEN, target);
		} else if (!nick_identified(u)) {
			notice_lang(s_MemoServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
		} else {
			if (check_access(u, ci, CA_HALFOP) || check_access(u, ci, CA_HALFOPME)) {
				memo_send(u, target, text, z);
				free(target);
				free(text);
				return MOD_STOP;
			} else {
				notice_lang(s_MemoServ, u, ACCESS_DENIED);
				free(target);
				free(text);
				return MOD_STOP;
			}
		}
	} else {
		return MOD_CONT;
	}
	if (target)
		free(target);
	if (text)
		free(text);
	return MOD_STOP;
}

/* EOF */

