/*------------------------------------------------------------
* Name:    os_statsonid
* Author:  LEthaLity 'Lee' <lee@lethality.me.uk>
* Date:    25th August 2009
* Version: 1.0.0
* ------------------------------------------------------------
* This module is a port/rewrite of Certus' ns_identify_stats.
* On identify, ServiceAdmin will be presented with some stats
* such as uptime, users, channels etc. Functions minimal at
* present until I maybe add config options to allow selection
* of stats, and let it show more.
*
* ------------------------------------------------------------
* This module doesn't add any new Commands, and require NO
* configuration.
* This module will only work on Anope-1.9.0
*
* ------------------------------------------------------------
*/

#include "module.h"

#define AUTHOR "LEthaLity"
#define VERSION "1.0.0"

static Module *me;

int show_stats(User *u);

class OSStatsOnID : public Module
{
	public:
		OSStatsOnID(const std::string &modname, const std::string &creator) : Module(modname, creator)
		{
			
			me = this;
			Command *c;

			c = createCommand("identify", show_stats, NULL, -1, -1, -1, -1, -1); // Adding our function to NS identify
			this->AddCommand(NICKSERV, c, MOD_TAIL);

            this->SetAuthor(AUTHOR);
            this->SetVersion(VERSION);
		}
};

/* No reason to change anything below with how the module works in this version as it works fine */
int show_stats(User *u) {
	time_t uptime = time(NULL) - start_time;
	int days = uptime / 86400, hours = (uptime / 3600) % 24,
	mins = (uptime / 60) % 60, secs = uptime % 60;
	long count, mem;
	int c;

	if (u && is_services_admin(u)) { // Check if identified user is services admin
		notice_user(s_OperServ, u, "******** \x2Services Stats Report\x2 ********");
		notice_user(s_OperServ, u, "Services Version: Anope %s", version_number);
		notice_user(s_OperServ, u, "Uptime: %d days, %d hours, %d mins, %d secs", days, hours, mins, secs);
		notice_user(s_OperServ, u, "Current Users: %d; Maximum User Count: %d", usercnt, maxusercnt);
        	get_channel_stats(&count, &mem);
		notice_user(s_OperServ, u, "Current Opers: %d; Current Channels: %ld", opcnt, count);
		get_core_stats(&count, &mem);
		c=count;
		get_chanserv_stats(&count, &mem);
		notice_user(s_OperServ, u, "Registered: %d Nicks and %ld Chans", c, count );
		notice_user(s_OperServ, u, "************* \x2Report End\x2 *************");
		return MOD_CONT;
		}
	return MOD_CONT;
}

MODULE_INIT("os_statsonid", OSStatsOnID)


