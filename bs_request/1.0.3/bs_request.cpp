/* bs_request.cpp - Add request and activate functionality to BotServ,
 *
 * (C) 2012 Flux-net.net, Justasic
 * Contact me at Justin@Flux-Net.net or Justasic@gmail.com
 *
 * Based on the original HostServ module by Rob <rob@anope.org>
 * Based on the original Source of anope's HostServ module for
 * anope 1.9.6 and above
 *
 * Please read COPYING and README for further details.
 *
 */

/* Adds Nodes:
 * /msg BotServ Request  - botserv/request
 * /msg BotServ Activate - botserv/activate
 * /msg BotServ Reject   - botserv/reject
 * /msg BotServ Waiting  - botserv/waiting
 *
 * Adds Config variables:
 *
 * bs_request {
 *
 * # If set, services will send a memo to the user requesting
 * # the bot when its been approved or rejected
 *     memouser = yes|no
 *
 * # If set, Services will send a memo to all Services
 * # staff when a new vHost is requested.
 *     memooper = yes|no
 * }
 *
 * Copypasta config block for lamerz:

 module { name = "bs_request" }
 command { service = "BotServ"; name = "REQUEST"; command = "botserv/request"; }
 command { service = "BotServ"; name = "ACTIVATE"; command = "botserv/activate"; permission = "botserv/set"; }
 command { service = "BotServ"; name = "REJECT"; command = "botserv/reject"; permission = "botserv/set"; }
 command { service = "BotServ"; name = "WAITING"; command = "botserv/waiting"; permission = "botserv/set"; }
 bs_request {
   memouser = no
   memooper = no
 }
 
 * TODO:
 * Fix the bot activate/reject only using requester's nickname
 */

#include "module.h"
#include "memoserv.h"

static ServiceReference<MemoServService> MemoServService("MemoServService", "MemoServ");

static bool BSRequestMemoUser = false;
static bool BSRequestMemoOper = false;

void req_send_memos(CommandSource &source, const Anope::string&, const Anope::string&);

struct BotRequest : ExtensibleItem, Serializable
{
	Anope::string nick;
	Anope::string botnick;
	Anope::string ident;
	Anope::string host;
	Anope::string real;
	time_t time;

	BotRequest() : Serializable("BotRequest") { } 

	void Serialize(Serialize::Data &data) const anope_override
	{
		data["botnick"] << this->botnick;
		data["nick"] << this->nick;
		data["ident"] << this->ident;
		data["host"] << this->host;
		data["real"] << this->real;

		data.SetType("time", Serialize::Data::DT_INT);
		data["time"] << this->time;
	}

	static Serializable* Unserialize(Serializable *obj, Serialize::Data &data)
	{
		Anope::string snick;
		data["nick"] >> snick;

		NickAlias *na = NickAlias::Find(snick);
		if(na == NULL)
			return NULL;

		BotRequest *req;
		if(obj)
			req = anope_dynamic_static_cast<BotRequest *>(obj);
		else
			req = new BotRequest();

		req->nick = na->nick;
		data["botnick"] >> req->botnick;
		data["ident"] >> req->ident;
		data["host"] >> req->host;
		data["time"] >> req->time;
		data["real"] >> req->real;

		if(!obj)
			na->Extend("bs_request", req);

		return req;
	}
};

class CommandBSRequest : public Command
{

	bool IsValidBot(CommandSource &source, const std::vector<Anope::string> &params)
	{
		const Anope::string &nick = params[0];
		const Anope::string &user = params[1];
		const Anope::string &host = params[2];

		if (BotInfo::Find(nick))
		{
			source.Reply(_("Bot \002%s\002 already exists."), nick.c_str());
			return false;
		}

		if (nick.length() > Config->NickLen)
		{
			source.Reply(_("Bot Nicks may only contain valid nick characters."));
			return false;
		}

		if (user.length() > Config->UserLen)
		{
			source.Reply(_("Bot Idents may only contain %d characters."), Config->UserLen);
			return false;
		}

		if (host.length() > Config->HostLen)
		{
			source.Reply(_("Bot Hosts may only contain %d characters."), Config->HostLen);
			return false;
		}

		/* check for hardcored ircd forbidden nicks */
		if (!IRCD->IsNickValid(nick))
		{
			source.Reply(_("Bot Nicks may only contain valid nick characters."));
			return false;
		}

		/* Check the host is valid */
		if (!IRCD->IsHostValid(host))
		{
			source.Reply(_("Bot Hosts may only contain valid host characters."));
			return false;
		}

		if (!IRCD->IsIdentValid(user))
		{
			source.Reply(_("Bot Idents may only contain valid characters."), Config->UserLen);
			return false;
		}

		/* We check whether the nick is registered, and inform the user
		* if so. You need to drop the nick manually before you can use
		* it as a bot nick from now on -GD
		*/
		if (NickAlias::Find(nick))
		{
			source.Reply(NICK_ALREADY_REGISTERED, nick.c_str());
			return false;
		}

		return true;
	}

