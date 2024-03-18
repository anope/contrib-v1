/*------------------------------------------------------------
* Name:    os_statsonid
* Author:  LEthaLity 'Lee' <lee@lethality.me.uk>
* Date:    16th January 2010
* Version: 1.2
* Anope Versions: Anope-1.8.x
* ------------------------------------------------------------
* Back-Port/Rewrite of my os_statsonid for anope-1.9.x which
* itself was a port from Certus' ns_identify_stats. (Added
* some more features and stats.) When a Services Administrator
* identifies they are presented with some Services stats.
* ------------------------------------------------------------
* This module doesn't add any new Commands, and require NO
* configuration.
* ------------------------------------------------------------
* Changes:
* States the ServicesServer Name
* Added akill count on request
* TODO:
* Number of HostServ Waiting Requests
*/

#include <stdio.h>
#include "module.h"

#define AUTHOR "LEthaLity"
#define VERSION "1.2"

int do_stats(User *u);

void AnopeInit(void) {
	Command *c;
	c=createCommand("IDENTIFY", do_stats, NULL, -1,-1,-1,-1,-1);
	alog("Loading os_statsonid [Status: %d]",moduleAddCommand(NICKSERV,c,MOD_TAIL));
}

void AnopeFini(void) { //Module unloaded
	alog("Unloading os_statsonid");
}

int do_stats(User *u) { //Main function - stats gathering/calculations below changed little
	time_t uptime = time(NULL) - start_time;
	int days = uptime / 86400, hours = (uptime / 3600) % 24,
	mins = (uptime / 60) % 60, secs = uptime % 60;
	long count, mem;
	int c;

	if (u && is_services_admin(u)) {
		notice(s_OperServ, u->nick, "************************ \2Status Report\2 ************************");
		notice(s_OperServ, u->nick, "\2Services name:\2 %s; \2Version\2 Anope %s", ServerName, version_number);
		notice(s_OperServ, u->nick, "Uptime: %d days, %d hours, %d mins, %d secs", days, hours, mins, secs);
		notice(s_OperServ, u->nick, "Current Users: %d; Maximum User Count: %d", usercnt, maxusercnt);
        	get_channel_stats(&count, &mem);
		notice(s_OperServ, u->nick, "Current Opers: %d; Current Channels: %ld", opcnt, count);
		get_core_stats(&count, &mem);
		c=count;
		get_chanserv_stats(&count, &mem);
		notice(s_OperServ, u->nick, "Registered: %d Nicks and %ld Chans", c, count );
		notice(s_OperServ, u->nick, "Current aKills: %d", akills.count);
		notice(s_OperServ, u->nick, "************************* \2Report End\2 *************************");
		return MOD_CONT;
	}
	return MOD_CONT;
}






