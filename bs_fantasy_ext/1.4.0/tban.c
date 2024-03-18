/**
 * Functionality used to temporary ban a user from a channel - Source
 *
 ***********
 * Module Name    : bs_fantasy_ext
 * Author         : Viper <Viper@Anope.org>
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
 * Last Updated   : 23/12/2011
 *
 **/

#include "tban.h"

#ifdef ENABLE_TBAN
int do_tban(User * u, Channel *c, char *target, int timeout) {
	ChannelInfo *ci;
	User *u2;
	int is_same, exists;

	ci = c->ci;

	if (!target) {
		target = u->nick;
		is_same = 1;
	} else
		is_same = (target == u->nick) ? 1 : (stricmp(target, u->nick) == 0);

	if (timeout <= 0)
		timeout = 3600;

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
	} else if (exists && RestrictKB && ((!is_founder(u, ci) && is_services_oper(u2)) ||
			(is_founder(u, ci) && is_services_admin(u2)))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
	} else if (stricmp(target, ci->bi->nick) == 0) {
		bot_raw_ban(u, ci, u->nick, "Oops!");
	} else if (exists) {
		char mask[BUFSIZE], expirebuf[512];
		make_timeleft(u->na, expirebuf, sizeof(expirebuf), timeout);

		do_up_down(u2, c, "down", 0);
		get_idealban(ci, u2, mask, sizeof(mask));
		addTempBan(c, timeout, mask);

		noticeLang(ci->bi->nick, u, LANG_TBAN_RESPONSE, mask, c->name, expirebuf);
	} else {
		char expirebuf[256];
		make_timeleft(u->na, expirebuf, sizeof(expirebuf), timeout);

		/* Only continue if the mask doesn't match an exception or is otherwise prohibited.. */
		if (check_banmask(u, c, target)) {
			addTempBan(c, timeout, target);

			/* Remove all modes from affected users.. */
			do_up_down_mask(u, c, "down", 0, target);

			noticeLang(ci->bi->nick, u, LANG_TBAN_RESPONSE, target, c->name, expirebuf);
		}
	}

	return MOD_CONT;
}
#endif


#ifdef ENABLE_TKICKBAN
int do_tkban(User * u, Channel *c, char *target, int timeout, char *reason) {
	ChannelInfo *ci;
	User *u2;
	int is_same, exists;

	ci = c->ci;

	if (!target) {
		target = u->nick;
		is_same = 1;
	} else
		is_same = (target == u->nick) ? 1 : (stricmp(target, u->nick) == 0);

	if (timeout <= 0)
		timeout = 3600;

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
	} else if (exists && RestrictKB && ((!is_founder(u, ci) && is_services_oper(u2)) ||
			(is_founder(u, ci) && is_services_admin(u2)))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
	} else if (stricmp(target, ci->bi->nick) == 0) {
		bot_raw_ban(u, ci, u->nick, "Oops!");
	} else if (exists) {
		char mask[BUFSIZE], expirebuf[512];
		make_timeleft(u->na, expirebuf, sizeof(expirebuf), timeout);

		do_up_down(u2, c, "down", 0);
		get_idealban(ci, u2, mask, sizeof(mask));
		addTempBan(c, timeout, mask);

		if (is_on_chan(c, u2))
			bot_raw_kick(u, ci, u2->nick, reason);

		noticeLang(ci->bi->nick, u, LANG_TBAN_RESPONSE, mask, c->name, expirebuf);

	} else if (my_match_wild_nocase("*@*", target)) {
		/* Only continue if the mask doesn't match an exception or is otherwise prohibited.. */
		if (check_banmask(u, c, target)) {
			struct c_userlist *cu = NULL, *next = NULL;
			char expirebuf[256];
			make_timeleft(u->na, expirebuf, sizeof(expirebuf), timeout);

			addTempBan(c, timeout, target);

			cu = c->users;
			while (cu) {
				next = cu->next;
				/* This only checks against the cloacked host & vhost for normal users.
				 * IPs are only checked when triggered by an oper.. */
				if (is_oper(u) ? match_usermask_full(target, cu->user, true) : match_usermask(target, cu->user))
					bot_raw_kick(u, ci, cu->user->nick, reason);

				cu = next;
			}

			noticeLang(ci->bi->nick, u, LANG_TBAN_RESPONSE, target, c->name, expirebuf);
		}
	} else
		noticeLang(ci->bi->nick, u, LANG_REQ_NICK_OR_MASK);

	return MOD_CONT;
}
#endif

/* EOF */
