#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: cs_umodeq.c v1.0.0 19-08-2007 n00bie $"

/* 
** This is a simple module which will add
** umode +q (/helpop ?umodes) to ChanServ.
**
** Module requested by PTone
** Module is for UnrealIRCd ONLY.
*/

int AnopeInit(int argc, char **argv)
{
	if (!stricmp(IRCDModule, "unreal32")) {
		if (s_ChanServ) {
			send_cmd(NULL, "SVSMODE %s +q", s_ChanServ);
			if (LogChannel) {
				alog("Changing %s umode to: %sq", s_ChanServ, ircd->chanservmode);
			}
		}
	} else {
		alog("IRCd not supported.");
		return MOD_STOP;
	}
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void)
{
	/* Restore ChanServ umode to its default mode */
	if (s_ChanServ) {
		send_cmd(NULL, "SVSMODE %s -q%s", s_ChanServ, ircd->chanservmode);
		if (LogChannel) {
			alog("Setting %s umode to default: %s", s_ChanServ, ircd->chanservmode);
		}
	}
}
/* EOF */
