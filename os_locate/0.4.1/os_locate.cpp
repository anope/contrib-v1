
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
 *  3rd Party License Information:
 * ================================
 * This Module includes GeoLite data created by MaxMind, available from
 * http://maxmind.com/. For further Details have a look into their License:
 * http://geolite.maxmind.com/download/geoip/database/LICENSE.txt
 *
 *
 *  Prerequisite:
 * ===============
 *  - You need libgeoip and the corresponding source packet to be installed to
 *    compile this Module.
 *  - You need the GeoLiteCity Database.
 *    (Download: http://geolite.maxmind.com/download/geoip/database/GeoLiteCity.dat.gz)
 * 
 *
 *  Installation:
 * ===============
 *  1. Copy this file to your "src/modules" dir.
 *  2. cd into your "build" dir.
 *  3. "cmake ."
 *  4. "make"
 *  5. "make install"
 *  6. Edit your services.conf file to autoload the module on services start.
 *
 *
 *  Configuration:
 * ================
 * This Module only having some configuration options. The first one defines
 * where your GeoLiteCity.dat file is loacated.
 *
 * The second configuration option enables or disables oper notification via globops
 * on command usage.
 *
 * The third one enables or disables a welcome Message for new connecting users
 * which showes them where they are.
 *
 *
 *  Known Issues:
 * ===============
 *  - Services starting slow because it took some time to add the geodata to the userrecords at start.
 *    At my 300 user Live Network the services running slow while synching to the net. Lookup Speed is
 *    at my machine 10-15 users per second, so the initial netjoin took 25 seconds. After netjoin, services
 *    running at normal speed. The same "Problem" occours right after a netsplit when a server rejoins and
 *    a large amount of users reconnecting.
 *
 *   The Issue is planned to be fixed in a future release, but not nessesary the next one ;-)
 *
 *
 *  Changes:
 * ==========
 * 0.4.1 - öäü Issue solved (Thanks to DukePyrolator for the Tip.)
 *      
 *       - Alternative Memory Caching Method for slightly more performance.
 *
 *
 * 0.4.0 - Services could crash when "myGeoIPCityPath" had a wrong value. Resolved.
 *
 *       - Added "GEOFIND" feature. You can now search for People which are connected
 *         from a specific country, region or city.
 *
 *       - Renamed the LOCATE Command to GEOLOCATE to be more consistent in commandnames
 *         and their purpose.
 *
 *       - Changed the required services permission from operserv/locate to operserv/geolocate
 *         to reflect the purpose of the module.
 *
 *       - Added a welcome Message (Sent by the GlobalNoticer) which shows users from where they
 *         are connecting.
 *
 * 0.3.5 - Initial Release
 */

#include "module.h"
#include <GeoIP.h>
#include <GeoIPCity.h>
#include <string.h>
#define AUTHOR "Andre (Phantomal) Lanvermann"
#define VERSION "0.4.1"

/****************************************************************************/
/*                         MODULE CONFIGURATION                             */
/****************************************************************************/

// YOU MUST ENTER THE CORRECT PATH HERE!!!
#define myGeoIPCityPath "/home/phantomal/irc/geoipdb/GeoLiteCity.dat"

// If you do not want GlobOps been sent on lookup, use false instead of true
#define useGlobOps true

// Show users their location on connect
#define showLocationOnConnect true

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
	CommandOSLocate() : Command("geolocate", 2, 2, "operserv/geolocate")
	{
		// Load Geo IP Datenbank - used for Host and IP lookup
		gi = GeoIP_open(myGeoIPCityPath, GEOIP_MMAP_CACHE);
		GeoIP_set_charset(gi,1);
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
				ircdproto->SendGlobops(s_OperServ, "%s requested geographic location for nick %s",u->nick,params[1].c_str());

			User *target = finduser(params[1].c_str());
			if (!target) 
			{ 
				u->SendMessage(s_OperServ, "User %s not found",params[1].c_str()); 
				return MOD_CONT; 
			}


			const char* region;
			const char* countryname;
			const char* city;

			target->GetExt("geo_countryname", countryname);
			target->GetExt("geo_region", region);
			target->GetExt("geo_city", city);

			u->SendMessage(s_OperServ, "User %s is connecting from: %s,%s,%s",target->nick, city, region, countryname);
					
		}
		else if (params[0] == "HOST")
		{
			alog("os_locate: %s requested geographic location for host %s",u->nick,params[1].c_str());			

			if (useGlobOps)			
				ircdproto->SendGlobops(s_OperServ, "%s requested geographic location for host %s",u->nick,params[1].c_str());

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
				ircdproto->SendGlobops(s_OperServ, "%s requested geographic location for ip %s",u->nick,params[1].c_str());


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
		u->SendMessage(s_OperServ, "Syntax: GEOLOCATE NICK mask");
		u->SendMessage(s_OperServ, "GEOLOCATE HOST mask");
		u->SendMessage(s_OperServ, "GEOLOCATE IP mask");
		u->SendMessage(s_OperServ, " ");
		u->SendMessage(s_OperServ, "Allows Services Administrators to lookup the geographic");
		u->SendMessage(s_OperServ, "location of an user, a hostname or an ip address.");
		return true;
	}

	void OnSyntaxError(User *u)
	{
		u->SendMessage(s_OperServ, "Syntax: GEOLOCATE {NICK | HOST | IP} mask");
		u->SendMessage(s_OperServ, "/msg OperServ HELP GEOLOCATE for more information.");
	}
};



