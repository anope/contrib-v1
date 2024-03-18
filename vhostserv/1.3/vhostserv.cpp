#include "module.h"

#define AUTHOR "Adam"
#define VERSION "1.3"

#define LNG_MUST_REGISTER	0
#define LNG_INVALID_HOST	1
#define LNG_RESHOST_HELP	2
#define LNG_NO_HELP			3
#define LNG_HELP_LIST		4
#define LNG_RESHOST_SYNTAX	5
#define LNG_RESLIST_EMPTY	6
#define LNG_RESHOST_HEAD	7
#define LNG_RESHOST_TAIL	8
#define LNG_RESHOST_ENTRY1	9
#define LNG_RESHOST_ENTRY2	10
#define LNG_RESHOST_ADD		11
#define LNG_RESHOST_ADDFAIL	12
#define LNG_RESHOST_DEL		13
#define LNG_RESHOST_DELFAIL	14
#define LNG_SETHOST_IDENT	15
#define LNG_SETHOST			16
#define LNG_VHOST_SYNTAX	17
#define LNG_GVHOST_SYNTAX	18
#define LNG_NUM_STRINGS		19

static Module *me;

CommandHash *VHOSTSERV[MAX_CMD_HASH];

BotInfo *vHostServ;
char *s_vHostServ;
std::string BotNick;
std::string BotIdent;
std::string BotHost;
std::string BotRealName;
std::string BotModes;
std::string BanTime;
std::string KickReason;
time_t BanTimeT;

int do_fantasy(int argc, char **argv);
int do_help(User *u);
int do_restrictedhosts(User *u);
bool SetVhost(User *u, std::string &param, bool group);
void PlaceBan(User *u, ChannelInfo *ci);
void HSSync(NickCore *nc, char *ident, char *host, char *creator, time_t time);
int delBan(int arg, char **argv);
int do_reload(int argc, char **argv);
int do_connect(int argc, char **argv);

struct RestrictedHost
{
	std::string nick;
	std::string host;
	time_t t;
};

class _RestrictedHosts
{
	public:
		std::vector<RestrictedHost *> Hosts;

		bool AddHost(std::string &host, std::string nick)
		{
			RestrictedHost *rh;

			if (!IsRestricted(host))
			{
				rh = new RestrictedHost;

				rh->nick = nick;
				rh->host = host;
				rh->t = time(NULL);

				Hosts.push_back(rh);

				return true;
			}

			return false;
		}

		bool DelHost(std::string &host)
		{
			RestrictedHost *rh;
			unsigned i;

			for (i = 0; i < Hosts.size(); ++i)
			{
				rh = Hosts[i];

				if (rh->host == host)
				{
					Hosts.erase(Hosts.begin() + i);
					delete rh;
					
					return true;
				}
			}
			
			return false;
		}

		const bool IsRestricted(std::string &host)
		{
			RestrictedHost *rh;
			unsigned i;

			for (i = 0; i < Hosts.size(); ++i)
			{
				rh = Hosts[i];

				if (Anope::Match(rh->host.c_str(), host.c_str(), false))
					return true;
			}

			return false;
		}
};
_RestrictedHosts RestrictedHosts;

class CommandHelp : public Command
{
	public:
		CommandHelp(const std::string &cname) : Command(cname, 1, 1)
		{
		}

		CommandReturn Execute(User *u, std::vector<ci::string> &params)
		{
			Command *c = findCommand(VHOSTSERV, params[0].c_str());

			if (!c || !c->OnHelp(u, ""))
				me->NoticeLang(s_vHostServ, u, LNG_NO_HELP, params[0].c_str());

			return MOD_CONT;
		}

		void OnSyntaxError(User *u)
		{
			me->NoticeLang(s_vHostServ, u, LNG_HELP_LIST);
		}
};

class CommandRestrictedHosts : public Command
{
	private:
		void DoList(User *u)
		{
			unsigned i;
			RestrictedHost *rh;
			tm tm;
			char timebuf[64];

			if (RestrictedHosts.Hosts.empty())
				me->NoticeLang(s_vHostServ, u, LNG_RESLIST_EMPTY);
			else
			{
				me->NoticeLang(s_vHostServ, u, LNG_RESHOST_HEAD);
				for (i = 0; i < RestrictedHosts.Hosts.size(); ++i)
				{
					rh = RestrictedHosts.Hosts[i];

					me->NoticeLang(s_vHostServ, u, LNG_RESHOST_ENTRY1, i + 1, rh->host.c_str());
					tm = *localtime(&rh->t);
					strftime_lang(timebuf, sizeof(timebuf), u, STRFTIME_SHORT_DATE_FORMAT, &tm);
					me->NoticeLang(s_vHostServ, u, LNG_RESHOST_ENTRY2, rh->nick.c_str(), timebuf);
				}
				me->NoticeLang(s_vHostServ, u, LNG_RESHOST_TAIL);
			}
		}

