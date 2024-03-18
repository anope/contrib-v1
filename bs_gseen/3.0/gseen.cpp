/* dont remove the following line!1!      */
/* RequiredLibraries: mysqlpp,mysqlclient,xml2 */

#include "gseen.h"

mysqlpp::Connection con;
mysqlpp::Query query = con.query();
langs lng;


class BS_GSEEN : public Module
{
 public:
	BS_GSEEN(const std::string &modname, const std::string &creator) : Module(modname, creator)
	{
		this->SetAuthor("Anope");
		this->SetVersion("$Id$");
		this->SetType(THIRD);
		this->OnReload(true);
		this->AddCommand(CHANSERV, new CommandSEEN());
		ModuleManager::Attach(I_OnReload, this);
		ModuleManager::Attach(I_OnUserConnect, this);
 		ModuleManager::Attach(I_OnUserNickChange, this);
 		ModuleManager::Attach(I_OnUserQuit, this);
		ModuleManager::Attach(I_OnJoinChannel, this);
		ModuleManager::Attach(I_OnPartChannel, this);
		ModuleManager::Attach(I_OnUserKicked, this);
	}


	void OnReload(bool starting)
	{
		ConfigReader config;
		std::string mysqlHost, mysqlName, mysqlUser, mysqlPass, mysqlPort;

		mysqlHost = config.ReadValue("database", "host", 0);
		mysqlPort = config.ReadValue("database", "port", 0);
		mysqlName = config.ReadValue("database", "name", 0);
		mysqlUser = config.ReadValue("database", "user", 0);
		mysqlPass = config.ReadValue("database", "pass", 0);

		if (mysqlName.size() == 0) throw ModuleException("missing configuration parameter: database name");
		if (mysqlHost.size() == 0) throw ModuleException("missing configuration parameter: database host");
		if (mysqlUser.size() == 0) throw ModuleException("missing configuration parameter: database user");
		if (mysqlPass.size() == 0) throw ModuleException("missing configuration parameter: database pass");
		if (mysqlPort.size() > 0)
		{
			mysqlHost += ":";
			mysqlHost += mysqlPort;
		}
		if (!con.connect(mysqlName.c_str(), mysqlHost.c_str(), mysqlUser.c_str(), mysqlPass.c_str()))
			throw ModuleException("Error connecting to the MySQL Database");
		lng.load("gseen.xml");
		SQL_Initialize_DB();
	} // OnReload

	void SQL_Initialize_DB()
	{
		mysqlpp::StoreQueryResult res;

	// 	1. check for database and create it
		query.reset(); 
		query << "SHOW TABLES LIKE " << DBNAME << ";";
		query.parse();
		if (debug)
			alog("debug: %s", query.str().c_str());
		if ((res = query.store()) && (res.num_rows() == 0))
		{	// table does not exist, so create it
			query.reset();
			query << "CREATE TABLE " << DBNAME << " "
				<< "nick VARCHAR(128) NOT NULL, PRIMARY KEY (nick), "
				<< "type INT, "
				<< "host TEXT, "
				<< "vhost TEXT, "
				<< "chan VARCHAR(512), " // in some ircds the length is free configurable
				<< "msg TEXT, "
				<< "last BIGINT(14), "
				<< "spent INT, "  // not used atm
				<< "newnick VARCHAR(128)) " // in some ircds the length is free configurable
				<< "TYPE=myISAM ";
			query.parse();
			if (debug)
				alog("debug: %s", query.str().c_str());
			query.store();
		}
	/**
	 *	2. insert the cleanup-event into mysql (drop the old cron before)
	 *	damn! the EVENT system in mysql requires version 5.1
	 *	most stable linux distributions use still 5.0
	 *	so we have to cleanup the database with an external, cron-triggered script
	 */
	}
	void OnUserConnect(User *u)
	{
		std::string host, vhost;
		size_t now = time(NULL);

		host = u->GetIdent() + "@" + u->host;
		vhost = u->GetVIdent() + "@" + u->GetCloakedHost();
		query.reset();
		query << "INSERT INTO " << DBNAME << " "
			<< "(`nick`, `type`, `host`, `vhost`, `last`) "
			<< "VALUES (%0q, 1, %1q, %2q, %3q) "
			<< "ON DUPLICATE KEY UPDATE "
			<< "type=VALUES(type), host=VALUES(host), vhost=VALUES(vhost), "
			<< "last=VALUES(last);";
		query.parse();
		if (debug)
			alog("debug: %s", query.str(u->nick, host, vhost, now).c_str());
		query.store(u->nick, host, vhost, now);
	}

