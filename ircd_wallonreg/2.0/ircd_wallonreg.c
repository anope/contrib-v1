/**
 * ------------------------------------------------------------
 * Name: ircd_wallonreg
 * Author: Viper
 * Date: 28/07/2006
 * ------------------------------------------------------------
 * Based on the original code of Brandx, updated to use events.
 *
 * This module will alert ircops when someone registers a
 * nickname or channel. My many thanks goes to DukePyrolator
 * for helping me code this.
 * ------------------------------------------------------------
 **/

#include "module.h"

#define AUTHOR "Viper"
#define VERSION "2.0"

int chan_regged(int ac, char **av);
int nick_regged(int ac, char **av);


int AnopeInit(int argc, char **argv) {
	EvtHook *hook;

	hook = createEventHook(EVENT_CHAN_REGISTERED, chan_regged);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[ircd_wallonreg] Can't hook to EVENT_CHAN_REGISTERED event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_NICK_REGISTERED, nick_regged);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[wallonreg] Can't hook to EVENT_NICK_REGISTERED event");
		return MOD_STOP;
	}

	alog("[ircd_wallonreg] Module ircd_wallonreg loaded and active.");

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	return MOD_CONT;
}

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
