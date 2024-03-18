/**
 * -----------------------------------------------------------------------------
 * Name    : os_shun
 * Author  : Viper <Viper@Anope.org>
 * Date    : 16/09/2007 (Last update: 25/01/2009)
 * Version : 1.3
 * -----------------------------------------------------------------------------
 * Limitations : IRCD must support shun/tshun (Currently only UnrealIRCd)
 * Requires    : Anope 1.8.0, UnrealIRCd
 * Tested      : Anope 1.8.0 + UnrealIRCd 3.2.6
 * -----------------------------------------------------------------------------
 * This module adds the operserv SHUN and TSHUN commands that use the IRCd's
 * shun and tempshun abilities. Both are accessible to services operators and above.
 * When used in combination with bs_fantasy_ext (v. 1.1.13 or higher) the
 * !shun and !tshun commands will also be available.
 *
 * This module simply provides an alias for /shun and /tempshun and is currently
 * mainly meant for people using ircd_tkl_restricted.
 *
 * This module currently only supports UnrealIRCd.
 * -----------------------------------------------------------------------------
 *
 * Changelog:
 *
 *   1.3  -  Fixed crashbug in SHUN when running on windows.
 *
 *   1.2  -  Fixed crashbugs when commands are issued without parameters.
 *        -  Changed syntax to match that of AKILL and other core commands:
 *                 expiry now comes before the nick or mask.
 *
 *   1.1  -  Minor lang fixes.
 *        -  Give error when using TSHUN on an IRC Operator.
 *        -  Fixed crashbug in SHUN when giving a mask instead of a user.
 *
 *   1.0  -  Initial release
 *
 * -----------------------------------------------------------------------------
 **/

/**
 * TODO:
 *
 *  << Hope there are no more bugs ;-) >>..
 *
 **/

/**
 * Configuration directives that should be copy-pasted to services.conf

# SHUNExpiry [OPTIONAL]
# Module: os_shun
#
# Determine the default time a SHUN will last before it expires.
# If not set, this defaults to 48 hours.
#
#SHUNExpiry 48h

 *
 **/

#include "module.h"
#define AUTHOR "Viper"
#define VERSION "1.3"

/* Language defines */
#define LANG_NUM_STRINGS 					12

#define LANG_SHUN_DESC						0
#define LANG_TSHUN_DESC						1
#define LANG_SHUN_SYNTAX					2
#define LANG_SHUN_SYNTAX_EXT				3
#define LANG_TSHUN_SYNTAX					4
#define LANG_TSHUN_SYNTAX_EXT				5
#define LANG_SHUN_ADDED						6
#define LANG_SHUN_DELETION					7
#define LANG_TSHUN_ADDED					8
#define LANG_TSHUN_DELETION					9
#define LANG_TSHUN_TARGET_NEXIST			10
#define LANG_TSHUN_TARGET_OPER				11


/* variables */
int SHUNExpiry;


/* constants */
int DefSHUNExpiry = 172800;			/* 48hrs (60.60.48) */


/* Functions */
int help_shun(User *u);
int help_tshun(User *u);
void list_cmds(User *u);

int do_shun(User * u);
int do_tshun(User * u);

void load_config(void);
int reload_config(int argc, char **argv);
int valid_ircd(void);

void add_languages(void);


/* ------------------------------------------------------------------------------- */

/**
 * Create the command, and tell anope about it.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv) {
	Command *c;
	EvtHook *hook;

	if (!moduleMinVersion(1,8,0,1899)) {
		alog("[\002cs_sync\002] Your version of Anope isn't supported. Please update to a newer release.");
		return MOD_STOP;
	}

	if (!valid_ircd()) {
		alog("[\002os_shun\002] ERROR: IRCd not supported by this module");
		alog("[\002os_shun\002] Auto-Unloading module.");
		return MOD_STOP;
	}

	c = createCommand("SHUN",do_shun,is_services_oper,-1,-1,-1,-1,-1);
	moduleAddCommand(OPERSERV,c,MOD_HEAD);
	moduleAddHelp(c,help_shun);

	c = createCommand("TSHUN",do_tshun,is_services_oper,-1,-1,-1,-1,-1);
	moduleAddCommand(OPERSERV,c,MOD_HEAD);
	moduleAddHelp(c,help_tshun);
	moduleSetOperHelp(list_cmds);

	/* Hook to some events.. */
	hook = createEventHook(EVENT_RELOAD, reload_config);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002os_shun\002] Can't hook to EVENT_RELOAD event");
		return MOD_STOP;
	}

	load_config();
	add_languages();

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	alog("[\002os_shun\002] Module loaded successfully...");

	return MOD_CONT;
}


