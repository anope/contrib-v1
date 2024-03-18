/**
 * Functionality used to temporary ban a user from a channel - Source
 *
 ***********
 * Module Name    : bs_fantasy_ext
 * Author         : Viper <Viper@Absurd-IRC.net>
 * Creation Date  : 22/02/2007
 *
 * More info on http://modules.anope.org and http://forum.anope.org
 ***********
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***********
 *
 * Last Updated   : 04/04/2007
 *
 **/

#include "tban.h"

#ifdef ENABLE_TBAN
int do_tban(User * u, Channel *c, char *target, int time) {
	ChannelInfo *ci;
	User *u2;
	int is_same, exists;

	ci = c->ci;

	if (!target) {
		target = u->nick;
		is_same = 1;
	} else
		is_same = (target == u->nick) ? 1 : (stricmp(target, u->nick) == 0);

	if (time <= 0)
		time = 3600;

	if (is_same) {
		u2 = u;
		exists = 1;
	} else
		exists = ((u2 = finduser(target)) ? 1 : 0);

	if (!is_same ? !check_access(u, ci, CA_BAN) : !check_access(u, ci, CA_BANME)) {
		notice_lang(ci->bi->nick, u, ACCESS_DENIED);
	} else if (!is_same && exists && (ci->flags & CI_PEACE) && (get_access(u2, ci) >= get_access(u, ci))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
	/**
	 * Dont ban the user on channels where he is excepted
	 * to prevent services <-> server wars.
	 **/
	} else if (exists && (ircd->except && is_excepted(ci, u2))) {
		notice_lang(ci->bi->nick, u, CHAN_EXCEPTED, u2->nick, ci->name);
	} else if (!exists && (ircd->except && is_excepted_mask(ci, target))) {
		notice_lang(ci->bi->nick, u, CHAN_EXCEPTED, target, ci->name);
	} else if (exists && ((ircd->protectedumode && is_protected(u2)) && !is_founder(u, ci))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
	} else if (stricmp(target, ci->bi->nick) == 0) {
		bot_raw_ban(u, ci, u->nick, "Oops!");
	} else if (exists) {
		char mask[BUFSIZE];
		char *t = makeexpiry(time);

		do_up_down(u2, c, "down", 0);
		get_idealban(ci, u2, mask, sizeof(mask));
		addTempBan(c, time, mask);

		moduleNoticeLang(ci->bi->nick, u, LANG_TBAN_RESPONSE, mask, c->name, t);
		free(t);
	} else {
		char *t = makeexpiry(time);
		addTempBan(c, time, target);

		moduleNoticeLang(ci->bi->nick, u, LANG_TBAN_RESPONSE, target, c->name, t);
		free(t);
	}

	return MOD_CONT;
}
#endif


#ifdef ENABLE_TKICKBAN
int do_tkban(User * u, Channel *c, char *target, int time, char *reason) {
	ChannelInfo *ci;
	User *u2;
	int is_same, exists;

	ci = c->ci;

	if (!target) {
		target = u->nick;
		is_same = 1;
	} else
		is_same = (target == u->nick) ? 1 : (stricmp(target, u->nick) == 0);

	if (time <= 0)
		time = 3600;

	if (!reason)
		reason = "Requested";

	if (is_same) {
		u2 = u;
		exists = 1;
	} else
		exists = ((u2 = finduser(target)) ? 1 : 0);

	if (!is_same ? !check_access(u, ci, CA_BAN) : !check_access(u, ci, CA_BANME)) {
		notice_lang(ci->bi->nick, u, ACCESS_DENIED);
	} else if (!is_same && exists && (ci->flags & CI_PEACE) && (get_access(u2, ci) >= get_access(u, ci))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
	} else if (exists && ((ircd->protectedumode && is_protected(u2)) && !is_founder(u, ci))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
	/**
	 * Dont ban the user on channels where he is excepted
	 * to prevent services <-> server wars.
	 **/
	} else if (exists && (ircd->except && is_excepted(ci, u2))) {
		notice_lang(ci->bi->nick, u, CHAN_EXCEPTED, u2->nick, ci->name);
	} else if (!exists && (ircd->except && is_excepted_mask(ci, target))) {
		notice_lang(ci->bi->nick, u, CHAN_EXCEPTED, target, ci->name);
	} else if (stricmp(target, ci->bi->nick) == 0) {
		bot_raw_ban(u, ci, u->nick, "Oops!");
	} else {
		if (exists) {
			char mask[BUFSIZE];
			char *t = makeexpiry(time);

			do_up_down(u2, c, "down", 0);
			get_idealban(ci, u2, mask, sizeof(mask));
			addTempBan(c, time, mask);

			if (is_on_chan(c, u2))
				bot_raw_kick(u, ci, u2->nick, reason);

			moduleNoticeLang(ci->bi->nick, u, LANG_TBAN_RESPONSE, mask, c->name, t);
			free(t);

		} else if (match_wild_nocase("*@*", target)) {
			struct c_userlist *cu = NULL;
			int ok = 1;
			cu = c->users;

			/* Check to make sure were are not going to ban an admin here... */
			while ((cu) && ok) {
				if (match_usermask(target, cu->user)) {
					if (ircd->protectedumode && is_protected(cu->user) && !is_founder(u, ci)) {
						notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
						ok = 0;
					} else if ((ci->flags & CI_PEACE) && (get_access(cu->user, ci) >= get_access(u, ci))) {
						notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
						ok = 0;
					} else if (ircd->except && is_excepted(ci, cu->user)) {
						notice_lang(ci->bi->nick, u, CHAN_EXCEPTED, cu->user->nick, ci->name);
						ok = 0;
					}
				}
				cu = cu->next;
			}

			if (ok) {
				char *t = makeexpiry(time);
				addTempBan(c, time, target);
				cu = c->users;

				while (cu) {
					if (match_usermask(target, cu->user))
						bot_raw_kick(u, ci, cu->user->nick, reason);

					cu = cu->next;
				}

				moduleNoticeLang(ci->bi->nick, u, LANG_TBAN_RESPONSE, target, c->name, t);
				free(t);
			}
		} else
			moduleNoticeLang(ci->bi->nick, u, LANG_REQ_NICK_OR_MASK);
	}

	return MOD_CONT;
}
#endif


/* EOF */
