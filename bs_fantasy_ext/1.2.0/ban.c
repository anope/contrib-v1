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
 * Last Updated   : 11/01/2009
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

			av[0] = sstrdup("+b");
			get_idealban(ci, u2, mask, sizeof(mask));
			av[1] = sstrdup(mask);
			anope_cmd_mode(ci->bi->nick, c->name, "+b %s", av[1]);
			chan_set_modes(ci->bi->nick, c, 2, av, 1);

			if (av[0]) free(av[0]);
			if (av[1]) free(av[1]);
		}
	/* It could be a hostmask.. set whatever we get */
	} else {
		struct c_userlist *cu = NULL;
		struct c_userlist *next;
		char *target;
		int ok = 0;

		/* If we get a *!*@* target we don't need to do anything... */
		if (my_match_wild_nocase("*!*@*", params)) {
			ok = 1;
			target = sstrdup(params);
		} else if (my_match_wild_nocase("*@*", params)) {
			char buf[BUFSIZE];
			snprintf(buf, BUFSIZE, "*!%s", params);;
			target = sstrdup(buf);
			ok = 1;
		} else {
			char buf[BUFSIZE];
			snprintf(buf, BUFSIZE, "%s!*@*", params);
			target = sstrdup(buf);
			ok = 1;
		}

#if CPU_USAGE_REDUCTION < 10
		cu = c->users;

		/* Check to make sure were are not going to ban an admin here... */
		while ((cu) && ok) {
			next = cu->next;
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
				} else if (RestrictKB && ((!is_founder(u, ci) && is_services_oper(cu->user)) ||
						(is_founder(u, ci) && is_services_admin(cu->user)))) {
					notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
					ok = 0;
				}
			}
			cu = next;
		}
#endif
		if (ok) {
			av[0] = "+b";
			av[1] = target;

			anope_cmd_mode(ci->bi->nick, c->name, "+b %s", av[1]);
			chan_set_modes(ci->bi->nick, c, 2, av, 1);

#if CPU_USAGE_REDUCTION < 5
			cu = c->users;

			while ((cu) && ok) {
				next = cu->next;
				if (match_usermask(target, cu->user))
					do_up_down(cu->user, c, "down", 0);
				cu = next;
			}
#endif
		}

		free(target);
	}

	return MOD_CONT;
}
#endif


#ifdef ENABLE_UNBAN
/**
 * We attempt to unban the user without the use of anope_cmd_unban() which uses svsmode unban
 * and can potentially be used to "guess" a users' real mask/IP.
 * When we unban ourself we still use anope_cmd_unban().
 *
 * To avoid this, we go over the banlist ourselves and check whether it s considered
 * safe to remove matching bans:
 *    - Only bans on vhosts and cloaked hosts/IPs (only if no vhost is set) are not affected by added restrictions.
 *      This means that bans on cloaked masks will not be removed if the user has a vhost set even though they will match.
 *      As anope 1.8 only stores 3 things: host, IP and cloaked host or vhost. So we match against either vhost or
 *       cloacked host, depending on what the users' visible mas currently is.
 *      Matching against real host and IP falls under added restrictions.
 *    - If the nickserv SET HIDE USERMASK option is enabled, we will not remove host/IP bans matching the user.
 *    - !unban may not remove bans matching users host/IP when the target user is not identified.
 *    - !unban may not remove more then MaxUnbanIP (defaults to 3) host/IP bans on an identified user
 *      unless his IP has changed.
 *
 * Note that the last option does not store the data between services restarts so there is stil
 * some room for potential abuse of the !unban command. This would however require services to go down
 * a few times in a relatively short timespan.
 * At a later time, I may look into adding a db backend to tackle this problem.
 * Because we cannot provide a 100% guarantee an EnUnbanIP directive has been added to services.conf.
 *
 * Do keep in mind this may not find ALL bans matching a user...
 * and even if it finds all, it may not remove all..
 **/
