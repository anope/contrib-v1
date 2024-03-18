#include "module.h"
#include "hashcomp.h"

#define AUTHOR "Adam"
#define VERSION "1.2"

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
#define LNG_NUM_STRINGS		18

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

				if (match_wild_nocase(rh->host.c_str(), host.c_str()))
					return true;
			}

			return false;
		}
};
_RestrictedHosts RestrictedHosts;

class Module_vHostServ : public Module
{
	public:
		Module_vHostServ(const std::string &modname, const std::string &creator) : Module(modname, creator)
		{
			EvtHook *hook;
			Command *c;

			this->SetAuthor(AUTHOR);
			this->SetVersion(VERSION);
			this->SetType(THIRD);

			me = this;

			Module_vHostServ::LoadConfig();

			this->LoadLanguages();

			hook = createEventHook(EVENT_RELOAD, do_reload);
			this->AddEventHook(hook);

			hook = createEventHook(EVENT_BOT_FANTASY, do_fantasy);
			this->AddEventHook(hook);

			hook = createEventHook(EVENT_BOT_FANTASY_NO_ACCESS, do_fantasy);
			this->AddEventHook(hook);

			hook = createEventHook(EVENT_SERVER_CONNECT, do_connect);
			this->AddEventHook(hook);

			c = createCommand("HELP", do_help, is_services_oper, -1, -1, -1, -1, -1);
			this->AddCommand(VHOSTSERV, c, MOD_HEAD);

			c = createCommand("RESTRICTEDHOSTS", do_restrictedhosts, is_services_oper, -1, -1, -1, -1, -1);
			this->AddCommand(VHOSTSERV, c, MOD_HEAD);
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
						this->DelCommand(VHOSTSERV, c->name);
				}
			}

			if (vHostServ)
			{
				ircdproto->SendQuit(vHostServ, NULL);
				delete vHostServ;
			}
		}

		static void LoadConfig()
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
			};

			this->InsertLanguage(LANG_EN_US, LNG_NUM_STRINGS, langtable_en_us);
		}
};

int do_fantasy(int argc, char **argv)
{
	User *u;
	ChannelInfo *ci;

	if (argc < 2)
		return MOD_CONT;
	else if (!(u = finduser(argv[1])))
		return MOD_CONT;
	else if (!(ci = cs_findchan(argv[2])))
		return MOD_CONT;
	else if (ci->bi != vHostServ)
		return MOD_CONT;
	else if (!nick_identified(u))
	{
		if (argc > 0)
		{
			if (!stricmp(argv[0], "VHOST") || !stricmp(argv[0], "GROUPVHOST"))
				me->NoticeLang(s_vHostServ, u, LNG_MUST_REGISTER);
		}
		return MOD_CONT;
	}
	else if (argc < 4)
	{
		if (argc > 0)
		{
			if (!stricmp(argv[0], "VHOST") || !stricmp(argv[0], "GROUPVHOST"))
				me->NoticeLang(s_vHostServ, u, LNG_VHOST_SYNTAX);
		}
		return MOD_CONT;
	}

	spacesepstream buf(argv[3]);
	std::string param;

	buf.GetToken(param);

	if (RestrictedHosts.IsRestricted(param))
	{
		me->NoticeLang(s_vHostServ, u, LNG_INVALID_HOST);
	}
	else if (!stricmp(argv[0], "VHOST"))
	{
		if (!param.empty())
		{
			if (SetVhost(u, param, false))
				PlaceBan(u, ci);
		}
	}
	else if (!stricmp(argv[0], "GROUPVHOST"))
	{
		if (!param.empty())
		{
			if (SetVhost(u, param, true))
				PlaceBan(u, ci);
		}
	}

	return MOD_CONT;
}

