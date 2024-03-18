/**
 * -----------------------------------------------------------------------------
 * Name    : bs_autoassign
 * Author  : Viper  <Viper@Absurd-IRC.net>
 * Date    : 22/08/2007  (Last update: 23/08/2007)
 * Version : 2.0
 * -----------------------------------------------------------------------------
 * Tested  : Anope-1.7.19 + UnrealIRCd 3.2.6
 * -----------------------------------------------------------------------------
 * This module allows Botserv clients to be automatically assigned to newly registered
 * channels. The bot to be auto-assigned can be configured in the services configuration
 * file. If no nick is given, or no bot with that nick exists, no bot will be assigned.
 *
 * This module is a completely redesigned and rewritten version of the bs_autoassign
 * module wirtten by SGR. This version has been written specifically for the 1.7 branch
 * and will NOT work with the 1.6 branch or old 1.7 releases.
 * -----------------------------------------------------------------------------
 *
 * Changelog:
 *
 *    2.0    Module Development taken over by me (Viper)
 *           Beginning with clean development version.
 *
 * -----------------------------------------------------------------------------
 **/

 /**
  * Configuration directives that should be copy-pasted to services.conf

# BSAutoAssignBot [OPTIONAL]
# Module: bs_autoassign
#
# Specify the nickname of the Botserv client you want to automatically
# assign to a newly registered channel.
# If not specified or if no bot with the given nick exists,
# no bot will be assigned to the channel.
#
#BSAutoAssignBot "ServiceBot"

  *
  **/

/*------------------------------Configuration Block----------------------------*/

/* There are currently no configurable items... */

/*-------------------------End of Configuration Block--------------------------*/


#include "module.h"

#define AUTHOR "Viper"
#define VERSION "2.0"


/* Variables */
char *BotNick;

/* Functions */
int do_registered(int ac, char **av);
void load_config(void);
int reload_config(int argc, char **argv);


/* ------------------------------------------------------------------------------- */

/**
 * Create the command, and tell anope about it.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv) {
	EvtHook *hook;

	alog("[\002bs_autoassign\002] Loading module...");

	/* Hook to events.. */
	hook = createEventHook(EVENT_CHAN_REGISTERED, do_registered);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_autoassign\002] Can't hook to EVENT_CHAN_REGISTERED event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_RELOAD, reload_config);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_autoassign\002] Can't hook to EVENT_RELOAD event");
		return MOD_STOP;
	}

	load_config();

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	alog("[bs_autoassign] The following bot will be assigned to all newly registered channels: %s ",
		(BotNick ? BotNick : "<NONE>"));
	alog("[\002bs_autoassign\002] Yayness!(tm) - MODULE LOADED AND ACTIVE");

	return MOD_CONT;
}

/**
 * Unload the module
 **/
void AnopeFini(void) {
	/* Free the memory we used.. */
	if (BotNick)
		free(BotNick);
}


/* ------------------------------------------------------------------------------- */

/**
 * Do some sanity checks - some of which should not even be necessary - and assign
 * bot with pre-configured nick  - if it exists - to the channel.
 **/
int do_registered(int ac, char **av) {
	BotInfo *bi;
	ChannelInfo *ci;

	/* Some sanity checks.. */
	if (!BotNick || ac != 1 || (!(ci = cs_findchan(av[0]))) || ci->bi
		|| (ci->botflags & BS_NOBOT) || (!(bi = findbot(BotNick))))
		return MOD_CONT;

	alog("[bs_autoassign] Auto assigning Bot to: %s [Bot: %s] [Founder: %s]", av[0], BotNick, ci->founder->display);

	ci->bi = bi;
	bi->chancount++;
	if (ci->c && ci->c->usercount >= BSMinUsers)
		bot_join(ci);

	anope_cmd_privmsg(bi->nick, ci->name,
		"I have been successfully assigned to this channel and shall join whenever there are mode then %d users present.",
		BSMinUsers);

	return MOD_CONT;
}


/* ------------------------------------------------------------------------------- */

/**
 * (Re)Load the configuration directives
 **/
void load_config(void) {
	int i;

	Directive confvalues[][1] = {
		{{"BSAutoAssignBot", {{PARAM_STRING, PARAM_RELOAD, &BotNick}}}},
	};

	if (BotNick)
		free(BotNick);
	BotNick = NULL;

	for (i = 0; i < 1; i++)
		moduleGetConfigDirective(confvalues[i]);

	if (debug)
		alog ("[bs_autoassign] Debug: BSAutoAssignBot set to %s", BotNick);
}


/**
 * Upon /os reload call the routines for reloading the configuration directives
 **/
int reload_config(int argc, char **argv) {
	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			alog("[bs_autoassign]: Reloading configuration directives...");
			load_config();
			alog("[bs_autoassign] The following bot will be assigned to all newly registered channels: %s ",
				(BotNick ? BotNick : "<NONE>"));

		}
	}
	return MOD_CONT;
}

/* EOF */