		void DoAdd(User *u, ci::string &host)
		{
			std::string rhost = host.c_str();
			if (RestrictedHosts.AddHost(rhost, std::string(u->nick)))
				me->NoticeLang(s_vHostServ, u, LNG_RESHOST_ADD, host.c_str());
			else
				me->NoticeLang(s_vHostServ, u, LNG_RESHOST_ADDFAIL, host.c_str());
		}

		void DoDel(User *u, ci::string &host)
		{
			std::string rhost = host.c_str();
			if (RestrictedHosts.DelHost(rhost))
				me->NoticeLang(s_vHostServ, u, LNG_RESHOST_DEL, host.c_str());
			else
				me->NoticeLang(s_vHostServ, u, LNG_RESHOST_DELFAIL, host.c_str());
		}

	public:
		CommandRestrictedHosts(const std::string &cname) : Command(cname, 1, 2, "vhostserv/restrictedhosts")
		{
		}

		CommandReturn Execute(User *u, std::vector<ci::string> &params)
		{
			if (params[0] == "LIST")
			{
				this->DoList(u);
			}
			else if (params.size() < 2)
			{
				this->OnSyntaxError(u);
			}
			else if (params[0] == "ADD")
			{
				this->DoAdd(u, params[1]);
			}
			else if (params[0] == "DEL")
			{
				this->DoDel(u, params[1]);
			}

			return MOD_CONT;
		}

		void OnSyntaxError(User *u)
		{
			me->NoticeLang(s_vHostServ, u, LNG_RESHOST_SYNTAX, s_vHostServ);
		}

		bool OnHelp(User *u, const ci::string &subcommand)
		{
			me->NoticeLang(s_vHostServ, u, LNG_RESHOST_HELP);
			return true;
		}
};

class Module_vHostServ : public Module
{
	public:
		Module_vHostServ(const std::string &modname, const std::string &creator) : Module(modname, creator)
		{
			this->SetAuthor(AUTHOR);
			this->SetVersion(VERSION);
			this->SetType(THIRD);

			me = this;

			OnReload(false);
			this->LoadLanguages();

			Implementation i[] = { I_OnBotFantasy, I_OnBotNoFantasyAccess, I_OnReload, I_OnServerConnect };
			ModuleManager::Attach(i, this, 4);

			this->AddCommand(VHOSTSERV, new CommandRestrictedHosts("RESTRICTEDHOSTS"));
			this->AddCommand(VHOSTSERV, new CommandHelp("HELP"));
		}

		~Module_vHostServ()
		{
			CommandHash *current;
			Command *c;
			int i;

			for (i = 0; i < MAX_CMD_HASH; ++i)
			{
				for (current = VHOSTSERV[i]; current; current = current->next)
				{
					for (c = current->c; c; c = c->next)
						this->DelCommand(VHOSTSERV, c->name.c_str());
				}
			}

			if (vHostServ)
			{
				ircdproto->SendQuit(vHostServ, NULL);
				delete vHostServ;
			}
		}

		void OnBotFantasy(char *command, User *u, ChannelInfo *ci, char *params)
		{
			OnBotNoFantasyAccess(command, u, ci, params);
		}

		void OnBotNoFantasyAccess(const char *command, User *u, ChannelInfo *ci, const char *params)
		{
			if (!command || ci->bi != vHostServ)
			{
				return;
			}
			else if (!params)
			{
				if (!stricmp(command, "GROUPVHOST"))
					me->NoticeLang(s_vHostServ, u, LNG_GVHOST_SYNTAX);
				else if (!stricmp(command, "VHOST"))
					me->NoticeLang(s_vHostServ, u, LNG_VHOST_SYNTAX);
			}
			else if (!stricmp(command, "VHOST"))
			{
				std::string vhost = params;
				if (SetVhost(u, vhost, false))
					PlaceBan(u, ci);
			}
			else if (!stricmp(command, "GROUPVHOST"))
			{
				std::string vhost = params;
				if (SetVhost(u, vhost, true))
					PlaceBan(u, ci);
			}
		}

