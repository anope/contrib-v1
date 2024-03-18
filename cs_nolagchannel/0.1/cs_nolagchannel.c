//************************************************************************
//  Copyright © 2010, "Kyle R. Spier-Swenson"
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, in the latest version of the license.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//
//  Author:
//     "MrStonedOne"
//
//  Purpose:
//     Removes unreal's fake lag for any user in the selected channel with CS access above 3
//	(this is beta, doesn't work with xop, doesn't check that the user is identified, 
//		and doesn't remove nolag when user parts channel or is kicked)
//
//
//  Revision History
//  Too lazy to keep up this to date
//  
//
//************************************************************************

#include "module.h"
#define AUTHOR "MrStonedOne"
#define VERSION "$Id: ns_sitelink.c V 0.0.1 $"
int sl_nolag(int argc, char **argv);

//SETTINGS!!!!

#define NOLAGCHAN "#nolag"

//start code
int AnopeInit(int argc, char **argv) {
	Command *c;
	int status;
	EvtHook *hook;

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	hook = createEventHook(EVENT_JOIN_CHANNEL, sl_nolag); 
	status = moduleAddEventHook(hook);
	
	return MOD_CONT;

}

void AnopeFini(void) {
//nothing to do here
return;
}

int sl_nolag(int argc, char **argv) {
 	if (!stricmp(argv[2],NOLAGCHAN) && stricmp(argv[0], EVENT_START)) {
		User *u = NULL;
		Channel *c = NULL;
		u = finduser(argv[1]);
		c = findchan(argv[2]);

		if (!c || !c->ci)
			return; /* chan not registered */
		if (!u || !nick_identified(u))
			return;       /* user not found/identified */

		if (get_access_level(c->ci, u->na) >= 3) {
			send_cmd(NULL, ":OperServ svsnolag + %s", u->nick);
		}
	}
	return MOD_CONT;
}
