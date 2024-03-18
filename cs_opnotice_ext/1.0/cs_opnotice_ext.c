/**
 * -----------------------------------------------------------------------------
 * Name: cs_opnotice_ext
 * Author: Viper  <Viper@Absurd-IRC.net>
 * Date: 21/11/2006  (Last update: 02/12/2006)
 * Version: 1.0
 * -----------------------------------------------------------------------------
 * Tested: Anope-1.7.17 + UnrealIRCd 3.2.5
 * -----------------------------------------------------------------------------
 * This module was based on the idea of cs_opnoticehalfop by SGR.
 * We simply check if opnotice is enabled, if it is, we also send notices about
 * voice, halfop, protect and admin, if supported by the IRCd.
 * -----------------------------------------------------------------------------
 *
 * Changes:
 *
 *  1.0  Initial release
 *
 *
 * -----------------------------------------------------------------------------
 **/


#include "module.h"
#define AUTHOR "Viper"
#define VERSION "1.0"

/* ------------------------------------------------------------------------------- */

/* Functions */
int do_notice(User * u, CSModeUtil *util, char *s);
int do_voice(User * u);
int do_devoice(User * u);
int do_halfop(User * u);
int do_dehalfop(User * u);
int do_protect(User * u);
int do_deprotect(User * u);
int do_admin(User * u);
int do_deadmin(User * u);


/* ------------------------------------------------------------------------------- */


/**
 * Create the command, and tell anope about it.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv) {
	Command *c;

	c = createCommand("VOICE", do_voice, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(CHANSERV, c, MOD_TAIL);
	c = createCommand("DEVOICE", do_devoice, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(CHANSERV, c, MOD_TAIL);
	if (ircd->halfop) {
		c = createCommand("HALFOP", do_halfop, NULL, -1, -1, -1, -1, -1);
		moduleAddCommand(CHANSERV, c, MOD_TAIL);
		c = createCommand("DEHALFOP", do_dehalfop, NULL, -1, -1, -1, -1, -1);
		moduleAddCommand(CHANSERV, c, MOD_TAIL);
	}
	if (ircd->protect) {
		c = createCommand("PROTECT", do_protect, NULL, -1, -1, -1, -1, -1);
		moduleAddCommand(CHANSERV, c, MOD_TAIL);
		c = createCommand("DEPROTECT", do_deprotect, NULL, -1, -1, -1, -1, -1);
		moduleAddCommand(CHANSERV, c, MOD_TAIL);
	}
	if (ircd->admin) {
		c = createCommand("ADMIN", do_admin, NULL, -1, -1, -1, -1, -1);
		moduleAddCommand(CHANSERV, c, MOD_TAIL);
		c = createCommand("DEADMIN", do_deadmin, NULL, -1, -1, -1, -1, -1);
		moduleAddCommand(CHANSERV, c, MOD_TAIL);
	}

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	alog("[\002cs_opnotice_ext\002] Module loaded...");

	return MOD_CONT;

}


/**
 * Unload the module
 **/
void AnopeFini(void) {
	alog("[\002cs_opnotice_ext\002] Unloading module...");
}


/* ------------------------------------------------------------------------------- */

int do_voice(User * u) {
	return do_notice(u, &csmodeutils[MUT_VOICE], "VOICE command used for %s by %s.");
}

int do_devoice(User * u) {
	return do_notice(u, &csmodeutils[MUT_DEVOICE], "DEVOICE command used for %s by %s.");
}

int do_halfop(User * u) {
	if (ircd->halfop)
		return do_notice(u, &csmodeutils[MUT_HALFOP], "HALFOP command used for %s by %s.");

	return MOD_CONT;
}

int do_dehalfop(User * u) {
	if (ircd->halfop)
		return do_notice(u, &csmodeutils[MUT_DEHALFOP], "DEHALFOP command used for %s by %s.");

	return MOD_CONT;
}

int do_protect(User * u) {
	if (ircd->protect)
		return do_notice(u, &csmodeutils[MUT_PROTECT], "PROTECT command used for %s by %s.");

	return MOD_CONT;
}

int do_deprotect(User * u) {
	if (ircd->protect)
		return do_notice(u, &csmodeutils[MUT_DEPROTECT], "DEPROTECT command used for %s by %s.");

	return MOD_CONT;
}

int do_admin(User * u) {
	if (ircd->admin)
		return do_notice(u, &csmodeutils[MUT_PROTECT], "ADMIN command used for %s by %s.");

	return MOD_CONT;
}

int do_deadmin(User * u) {
	if (ircd->admin)
		return do_notice(u, &csmodeutils[MUT_DEPROTECT], "DEADMIN command used for %s by %s.");

	return MOD_CONT;
}



/* ------------------------------------------------------------------------------- */

int do_notice(User * u, CSModeUtil *util, char *s) {
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	char *nick = myStrGetToken(buf, ' ', 1);

	Channel *c;
	ChannelInfo *ci;
	User *u2;

	int is_same;

	if (!chan) {
		struct u_chanlist *uc;

		for (uc = u->chans; uc; uc = uc->next) {
			if ((ci = uc->chan->ci) && !(ci->flags) & CI_VERBOTEN && is_founder(u, ci)) {
				if (ci->flags & CI_OPNOTICE)
					notice(whosends(ci), uc->chan->name, s, u->nick, u->nick);
			}
		}
	} else {

		if (!nick)
			nick = sstrdup(u->nick);

		is_same = (nick == u->nick) ? 1 : (stricmp(nick, u->nick) == 0);

		if (((c = findchan(chan)))  && (ci = c->ci) && (!(ci->flags & CI_VERBOTEN)) && (!(is_same ? !(u2 = u) :
			!(u2 = finduser(nick)))) && (is_on_chan(c, u2)) && (!(is_same ? !check_access(u, ci, util->levelself) :
			!check_access(u, ci, util->level))) && (!(*util->mode == '-' && !is_same && (ci->flags & CI_PEACE)
			&& (get_access(u2, ci) >= get_access(u, ci)))) && (!(*util->mode == '-' && is_protected(u2) && !is_same))) {

			if (ci->flags & CI_OPNOTICE)
				notice(whosends(ci), c->name, s, u2->nick, u->nick);
		}
	}

	if (chan)
		free(chan);
	if (nick)
	free(nick);

	return MOD_CONT;
}


/* EOF */
