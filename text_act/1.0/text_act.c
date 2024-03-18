#include "module.h"

/**
 * -----------------------------------------------------------------------------
 * Name: text_act
 * Author: Viper  <Viper@Absurd-IRC.net>
 * Date: 25/08/2006  (Last update: 25/08/2006)
 * -----------------------------------------------------------------------------
 * Tested: Anope-1.7.15 + UnrealIRCd 3.2.3
 * -----------------------------------------------------------------------------
 * This module has been tested on Anope-1.7.15, it might work on older
 * 1.7 versions, but it is not supported.
 *
 * This module will perform an action on a user when he says a certain text in
 * a channel. Dont ask me what this could be usefull for, but someone asked for
 * this. I don't recommend using this as spamfilter, there are better ways to
 * deal with spam then this.
 * -----------------------------------------------------------------------------
 *
 * Changes:
 *
 *  1.0  First release
 *
 *
 * -----------------------------------------------------------------------------
 **/


/**
 * Configuration directives that should be copy-pasted to services.conf

# TXTAFilters [REQUIRED]
# Module: text_act
#
# The wildcards we will look for in channel messages. If a message matches the wildward,
# we will take the action specified in TXTAAction. This is a space seperated string of wildcards.
# If this is not specified, the module will not perform any checks.
#
# To look for a full message for example "<Viper> test" simple use TXTAFilters "test".
# Note that this will not match anything inside a sentense, to make it do that use TXTAFilters "*test*".
# Note that this would also match "<Viper> i am just testing". Because spaces are seen as borders between
# different filters, putting a sentense in here to filter on will not work.
#TXTAFilters ""

# TXTAAction [REQUIRED]
# Module: text_act
#
# This tells the module which action to take when a channel message matches a
# wildcard string in TXTAFilters.
# Following are posisble options:
#    0 - Disable module
#    1 - Do nothing
#    2 - Send a globops
#    3 - kill the user
#    4 - akill the user
#
# If this is not specified, the module will NOT act when it finds a match, it will
# still make a note to the log/logchannel however.
#TXTAAction 0

# TXTAAkillTime [OPTIONAL]
# Module: text_act
#
# This determines how long users will be Akilled if the action is set to akill.
# Please note this is subject to ExpireTimeout.
# If this is not specified a default value of 30 minutes will be used instead.
#TXTAAkillTime 2700s

# TXTAKillReason [OPTIONAL]
# Module: text_act
#
# The reason that is given when killing a user who said something matching a filter.
# If this is not specified a default message will be used instead.
#TXTAKillReason "You matched a spamfilter..."

# TXTAAkillReason [OPTIONAL]
# Module: text_act
#
# The reason that is given when akilling a user who said something matching a filter.
# If this is not specified a default message will be used instead.
#TXTAAkillReason "You have been akilled for matching a spamfilter."

 *
 **/


#define AUTHOR "Viper"
#define VERSION "1.0"

// default settings
char *DefKillReason = "You matched a spamfilter...";
char *DefAkillReason = "You have been akilled for matching a spamfilter.";

// variables
int nr_filters, action, AkillTime;
char *KillReason, *AkillReason;
char **filters;


// functions
int my_privmsg(char *source, int ac, char **av);
int do_act(User *u, char *filter, char *text);
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
    Message *msg;
    EvtHook *hook;
    int status;

    msg = createMessage("PRIVMSG", my_privmsg);
    status = moduleAddMessage(msg, MOD_HEAD);

	hook = createEventHook(EVENT_RELOAD, reload_config);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002text_act\002] Can't hook to EVENT_RELOAD event");
		return MOD_STOP;
	}

	load_config();

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	alog("[\002text_act\002] Module loaded... [Status: %d]", status);

	return MOD_CONT;
}


/**
 * Unload the module
 **/
void AnopeFini(void) {
	int i;
	alog("[\002text_act\002] Unloading module...");

	// free vars
	for ( i = 0; i < nr_filters; i++)
		if (filters[i])
			free(filters[i]);

	if (KillReason) free(KillReason);
	if (AkillReason) free(AkillReason);
}


/* ------------------------------------------------------------------------------- */


/**
 * Look for channel messages...
 **/
