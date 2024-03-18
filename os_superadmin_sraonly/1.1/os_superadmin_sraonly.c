/**
 * -----------------------------------------------------------------------------
 * Name   : os_superadmin_sraonly
 * Author : Viper <Viper@Absurd-IRC.net>
 * Date   : 01/10/2006  (Last updated: 01/10/2006 )
 * -----------------------------------------------------------------------------
 * Tested: Anope 1.7.15 + UnrealIRCd 3.2.3
 * -----------------------------------------------------------------------------
 * This module should work on all Anope 1.7.x releases with all supported IRCDs
 *
 * This module will restrict the SET SUPERADMIN command to Services Root Administrators.
 *
 * -----------------------------------------------------------------------------
 * Changelog:
 *   - 1.1  Fixed memleak
 *   - 1.0  First Release
 **/

#include "module.h"
#define AUTHOR "Viper"
#define VERSION "1.1"

int my_do_set(User *u);

int AnopeInit(int argc, char **argv) {
	Command *c;
	c = createCommand("SET", my_do_set, is_services_admin,-1,-1,-1,-1,-1);
	moduleAddCommand(OPERSERV, c, MOD_HEAD);

	alog("[os_superadmin_sraonly] OperServ SET SUPERADMIN command is now restricted to Services Roots.");
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	alog("[os_superadmin_sraonly] This module has loaded and is now active.");
	return MOD_CONT;
}

void AnopeFini(void) {
	alog("[os_superadmin_sraonly] The SET SUPERADMIN command is no longer restricted to Services Roots.");
}

int my_do_set(User *u) {
	char *buffer, *option;

	buffer = moduleGetLastBuffer();
	option = myStrGetToken(buffer, ' ', 0);

	if (option && (stricmp(option, "SUPERADMIN") == 0)) {
		if (!is_services_root(u)) {
			notice_lang(s_OperServ, u, PERMISSION_DENIED);
			if (option) free(option);
			return MOD_STOP;
		}
	}
	if (option) free(option);

	return MOD_CONT;
}