class CommandOSGeofind : public Command
{

 public:
	// 1. param: The name of the new comand 
	// 2. param: The minimum number of parameters the parser will require to execute this command
	// 3. param: The maximum number of parameters the parser will create, after max_params, all will be combined into the last argument.
	// 4. param: The required permission to access this command (you can invent new permissions if you wish)
	CommandOSGeofind() : Command("geofind", 2, 2, "operserv/geolocate")
	{
		
	}


	~CommandOSGeofind()
	{
		
	}


	CommandReturn Execute(User *u, std::vector<ci::string> &params)
	{
		
		ci::string cmd = params[0];

	
		if (params[0] == "COUNTRY")
		{			
			alog("os_locate: %s is searching for users from country %s",u->nick,params[1].c_str());			

			if (useGlobOps == true)			
				ircdproto->SendGlobops(s_OperServ, "%s is searching for users from %s",u->nick,params[1].c_str());


			u->SendMessage(s_OperServ, "Users connected from %s",params[1].c_str()); 

			User *target = firstuser();
			
			while (target)
			{
			
				const char* countryname;
				const char* countrycode;

				target->GetExt("geo_countryname", countryname);
				target->GetExt("geo_countrycode", countrycode);


				if (!stricmp(params[1].c_str(), countryname))
				{
					u->SendMessage(s_OperServ, "%s",target->nick); 
				}
				else if (!stricmp(params[1].c_str(), countrycode))
				{
					u->SendMessage(s_OperServ, "%s",target->nick); 
				}
					
				target = nextuser();
			}

			u->SendMessage(s_OperServ, "End of GeoFind List."); 

		}
		else if (params[0] == "REGION")
		{
			alog("os_locate: %s is searching for users from region %s",u->nick,params[1].c_str());			

			if (useGlobOps == true)			
				ircdproto->SendGlobops(s_OperServ, "%s is searching for users from %s",u->nick,params[1].c_str());


			u->SendMessage(s_OperServ, "Users connected from %s",params[1].c_str()); 

			User *target = firstuser();
			
			while (target)
			{
			
				const char* region;

				target->GetExt("geo_region", region);

				if (!stricmp(params[1].c_str(), region))
				{
					u->SendMessage(s_OperServ, "%s",target->nick); 
				}
					
				target = nextuser();
			}

			u->SendMessage(s_OperServ, "End of GeoFind List."); 
		}
		else if (params[0] == "CITY")
		{
			alog("os_locate: %s is searching for users from city %s",u->nick,params[1].c_str());			

			if (useGlobOps == true)			
				ircdproto->SendGlobops(s_OperServ, "%s is searching for users from %s",u->nick,params[1].c_str());


			u->SendMessage(s_OperServ, "Users connected from %s",params[1].c_str()); 

			User *target = firstuser();
			
			while (target)
			{
			
				const char* city;

				target->GetExt("geo_city", city);

				if (!stricmp(params[1].c_str(), city))
				{
					u->SendMessage(s_OperServ, "%s",target->nick); 
				}
					
				target = nextuser();
			}

			u->SendMessage(s_OperServ, "End of GeoFind List."); 
		}
		
		return MOD_CONT;
	}

