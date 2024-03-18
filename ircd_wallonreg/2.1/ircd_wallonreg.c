/**
 * -----------------------------------------------------------------------------
 * Name: ircd_wallonreg
 * Author: Viper  <Viper@Absurd-IRC.net>
 * Date: 28/07/2006  (Last update: 21/10/2006)
 * Version 2.1
 * -----------------------------------------------------------------------------
 * Based on the original code of Brandx, updated to use events.
 *
 * This module will alert ircops when someone registers a
 * nickname or channel. My many thanks goes to DukePyrolator
 * for helping me code this.
 * -----------------------------------------------------------------------------
 *
 * Changes:
 *
 *  2.1  Updated to work compile correctly on Windows
 *
 *  2.0  First release by me
 *
 *
 * -----------------------------------------------------------------------------
 **/

#include "module.h"

#define AUTHOR "Viper"
#define VERSION "2.1"


/* Functions */
int chan_regged(int ac, char **av);
int nick_regged(int ac, char **av);


/* ------------------------------------------------------------------------------- */

/**
 * Create the command, and tell anope about it.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv) {
	EvtHook *hook;

	hook = createEventHook(EVENT_CHAN_REGISTERED, chan_regged);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002ircd_wallonreg\002] Can't hook to EVENT_CHAN_REGISTERED event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_NICK_REGISTERED, nick_regged);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002wallonreg\002] Can't hook to EVENT_NICK_REGISTERED event");
		return MOD_STOP;
	}

	alog("[ircd_wallonreg] Module ircd_wallonreg loaded and active.");

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	return MOD_CONT;
}

/**
 * Unload the module
 **/
void AnopeFini(void) {
	alog("[ircd_wallonreg] Unloading module...");
}


/* ------------------------------------------------------------------------------- */

int chan_regged(int ac, char **av) {
	ChannelInfo *ci;
	User *u;

	if (ac != 1)
		return MOD_CONT;

	if ((ci = cs_findchan(av[0]))) {
		if ((u = finduser(ci->founder->display)))
			wallops(s_ChanServ, "New channel ( %s ) registered by %s!%s@%s",  ci->name, u->nick, u->username, common_get_vhost(u));
	}

	return MOD_CONT;
}

int nick_regged(int ac, char **av) {
	User *u;
	if (ac != 1)
		return MOD_CONT;

	if ((u = finduser(av[0])))
		wallops(s_NickServ, "New Nickname ( %s ) Registered by %s@%s", u->nick, u->username, common_get_vhost(u));

	return MOD_CONT;
}

/* EOF */