	public:
	CommandBSRequest(Module *creator) : Command(creator, "botserv/request", 4, 4)
	{
		this->SetDesc(_("Request a bot for your nick"));
		this->SetSyntax(_("\37nick\37 \37user\37 \37host\37 \37real\37"));
	}

	void Execute(CommandSource &source, const std::vector<Anope::string> &params)
	{
		User *u = source.GetUser();

		if(!u)
			return;

		NickAlias *na = NickAlias::Find(u->nick);
		if (na == NULL)
			na = NickAlias::Find(u->Account()->display);
		if (!na)
			return;

		const Anope::string &nick = params[0];
		const Anope::string &user = params[1];
		const Anope::string &host = params[2];
		const Anope::string &real = params[3];

		if(nick.empty() || user.empty() || host.empty())
		{
			this->OnSyntaxError(source, "");
			return;
		}

		if(!this->IsValidBot(source, params))
		  return;

		BotRequest *req = new BotRequest;
		req->nick = u->nick;
		req->botnick = nick;
		req->ident = user;
		req->host = host;
		req->real = real;
		req->time = Anope::CurTime;
		na->Extend("bs_request", req);

		source.Reply(_("Your bot has been requested"));
		req_send_memos(source, Anope::string(nick+"!"+user+"@"+host), real);
		Log(LOG_COMMAND, source, this, NULL) << "to request new bot " << nick << '!' << user << '@' << host << " " << real;

		return;
	}

	bool OnHelp(CommandSource &source, const Anope::string &subcommand)
	{
		this->SendSyntax(source);
		source.Reply(" ");
		source.Reply(_("Request the given bot to be actived for your nick by the\n"
			"network administrators. Please be patient while your request\n"
			"is being considered."));
		return true;
	}
};

class CommandBSActivate : public Command
{
 public:
	CommandBSActivate(Module *creator) : Command(creator, "botserv/activate", 1, 1)
	{
		this->SetDesc(_("Approve the requested bot of a user"));
		this->SetSyntax(_("\037requestnick\037"));
	}

	void Execute(CommandSource &source, const std::vector<Anope::string> &params)
	{
		User *u = source.GetUser();

		if(!u)
			return;

		const Anope::string &nick = params[0];

		NickAlias *na = NickAlias::Find(nick);
		BotRequest *req = na ? na->GetExt<BotRequest *>("bs_request") : NULL;
		if (req)
		{
		    BotInfo *bi = new BotInfo(req->botnick, req->ident, req->host, req->real);

		    if (BSRequestMemoUser && MemoServService)
		      MemoServService->Send(Config->BotServ, na->nick, _("[auto memo] Your requested bot has been approved."), true);

		    source.Reply(_("%s!%s@%s (%s) activated and added to the bot list."), bi->nick.c_str(), bi->GetIdent().c_str(), bi->host.c_str(), bi->realname.c_str());

		    Log(LOG_COMMAND, source, this, NULL) << "on bot request for " << req->nick << " adds bot " << bi->GetMask() << " (" << bi->realname << ") to the bot list";

		    na->Shrink("bs_request");
		    FOREACH_MOD(I_OnBotCreate, OnBotCreate(bi));
		}
		else
			source.Reply(_("No request for nick %s found."), nick.c_str());
	}

	bool OnHelp(CommandSource &source, const Anope::string &subcommand)
	{
		this->SendSyntax(source);
		source.Reply(" ");
		source.Reply(_("Activate the requested bot for the given nick."));
		if (BSRequestMemoUser)
			source.Reply(_("A memo informing the user will also be sent."));

		return true;
	}
};

class CommandBSReject : public Command
{
 public:
	CommandBSReject(Module *creator) : Command(creator, "botserv/reject", 1, 2)
	{
		this->SetDesc(_("Reject the requested bot of a user"));
		this->SetSyntax("\37requestnick\37 [\037reason\037]");
	}

	void Execute(CommandSource &source, const std::vector<Anope::string> &params)
	{
		User *u = source.GetUser();

		if(!u)
			return;

		const Anope::string &nick = params[0];
		const Anope::string &reason = params.size() > 1 ? params[1] : "";

		NickAlias *na = NickAlias::Find(nick);
		BotRequest *req = na ? na->GetExt<BotRequest*>("bs_request") : NULL;
		if (req)
		{
			na->Shrink("bs_request");

			if (BSRequestMemoUser && MemoServService)
			{
				Anope::string message;
				if (!reason.empty())
					message = Anope::printf(_("[auto memo] Your requested bot has been rejected. Reason: %s"), reason.c_str());
				else
					message = _("[auto memo] Your requested bot has been rejected.");

				MemoServService->Send(Config->BotServ, nick, message, true);
			}

			source.Reply(_("Bot for %s has been rejected"), nick.c_str());
			Log(LOG_COMMAND, source, this, NULL) << "on bot request for " << nick << (!reason.empty() ? " ("+reason+")" : "");
		}
		else
			source.Reply(_("No request for nick %s found."), nick.c_str());

		return;
	}

