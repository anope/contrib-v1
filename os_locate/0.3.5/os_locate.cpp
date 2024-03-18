
/* RequiredLibraries: GeoIP */

/* os_locate.c - Adds operserv geo ip lookup functions
 *
 * (C) 2003-2009 Andre (Phantomal) Lanvermann
 * Contact me at <phantomal@solidirc.com>
 *
 * This Module has been created with much help from
 * DukePyrolator. Thanks again :-)
 *
 * Send bug reports to the Module Coder instead of the anope
 * authors, because this is a third party module and not supported
 * by the anope dev team.
 *
 * 
 * Configuration:
 * This Module only having 2 configuration options. The first one defines
 * where your GeoLiteCity.dat file is loacated. If you don't have such a file,
 * you can download it at:
 * http://geolite.maxmind.com/download/geoip/database/GeoLiteCity.dat.gz
 *
 * The second configuration options let you decide wether to send glopops on
 * geographic lookup or not.
 */

#include "module.h"
#include <GeoIP.h>
#include <GeoIPCity.h>
#define AUTHOR "Andre (Phantomal) Lanvermann"
#define VERSION "0.3.5"

/****************************************************************************/
/*                         MODULE CONFIGURATION                             */
/****************************************************************************/

// YOU MUST ENTER THE CORRECT PATH HERE!!!
#define myGeoIPCityPath "/home/phantomal/irc/geolitecity/GeoLiteCity.dat"

// If you do not want GlobOps been sent on lookup, use false instead of true
#define useGlobOps true

/****************************************************************************/
/* DO NOT CHANGE ANYTHING BELOW THIS LINE UNLESS YOU KNOW WHAT YOU'RE DOING */
/****************************************************************************/

class CommandOSLocate : public Command
{
 private:
	GeoIP * gi;

 public:
	// 1. param: The name of the new comand 
	// 2. param: The minimum number of parameters the parser will require to execute this command
	// 3. param: The maximum number of parameters the parser will create, after max_params, all will be combined into the last argument.
	// 4. param: The required permission to access this command (you can invent new permissions if you wish)
	CommandOSLocate() : Command("locate", 2, 2, "operserv/locate")
	{
		// Load Geo IP Datenbank
		gi = GeoIP_open(myGeoIPCityPath, GEOIP_MEMORY_CACHE);
	}


	~CommandOSLocate()
	{
		if(gi)
			GeoIP_delete(gi);

	}


	CommandReturn Execute(User *u, std::vector<ci::string> &params)
	{
		ci::string cmd = params[0];

		if (params[0] == "NICK")
		{			
			alog("os_locate: %s requested geographic location for nick %s",u->nick,params[1].c_str());			

			if (useGlobOps == true)			
				ircdproto->SendGlobops(s_OperServ, "os_locate: %s requested geographic location for nick %s",u->nick,params[1].c_str());

			User *target = finduser(params[1].c_str());
			if (!target) 
			{ 
				u->SendMessage(s_OperServ, "User %s not found",params[1].c_str()); 
				return MOD_CONT; 
			}

			GeoIPRecord *geo = GeoIP_record_by_name(gi, target->host);
			if (geo)
			{
				const char* region = GeoIP_region_name_by_code(geo->country_code, geo->region);
				
				u->SendMessage(s_OperServ, "User %s is connecting from: %s%s%s%s%s",target->nick,
										geo->city ? geo->city : "", geo->city ? ", " : "",
										region ? region : "", region ? ", " : "",
										geo->country_name ? geo->country_name : "");

				GeoIPRecord_delete(geo); /* free allocated memory */
					
			}
			else
			{
				u->SendMessage(s_OperServ, "Unable to retrieve Geo Location Data for User %s",target->nick);
			}
		}
		else if (params[0] == "HOST")
		{
			alog("os_locate: %s requested geographic location for host %s",u->nick,params[1].c_str());			

			if (useGlobOps)			
				ircdproto->SendGlobops(s_OperServ, "os_locate: %s requested geographic location for host %s",u->nick,params[1].c_str());

			GeoIPRecord *geo = GeoIP_record_by_name(gi, params[1].c_str());
			if (geo)
			{
				const char* region = GeoIP_region_name_by_code(geo->country_code, geo->region);
				
				u->SendMessage(s_OperServ, "Host %s is located at: %s%s%s%s%s",params[1].c_str(),
										geo->city ? geo->city : "", geo->city ? ", " : "",
										region ? region : "", region ? ", " : "",
										geo->country_name ? geo->country_name : "");

				GeoIPRecord_delete(geo); /* free allocated memory */
					
			}
			else
			{
				u->SendMessage(s_OperServ, "Unable to retrieve Geo Location Data for Host %s",params[1].c_str());
			}

		}
		else if (params[0] == "IP")
		{

			alog("os_locate: %s requested geographic location for ip %s",u->nick,params[1].c_str());			

			if (useGlobOps)			
				ircdproto->SendGlobops(s_OperServ, "os_locate: %s requested geographic location for ip %s",u->nick,params[1].c_str());


			GeoIPRecord *geo = GeoIP_record_by_addr(gi, params[1].c_str());
			if (geo)
			{
				const char* region = GeoIP_region_name_by_code(geo->country_code, geo->region);
				
				u->SendMessage(s_OperServ, "IP Address %s is located at: %s%s%s%s%s",params[1].c_str(),
										geo->city ? geo->city : "", geo->city ? ", " : "",
										region ? region : "", region ? ", " : "",
										geo->country_name ? geo->country_name : "");

				GeoIPRecord_delete(geo); /* free allocated memory */
					
			}
			else
			{
				u->SendMessage(s_OperServ, "Unable to retrieve Geo Location Data for IP Address %s",params[1].c_str());
			}
		}
		
		return MOD_CONT;
	}

	bool OnHelp(User *u, const ci::string &subcommand)
	{
		u->SendMessage(s_OperServ, "Syntax: LOCATE NICK mask");
		u->SendMessage(s_OperServ, "LOCATE HOST mask");
		u->SendMessage(s_OperServ, "LOCATE IP mask");
		u->SendMessage(s_OperServ, " ");
		u->SendMessage(s_OperServ, "Allows Services Administrators to lookup the geographic");
		u->SendMessage(s_OperServ, "location of an user, a hostname or an ip address.");
		return true;
	}

	void OnSyntaxError(User *u)
	{
		u->SendMessage(s_OperServ, "Syntax: LOCATE {NICK | HOST | IP} mask");
		u->SendMessage(s_OperServ, "/msg OperServ HELP LOCATE for more information.");
	}
};



class OSLocate : public Module
{
  public:

	OSLocate(const std::string &modname, const std::string &creator) : Module(modname, creator)
	{
		this->SetAuthor(AUTHOR); 
		this->SetVersion(VERSION);
		this->SetType(THIRD);
		this->AddCommand(OPERSERV, new CommandOSLocate());
	}
	
	void OperServHelp(User *u)
	{
		u->SendMessage(s_OperServ, "    LOCATE      Locates a user, host or ip based on the Geo IP Database");
	}

};

MODULE_INIT(OSLocate)

