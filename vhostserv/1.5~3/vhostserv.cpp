#include "module.h"

#define AUTHOR "Adam"
#define VERSION "1.6"

static Module *me;
static BotInfo *vHostServ;
static time_t bantime;
static Anope::string KickReason;

bool SetVhost(User *u, const Anope::string &param, bool group);
void PlaceBan(User *u, ChannelInfo *ci);
void HSSync(NickCore *nc, const Anope::string &ident, const Anope::string &host, const Anope::string &creator, time_t time);

struct RestrictedHost
{
	Anope::string nick;
	Anope::string host;
	time_t t;
};

namespace RestrictedHosts
{
	std::vector<RestrictedHost> Hosts;

	bool IsRestricted(const Anope::string &host)
	{
		for (unsigned i = 0; i < RestrictedHosts::Hosts.size(); ++i)
		{
			const RestrictedHost &rh = RestrictedHosts::Hosts[i];

			if (Anope::Match(rh.host.c_str(), host.c_str(), false))
				return true;
		}

		return false;
	}

	bool AddHost(const Anope::string &host, const Anope::string &nick)
	{
		if (!IsRestricted(host))
		{
			RestrictedHost rh;
			rh.nick = nick;
			rh.host = host;
			rh.t = Anope::CurTime;

			RestrictedHosts::Hosts.push_back(rh);

			return true;
		}

		return false;
	}

	bool DelHost(const Anope::string &host)
	{
		for (unsigned i = 0; i < RestrictedHosts::Hosts.size(); ++i)
		{
			const RestrictedHost &rh = RestrictedHosts::Hosts[i];

			if (rh.host == host)
			{
				RestrictedHosts::Hosts.erase(RestrictedHosts::Hosts.begin() + i);
				return true;
			}
		}
			
		return false;
	}
}

class CommandHelp : public Command
{
 public:
	CommandHelp() : Command("HELP", 1, 1)
	{
	}

	CommandReturn Execute(User *u, const std::vector<Anope::string> &params)
	{
		Command *c = FindCommand(vHostServ, params[0]);

		if (!c || !c->OnHelp(u, ""))
			me->SendMessage(vHostServ, u, "No help available for \2%s\2", params[0].c_str());

		return MOD_CONT;
	}

	void OnSyntaxError(User *u, const Anope::string &)
	{
		me->SendMessage(vHostServ, u, _("Available commands:\n"
			" \n"
			"     HELP                Get help\n"
			"     RESTRICTEDHOSTS     Manage restricted hosts\n"
			" \n"
			"This service is limited to services operators"));
	}
};

class CommandRestrictedHosts : public Command
{
 private:
	void DoList(User *u)
	{
		if (RestrictedHosts::Hosts.empty())
			me->SendMessage(vHostServ, u, _("The restricted hosts list is empty"));
		else
		{
			me->SendMessage(vHostServ, u, _("Restricted hosts list:"));
			for (unsigned i = 0; i < RestrictedHosts::Hosts.size(); ++i)
			{
				const RestrictedHost &rh = RestrictedHosts::Hosts[i];

				me->SendMessage(vHostServ, u, "%d: %s", i + 1, rh.host.c_str());
				me->SendMessage(vHostServ, u, _("Set by: %s on %s"), rh.nick.c_str(), do_strftime(rh.t).c_str());
			}
			me->SendMessage(vHostServ, u, _("End of list"));
		}
	}

	void DoAdd(User *u, const Anope::string &host)
	{
		if (RestrictedHosts::AddHost(host, u->nick))
			me->SendMessage(vHostServ, u, _("\2%s\2 added to the restricted hosts list."), host.c_str());
		else
			me->SendMessage(vHostServ, u, _("Could not add host \2%s\2"), host.c_str());
	}

	void DoDel(User *u, const Anope::string &host)
	{
		if (RestrictedHosts::DelHost(host))
			me->SendMessage(vHostServ, u, _("Host \2%s\2 deleted from the restricted hosts list."), host.c_str());
		else
			me->SendMessage(vHostServ, u, _("Unable to delete host \2%s\2"), host.c_str());
	}

 public:
	CommandRestrictedHosts() : Command("RESTRICTEDHOSTS", 1, 2, "vhostserv/restrictedhosts")
	{
	}

