/*--------------------------------------------------------------
* Name:    ns_massset
* Author:  LEthaLity 'Lee' <lethality@anope.org>
* Date:    21th November 2011
* Version: 0.1
* --------------------------------------------------------------
* This module provides the ability for a Services Oper with the
* nickserv/massset privilege to change NickServ settings for 
* all registered users.
* This module is usually only loaded when needed, to override a
* setting you wrongly enabled/disabled in the configs default
* options, and wish to put right. Note that using this module
* some time down the line, undoing a users desired settings may
* "annoy" some.
* --------------------------------------------------------------
* This module provides the following command:
* /msg NickServ MASSSET <option> param
* Option can be one of KILL, KILLQUICK, SECURE and AUTOOP, the
* available params for the options are ON or OFF.*
* --------------------------------------------------------------
* Configuration: nickserv.conf
module { name = "ns_massset" }
command { service = "NickServ"; name = "MASSSET"; command = "nickserv/massset"; permission = "nickserv/massset"; }
* --------------------------------------------------------------
*/

#include "module.h"

#define AUTHOR "LEthaLity"
#define VERSION "0.1"


class CommandNSMassSet : public Command
{
public:
	CommandNSMassSet(Module *creator) : Command(creator, "nickserv/massset", 2, 2) // Users need nickserv/massset flag to be able to use this.
	{
		this->SetDesc(_("Set options for ALL registered users"));
		this->SetSyntax(_("\037option\037 \037parameter\037"));
	}

	void Execute(CommandSource &source, const std::vector<Anope::string> &params)
	{
		const Anope::string &option = params[0]; // Our SET option, Kill, AutoOP, etc
		const Anope::string &param = params[1];  // Setting for the above option - on/off
		int count = 0;

		if (!option[0] || !param[0])
		{
			this->SendSyntax(source);
			source.Reply(_("\002/msg %s HELP MASSSET\002 for more information."), Config->NickServ.c_str());
		}
		else if (option.equals_ci("KILL"))
		{
			if (param.equals_ci("ON"))
			{
				for (nickcore_map::const_iterator it = NickCoreList.begin(), it_end = NickCoreList.end(); it != it_end; ++it)
				{
					NickCore *nc = it->second;
					count++;

					nc->SetFlag(NI_KILLPROTECT);
					nc->UnsetFlag(NI_KILL_QUICK);
					nc->UnsetFlag(NI_KILL_IMMED);
				}
				source.Reply(_("Successfully enabled \002kill protection\002 for \002%d\002 users."), count);
			}
			else if (param.equals_ci("OFF"))
			{
				for (nickcore_map::const_iterator it = NickCoreList.begin(), it_end = NickCoreList.end(); it != it_end; ++it)
				{
					NickCore *nc = it->second;
					count++;

					nc->UnsetFlag(NI_KILLPROTECT);
					nc->UnsetFlag(NI_KILL_QUICK);
					nc->UnsetFlag(NI_KILL_IMMED);
				}
				source.Reply(_("Successfully disabled \002kill protection\002 for \002%d\002 users."), count);
			}
			else {
				source.Reply(_("Syntax: \002MASSSET KILL \037on|off\037\002"), Config->NickServ.c_str());
			}
		}
		else if (option.equals_ci("KILLQUICK"))
		{
			if (param.equals_ci("ON"))
			{
				for (nickcore_map::const_iterator it = NickCoreList.begin(), it_end = NickCoreList.end(); it != it_end; ++it)
				{
					NickCore *nc = it->second;
					count++;

					nc->SetFlag(NI_KILLPROTECT);
					nc->SetFlag(NI_KILL_QUICK);
					nc->UnsetFlag(NI_KILL_IMMED);
				}
				source.Reply(_("Successfully enabled \002quick kill protection\002 for \002%d\002 users."), count);
			}
			else if (param.equals_ci("OFF"))
			{
				for (nickcore_map::const_iterator it = NickCoreList.begin(), it_end = NickCoreList.end(); it != it_end; ++it)
				{
					NickCore *nc = it->second;
					count++;

					nc->SetFlag(NI_KILLPROTECT);
					nc->UnsetFlag(NI_KILL_QUICK);
					nc->UnsetFlag(NI_KILL_IMMED);
				}
				source.Reply(_("Successfully disabled \002quick kill protection\002 for \002%d\002 users."), count);
			}
			else {
				source.Reply(_("Syntax: \002MASSSET KILLQUICK \037on|off\037\002"), Config->NickServ.c_str());
			}
		}
		else if (option.equals_ci("SECURE"))
		{
			if (param.equals_ci("ON"))
			{
				for (nickcore_map::const_iterator it = NickCoreList.begin(), it_end = NickCoreList.end(); it != it_end; ++it)
				{
					NickCore *nc = it->second;
					count++;

					nc->SetFlag(NI_SECURE);
				}
				source.Reply(_("Successfully enabled \002NickServ Security\002 for \002%d\002 users."), count);
			}
			else if (param.equals_ci("OFF"))
			{
				source.Reply(_("For security reasons, mass-setting \002SECURE\002 to OFF is disabled."));
			}
			else {
				source.Reply(_("Syntax: \002MASSSET SECURE \037on|off\037\002"), Config->NickServ.c_str());
			}
		}
		else if (option.equals_ci("AUTOOP"))
		{
			if (param.equals_ci("ON"))
			{
				for (nickcore_map::const_iterator it = NickCoreList.begin(), it_end = NickCoreList.end(); it != it_end; ++it)
				{
					NickCore *nc = it->second;
					count++;

					nc->SetFlag(NI_AUTOOP);
				}
				source.Reply(_("Successfully enabled \002AutoOP\002 for \002%d\002 users."), count);
			}
			else if (param.equals_ci("OFF"))
			{
				for (nickcore_map::const_iterator it = NickCoreList.begin(), it_end = NickCoreList.end(); it != it_end; ++it)
				{
					NickCore *nc = it->second;
					count++;

					nc->UnsetFlag(NI_AUTOOP);
				}
				source.Reply(_("Successfully disabled \002AutoOP\002 for \002%d\002 users."), count);
			}
			else {
				source.Reply(_("Syntax: \002MASSSET AUTOOP \037on|off\037\002"), Config->NickServ.c_str());
			}
		}
		else {
			source.Reply(_("Unknown \002MASSSET\002 option \002%s\002\n"
				"\002/msg %s HELP MASSSET\002 for more information"), option.c_str(), Config->NickServ.c_str());
		}
		return;
	}

	bool OnHelp(CommandSource &source, const Anope::string &subcommand)
	{
		this->SendSyntax(source);
		source.Reply(" ");
		source.Reply(_("Allows options to be mass-set for ALL registered users\n"
						"	KILL	   Turns automatic kill protection on/off\n"
						"              for all registered nicks.\n"
						"	KILLQUICK  Turns automatic kill protection with a \n"
						"              reduced delay on/off for all registered nicks.\n"
						"	SECURE	   Turns NickServ's security features on/off\n"
						"              for all registered nicks.\n"
						"	AUTOOP	   Turns NickServ's autoop features on/off\n"
						"              for all registered nicks."));
		return true;
	}

};


class NSMassSet : public Module
{
	CommandNSMassSet commandnsmassset;

public:
	NSMassSet(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator, THIRD),
		commandnsmassset(this)
	{
		this->SetAuthor(AUTHOR);
		this->SetVersion(VERSION);
	}
};

MODULE_INIT(NSMassSet)