		void OnReload(bool startup)
		{
			ConfigReader Config;
			std::string RestrictedHostsList;

			BotNick = Config.ReadValue("vhostserv", "nick", "vHostServ", 0);
			BotIdent = Config.ReadValue("vhostserv", "ident", "vHostServ", 0);
			BotHost = Config.ReadValue("vhostserv", "host", ServiceHost, 0);
			BotRealName = Config.ReadValue("vhostserv", "realname", "vHostServ", 0);
			BotModes = Config.ReadValue("vhostserv", "modes", ircd->pseudoclient_mode, 0);
			RestrictedHostsList = Config.ReadValue("vhostserv", "restrictedhosts", "", 0);
			BanTime = Config.ReadValue("vhostserv", "bantime", "", 0);
			KickReason = Config.ReadValue("vhostserv", "kickreason", "Done. You can request another vHost in %d seconds", 0);

			spacesepstream HostsList(RestrictedHostsList);
			std::string Host;

			while (HostsList.GetToken(Host))
			{
				RestrictedHosts.AddHost(Host, "Config");
			}

			if (BanTime.empty())
				BanTimeT = 0;
			else
				BanTimeT = dotime(BanTime.c_str());

			s_vHostServ = const_cast<char *>(BotNick.c_str());
		}

		void OnPostLoadDatabases()
		{
			OnServerConnect();
		}

		void OnServerConnect()
		{
			vHostServ = findbot(BotNick.c_str());

			if (!vHostServ)
			{
				vHostServ = new BotInfo(BotNick.c_str(), BotIdent.c_str(), BotHost.c_str(), BotRealName.c_str());
				ircdproto->SendClientIntroduction(BotNick.c_str(), BotIdent.c_str(), BotHost.c_str(), BotRealName.c_str(), BotModes.c_str(), vHostServ->uid.c_str());
			}

			vHostServ->cmdTable = VHOSTSERV;
			vHostServ->flags |= BI_PRIVATE;
		}

		void LoadLanguages()
		{
			const char *langtable_en_us[] = {
				/* LNG_MUST_REGISTER */
				"You must register your nick before you can use this command.",

				/* LNG_INVALID_HOST */
				"Invalid vHost.",

				/* LNG_RESHOST_HELP */
				"Syntax: \2RESTRICTEDHOSTS\2 ADD \2\37host\37\2\n"
				"        \2RESTRICTEDHOSTS\2 DEL \2\37host\37\2\n"
				"        \2RESTRICTEDHOSTS\2 LIST\n"
				" \n"
				"This command allows you to manage hosts are are not allowed\n"
				"to be used as vHosts. Wildcards are supported",

				/* LNG_NO_HELP */
				"No help available for \2%s\2",

				/* LNG_HELP_LIST */
				"Available commands:\n"
				" \n"
				"     HELP                Get help\n"
				"     RESTRICTEDHOSTS     Manage restricted hosts\n"
				" \n"
				"This service is limited to services operators",

				/* LNG_RESHOST_SYNTAX */
				"Syntax: \2RESTRICTEDHOSTS\2 ADD \2\37host\37\2\n"
				"        \2RESTRICTEDHOSTS\2 DEL \2\37host\37\2\n"
				"        \2RESTRICTEDHOSTS\2 LIST\n"
				"\2/msg %s HELP RESTRICTED\2 for more information",

				/* LNG_RESLIST_EMPTY */
				"The restricted hosts list is empty",

				/* LNG_RESHOST_HEAD */
				"Restricted hosts list:",

				/* LNG_RESHOST_TAIL */
				"End of list",

				/* LNG_RESHOST_ENTRY1 */
				"%d: %s",

				/* LNG_RESHOST_ENTRY2 */
				"Set by: %s on %s",

				/* LNG_RESHOST_ADD */
				"\2%s\2 added to the restricted hosts list.",

				/* LNG_RESHOST_ADDFAIL */
				"Could not add host \2%s\2",

				/* LNG_RESHOST_DEL */
				"Host \2%s\2 deleted from the restricted hosts list.",

				/* LNG_RESHOST_DELFAIL */
				"Unable to delete host \2%s\2",

				/* LNG_SETHOST_IDENT */
				"Your vHost has been set to \2%s@%s\2 and activated.",

				/* LNG_SETHOST */
				"Your vHost has been set to \2%s\2 and activated.",

				/* LNG_VHOST_SYNTAX */
				"Syntax: \2VHOST\2 [\2\37ident\37\2@]\2\37host\37\2",

				/* LNG_GVHOST_SYNTAX */
				"Syntax: \2GROUPVHOST\2 [\2\37ident\37\2@]\2\37host\37\2"
			};

			this->InsertLanguage(LANG_EN_US, LNG_NUM_STRINGS, langtable_en_us);
		}
};

