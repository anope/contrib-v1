/**
 * -----------------------------------------------------------------------------
 * Name    : cs_why
 * Author  : Viper  <Viper@Anope.org>
 * Date    : 27/11/2011  (Last update: 24/12/2011)
 * Version : 1.2
 * -----------------------------------------------------------------------------
 * Requires    : Anope-1.8.6
 * Tested      : Anope 1.8.7 + UnrealIRCd 3.2.8
 * -----------------------------------------------------------------------------
 * This module will give information on the kind of access a user has in a channel.
 *
 * This module is an updated version of cs_why 1.0.3 by DrStein.
 *
 * This module is released under the GPL 2 license.
 * -----------------------------------------------------------------------------
 * Changelog:
 *
 *   1.2   Module Development taken over by me (Viper).
 *         Code update & cleanup.
 *         Added support for fantasy command !why.
 *         Added distinction for superadmins.
 *
 * -----------------------------------------------------------------------------
 **/

#include "module.h"
#define AUTHOR "Viper"
#define VERSION "1.2"


/* Language defines */
#define LANG_NUM_STRINGS					12

#define LANG_WHY_DESC						0
#define LANG_WHY_SYNTAX						1
#define LANG_WHY_SYNTAX_FANT				2
#define LANG_WHY_SYNTAX_EXT					3
#define USER_X_NOT_IDENTIFIED				4
#define LANG_USER_NOT_IN_CHAN				5
#define LANG_NICK_X_HAS_NO_ACCESS			6
#define LANG_NICK_X_HAS_XOP					7
#define LANG_NICK_X_HAS_ACCESS				8
#define LANG_NICK_X_IS_FOUNDER				9
#define LANG_NICK_X_IS_REAL_FOUNDER			10
#define LANG_NICK_X_IS_SUPERADMIN			11


/* Functions */
int do_why(User * u);
int do_fantasy(int ac, char **av);
void parse_why(User *u, char *bot, ChannelInfo *ci, char *nick);

void do_help_list(User *u);
int do_help(User *u);

void add_languages(void);

/* ------------------------------------------------------------------------------- */

/**
 * AnopeInit is called when the module is loaded.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it.
 **/
int AnopeInit(int argc, char **argv) {
	Command *c;
	EvtHook *hook;

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	if (!moduleMinVersion(1, 8, 6, 3071)) {
		alog("[\002cs_why\002] Your version of Anope isn't supported. Please update to a newer release.");
		return MOD_STOP;
	}

	c = createCommand("WHY", do_why, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(CHANSERV,c,MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002cs_why\002] Cannot create WHY command...");
		return MOD_STOP;
	}
	moduleAddHelp(c, do_help);
	moduleSetChanHelp(do_help_list);

	hook = createEventHook(EVENT_BOT_FANTASY, do_fantasy);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002cs_why\002] Can't hook to EVENT_BOT_FANTASY event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_BOT_FANTASY_NO_ACCESS, do_fantasy);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002cs_why\002] Can't hook to EVENT_BOT_FANTASY_NO_ACCESS event.");
		return MOD_STOP;
	}

	add_languages();

	alog("[\002cs_why\002] Module loaded successfully...");

	return MOD_CONT;
}

/**
 * Unload the module.
 **/
void AnopeFini(void) {
	alog("[\002cs_why\002] Unloading module...");
}

/* ------------------------------------------------------------------------------- */

/**
 * Add the SYNC command to the ChanServ HELP listing.
 **/
void do_help_list(User * u) {
	moduleNoticeLang(s_ChanServ, u, LANG_WHY_DESC);
}


/**
 * Show the extended help on the SYNC command.
 **/
