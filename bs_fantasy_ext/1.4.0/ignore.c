/**
 * Methods to modify and access the Services ingore list. - Source
 *
 ***********
 * Module Name    : bs_fantasy_ext
 * Author         : Viper <Viper@Anope.org>
 * Creation Date  : 21/07/2006
 *
 * More info on http://modules.anope.org and http://forum.anope.org
 ***********
 *
 * Based on the code of Anope by The Anope Dev Team
 *
 ***********
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***********
 *
 * Last Updated   : 26/11/2011
 *
 **/

#ifdef ENABLE_IGNORE
#include "ignore.h"

int do_ignoreuser(User * u, Channel *c, char *cmd, char *nick, char *time) {
	ChannelInfo *ci = c->ci;
	int t;

	if (!cmd) {
		notice_lang(ci->bi->nick, u, OPER_IGNORE_SYNTAX);
		return MOD_CONT;
	}

	if (!stricmp(cmd, "ADD")) {
		if (!nick) {
			notice_lang(ci->bi->nick, u, OPER_IGNORE_SYNTAX);
			return MOD_CONT;
		} else if (!time) {
			notice_lang(ci->bi->nick, u, OPER_IGNORE_SYNTAX);
			return MOD_CONT;
		} else {
			t = dotime(time);

			if (t <= -1) {
				notice_lang(ci->bi->nick, u, OPER_IGNORE_VALID_TIME);
				return MOD_CONT;
			} else if (t == 0) {
				t = 157248000;  /* if 0 is given, we set time to 157248000 seconds == 5 years (let's hope the next restart will  be before that time ;-)) */
				add_ignore(nick, t);
				notice_lang(ci->bi->nick, u, OPER_IGNORE_PERM_DONE, nick);
				alog("[bs_fantasy_ext] %s added %s permanently to the ignore list.", u->nick, nick);
			} else {
				add_ignore(nick, t);
				notice_lang(ci->bi->nick, u, OPER_IGNORE_TIME_DONE, nick, time);
				alog("[bs_fantasy_ext] %s added %s to the ignore list for %d seconds.", u->nick, nick, t);
			}
		}
	} else if (!stricmp(cmd, "LIST")) {
			do_ignorelist(u, ci);
			alog("[bs_fantasy_ext] %s requested the ignore list.", u->nick);
	}

	else if (!stricmp(cmd, "DEL")) {
		if (!nick) {
			notice_lang(ci->bi->nick, u, OPER_IGNORE_SYNTAX);
		} else {
			if (delete_ignore(nick)) {
				notice_lang(ci->bi->nick, u, OPER_IGNORE_DEL_DONE, nick);
				alog("[bs_fantasy_ext] %s deleted %s from the ignore list.", u->nick, nick);
				return MOD_CONT;
			} else
				notice_lang(ci->bi->nick, u, OPER_IGNORE_LIST_NOMATCH, nick);
		}
	} else if (!stricmp(cmd, "CLEAR")) {
		clear_ignores();
		notice_lang(ci->bi->nick, u, OPER_IGNORE_LIST_CLEARED);
		alog("[bs_fantasy_ext] %s cleared the ignore list.", u->nick);
		return MOD_CONT;
	} else
		notice_lang(ci->bi->nick, u, OPER_IGNORE_SYNTAX);
	return MOD_CONT;
}

/* shows the Services ignore list */
static int do_ignorelist(User * u, ChannelInfo *ci) {
	IgnoreData *id;

	if (!ignore) {
		notice_lang(ci->bi->nick, u, OPER_IGNORE_LIST_EMPTY);
		return MOD_CONT;
	}

	notice_lang(ci->bi->nick, u, OPER_IGNORE_LIST);
	for (id = ignore; id; id = id->next)
		notice_user(ci->bi->nick, u, "%s", id->mask);

	return MOD_CONT;
}
#endif

/* EOF */
