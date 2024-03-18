#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: ns_greetoper.c v1.0.0, 19-10-2006 n00bie $"

/* ---------------------------------------------------------------------------------
 * Name		: ns_greetoper.c
 * Author	: n00bie
 * Version	: 1.0
 * Date		: 10th Oct, 2006
 * ---------------------------------------------------------------------------------
 * Description:
 *
 * Services Opers/Admins/Roots nickserv greet message will be displayed on a channel
 * upon joining a channel. The greet message will be displayed only on channel(s)
 * which they don't have access (in order to prevent double greeting from BotServ).
 * ---------------------------------------------------------------------------------
 * Tested and supported: Anope-1.7.15, 1.7.16, 1.7.17
 * ---------------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 1, or (at your option) any later version.
 * ---------------------------------------------------------------------------------
 * This module have no configurable option.
 */

int doGreet(int argc, char **argv);
int AnopeInit(int argc, char **argv)
{
	EvtHook *hook = NULL;
	int status = 0;
	hook = createEventHook(EVENT_JOIN_CHANNEL, doGreet);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("Unable to hook to EVENT_JOIN_CHANNEL. Unloading module [%d]", status);
		return MOD_STOP;
	} else {
		alog("%s: ns_greetoper: Successfully loaded module.", s_NickServ);
	}
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("%s: ns_greetoper: Module unloaded.", s_NickServ);
}

int doGreet(int argc, char **argv)
{
	User *u = finduser(argv[1]);
	NickAlias *na;
	ChannelInfo *ci;
	na = findnick(u->nick);
	if (!(ci = cs_findchan(argv[2]))) {
		return MOD_CONT;
	} else if (ci->flags & CI_VERBOTEN) {
		return MOD_CONT;
	} else if (ci->flags & CI_SUSPENDED) {
		return MOD_CONT;
	}
	if (!stricmp(argv[0], EVENT_STOP)) {
		if (is_services_oper(u)) {
			if (check_access(u, ci, CA_OPDEOP) || check_access(u, ci, CA_OPDEOPME)) {
				return MOD_CONT;
			} else {
				if (na->nc->greet) {
					anope_cmd_privmsg(whosends(ci), ci->name, "[%s] %s", u->nick, na->nc->greet);
				} else {
					return MOD_CONT;
				}
			}
		}
	}
	return MOD_CONT;
}

/* EOF */