	CommandReturn Execute(User *u, const std::vector<Anope::string> &params)
	{
		if (params[0].equals_ci("LIST"))
			this->DoList(u);
		else if (params.size() < 2)
			this->OnSyntaxError(u, "");
		else if (params[0].equals_ci("ADD"))
			this->DoAdd(u, params[1]);
		else if (params[0].equals_ci("DEL"))
			this->DoDel(u, params[1]);

		return MOD_CONT;
	}

	void OnSyntaxError(User *u, const Anope::string &)
	{
		me->SendMessage(vHostServ, u, _("Syntax: \2RESTRICTEDHOSTS\2 ADD \2\37host\37\2\n"
				"        \2RESTRICTEDHOSTS\2 DEL \2\37host\37\2\n"
				"        \2RESTRICTEDHOSTS\2 LIST\n"
				"\2/msg %s HELP RESTRICTED\2 for more information"), vHostServ->nick.c_str());
	}

	bool OnHelp(User *u, const Anope::string &subcommand)
	{
		me->SendMessage(vHostServ, u, _("Syntax: \2RESTRICTEDHOSTS\2 ADD \2\37host\37\2\n"
			"        \2RESTRICTEDHOSTS\2 DEL \2\37host\37\2\n"
			"        \2RESTRICTEDHOSTS\2 LIST\n"
			" \n"
			"This command allows you to manage hosts are are not allowed\n"
			"to be used as vHosts. Wildcards are supported"));
		return true;
	}
};

class Module_vHostServ : public Module
{
	CommandRestrictedHosts commandrestrictedhosts;
	CommandHelp commandhelp;

 public:
	Module_vHostServ(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator)
	{
		this->SetAuthor(AUTHOR);
		this->SetVersion(VERSION);
		this->SetType(THIRD);

		me = this;

		OnReload(false);

		Implementation i[] = { I_OnBotFantasy, I_OnBotNoFantasyAccess, I_OnReload };
		ModuleManager::Attach(i, this, 3);

		this->AddCommand(vHostServ, &commandrestrictedhosts);
		this->AddCommand(vHostServ, &commandhelp);
	}

	~Module_vHostServ()
	{
		if (vHostServ)
		{
			if (UplinkSock)
				ircdproto->SendQuit(vHostServ, NULL);
			delete vHostServ;
		}
	}

	void OnBotFantasy(const Anope::string &command, User *u, ChannelInfo *ci, const Anope::string &params)
	{
		this->OnBotNoFantasyAccess(command, u, ci, params);
	}

	void OnBotNoFantasyAccess(const Anope::string &command, User *u, ChannelInfo *ci, const Anope::string &params)
	{
		if (!u->Account() || !ci || ci->bi != vHostServ)
			return;
		else if (params.empty())
		{
			if (command.equals_ci("GROUPVHOST"))
				me->SendMessage(vHostServ, u, _("Syntax: \2GROUPVHOST\2 [\2\37ident\37\2@]\2\37host\37\2"));
			else if (command.equals_ci("VHOST"))
				me->SendMessage(vHostServ, u, _("Syntax: \2VHOST\2 [\2\37ident\37\2@]\2\37host\37\2"));
		}
		else if (command.equals_ci("GROUPVHOST"))
		{
			if (SetVhost(u, params, true))
				PlaceBan(u, ci);
		}
		else if (command.equals_ci("VHOST"))
		{
			if (SetVhost(u, params, false))
				PlaceBan(u, ci);
		}
	}

	void OnReload(bool startup)
	{
		ConfigReader config;

		Anope::string BotNick = config.ReadValue("vhostserv", "nick", "vHostServ", 0);
		Anope::string BotIdent = config.ReadValue("vhostserv", "ident", "vHostServ", 0);
		Anope::string BotHost = config.ReadValue("vhostserv", "host", Config->ServiceHost, 0);
		Anope::string BotRealName = config.ReadValue("vhostserv", "realname", "vHostServ", 0);
		Anope::string RestrictedHostsList = config.ReadValue("vhostserv", "restrictedhosts", "", 0);
		Anope::string BanTime = config.ReadValue("vhostserv", "bantime", "", 0);
		KickReason = config.ReadValue("vhostserv", "kickreason", "Done. You can request another vHost in %d seconds", 0);

		spacesepstream HostsList(RestrictedHostsList);
		Anope::string Host;
		while (HostsList.GetToken(Host))
			RestrictedHosts::AddHost(Host, "Config");

		if (BanTime.empty())
			bantime = 0;
		else
			bantime = dotime(BanTime.c_str());

		vHostServ = findbot(BotNick);
		if (!vHostServ)
			vHostServ = new BotInfo(BotNick, BotIdent, BotHost, BotRealName);
		vHostServ->SetFlag(BI_PRIVATE);
	}
};

