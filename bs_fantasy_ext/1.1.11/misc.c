/**
 * Misc methodes used by the modules - Source
 * Misc commands the module adds - Source
 *
 ***********
 * Module Name: bs_fantasy_ext
 * Author: Viper <Viper@Absurd-IRC.net>
 * Creation Date: 21/07/2006
 * More info on http://forum.anope.org/index.php
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
 * Last Updated: 12/08/2006
 *
 **/

#include "misc.h"
#include "version.h"

static int check_opercmds(User *u) {
// 0 - Disabled
// 1 - Enabled for Services Root Administrators
// 2 - Enabled for Services Admins (and above)
// 3 - Enabled for Services Opers (and above)

	if (!(EnOperCmds))
		return 0;

	if (EnOperCmds == 0)
		return 0;

	if (EnOperCmds == 1) {
		if (is_services_root(u))
			return 1;

	} else if (EnOperCmds == 2) {
		if (is_services_admin(u))
			return 1;

	} else if (EnOperCmds == 3)
		if (is_services_oper(u))
			return 1;

	return 0;
}

static int my_check_access(User *u, ChannelInfo *ci, int what) {
	if (check_access(u, ci, what))
		return 1;
	else {
		if (is_services_admin(u) && SAdminOverride)
			return 1;
	}

	return 0;
}


static int do_core_fantasy(int ac, User *u, ChannelInfo *ci, char *cmd, char *params) {
	CSModeUtil *util = csmodeutils;
	do {
		if (stricmp(cmd, util->bsname) == 0) {
			if (ac != 3) {
				int count = 0, counter = 0, newac = 0;
				char *end1, *end2;
				char modes[BUFSIZE], nicks[BUFSIZE], buf[BUFSIZE];
				char **newav = scalloc(sizeof(char *) * 9, 1);
				end1 = modes;
				end2 = nicks;
				*end1 = 0;
				*end2 = 0;
				newav[newac] = ci->name;
				newac++;
				if (ircdcap->tsmode) {
					snprintf(buf, BUFSIZE - 1, "%ld", (long int) time(NULL));
					newav[newac] = buf;
					newac++;
				}

				if (!check_access(u, ci, util->level))
					return MOD_CONT;

				if (!ircd->halfop) {
					if (!stricmp(cmd, "halfop") || !stricmp(cmd, "dehalfop")) {
						return MOD_CONT;
					}
				}

				// fixed a few serious memory leaks here - Thx to Rob and Trystan
				while (counter < 6) {
					char *tnick = myStrGetToken(params, ' ', counter++);

					if (tnick == NULL)
						break;

					else {
						char *mode = util->mode;
						User *target = finduser(tnick);
						if (!target || !is_on_chan(ci->c, target))
							continue;

						if (ircd->protectedumode) {
							if (is_protected(target) && *mode == '-' && (stricmp(u->nick, target->nick))) {
								continue;
							}
						}

						if (*mode == '-' && (ci->flags & CI_PEACE) && stricmp(u->nick, target->nick)
						&& (get_access(target, ci) >= get_access(u, ci)))
							continue;

						end1 += snprintf(end1, sizeof(modes) - (end1 - modes), "%s", util->mode);
						end2 += snprintf(end2, sizeof(nicks) - (end2 - nicks), "%s ", target->nick);
						newav[newac+1] = target->nick;
						newac++;
						count++;
					}
					free(tnick);
				}

				if (ircdcap->tsmode)
					newav[2] = modes;
				else
					newav[1] = modes;
				newac++;

				anope_cmd_mode(ci->bi->nick, ci->name, "%s %s", modes, nicks);
				do_cmode(ci->bi->nick, newac, newav);

				/* free everything we ve used */
				free(newav);

			} else {
				if (check_access(u, ci, util->levelself)) {
					bot_raw_mode(u, ci, util->mode, u->nick);
				}
			}

			return MOD_STOP;
		}
	} while ((++util)->name != NULL);

	return MOD_CONT;
}


/**
 * Handle the up and down requests that give a user all the modes they can have (up)
 * or remove all modes they have (down).
 **/
