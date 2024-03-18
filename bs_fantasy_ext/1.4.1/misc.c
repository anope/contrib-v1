/**
 * Misc methodes used by the modules - Source
 * Misc commands the module adds or overrides - Source
 *
 ***********
 * Module Name    : bs_fantasy_ext
 * Author         : Viper <Viper@Anope.org>
 * Creation Date  : 21/07/2006
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
 * Last Updated   : 31/05/2013
 *
 **/

#include "misc.h"
#include "version.h"

static int check_opercmds(User *u) {
/* 0 - Disabled
 * 1 - Enabled for Services Root Administrators
 * 2 - Enabled for Services Admins (and above)
 * 3 - Enabled for Services Opers (and above) */

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

/* ------------------------------------------------------------------------------- */

static int do_core_fantasy(int ac, User *u, ChannelInfo *ci, char *cmd, char *params) {
	CSModeUtil *util = csmodeutils;
	do {
		if (stricmp(cmd, util->bsname) == 0) {
			if (ac != 3) {
				int count = 0, counter = 0, newac = 0;
				char *end1, *end2, *tnick, *mode;
				char modes[BUFSIZE], nicks[BUFSIZE], buf[BUFSIZE];
				char **newav = scalloc(sizeof(char *) * 13, 1);
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

				if (!check_access(u, ci, util->level)) {
					free(newav);
					return MOD_CONT;
				}

				if (!ircd->halfop) {
					if (!stricmp(cmd, "halfop") || !stricmp(cmd, "dehalfop")) {
						free(newav);
						return MOD_CONT;
					}
				}

				mode = util->mode;
				while ((tnick = myStrGetToken(params, ' ', counter++))) {
					User *target;

					/* If it's a nickname.. */
					if ((target = finduser(tnick))) {
						if (!is_on_chan(ci->c, target)) {
							free(tnick);
							continue;
						}

						if (ircd->protectedumode) {
							if (is_protected(target) && *mode == '-' && (stricmp(u->nick, target->nick))) {
								free(tnick);
								continue;
							}
						}

						if (*mode == '-' && (ci->flags & CI_PEACE) && stricmp(u->nick, target->nick)
								&& (get_access(target, ci) >= get_access(u, ci))) {
							free(tnick);
							continue;
						}

						end1 += snprintf(end1, sizeof(modes) - (end1 - modes), "%s", util->mode);
						end2 += snprintf(end2, sizeof(nicks) - (end2 - nicks), "%s ", GET_USER(target));
						newav[newac+1] = GET_USER(target);
						newac++;
						count++;

						/* Check whether the command hasn't grown too long yet.
						 * We don't allow more then 10 mode changes per command.. this should keep us safely below the 512 max length per cmd. */
						if (count == 10) {
							/* We've reached the maximum.. send the mode changes. */
							if (ircdcap->tsmode)
								newav[2] = modes;
							else
								newav[1] = modes;
							newac++;

							anope_cmd_mode(ci->bi->nick, ci->name, "%s %s", modes, nicks);
							do_cmode(ci->bi->nick, newac, newav);

							/* Reset the buffer for the next set of changes.. */
							if (ircdcap->tsmode)
								newac = 2;
							else
								newac = 1;
							end1 = modes;
							end2 = nicks;
							*end1 = 0;
							*end2 = 0;
							count = 0;
						}

					/* It s no nick, it s a mask.. */
					} else {
						char mask[BUFSIZE];
						struct c_userlist *cu = NULL;

						/* Complete the mask... */
						if (my_match_wild_nocase("*!*@*", tnick))
							snprintf(mask, BUFSIZE, "%s", tnick);
						else if (my_match_wild_nocase("*@*", tnick))
							snprintf(mask, BUFSIZE, "*!%s", tnick);
						else
							snprintf(mask, BUFSIZE, "%s!*@*", tnick);

						/* Go over all users in the channel and add those matching the mask to the list.. */
						cu = ci->c->users;
						while (cu) {
							/* This only checks against the cloacked host & vhost for normal users.
							 * IPs are only checked when triggered by an oper.. */
							if (is_oper(u) ? match_usermask_full(mask, cu->user, true) : match_usermask(mask, cu->user)) {
								if (ircd->protectedumode) {
									if (is_protected(cu->user) && *mode == '-' && (stricmp(u->nick, cu->user->nick))) {
										cu = cu->next;
										continue;
									}
								}

								if (*mode == '-' && (ci->flags & CI_PEACE) && stricmp(u->nick, cu->user->nick)
										&& (get_access(cu->user, ci) >= get_access(u, ci))) {
									cu = cu->next;
									continue;
								}

								end1 += snprintf(end1, sizeof(modes) - (end1 - modes), "%s", util->mode);
								end2 += snprintf(end2, sizeof(nicks) - (end2 - nicks), "%s ", GET_USER(cu->user));
								newav[newac+1] = GET_USER(cu->user);
								newac++;
								count++;
							}

							/* Check whether the command hasn't grown too long yet.
							 * We don't allow more then 10 mode changes per command.. this should keep us safely below the 512 max length per cmd. */
							if (count == 10) {
								/* We've reached the maximum.. send the mode changes. */
								if (ircdcap->tsmode)
									newav[2] = modes;
								else
									newav[1] = modes;
								newac++;

								anope_cmd_mode(ci->bi->nick, ci->name, "%s %s", modes, nicks);
								do_cmode(ci->bi->nick, newac, newav);

								/* Reset the buffer for the next set of changes.. */
								if (ircdcap->tsmode)
									newac = 2;
								else
									newac = 1;
								end1 = modes;
								end2 = nicks;
								*end1 = 0;
								*end2 = 0;
								count = 0;
							}

							cu = cu->next;
						}
					}
					free(tnick);
				}

				/* Make sure we actually have something to change... */
				if (count > 0) {
					if (ircdcap->tsmode)
						newav[2] = modes;
					else
						newav[1] = modes;
					newac++;

					anope_cmd_mode(ci->bi->nick, ci->name, "%s %s", modes, nicks);
					do_cmode(ci->bi->nick, newac, newav);
				}

				/* free everything we ve used */
				free(newav);

			} else {
				if (check_access(u, ci, util->levelself)) {
					bot_raw_mode(u, ci, util->mode, GET_USER(u));
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
 *
 * When issueing "down", it is possible to give a target level so for example an @ who uses
 * down on a moderated channel won't find himself unable to talk. He will get voice instead.
 * In addition it enables users to give a target level. An @ might want to have all modes
 * except voice removed.
 *
 * Legend for target and needed params:
 *   values   -   modes:
 *     0         no modes
 *     1          voice
 *     2          halfop
 *     3            op
 *     4       protected op / admin
 *     5         founder
 *
 * Note that no access levels are being checked here!!!
 **/
static int do_up_down(User *u, Channel *c, char *cmd, int target) {
	if (!u || !c || !cmd)
		return MOD_STOP;

	if (!stricmp(cmd, "up")) {
		uint32 flags = 0, cflags = 0;
		/* We will have to use a dirty hack to allow chan_set_correct_modes to work
		 * correctly. Other alternative is copying/rewriting it..
		 * Unlike what the name suggests, chan_set_correct_modes() is only suitable
		 * for use after a user joins. It checks whether autoop is on, if not, no modes
		 * are set.. but we don't want that with !up...
		 * We pretend like AUTOOP is on so it does its job. When it is finished we
		 * restore the original flags from our backup. */

		/* If not registered users have access to fantasia cmds and should gets ops, we should give it.
		 * They will not have a NickCore however... */
		if (u->na && u->na->nc) {
			flags = u->na->nc->flags;
			u->na->nc->flags &= ~NI_AUTOOP;
		}

		/* Turn secureops on so we actually remove modes people shouldn't have. */
		if (c->ci) {
			cflags = c->ci->flags;
			c->ci->flags |= CI_SECUREOPS;
		}

		chan_set_correct_modes(u, c, 1);

		/* Restore original settings.. */
		if (u->na && u->na->nc)
			u->na->nc->flags = flags;
		if (c->ci)
			c->ci->flags = cflags;
	}

	if (!stricmp(cmd, "down")) {
		int i = 0, count = 0, needed = target;
		char *mode[5], *param[5];

		for (i = 0; i < 5; i++) {
			if (mode[i]) mode[i] = NULL;
			if (param[i]) param[i] = NULL;
		}

		if (ircd->owner) {
			if (chan_has_user_status(c, u, CUS_OWNER)) {
				if (needed == 5)
					/* we need both +q and +o */
					needed = 3;
				else {
					/* another memory leak fixed here - Thx to Trystan */
					mode[0] = ircd->ownerunset;
					param[0] = GET_USER(u);
					count++;
				}
			} else {
				if (needed == 5) {
					mode[0] = ircd->ownerset;
					param[0] = GET_USER(u);
					needed = 3;
					count++;
				}
			}
		} else
			if (needed == 5)
				needed = 4;

		if (ircd->admin || ircd->protect) {
			if (chan_has_user_status(c, u, CUS_PROTECT)) {
				if (needed == 4)
					needed = 3;
				else {
					mode[1] = ircd->adminunset;
					param[1] = GET_USER(u);
					count++;
				}
			} else {
				if (needed == 4) {
					mode[1] = ircd->adminset;
					param[1] = GET_USER(u);
					needed = 3;
					count++;
				}
			}
		} else
			if (needed == 4)
				needed = 3;

		if (chan_has_user_status(c, u, CUS_OP)) {
			/* we don't really have to set needed to 0.. might as well delete this later on */
			if (needed == 3)
				needed = 0;
			else {
				mode[2] = "-o";
				param[2] = GET_USER(u);
				count++;
			}
		} else {
			if (needed == 3) {
				mode[2] = "+o";
				param[2] = GET_USER(u);
				needed = 0;
				count++;
			}
		}

		if (ircd->halfop) {
			if (chan_has_user_status(c, u, CUS_HALFOP)) {
				if (needed == 2)
					needed = 0;
				else {
					mode[3] = "-h";
					param[3] = GET_USER(u);
					count++;
				}
			} else {
				if (needed == 2) {
					mode[3] = "+h";
					param[3] = GET_USER(u);
					needed = 0;
					count++;
				}
			}
		} else
			if (needed == 2)
				needed = 1;

		if (chan_has_user_status(c, u, CUS_VOICE)) {
			if (needed == 1)
				needed = 0;
			else {
				mode[4] = "-v";
				param[4] = GET_USER(u);
				count++;
			}
		} else {
			if (needed == 1) {
				mode[4] = "+v";
				param[4] = GET_USER(u);
				needed = 0;
				count++;
			}
		}

		if (needed != 0)
			alog("[bs_fantasy_ext] Downing routing ended with non-zero! (Needed = %d)",needed);

		if (count > 0) {
			/* when you scalloc an array you need to free the array memory when
			 * done, not just its members - Thx Trystan */
			char **av = scalloc(sizeof(char *) * (count + 3), 1);
			char tmp[BUFSIZE], buf[BUFSIZE];
			int ac;

			if (debug)
				alog("[bs_fantasy_ext] Setting mode: %s on %s : %s%s%s%s%s %s %s %s %s %s", c->ci->bi->nick, c->name,
					get_str(mode[0]), get_str(mode[1]), get_str(mode[2]), get_str(mode[3]), get_str(mode[4]),
					get_str(param[0]), get_str(param[1]), get_str(param[2]), get_str(param[3]), get_str(param[4]));

			anope_cmd_mode(c->ci->bi->nick, c->name,"%s%s%s%s%s %s %s %s %s %s ", get_str(mode[0]), get_str(mode[1]),
				get_str(mode[2]), get_str(mode[3]), get_str(mode[4]), get_str(param[0]), get_str(param[1]),
				get_str(param[2]), get_str(param[3]), get_str(param[4]));;

			av[0] = c->name;
			snprintf(tmp, BUFSIZE, "%s%s%s%s%s", get_str(mode[0]), get_str(mode[1]), get_str(mode[2]),
				get_str(mode[3]), get_str(mode[4]));


			if (ircdcap->tsmode) {
				snprintf(buf, BUFSIZE - 1, "%ld", (long int) time(NULL));
				av[1] = buf;
				av[2] = tmp;
				/* We have to send as much nicks as modes, every nick takes a new place in the array */
				for ( i = 3; i < count + 3; i++) {
					av[i] = GET_USER(u);
				}
				ac = count + 3;

			} else {
				av[1] = tmp;
				/* We have to send as much nicks as modes, every nick takes a new place in the array */
				for ( i = 2; i < count + 2; i++) {
					av[i] = GET_USER(u);
				}
				ac = count + 2;
			}

			do_cmode(c->ci->bi->nick, ac, av);

			/* free everything we ve used */
			free(av);
		}
	}
	return MOD_CONT;
}

/**
 * Call do_up_down() for all users on given channel matching the given mask.
 **/
static void do_up_down_mask(User *u, Channel *c, char *cmd, int target, char *mask) {
	struct c_userlist *cu = NULL, *next = NULL;

	if (!u || !c || !mask)
		return;

	cu = c->users;
	while (cu) {
		next = cu->next;
		/* This only checks against the cloacked host & vhost for normal users.
		 * IPs are only checked when triggered by an oper.. */
		if (is_oper(u) ? match_usermask_full(mask, cu->user, true) : match_usermask(mask, cu->user))
			do_up_down(cu->user, c, "down", target);
		cu = next;
	}
}

/* ------------------------------------------------------------------------------- */

/**
 * (Re)Load the configuration directives
 **/
void load_config(void) {
	int i, saoverride = 0, overridecore = 0;
	char *excempt_nicks;

	Directive confvalues[][1] = {
		{{ "ListExempNick", { { PARAM_STRING, PARAM_RELOAD, &excempt_nicks } } }},
		{{ "EnOperCmds", { { PARAM_INT, PARAM_RELOAD, &EnOperCmds } } }},
		{{ "SAdminOverride", { { PARAM_SET, PARAM_RELOAD, &saoverride } } }},
		{{ "OverrideCoreCmds", { { PARAM_SET, PARAM_RELOAD, &overridecore } } }},
		{{ "SHUNExpiry", { {PARAM_TIME, PARAM_RELOAD, &SHUNExpiry} } }},
		{{ "IgnoreBots", { { PARAM_SET, PARAM_RELOAD, &IgnoreBots } } }},
	};

    /* free allready existing ListExempts */
	for ( i = 0; i < excempt_nr; i++)
		if (ListExempts[i])
			free(ListExempts[i]);

	excempt_nicks = NULL;
	excempt_nr = 0;
	ListExempts = NULL;
	EnOperCmds = 0;
	SHUNExpiry = 0;
	IgnoreBots = 0;

	for (i = 0; i < 6; i++)
		moduleGetConfigDirective(confvalues[i]);

	/* Thx to Trystan for making this part a bit more efficient */
	if ((excempt_nicks)) {
		ListExempts = buildStringList(excempt_nicks, &excempt_nr);
		if (debug)
			for (i = 0; i < excempt_nr; i++)
				alog("debug: [bs_fantasy_ext] Added '%s' to Exempt list", ListExempts[i]);
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

	if (!SHUNExpiry)
		SHUNExpiry = DefSHUNExpiry;

	if (debug)
		alog("debug: [bs_fantasy_ext] Set the Enable oper fantasy commands to: %d", EnOperCmds);

	if (debug)
		alog("debug: [bs_fantasy_ext] %s Services Admin fantasy override.", (SAdminOverride ? "Enabled":"Disabled"));

	if (debug)
		alog("debug: [bs_fantasy_ext] %s overriding core fantasy commands.", (OverrideCoreCmds ? "Enabled":"Disabled"));

	if (debug)
		alog("debug: [bs_fantasy_ext] SHUNExpiry set to %d", SHUNExpiry);

	if (debug)
		alog("debug: [bs_fantasy_ext] IgnoreBots set to %d", IgnoreBots);

	/* Need to clear what moduleGetConfigDirective fills - Thnx to Trystan */
	if (excempt_nicks)
		free(excempt_nicks);
}


/**
 * Upon /os reload call the routines for reloading the configuration directives
 **/
int reload_config(int argc, char **argv) {
	if (!enabled || argc < 1)
		return MOD_CONT;

	if (stricmp(argv[0], EVENT_START))
		return MOD_CONT;

	alog("[bs_fantasy_ext] Reloading configuration directives & language files...");
	load_config();

	/* It is possible OverrideCoreCmds was changed so we might need to load/unload the modules. */
	check_core_modules();

	/* Check whether we are still supposed to be loaded */
	if (check_modules() == MOD_STOP) {
		enabled = 0;
		alog("[\002bs_fantasy_ext\002] Disabling all module functionality..");
		alog("[\002bs_fantasy_ext\002] This can only be reset by restarting Anope.");
	}

	/* Update flags in version reply.. */
	update_version();

	/* Reload language files... */
	mod_lang_unload();
	mod_lang_init();

	return MOD_CONT;
}


/**
 * Unloads the core modules no longer needed when bs_fantasy_ext handles the commands
 * or reloads them if needed if bs_fantasy_ext does not override them.
 **/
void check_core_modules(void) {
#ifndef _WIN32
#ifdef AUTO_UNLOAD
	if (OverrideCoreCmds) {
		/* bs_fantasy, bs_fantasy_kickban and bs_fantasy_unban are no longer needed.. */
		if (unloadModule(findModule("bs_fantasy"),NULL) == MOD_ERR_OK)
			alog("[bs_fantasy_ext] Unloaded Core Module: bs_fantasy");

		if (unloadModule(findModule("bs_fantasy_kick"),NULL) == MOD_ERR_OK)
			alog("[bs_fantasy_ext] Unloaded Core Module: bs_fantasy_kick");

		if (unloadModule(findModule("bs_fantasy_kickban"),NULL) == MOD_ERR_OK)
			alog("[bs_fantasy_ext] Unloaded Core Module: bs_fantasy_kickban");

		if (unloadModule(findModule("bs_fantasy_unban"),NULL) == MOD_ERR_OK)
			alog("[bs_fantasy_ext] Unloaded Core Module: bs_fantasy_unban");

	} else {
		/* We may now need bs_fantasy and bs_fantasy_kickban again
		 * make sure they are loaded... */
		Module *current = mod_current_module;

		if (!findModule("bs_fantasy")) {
			Module *m = createModule("bs_fantasy");
			mod_current_module = m;
			if (loadModule(m,NULL) == MOD_ERR_OK)
				alog("[bs_fantasy_ext] Loaded Core Module: bs_fantasy");
			else
				alog("[bs_fantasy_ext] Loading bs_fantasy Failed! (This is not fatal!)");
		}
		if (!findModule("bs_fantasy_kick")) {
			Module *m = createModule("bs_fantasy_kick");
			mod_current_module = m;
			if (loadModule(m,NULL) == MOD_ERR_OK)
				alog("[bs_fantasy_ext] Loaded Core Module: bs_fantasy_kick");
			else
				alog("[bs_fantasy_ext] Loading bs_fantasy_kick Failed! (This is not fatal!)");
		}
		if (!findModule("bs_fantasy_kickban")) {
			Module *m = createModule("bs_fantasy_kickban");
			mod_current_module = m;
			if (loadModule(m,NULL) == MOD_ERR_OK)
				alog("[bs_fantasy_ext] Loaded Core Module: bs_fantasy_kickban");
			else
				alog("[bs_fantasy_ext] Loading bs_fantasy_kickban Failed! (This is not fatal!)");
		}

		mod_current_module = current;
	}
#endif
#endif
}


/**
 * Checks whether the right conditions to continue loading are met and
 * checks for other loaded modules.
 **/
int check_modules(void) {
	Module *m;
	RestrictKB = 0;
	en_sync = 0;
	en_shun = 0;
	en_f_dt = 0;
	en_f_karma = 0;
	en_f_vhost = 0;
	en_why = 0;

#ifdef SUPPORTED
	if (findModule("os_raw")) {
		alog("[\002bs_fantasy_ext\002] Unsupported module found: os_raw.. (This is fatal!)");
		return MOD_STOP;
	}

	if (findCommand(OPERSERV, "RAW")) {
		alog("[\002bs_fantasy_ext\002] Unsupported module found: os_raw.. (This is fatal!)");
		return MOD_STOP;
	}

	if (!DisableRaw) {
		alog("[\002bs_fantasy_ext\002] RAW has NOT been disabled! (This is fatal!)");
		return MOD_STOP;
	}

	if (supported) {
		if (findModule("ircd_init")) {
			alog("[\002bs_fantasy_ext\002] This module is unsupported in combination with ircd_init.");
			supported = 0;
		}

		if (findModule("cs_join")) {
			alog("[\002bs_fantasy_ext\002] This module is unsupported in combination with cs_join.");
			supported = 0;
		}

		if (findModule("bs_logchanmon")) {
			alog("[\002bs_fantasy_ext\002] This module is unsupported in combination with bs_logchanmon.");
			supported = 0;
		}

		if (findModule("ircd_gameserv")) {
			alog("[\002bs_fantasy_ext\002] This module is unsupported in combination with ircd_gameserv.");
			supported = 0;
		}
		
		if (findModule("cs_joinservices")) {
			alog("[\002bs_fantasy_ext\002] This module is unsupported in combination with cs_joinservices.");
			supported = 0;
		}

		if (findModule("ircd_vhostserv")) {
			alog("[\002bs_fantasy_ext\002] This module is unsupported in combination with ircd_vhostserv.");
			supported = 0;
		}

		if (findModule("os_staychan")) {
			alog("[\002bs_fantasy_ext\002] This module is unsupported in combination with os_staychan.");
			supported = 0;
		}

		if (findModule("os_psuedo_cont")) {
			alog("[\002bs_fantasy_ext\002] This module is unsupported in combination with os_psuedo_cont.");
			supported = 0;
		}

		if (findModule("os_clientjoin")) {
			alog("[\002bs_fantasy_ext\002] This module is unsupported in combination with os_clientjoin.");
			supported = 0;
		}
	}
#endif

	if (findModule("cs_restrict_kb"))
		RestrictKB = 1;

	if ((m = findModule("cs_sync"))) {
		/* Warn about old versions of the module.. */
		if (atof(m->version) < 1.3)
			alog("[\002bs_fantasy_ext\002] Your version of \002cs_sync is outdated\002. Please update to a newer version.");
		en_sync = 1;
	}

	if (findModule("os_shun"))
		en_shun = 1;

	if (findModule("bs_fantasy_dt"))
		en_f_dt = 1;

	if (findModule("bs_fantasy_karma"))
		en_f_karma = 1;

	if (findModule("bs_fantasy_vhost"))
		en_f_vhost = 1;

	if (findModule("cs_why"))
		en_why = 1;

	return MOD_CONT;
}

/**
 * When a module is loaded, check whether it s compatible or whether we should adjust our behaviour..
 **/
int event_modload(int argc, char **argv) {
	int old_supported = supported;

	if (argc != 1 || !argv[0])
		return MOD_CONT;

	if (!enabled)
		return MOD_CONT;

#ifdef SUPPORTED
	if (supported) {
		if (!stricmp(argv[0], "os_raw")) {
			alog("[\002bs_fantasy_ext\002] Unsupported module found: %s.. (This is fatal!)", argv[0]);
			enabled = supported = 0;
		}
		if (!stricmp(argv[0], "ircd_init") || !stricmp(argv[0], "cs_join") || !stricmp(argv[0], "bs_logchanmon")
				|| !stricmp(argv[0], "ircd_gameserv") || !stricmp(argv[0], "cs_joinservices") || !stricmp(argv[0], "ircd_vhostserv")
				|| !stricmp(argv[0], "os_staychan") || !stricmp(argv[0], "os_psuedo_cont") || !stricmp(argv[0], "os_clientjoin")) {
			alog("[\002bs_fantasy_ext\002] This module is unsupported in combination with %s.", argv[0]);
			supported = 0;
		}

		if (supported != old_supported) {
			if (!enabled) {
				alog("[\002bs_fantasy_ext\002] Disabling all module functionality..");
				alog("[\002bs_fantasy_ext\002] This can only be reset by restarting Anope.");
				return MOD_STOP;
			} else if (!supported) {
				alog("[\002bs_fantasy_ext\002] Warning: Module continuing in unsupported mode!");
			}
			update_version();
		}
	}
#endif

	if (!stricmp(argv[0], "cs_restrict_kb"))
		RestrictKB = 1;
	else if (!stricmp(argv[0], "cs_sync")) {
		Module *m;
		if ((m = findModule("cs_sync"))) {
			if (atof(m->version) < 1.3)
				alog("[\002bs_fantasy_ext\002] Your version of \002cs_sync is outdated\002. Please update to a newer version.");
		}
		en_sync = 1;
	} else if (!stricmp(argv[0], "os_shun"))
		en_shun = 1;
	else if (!stricmp(argv[0], "bs_fantasy_dt"))
		en_f_dt = 1;
	else if (!stricmp(argv[0], "bs_fantasy_karma"))
		en_f_karma = 1;
	else if (!stricmp(argv[0], "bs_fantasy_vhost"))
		en_f_vhost = 1;
	else if (!stricmp(argv[0], "cs_why"))
		en_why = 1;

	return MOD_CONT;
}

/**
 * When a module is unloaded, check if it s a module we were interacting with..
 **/
int event_modunload(int argc, char **argv) {
	if (argc != 1 || !argv[0])
		return MOD_CONT;

	if (!enabled)
		return MOD_CONT;

	if (!stricmp(argv[0], "cs_restrict_kb"))
		RestrictKB = 0;
	else if (!stricmp(argv[0], "cs_sync"))
		en_sync = 0;
	else if (!stricmp(argv[0], "os_shun"))
		en_shun = 0;
	else if (!stricmp(argv[0], "bs_fantasy_dt"))
		en_f_dt = 0;
	else if (!stricmp(argv[0], "bs_fantasy_karma"))
		en_f_karma = 0;
	else if (!stricmp(argv[0], "bs_fantasy_vhost"))
		en_f_vhost = 0;
	else if (!stricmp(argv[0], "cs_why"))
		en_why = 0;

	return MOD_CONT;
}

/**
 * When a command is created, check whether it s compatible.
 **/
int event_check_cmd(int argc, char **argv) {
	int old_supported = supported;

	if (argc != 1 || !argv[0])
		return MOD_CONT;

#ifdef SUPPORTED
	if (enabled && supported) {
		if (!stricmp(argv[0], "RAW") || !stricmp(argv[0], "JOIN")) {
			alog("[\002bs_fantasy_ext\002] Unsupported command found: %s (%s).. (This is fatal!)", argv[0], argv[1]) ;
			enabled = supported = 0;
		}

		if (supported != old_supported) {
			if (!enabled) {
				alog("[\002bs_fantasy_ext\002] Disabling all module functionality..");
				alog("[\002bs_fantasy_ext\002] This can only be reset by restarting Anope.");
				return MOD_STOP;
			} else if (!supported) {
				alog("[\002bs_fantasy_ext\002] Warning: Module continuing in unsupported mode!");
			}
			update_version();
		}
	}
#endif

	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

/**
 * Create a dummy user which we can use to fool the core when it requires a user..
 **/
void create_dummy(void) {
	/* If it exists, use the message sink.. */
	if (s_DevNull) {
		dummy = scalloc(sizeof(User), 1);
		strscpy(dummy->nick, s_DevNull, NICKMAX);
	} else {
		dummy = scalloc(sizeof(User), 1);
		strscpy(dummy->nick, s_GlobalNoticer, NICKMAX);
	}
}

void delete_dummy(void) {
	if (dummy) {
		free(dummy);
	}
}

/* ------------------------------------------------------------------------------- */

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
 * Sends the user the information on this module.
 **/
static void show_modinfo(User *u, ChannelInfo *ci) {
	char *flags = get_flags();

	notice(ci->bi->nick, u->nick, "Fantasy commands provided by \002bs_fantasy_ext\002. [Author: \002%s\002] [Version: \002%s\002] [Flags: \002%s\002]",
			AUTHOR, VERSION, flags);

	free(flags);
}

/**
 * Returns pointer to a string of flags.. Must be free'd!!
 **/
static char* get_flags() {
	char tmp[BUFSIZE];
	const char version_flags[] = " " VER_DEBUG VER_OS VER_MYSQL VER_MODULE;
	char *flags;

#ifdef SUPPORTED
	snprintf(tmp, BUFSIZE, "%s-%s%s%s-OpC%d", version_flags, (supported ? "S" : "u"),
			(SAdminOverride) ? "O" : "", (IgnoreBots) ? "I" : "", EnOperCmds);
#else
	snprintf(tmp, BUFSIZE, "%s-%s%s%s-OpC%d", version_flags, "U",
			(SAdminOverride) ? "O" : "", (IgnoreBots) ? "I" : "", EnOperCmds);
#endif

	flags = sstrdup(tmp);
	return flags;
}

/**
 * Updates the version info shown in modlist and modinfo.
 **/
void update_version(void) {
	Module *m;
	char tmp[BUFSIZE];
	char *flags = get_flags();

	if (mod_current_module)
		m = mod_current_module;
	else
		m = findModule("bs_fantasy_ext");

	snprintf(tmp, BUFSIZE, "%s [%s]", VERSION, flags);

	if (m->version)
		free(m->version);
	m->version = sstrdup(tmp);
	free(flags);
}

/* ------------------------------------------------------------------------------- */

/**
 * Checks a banmask to ensure we are not going to ban someone we're not supposed to..
 **/
int check_banmask(User *u, Channel *c, char *mask) {
	struct c_userlist *cu = NULL;
	ChannelInfo *ci;

	if (!u || !c || !c->ci || !mask)
		return 0;

	ci = c->ci;
	cu = c->users;
	/* Check to make sure were are not going to ban an admin here... */
	while (cu) {
		/* This only checks against the cloacked host & vhost for normal users.
		 * IPs are only checked when triggered by an oper.. */
		if (is_oper(u) ? match_usermask_full(mask, cu->user, true) : match_usermask(mask, cu->user)) {
			if (ircd->protectedumode && is_protected(cu->user) && !is_founder(u, ci)) {
				notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
				return 0;
			} else if ((ci->flags & CI_PEACE) && (get_access(cu->user, ci) >= get_access(u, ci))) {
				notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
				return 0;
			} else if (ircd->except && is_excepted(ci, cu->user)) {
				notice_lang(ci->bi->nick, u, CHAN_EXCEPTED, cu->user->nick, ci->name);
				return 0;
			} else if (RestrictKB && ((!is_founder(u, ci) && is_services_oper(cu->user)) ||
					(is_founder(u, ci) && is_services_admin(cu->user)))) {
				notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
				return 0;
			}
		}
		cu = cu->next;
	}

	return 1;
}

/* ------------------------------------------------------------------------------- */

/**
 * Adds a ban in a channel for the given mask and adds a callback for it
 * to be removed after the given period of time.
 **/
void addTempBan(Channel *c, time_t timeout, char *banmask) {
	char *av[3];
	char *cb[2];

	cb[0] = c->name;
	cb[1] = banmask;

	av[0] = sstrdup("+b");
	av[1] = banmask;

	anope_cmd_mode(c->ci->bi->nick, c->name, "+b %s", av[1]);
	chan_set_modes(c->ci->bi->nick, c, 2, av, 1);

	free(av[0]);

	moduleAddCallback("tban", time(NULL) + timeout, delTempBan, 2, cb);
}


/**
 * Callback function to remove a given ban from a channel.
 **/
int delTempBan(int argc, char **argv) {
	char *av[3];
	Channel *c;

	av[0] = sstrdup("-b");
	av[1] = argv[1];

	if ((c = findchan(argv[0])) && c->ci && c->ci->bi) {
		anope_cmd_mode(c->ci->bi->nick, c->name, "-b %s", av[1]);
		chan_set_modes(c->ci->bi->nick, c, 2, av, 1);
	}

	free(av[0]);

	return MOD_CONT;
}


/**
 * Generates a human readable string of type "expires in ...".
 *
 * @param na Nick Alias
 * @param buf buffer to store result into
 * @param len Size of the buffer
 * @param t_left Seconds left until expiry..
 * @return buffer
 **/
char *make_timeleft(NickAlias *na, char *buf, int len, int t_left) {
	memset(buf, 0, len);

	if (!t_left) {
		strncpy(buf, getstring(na, NO_EXPIRE), len);
	} else if (t_left <= 0) {
		strncpy(buf, getstring(na, EXPIRES_SOON), len);
	} else {
		int first = 1;
		char tmp[255];

		while (t_left > 59) {
			memset(tmp, 0, 255);

			if (t_left >= 86400) {
				int days = t_left / 86400;
				t_left -= 86400 * days;
				snprintf(tmp, sizeof(tmp), getstring(na, (days == 1) ? EXPIRES_1D : EXPIRES_D), days);
				strncat(buf, tmp, len);
			} else if (first && t_left > 3600) {
				int hours = t_left / 3600, minutes = 0;
				t_left -= 3600 * hours;
				minutes = t_left / 60;
				t_left -= 60 * minutes;

				snprintf(tmp, sizeof(tmp), getstring(na, ((hours == 1 && minutes == 1) ? EXPIRES_1H1M :
						((hours == 1 && minutes != 1) ? EXPIRES_1HM :
						((hours != 1 && minutes == 1) ? EXPIRES_H1M : EXPIRES_HM)))), hours, minutes);
				strncat(buf, tmp, len);
			} else if (!first && t_left >= 3600) {
				int hours = t_left / 3600;
				t_left -= 3600 * hours;

				strncat(buf, ", ", len - 3);
				if (hours == 1)
					snprintf(tmp, sizeof(tmp), getstring(na, DURATION_HOUR));
				else
					snprintf(tmp, sizeof(tmp), getstring(na, DURATION_HOURS), hours);
				strncat(buf, tmp, len - strlen(buf) -1);
			} else if (t_left >= 60) {
				int minutes = t_left / 60;
				t_left -= 60 * minutes;
				if (first) {
					snprintf(tmp, sizeof(tmp), getstring(na, (minutes == 1) ? EXPIRES_1M : EXPIRES_M), minutes);
					strncat(buf, tmp, len);
				} else {
					strncat(buf, ", ", len - 3);
					if (minutes == 1)
						snprintf(tmp, sizeof(tmp), getstring(na, DURATION_MINUTE));
					else
						snprintf(tmp, sizeof(tmp), getstring(na, DURATION_MINUTES), minutes);
					strncat(buf, tmp, len - strlen(buf) -1);
				}
			}
			first = 0;
		}

		/* Check for leftover seconds.. */
		if (t_left) {
			memset(tmp, 0, 255);
			if (!first) {
				strncat(buf, ", ", len - 3);
				if (t_left == 1)
					snprintf(tmp, sizeof(tmp), getstring(na, DURATION_SECOND));
				else
					snprintf(tmp, sizeof(tmp), getstring(na, DURATION_SECONDS), t_left);
			} else
				snprintf(tmp, sizeof(tmp), getLangString(na, (t_left == 1) ? EXPIRES_1S : EXPIRES_S), t_left);
			strncat(buf, tmp, len - strlen(buf) -1);
		}
	}

	return buf;
}

/* ------------------------------------------------------------------------------- */

/**
 * Deletes target ban from given channel.
 **/
int delBan(ChannelInfo *ci, Entry *ban) {
	char buf[BUFSIZE];
	char *av[4];
	int ac;

	if (!ci || !ci->bi || !ci->c)
		return 0;

	av[0] = ci->name;
	if (ircdcap->tsmode) {
		snprintf(buf, BUFSIZE - 1, "%ld", (long int) time(NULL));
		av[1] = buf;
		av[2] = "-b";
		av[3] = ban->mask;
		ac = 4;
	} else {
		av[1] = "-b";
		av[2] = ban->mask;
		ac = 3;
	}

	anope_cmd_mode(ci->bi->nick, ci->name, "-b %s", ban->mask);
	do_cmode(ci->bi->nick, ac, av);
	return 1;
}

/* ------------------------------------------------------------------------------- */

char* get_str(char *param) {
	if (param)
		return param;
	else
		return "";
}

/**
 * Matches the given string to the given pattern case sensitive.
 * This is way faster then the one provided by the anope core.
 *
 * This is based upon one of the wildmatching routines of Alessandro Felice Cantatore.
 * All copyrights on this function belong to him.
 **/
int my_match_wild(char *pat, char *str) {
	char *s, *p;
	int star = 0;

loopStartC:
	for (s = str, p = pat; *s; ++s, ++p) {
		switch (*p) {
			 case '?':
				if (*s == '.')
					goto starCheckC;
				break;

			 case '*':
				star = 1;
				str = s, pat = p;

				do { ++pat; } while (*pat == '*');
				if (!*pat)
					return 1;

				goto loopStartC;

			 default:
				if (*s != *p)
					goto starCheckC;
				break;
		}
	} while (*p == '*') ++p;
	return (!*p);

starCheckC:
	if (!star)
		return 0;
	str++;
	goto loopStartC;
}

/**
 * Matches the given string to the given pattern case insensitive.
 * This is way faster then the one provided by the anope core.
 *
 * This is based upon one of the wildmatching routines of Alessandro Felice Cantatore.
 * All copyrights on this function belong to him.
 **/
int my_match_wild_nocase(char *pat, char *str) {
	char *s, *p;
	int star = 0;

loopStartNC:
	for (s = str, p = pat; *s; ++s, ++p) {
		switch (*p) {
			 case '?':
				if (*s == '.')
					goto starCheckNC;
				break;

			 case '*':
				star = 1;
				str = s, pat = p;

				do { ++pat; } while (*pat == '*');
				if (!*pat)
					return 1;

				goto loopStartNC;

			 default:
				if (tolower(*s) != tolower(*p))
					goto starCheckNC;
				break;
		}
	} while (*p == '*') ++p;
	return (!*p);

starCheckNC:
	if (!star)
		return 0;
	str++;
	goto loopStartNC;
}

/**
 * Returns a pointer to the last occurrence of str2 in str1, or a null pointer if str2 is not part of str1.
 **/
char *strrstr(const char *str1, const char *str2) {
	char *strp;
	int len1, len2;

	len2 = strlen(str2);
	if (len2 == 0)
		return NULL;

	len1 = strlen(str1);
	if (len1 - len2 <= 0)
		return NULL;

	strp = (char*)(str1 + len1 - len2);
	while (strp != str1) {
		if (*strp == *str2) {
			if (!strncmp(strp, str2, len2))
				return strp;
		}
		strp--;
	}
	return NULL;
}

/* ------------------------------------------------------------------------------- */

int get_access_nc(NickCore *nc, ChannelInfo *ci) {
	ChanAccess *access;
	if (!ci || !nc) return 0;

	if ((access = get_access_entry(nc, ci)))
		return access->level;
	return 0;
}

/* ------------------------------------------------------------------------------- */


/* EOF */
