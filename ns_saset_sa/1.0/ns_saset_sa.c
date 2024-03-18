/**
 * -----------------------------------------------------------------------------
 * Name    : ns_saset_sa
 * Author  : Viper  <Viper@Absurd-IRC.net>
 * Date    : 18/01/2008  (Last update: 18/01/2008 )
 * Version : 1.0
 * -----------------------------------------------------------------------------
 * Tested  : Anope-1.7.21 + UnrealIRCd 3.2.6
 * -----------------------------------------------------------------------------
 *
 * As of Anope-1.7.20 NS SASET is (partially) available to services opers.
 * This module makes NS SASET restricted to SA again, as it used to be.
 *
 * Requested by "MM".
 * -----------------------------------------------------------------------------
 *
 * Changelog:
 *
 *    1.0    Initial Release
 *
 *
 * -----------------------------------------------------------------------------
 **/

#include "module.h"
#define AUTHOR "Viper"
#define VERSION "1.0"

int do_saset(User * u);

/**
 * Create the command, and tell anope about it.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv) {
	Command *c;

	alog("[\002ns_saset_sa\002] Loading module...");

	c = createCommand("SASET", do_saset, is_services_admin, -1, -1, -1, -1, -1);
	moduleAddCommand(NICKSERV, c, MOD_HEAD);

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	alog("[\002ns_saset_sa\002] Module loaded successfully...");

	return MOD_CONT;
}

/**
 * Unload the module
 **/
void AnopeFini(void) {
	alog("[\002ns_saset_sa\002] Unloading module...");
}


/* ------------------------------------------------------------------------------- */

int do_saset(User * u) {
	return MOD_CONT;
}

/* EOF */