static int do_up_down(User *u, Channel *c, char *cmd) {
	if (!stricmp(cmd, "up")) {
			chan_set_correct_modes(u, c, 1);
	}

	if (!stricmp(cmd, "down")) {
		int i = 0, count = 0;
		char *mode[5], *param[5], *temp;

		if (ircd->owner) {
			if (chan_has_user_status(c, u, CUS_OWNER)) {
				// another memory leak fixed here - Thx to Trystan
				temp = stripModePrefix(ircd->ownerset);
				mode[0] = sstrdup((temp ? temp : ""));
				param[0] = sstrdup(u->nick);
				count++;
				if (temp) free(temp);
			} else {
				mode[0] = sstrdup("");
				param[0] = sstrdup("");
			}
		} else {
			mode[0] = sstrdup("");
			param[0] = sstrdup("");
		}

		if (ircd->admin || ircd->protect) {
			if (chan_has_user_status(c, u, CUS_PROTECT)) {
				temp = stripModePrefix(ircd->adminset);
				mode[1] = sstrdup((temp ? temp : ""));
				param[1] = sstrdup(u->nick);
				count++;
				if (temp) free(temp);
			} else {
				mode[1] = sstrdup("");
				param[1] = sstrdup("");
			}
		} else {
			mode[1] = sstrdup("");
			param[1] = sstrdup("");
		}

		if (chan_has_user_status(c, u, CUS_OP)) {
			mode[2] = sstrdup("o");
			param[2] = sstrdup(u->nick);
			count++;
		} else {
			mode[2] = sstrdup("");
			param[2] = sstrdup("");
		}

		if (ircd->halfop) {
			if (chan_has_user_status(c, u, CUS_HALFOP)) {
				mode[3] = sstrdup("h");
				param[3] = sstrdup(u->nick);
				count++;
			} else {
				mode[3] = sstrdup("");
				param[3] = sstrdup("");
			}
		} else {
			mode[3] = sstrdup("");
			param[3] = sstrdup("");
		}

		if (chan_has_user_status(c, u, CUS_VOICE)) {
				mode[4] = sstrdup("v");
				param[4] = sstrdup(u->nick);
				count++;
		} else {
			mode[4] = sstrdup("");
			param[4] = sstrdup("");
		}

		if (count > 0) {
			// when you scalloc an array you need to free the array memory when
			//done, not just its members - Thx Trystan
			char **av = scalloc(sizeof(char *) * (count + 3), 1);
			char tmp[BUFSIZE], buf[BUFSIZE];
			int ac;

			if (debug)
				alog("Setting mode: %s on %s : -%s%s%s%s%s %s %s %s %s %s", c->ci->bi->nick, c->name, mode[0],
					mode[1], mode[2], mode[3], mode[4], param[0], param[1], param[2], param[3], param[4]);

			anope_cmd_mode(c->ci->bi->nick, c->name,"-%s%s%s%s%s %s %s %s %s %s ", mode[0], mode[1], mode[2],
				mode[3], mode[4], param[0], param[1], param[2], param[3], param[4]);

			av[0] = c->name;
			snprintf(tmp, BUFSIZE, "-%s%s%s%s%s", mode[0], mode[1], mode[2], mode[3], mode[4]);

			if (ircdcap->tsmode) {
				snprintf(buf, BUFSIZE - 1, "%ld", (long int) time(NULL));
				av[1] = buf;
				av[2] = tmp;
				/* We have to send as much nicks as modes, every nick takes a new place in the array */
				for ( i = 3; i < count + 3; i++) {
					av[i] = u->nick;
				}
				ac = count + 3;

			} else {
				av[1] = tmp;
				/* We have to send as much nicks as modes, every nick takes a new place in the array */
				for ( i = 2; i < count + 2; i++) {
					av[i] = u->nick;
				}
				ac = count + 2;
			}

			do_cmode(c->ci->bi->nick, ac, av);

			/* free everything we ve used */
			free(av);
		}

		for (i = 0; i < 5; i++) {
			if (mode[i]) free(mode[i]);
			if (param[i]) free(param[i]);
		}
	}

	return MOD_CONT;
}


/**
 * (Re)Load the configuration directives
 **/
