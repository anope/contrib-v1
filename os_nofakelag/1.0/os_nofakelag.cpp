#include "module.h"
#define VER "1.0"
/************************************************
* NoFakeLag Module
* 
* Original Module Author: mooncup (1.8 Version)
* Examples taken from Justasic 
* 
* Code Developed by: Cronus
*
* For UnrealIRCd ONLY!
*
*************************************************
* Syntax: /msg %s NOFAKELAG [nickname]
* Adds command operserv/nofakelag
*************************************************/

class CommandOSNoFakeLag : public Command
{
public:
	CommandOSNoFakeLag(Module *creator) : Command(creator, "operserv/nofakelag", 1, 2)
	{
		this->SetDesc(_("Enable nofakelag on a user on unrealircd"));
		this->SetSyntax(_("[\037nickname\037]"));
	}

	void Execute(CommandSource &source, const std::vector<Anope::string> &params)
	{
		const Anope::string &nick = params[0];

		User *u = source.GetUser();
		User *u2 = finduser(nick);
		BotInfo *bi = source.owner;

		if (!u) {
			return; 
		}
		
		if(!u2)
		{
			if (nickIsServices(nick, true))
			{
				source.Reply(_("Nick \002%s\002 is part of this networks services"), nick.c_str());
				return;
			}
			else
			{
				source.Reply(NICK_X_NOT_IN_USE, nick.c_str());
				return;
			}
		}
		else if (!source.IsServicesOper())
		{
			source.Reply(ACCESS_DENIED);
		}
		else
		{ 
			Log(LOG_OVERRIDE, source, this, NULL) << "to set nofakelag on " << u2->nick;
			source.Reply(_("You have enabled nofakelag on user \002%s\002"), nick.c_str());
			u2->SendMessage(bi, _("You were given nofakelag by \002%s\002"), u->nick.c_str());
			UplinkSocket::Message() << "SVS2NOLAG + " << u2->nick;
		}
		return;
	} 
	bool OnHelp(CommandSource &source, const Anope::string &subcommand)
	{ 
		this->SendSyntax(source);
		source.Reply(" ");
		source.Reply(_("Enable nofakelag on a user on unrealircd."));
		return true;
	}
};

class OSNoFakeLag : public Module
{
	CommandOSNoFakeLag commandosnofakelag;
	
public:
	OSNoFakeLag(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator),	
		commandosnofakelag(this)
	{
		if (!ModuleManager::FindModule("unreal32")) 
		{ 
			Log() << "ERROR: You are not running UnrealIRCd, this module only works on UnrealIRCd.";
			return; 
		}
		this->SetAuthor("Cronus");
		this->SetVersion(VER);
	}
};

MODULE_INIT(OSNoFakeLag) 
