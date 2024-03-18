/**
 * -----------------------------------------------------------------------------
 * Name    : bs_fantasy_redirect
 * Author  : VisioN  <vision@myirc.net>
 * Date: 15/1/2012   Last Updated: 15/1/2012
 * Version : 1.0
 * -----------------------------------------------------------------------------
 * Limitations : Any IRCd which supports SVSPART and SVSJOIN
 * Requires    : Anope 1.8.7
 * Tested      : Anope 1.8.7 + UnrealIRCd 3.2.8 & UnrealIRCD 3.2.9
 * -----------------------------------------------------------------------------
 * This module creates the !redirect command.
 * The command syntax is  !redirect user #channel
 * This command makes the user part the current channel and join the channel you
 * spesify with the "#channel" argument.
 * 
 *
 * I made this module because in my network we have various help channels (in
 * different languages), so it makes redirecting users to the correct help
 * channel easier. However you are welcome to use it anyway you like ;-)
 *
 * This module is based on Viper's bs_fantasy_fpart (he always makes my life easier)
 * -----------------------------------------------------------------------------
 * Changelog:
 *
 *   1.0 - Initial Module Release
 *
 *   
 *
 * -----------------------------------------------------------------------------
 **/

#include "module.h"

#define AUTHOR "VisioN v1.1"
#define VERSION "1.1"


/* Language defines */
#define LANG_NUM_STRINGS 					6

#define LANG_REDIRECT_SYNTAX			 	0
#define LANG_NO_SUCH_USER					1
#define LANG_CHANNAME_TOO_LONG				2
#define LANG_USER_NOT_IN_CHAN				3
#define LANG_REDIRECTING				       4
#define LANG_FORCE_REDIRECT					5


/* Functions */
int do_fantasy(int ac, char **av);
void do_redirect(User *u, Channel *c, char *nick, char *c2);
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
	EvtHook *hook;

	alog("[\002bs_fantasy_redirect\002] Loading module...");

	if (!valid_ircd()) {
		alog("[\002bs_fantasy_redirect\002] ERROR: IRCd not supported by this module");
		alog("[\002bs_fantasy_redirect\002] Unloading module...");
		return MOD_STOP;
	}

	if (!moduleMinVersion(1, 8, 7, 3089)) {
		alog("[\002bs_fantasy_redirect\002] Your version of Anope isn't supported. Please update to a newer release.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_BOT_FANTASY, do_fantasy);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_redirect\002] Can't hook to EVENT_BOT_FANTASY event");
		return MOD_STOP;
	}
	
	hook = createEventHook(EVENT_BOT_FANTASY_NO_ACCESS, do_fantasy);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_fpart\002] Can't hook to EVENT_BOT_FANTASY_NO_ACCESS event");
		return MOD_STOP;
	}

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	add_languages();

	alog("[\002bs_fantasy_redirect\002] Module loaded successfully...");

	return MOD_CONT;
}


/**
 * Unload the module
 **/
void AnopeFini(void) {
	alog("[\002bs_fantasy_redirect\002] Unloading module...");
}

/* ------------------------------------------------------------------------------- */

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

	if (stricmp(av[0], "redirect") == 0) {
		/* If executed without any params, show help */
		if (ac == 3)
			moduleNoticeLang(ci->bi->nick, u, LANG_REDIRECT_SYNTAX, BSFantasyCharacter);
		else {
			if (is_services_oper(u)) {
				/* Get the arguments: nick and the remainder is channel to go to. */
				char *arg1 = myStrGetToken(av[3],' ',0);
				char *arg2 = myStrGetTokenRemainder(av[3],' ',1);

				if (!arg1 || !arg2)
					moduleNoticeLang(ci->bi->nick, u, LANG_REDIRECT_SYNTAX, BSFantasyCharacter);
				else
					do_redirect(u, c, arg1, arg2);

				if (arg1) free(arg1);
				if (arg2) free(arg2);
			} else
				notice_lang(ci->bi->nick, u, PERMISSION_DENIED);

		}
	}

	/* Continue processig event.. maybe other modules want it too */
	return MOD_CONT;
}



/* ------------------------------------------------------------------------------- */

void do_redirect(User *u, Channel *c, char *nick, char *c2) {
	User *target = NULL;

	if (!u || !c || !nick || !c2)
		return;

	/* Check the 1st argument [nick] is a valid (in use) nick -
	 * This will filter out services/botserv bots.
	 */
	if (!(target = finduser(nick))) {
		moduleNoticeLang(c->ci->bi->nick, u, LANG_NO_SUCH_USER, nick);
		return;
	}

	/* Check whether user is allready on channel */
	if (!is_on_chan(c, target)) {
		moduleNoticeLang(c->ci->bi->nick, u, LANG_USER_NOT_IN_CHAN, nick);
		return;
	}

	moduleNoticeLang(c->ci->bi->nick, u, LANG_REDIRECTING, nick, c->name, c2);

	/* make some noise to prevent abusers */
	wallops(c->ci->bi->nick, "%s redirected %s from channel %s to channel %s .", u->nick, nick, c->name , c2);
	moduleNoticeLang(s_ChanServ, target, LANG_FORCE_REDIRECT, c->name, c2,u->nick);

	/* Force them out */
	anope_cmd_svspart(c->ci->bi->nick, nick, c->name);
	anope_cmd_svsjoin(s_OperServ, target->nick, c2, NULL);
}

/* ------------------------------------------------------------------------------- */

/**
 * We check if the ircd is supported
 **/
int valid_ircd(void) {
	if (!stricmp(IRCDModule, "unreal32"))
		return 1;

	if (!stricmp(IRCDModule, "viagra"))
		return 1;

	if (!stricmp(IRCDModule, "ptlink"))
		return 1;

	if (!stricmp(IRCDModule, "ultimate2"))
		return 1;

	if (!stricmp(IRCDModule, "ultimate3"))
		return 1;

	if (!stricmp(IRCDModule, "plexus3"))
		return 1;

	if (!stricmp(IRCDModule, "inspircd11"))
		return 1;

	if (!stricmp(IRCDModule, "inspircd12"))
		return 1;

	if (!stricmp(IRCDModule, "inspircd20"))
		return 1;

	return 0;
}


/* ------------------------------------------------------------------------------- */

/**
 * Add language strings to the module's language db.
 **/
void add_languages(void) {
	char *langtable_en_us[] = {
		/* LANG_REDIRECT_SYNTAX */
		"Syntax: %sredirect [\037nick\037] [\037#channel\037]",
		/* LANG_NO_SUCH_USER */
		" User [%s] does not exist or is a services client",
		/* LANG_CHANNAME_TOO_LONG */
		" That channel name is too long.",
		/* LANG_USER_NOT_IN_CHAN */
		" The user [%s] is not in channel.",
		/* LANG_REDIRECTING */
		"Redirecting %s from %s to %s .",
		/* LANG_FORCE_PARTED */
		"You have been redirected from %s to %s by operator %s .\n"
		"If you believe that you were on the correct channel please rejoin.",

	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}

/* EOF */
