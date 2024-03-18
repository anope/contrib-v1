/** ircd_globopsonreg.c - Sends GLOBOPS on channel/nickname register.
 *
 * Maintainer/Author: Taros
 * Date: 2009-02-09
 *  
 * Sends a GLOBOPS when a new channel/nickname is registered.
 * Based on Viper's ircd_wallonreg.c for previous versions of anope.
 * Ported to Anope 1.9 by Taros on the Tel'Laerad M&D Team
 * <http://tellaerad.net>
 */
/******************************************************************************/

#include "module.h"

#define AUTHOR "Taros"
#define VERSION "1.1"

int chan_regged(int ac, char **av);
int nick_regged(int ac, char **av);

static Module *me;


class GlobopsOnReg : public Module
{
 public:
	GlobopsOnReg(const std::string &modname, const std::string &creator) : Module(modname, creator)
	{
		EvtHook *hook;

		me = this;

                this->SetAuthor(AUTHOR);
                this->SetVersion(VERSION);

		hook = createEventHook(EVENT_CHAN_REGISTERED, chan_regged);
                me->AddEventHook(hook);

		hook = createEventHook(EVENT_NICK_REGISTERED, nick_regged);
                me->AddEventHook(hook);
	}
};

int chan_regged(int ac, char **av) {
	ChannelInfo *ci;
	User *u;

	if (ac != 1)
		return MOD_CONT;

	if ((ci = cs_findchan(av[0]))) {
		if ((u = finduser(ci->founder->display)))
			ircdproto->SendGlobops(s_ChanServ, "New channel ( %s ) registered by %s!%s@%s",  ci->name, u->nick, u->username, common_get_vhost(u));
		else
			ircdproto->SendGlobops(s_ChanServ, "New channel ( %s ) registered by %s",  ci->name, ci->founder->display);
	}

	return MOD_CONT;
}

int nick_regged(int ac, char **av) {
	User *u;
	if (ac != 1)
		return MOD_CONT;

	if ((u = finduser(av[0])))
		ircdproto->SendGlobops(s_NickServ, "New Nickname ( %s ) Registered by %s@%s", u->nick, u->username, common_get_vhost(u));

	return MOD_CONT;
}

MODULE_INIT("ircd_globopsonreg", GlobopsOnReg)
