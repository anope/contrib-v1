/**
 * -----------------------------------------------------------------------------
 * Name    : cs_statusupdate
 * Author  : Viper <Viper@Absurd-IRC.net>
 * Date    : 28/09/2007 (Last update: 30/09/2007)
 * Version : 2.0
 * -----------------------------------------------------------------------------
 * Requires    : Anope-1.7.19
 * Tested      : Anope 1.7.19 + UnrealIRCd 3.2.6
 * -----------------------------------------------------------------------------
 * This module will update a users' status on the channel when his entry
 * on the access list is changed.
 *
 * Currently this only affects the user on whom the access changes was performed,
 * not other users in the same nickname group.
 * Ignored users will not be given new modes, but modes will be taken if needed.
 * -----------------------------------------------------------------------------
 *
 * Changelog:
 *
 *   2.0  -  First release by Viper
 *           Started from scratch
 *
 * -----------------------------------------------------------------------------
 **/


#include "module.h"
#define AUTHOR "Viper"
#define VERSION "2.0"


/* Functions */
int access_add(int ac, char **av);
int access_change(int ac, char **av);

void give_correct_modes(Channel *c, User *u);


/* ------------------------------------------------------------------------------- */

/**
 * Create the command, and tell anope about it.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv) {
	EvtHook *hook;

	alog("[\002cs_statusupdate\002] Loading module...");

	if (!moduleMinVersion(1,7,18,1240)) {
		alog("[\002cs_statusupdate\002] Your version of Anope isn't supported. Please update to a newer release.");
		return MOD_STOP;
	}

	/* Hook to some events.. */
	hook = createEventHook(EVENT_ACCESS_ADD, access_add);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002cs_statusupdate\002] Can't hook to EVENT_ACCESS_ADD event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_ACCESS_CHANGE, access_change);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002cs_statusupdate\002] Can't hook to EVENT_ACCESS_CHANGE event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_ACCESS_DEL, access_change);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002cs_statusupdate\002] Can't hook to EVENT_ACCESS_DEL event");
		return MOD_STOP;
	}

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	alog("[\002cs_statusupdate\002] Module loaded successfully...");

	return MOD_CONT;
}


/**
 * Unload the module
 **/
void AnopeFini(void) {
	alog("[\002cs_statusupdate\002] Unloading module...");
}

/* ------------------------------------------------------------------------------- */

/**
 * We handle access add's
 **/
int access_add(int ac, char **av) {
	User *u;
	Channel *c;

	/* This shouldn't happen... */
	if (ac < 3)
		return MOD_CONT;
	if (!(c = findchan(av[0])))
		return MOD_CONT;
	if (!(u = finduser(av[2])))
		return MOD_CONT;
	if (!c->ci)
		return MOD_CONT;

	give_correct_modes(c, u);

	return MOD_CONT;
}

/**
 * We handle access changes
 **/
int access_change(int ac, char **av) {
	User *u;
	Channel *c;

	/* This shouldn't happen... */
	if (ac < 3)
		return MOD_CONT;
	if (!(c = findchan(av[0])))
		return MOD_CONT;
	if (!(u = finduser(av[2])))
		return MOD_CONT;
	if (!c->ci)
		return MOD_CONT;

	give_correct_modes(c, u);

	/* Still check whether user is allowed to keep +v if he has one. */
	if (chan_has_user_status(c, u, CUS_VOICE) && !check_access(u, c->ci, CA_AUTOVOICE)) {
		char **argv = scalloc(sizeof(char *) * 4, 1);
		char buf[BUFSIZE];
		int argc;

		argv[0] = c->name;
		if (ircdcap->tsmode) {
			snprintf(buf, BUFSIZE - 1, "%ld", (long int) time(NULL));
			argv[1] = buf;
			argv[2] = "-v";
			argv[3] = u->nick;
			argc = 4;

		} else {
			argv[1] = "-v";
			argv[2] = u->nick;
			argc = 3;
		}

		anope_cmd_mode(whosends(c->ci), c->name, "-v %s", u->nick);
		do_cmode(whosends(c->ci), argc, argv);

		free(argv);
	}

	return MOD_CONT;
}


/* ------------------------------------------------------------------------------- */

void give_correct_modes(Channel *c, User *u) {
	uint32 uflags, cflags;

	if (!c->ci)
		return;

	/* We will have to use a dirty hack to allow chan_set_correct_modes to work
	 * correctly. Other alternative is copying/rewriting it..
	 * Unlike what the name suggests, chan_set_correct_modes() is only suitable
	 * for use after a user joins. It checks whether autoop is on, if not, no modes
	 * are set..
	 * We pretend like AUTOOP is on so it does its job. When it is finished we
	 * restore the original flags from our backup.
	 * We also have to pretend secureops is turned on, otherwise no moves will be removed
	 * if the user is removed from the access list, yet we want that in this case,
	 * even if it s off. Wehn finished, restore original flags from backup. */
	uflags = u->na->nc->flags;
	cflags = c->ci->flags;

	u->na->nc->flags &= ~NI_AUTOOP;
	c->ci->flags |= CI_SECUREOPS;

	chan_set_correct_modes(u, c, 1);

	u->na->nc->flags = uflags;
	c->ci->flags = cflags;
}

/* EOF */
