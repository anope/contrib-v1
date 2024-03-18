/**
 * ------------------------------------------------------------
 * Name:    os_mamode
 * Author:  LEthaLity 'Lee' (lethality@anope.org)
 * Date:    6th September 2010
 * Version: 1.2
 * ------------------------------------------------------------
 * This module was designed and tested for 1.9-Git, it should 
 * works on latest Anope Git (238) and "should" work on 1.9.2-p2,
 * I give no guarantee
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
 * 1.2 - Rewrite of the 1.9.0 version so it works on later versions.
 * ------------------------------------------------------------
 **/

#include "module.h"

#define AUTHOR "LEthaLity"
#define VERSION "1.2"


class CommandOSMaMode : public Command
{
 public:
	CommandOSMaMode() : Command("MAMODE", 1, 1, "operserv/massmode") // Users need operserv/massmode flag to be able to use this.
	{
	}

	CommandReturn Execute(User *u, const std::vector<Anope::string> &params)
	{
		Anope::string modes = params[0];

		if (modes.length() < 2)
		{
			u->SendMessage(Config->s_OperServ, "Syntax: \2MAMODE +(-)modes\2");
			u->SendMessage(Config->s_OperServ, "\2/msg OperServ HELP MAMODE\2 for more information.");
			return MOD_CONT;
		}

		if (modes[0] == '+' || modes[0] == '-')
		{
			Log(OperServ, "operserv/massmode") << "MAMODE: Setting " << modes << " on all channels";
			ircdproto->SendGlobops(OperServ, "%s used MAMODE setting %s on all channels", u->nick.c_str(), modes.c_str());
			MassChannelModes(OperServ, modes);  // Use the same method that DefCon does to apply mass modes.
		}
		return MOD_CONT;
	}

	
	bool OnHelp(User *u, const Anope::string &subcommand)
	{
		u->SendMessage(Config->s_OperServ, "Allows a Services Admin with correct flag to");
		u->SendMessage(Config->s_OperServ, "apply channel modes to ALL registered channels");
		return true;
	}

	void OnSyntaxError(User *u, const Anope::string &subcommand)
	{
		u->SendMessage(Config->s_OperServ, "Syntax: \2MAMODE +(-)modes\2");
		u->SendMessage(Config->s_OperServ, "\2/msg OperServ HELP MAMODE\2 for more information.");
	}

};


class OSMaMode : public Module
{
	CommandOSMaMode commandosmamode;

 public:
	OSMaMode(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator)
	{
		this->SetAuthor(AUTHOR);
		this->SetVersion(VERSION);
		this->SetType(THIRD); //This is a 3rd party module, don't bug #anope for support :P

		this->AddCommand(OperServ, &commandosmamode);
	}

	void OnOperServHelp(User *u)
	{
		u->SendMessage(Config->s_OperServ, "    MAMODE    Set a mode on all registered Channels");
	}
};

MODULE_INIT(OSMaMode)