	bool OnHelp(User *u, const ci::string &subcommand)
	{
		u->SendMessage(s_OperServ, "Syntax: GEOFIND COUNTRY mask");
		u->SendMessage(s_OperServ, "GEOFIND REGION mask");
		u->SendMessage(s_OperServ, "GEOFIND CITY mask");
		u->SendMessage(s_OperServ, " ");
		u->SendMessage(s_OperServ, "Allows Services Administrators to search for users which");
		u->SendMessage(s_OperServ, "are connecting from a specified location.");
		u->SendMessage(s_OperServ, " ");
		u->SendMessage(s_OperServ, "GEOFIND COUNTRY mask");
		u->SendMessage(s_OperServ, "You can use the full country name (i.e.: Germany) or the");
		u->SendMessage(s_OperServ, "short country code (i.e. DE) as mask.");
		return true;
	}

	void OnSyntaxError(User *u)
	{
		u->SendMessage(s_OperServ, "Syntax: GEOFIND {COUNTRY | REGION | CITY} mask");
		u->SendMessage(s_OperServ, "/msg OperServ HELP GEOFIND for more information.");
	}
};


class OSLocate : public Module
{
  private:
	GeoIP * gi;  

  public:

	OSLocate(const std::string &modname, const std::string &creator) : Module(modname, creator)
	{
		this->SetAuthor(AUTHOR); 
		this->SetVersion(VERSION);
		this->SetType(THIRD);
		this->AddCommand(OPERSERV, new CommandOSLocate());
		this->AddCommand(OPERSERV, new CommandOSGeofind());
		ModuleManager::Attach(I_OnUserConnect, this);

		// Load Geo IP Datenbank
		gi = GeoIP_open(myGeoIPCityPath, GEOIP_MMAP_CACHE);
		GeoIP_set_charset(gi,1);
	}
	
	~OSLocate()
	{
		if(gi)
			GeoIP_delete(gi);

	}

	void OperServHelp(User *u)
	{
		u->SendMessage(s_OperServ, "    GEOLOCATE   Locates a user, host or ip based on the Geo IP Database");
		u->SendMessage(s_OperServ, "    GEOFIND     Shows all users which are connected from a specified");
		u->SendMessage(s_OperServ, "                Country, Region or City.");
	}

	void OnUserConnect(User *u)  // called when a new user connects to the network
	{
		alog("os_locate Debug: User connecting: %s", u->nick);


		if (!gi) 
		{ 
			alog("os_locate: Error: Unable to open GeoLiteCity.dat.");

			u->SendMessage(s_OperServ, "Error: Unable to open GeoLiteCity.dat.");

			if (useGlobOps == true)			
				ircdproto->SendGlobops(s_OperServ, "Error: Unable to open GeoLiteCity.dat.");
		}
		else
		{
			GeoIPRecord *geo = GeoIP_record_by_name(gi, u->host);

			if (geo)
			{

				const char* region = GeoIP_region_name_by_code(geo->country_code, geo->region);
				const char* countryname = geo->country_name;
				const char* countrycode = geo->country_code;
				const char* city = geo->city;
				char *value = new char;
				strcpy(value, "unknown");

				if (countryname == NULL) {
					u->Extend("geo_countryname", sstrdup(value));
				}
				else
				{
					u->Extend("geo_countryname", sstrdup(countryname));
				}


				if (countrycode == NULL) {
					u->Extend("geo_countrycode", sstrdup(value));
				}
				else
				{
					u->Extend("geo_countrycode", sstrdup(countrycode));
				}

				if (region == NULL) {
					u->Extend("geo_region", sstrdup(value));
				}
				else
				{
					u->Extend("geo_region", sstrdup(region));
				}

				if (city == NULL) {
					u->Extend("geo_city", sstrdup(value));
				}
				else
				{
					u->Extend("geo_city", sstrdup(city));
				}

				alog("os_locate: Debug - Extend Data for nick %s: Countryname: %s, Countrycode: %s, Region: %s, City: %s", u->nick, countryname, countrycode,region,city);

				// Message user his location if enabled

				if (showLocationOnConnect == true)
					u->SendMessage(s_GlobalNoticer, "You are connecting from: %s%s%s%s%s", geo->city ? geo->city : "",
										geo->city ? ", " : "",
										region ? region : "", region ? ", " : "",
										geo->country_name ? geo->country_name : "");

				GeoIPRecord_delete(geo); /* free allocated memory */
		
			}
			else
			{
				char *value = new char;
				strcpy(value, "unknown");

				u->Extend("geo_countryname", sstrdup(value));
				u->Extend("geo_countrycode", sstrdup(value));
				u->Extend("geo_region", sstrdup(value));
				u->Extend("geo_city", sstrdup(value));

			}
		}
	}
};

MODULE_INIT(OSLocate)

