/**
 * Provides staff listing functions. - Source
 *
 ***********
 * Module Name    : bs_fantasy_ext
 * Author         : Viper <Viper@Anope.org>
 * Creation Date  : 24/12/2008
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
 
#ifdef ENABLE_STAFF
#include "list.h"

int list_global_opers(User * u) {
	int j, i , carryon, count = 0;
	User *next;
	User *u2;
	char *access;

	noticeLang(s_OperServ, u, LANG_GOLIST_HEADER);
	for (j = 0; j < 1024; j++) {
		for (u2 = userlist[j]; u2; u2 = next) {
			next = u2->next;
			carryon = 0;

			/* Prevent listing of users with +H */
			if (finduser((u2->nick)) && !has_umode_H(u2) && !has_umode_B(u2) && !is_ulined(u->server->name)) {
				i = 0;
				while (i < excempt_nr) {
					if (!ListExempts[i] || !u2->nick)
						break;
					if (my_match_wild_nocase(ListExempts[i], u2->nick)) {
						carryon = 1;
						break;
					}
					i++;
				}

				if (carryon)
					continue;

				if (is_oper(u2)) {
					count++;
					access = getLangString(u, LANG_GOLIST_OPER_ONLY);
					if (is_services_oper(u2))
						access = getLangString(u, LANG_GOLIST_OPER_AND_SO);
					if (is_services_admin(u2))
						access = getLangString(u, LANG_GOLIST_OPER_AND_SA);
					if (is_services_root(u2))
						access = getLangString(u, LANG_GOLIST_OPER_AND_SRA);
					notice(s_OperServ, u->nick, "%-15s  -  %s", u2->nick, access);
				}
			}
		}
	}

	if (count == 0)
		noticeLang(s_OperServ, u, LANG_GOLIST_NONE);
	noticeLang(s_OperServ, u, LANG_GOLIST_FOOTER, count);

	return MOD_CONT;
}


int list_admins(User * u) {
	int j, i , carryon, count = 0;
	User *next;
	User *u2;

	noticeLang(s_OperServ, u, LANG_ADLIST_HEADER);
	for (j = 0; j < 1024; j++) {
		for (u2 = userlist[j]; u2; u2 = next) {
			next = u2->next;
			carryon = 0;

			/* Prevent listing of users with +H */
			if (finduser((u2->nick)) && !has_umode_H(u2) && !has_umode_B(u2) && !is_ulined(u->server->name)) {
				i = 0;
				while (i < excempt_nr) {
					if (!ListExempts[i] || !u2->nick)
						break;
					if (!stricmp(u2->nick, ListExempts[i]))
						carryon = 1;
					i++;
				}

				if (carryon)
					continue;

				if (is_oper(u2)) {
					if (is_services_root(u2)) {
						count++;
						notice(s_OperServ, u->nick, "%-15s  -  %s", u2->nick, getLangString(u, LANG_ADLIST_SRA));
						continue;
					}
					if (is_services_admin(u2)) {
						count++;
						notice(s_OperServ, u->nick, "%-15s  -  %s", u2->nick, getLangString(u, LANG_ADLIST_SA));
					}
				}
			}
		}
	}

	if (count == 0)
		noticeLang(s_OperServ, u, LANG_ADLIST_NONE);
	noticeLang(s_OperServ, u, LANG_ADLIST_FOOTER, count);

	return MOD_CONT;
}
#endif

/* EOF */
