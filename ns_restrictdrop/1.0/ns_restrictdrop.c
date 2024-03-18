/*------------------------------------------------------------
* Name:    ns_restrictdrop
* Author:  LEthaLity 'Lee' <lee@lethality.me.uk>
* Version: 1.0
* Date:    10th January 2010
* ------------------------------------------------------------
* This Module takes away the NickServ DROP command from the
* normal user, allowing only services admin to use it.
* ------------------------------------------------------------
* This module is made for Anope-1.8.x Tested on 1.8.2SVN
* ------------------------------------------------------------
* No Configurable options
* ------------------------------------------------------------
*/

#include "module.h"

#define AUTHOR "LEthaLity"
#define VERSION "1.0"

int ns_drop(User * u);

int AnopeInit(int argc, char **argv)
{
	Command *c;

	c = createCommand("DROP",ns_drop,NULL,-1,-1,-1,-1,-1);
	moduleAddCommand(NICKSERV, c, MOD_HEAD);

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	alog("Loaded ns_restrictdrop: Only Service Admins and Root will be able to use NickServ Drop");

	return MOD_CONT;
}

void AnopeFini()
{
	alog("ns_restrictdrop: Unloading Module - Users will be able to drop their nicknames again");
}

int ns_drop(User * u)
{
	if (!(is_services_admin(u))) {
		notice_user(s_NickServ, u, "Use of NickServ DROP has been restricted to Service Administrators");
		return MOD_STOP;
	}
	return MOD_CONT;
}



