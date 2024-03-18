#include "module.h"

/* ----------------------------------------------------------------------------------
 * Name    : ircd_guestnick.c
 * Author  : n00bie
 * Version : v1.1.0
 * Date    : 30th August, 2006
 * ----------------------------------------------------------------------------------
 * Description:
 *
 * When a users nick changed to 'NSGuestNickPrefix' from services.conf
 * this module will make them part all the channels they are in.
 * ----------------------------------------------------------------------------------
 * Tested & Supported: UnrealIRCd3.2.5 + Anope-1.7.17 only
 * ----------------------------------------------------------------------------------
 * Thanks Trystan ...
 *  <Trystan> and stricmp() will never match the NSGuestNickPrefix
 * ----------------------------------------------------------------------------------
 * This module have no configurable option.
 */

int guest_part(int argc, char **argv);
int AnopeInit(int argc, char **argv)
{
	EvtHook *hook = NULL;
	int status;
	hook = createEventHook(EVENT_CHANGE_NICK, guest_part);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("Unable to hook to EVENT_CHANGE_NICK. Unloading module [%d]", status);
		return MOD_STOP;
	} else {
		alog("Successfully loaded \2%s\2 nick /PART module.", NSGuestNickPrefix);
	}
	moduleAddAuthor("n00bie");
	moduleAddVersion("$Id: ircd_guestnick.c v1.1.0 30-08-2006 n00bie $");
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("Unloaded \2%s\2 nick /PART module.", NSGuestNickPrefix);
}

int guest_part(int argc, char **argv)
{
	if (argc >= 1) {
		if (strstr(argv[0], NSGuestNickPrefix)) {
			notice(s_NickServ, argv[0], "Guest nick are not welcomed on this network.");
	   		send_cmd(s_NickServ, "SVSJOIN %s 0", argv[0]);
	  	} 
  	} 
	return MOD_CONT;
}

/* EOF */
