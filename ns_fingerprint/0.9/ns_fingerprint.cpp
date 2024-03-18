#include "module.h"

#define AUTHOR "DukePyrolator"
#define VERSION "0.9"




void DoAutoIdentify(User *u)
{
	NickAlias *na = findnick(u->nick);
	std::string fingerprint;

	if (!na)
		return;
	if (u->IsIdentified() && (u->Account() == na->nc))
		return;
	if ((na->HasFlag(NS_FORBIDDEN)) || (na->nc->HasFlag(NI_SUSPENDED)))
		return; 
	if (na->nc->access.empty()) // access list is empty 
		return;
	if (!u->GetExtRegular("fingerprint", fingerprint)) // user is not using ssl
		return;
	
	for (unsigned i = 0; i < na->nc->access.size(); i++)
	{	
		std::string access = na->nc->GetAccess(i);
		if (Anope::Match(fingerprint, access, false))
		{
			u->UpdateHost();
			if (na->last_realname)
				delete [] na->last_realname;
			na->last_realname = sstrdup(u->realname);
			na->last_seen = time(NULL);
			u->Login(na->nc);

			ircdproto->SendAccountLogin(u, u->Account());
			ircdproto->SetAutoIdentificationToken(u);
			FOREACH_MOD(I_OnNickIdentify, OnNickIdentify(u));
			Alog() << Config.s_NickServ << ": " << u->GetMask() << "automatically identified for nick " << u->nick << " by SSL Fingerprint";
			u->SendMessage(Config.s_NickServ, "SSL Fingerprint accepted. You are now recognized.");
			if (ircd->vhost)
			        do_on_id(u);
			if (Config.NSModeOnID)
			        do_setmodes(u);
			if (Config.NSForceEmail && u->IsIdentified() && !u->Account()->email)
			{
				notice_lang(Config.s_NickServ, u, NICK_IDENTIFY_EMAIL_REQUIRED);
				notice_help(Config.s_NickServ, u, NICK_IDENTIFY_EMAIL_HOWTO);
			}

			if (u->IsIdentified())
				check_memos(u);
		} 
	}
}

//   debug: Received: :409 METADATA 409AAAAAA ssl :ON
//   debug: Received: :409 METADATA 409AAAAAA ssl_cert :vTrSe c38070ce96e41cc144ed6590a68d45a6 <...> <...>
//   Debug: Received: :409 METADATA 409AAAAAC ssl_cert :vTrSE Could not get peer certificate: error:00000000:lib(0):func(0):reason(0)
//   source = numeric of the sending server
//   ac     = number of parameters (default 3 ?) 
//   av[0]  = uuid
//   av[1]  = metadata name
//   av[2]  = data

int DoOnMetaData(const char *source, int ac, const char **av)
{
	std::string data = av[2];
	User* u;
	std::string fingerprint;
	size_t pos1, pos2;

	if (ac != 3) return MOD_CONT; // received invalid METADATA
	if (!stricmp(av[1],"ssl_cert") && (u = find_byuid(av[0])))
	{
		pos1 = data.find(' ') + 1;
		pos2 = data.find(' ', pos1);
		fingerprint.assign(data, pos1,pos2-pos1);
		if (fingerprint.size() < 30) // not a valid fingerprint
			return EVENT_CONTINUE;
		u->Extend("fingerprint", new ExtensibleItemRegular<std::string>(fingerprint));
		DoAutoIdentify(u);
	}
	return EVENT_CONTINUE;
}

EventReturn doOnAccess(User *u, const std::vector<ci::string> &params)
{
		const char *cmd = params[0].c_str();
		const char *mask = params.size() > 1 ? params[1].c_str() : NULL;
		std::string fingerprint;
		if (!u->IsIdentified()) return EVENT_CONTINUE; // nick not identified
		if (!mask) return EVENT_CONTINUE; // let it handle by ns_access
		if (strchr(mask, '@')) return EVENT_CONTINUE; // let it handle by ns_access 
		if (!stricmp(cmd,"ADD") && !stricmp(mask,"fingerprint"))
		{
			if (u->GetExtRegular("fingerprint", fingerprint))
			{
				if (u->Account()->access.size() >= Config.NSAccessMax)
				{
					notice_lang(Config.s_NickServ, u, NICK_ACCESS_REACHED_LIMIT, Config.NSAccessMax);
					return EVENT_STOP;
				}
				if (u->Account()->FindAccess(fingerprint))
				{
					notice_lang(Config.s_NickServ, u, NICK_ACCESS_ALREADY_PRESENT, fingerprint.c_str());
					return EVENT_STOP;
				}
				u->Account()->AddAccess(fingerprint);
				notice_lang(Config.s_NickServ, u, NICK_ACCESS_ADDED, fingerprint.c_str());
				return EVENT_STOP;
			}
			else
			{
				u->SendMessage(Config.s_NickServ, "You have no SSL Fingerprint to add");
				return EVENT_STOP;
			}
		}	
		if (!stricmp(cmd,"DEL"))
		{
			if (!stricmp(mask,"fingerprint"))
			{
				if (u->GetExtRegular("fingerprint", fingerprint))
				{
					if (u->Account()->FindAccess(fingerprint))
					{
						notice_lang(Config.s_NickServ, u, NICK_ACCESS_DELETED, fingerprint.c_str());
						u->Account()->EraseAccess(fingerprint);
					}
					else
					{
						notice_lang(Config.s_NickServ, u, NICK_ACCESS_NOT_FOUND, fingerprint.c_str());
					}
				}
				else
				{
					u->SendMessage(Config.s_NickServ, "You have no SSL Fingerprint to delete");
				}
				return EVENT_STOP;
			}
			else if (u->Account()->FindAccess(mask))
			{
				
				notice_lang(Config.s_NickServ, u, NICK_ACCESS_DELETED, mask);
				u->Account()->EraseAccess(mask);
				return EVENT_STOP;
			}
		}
		return EVENT_CONTINUE;
}


class NSFingerprint : public Module
{ 
 public:
	NSFingerprint(const std::string &modname, const std::string &creator) : Module(modname, creator)
	{ /* constructor */

		addMessage(IRCD, createMessage("METADATA", DoOnMetaData), 1); 
		ModuleManager::Attach(I_OnUserNickChange, this);
		ModuleManager::Attach(I_OnPreCommand, this);
		ModuleManager::Attach(I_OnUserLogoff, this);
		this->SetAuthor(AUTHOR);
		this->SetVersion(VERSION);
		this->SetType(THIRD);

	}
	~NSFingerprint()
	{
		delMessage(IRCD, findMessage(IRCD, "METADATA"));
	}

	void OnUserNickChange(User *u, const std::string &oldnick)  // called on event nickchange
	{
		DoAutoIdentify(u);
	}
	
	EventReturn OnPreCommand(User *u, const std::string &service, const ci::string &command, 
								const std::vector<ci::string> &params)
	{
		if (service == Config.s_NickServ)
		{
			if (command == "ACCESS")
				return doOnAccess(u, params);
		}
		return EVENT_CONTINUE;
	}
/*
	void OnUserLogoff(User *u)
	{
		std::string fingerprint;
		if (u->GetExt("fingerprint"))
		{
			u->Shrink("fingerprint");
		}
	}
*/
};

MODULE_INIT(NSFingerprint)

