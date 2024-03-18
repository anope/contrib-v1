#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: bs_restrict_kb.c v1.0.0 19-02-2007 n00bie $"

/* ------------------------------------------------------------------------------------
 * Name		: bs_restrict_kb
 * Author	: n00bie
 * Version	: 1.0.0
 * Date		: 19th February, 2007
 * ------------------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 1, or (at your option) any later version.
 * ------------------------------------------------------------------------------------
 * Tested	: Anope-1.7.17, 1.7.18
 *
 * This module is requested by thermometer.
 * ------------------------------------------------------------------------------------
 * Description:
 *
 * This module will restrict the use of BotServ fantasy commands !kick, !k, !kickban !kb
 * against identified/opered Services Operator and above.
 * ------------------------------------------------------------------------------------
 * This module have no configurable option.
 */

int do_fantasy(int argc, char **argv);
int AnopeInit(int argc, char **argv)
{
	EvtHook *hook = NULL;
	int status = 0;
	hook = createEventHook(EVENT_BOT_FANTASY, do_fantasy);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("bs_restrict_kb%s: Unable to bind to EVENT_BOT_FANTASY.", MODULE_EXT);
		alog("bs_restrict_kb%s: Unloading module.", MODULE_EXT);
		return MOD_STOP;
	} else {
		alog("bs_restrict_kb%s: Successfully hooked to EVENT_BOT_FANTASY.", MODULE_EXT);
		alog("bs_restrict_kb%s: Module loaded and active.", MODULE_EXT);
	}
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    return MOD_CONT;
}

void AnopeFini(void)
{
	alog("bs_restrict_kb%s: Module Unloaded.", MODULE_EXT);
}

int do_fantasy(int argc, char **argv)
{
	User *u, *u2;
	ChannelInfo *ci;
	char *target = NULL;
	//NickAlias *na;
	//NickCore *nc = NULL;
	int i = 0;
	if (argc >= 4) {
		target = myStrGetToken(argv[3], ' ', 0);
	}
	if (!(ci = cs_findchan(argv[2])))
		return MOD_CONT;
	if (!(u = finduser(argv[1])))
		return MOD_CONT;
	u2 = finduser(target);
	if (stricmp(argv[0], "k") == 0) {
		if (!target || !u2) {
			return MOD_CONT;
		} else {
			if (is_services_oper(u2)) {
				if (target) free(target);
				return MOD_STOP;
			}
		}
		/*
		for (i = 0; i < RootNumber; i++) {
			if (stristr(target, ServicesRoots[i])) {
				return MOD_STOP;
			}
		}
		for (i = 0; i < servadmins.count && (nc = servadmins.list[i]); i++) {
			if (stristr(target, nc->display)) {
				return MOD_STOP;
			}
		}
		for (i = 0; i < servopers.count && (nc = servopers.list[i]); i++) {
			if (stristr(target, nc->display)) {
				return MOD_STOP;
			}
		}
		*/
	} else if (stricmp(argv[0], "kick") == 0) {
		if (!target || !u2) {
			return MOD_CONT;
		} else {
			if (is_services_oper(u2)) {
				if (target) free(target);
				return MOD_STOP;
			}
		}
	} else if (stricmp(argv[0], "kb") == 0) {
		if (!target || !u2) {
			return MOD_CONT;
		} else {
			if (is_services_oper(u2)) {
				if (target) free(target);
				return MOD_STOP;
			}
		}
	} else if (stricmp(argv[0], "kickban") == 0) {
		if (!target || !u2) {
			return MOD_CONT;
		} else {
			if (is_services_oper(u2)) {
				if (target) free(target);
				return MOD_STOP;
			}
		}
	}
	if (target)
		free(target);
	return MOD_CONT;
}

/* EOF */
