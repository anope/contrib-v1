#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: ns_joinhelpchan.c v1.0 06-09-2006 spa $"

/* -------------------------------------------------------------------------------
 * Name		: ns_joinhelpchan.c
 * Author	: n00bie
 * Version	: 1.0.0
 * Date		: 06-09-2006
 * -------------------------------------------------------------------------------
 * Tested & Supported: Unreal3.2.5 & Anope-1.7.13 above
 *
 * Description: When a user having HalfOP access and above on the Network Help
 * channel identify his nick to services, this module will forcefully join the
 * user to the network Help Channel (which is defined on services.conf).
 * If the user have no access on the Help Channel nothing is done.
 *
 * NOTE: In order for this module to work HelpChannel must be enabled or defined
 * in services.conf
 * --------------------------------------------------------------------------------
 * This module have no configurable option.
*/

int joinhelp(User *u);
int AnopeInit(int argc, char **argv)
{
	Command *c;
	c = createCommand("ID", joinhelp, NULL, -1, -1, -1, -1, -1);
	alog("NickServ: Providing command for '/msg NickServ ID'");
	moduleAddCommand(NICKSERV, c, MOD_TAIL);
	c = createCommand("IDENTIFY", joinhelp, NULL, -1, -1, -1, -1, -1);
	alog("NickServ: Providing command for '/msg NickServ IDENTIFY'");
	moduleAddCommand(NICKSERV, c, MOD_TAIL);
	if (!HelpChannel) {
		alog("\002HelpChannel\002 is not enabled in services.conf. Unloading module.");
		return MOD_STOP;
	} else {
		alog("%s: ns_joinhelpchan: Successfully loaded module.", s_NickServ);
	}
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

int joinhelp(User * u)
{
	ChannelInfo *ci = cs_findchan(HelpChannel);
	if (check_access(u, ci, CA_HALFOP) || (check_access(u, ci, CA_HALFOPME))) {
		send_cmd(NULL, "SVSJOIN %s %s", u->nick, HelpChannel);
	}
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("%s: ns_joinhelpchan%s: Module Unloaded.", s_NickServ, MODULE_EXT);
}

/* EOF */