void load_config(void) {
	int i, saoverride, overridecore;
	char *excempt_nicks;

	Directive confvalues[][1] = {
		{{ "ListExcempNick", { { PARAM_STRING, PARAM_RELOAD, &excempt_nicks } } }},
		{{ "EnOperCmds", { { PARAM_POSINT, PARAM_RELOAD, &EnOperCmds } } }},
		{{ "SAdminOverride", { { PARAM_SET, PARAM_RELOAD, &saoverride } } }},
		{{ "OverrideCoreCmds", { { PARAM_SET, PARAM_RELOAD, &overridecore } } }}
	};

    // free allready existing GOListExcempts
	for ( i = 0; i < excempt_nr; i++)
		if (ListExcempts[i])
			free(ListExcempts[i]);

	excempt_nicks = NULL;
	excempt_nr = 0;
	ListExcempts = NULL;
	EnOperCmds = 0;

	for (i = 0; i < 4; i++)
    	moduleGetConfigDirective(confvalues[i]);

	// Thx to Trystan for making this part a bit more efficient
	if ((excempt_nicks)) {
		ListExcempts = buildStringList(excempt_nicks, &excempt_nr);
		if (debug)
			for (i = 0; i < excempt_nr; i++)
				alog("debug: [bs_fantasy_ext] Added '%s' to Excempt list", ListExcempts[i]);
	}

	if (!((EnOperCmds >= 0) && (EnOperCmds <= 3))) {
		alog("[\002bs_fantasy_ext\002] Invalid EnOperCmds Option: %d", EnOperCmds);
		EnOperCmds = 0;
	}

	if (saoverride)
		SAdminOverride = 1;
	else
		SAdminOverride = 0;

	if (overridecore)
		OverrideCoreCmds = 1;
	else
		OverrideCoreCmds = 0;

	if (debug)
		alog("debug: [bs_fantasy_ext] Set the Enable oper fantasy commands to: %d", EnOperCmds);

	if (debug)
		alog("debug: [bs_fantasy_ext] %s Services Admin fantasy override.", (SAdminOverride ? "Enabled":"Disabled"));

	// need to clear what moduleGetConfigDirective fills - Thx to Trystan
	if (excempt_nicks)
		free(excempt_nicks);
}


/**
 * Upon /os reload call the routines for reloading the configuration directives
 **/
int reload_config(int argc, char **argv) {
	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			alog("[\002bs_fantasy_ext\002]: Reloading configuration directives...");
			load_config();
		}
	}
	return MOD_CONT;
}


/**
 * Sends the user the version of services.
 **/
static void show_version(User *u, ChannelInfo *ci) {
	const char version_number[] = VERSION_STRING;
	const char version_build[] = "build #" BUILD ", compiled " __DATE__ " " __TIME__;

	notice(ci->bi->nick, u->nick, "Anope version %s %s %s", version_number, version_build, version_flags);
	notice(ci->bi->nick, u->nick, "More info on: http://www.anope.org");
}


/**
 * List IRC-Operators
 **/
#ifdef ENABLE_STAFF
int list_global_opers(User * u) {
    int j, i , carryon, count = 0;
    User *next;
    User *u2;
    char *access;

	moduleNoticeLang(s_OperServ, u, LANG_GOLIST_HEADER);

	for (j = 0; j < 1024; j++) {
		for (u2 = userlist[j]; u2; u2 = next) {
			next = u2->next;
			carryon = 0;

			if (finduser((u2->nick))) {
				i = 0;

				while (i < excempt_nr) {
					if (!ListExcempts[i] || !u2->nick)
						break;

					if (match_wild_nocase(ListExcempts[i], u2->nick)) {
						carryon = 1;
						break;
					}

					i++;
				}

				if (carryon)
					continue;

				if (is_oper(u2)) {
					count++;
					access = moduleGetLangString(u, LANG_GOLIST_OPER_ONLY);
					if (is_services_oper(u2))
						access = moduleGetLangString(u, LANG_GOLIST_OPER_AND_SO);
					if (is_services_admin(u2))
						access = moduleGetLangString(u, LANG_GOLIST_OPER_AND_SA);
					if (is_services_root(u2))
						access = moduleGetLangString(u, LANG_GOLIST_OPER_AND_SRA);

					notice(s_OperServ, u->nick, "%-15s  -  %s", u2->nick, access);
				}
			}

		}

	}
	if (count == 0)
		moduleNoticeLang(s_OperServ, u, LANG_GOLIST_NONE);
	moduleNoticeLang(s_OperServ, u, LANG_GOLIST_FOOTER, count);

	return MOD_CONT;
}

