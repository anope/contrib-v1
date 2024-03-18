#include "module.h"

#define AUTHOR "Adam"
#define VERSION "3.1.0"

#define DATABASEFILE "swhois.db"

std::map<NickCore *, std::string> SWhoises;

class CommandOSSwhois : public Command
{
	public:
		CommandOSSwhois(const std::string &cname) : Command(cname, 1, 3)
		{
		}

		CommandReturn Execute(User *u, std::vector<ci::string> &params)
		{
			std::map<NickCore *, std::string>::iterator it;
			int i;
			NickAlias *na;

			if (params[0] == "LIST")
			{
				if (SWhoises.empty())
				{
					u->SendMessage(s_OperServ, "The SWhois list is empty.");
				}
				else
				{
					u->SendMessage(s_OperServ, "SWhois list:");
					u->SendMessage(s_OperServ, "Num Nick SWhois");

					for (it = SWhoises.begin(), i = 1; it != SWhoises.end(); ++it, ++i)
					{
						u->SendMessage(s_OperServ, "%d %s: %s", i, it->first->display, it->second.c_str());
					}
				}
			}
			else if (params.size() < 2)
				OnSyntaxError(u);
			else if (!(na = findnick(params[1].c_str())))
			{
				notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, params[1].c_str());
			}
			else if (params[0] == "DEL")
			{
				it = SWhoises.find(na->nc);

				if (it != SWhoises.end())
				{
					SWhoises.erase(it);
					u->SendMessage(s_OperServ, "\2%s\2 removed from the SWhois list.", na->nc->display);
				}
				else
					u->SendMessage(s_OperServ, "\2%s\2 not found on the SWhois list.", na->nc->display);
			}
			else if (params.size() < 3)
				OnSyntaxError(u);
			else if (params[0] == "ADD")
			{
				it = SWhoises.find(na->nc);

				if (it != SWhoises.end())
					SWhoises.erase(it);

				SWhoises.insert(std::make_pair(na->nc, params[2].c_str()));

				u->SendMessage(s_OperServ, "\2%s\2 added to the SWhois list", na->nc->display);
			}
			else
				OnSyntaxError(u);

			return MOD_CONT;
		}

		void OnSyntaxError(User *u)
		{
			u->SendMessage(s_OperServ, "Syntax: \x2SWHOIS {ADD | DEL | LIST} [\x1Fnick\x1F] [\x1Fswhois\x1F]\x2");
			u->SendMessage(s_OperServ, "\x2/msg %s HELP SWHOIS\x2 for more information.", s_OperServ);
		}

		bool OnHelp(User *u, const ci::string &subcommand)
		{
			u->SendMessage(s_OperServ, "Syntax: \x2SWHOIS {ADD | DEL | LIST} [\x1Fnick\x1F] [\x1Fswhois\x1F]\x2");
			u->SendMessage(s_OperServ, " ");
			u->SendMessage(s_OperServ, "Allows you to assign or remove swhoises");
			u->SendMessage(s_OperServ, "for a given user.");
			return true;
		}
};

class OSSWhois : public Module
{
	public:
		OSSWhois(const std::string &modname, const std::string &creator) : Module(modname, creator)
		{
			SetAuthor(AUTHOR);
			SetVersion(VERSION);

			this->AddCommand(OPERSERV, new CommandOSSwhois("SWHOIS"));

			Implementation i[] = {I_OnNickIdentify, I_OnDelCore, I_OnPostCommand, I_OnSaveDatabase};
			ModuleManager::Attach(i, this, 4);

			LoadDataBase();
		}

		~OSSWhois()
		{
			WriteDataBase();
		}

		void OperServHelp(User *u)
		{
			u->SendMessage(s_OperServ, "    SWHOIS    Manage swhois list");
		}

		void OnNickIdentify(User *u)
		{
			if (!u->nc)
				return;

			std::map<NickCore *, std::string>::iterator it = SWhoises.find(u->nc);

			if (it != SWhoises.end())
			{
				ircdproto->SendSWhois(s_NickServ, u->nick, it->second.c_str());
				u->SendMessage(s_NickServ, "Your SWhois of \2%s\2 is now activated.", it->second.c_str());
			}
		}

		void OnDelCore(NickCore *nc)
		{
			std::map<NickCore *, std::string>::iterator it = SWhoises.find(nc);

			if (it != SWhoises.end())
			{
				SWhoises.erase(it);
			}
		}

		void OnPostCommand(User *u, const std::string &service, const ci::string &command, const std::vector<ci::string> &params)
		{
			if (nick_identified(u) && service == s_NickServ && command == "UPDATE")
			{
				OnNickIdentify(u);
			}
		}

		void LoadDataBase()
		{
			std::ifstream conf(DATABASEFILE);
			char line[512];
			std::string buf;
			NickCore *nc;

			if (conf.is_open())
			{
				while (conf.getline(line, sizeof(line)))
				{
					spacesepstream ss(line);

					ss.GetToken(buf);

					if (!buf.empty() && (nc = findcore(buf.c_str())))
					{
						buf = ss.GetRemaining();

						while (buf[buf.size() - 1] == '\n')
							buf.erase(buf.end());

						SWhoises.insert(std::make_pair(nc, buf));
					}
				}

				conf.close();
			}
		}

		void WriteDataBase()
		{
			std::map<NickCore *, std::string>::iterator it;
			std::ofstream db;
			char buf[512];

			db.open(DATABASEFILE);

			if (!db.is_open())
			{
				alog("os_swhois: error, unable to open database file for writing!");
				return;
			}

			for (it = SWhoises.begin(); it != SWhoises.end(); ++it)
			{
				snprintf(buf, sizeof(buf), "%s %s\n", it->first->display, it->second.c_str());
				db.write(buf, strlen(buf));
			}

			db.close();
		}

		void OnSaveDatabase()
		{
			WriteDataBase();
		}
};

MODULE_INIT(OSSWhois)
