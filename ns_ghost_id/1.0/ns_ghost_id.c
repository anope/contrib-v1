/**
 * -----------------------------------------------------------------------------
 * Name        : ns_ghost_id
 * Author      : Viper  <Viper@Absurd-IRC.net>
 * Date        : 25/05/2008
 * Last update : 25/05/2008
 * Version     : 1.0
 * -----------------------------------------------------------------------------
 * Tested      : Anope-1.7.21 + UnrealIRCd 3.2.6
 * -----------------------------------------------------------------------------
 * This module adds extra functionality to the NickServ GHOST command so that
 * if someone executes a ghost with password and is not already identified for
 * a nick, NickServ will automatically recognize him as owner of the ghosted
 * nickname after changing nick.
 *
 * Note that this will only work on networks with NSNickTracking enabled in the
 * services.conf!
 *
 * This module is published under GPLv2.
 * -----------------------------------------------------------------------------
 *
 * Changelog:
 *
 *    1.0    Initial Release
 *
 * -----------------------------------------------------------------------------
 **/


#include "module.h"
#define AUTHOR "Viper"
#define VERSION "1.0"


/* Functions */
int do_ghost_id(User * u);


/* ------------------------------------------------------------------------------- */

/**
 * Create the command, and tell anope about it.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv) {
	Command *c;

	alog("[\002ns_ghost_id\002] Loading module...");

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	c = createCommand("GHOST", do_ghost_id, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV, c, MOD_TAIL) != MOD_ERR_OK) {
		alog("[\002ns_ghost_id\002] Cannot hook to GHOST command...");
		return MOD_STOP;
	}

	if (!NSNickTracking) {
		alog("[\002ns_ghost_id\002] NSNickTracking is \002DISABLED\002 but required by this module.");
		return MOD_STOP;
	}

	alog("[\002ns_ghost_id\002] Module loaded successfully...");

	return MOD_CONT;
}

void AnopeFini(void) {
	alog("[\002ns_ghost_id\002] Unloading module...");
}

/* ------------------------------------------------------------------------------- */

int do_ghost_id(User * u) {
	char *buffer, *nick, *pass;
	NickAlias *na;

	if (!NSNickTracking)
		return MOD_CONT;

	buffer = moduleGetLastBuffer();
	nick = myStrGetToken(buffer, ' ', 0);
	pass = myStrGetToken(buffer, ' ', 1);

	if (nick && pass && ((na = findnick(nick))) && !(na->status & NS_VERBOTEN) &&
			!(na->nc->flags & NI_SUSPENDED) && stricmp(nick, u->nick)) {

		/* Check user isn't already identified to a nick in the same group */
		if (!group_identified(u, na->nc)) {
			/* If user is using a registered nick, he may not want to replace the tracking or change
			 * to the nick he s ghosting, so don't replace nicktrack.. */
			if (!nick_identified(u)) {
				if (u->nickTrack) free(u->nickTrack);
				u->nickTrack = sstrdup(na->nc->display);

				alog("[ns_ghost_id] %s: %s!%s@%s identified for nick %s through GHOST.", s_NickServ, u->nick,
						u->username, u->host, nick);
			}
		}
	}

	return MOD_CONT;
}

/* EOF */