bool SetVhost(User *u, std::string &param, bool group)
{
	char *ident, *host;
	char *s;
	bool identok = false;
	time_t t = time(NULL);
	NickCore *nc;
	bool retval = false;

	if (strchr(param.c_str(), '@'))
	{
		ident = myStrGetToken(param.c_str(), '@', 0);
		host = myStrGetTokenRemainder(param.c_str(), '@', 1);
	}
	else
	{
		host = sstrdup(param.c_str());
		ident = NULL;
	}

	if (!host)
	{
	}
	else if (ident)
	{
		if (strlen(ident) > USERMAX - 1)
		{
			notice_lang(s_vHostServ, u, HOST_SET_IDENTTOOLONG, USERMAX);
		}
		else
		{
			for (s = ident; *s; s++)
			{
				if (!isvalidchar(*s))
				{
					notice_lang(s_vHostServ, u, HOST_SET_IDENT_ERROR);
					break;
				}
			}

			identok = true;
		}
	}
	else
		identok = true;

	if (identok)
	{
		if (!ircd->vident)
		{
			notice_lang(s_vHostServ, u, HOST_NO_VIDENT);
		}
		else if (strlen(host) > HOSTMAX - 1)
		{
			notice_lang(s_vHostServ, u, HOST_SET_TOOLONG, HOSTMAX);
		}
		else if (!isValidHost(host, 3))
		{
			notice_lang(s_vHostServ, u, HOST_SET_ERROR);
		}
		else
		{
			if (!group)
			{
				addHostCore(u->nick, ident, host, u->nick, t);
				ircdproto->SendVhost(u->nick, ident, host);
			}
			else
			{
				if ((nc = findcore(u->nick)))
					HSSync(nc, ident, host, u->nick, t);
			}

			if (ident)
			{
				u->SetVIdent(ident);
			}

			u->SetDisplayedHost(host);

			if (ident)
			{
				alog("%s: Set the vHost of %s (%s@%s) to \2%s@%s\2", s_vHostServ, u->nick, u->GetIdent().c_str(), u->host, ident, host);
				me->NoticeLang(s_vHostServ, u, LNG_SETHOST_IDENT, ident, host);
			}
			else
			{
				alog("%s: Set the vHost of %s (%s@%s) to \2%s\2", s_vHostServ, u->nick, u->GetIdent().c_str(), u->host, host);
				me->NoticeLang(s_vHostServ, u, LNG_SETHOST, host);
			}

			retval = true;
		}
	}

	if (ident)
		delete [] ident;
	delete [] host;

	return retval;
}

class DoUnban : public Timer
{
	std::string channame;
	std::string banmask;

	public:
		DoUnban(time_t time, std::string &cname, std::string &bmask) : Timer(time), channame(cname), banmask(bmask)
		{
		}

		void Tick(time_t)
		{
			const char *av[2];
			ChannelInfo *ci = cs_findchan(channame.c_str());

			av[0] = "-b";
			av[1] = banmask.c_str();

			if (ci && ci->c)
			{
				ircdproto->SendMode(whosends(ci), ci->name, "-b %s", av[1]);
				chan_set_modes(s_ChanServ, ci->c, 2, av, 1);
			}
		}
};

void PlaceBan(User *u, ChannelInfo *ci)
{
	char banmask[BUFSIZE], reason[BUFSIZE];;
	Channel *c;
	const char *av[3];
	char *cb[2];

	if (BanTimeT && !check_access(u, ci, CA_OPDEOPME))
	{
		if ((c = findchan(ci->name)) && c->ci)
		{
			snprintf(banmask, sizeof(banmask), "*!*@%s", u->vhost);
			snprintf(reason, sizeof(reason), KickReason.c_str(), BanTimeT);

			cb[0] = c->name;
			cb[1] = banmask;

			av[0] = "+b";
			av[1] = banmask;

			ircdproto->SendMode(whosends(ci), c->name, "+b %s", av[1]);
			chan_set_modes(s_ChanServ, c, 2, av, 1);

			av[0] = c->name;
			av[1] = u->nick;
			av[2] = reason;
			ircdproto->SendKick(whosends(ci), c->name, u->nick, "%s", reason);
			do_kick(s_ChanServ, 3, av);

			std::string channame = c->name;
			std::string banmask2 = banmask;
			me->AddCallBack(new DoUnban(BanTimeT, channame, banmask2));
		}
	}
}

void HSSync(NickCore *nc, char *ident, char *host, char *creator, time_t time)
{
	int i;
	NickAlias *na;
	User *u;

	for (i = 0; i < nc->aliases.count; ++i)
	{
		na = static_cast<NickAlias *>(nc->aliases.list[i]);
		addHostCore(na->nick, ident, host, creator, time);

		if ((u = finduser(na->nick)))
			ircdproto->SendVhost(u->nick, ident, host);
	}
}

MODULE_INIT(Module_vHostServ)