int do_help(User * u) {
	moduleNoticeLang(s_ChanServ, u, LANG_WHY_SYNTAX);
	moduleNoticeLang(s_ChanServ, u, LANG_WHY_SYNTAX_EXT);
	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

int do_why(User * u) {
	ChannelInfo *ci;
	char *buffer, *chan, *nick;

	buffer = moduleGetLastBuffer();
	chan = myStrGetToken(buffer, ' ', 0);
	nick = myStrGetToken(buffer, ' ', 1);

	if (!chan || !nick)
		moduleNoticeLang(s_ChanServ, u, LANG_WHY_SYNTAX);
	else if (!(ci = cs_findchan(chan)))
		notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
	else if (ci->flags & CI_VERBOTEN)
		notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
	else if (ci->flags & CI_SUSPENDED)
		notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
	else
		parse_why(u, s_ChanServ, ci, nick);

	if (chan) free(chan);
	if (nick) free(nick);
	return MOD_CONT;
}


/**
 * Handles all fantasy commands.
 * Here we ll identify the command and call the right routines.
 **/
int do_fantasy(int ac, char **av) {
	User *u;
	ChannelInfo *ci;
	Channel *c;

	/* Some basic error checking... should never match */
	if (ac < 3)
		return MOD_CONT;

	if (!(ci = cs_findchan(av[2])))
		return MOD_CONT;
	if (!(u = finduser(av[1])))
		return MOD_CONT;
	if (!(c = findchan(ci->name)))
		return MOD_CONT;
	if (!ci->bi)
		return MOD_CONT;

	if (!stricmp(av[0], "why")) {
		if (ac == 3)
			moduleNoticeLang(ci->bi->nick, u, LANG_WHY_SYNTAX_FANT, BSFantasyCharacter);
		else {
			char *nick;
			nick = myStrGetToken(av[3], ' ', 0);
			if (nick) { 
				parse_why(u, ci->bi->nick, ci, nick);
				free(nick);
			} else
				moduleNoticeLang(ci->bi->nick, u, LANG_WHY_SYNTAX_FANT, BSFantasyCharacter);
		}
	} else if (!stricmp(av[0], "help")) {
		if (ac > 3) {
			char *cmd;
			cmd = myStrGetToken(av[3],' ',0);
			if (cmd && !stricmp(cmd, "why")) {
				moduleNoticeLang(ci->bi->nick, u, LANG_WHY_SYNTAX_FANT, BSFantasyCharacter);
				moduleNoticeLang(ci->bi->nick, u, LANG_WHY_SYNTAX_EXT);
			}
			if (cmd) free(cmd);
		}
	}
	return MOD_CONT;
}


void parse_why(User *u, char *bot, ChannelInfo *ci, char *nick) {
	ChanAccess *access;
	User *u2;
	const char *xop;

	if (!(u2 = finduser(nick)))
		notice_lang(bot, u, NICK_X_NOT_IN_USE, nick);
	else if (!u2->na || !u2->na->nc)
		notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, nick);
	else if (!nick_identified(u2))
		moduleNoticeLang(bot, u, USER_X_NOT_IDENTIFIED, nick);
	else if (!ci->c || !is_on_chan(ci->c, u2))
		moduleNoticeLang(bot, u, LANG_USER_NOT_IN_CHAN, nick, ci->name);
	else if (u2->isSuperAdmin)
		moduleNoticeLang(bot, u, LANG_NICK_X_IS_SUPERADMIN, nick, u2->na->nc->display);
	else if (is_real_founder(u2, ci))
		moduleNoticeLang(bot, u, LANG_NICK_X_IS_REAL_FOUNDER, nick, u2->na->nc->display, ci->name);
	else if (is_founder(u2, ci))
		moduleNoticeLang(bot, u, LANG_NICK_X_IS_FOUNDER, nick, u2->na->nc->display, ci->name);
	else if (!ci->accesscount || !(access = get_access_entry(u2->na->nc, ci)))
		moduleNoticeLang(bot, u, LANG_NICK_X_HAS_NO_ACCESS, nick, ci->name);
	else if (access->level && (ci->flags & CI_XOP) && access->level >= ACCESS_VOP && (xop = get_xop_level(access->level)))
		moduleNoticeLang(bot, u, LANG_NICK_X_HAS_XOP, nick, u2->na->nc->display, xop, ci->name);
	else if (access->level && (!(ci->flags & CI_XOP)))
		moduleNoticeLang(bot, u, LANG_NICK_X_HAS_ACCESS, nick, u2->na->nc->display, access->level, ci->name);
	else 
		moduleNoticeLang(bot, u, LANG_NICK_X_HAS_NO_ACCESS, nick, ci->name);
}

/* ------------------------------------------------------------------------------- */

void add_languages(void) {
	char *langtable_en_us[] = {
		/* LANG_WHY_DESC */
		"    WHY        Find out about a users access on a channel.",
		/* LANG_WHY_SYNTAX */
		"Syntax: WHY \037#channel\037 \037nick\037",
		/* LANG_WHY_SYNTAX_FANT */
		"Syntax: %sWHY \037nick\037",
		/* LANG_WHY_SYNTAX_EXT */
		"\n"
		"Give information on the access a user has in a channel.",
		/* USER_X_NOT_IDENTIFIED */
		"User %s isn't identified.",
		/* LANG_USER_NOT_IN_CHAN */
		"User %s is not in channel %s.",
		/* LANG_NICK_X_HAS_NO_ACCESS */
		"%s does not have access to channel %s.",
		/* LANG_NICK_X_HAS_XOP */
		"%s (%s) has %s access to channel %s.",
		/* LANG_NICK_X_HAS_ACCESS */
		"%s (%s) has access level %d to channel %s.",
		/* LANG_NICK_X_IS_FOUNDER */
		"%s (%s) has identified as founder of channel %s.",
		/* LANG_NICK_X_IS_REAL_FOUNDER */
		"%s (%s) is founder of channel %s.",
		/* LANG_NICK_X_IS_SUPERADMIN */
		"%s (%s) is a Super-Administrator and thus 'owner' of all channels."
	};

	char *langtable_nl[] = {
		/* LANG_WHY_DESC */
		"    WHY        Vraag info over de toegang van een gebruiker tot een kanaal.",
		/* LANG_WHY_SYNTAX */
		"Syntax: WHY \037#kanaal\037 \037nick\037",
		/* LANG_WHY_SYNTAX_FANT */
		"Syntax: %sWHY \037nick\037",
		/* LANG_WHY_SYNTAX_EXT */
		"\n"
		"Geef informatie over het toegangsniveau van een gebruiker tot een kanaal.",
		/* USER_X_NOT_IDENTIFIED */
		"Gebruiker %s is niet geidentificeerd.",
		/* LANG_USER_NOT_IN_CHAN */
		"Gebruiker %s is niet in kanaal %s.",
		/* LANG_NICK_X_HAS_NO_ACCESS */
		"%s heeft geen toegang tot kanaal %s.",
		/* LANG_NICK_X_HAS_XOP */
		"%s (%s) heeft %s toegang tot kanaal %s.",
		/* LANG_NICK_X_HAS_ACCESS */
		"%s (%s) heeft toegangsniveau %d tot kanaal %s.",
		/* LANG_NICK_X_IS_FOUNDER */
		"%s (%s) heeft ingelogd als stichter van kanaal %s.",
		/* LANG_NICK_X_IS_REAL_FOUNDER */
		"%s (%s) is de stichter van kanaal%s.",
		/* LANG_NICK_X_IS_SUPERADMIN */
		"%s (%s) is een Super-Administrator en dus 'eigenaar' van alle kanalen."
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
	moduleInsertLanguage(LANG_NL, LANG_NUM_STRINGS, langtable_nl);
}

/* EOF */
