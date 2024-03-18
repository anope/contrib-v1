/**
 * Main functions for handling the mute and unmute commands - Source
 *
 ***********
 * Module Name    : bs_fantasy_ext
 * Author         : Viper <Viper@Anope.org>
 * Creation Date  : 14/09/2006
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
 * Last Updated   : 05/10/2011
 *
 **/

#ifdef ENABLE_MUTEUNMUTE
#include "mute.h"

int do_mute(User * u, Channel *c, char *params) {
	ChannelInfo *ci = c->ci;
	User *u2;
	int is_same, exists;

	if (!params) {
		params = u->nick;
	}

	is_same = (params == u->nick) ? 1 : (stricmp(params, u->nick) == 0);

	if (is_same) {
		u2 = u;
		exists = 1;
	} else
		exists = ((u2 = finduser(params)) ? 1 : 0);

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
	} else if (!exists && (ircd->except && is_excepted_mask(ci, params))) {
		notice_lang(ci->bi->nick, u, CHAN_EXCEPTED, params, ci->name);
	} else if (exists && RestrictKB && ((!is_founder(u, ci) && is_services_oper(u2)) ||
			(is_founder(u, ci) && is_services_admin(u2)))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
	} else {
		if (!stricmp(params, ci->bi->nick))
			bot_raw_kick(u, ci, u->nick, "Oops!");

		else {
			char mask[BUFSIZE];
			int ok = 1;

			if (exists) {
				do_up_down(u2, c, "down", 0);
				get_idealban(ci, u2, mask, sizeof(mask));
			} else if (my_match_wild_nocase("*@*", params)) {
				/* If we get a *@* target we need to add the *!... */
				if (!my_match_wild_nocase("*!*@*", params))
					snprintf(mask, BUFSIZE, "*!%s", params);
				else
					snprintf(mask, BUFSIZE, "%s", params);

				/* Only continue if the mask doesn't match an exception or is otherwise prohibited.. */
				ok = check_banmask(u, c, mask);
			} else {
				noticeLang(ci->bi->nick, u, LANG_REQ_NICK_OR_MASK);
				ok = 0;
			}

			if (ok) {
				char buf[BUFSIZE];
				char *av[2];

				if (!stricmp(IRCDModule, "unreal32")) {
					snprintf(buf, sizeof(buf), "~q:%s", mask);
					av[0] = "+b";
					av[1] = buf;
				} else if (!stricmp(IRCDModule, "inspircd12") || !stricmp(IRCDModule, "inspircd20")) {
					snprintf(buf, sizeof(buf), "m:%s", mask);
					av[0] = "+b";
					av[1] = buf;
				} else if (!stricmp(IRCDModule, "charybdis")) {
					av[0] = "+q";
					av[1] = mask;
				} else {
					av[0] = "+b";
					av[1] = mask;
				}

				anope_cmd_mode(ci->bi->nick,c->name, "%s %s", av[0], av[1]);
				chan_set_modes(ci->bi->nick, c, 2, av, 1);

				if (!exists) {
					/* Remove all modes from affected users.. */
					do_up_down_mask(u, c, "down", 0, mask);
				}
			}
		}
	}

	return MOD_CONT;
}

int do_unmute(User * u, Channel *c, char *params) {
	ChannelInfo *ci = c->ci;
	User *u2;
	int is_same, exists;

	if (!params) {
		params = u->nick;
	}

	is_same = (params == u->nick) ? 1 : (stricmp(params, u->nick) == 0);

	if (is_same) {
		u2 = u;
		exists = 1;
	} else
		exists = ((u2 = finduser(params)) ? 1 : 0);

	if (!check_access(u, ci, CA_UNBAN)) {
		notice_lang(ci->bi->nick, u, ACCESS_DENIED);
	} else {
		char mask[BUFSIZE], buf[BUFSIZE];
		char *av[2];

		if (exists) {
			get_idealban(ci, u2, mask, sizeof(mask));
		} else
			snprintf(mask, BUFSIZE, "%s", params);

		if (!stricmp(IRCDModule, "unreal32")) {
			snprintf(buf, sizeof(buf), "~q:%s", mask);
			av[0] = "-b";
			av[1] = buf;
		} else if (!stricmp(IRCDModule, "inspircd12") || !stricmp(IRCDModule, "inspircd20")) {
			snprintf(buf, sizeof(buf), "m:%s", mask);
			av[0] = "-b";
			av[1] = buf;
		} else if (!stricmp(IRCDModule, "charybdis")) {
			av[0] = "-q";
			av[1] = mask;
		} else {
			av[0] = "-b";
			av[1] = sstrdup(mask);
		}

		anope_cmd_mode(ci->bi->nick,c ->name, "%s %s", av[0], av[1]);
		chan_set_modes(ci->bi->nick, c, 2, av, 1);
	}

	return MOD_CONT;
}
#endif


/* EOF */
