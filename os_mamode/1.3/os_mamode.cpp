/**
 * ------------------------------------------------------------
 * Name:    os_mamode
 * Author:  LEthaLity 'Lee' (lethality@anope.org)
 * Date:    14th September 2011
 * Version: 1.3
 * ------------------------------------------------------------
 * This module was designed and tested on the recent Anope-1.9.5
 * release.
 * ------------------------------------------------------------
 * Description:
 * This module provides an OperServ Command allowing Services
 * Admin with the correct OperServ Flag to apply MassModes.
 * The channel modes will be applied to ALL Channels.
 * 
 * ------------------------------------------------------------
 * Usage:
 * Command /operserv mamode +/-modes.
 * Eg, /operserv mamode +m (Sets all Chans to +m)
 *
 * This command requires the operflag operserv/massmode
 * ------------------------------------------------------------
 * Changes:
 * 1.3 - Rewrite the mode setting and general update to work on 1.9.5
 * 1.2 - Rewrite of the 1.9.0 version so it works on later versions.
 * ------------------------------------------------------------
 * Configuration (Add the following to operserv.conf to enable the command)

 module { name = "os_mamode" }
 command { service = "OperServ"; name = "MAMODE"; command = "operserv/mamode"; permission = "operserv/massmode"; }

 * ------------------- End of Configuration -------------------
 **/

#include "module.h"

#define AUTHOR "LEthaLity"
#define VERSION "1.3"


class CommandOSMaMode : public Command
{
 public:
	CommandOSMaMode(Module *creator) : Command(creator, "operserv/mamode", 1, 1) // Users need operserv/massmode flag to be able to use this.
	{
		this->SetDesc(_("Set modes for ALL channels"));
		this->SetSyntax(_("\037modes\037"));
	}

	void Execute(CommandSource &source, const std::vector<Anope::string> &params)
	{
		User *u = source.u;
		const Anope::string &modes = params[0];

		if (modes.length() < 2)
		{
			this->SendSyntax(source);
			source.Reply(_("\2/msg OperServ HELP MAMODE\2 for more information."));
		}

		if (modes[0] == '+' || modes[0] == '-')
		{
			Log(LOG_ADMIN, u, this) << "setting " << modes << " on all channels.";
			ircdproto->SendGlobops(findbot(Config->OperServ), "%s used MAMODE setting %s on all channels", u->nick.c_str(), modes.c_str());
			for (channel_map::const_iterator it = ChannelList.begin(), it_end = ChannelList.end(); it != it_end; ++it)
				it->second->SetModes(findbot(Config->OperServ), false, "%s", modes.c_str());
		}
	}

	bool OnHelp(CommandSource &source, const Anope::string &subcommand)
	{
		this->SendSyntax(source);
		source.Reply(" ");
		source.Reply(_("Allows a Services Admin with the correct flag to\n"
			           "apply channel modes to ALL channels"));
		return true;
	}

};


class OSMaMode : public Module
{
	CommandOSMaMode commandosmamode;

 public:
	OSMaMode(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator, THIRD),
		commandosmamode(this)
	{
		this->SetAuthor(AUTHOR);
		this->SetVersion(VERSION);

		ModuleManager::RegisterService(&commandosmamode);
	}
};

MODULE_INIT(OSMaMode)
