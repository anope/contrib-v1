#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: os_root_superadmin v1.0.0 13-04-2007 01:15PM n00bie $"

/* -----------------------------------------------------------------------------------
 * Name		: os_root_superadmin
 * Author	: n00bie
 * Version	: 1.0.0
 * Date		: 13th April, 2007
 * -----------------------------------------------------------------------------------
 * Description: This module will automatically set SuperAdmin ON upon identifying to
 * NickServ. This module will work even if SuperAdmin is Disabled on the services.conf
 * -----------------------------------------------------------------------------------
 * Tested: Anope-1.7.18
 * -----------------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 1, or (at your option) any later version.
 * -----------------------------------------------------------------------------------
 * This module have no configurable option.
 */

int do_superadmin(int argc, char **argv);
int AnopeInit(int argc, char **argv)
{
	EvtHook *hook = NULL;
	int status;
	hook = createEventHook(EVENT_NICK_IDENTIFY, do_superadmin);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("Unable to hook to EVENT_NICK_IDENTIFY. Unloading module [%d]", status);
		return MOD_STOP;
	} else {
		alog("os_root_superadmin: Module successfully loaded.");
	}
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    moduleSetType(THIRD);
    return MOD_CONT;
}

int do_superadmin(int argc, char **argv)
{
	User *u;
	u = finduser(argv[0]);
	if (is_services_root(u)) {
		u->isSuperAdmin = 1;
	}
	return MOD_CONT;
}

void AnopeFini(void)
{

}

/* EOF */