/**
 * Unload the module
 **/
void AnopeFini(void) {
	alog("[\002os_shun\002] Unloading module...");
}


/* ------------------------------------------------------------------------------- */

/**
 * Help functions
 **/
int help_shun(User *u) {
	if (is_services_oper(u))
		moduleNoticeLang(s_OperServ, u, LANG_SHUN_SYNTAX_EXT);
	else
		notice_lang(s_OperServ, u, PERMISSION_DENIED);

	return MOD_STOP;
}

int help_tshun(User *u) {
	if (is_services_oper(u))
		moduleNoticeLang(s_OperServ, u, LANG_TSHUN_SYNTAX_EXT);
	else
		notice_lang(s_OperServ, u, PERMISSION_DENIED);

	return MOD_STOP;
}

void list_cmds(User *u) {
	if (is_services_oper(u)) {
		moduleNoticeLang(s_OperServ, u, LANG_SHUN_DESC);
		moduleNoticeLang(s_OperServ, u, LANG_TSHUN_DESC);
	}
}


/* ------------------------------------------------------------------------------- */

/**
 * We handle SHUN
 **/
int do_shun(User * u) {
	User *u2;
	char breason[BUFSIZE];
	char *buffer, *cmd, *target, *expiry = NULL, *reason = NULL, *temp;
	int index = 0, action = 0;
	time_t expires = 0;

	buffer = moduleGetLastBuffer();
	cmd = myStrGetToken(buffer, ' ', index);

	if (!cmd) {
		moduleNoticeLang(s_OperServ, u, LANG_SHUN_SYNTAX);
		return MOD_CONT;
	}

	if (!stricmp(cmd, "ADD")) {
		action = 0;
		index++;
	} else if (!stricmp(cmd, "DEL")) {
		action = 1;
		index++;
	}

	if (!action) {
		temp = myStrGetToken(buffer, ' ', index);

		if (temp && *temp == '+') {
			expiry = temp;
			index++;
			target = myStrGetToken(buffer, ' ', index++);
			reason = myStrGetTokenRemainder(buffer, ' ', index);
		} else {
			expiry = NULL;
			free(temp);
			target = myStrGetToken(buffer, ' ', index++);
			reason = myStrGetTokenRemainder(buffer, ' ', index);
		}

		expires = expiry ? dotime(expiry) : SHUNExpiry;
		/* If the expiry given does not contain a final letter, it's in days. */
		if (expiry && isdigit(expiry[strlen(expiry) - 1]))
			expires *= 86400;

		if (!reason) {
			char buf[BUFSIZE];
			snprintf(buf, BUFSIZE, "Shunned by %s", u->nick);
			reason = sstrdup(buf);
		} else {
			if (AddAkiller) {
				snprintf(breason, sizeof(breason), "[%s] %s", u->nick, reason);
				free(reason);
				reason = sstrdup(breason);
			}
		}
	} else {
		target = myStrGetToken(buffer, ' ', index++);
	}

	if (!target)
		moduleNoticeLang(s_OperServ, u, LANG_SHUN_SYNTAX);
	/* Do not allow less than a minute expiry time */
	else if (expires != 0 && expires < 60)
		notice_lang(s_OperServ, u, BAD_EXPIRY_TIME);
	else {
		if ((u2 = finduser(target))) {
			if (!action) {
				send_cmd(NULL, "TKL + s * %s %s %ld %ld :%s", u2->host, u->nick,
						(long int) (time(NULL) + expires), (long int) time(NULL), reason);
				moduleNoticeLang(s_OperServ, u, LANG_SHUN_ADDED, "*", u2->host);

				if (WallOSAkill) {
					char buf[128];

					if (!expires) {
						strcpy(buf, "Does not expire");
					} else {
						int wall_expiry = expires;
						char *s = NULL;

						if (wall_expiry >= 86400) {
							wall_expiry /= 86400;
							s = "day";
						} else if (wall_expiry >= 3600) {
							wall_expiry /= 3600;
							s = "hour";
						} else if (wall_expiry >= 60) {
							wall_expiry /= 60;
							s = "minute";
						}

						snprintf(buf, sizeof(buf), "expires in %d %s%s", wall_expiry, s ? s : "",
								(wall_expiry == 1) ? "" : "s");
					}

					anope_cmd_global(s_OperServ, "%s added a SHUN on %s [*@%s] (%s) (%s)",
									 u->nick, u2->nick, u2->host, reason, buf);
				}

			} else {
				send_cmd(NULL, "TKL - s * %s %s", u2->host, u->nick);
				moduleNoticeLang(s_OperServ, u, LANG_SHUN_DELETION, "*", u2->host);
				anope_cmd_global(s_OperServ, "%s removed SHUN on %s [*@%s] ", u->nick, u2->nick,
						u2->host);
			}
		} else if (match_wild_nocase("*@*", target) && !match_wild_nocase("*!*@*", target)) {
			char *uname, *host;

			uname = myStrGetToken(target, '@', 0);
			host = myStrGetToken(target, '@', 1);

			if (!action) {
				send_cmd(NULL, "TKL + s %s %s %s %ld %ld :%s", uname, host, u->nick,
						(long int) (time(NULL) + expires), (long int) time(NULL), reason);
				moduleNoticeLang(s_OperServ, u, LANG_SHUN_ADDED, uname, host);

				if (WallOSAkill) {
					char buf[128];

					if (!expires) {
						strcpy(buf, "Does not expire");
					} else {
						int wall_expiry = expires;
						char *s = NULL;

						if (wall_expiry >= 86400) {
							wall_expiry /= 86400;
							s = "day";
						} else if (wall_expiry >= 3600) {
							wall_expiry /= 3600;
							s = "hour";
						} else if (wall_expiry >= 60) {
							wall_expiry /= 60;
							s = "minute";
						}

						snprintf(buf, sizeof(buf), "expires in %d %s%s", wall_expiry, s ? s : "",
								(wall_expiry == 1) ? "" : "s");
					}

					anope_cmd_global(s_OperServ, "%s added a SHUN on %s@%s (%s) (%s)",
									 u->nick, uname, host, reason, buf);
				}

			} else {
				send_cmd(NULL, "TKL - s %s %s %s", uname, host, u->nick);
				moduleNoticeLang(s_OperServ, u, LANG_SHUN_DELETION, uname, host);
				anope_cmd_global(s_OperServ, "%s removed SHUN on %s@%s ", u->nick, uname, host);
			}

			free(uname);
			free(host);
		} else
			moduleNoticeLang(s_OperServ, u, LANG_SHUN_SYNTAX);
	}

	free(cmd);
	if (target) free(target);
	if (expiry) free(expiry);
	if (reason) free(reason);

	return MOD_CONT;
}


