#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: ns_joinhelpchan.c v2.0.0 03-02-2008 n00bie $"

/* -------------------------------------------------------------------------------
 * Name		: ns_joinhelpchan.c
 * Author	: n00bie [n00bie@rediffmail.com]
 * Version	: 2.0.0
 * Date		: 6th Sep, 2006
 * Updated	: 3rd Feb, 2008
 * -------------------------------------------------------------------------------
 * Tested: Unreal3.2.7
 * 
 * Description: When a user having HalfOP access and above on the Network Help
 * channel identify his nick to services, this module will forcefully join the
 * user to the network Help Channel (which is defined on services.conf).
 * If the user have no access on the Help Channel nothing is done.
 *
 * NOTE: In order for this module to work HelpChannel must be enabled or defined
 * in services.conf
 * --------------------------------------------------------------------------------
 * Changelog:
 *	v1.0.0 - First Public Release.
 *	v1.0.1 - Minor Code Update.
 *			 Added support for InspIRCd, PTlink, UltimateIRCd2, ViagraIRCd.
 *  v2.0.0 - Cleaned up codes.
 * --------------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 1, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * --------------------------------------------------------------------------------
 * This module have no configurable option.
*/

int joinhelp(User *u);
int AnopeInit(int argc, char **argv)
{
	Command *c;
	int status = 0;
	c = createCommand("ID", joinhelp, NULL, -1, -1, -1, -1, -1);
	status = moduleAddCommand(NICKSERV, c, MOD_TAIL);
	c = createCommand("IDENTIFY", joinhelp, NULL, -1, -1, -1, -1, -1);
	status = moduleAddCommand(NICKSERV, c, MOD_TAIL);
	c = createCommand("SIDENTIFY", joinhelp, NULL, -1, -1, -1, -1, -1);
	status = moduleAddCommand(NICKSERV, c, MOD_TAIL);
	if (!moduleMinVersion(1,7,21,1341)) {
		alog("%s: ns_joinhelpchan: Sorry, your version of Anope is not supported. This module require Anope-1.7.21 (1341) or later. Please upgrade to a new version.", s_NickServ);
		return MOD_STOP;
	}
	if (!HelpChannel) {
		alog("%s: ns_joinhelpchan: \002HelpChannel\002 is not defined in services.conf. Unloading module.", s_NickServ);
		return MOD_STOP;
	}
	if (status == MOD_ERR_OK) {
		alog("%s: ns_joinhelpchan: Successfully loaded module.", s_NickServ);
	} else {
		return MOD_STOP;
	}
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

int joinhelp(User *u)
{
	ChannelInfo *ci = cs_findchan(HelpChannel);
	if (check_access(u, ci, CA_HALFOP) || check_access(u, ci, CA_HALFOPME)) {
		anope_cmd_svsjoin(s_NickServ, u->nick, HelpChannel, NULL);
	}
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("%s: ns_joinhelpchan%s: Module Unloaded.", s_NickServ, MODULE_EXT);
}

/* EOF */