	void OnUserNickChange(User *u, const std::string &oldnick)
	{
		std::string host, vhost;
		size_t now = time(NULL);

		host = u->GetIdent() + "@" + u->host;
		vhost = u->GetVIdent() + "@" + u->GetCloakedHost();
		// build query 
		query.reset();
		query << "INSERT INTO " << DBNAME << " "
			<< "(`nick`, `type`, `host`, `vhost`, `last`, `newnick`) "
			<< "VALUES (%0q, %1q, %2q, %3q, %4q, %5q) "
			<< "ON DUPLICATE KEY UPDATE "
			<< "type=VALUES(type), host=VALUES(host), vhost=VALUES(vhost), "
			<< "last=VALUES(last), newnick=VALUES(newnick);";
		query.parse();
		// update old nick
		if (debug)
			alog("debug: %s", query.str(oldnick, 2, host, vhost, now, u->nick).c_str());
		query.store(oldnick, 2, host, vhost, now, u->nick);
		// update new nick
		if (debug)
			alog("debug: %s", query.str(u->nick, 3, host, vhost, now, oldnick).c_str());
		query.store(u->nick, 3, host, vhost, now, oldnick);
	}

	void OnUserQuit(User *u, const std::string &msg)
	{
		std::string host, vhost;
		size_t now = time(NULL);

		host = u->GetIdent() + "@" + u->host;
		vhost = u->GetVIdent() + "@" + u->GetCloakedHost();

		query.reset();
		query << "INSERT INTO " << DBNAME << " "
			<< "(nick, type, host, vhost, last, msg) "
			<< "VALUES (%0q, 6, %1q, %2q, %3q, %4q) "
			<< "ON DUPLICATE KEY UPDATE "
			<< "type=VALUES(type), host=VALUES(host), vhost=VALUES(vhost), "
			<< "last=VALUES(last), msg=VALUES(msg);";
		query.parse();
		if (debug)
			alog("debug: %s", query.str(u->nick, host, vhost, now, msg).c_str());
		query.store(u->nick, host, vhost, now, msg);
	}

	void OnJoinChannel(User *u, Channel *c)
	{
		std::string host, vhost;
		size_t now = time(NULL);

		host = u->GetIdent() + "@" + u->host;
		vhost = u->GetVIdent() + "@" + u->GetCloakedHost();

		query.reset();
		query << "INSERT INTO " << DBNAME << " "
			<< "(nick, type, host, vhost, last, chan) "
			<< "VALUES (%0q, 4, %1q, %2q, %3q, %4q) "
			<< "ON DUPLICATE KEY UPDATE "
			<< "type=VALUES(type), host=VALUES(host), vhost=VALUES(vhost), "
			<< "last=VALUES(last), chan=VALUES(chan);";
		query.parse();
		if (debug)
			alog("debug: %s", query.str(u->nick, host, vhost, now, c->name).c_str());
		query.store(u->nick, host, vhost, now, c->name);
	}

	void OnPartChannel(User *u, Channel *c, const std::string &msg)
	{
		std::string host, vhost;
		size_t now = time(NULL);

		host = u->GetIdent() + "@" + u->host;
		vhost = u->GetVIdent() + "@" + u->GetCloakedHost();

		query.reset();
		query << "INSERT INTO " << DBNAME << " "
			<< "(nick, type, host, vhost, last, chan, msg) "
			<< "VALUES (%0q, 5, %1q, %2q, %3q, %4q, %5q) "
			<< "ON DUPLICATE KEY UPDATE "
			<< "type=VALUES(type), host=VALUES(host), vhost=VALUES(vhost), "
			<< "last=VALUES(last), chan=VALUES(chan), msg=VALUES(msg);";
		query.parse();
		if (debug)
			alog("debug: %s", query.str(u->nick, host, vhost, now, c->name, msg).c_str());
		query.store(u->nick, host, vhost, now, c->name, msg);
	}

	void OnUserKicked(Channel *c, User *u, const std::string &source, const std::string &kickmsg)
	{
		std::string host, vhost;
		size_t now = time(NULL);

		host = u->GetIdent() + "@" + u->host;
		vhost = u->GetVIdent() + "@" + u->GetCloakedHost();

		query.reset();
		query << "INSERT INTO " << DBNAME << " "
			<< "(nick, type, host, vhost, last, chan, msg, newnick) "
			<< "VALUES (%0q, 7, %1q, %2q, %3q, %4q, %5q, %6q) "
			<< "ON DUPLICATE KEY UPDATE "
			<< "type=VALUES(type), host=VALUES(host), vhost=VALUES(vhost), "
			<< "last=VALUES(last), chan=VALUES(chan), msg=VALUES(msg), newnick=VALUES(newnick);";
		query.parse();
		if (debug)
			alog("debug: %s", query.str(u->nick, host, vhost, now, c->name, kickmsg, source).c_str());
		query.store(u->nick, host, vhost, now, c->name, kickmsg, source);
	}
};

MODULE_INIT(BS_GSEEN)

