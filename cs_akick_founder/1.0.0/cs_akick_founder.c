#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "1.0.0-22/03/2008"

/***********************************************************************************
** Module: cs_akick_founder.c
** Author: n00bie
** Version: 1.0.0
** Released: Saturday March 03, 2008
**
** Description: Simple module which will NOT allow the channel founder nick to be added
** on the channel autokick list. Yea, yea, channel founder can delete himself from
** the akick list, but thats just not nice.. ^^
**
** Tested: Anope-1.7.21 (1341), FreeBSD, Windows
*************************************************************************************
** This program is free software; you can redistribute it and/or modify it under the
** terms of the GNU General Public License as published by the Free Software
** Foundation; either version 1, or (at your option) any later version. 
**
** This program is distributed in the hope that it will be useful, but WITHOUT ANY
** WARRANTY; without even the implied warranty of MERCHANTABILITY or 
** FITNESS for a PARTICULAR PURPOSE.
**
** See the GNU General Public License for more details.
*************************************************************************************
** This module have no configurable option.
*/

#define MyFree(x) if ((x) != NULL) { free(x); }
int do_akick(User *u);

/**
 * Create the command, and tell anope about it.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv)
{
    Command *c;
	int status = 0;
	c = createCommand("AKICK", do_akick, NULL, CHAN_HELP_AKICK, -1, -1, -1, -1);
    status = moduleAddCommand(CHANSERV, c, MOD_HEAD);
	if (status == MOD_ERR_OK) {
		alog("cs_akick_founder: module loaded.");
	} else {
		alog("cs_akick_founder: somethin isn't init right. Unloading...");
		return MOD_STOP;
	}
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    moduleSetType(THIRD);
    return MOD_CONT;
}

/**
 * Unload the module
 **/
void AnopeFini(void)
{
	alog("cs_akick_founder: module unloaded.");
}

/**
 * The /cs akick command.
 * @param u The user who issued the command
 * @param MOD_CONT to continue processing other modules, MOD_STOP to stop processing.
 **/
int do_akick(User *u)
{
	char *buf = moduleGetLastBuffer();
    char *chan = myStrGetToken(buf, ' ', 0);
    char *cmd = myStrGetToken(buf, ' ', 1);
    char *mask = myStrGetToken(buf, ' ', 2);
    char *reason = myStrGetTokenRemainder(buf, ' ', 3);
    ChannelInfo *ci;

	if (!cmd || (!mask && (!stricmp(cmd, "ADD") || !stricmp(cmd, "STICK")
		|| !stricmp(cmd, "UNSTICK") || !stricmp(cmd, "DEL")))) {
			syntax_error(s_ChanServ, u, "AKICK", CHAN_AKICK_SYNTAX);
			MyFree(chan);
			MyFree(cmd);
			return MOD_STOP;
    } else if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
		MyFree(chan);
		MyFree(cmd);
		MyFree(mask);
		MyFree(reason);
		return MOD_STOP;
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
		MyFree(chan);
		MyFree(cmd);
		MyFree(mask);
		MyFree(reason);
		return MOD_STOP;
    } else if (!check_access(u, ci, CA_AKICK) && !is_services_admin(u)) {
        notice_lang(s_ChanServ, u, ACCESS_DENIED);
		MyFree(chan);
		MyFree(cmd);
		MyFree(mask);
		MyFree(reason);
		return MOD_STOP;
    } else if (stricmp(cmd, "ADD") == 0) {
		NickAlias *na = findnick(mask);
		NickCore *nc = NULL;
		if (na) {
			nc = na->nc;
			if (na->nc == ci->founder) {
				notice_user(s_ChanServ, u, 
					"You cannot add \2%s\2 to akick list because (s)he's the channel founder.", mask);
				MyFree(chan);
				MyFree(cmd);
				MyFree(mask);
				MyFree(reason);
				return MOD_STOP;
			}
		}
	}
	MyFree(chan);
	MyFree(cmd);
	MyFree(mask);
	MyFree(reason);
	return MOD_CONT;
}

/* EOF */