bool SetVhost(User *u, const Anope::string &param, bool group)
{
	Anope::string ident, host;

	size_t at = param.find('@');
	if (at != Anope::string::npos)
	{
		ident = myStrGetToken(param, '@', 0);
		host = myStrGetTokenRemainder(param, '@', 1);
	}
	else
		host = param;

	if (!ident.empty())
	{
		if (ident.length() > Config->UserLen - 1)
			u->SendMessage(vHostServ, HOST_SET_IDENTTOOLONG, Config->UserLen - 1);
		else
		{
			for (unsigned i = 0; i < ident.length(); ++i)
			{
				if (!isvalidchar(ident[i]))
				{
					u->SendMessage(vHostServ, HOST_SET_IDENT_ERROR);
					return false;
				}
			}
		}
	}

	if (!ircd->vident)
		u->SendMessage(vHostServ, HOST_NO_VIDENT);
	else if (host.length() > Config->HostLen - 1)
		u->SendMessage(vHostServ, HOST_SET_TOOLONG, Config->HostLen - 1);
	else if (!isValidHost(host, 3))
		u->SendMessage(vHostServ, HOST_SET_ERROR);
	else
	{
		if (!group)
		{
			NickAlias *na = findnick(u->nick);
			if (na)
			{
				na->hostinfo.SetVhost(ident, host, u->nick);
				ircdproto->SendVhost(u, ident, host);
			}
		}
		else
		{
			NickAlias *na = findnick(u->nick);
			if (na)
				HSSync(na->nc, ident, host, u->nick, Anope::CurTime);
		}

		if (!ident.empty())
			u->SetVIdent(ident);
		u->SetDisplayedHost(host);

		if (!ident.empty())
		{
			Log(HostServ) << vHostServ->nick << " set the VHost of " << u->nick << " (" << u->GetIdent() << "@" << u->host << ") to \2" << ident << "@" << host << "\2";
			me->SendMessage(vHostServ, u, _("Your vHost has been set to \2%s@%s\2 and activated."), ident.c_str(), host.c_str());
		}
		else
		{
			Log(HostServ) << vHostServ->nick << ": set the VHost of " << u->nick << " (" << u->GetIdent() << "@" << u->host << ") to \2" << host << "\2";
			me->SendMessage(vHostServ, u, _("Your vHost has been set to \2%s\2 and activated."), host.c_str());
		}

		return true;
	}

	return false;
}

class DoUnban : public CallBack
{
	dynamic_reference<Channel> chan;
	Anope::string user;
	Anope::string banmask;

 public:
	DoUnban(Channel *c, const Anope::string &nick, const Anope::string &bmask) : CallBack(me, Anope::CurTime), chan(c), user(nick), banmask(bmask)
	{
	}

	void Tick(time_t)
	{
		if (!this->chan)
			return;

		this->chan->RemoveMode(vHostServ, CMODE_BAN, banmask.c_str());
		ircdproto->SendPrivmsg(vHostServ, this->chan->name, "Ban for %s (%s) expiring", this->user.c_str(), this->banmask.c_str());
	}
};

void PlaceBan(User *u, ChannelInfo *ci)
{
	if (bantime && !check_access(u, ci, CA_OPDEOPME) && ci->c)
	{
		Channel *c = ci->c;
		char reason[512];

		Anope::string banmask = "*!*@" + u->host;
		snprintf(reason, sizeof(reason), KickReason.c_str(), bantime);

		Anope::string channame = c->name;
		Anope::string banmask2 = banmask;

		c->SetMode(vHostServ, CMODE_BAN, banmask);
		c->Kick(vHostServ, u, "%s", reason);

		new DoUnban(c, u->nick, banmask2);
	}
}

void HSSync(NickCore *nc, const Anope::string &ident, const Anope::string &host, const Anope::string &creator, time_t time)
{
	for (std::list<NickAlias *>::const_iterator it = nc->aliases.begin(), it_end = nc->aliases.end(); it != it_end; ++it)
	{
		NickAlias *na = *it;
		na->hostinfo.SetVhost(ident, host, creator, time);

		User *u = finduser(na->nick);
		if (u)
			ircdproto->SendVhost(u, ident, host);
	}
}

MODULE_INIT(Module_vHostServ)
