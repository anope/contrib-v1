/**
 * -----------------------------------------------------------------------------
 * Name    : ns_group_operonly
 * Author  : Viper  <Viper@Absurd-IRC.net>
 * Date    : 07/02/2008  (Last update: 08/02/2008)
 * Version : 1.0
 * -----------------------------------------------------------------------------
 * Tested: Anope-1.7.21 + UnrealIRCd 3.2.6
 * -----------------------------------------------------------------------------
 * This module will restrict the NICKSERV GROUP command to IRCops.
 *
 * Note that without a module providing SAGROUP there will be no way to group
 * nicknames for non IRCops since anope itself does not provide a tool for admins
 * to do this for them.
 * -----------------------------------------------------------------------------
 * Changes:
 *
 *  1.0    Initial Release
 *
 * -----------------------------------------------------------------------------
 **/


/*------------------------- Source - Don't change below --------------------------*/


#include "module.h"
#define AUTHOR "Viper"
#define VERSION "1.0"


/* Functions */
int oper_only_group(User * u);


/* ------------------------------------------------------------------------------- */

/**
 * Create the command, and tell anope about it.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv) {
	Command *c;

	alog("[\002ns_group_operonly\002] Loading module...");

	c = createCommand("GROUP", oper_only_group, is_oper,-1,-1,-1,-1,-1);
	if (moduleAddCommand(NICKSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002ns_group_operonly\002] Cannot hook to GROUP command...");
		return MOD_STOP;
	}

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	return MOD_CONT;
}


/**
 * Unload the module
 **/
void AnopeFini(void) {
	alog("[\002ns_group_operonly\002] Unloading module...");
}

/* ------------------------------------------------------------------------------- */


int oper_only_group(User * u) {
	return MOD_CONT;
}

/* EOF */
