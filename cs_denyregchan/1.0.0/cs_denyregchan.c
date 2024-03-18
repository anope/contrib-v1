#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: cs_denyregchan.c v1.0.0 04-09-2006 n00bie $"

/* -----------------------------------------------------------------------------------
 * Name 	: cs_denyregchan.c
 * Version 	: v1.0.0
 * Author 	: n00bie
 * Date 	: 04-09-2006
 * -----------------------------------------------------------------------------------
 * Description:
 * This module will restrict channel registration for a particular channel.
 *
 * Tested: Anope-1.7.13, 1.7.14, 1.7.15
 *
 * TODO: Deny registration for multiple channel(s)
 * -----------------------------------------------------------------------------------
 * This module have been requested by netstat on Anope's forum.
 * http://forum.anope.org/viewthread.php?tid=781#pid4300
 * -----------------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 1, or (at your option) any later version.
 * -----------------------------------------------------------------------------------
 * This module have 1 configurable option.
 */

/*********** Start of Configuration ***********/

// Enter below the Channel name to restrict
// for registration.

#define DenyRegChan "#CCPower"

/********** End of Configuration **************/

int do_denyreg(User *u);
int AnopeInit(int argc, char **argv)
{
	Command *c;
	c = createCommand("REGISTER", do_denyreg, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(CHANSERV, c, MOD_HEAD);
	alog("%s: Channel registration for '%s' is now forbidden.", s_ChanServ, DenyRegChan);
	alog("%s: cs_denyregchan: Successfully loaded module.", s_ChanServ);
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("%s: cs_denyregchan: Module unloaded.", s_ChanServ);
}

int do_denyreg(User *u)
{
	ChannelInfo *ci;
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	ci = cs_findchan(chan);
	if (!stricmp(chan, DenyRegChan)) {
		notice_lang(s_ChanServ, u, CHAN_MAY_NOT_BE_REGISTERED, chan);
		free(chan);
		return MOD_STOP;
	}
	return MOD_CONT;
}

/* EOF */