int do_help(User *u)
{
	char *buf, *arg;

	buf = moduleGetLastBuffer();
	arg = myStrGetToken(buf, ' ', 0);

	if (arg)
	{
		if (!stricmp(arg, "RESTRICTEDHOSTS"))
		{
			me->NoticeLang(s_vHostServ, u, LNG_RESHOST_HELP);
		}
		else
		{
			me->NoticeLang(s_vHostServ, u, LNG_NO_HELP, arg);
		}

		delete [] arg;
	}
	else
	{
		me->NoticeLang(s_vHostServ, u, LNG_HELP_LIST);
	}
	return MOD_CONT;
}

void do_restrict_syntax(User *u)
{
	me->NoticeLang(s_vHostServ, u, LNG_RESHOST_SYNTAX, s_vHostServ);
}

int do_restrictedhosts(User *u)
{
	char *buf, *arg, *arg2;
	unsigned i;
	RestrictedHost *rh;
	struct tm tm;
	char timebuf[64];
	std::string sstring;

	buf = moduleGetLastBuffer();
	arg = myStrGetToken(buf, ' ', 0);
	arg2 = myStrGetToken(buf, ' ', 1);

	if (!buf || !arg)
	{
		do_restrict_syntax(u);
	}
	else if (!arg2 && stricmp(arg, "LIST"))
	{
		do_restrict_syntax(u);
	}
	else if (!stricmp(arg, "LIST"))
	{
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
	else if (!stricmp(arg, "ADD"))
	{
		sstring = arg2;

		if (RestrictedHosts.AddHost(sstring, std::string(u->nick)))
			me->NoticeLang(s_vHostServ, u, LNG_RESHOST_ADD, arg2);
		else
			me->NoticeLang(s_vHostServ, u, LNG_RESHOST_ADDFAIL, arg2);
	}
	else if (!stricmp(arg, "DEL"))
	{
		sstring = arg2;

		if (RestrictedHosts.DelHost(sstring))
			me->NoticeLang(s_vHostServ, u, LNG_RESHOST_DEL, arg2);
		else
			me->NoticeLang(s_vHostServ, u, LNG_RESHOST_DELFAIL, arg2);
	}

	if (arg)
		delete [] arg;
	if (arg2)
		delete [] arg2;

	return MOD_CONT;
}

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
				if (u->vident)
					delete [] u->vident;
				u->vident = sstrdup(ident);
			}
			if (u->vhost)
				delete [] u->vhost;
			u->vhost = sstrdup(host);

			if (ident)
			{
				alog("%s: Set the vHost of %s (%s@%s) to \2%s@%s\2", s_vHostServ, u->nick, u->username, u->host, ident, host);
				me->NoticeLang(s_vHostServ, u, LNG_SETHOST_IDENT, ident, host);
			}
			else
			{
				alog("%s: Set the vHost of %s (%s@%s) to \2%s\2", s_vHostServ, u->nick, u->username, u->host, host);
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

			me->AddCallback("unban", time(NULL) + BanTimeT, delBan, 2, cb);
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

int delBan(int arg, char **argv)
{
	const char *av[3];
	ChannelInfo *ci;

	av[0] = "-b";
	av[1] = argv[1];

	if ((ci = cs_findchan(argv[0])) && ci->c)
	{
		ircdproto->SendMode(whosends(ci), ci->name, "-b %s", av[1]);
		chan_set_modes(s_ChanServ, ci->c, 2, av, 1);
	}

	return MOD_CONT;
}

int do_reload(int argc, char **argv)
{
	Module_vHostServ::LoadConfig();
	
	return MOD_CONT;
}

int do_connect(int argc, char **argv)
{
	vHostServ = findbot(BotNick.c_str());

	if (!vHostServ)
	{
		vHostServ = new BotInfo(BotNick.c_str(), BotIdent.c_str(), BotHost.c_str(), BotRealName.c_str());
		ircdproto->SendClientIntroduction(BotNick.c_str(), BotIdent.c_str(), BotHost.c_str(), BotRealName.c_str(), BotModes.c_str(), vHostServ->uid.c_str());
	}

	vHostServ->cmdTable = VHOSTSERV;
	vHostServ->flags |= BI_PRIVATE;
	
	return MOD_CONT;
}

MODULE_INIT("vhostserv", Module_vHostServ)