int do_unban(User * u, Channel *c, char *params) {
	Entry *ban, *next;
	ChannelInfo *ci = c->ci;
	User *u2;
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
			/* Make sure we have IP for the user... */
			if (!u2->hostip)
				u2->hostip = host_resolve(u2->host);

			/* We can't use SVSMODE UNBAN because it might give the users' IP away, so we
			 * do it ourselves... */
			for (ban = c->bans->entries; ban; ban = next) {
				char *nick = NULL, *mask = NULL, *uname = NULL, *host = NULL, *temp = NULL;
				int type = 0;
				next = ban->next;

				/* If it s UnrealIRCd, check whether it s an extended ban...  */
				if (!stricmp(IRCDModule, "unreal32")) {
					if (my_match_wild_nocase("~r:*", ban->mask)) {
						nick = myStrGetToken(ban->mask, ':', 1);
						type = 3;
					} else if (my_match_wild_nocase("??:*!*@*", ban->mask)) {
						temp = myStrGetToken(ban->mask, ':', 1);
						type = 2;
					} else
						type = 1;
				} else
					type = 1;

				if (type == 1)
					temp = ban->mask;

				if (temp) {
					split_usermask(temp, &nick, &uname, &host);
				}

				if (type == 2)
					free(temp);

				if (my_match_wild_nocase(nick, u2->nick)) {
					if (type == 3) {
						delBan(ci, ban);
					} else if ((my_match_wild_nocase(uname, u2->username) ||
							(u2->vident && my_match_wild_nocase(uname, u2->vident)))) {
						/* Match against vhost... 
						 * This should always match against the host currently visible to the end-users. */
						if (u2->vhost && my_match_wild_nocase(host, u2->vhost)) {
							delBan(ci, ban);
						}

						/* Only look for other matches if EnUnbanIP is enabled or we are unbanning ourselves..
						 * Watch out because this is where we have to make sure we don't give away the users' IP.. */
						else if ((EnUnbanIP || is_same) && (my_match_wild_nocase(host, u2->host) || 
								(u2->hostip && my_match_wild_nocase(host, u2->hostip)))) {
							/* Only continue if target is identified and NS SET HIDE USERMASK is turned OFF.
							 * Even when unbanning ourselves, check this as extra precaution.. */
							if (u2->na && u2->na->nc && !(u2->na->nc->flags & NI_HIDE_MASK)) {
								char *PrevIP, *temp;
								int count = 0, same = 0;

								/* Check whether IP has changed.. */
								PrevIP = moduleGetData(&u2->na->nc->moduleData, PrevUnbanIP);
								if (!is_same && PrevIP && !stricmp(PrevIP, u2->hostip)) {
									same = 1;
									temp = moduleGetData(&u2->na->nc->moduleData, NrUnbanIP);
									if (temp) {
										count = atoi(temp);
										free(temp);
									}
								}

								/* Only continue if it hasn't been used twice already.. */
								if (count < MaxUnbanIP) {
									char buf[BUFSIZE];
									count++;
									snprintf(buf, BUFSIZE, "%d", count);
									delBan(ci, ban);

									/* If we weren't unbanning ourselves, update count.. */
									if (!is_same) {
										if (same) {
											moduleAddData(&u2->na->nc->moduleData, NrUnbanIP, buf);
										} else {
											moduleAddData(&u2->na->nc->moduleData, PrevUnbanIP, u2->hostip);
											moduleAddData(&u2->na->nc->moduleData, NrUnbanIP, buf);
										}
									}
								}
							}
						}
					}
					free(nick);
					free(mask);
					free(uname);
					free(host);
				}
			}
			if (!is_same) noticeLang(ci->bi->nick, u, LANG_UNBAN_NOTE, u2->nick);

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
				/* If it s Unreal, also check extended bantypes...
				 * I should look whether any other ircds have stuff like this...  */
				if (!stricmp(IRCDModule, "unreal32")) {
					char eban[BUFSIZE], rban[BUFSIZE];

					snprintf(eban, BUFSIZE, "??:%s", params);
					snprintf(rban, BUFSIZE, "~r:%s", params);

					if (my_match_wild_nocase("??:*!*@*", ban->mask)
							&& my_match_wild_nocase(eban, ban->mask)) {
						delBan(ci, ban);
					} else if (my_match_wild_nocase("~r:*", ban->mask) &&
							my_match_wild_nocase(rban, ban->mask)) {
						delBan(ci, ban);
					}
				}
			}
		}
	}
	return MOD_CONT;
}
#endif

/* EOF */
