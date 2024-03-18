#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: bs_noticeguest.c v1.0.0 25-09-2006 n00bie $"

/* ------------------------------------------------------------------------------------
 * Name		: bs_noticeguest.c
 * Author	: n00bie
 * Version	: 1.0
 * Date		: 25th Sept, 2006
 * ------------------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 1, or (at your option) any later version.
 * ------------------------------------------------------------------------------------
 * Tested	: Anope-1.7.17, 1.7.18
 *
 * This module have been requested by Blake81 on Anope's forum.
 * http://forum.anope.org/viewthread.php?tid=204#pid1072
 * Yeah i know its an old thread but i wanted to give it a try anyway and here we go ^^
 * ------------------------------------------------------------------------------------
 * Description:
 *
 * If a Guest user matching the NSGuestNickPrefix from services.conf joined a channel,
 * BotServ bot will give a /NOTICE to the Guest user using a pre-defined messages which
 * is settable on services.conf (BSGuestNotice)
 * ------------------------------------------------------------------------------------
 * Usage:
 *
 * This module have 1 configurable option.
 *
 * ----- Put this lines on your services.conf -----------------------------------------

 # BSGuestNotice [OPTIONAL]
 # When defined, BotServ Bot will notice a Guest nick/user
 # joining the channel with the messages below.

 BSGuestNotice "Guest nick are not welcomed on this channel. Please change your nick to a suitable nick."

 * ----- End of Configuration ----------------------------------------------------------
 */

char *BSGuestNotice;
int LoadConfig(void);
int do_reload(int argc, char **argv);
int do_GuestNotice(int argc, char **argv);
int AnopeInit(int argc, char **argv)
{
	EvtHook *hook = NULL;
	int status;
	hook = createEventHook(EVENT_JOIN_CHANNEL, do_GuestNotice);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("Unable to hook to EVENT_JOIN_CHANNEL. Unloading module [%d]", status);
		return MOD_STOP;
	}
	hook = createEventHook(EVENT_RELOAD, do_reload);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("Unable to hook to EVENT_RELOAD. Unloading module [%d]", status);
		return MOD_STOP;
	}
	if (!LoadConfig()) {
		return MOD_STOP;
	}
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void)
{
	if (BSGuestNotice) {
		free(BSGuestNotice);
	}
	alog("%s: bs_noticeguest: Module unloaded.", s_BotServ);
}

int do_reload(int argc, char **argv)
{
    LoadConfig();
    return MOD_CONT;
}

int LoadConfig(void)
{
    int i;
    int retval = 1;
    Directive confvalues[][1] = {
        {{"BSGuestNotice", {{PARAM_STRING, 0, &BSGuestNotice}}}},
    };
    for (i = 0; i < 1; i++) {
        moduleGetConfigDirective(confvalues[i]);
    }
    if (!BSGuestNotice) {
        alog("bs_noticeguest: \2BSGuestNotice\2 message is missing on services.conf");
        retval = 0;
    } else {
		alog("bs_noticeguest: \2BSGuestNotice\2 message set to: '%s'", BSGuestNotice);
    }
    return retval;
}

int do_GuestNotice(int argc, char **argv)
{
	User *u = finduser(argv[1]);
	ChannelInfo *ci;

	if (argc < 3) {
		return MOD_CONT;
	}

	if (!BSGuestNotice) {
		return MOD_CONT;
	}

	if (!stricmp(argv[0], EVENT_STOP)) {
		if (!(ci = cs_findchan(argv[2]))) {
			return MOD_CONT;
		} else if ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED)) {
			return MOD_CONT;
		} else {
			if (ci->bi) {
				if (strstr(u->nick, NSGuestNickPrefix)) {
					notice(ci->bi->nick, u->nick, "%s", BSGuestNotice);
				} else {
					return MOD_CONT;
				}
			} else {
				return MOD_CONT;
			}
		}
	}
	return MOD_CONT;
}

/* EOF */