/**
 * We handle TSHUN
 **/
int do_tshun(User * u) {
	User *u2;
	char breason[BUFSIZE];
	char *buffer, *cmd, *target, *reason = NULL;
	int index = 0, action = 0;

	buffer = moduleGetLastBuffer();
	cmd = myStrGetToken(buffer, ' ', 0);

	if (!cmd) {
		moduleNoticeLang(s_OperServ, u, LANG_TSHUN_SYNTAX);
		return MOD_CONT;
	}

	if (!stricmp(cmd, "ADD")) {
		action = 0;
		index++;
	} else if (!stricmp(cmd, "DEL")) {
		action = 1;
		index++;
	}

	target = myStrGetToken(buffer, ' ', index++);

	if (!action && target) {
		reason = myStrGetTokenRemainder(buffer, ' ', index++);

		if (!reason) {
			char buf[BUFSIZE];
			snprintf(buf, BUFSIZE, "Temporarily Shunned by %s", u->nick);
			reason = sstrdup(buf);
		} else {
			if (AddAkiller) {
				snprintf(breason, sizeof(breason), "[%s] %s", u->nick, reason);
				free(reason);
				reason = sstrdup(breason);
			}
		}
	}

	if (!target)
		moduleNoticeLang(s_OperServ, u, LANG_TSHUN_SYNTAX);
	else if (!(u2 = finduser(target)))
		moduleNoticeLang(s_OperServ, u, LANG_TSHUN_TARGET_NEXIST, target);
	else {
		if (!action) {
			send_cmd(NULL, "TEMPSHUN +%s %s", u2->nick, reason);
			moduleNoticeLang(s_OperServ, u, LANG_TSHUN_ADDED, u2->nick);
			anope_cmd_global(s_OperServ, "%s added a TEMPSHUN on %s (%s)", u->nick, u2->nick, reason);
		} else {
			send_cmd(NULL, "TEMPSHUN -%s", u2->nick);
			moduleNoticeLang(s_OperServ, u, LANG_TSHUN_DELETION, u2->nick);
			anope_cmd_global(s_OperServ, "%s removed TEMPSHUN on %s", u->nick, u2->nick);
		}
	}

	free(cmd);
	if (target) free(target);
	if (reason) free(reason);

	return MOD_CONT;
}