int list_admins(User * u) {
    int j, i , carryon, count = 0;
    User *next;
    User *u2;

	moduleNoticeLang(s_OperServ, u, LANG_ADLIST_HEADER);

	for (j = 0; j < 1024; j++) {
		for (u2 = userlist[j]; u2; u2 = next) {
			next = u2->next;
			carryon = 0;

			if (finduser((u2->nick))) {
				i = 0;

				while (i < excempt_nr) {
					if (!ListExcempts[i] || !u2->nick)
						break;

					if (!stricmp(u2->nick, ListExcempts[i]))
						carryon = 1;

					i++;
				}

				if (carryon)
					continue;

				if (is_oper(u2)) {
					if (is_services_root(u2)) {
						count++;
						notice(s_OperServ, u->nick, "%-15s  -  %s", u2->nick, moduleGetLangString(u, LANG_ADLIST_SRA));
						continue;
					}
					if (is_services_admin(u2)) {
						count++;
						notice(s_OperServ, u->nick, "%-15s  -  %s", u2->nick, moduleGetLangString(u, LANG_ADLIST_SA));
					}
				}
			}

		}

	}
	if (count == 0)
		moduleNoticeLang(s_OperServ, u, LANG_ADLIST_NONE);
	moduleNoticeLang(s_OperServ, u, LANG_ADLIST_FOOTER, count);

	return MOD_CONT;
}
#endif


/**
 * The info command.
 **/
