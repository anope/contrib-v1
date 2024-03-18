#include "module.h"

#define AUTHOR "Adam"
#define VERSION "1.6"

static BotInfo *vHostServ;
static Anope::string BotNick;
static time_t bantime;
static Anope::string KickReason;

namespace RestrictedHosts
{
	struct RestrictedHost
	{
		Anope::string nick;
		Anope::string host;
		time_t t;
	};

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

class DoUnban : public CallBack
{
	dynamic_reference<Channel> chan;
	Anope::string user;
	Anope::string banmask;

 public:
	DoUnban(Module *me, Channel *c, const Anope::string &nick, const Anope::string &bmask) : CallBack(me, bantime), chan(c), user(nick), banmask(bmask)
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

class CommandRestrictedHosts : public Command
{
 private:
	void DoList(CommandSource &source)
	{
		if (RestrictedHosts::Hosts.empty())
			source.Reply(_("The restricted hosts list is empty"));
		else
		{
			source.Reply(_("Restricted hosts list:"));
			for (unsigned i = 0; i < RestrictedHosts::Hosts.size(); ++i)
			{
				const RestrictedHosts::RestrictedHost &rh = RestrictedHosts::Hosts[i];

				source.Reply(_("%d: %s"), i + 1, rh.host.c_str());
				source.Reply(_("Set by: %s on %s"), rh.nick.c_str(), do_strftime(rh.t).c_str());
			}
			source.Reply(_("End of list"));
		}
	}

	void DoAdd(CommandSource &source, const Anope::string &host)
	{
		if (RestrictedHosts::AddHost(host, source.u->nick))
			source.Reply(_("\2%s\2 added to the restricted hosts list."), host.c_str());
		else
			source.Reply(_("Could not add host \2%s\2"), host.c_str());
	}

	void DoDel(CommandSource &source, const Anope::string &host)
	{
		if (RestrictedHosts::DelHost(host))
			source.Reply(_("Host \2%s\2 deleted from the restricted hosts list."), host.c_str());
		else
			source.Reply(_("Unable to delete host \2%s\2"), host.c_str());
	}

 public:
	CommandRestrictedHosts(Module *creator) : Command(creator, "vhostserv/restrictedhosts", 1, 2)
	{
		this->SetDesc("Control what vhosts are automatically rejected");
		this->SetSyntax("ADD \037host\037");
		this->SetSyntax("DEL \037host\037");
		this->SetSyntax("LIST");
	}

	void Execute(CommandSource &source, const std::vector<Anope::string> &params)
	{
		if (params[0].equals_ci("LIST"))
			this->DoList(source);
		else if (params.size() < 2)
			this->OnSyntaxError(source, "");
		else if (params[0].equals_ci("ADD"))
			this->DoAdd(source, params[1]);
		else if (params[0].equals_ci("DEL"))
			this->DoDel(source, params[1]);
	}

	bool OnHelp(CommandSource &source, const Anope::string &subcommand)
	{
		this->SendSyntax(source);
		source.Reply(" ");
		source.Reply(_("This command allows you to manage hosts are are not allowed\n"
			"to be used as vHosts. Wildcards are supported"));
		return true;
	}
};

class ModulevHostServ : public Module
{
	CommandRestrictedHosts commandrestrictedhosts;

	void PlaceBan(User *u, Channel *c)
	{
		if (!bantime || !u || !c)
			return;

		UserContainer *uc = c->FindUser(u);
		if (uc == NULL || uc->Status->FlagCount() > 0)
			return;

		Anope::string banmask = "*!*@" + u->host;
		Anope::string reason = Anope::printf(KickReason.c_str(), bantime);

		c->SetMode(vHostServ, CMODE_BAN, banmask);
		c->Kick(vHostServ, u, "%s", reason.c_str());

		new DoUnban(this, c, u->nick, banmask);
	}

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
					this->HSSync(na->nc, ident, host, u->nick, Anope::CurTime);
			}

			if (!ident.empty())
				u->SetVIdent(ident);
			u->SetDisplayedHost(host);

			if (!ident.empty())
			{
				Log(vHostServ) << "Set the VHost of " << u->nick << " (" << u->GetIdent() << "@" << u->host << ") to \2" << ident << "@" << host << "\2";
				u->SendMessage(vHostServ, _("Your vHost has been set to \2%s@%s\2 and activated."), ident.c_str(), host.c_str());
			}
			else
			{
				Log(vHostServ) << "Set the VHost of " << u->nick << " (" << u->GetIdent() << "@" << u->host << ") to \2" << host << "\2";
				u->SendMessage(vHostServ, _("Your vHost has been set to \2%s\2 and activated."), host.c_str());
			}

			return true;
		}

		return false;
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

 public:
	ModulevHostServ(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator, THIRD),
		commandrestrictedhosts(this)
	{
		this->SetAuthor(AUTHOR);
		this->SetVersion(VERSION);

		OnReload();

		Implementation i[] = { I_OnPrivmsg, I_OnReload };
		ModuleManager::Attach(i, this, sizeof(i) / sizeof(Implementation));

		ModuleManager::RegisterService(&commandrestrictedhosts);
	}

	void OnPrivmsg(User *u, Channel *c, Anope::string &msg)
	{
		if (!u || !u->Account() || !c || c->FindUser(vHostServ) == NULL)
			return;

		NickAlias *na = findnick(u->nick);
		if (!na || na->nc != u->Account())
			return;

		std::vector<Anope::string> params = BuildStringVector(msg);

		if (params.empty())
			return;
		else if (params.size() == 1)
		{
			if (params[0].equals_ci("!GROUPVHOST"))
				u->SendMessage(vHostServ, _("Syntax: \2GROUPVHOST\2 [\2\37ident\37\2@]\2\37host\37\2"));
			else if (params[0].equals_ci("!VHOST"))
				u->SendMessage(vHostServ, _("Syntax: \2VHOST\2 [\2\37ident\37\2@]\2\37host\37\2"));
		}
		else if (params[0].equals_ci("!GROUPVHOST"))
		{
			if (this->SetVhost(u, params[1], true))
				this->PlaceBan(u, c);
		}
		else if (params[0].equals_ci("!VHOST"))
		{
			if (this->SetVhost(u, params[1], false))
				this->PlaceBan(u, c);
		}
	}

	void OnReload()
	{
		ConfigReader config;

		BotNick = config.ReadValue("vhostserv", "nick", "vHostServ", 0);
		Anope::string BotIdent = config.ReadValue("vhostserv", "ident", "vHostServ", 0);
		Anope::string BotHost = config.ReadValue("vhostserv", "host", "vhost.serv", 0);
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

MODULE_INIT(ModulevHostServ)
