/**
 * -----------------------------------------------------------------------------
 * Name  : os_mode_sraonly
 * Author: Viper <Viper@Absurd-IRC.net>
 * Date  : 27/06/2006
 * -----------------------------------------------------------------------------
 * Tested: Anope 1.7.14 + UnrealIRCd 3.2.3
 * -----------------------------------------------------------------------------
 * This module should work on all Anope 1.7.x releases with all supported IRCDs
 *
 * This module will restrict the MODE command to Services Roots.
 *
 **/

#include "module.h"
#define AUTHOR "Viper"
#define VERSION "1.0"

int my_do_mode(User *u);

int AnopeInit(int argc, char **argv) {
	Command *c;
	c = createCommand("MODE", my_do_mode, is_services_root,-1,-1,-1,-1,-1);
	moduleAddCommand(OPERSERV, c, MOD_HEAD);
	alog("[os_mode_sraonly] OperServ MODE command is now restricted to Services Roots.");
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	alog("[os_mode_sraonly] This module has loaded and is now active.");
	return MOD_CONT;
}

void AnopeFini(void) {
	alog("[os_mode_sraonly] The MODE command is no longer restricted to Services Roots.");
}

int my_do_mode(User *u) {
	return MOD_CONT;
}