/* ------------------------------------------------------------------------------- */

/**
 * (Re)Load the configuration directives
 **/
void load_config(void) {
	int i;

	Directive confvalues[][1] = {
		{{"SHUNExpiry", {{PARAM_TIME, PARAM_RELOAD, &SHUNExpiry}}}},
	};

	SHUNExpiry = 0;

	for (i = 0; i < 1; i++)
		moduleGetConfigDirective(confvalues[i]);

	if (!SHUNExpiry)
		SHUNExpiry = DefSHUNExpiry;

	if (debug)
		alog("[os_shun] debug: SHUNExpiry set to %d", SHUNExpiry);
}


/**
 * Upon /os reload call the routines for reloading the configuration directives
 **/
int reload_config(int argc, char **argv) {
	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			alog("[os_shun]: Reloading configuration directives...");
			load_config();
		}
	}
	return MOD_CONT;
}


/**
 * We check if the ircd is supported
 **/
int valid_ircd(void) {
	if (!stricmp(IRCDModule, "unreal32"))
		return 1;

	return 0;
}


/* ------------------------------------------------------------------------------- */

/**
 * Add language strings to the module's language db.
 **/
void add_languages(void) {
	char *langtable_en_us[] = {
		/* LANG_SHUN_DESC */
		"    SHUN       Adds or deletes a shun.",
		/* LANG_TSHUN_DESC */
		"    TSHUN      Adds or deletes temporary shun for a given user.",
		/* LANG_SHUN_SYNTAX */
		" Syntax: \002SHUN [\037ADD\037 | \037DEL\037] [+\037expiry\037] [\037nick\037 | \037mask\037] [\037reason\037]\002",
		/* LANG_SHUN_SYNTAX_EXT */
		" Syntax: \002SHUN  [\037ADD\037] [+\037expiry\037] [\037nick\037 | \037mask\037] [\037reason\037]\002\n"
		"         \002SHUN  DEL [\037nick\037 | \037mask\037]\002\n"
		" \n"
		" This command adds or deletes a shun on the given nick or username@usermask\n"
		" If 'ADD' or 'DEL' is not specified, ADD is assumed.\n"
		" The time given is considered to be in days, unless another interval is specified.\n"
		" Possible alternatives: secondes (s), minutes, (m), hours, (h) and days (d).\n"
		" Note that these may not be combined! Only the first part will be read.\n"
		" If no reason is specified, a default reason will be used.\n"
		" If the mask starts with a '+', an expiry \037must\037 be given!\n"
		" \n"
		" Note that SHUN DEL is only guaranteed to work for shuns added by the SHUN command\n"
		" and only if the user is still online. In any other case, use uname@mask.\n"
		" When shunning the target 'add' or 'del', 'ADD' must be specified.",
		/* LANG_TSHUN_SYNTAX */
		" Syntax: \002TSHUN [\037ADD\037 | \037DEL\037] [\037NICK\037] [\037REASON\037]\002",
		/* LANG_TSHUN_SYNTAX_EXT */
		" Syntax: \002TSHUN [\037ADD\037] [\037nick\037] [\037reason\037]\002\n"
		"         \002TSHUN DEL [\037nick\037]\002\n"
		" \n"
		" This command adds or deletes a temporary shun on the given user.\n"
		" Note that this shun will be automatically removed when the user disconnects\n"
		" and that it will only affect one session.",
		/* LANG_SHUN_ADDED */
		" SHUN added for %s@%s.",
		/* LANG_SHUN_DELETION */
		" SHUN on %s@%s has been deleted, if it existed.",
		/* LANG_TSHUN_ADDED */
		" Temporary SHUN placed on user %s.",
		/* LANG_TSHUN_DELETION */
		" Removed temporary SHUN on user %s.",
		/* LANG_TSHUN_TARGET_NEXIST */
		" User %s does not currently exist.",
		/* LANG_TSHUN_TARGET_OPER */
		" Cannot use TEMPSHUN on IRC operators.",
	};

	char *langtable_nl[] = {
		/* LANG_SHUN_DESC */
		"    SHUN       Plaatst of verwijderd een shun.",
		/* LANG_TSHUN_DESC */
		"    TSHUN      Plaatst of verwijderd een tijdelijke shun voor een gegeven gebruiker.",
		/* LANG_SHUN_SYNTAX */
		" Syntax: \002SHUN [\037ADD\037 | \037DEL\037] [+\037verloop\037] [\037nick\037 | \037mask\037] [\037reden\037]\002",
		/* LANG_SHUN_SYNTAX_EXT */
		" Syntax: \002SHUN  [\037ADD\037] [+\037verloop\037] [\037nick\037 | \037mask\037] [\037reden\037]\002\n"
		"         \002SHUN  DEL [\037nick\037 | \037mask\037]\002\n"
		" \n"
		" Dit commando plaatst of verwijderd een shun op de gegeven gebruiker of user@host.\n"
		" Als 'ADD' of 'DEL' niet opgegeven zijn, wordt 'ADD' verondersteld.\n"
		" De verlooptijd wordt verondersteld in dagen te zijn opgegeven, tenzij dit anders\n"
		" is opgegeven. Mogelijke alternatieven zijn: seconden (s), minuten, (m), uren, (h) \n"
		" en dagen (d). Deze mogen niet worden gecombineerd! Enkel het eerste deel wordt gelezen.\n"
		" Indien geen reden is opgegeven wordt een standaard gebruikt.\n"
		" Indien de mask start met een '+' moet een verlooptijd opgegeven worden!\n"
		" \n"
		" SHUN DEL NICK werkt enkel gegarandeerd voor shuns die toegevoegd zijn via het SHUN commando\n"
		" en enkel indien de gebruiker nog steeds online is. In andere gevallen, gebruik SHUN DEL name@host.\n"
		" Wanneer de doelnick 'add' of 'del' is moet 'ADD' opgegeven worden.",
		/* LANG_TSHUN_SYNTAX */
		" Syntax: \002TSHUN [\037ADD\037 | \037DEL\037][\037nick\037] [\037reden\037]\002",
		/* LANG_TSHUN_SYNTAX_EXT */
		" Syntax: \002TSHUN [\037ADD\037] [\037nick\037] [\037reden\037]\002\n"
		"         \002TSHUN DEL [\037NICK\037]\002\n"
		" \n"
		" Dit commando plaatst of verwijderd een tijdelijke SHUN op de gegeven gebruiker.\n"
		" Deze shun zal automatisch worden verwijderd wanneer de gebruiker de verbinding verbreekt\n"
		" en zal steeds slechts effect hebben op 1 sessie.",
		/* LANG_SHUN_ADDED */
		" SHUN toegevoegd voor %s@%s.",
		/* LANG_SHUN_DELETION */
		" SHUN ,indien deze bestond, verwijderd voor %s@%s.",
		/* LANG_TSHUN_ADDED */
		" Tijdelijke SHUN geplaatst op gebruiker %s.",
		/* LANG_TSHUN_DELETION */
		" Tijdelijke SHUN op gebruiker %s verwijderd.",
		/* LANG_TSHUN_TARGET_NEXIST */
		" Gebruiker %s is niet online.",
		/* LANG_TSHUN_TARGET_OPER */
		" TEMPSHUN kan niet op IRC Operators toegepast worden.",
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
	moduleInsertLanguage(LANG_NL, LANG_NUM_STRINGS, langtable_nl);
}

/* EOF */
