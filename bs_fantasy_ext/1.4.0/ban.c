/**
 * Main routines for the fantasy ban and unban commands - Source
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
 * Last Updated   : 25/11/2011
 *
 **/

#include "ban.h"

#ifdef ENABLE_BAN
int do_ban(User * u, Channel *c, char *params) {
	ChannelInfo *ci = c->ci;
	User *u2;
	int is_same, exists;
	char *av[2];

	if (!params) {
		params = u->nick;
		is_same = 1;
	} else
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
	/**
	 * Dont ban the user on channels where he is excepted
	 * to prevent services <-> server wars.
	 **/
	} else if (exists && (ircd->except && is_excepted(ci, u2))) {
		notice_lang(ci->bi->nick, u, CHAN_EXCEPTED, u2->nick, ci->name);
	} else if (!exists && (ircd->except && is_excepted_mask(ci, params))) {
		notice_lang(ci->bi->nick, u, CHAN_EXCEPTED, params, ci->name);
	} else if (exists && ((ircd->protectedumode && is_protected(u2)) && !is_founder(u, ci))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
	} else if (exists && RestrictKB && ((!is_founder(u, ci) && is_services_oper(u2)) ||
			(is_founder(u, ci) && is_services_admin(u2)))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
	} else if (exists) {
		char mask[BUFSIZE];

		if (!stricmp(u2->nick, ci->bi->nick))
			bot_raw_kick(u, ci, u->nick, "Oops!");

		else {
			do_up_down(u2, c, "down", 0);

			av[0] = "+b";
			get_idealban(ci, u2, mask, sizeof(mask));
			av[1] = mask;
			anope_cmd_mode(ci->bi->nick, c->name, "+b %s", av[1]);
			chan_set_modes(ci->bi->nick, c, 2, av, 1);
		}
	/* It could be a hostmask.. set whatever we get */
	} else {
		char target[BUFSIZE];

		/* If we get a *!*@* target we don't need to do anything... */
		if (my_match_wild_nocase("*!*@*", params))
			snprintf(target, BUFSIZE, "%s", params);
		else if (my_match_wild_nocase("*@*", params))
			snprintf(target, BUFSIZE, "*!%s", params);
		else
			snprintf(target, BUFSIZE, "%s!*@*", params);

		/* Only continue if the mask doesn't match an exception or is otherwise prohibited.. */
		if (check_banmask(u, c, target)) {
			av[0] = "+b";
			av[1] = target;

			anope_cmd_mode(ci->bi->nick, c->name, "+b %s", av[1]);
			chan_set_modes(ci->bi->nick, c, 2, av, 1);

			/* Remove all modes from affected users.. */
			do_up_down_mask(u, c, "down", 0, target);
		}
	}

	return MOD_CONT;
}
#endif


#ifdef ENABLE_UNBAN
/**
 * Unbanning users based on IP or real host imposes the risk of disclosing a users'
 * real host or IP address. For this reason bans are only matched against a users'
 * cloaked host, IP or their vhost.
 *
 * When unbanning yourself, bans will still be compared against your real host & IP.
 **/
