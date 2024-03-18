/*----------------------------------------------------------------------------------------
 * Name:        cs_accessfounder
 * Author:      Jens 'DukePyrolator' Voss
 *              <DukePyrolator@anope.org>
 * Version:     4.1
 *
 * ---------------------------------------------------------------------------------------
 * Supported IRCd:      all
 * Requires:            Anope1.8.0, runs fine with 1.8.2
 *----------------------------------------------------------------------------------------
 *
 * If you find bugs, please send me a Mail or contact me on irc.anope.org #anope
 * here you can find the latest version: http://dev.anope.de/modules_1.8/cs_accessfounder.c
 *
 *---------------------------------------------------------------------------------------
 * Credits
 *  - thanks to all the people in #anope :-)
 *  - special thanks to Trystan who maintained my module when I was inactive
 *
 *----------------------------------------------------------------------------------------
 * Changelog:
 *
 * v4.1 22/07/2009	- uuups, I did it again (fixed a small, but annoying bug)
 * v4.0	22/07/2009	- users with founderaccess are now logged out if removed from
 *			  the access list (this feature was requested by many peoples)
 *			- some code cleanup
 * v3.3	03/10/2006	- module should now work with any ircd that supports owner
 *			- removed the function "is_identified" from the module because
 *			  its no longer declared as "static" in the core.
 *			- removed the function "is_real_founder" from the module because
 *			  its no longer declared as "static" in the core.
 *			- added a small workaround for an issue, where founder status is
 *			  not deleted on logout.
 * v3.2	09/17/2006	- dont give any modes if autoop is off
 * v3.1	09/16/2006	- first update since 2004 
 *			  works now with anope 1.7.15
 *
 *----------------------------------------------------------------------------------------*/


#define AUTHOR "Jens 'DukePyrolator' Voss <DukePyrolator@anope.org>"
#define VERSION "4.1"

#include "module.h"
#include <version.h>

/* access level for auto +q */
#define CA_AUTOQ 9999

int do_on_join(int ac, char **av);
int do_on_identify(int ac, char **av);
int do_on_update(User *u);
int do_on_logout(int ac, char **av);
int do_on_access_del(int ac, char **av);
int do_set_founder(User * u);
void del_founder_status(Channel *c, User *u);

int AnopeInit(int argc, char **argv) {
	Command *c = NULL;
	EvtHook *hook;
	int status = 0;


	if (!ircd->owner)
	{
		alog("[cs_accessfounder] your ircd is not supported. sorry");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_JOIN_CHANNEL, do_on_join);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK)
	{
		alog("[cs_accessfounder] unable to bind to EVENT_JOIN_CHANNEL error [%d]", status);
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_NICK_IDENTIFY, do_on_identify);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK)
	{
		alog("[cs_accessfounder] unable to bind to EVENT_NICK_IDENTIFY error [%d]", status);
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_NICK_LOGOUT, do_on_logout);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK)
	{
		alog("[cs_accessfounder] unable to bind to EVENT_NICK_LOGOUT error [%d]", status);
		return MOD_STOP;
	}
	hook = createEventHook(EVENT_ACCESS_DEL, do_on_access_del);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK)
	{
		alog("[cs_accessfounder] unable to bind to EVENT_ACCESS_DEL error [%d]", status);
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_ACCESS_CHANGE, do_on_access_del); // yes, we bind it to "del" !
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK)
	{
		alog("[cs_accessfounder] unable to bind to EVENT_ACCESS_CHANGE error [%d]", status);
		return MOD_STOP;
	}


	c = createCommand("UPDATE", do_on_update, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(NICKSERV, c, MOD_HEAD);

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	moduleSetType(THIRD);

	return MOD_CONT;
}


int do_on_join(int ac, char **av) 
{
	User *u;
	Channel *c = NULL;

	struct u_chaninfolist *uc;
	if (!stricmp(av[0], EVENT_START)) 
	{
		c = findchan(av[2]);
		if (!c || !c->ci) return MOD_CONT; /* chan not registered */
		if (!(u = finduser(av[1]))) return MOD_CONT;
		if (!nick_identified(u)) return MOD_CONT;
		if ((get_access_level(c->ci, u->na)>=CA_AUTOQ) && (!is_real_founder(u,c->ci)))
		{
			if (!is_identified(u, c->ci)) 
			{
				uc = scalloc(sizeof(*uc), 1);
				uc->next = u->founder_chans;
				if (u->founder_chans) 
					u->founder_chans->prev = uc;
				u->founder_chans = uc;
				uc->chan = c->ci;
			}
		}
	}
	return MOD_CONT;
}



int do_on_identify(int ac, char **av)
{
	User *u;
	u = finduser(av[0]);
	if (!nick_identified(u))
		return MOD_CONT;
	if (NSModeOnID) 
		do_set_founder(u);
	return MOD_CONT;
}

int do_on_update(User *u)
{
	if (!nick_identified(u))
		return MOD_CONT;
	if (NSModeOnID) 
		do_set_founder(u);
	return MOD_CONT;
}

int do_on_logout(int ac, char **av)
{
	User *u;
	struct u_chaninfolist *ci, *ci2;

	 u = finduser(av[0]);
	/* deletes founder-status from all channels */
	ci = u->founder_chans;
	while (ci)
	{
		ci2 = ci->next;
		free(ci);
		ci = ci2;
	}
	u->founder_chans = NULL;  /* never point into unassigned memory */
	return MOD_CONT;
}


int do_set_founder(User * u) 
{
	struct u_chanlist *uc;
	struct u_chaninfolist *uci;
	Channel *c;

	/* Walk users current channels */
	for (uc = u->chans; uc; uc = uc->next) 
	{
		if ((c = findchan(uc->chan->name)) && (c->ci))
		{
			if (should_mode_change(uc->status, CUS_OWNER)
					&& ((get_access_level(c->ci,u->na)>=CA_AUTOQ)
					&& (!is_real_founder(u,c->ci))))
			{
				if (!is_identified(u, c->ci))
				{
					uci = scalloc(sizeof(*uci), 1);
					uci->next = u->founder_chans;
					if (u->founder_chans)
						u->founder_chans->prev = uci;
					u->founder_chans = uci;
					uci->chan = c->ci;
				}
			}
		}
	}
	return MOD_CONT;
}

/* send_event(EVENT_ACCESS_DEL, 3, ci->name, u->nick, nick); */
/* send_event(EVENT_ACCESS_CHANGE, 4, ci->name, u->nick, na->nick, event_access); */

int do_on_access_del(int ac, char **argv)
{
	Channel *c;
	User *u;

	if (ac < 3)	// the deleted nick is not online, so nothing to do for us
		return MOD_CONT;
	if (!(c = findchan(argv[0])) || !c->ci)	// channel is not registered
		return MOD_CONT;
	if (!(u = finduser(argv[2])))	// should never happen, but a double-check is better than a segfault ;)
		return MOD_CONT;
	del_founder_status(c, u);
	return MOD_CONT;
}

void del_founder_status(Channel *c, User *u)
{
	struct u_chaninfolist *ci, *ci2;

	ci = u->founder_chans;
	while (ci)
	{
		ci2 = ci->next;
		if (ci->chan == c->ci) // we found the right channel
		{
			if (ci->next)
				ci->next->prev = ci->prev;
			if (ci->prev)
				ci->prev->next = ci->next;
			else
				u->founder_chans = ci->next;
			free(ci);
		}
		ci = ci2;
	}
}