#ifdef ENABLE_INFO
int do_info(User * u, Channel *c, char *param) {
	ChannelInfo *ci = c->ci;
	char buf[BUFSIZE], *end;
	struct tm *tm;
	int need_comma = 0;
	const char *commastr = getstring(u->na, COMMA_SPACE);
	int is_servadmin = is_services_admin(u);
	int show_all = 0;
	time_t expt;


	if (ci->flags & CI_VERBOTEN) {
		if (is_oper(u) && ci->forbidby)
			notice_lang(ci->bi->nick, u, CHAN_X_FORBIDDEN_OPER, ci->name, ci->forbidby, (ci->forbidreason ? ci->
			forbidreason : getstring(u->na, NO_REASON)));
		else
			notice_lang(ci->bi->nick, u, CHAN_X_FORBIDDEN, ci->name);
	} else if (!ci->founder) {
		/* Paranoia... this shouldn't be able to happen */
		delchan(ci);
		notice_lang(ci->bi->nick, u, CHAN_X_NOT_REGISTERED, ci->name);
	} else {
		/* Should we show all fields? Only for sadmins and identified users */
		if (param && stricmp(param, "ALL") == 0 && (check_access(u, ci, CA_INFO) || is_servadmin))
			show_all = 1;

		notice_lang(ci->bi->nick, u, CHAN_INFO_HEADER, ci->name);
		notice_lang(ci->bi->nick, u, CHAN_INFO_NO_FOUNDER, ci->founder->display);

		if (show_all && ci->successor)
			notice_lang(ci->bi->nick, u, CHAN_INFO_NO_SUCCESSOR, ci->successor->display);

		notice_lang(ci->bi->nick, u, CHAN_INFO_DESCRIPTION, ci->desc);
		tm = localtime(&ci->time_registered);
		strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
		notice_lang(ci->bi->nick, u, CHAN_INFO_TIME_REGGED, buf);
		tm = localtime(&ci->last_used);
		strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
		notice_lang(ci->bi->nick, u, CHAN_INFO_LAST_USED, buf);
		if (ci->last_topic && (show_all || (!(ci->mlock_on & anope_get_secret_mode()) &&
		(!ci->c || !(ci->c->mode & anope_get_secret_mode()))))) {
			notice_lang(ci->bi->nick, u, CHAN_INFO_LAST_TOPIC, ci->last_topic);
			notice_lang(ci->bi->nick, u, CHAN_INFO_TOPIC_SET_BY, ci->last_topic_setter);
		}

		if (ci->entry_message && show_all)
			notice_lang(ci->bi->nick, u, CHAN_INFO_ENTRYMSG, ci->entry_message);
		if (ci->url)
			notice_lang(ci->bi->nick, u, CHAN_INFO_URL, ci->url);
		if (ci->email)
			notice_lang(ci->bi->nick, u, CHAN_INFO_EMAIL, ci->email);

		if (show_all) {
			notice_lang(ci->bi->nick, u, CHAN_INFO_BANTYPE, ci->bantype);

			end = buf;
			*end = 0;
			if (ci->flags & CI_KEEPTOPIC) {
				end += snprintf(end, sizeof(buf) - (end - buf), "%s", getstring(u->na, CHAN_INFO_OPT_KEEPTOPIC));
				need_comma = 1;
			}
			if (ci->flags & CI_OPNOTICE) {
				end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
				need_comma ? commastr : "", getstring(u->na, CHAN_INFO_OPT_OPNOTICE));
				need_comma = 1;
			}
			if (ci->flags & CI_PEACE) {
				end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
				need_comma ? commastr : "", getstring(u->na, CHAN_INFO_OPT_PEACE));
				need_comma = 1;
			}
			if (ci->flags & CI_PRIVATE) {
				end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
				need_comma ? commastr : "", getstring(u->na, CHAN_INFO_OPT_PRIVATE));
				need_comma = 1;
			}
			if (ci->flags & CI_RESTRICTED) {
				end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
				need_comma ? commastr : "", getstring(u->na, CHAN_INFO_OPT_RESTRICTED));
				need_comma = 1;
			}
			if (ci->flags & CI_SECURE) {
				end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
				need_comma ? commastr : "", getstring(u->na, CHAN_INFO_OPT_SECURE));
				need_comma = 1;
			}
			if (ci->flags & CI_SECUREOPS) {
				end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
				need_comma ? commastr : "", getstring(u->na, CHAN_INFO_OPT_SECUREOPS));
				need_comma = 1;
			}
			if (ci->flags & CI_SECUREFOUNDER) {
				end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
				need_comma ? commastr : "", getstring(u->na, CHAN_INFO_OPT_SECUREFOUNDER));
				need_comma = 1;
			}
			if ((ci->flags & CI_SIGNKICK) || (ci->flags & CI_SIGNKICK_LEVEL)) {
				end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
				need_comma ? commastr : "", getstring(u->na, CHAN_INFO_OPT_SIGNKICK));
				need_comma = 1;
			}
			if (ci->flags & CI_TOPICLOCK) {
				end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
				need_comma ? commastr : "", getstring(u->na, CHAN_INFO_OPT_TOPICLOCK));
				need_comma = 1;
			}
			if (ci->flags & CI_XOP) {
				end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
				need_comma ? commastr : "", getstring(u->na, CHAN_INFO_OPT_XOP));
				need_comma = 1;
			}
			notice_lang(ci->bi->nick, u, CHAN_INFO_OPTIONS, *buf ? buf : getstring(u->na, CHAN_INFO_OPT_NONE));
			notice_lang(ci->bi->nick, u, CHAN_INFO_MODE_LOCK, get_mlock_modes(ci, 1));

		}
		if (show_all) {
			if (ci->flags & CI_NO_EXPIRE) {
				notice_lang(ci->bi->nick, u, CHAN_INFO_NO_EXPIRE);
			} else {
				if (is_servadmin) {
					expt = ci->last_used + CSExpire;
					tm = localtime(&expt);
					strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
					notice_lang(ci->bi->nick, u, CHAN_INFO_EXPIRE, buf);
				}
			}
		}
		if (ci->flags & CI_SUSPENDED) {
			notice_lang(ci->bi->nick, u, CHAN_X_SUSPENDED, ci->forbidby,
			(ci->forbidreason ? ci-> forbidreason : getstring(u->na, NO_REASON)));
		}
	}
	return MOD_CONT;
}
#endif

/* EOF */