int do_unban(User * u, Channel *c, char *params) {
	Entry *ban, *next;
	ChannelInfo *ci = c->ci;
	User *u2;
	uint32 ip = 0;
	int is_same, exists;

	/* If the banlist is empty, stop here.. */
	if (!c->bans || !c->bans->entries || !c->bans->count)
		return MOD_CONT;

	if (!params) {
		params = u->nick;
		is_same = 1;
	} else
		is_same = (params == u->nick) ? 1 : (stricmp(params, u->nick) == 0);

	if (is_same) {
		u2 = u;
		exists = 1;
	} else
		exists = ((u2 = finduser(params)) ? 1 : 0);

	if (!check_access(u, ci, CA_UNBAN)) {
		notice_lang(ci->bi->nick, u, ACCESS_DENIED);
	} else {
		if (exists) {
			/* If we are unbanning ourselves, allow checking the IP so make sure we have the IP... */
			if (is_same || is_oper(u)) {
				if (!u2->hostip)
					u2->hostip = host_resolve(u2->host);
				if (u2->hostip)
					ip = str_is_ip(u2->hostip);
			}

			/* We can't use SVSMODE UNBAN because it might give the users' IP away, so we
			 * do it ourselves...
			 * We can't rely on the cores functions either because we remove extended bans too.. */
			for (ban = c->bans->entries; ban; ban = next) {
				char *nick = NULL, *uname = NULL, *host = NULL, *temp = NULL;
				int type = 0;
				next = ban->next;

				/* If it s UnrealIRCd, check whether it s an extended ban...  */
				if (!stricmp(IRCDModule, "unreal32")) {
					if (my_match_wild("~r:*", ban->mask)) {
						nick = myStrGetToken(ban->mask, ':', 1);
						type = 3;
					} else if (my_match_wild("??:*!*@*", ban->mask)) {
						temp = myStrGetToken(ban->mask, ':', 1);
						type = 2;
					} else
						type = 1;
				} else if (!stricmp(IRCDModule, "inspircd12") || !stricmp(IRCDModule, "inspircd20")) {
					if (my_match_wild("M:*", ban->mask) || my_match_wild("R:*", ban->mask)) {
						nick = myStrGetToken(ban->mask, ':', 1);
						type = 4;
					} else if (my_match_wild("r:*", ban->mask)) {
						nick = myStrGetToken(ban->mask, ':', 1);
						type = 3;
					} else if (my_match_wild("?:*!*@*", ban->mask)) {
						temp = myStrGetToken(ban->mask, ':', 1);
						type = 2;
					} else
						type = 1;
				} else
					type = 1;

				/* Regular bans require no special treatment and can be most efficiently handled by the core.. */
				if (type == 1) {
					if (((is_same || is_oper(u)) && entry_match(ban, u2->nick, u2->username, u2->host, ip)) ||
							entry_match(ban, u2->nick, u2->vident, u2->vhost, 0) ||
							entry_match(ban, u2->nick, u2->username, u2->chost, 0))
						delBan(ci, ban);

				/* Extended bans of this type are checked against the users display nick.. */
				} else if (type == 4) {
					if (u2->na && u2->na->nc && !stricmp(nick, u2->na->nc->display))
						delBan(ci, ban);

				/* Extended bans we match against ourselves.. */
				} else {
					if (type == 2)
						split_usermask(temp, &nick, &uname, &host);

					if (my_match_wild_nocase(nick, u2->nick)) {
						/* Extended bans of type "~r:nickname" */
						if (type == 3)
							delBan(ci, ban);

						/* Extended bans of type "??:nick!user@host" */
						else if (my_match_wild_nocase(uname, u2->username) ||
								(u2->vident && my_match_wild_nocase(uname, u2->vident))) {
							/* Match against the cloaked host and vhost... */
							if (my_match_wild_nocase(host, u2->chost) ||
									(u2->vhost && my_match_wild_nocase(host, u2->vhost))) {
								delBan(ci, ban);
							} else

							/* Only look for IP based matches if we are unbanning ourselves.. */
							if ((is_same || is_oper(u)) && (my_match_wild_nocase(host, u2->host) ||
									(u2->hostip && my_match_wild_nocase(host, u2->hostip)))) {
								delBan(ci, ban);
							}
						}
					}

					free(temp);
					free(nick);
					free(uname);
					free(host);
				}
			}
			if (!is_same && !is_oper(u))
				noticeLang(ci->bi->nick, u, LANG_UNBAN_NOTE, u2->nick);

		} else {
			char *alt_target;

			/* If we get a *!*@* target we don't need to do anything...
			 * in all other cases, 	autocomplete so we can remove bans set by !b and autocompleted. */
			if (my_match_wild_nocase("*!*@*", params))
				alt_target = NULL;
			else if (my_match_wild_nocase("*@*", params)) {
				char buf[BUFSIZE];
				snprintf(buf, BUFSIZE, "*!%s", params);;
				alt_target = sstrdup(buf);
			} else {
				char buf[BUFSIZE];
				snprintf(buf, BUFSIZE, "%s!*@*", params);
				alt_target = sstrdup(buf);
			}

			/* User doesn't exist, we compare all bans to the given description and a completed description... */
			for (ban = c->bans->entries; ban; ban = next) {
				next = ban->next;
				if (my_match_wild_nocase(params, ban->mask) || (alt_target &&
						my_match_wild_nocase(alt_target, ban->mask))) {
					delBan(ci, ban);
				} else
				/* If it s Unreal or InspIRCd, also check extended bantypes...  */
				if (!stricmp(IRCDModule, "unreal32")) {
					char eban[BUFSIZE];
					snprintf(eban, BUFSIZE, "??:%s", params);

					if (my_match_wild("??:*!*@*", ban->mask) && my_match_wild_nocase(eban, ban->mask)) {
						delBan(ci, ban);
					}
				} else if (!stricmp(IRCDModule, "inspircd12") || !stricmp(IRCDModule, "inspircd20")) {
					char eban[BUFSIZE];
					snprintf(eban, BUFSIZE, "?:%s", params);

					if (my_match_wild("?:*!*@*", ban->mask) && my_match_wild_nocase(eban, ban->mask)) {
						delBan(ci, ban);
					}
				}
			}

			if (alt_target) free(alt_target);
		}
	}
	return MOD_CONT;
}
#endif

/* EOF */
