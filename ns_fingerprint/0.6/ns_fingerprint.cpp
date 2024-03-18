#include "module.h"

#define AUTHOR "DukePyrolator"
#define VERSION "0.6"




void DoAutoIdentify(User *u)
{
	NickAlias *na = findnick(u->nick);
	char *fingerprint = NULL;

	if (!(na = findnick(u->nick)))
		return;
	if (u->nc && (u->nc == na->nc))
		return;
	if ((na->status & NS_FORBIDDEN) || (na->nc->flags & NI_SUSPENDED))
		return; 
	if (na->nc->access.empty()) // access list is empty 
		return;
	if (!u->GetExt("fingerprint", fingerprint)) // user is not using ssl
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
			u->nc = na->nc;

			ircdproto->SendAccountLogin(u, u->nc);
			ircdproto->SetAutoIdentificationToken(u);
			FOREACH_MOD(I_OnNickIdentify, OnNickIdentify(u));
			alog("%s: %s!%s@%s automatically identified for nick %s by SSL Fingerprint", s_NickServ, 
					u->nick, u->GetIdent().c_str(),	u->host, u->nick);
			u->SendMessage(s_NickServ, "SSL Fingerprint accepted. You are now recognized.");
			if (ircd->vhost)
			        do_on_id(u);
			if (NSModeOnID)
			        do_setmodes(u);
			if (NSForceEmail && u->nc && !u->nc->email)
			{
				notice_lang(s_NickServ, u, NICK_IDENTIFY_EMAIL_REQUIRED);
				notice_help(s_NickServ, u, NICK_IDENTIFY_EMAIL_HOWTO);
			}

			if (nick_identified(u))
				check_memos(u);
			/* Clear any timers */
			if (na->nc->flags & NI_KILLPROTECT)
				del_ns_timeout(na, 0); // TO_COLLIDE
		} 
	}
}

//   debug: Received: :409 METADATA 409AAAAAA ssl :ON
//   debug: Received: :409 METADATA 409AAAAAA ssl_cert :vTrSe c38070ce96e41cc144ed6590a68d45a6 <...> <...>
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
		fingerprint = data.substr(pos1,pos2-pos1);
		u->Extend("fingerprint", sstrdup(fingerprint.c_str()));
		DoAutoIdentify(u);
	}
	return EVENT_CONTINUE;
}

EventReturn doOnAccess(User *u, const std::vector<ci::string> &params)
{
		const char *cmd = params[0].c_str();
		const char *mask = params.size() > 1 ? params[1].c_str() : NULL;
		const char *fingerprint;
		if (!u->nc) return EVENT_CONTINUE; // nick not identified
		if (!mask) return EVENT_CONTINUE; // let it handle by ns_access
		if (strchr(mask, '@')) return EVENT_CONTINUE; // let it handle by ns_access 
		if (!stricmp(cmd,"ADD") && !stricmp(mask,"fingerprint"))
		{
			if (u->GetExt("fingerprint", fingerprint))
			{
				if (u->nc->access.size() >= NSAccessMax)
				{
					notice_lang(s_NickServ, u, NICK_ACCESS_REACHED_LIMIT, NSAccessMax);
					return EVENT_STOP;
				}
				if (u->nc->FindAccess(fingerprint))
				{
					notice_lang(s_NickServ, u, NICK_ACCESS_ALREADY_PRESENT, fingerprint);
					return EVENT_STOP;
				}
				u->nc->AddAccess(fingerprint);
				notice_lang(s_NickServ, u, NICK_ACCESS_ADDED, fingerprint);
				return EVENT_STOP;
			}
			else
			{
				u->SendMessage(s_NickServ, "You have no SSL Fingerprint to add");
				return EVENT_STOP;
			}
		}	
		if (!stricmp(cmd,"DEL"))
		{
			if (!stricmp(mask,"fingerprint"))
			{
				if (u->GetExt("fingerprint", fingerprint))
				{
					if (u->nc->FindAccess(fingerprint))
					{
						notice_lang(s_NickServ, u, NICK_ACCESS_DELETED, fingerprint);
						u->nc->EraseAccess(fingerprint);
					}
					else
					{
						notice_lang(s_NickServ, u, NICK_ACCESS_NOT_FOUND, fingerprint);
					}
				}
				else
				{
					u->SendMessage(s_NickServ, "You have no SSL Fingerprint to delete");
				}
				return EVENT_STOP;
			}
			else if (u->nc->FindAccess(mask))
			{
				
				notice_lang(s_NickServ, u, NICK_ACCESS_DELETED, mask);
				u->nc->EraseAccess(mask);
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

	void OnUserNickChange(User *u, const char *oldnick)  // called on event nickchange
	{
		DoAutoIdentify(u);
	}
	
	EventReturn OnPreCommand(User *u, const std::string &service, const ci::string &command, 
								const std::vector<ci::string> &params)
	{
		if (service == s_NickServ)
		{
			if (command == "ACCESS")
				return doOnAccess(u, params);
		}
		return EVENT_CONTINUE;
	}

	void OnUserLogoff(User *u)
	{
		const char *fingerprint;
		if (u->GetExt("fingerprint", fingerprint))
		{
			delete [] fingerprint;
			u->Shrink("fingerprint");
		}
	}

};

MODULE_INIT(NSFingerprint)

