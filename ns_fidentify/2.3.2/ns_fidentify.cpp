#include "module.h"
#define VER "2.3.2"
/************************************************
 * Force Identify Module
 * 
 * Origonal Module Author: n00bie
 * Date of Origonal: 14/01/07
 * Date of this version: 03/17/11
 * 
 * Code Developed by: Justasic
 * Code Developed for: irc.Flux-Net.net
 *************************************************
 * Module used in: whatever your config is
 * Syntax: /msg %s FORCEIDENTIFY [nickname] account
 * Adds command nickserv/fidentify to config
 *************************************************/
#define NICK_IDENTIFIED "\2%s\2 has already been identified"
#define NICK_NOW_IDENTIFIED "You have forcefully identified user \002%s\002 to account \002%s\002"
#define FORCE_IDENTIFIED "You where forced to identify by \002%s\002 to account \002%s\002"
#define NICK_X_IS_SERVICES "Nick \002%s\002 is part of this networks services"

class CommandNSFidentify : public Command
{
  public:
	CommandNSFidentify(Module *creator) : Command(creator, "nickserv/fidentify", 1, 2)
	{
	  this->SetDesc(_("Forcefully Identify a nickname to an account"));
	  this->SetSyntax(_("[\037nickname\037] account"));
	}

	void Execute(CommandSource &source, const std::vector<Anope::string> &params)
	{
	  const Anope::string &nick = params[0];
	  const Anope::string &acc = params.size() == 2 ? params[1] : params[0];

          User *u = source.GetUser();
	  User *u2 = finduser(nick);
	  NickAlias *na = findnick(acc);
	  NickAlias *ana = findnick(u->nick);
	  BotInfo *bi = source.owner;

	  if(!u)
		return;
	  
	  if(!u2)
	  {
	      if(nickIsServices(nick, true))
	      {
		  source.Reply(_(NICK_X_IS_SERVICES), nick.c_str());
		  return;
	      }else{
		  source.Reply(_(NICK_X_NOT_IN_USE), nick.c_str());
		  return;
	      }
	  }
	  if(!na || !ana){
	    source.Reply(_("Account \2%s\2 does not exsist"), acc.c_str());
	  }
	  else if(u2 == u)
	      source.Reply(_(NICK_IDENTIFIED), nick.c_str());
	  else if(!ana->nc->IsServicesOper())
	      source.Reply(ACCESS_DENIED);
	  else if(na && na->nc->HasFlag(NI_SUSPENDED))
	      source.Reply(_(NICK_X_SUSPENDED), na->nick.c_str());
	  else if(u2->Account() && na && u2->Account() == na->nc)
	      source.Reply(_(NICK_IDENTIFIED), nick.c_str());
	  else
	    { 
	      Log(LOG_OVERRIDE, source, this, NULL) << "to force identify " << u->nick << " to user account \"" << na->nc->display << "\"";			
	      source.Reply(_(NICK_NOW_IDENTIFIED), nick.c_str(), acc.c_str());
	      u2->SendMessage(bi, _(FORCE_IDENTIFIED), u->nick.c_str(), acc.c_str());
	      u2->Identify(na);
	    }
	    return;
    } 
	bool OnHelp(CommandSource &source, const Anope::string &subcommand)
	{ 
	  this->SendSyntax(source);
	  source.Reply(" ");
	  source.Reply(_("Forcefully identifies a user to their nickname or to a specifiec account."));
	  return true;
	}
};

class NSFidentify : public Module
{
	CommandNSFidentify commandnsfidentify;

 public:
	NSFidentify(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator),	
		commandnsfidentify(this)
	{
		this->SetAuthor("Justasic");
		this->SetVersion(VER);
	}
};

MODULE_INIT(NSFidentify) 