int my_privmsg(char *source, int ac, char **av) {
	User *u;
	ChannelInfo *ci;
	char *text = NULL;
	int i = 0, status = MOD_CONT;

	if (ac != 2)
		return MOD_CONT;
	if (!(u = finduser(source))) {
		return MOD_CONT;
	}
	if (action == 0)
		return MOD_CONT;


	if (*av[0] == '#') {
		if ((ci = cs_findchan(av[0]))) {
			if (mod_current_buffer) {
				text = mod_current_buffer;
				while (i < nr_filters) {
					if (!filters[i])
						break;

					if (match_wild_nocase(filters[i], text)) {
						alog("[text_act] %s matched a filter (%s) with '%s' ", u->nick, filters[i], text);
						status = do_act(u, filters[i], text);
						break;
					}

					i++;
				}

			}
		}
	}

	return status;
}


int do_act(User *u, char *filter, char *text) {
	char akillmask[BUFSIZE];
	time_t expires = AkillTime + time(NULL);

	if (action == 1)
		alog("[text_act] Taking no action against %s...", u->nick);
	else if (action == 2) {
		anope_cmd_global(s_OperServ, "%s matched a filter (%s) with (%s)", u->nick, filter, text);
	} else if (action == 3) {
		alog("[text_act] Killing %s...", u->nick);
		anope_cmd_global(s_OperServ, "%s matched a filter (%s) with (%s)", u->nick, filter, text);
		kill_user(s_OperServ, u->nick, KillReason);
		return MOD_STOP;
	} else if (action == 4) {
		sprintf(akillmask, "*@%s", u->host);
		alog("[text_act] Akilling %s...", u->nick);
		anope_cmd_global(s_OperServ, "%s matched a filter (%s) with (%s)", u->nick, filter, text);
		add_akill(NULL, akillmask, s_OperServ, expires, AkillReason);
		return MOD_STOP;
	}

	return MOD_CONT;
}


/* ------------------------------------------------------------------------------- */


/**
 * (Re)Load the configuration directives
 **/
void load_config(void) {
	int i;
	char *tkillreason = NULL, *takillreason = NULL, *tfilters = NULL;

	Directive confvalues[][1] = {
		{{ "TXTAFilters", { { PARAM_STRING, PARAM_RELOAD, &tfilters } } }},
		{{ "TXTAAction", { { PARAM_POSINT, PARAM_RELOAD, &action } } }},
		{{ "TXTAAkillTime", { { PARAM_TIME, PARAM_RELOAD, &AkillTime } } }},
		{{ "TXTAKillReason", { { PARAM_STRING, PARAM_RELOAD, &tkillreason } } }},
		{{ "TXTAAkillReason", { { PARAM_STRING, PARAM_RELOAD, &takillreason } } }},
	};

    // free allready existing filters
	for ( i = 0; i < nr_filters; i++)
		if (filters[i])
			free(filters[i]);

	filters = NULL;
	nr_filters = 0;
	action = 0;
	AkillTime = 0;
	if (KillReason) free(KillReason);
	KillReason = NULL;
	if (AkillReason) free(AkillReason);
	AkillReason = NULL;

	for (i = 0; i < 5; i++)
		moduleGetConfigDirective(confvalues[i]);

	if ((tfilters)) {
		filters = buildStringList(tfilters, &nr_filters);
		if (debug)
			for (i = 0; i < nr_filters; i++)
				alog("debug: [text_act] Added '%s' to the filter list", filters[i]);
	}

	if ((action < 0) || (action > 4)) {
		action = 0;
		alog("[\002text_act\002] TXTAAction needs to be a value between 0 and 5. Using default value 0...");
	}

	if (!AkillTime)
		AkillTime = 30 * 60;  // default time = 30 mins
	else if (AkillTime < 60) {
		AkillTime = 30 * 60;
		alog("[\002text_act\002] TXTAAkillTime needs to be at least 60. Using default values...");
	}

	if (tkillreason)
		KillReason = sstrdup(tkillreason);
	else
		KillReason = sstrdup(DefKillReason);

	if (takillreason)
		AkillReason = sstrdup(takillreason);
	else
		AkillReason = sstrdup(DefAkillReason);

	if (debug)
		alog("debug: [text_act] Config Vars: Action=%d AkillTime=%d KillReason='%s' AkillReason='%s'", action, AkillTime, KillReason, AkillReason);

	// free everything we used...
	free(tkillreason);
	free(takillreason);
	free(tfilters);
}


/**
 * Upon /os reload call the routines for reloading the configuration directives
 **/
int reload_config(int argc, char **argv) {
	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			alog("[\002text_act\002] Reloading configuration directives...");
			load_config();
		}
	}
	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

/* EOF */
