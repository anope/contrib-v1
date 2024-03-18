/*------------------------------------------------------------
* Name:    os_statsonid
* Author:  LEthaLity 'Lee' <lee@lethality.me.uk>
* Date:    21st November 2009
* Version: 1.1
* ------------------------------------------------------------
* Re-code of my 1.9.0 module of the same name, this is made to
* work on Anope-1.9.1 and higher. Tested on 1.9.1 with Unreal.
*
* This module displays some stats to service opers when they 
* identify.
* ------------------------------------------------------------
* This module doesn't add any new Commands, and require NO
* configuration.
*
* Changes:
* Changes in module api now mean the module will react to 
* NickIdentify event, and not overwrite NickServ Identify
* as previous.
* ------------------------------------------------------------
*/

#include "module.h"

#define AUTHOR "LEthaLity"
#define VERSION "1.1"

static Module *me;

class OS_StatsOnID : public Module
{
   public:
	OS_StatsOnID(const std::string &modname, const std::string &creator) : Module(modname, creator)
	{
		me = this;
		this->SetAuthor(AUTHOR);
		this->SetVersion(VERSION);
		this->SetType(THIRD); // This is a 3rd Party Module

		ModuleManager::Attach(I_OnNickIdentify, this); // Module listens to Nick Identifying

	}

	void OnNickIdentify(User *u) // Beginning of main function when user identifies
	{
		time_t uptime = time(NULL) - start_time;
		int days = uptime / 86400, hours = (uptime / 3600) % 24,
		mins = (uptime / 60) % 60, secs = uptime % 60;
		long count, mem;
		int c;
		int is_servoper = u->nc && u->nc->IsServicesOper(); // New method for making sure stats appear to right ppl

		if (is_servoper) // user identifying is a services oper
		{
			u->SendMessage(s_OperServ, "******** \x2Services Stats Report\x2 ********");
			u->SendMessage(s_OperServ, "Services Version: Anope %s", version_number);
			u->SendMessage(s_OperServ, "Uptime: %d days, %d hours, %d mins, %d secs", days, hours, mins, secs);
			u->SendMessage(s_OperServ, "Current Users: %d; Maximum User Count: %d", usercnt, maxusercnt);
        	get_channel_stats(&count, &mem);
			u->SendMessage(s_OperServ, "Current Opers: %d; Current Channels: %ld", opcnt, count);
			get_core_stats(&count, &mem);
			c=count;
			get_chanserv_stats(&count, &mem);
			u->SendMessage(s_OperServ, "Registered: %d Nicks and %ld Chans", c, count );
			u->SendMessage(s_OperServ, "************* \x2Report End\x2 *************");
		}
	}
	
};

MODULE_INIT(OS_StatsOnID)

