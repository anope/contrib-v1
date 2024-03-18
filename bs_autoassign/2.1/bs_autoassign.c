/**
 * -----------------------------------------------------------------------------
 * Name      : bs_autoassign
 * Author    : Viper  <Viper@Anope.org>
 * Date      : 22/08/2007  (Last update: 12/10/2008)
 * Version   : 2.1
 * -----------------------------------------------------------------------------
 * Requires  : Anope-1.7.23
 * Tested    : Anope-1.7.23 + UnrealIRCd 3.2.6
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
 *    2.1    Fixed typo (Reported by d3xt0r)
 *           Added support for a random selection from multiple bots.
 *           Added possibility to have a custom message send.
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
# Specify the nickname(s) of the Botserv client(s) you want to automatically
# assign to a newly registered channel. When multiple nicks are listed (space
# seperated) one bot is randomly selected.
# If not specified or if the bot with the selected nick doesn't exist,
# no bot will be assigned to the channel.
#
#BSAutoAssignBots "Viper Rulez"

# BSAutoAssign [OPTIONAL]
# module bs_autoassign
#
# Specify a custom message to be send to the channel after auto-assigning 
# completes. If defined this is send after the default assign message.
#
#BSAutoAssignMessage "You have successfully registered your channel. Welcome to the network. Should you need help, join #help."

  *
  **/

/*------------------------------Configuration Block----------------------------*/

/* There are currently no configurable items... */

/*-------------------------End of Configuration Block--------------------------*/


#include "module.h"

#define AUTHOR "Viper"
#define VERSION "2.1"


/* Variables */
int bot_count;
char **BotNicks;
char *AssignBots;
char *AssignMessage;

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
	char buf[BUFSIZE];
	EvtHook *hook;

	alog("[\002bs_autoassign\002] Loading module...");

	if (!moduleMinVersion(1,7,23,1469)) {
		alog("[\002bs_autoassign\002] Your version of Anope isn't supported. Please update to a newer release.");
		return MOD_STOP;
	}

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


	/* Build list of all bots. */
	if (!BotNicks)
		snprintf(buf, BUFSIZE - 1, "<NONE>");
	else
		snprintf(buf, BUFSIZE - 1, "%s", AssignBots);

	alog("[bs_autoassign] One of the following bot(s) will be automatically assigned to all newly registered channels: %s", buf);
	alog("[\002bs_autoassign\002] Yayness!(tm) - MODULE LOADED AND ACTIVE");

	return MOD_CONT;
}

/**
 * Unload the module
 **/
void AnopeFini(void) {
	int i;

	/* Free the memory we used.. */
	if (AssignBots) free(AssignBots);
	if (AssignMessage) free(AssignMessage);
	for (i = 0; i < bot_count; i++)
		if (BotNicks[i])
			free(BotNicks[i]);
	BotNicks = NULL;
	bot_count = 0;
}


/* ------------------------------------------------------------------------------- */

/**
 * Do some sanity checks - some of which should not even be necessary - and assign
 * bot with pre-configured nick  - if it exists - to the channel.
 **/
int do_registered(int ac, char **av) {
	BotInfo *bi;
	ChannelInfo *ci;
	int nr;

	/* Some sanity checks.. */
	if (!BotNicks || ac != 1 || (!(ci = cs_findchan(av[0]))) || ci->bi
		|| (ci->botflags & BS_NOBOT))
		return MOD_CONT;
		
	/* Pick a random bot from the list and make sure it exists. */
	srand(time(NULL));
	nr = rand() % bot_count;
	if (!BotNicks[nr] || !(bi = findbot(BotNicks[nr]))) {
		alog("[bs_autoassign] Attempted to assign %s to %s but failed.", BotNicks[nr], av[0]);
		return MOD_CONT;
	}

	alog("[bs_autoassign] Auto assigning Bot to: %s [Bot: %s] [Founder: %s]", av[0], BotNicks[nr], ci->founder->display);

	ci->bi = bi;
	bi->chancount++;
	if (ci->c && ci->c->usercount >= BSMinUsers)
		bot_join(ci);

	anope_cmd_privmsg(bi->nick, ci->name,
		"I have been automatically assigned to this channel and shall join whenever there are more then %d users present.",
		BSMinUsers);
		
	/* Send custom message if it exists..*/
	if (AssignMessage)
		anope_cmd_privmsg(bi->nick, ci->name, AssignMessage);

	return MOD_CONT;
}


/* ------------------------------------------------------------------------------- */

/**
 * (Re)Load the configuration directives
 **/
void load_config(void) {
	int i;
	char *assignbots = NULL;

	Directive confvalues[][1] = {
		{{"BSAutoAssignBots", {{PARAM_STRING, PARAM_RELOAD, &assignbots}}}},
		{{"BSAutoAssignMessage", {{PARAM_STRING, PARAM_RELOAD, &AssignMessage}}}},
	};

	/* free all currently existing entries.. */
	if (AssignBots) free(AssignBots);
	if (AssignMessage) free(AssignMessage);
	for (i = 0; i < bot_count; i++)
		if (BotNicks[i])
			free(BotNicks[i]);
	BotNicks = NULL;
	AssignMessage = NULL;
	bot_count = 0;

	for (i = 0; i < 2; i++)
		moduleGetConfigDirective(confvalues[i]);

	if ((assignbots)) {
		AssignBots = sstrdup(assignbots);
		BotNicks = buildStringList(assignbots, &bot_count);
		free(assignbots);
	}
}


/**
 * Upon /os reload call the routines for reloading the configuration directives
 **/
int reload_config(int argc, char **argv) {
	char buf[BUFSIZE];
	
	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			alog("[bs_autoassign]: Reloading configuration directives...");
			load_config();
			
			/* Build list of all bots. */
			if (!BotNicks)
				snprintf(buf, BUFSIZE - 1, "<NONE>");
			else
				snprintf(buf, BUFSIZE - 1, "%s", AssignBots);

			
			alog("[bs_autoassign] One of the following bot(s) will be automatically assigned to all newly registered channels: %s ", buf);
		}
	}
	return MOD_CONT;
}

/* EOF */
