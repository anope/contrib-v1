/**
 * -----------------------------------------------------------------------------
 * Name    : os_set_sa
 * Author  : Viper  <Viper@Absurd-IRC.net>
 * Date    : 06/01/2008  (Last update: 07/01/2008 )
 * Version : 1.1
 * -----------------------------------------------------------------------------
 * Tested     : Anope-1.7.20 + UnrealIRCd 3.2.6
 * -----------------------------------------------------------------------------
 *
 * As of Anope-1.7.20 OS SET is restricted to services roots.
 * This module makes OS SET SUPERADMIN available to SA again, as it used to be.
 *
 * -----------------------------------------------------------------------------
 *
 * Changelog:
 *
 *    1.1    Rewrite so all logic remains in the core...
 *
 *    1.0    Initial Release
 *
 *
 * -----------------------------------------------------------------------------
 **/

#include "module.h"
#define AUTHOR "Viper"
#define VERSION "1.1"

int do_set(User * u);

/**
 * Create the command, and tell anope about it.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv) {
	Command *c;

	alog("[\002os_set_sa\002] Loading module...");

	c = createCommand("SET", do_set, is_services_admin, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV, c, MOD_HEAD);

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	alog("[\002os_set_sa\002] Module loaded successfully...");

	return MOD_CONT;
}

/**
 * Unload the module
 **/
void AnopeFini(void) {
	alog("[\002os_set_sa\002] Unloading module...");
}


/* ------------------------------------------------------------------------------- */

/**
 * The /os set command with only the superadmin part.
 * @param u The user who issued the command
 * @param MOD_CONT to continue processing other modules, MOD_STOP to stop processing.
 **/
int do_set(User * u) {
	char *buffer, *option;

	buffer = moduleGetLastBuffer();
	option = myStrGetToken(buffer, ' ', 0);

	if (option && stricmp(option, "SUPERADMIN") == 0) {
		free(option);
		return MOD_CONT;
	} else if (!is_services_root(u)) {
		notice_lang(s_OperServ, u, ACCESS_DENIED);
		if (option)	free(option);
		return MOD_STOP;
	}

	if (option)	free(option);
	return MOD_CONT;
}

/* EOF */
