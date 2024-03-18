/**
 * -----------------------------------------------------------------------------
 * Name: cs_autoowner
 * Author: djGrrr <djgrrr@p2p-network.net>
 * Date: 2006/10/04  (Last update: 2006/10/04)
 * -----------------------------------------------------------------------------
 * Limitations: AutoOwner LEVEL not configurable per channel, only via a global
 * services.conf setting. Only LEVELS are supported (not XOP)
 * Tested: Anope-1.7.15 + UnrealIRCd 3.2.5
 * -----------------------------------------------------------------------------
 * This module has been tested on Anope-1.7.15, it might work on older
 * 1.7 versions, but its not supported.
 *
 * This module allows allows channel founders to give users the channel owner
 * mode (+q in Unreal) on ircd's that support it.
 *
 * This module is a near complete rewrite of the cs_accessfounder module by Jens
 * Some of the code is the same as the original, but a lot of it is changed or
 * removed. Many thanks to Jens for writting the original module :) I decided to
 * use a different name than the original because its name was missleading and
 * was not intended to actually give founder access to the channel.
 *
 * All the bugs of the original module _should_ be fixed
 * This is my first module, so it may have a few bugs, but hopefully it doesn't :)
 * If you do find any bugs, please let me know asap via email or on IRC at
 * irc.p2p-network.net with the nick djGrrr :)
 * -----------------------------------------------------------------------------
 *
 * Changes:
 *
 *  1.0  First release by me
 *       Now using Hook events instead of raw messages
 *       IDENTIFY / UPDATE trigger actually works now
 *       Auto Owner level can now be changed with a config varible, rather than
 *       a hard coded define
 *       Can be turned on/off with a simple config file change and /OS RELOAD
 *       Fixed a lot of bugs that was giving users level 10000 (real founder)
 *       Now supports all ircds that have an owner mode (hopefully)
 *
 * -----------------------------------------------------------------------------
 *
 * Config to be placed in services.conf

# AutoOwner [OPTIONAL]
# Module: cs_autoowner
#
# Automatically give users owner level on channels where the users are at level
# specified by AutoOwnerLevel or higher
#
AutoOwner

# AutoOwnerLevel <level> [OPTIONAL]
# Module: cs_autoowner
#
# Set the level at which users get auto-owner on a channel.
# If not specified a default level of 9999 is assumed
#
AutoOwnerLevel 9999

*/

#include "module.h"
#include <version.h>

#define AUTHOR "djGrrr"
#define VERSION "1.0"

int AutoOwner = 0;
int AutoOwnerLevel = 0;

int do_on_join(int argc, char **argv);
int do_on_identify(int argc, char **argv);
int do_on_update(User *u);
int do_set_modes(User *u);
int reload_config(int argc, char **argv);
void load_config(void);

int AnopeInit(int argc, char **argv) {
	Command *c = NULL;
	EvtHook *hook = NULL;

	if (!moduleMinVersion(1,7,10,810)) {
		alog("%s: [cs_accessowner]: Your version of Anope isn't supported.", s_OperServ);
		return MOD_STOP;
	}

	if (!ircd->owner) {
		alog("%s: [cs_accessowner]: Your IRCd does not support channel owner mode, unloading...", s_OperServ);
		return MOD_STOP;
	}

	load_config();

	hook = createEventHook(EVENT_RELOAD, reload_config);
	moduleAddEventHook(hook);
	hook = createEventHook(EVENT_JOIN_CHANNEL, do_on_join);
	moduleAddEventHook(hook);
    hook = createEventHook(EVENT_NICK_IDENTIFY, do_on_identify);
    moduleAddEventHook(hook);

	c = createCommand("UPDATE", do_on_update, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(NICKSERV, c, MOD_TAIL);

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	moduleSetType(THIRD);

	alog("%s: [cs_accessowner]: Module loaded successfully", s_OperServ);

	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("%s: [cs_accessowner]: Module unloaded successfully", s_OperServ);
}

int do_on_join(int argc, char **argv) {
	if (!AutoOwner)
		return MOD_CONT;

	User *u;
	Channel *c = NULL;
	int status;
	char *tmp;

	if (argc != 3)
		return MOD_CONT;

	if (stricmp(argv[0], EVENT_STOP)) {
		return MOD_CONT;
	}

	if (UseTS6 && ircd->ts6) {
		u = find_byuid(argv[1]);
		if (!u)
			u = finduser(argv[1]);
	} else
		u = finduser(argv[1]);
	c = findchan(argv[2]);

	if (!c || !c->ci)
		return MOD_CONT; /* chan not registered */
	if (!u || !nick_identified(u))
		return MOD_CONT;       /* user not found/identified */

	status = chan_get_user_status(c, u);

	if ((get_access_level(c->ci, u->na) >= AutoOwnerLevel) && (!is_founder(u,c->ci))) {
		if (!(status & CUS_OWNER)) {
			tmp = stripModePrefix(ircd->ownerset);
			anope_cmd_mode(whosends(c->ci), c->name, "+%s %s", tmp, u->nick);
			if (tmp) free(tmp);
			chan_set_user_status(c, u, CUS_OWNER);
		}
	}
	return MOD_CONT;
}


int do_on_identify(int argc, char **argv) {
	User *u;

	if (!AutoOwner || argc != 1)
		return MOD_CONT;

	u = finduser(argv[0]);

	if (!u || !nick_identified(u))
		return MOD_CONT;       /* user not found/identified */

	if (NSModeOnID)
		do_set_modes(u);

	return MOD_CONT;
}

int do_on_update(User *u) {
	if (!AutoOwner || !nick_identified(u))
		return MOD_CONT;

	if (NSModeOnID)
		do_set_modes(u);

	return MOD_CONT;
}



int do_set_modes(User *u) {
	struct u_chanlist *uc = NULL;
	char *tmp;
	Channel *c;


	/* Walk users current channels */
	for (uc = u->chans; uc; uc = uc->next) {
		if ((c = findchan(uc->chan->name)) && (c->ci)) {
			if ((get_access_level(c->ci, u->na) >= AutoOwnerLevel) && !is_founder(u, c->ci)) {
				if (!(uc->status & CUS_OWNER)) {
					tmp = stripModePrefix(ircd->ownerset);
					anope_cmd_mode(whosends(c->ci), c->name, "+%s %s", tmp, u->nick);
					if (tmp) free(tmp);
					chan_set_user_status(c, u, CUS_OWNER);
				}
			}
		}
	}
	return MOD_CONT;
}

int reload_config(int argc, char **argv) {
	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			alog("%s: [cs_autoowner]: Reloading configuration directives...", s_OperServ);
			load_config();
		}
	}

	return MOD_CONT;
}

void load_config(void)
{
	int i = 0;

	Directive confvalues[][1] = {
		{{"AutoOwner", {{PARAM_SET, PARAM_RELOAD, &AutoOwner}}}},
		{{"AutoOwnerLevel", {{PARAM_INT, PARAM_RELOAD, &AutoOwnerLevel}}}}
	};

	for (i = 0; i < 2; i++)
		moduleGetConfigDirective(confvalues[i]);

	if (AutoOwner && (!AutoOwnerLevel || AutoOwnerLevel < 0 || AutoOwnerLevel > 9999)) {
		AutoOwnerLevel = 9999;
	}
}