	bool OnHelp(CommandSource &source, const Anope::string &subcommand)
	{
		this->SendSyntax(source);
		source.Reply(" ");
		source.Reply(_("Reject the requested bot for the given nick."));
		if (BSRequestMemoUser)
			source.Reply(_("A memo informing the user will also be sent."));

		return true;
	}
};

class CommandBSWaiting : public Command
{
	void DoList(CommandSource &source)
	{
		int counter = 1;
		int from = 0, to = 0;
		unsigned display_counter = 0;
		ListFormatter list;

		list.AddColumn("Number").AddColumn("Nick").AddColumn("Bot").AddColumn("Created");

		for (nickalias_map::const_iterator it = NickAliasList->begin(), it_end = NickAliasList->end(); it != it_end; ++it)
		{
			NickAlias *na = it->second;
			BotRequest *br = na->GetExt<BotRequest *>("bs_request");
			if (!br)
				continue;

			if (((counter >= from && counter <= to) || (!from && !to)) && display_counter < Config->NSListMax)
			{
				++display_counter;

				ListFormatter::ListEntry entry;
				entry["Number"] = stringify(counter);
				entry["Nick"] = it->first;
				entry["Bot"] = br->botnick + " ("+br->ident+"@"+br->host+")";
				entry["Created"] = Anope::strftime(br->time);
				list.AddEntry(entry);
			}
			++counter;
		}
		source.Reply(_("Displayed all records (Count: \002%d\002)"), display_counter);

		std::vector<Anope::string> replies;
		list.Process(replies);
		

		for (unsigned i = 0; i < replies.size(); ++i)
			source.Reply(replies[i]);
	}

 public:
	CommandBSWaiting(Module *creator) : Command(creator, "botserv/waiting", 0, 0)
	{
		this->SetDesc(_("Retrieves the bot requests waiting for approval"));
		this->SetSyntax("");
	}

	void Execute(CommandSource &source, const std::vector<Anope::string> &params)
	{
		return this->DoList(source);
	}

	bool OnHelp(CommandSource &source, const Anope::string &subcommand)
	{
		this->SendSyntax(source);
		source.Reply(" ");
		source.Reply(_("This command retrieves the bot requests"));

		return true;
	}
};

class BSRequest : public Module
{
	Serialize::Type request_type;
	CommandBSRequest commandbsrequest;
	CommandBSActivate commandbsactive;
	CommandBSReject commandbsreject;
	CommandBSWaiting commandbswaiting;

 public:
	BSRequest(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator),
		request_type("BotRequest", BotRequest::Unserialize), commandbsrequest(this), commandbsactive(this), commandbsreject(this), commandbswaiting(this)
	{
		this->SetAuthor("Justasic");
		this->SetVersion("1.0.3");

		Implementation i[] = { I_OnReload };
		ModuleManager::Attach(i, this, sizeof(i) / sizeof(Implementation));

		this->OnReload();
	}

	~BSRequest()
	{
		for (nickalias_map::const_iterator it = NickAliasList->begin(), it_end = NickAliasList->end(); it != it_end; ++it)
			it->second->Shrink("bs_request");
	}

	void OnReload()
	{
		ConfigReader config;
		BSRequestMemoUser = config.ReadFlag("bs_request", "memouser", "no", 0);
		BSRequestMemoOper = config.ReadFlag("bs_request", "memooper", "no", 0);

		Log(LOG_DEBUG) << "[bs_request] Set config vars: MemoUser=" << BSRequestMemoUser << " MemoOper=" <<  BSRequestMemoOper;
	}
};

void req_send_memos(CommandSource &source, const Anope::string &botfullhost, const Anope::string &botreal)
{
  if (BSRequestMemoOper == 1 && MemoServService)
	  for (unsigned i = 0; i < Config->Opers.size(); ++i)
	  {
		  Oper *o = Config->Opers[i];

		  NickAlias *na = NickAlias::Find(o->name);
		  if (!na)
			  continue;

		  Anope::string message = Anope::printf(_("[auto memo] Bot \002%s (%s)\002 has been requested by %s."), botfullhost.c_str(), botreal.c_str(), source.GetUser() ? source.GetUser()->GetMask().c_str() : "(no user)");

		  MemoServService->Send(Config->BotServ, na->nick, message, true);
	  }
}

MODULE_INIT(BSRequest)
