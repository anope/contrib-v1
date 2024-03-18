/* m_tagfantasy.cpp - Adds fantasy command aliases similar to Atheme (see example below)
 * 
 * (C) 2012 Flux-net.net, Justasic
 * Contact me at Justin@Flux-Net.net or Justasic@gmail.com
 *
 * Please read COPYING and README for further details.
 *
 * This command adds Bot aliases/fantasy commands like so:
 *
 * <nenolod> ChanServ: topic atheme.org discussion - some repositories (libguess) still remain locked
 * »» ChanServ has changed the topic to: atheme.org discussion - some repositories (libguess) still remain locked
 *
 * Basically just tag a services' bot in a channel with a command and it executes the command for that channel
 * this command works as a fantasy forwarder (infact, it just takes the tag, makes sure its a bot, then runs it
 * through the fantasy system already in place with anope), which means the channel must be set with fantasy
 * commands enabled. I figured this was pretty cool so I made it for anope since I use anope and not atheme ;)
 */

#include "module.h"

// I like this alias, maybe some day Adam will think of adding it :)
namespace Anope { typedef std::vector<Anope::string> vector; }

class botalias : public Module
{
public:
	botalias(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator)
	{
		this->SetAuthor("Justasic");
		this->SetVersion("1.0.2");

		ModuleManager::Attach(I_OnPrivmsg, this);
	}
  
	void OnPrivmsg(User *u, Channel *c, Anope::string &msg)
	{
		Anope::vector params;
		spacesepstream(msg).GetTokens(params);

		if(params.size() <= 1)
			return;

		Anope::string bottag = params[0];

		if(bottag.empty())
			return;

		// There is probably some easier way of dealing with this, but meh.
		switch(bottag[bottag.length() - 1])
		{
			case ':':
			case ',':
			case ';':
			case '.':
			case '?':
			case '|':
			case '\'':
			case '`':
				bottag.erase(bottag.length() - 1);
				break;
			default:
				break;
		}

		// Make sure its a bot's nick and not someone saying something or we're not calling ourselfs >.>
		if(!BotInfo::Find(bottag))
			return;

		// Rebuild the string so that the core can parse it
		Anope::string cmd = "";
		for(unsigned i = 1; i < params.size(); ++i)
		{
			if((i == 1))
				cmd = Config->GetModule(this)->Get<const Anope::string>("fantasycharacter", "!") + params[i] + " ";
			else
				cmd += params[i] + " ";
		}
		cmd.trim();

		FOREACH_MOD(I_OnPrivmsg, OnPrivmsg(u, c, cmd));
	}
};

MODULE_INIT(botalias)
