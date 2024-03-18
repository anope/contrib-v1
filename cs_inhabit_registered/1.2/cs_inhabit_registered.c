/**
 * -----------------------------------------------------------------------------
 * Name    : cs_inhabit_registered
 * Author  : Viper  <Viper@Anope.org>
 * Date    : 01/04/2008  (Last update: 10/01/2010)
 * Version : 1.2
 * -----------------------------------------------------------------------------
 * Tested     : Anope-1.8.5 + UnrealIRCd 3.2.6
 * Requires   : Anope-1.8.5
 * -----------------------------------------------------------------------------
 * This module will make ChanServ join all registered channels and optionally
 * unassign all botserv bots if only one services client is allowed per channel.
 *
 * Unlike the other alternatives that join services clients to channels, this module
 * will honor the BSMinUsers configuration directive. This means that when the botserv
 * bots leave the channel (when no real users are left or the usercount drops below
 * BSMinUsers) ChanServ will also leave.
 * This should eliminate the main disadvantage of modules like ircd_init and cs_join
 * which cause desyncs which in turn can break commands like chanserv invite.
 * In addition it will also honor the NOBOT setting.
 *
 * THIS MODULE CAN NOT BE USED IN COMBINATION WITH OTHER SIMILAR MODULES.
 * More precisely: any module that also joins ChanServ into a channel.
 * Examples are ircd_init and cs_join (there may be more..)
 * This module will work with, but will be unsupported in combination with
 * bs_logchanmon, ircd_gameserv and any other modules that put services clients
 * in channels without honoring BSMinUsers restrictions.
 * If used in combination with serv_inhabit, the chanserv inhabit command of serv_inhabit
 * will be disabled.
 * Also note that it shares the OneClientPerChan configuration directive.
 *
 * The module will also not load if RAW is enabled or loaded.
 * If the module refuses to load because any of above modules is also loaded,
 * cs_inhabit_registered needs to be recompiled in unsupported mode.
 * Undefine or comment out the "SUPPORTED" setting in the configuration section
 * of the source to do this.
 * For windows users: this will require that you compile the module from the source
 * yourself since I will not distribute unsupported versions.
 *
 * Note that I DO NOT RECOMMEND USING THIS MODULE since it is my opinion that
 * the only services clients that ever need to be in a channel are botserv bots,
 * any others are purely out of vanity.
 * -----------------------------------------------------------------------------
 *
 * Changelog:
 *
 *    1.2    Fixed potention segfault on expiring channels.
 *
 *    1.1    Updated to work with Anope-1.8.2
 *           Fixed typo in description of pre-compile configuration block.
 *
 *    1.0    Initial release
 *
 * -----------------------------------------------------------------------------
 **/

/**
 * TODO:
 *
 *    - Find a way to allow ChanServ to stay in empty channel without causing desyncs.
 *          (Cf. Adam's m_permchan.)
 *
 **/

/**
 * Configuration directives that should be copy-pasted to services.conf

# CSInhabitRegistered [OPTIONAL - REQUIRED by cs_inhabit_registered]
# Module: cs_inhabit_registered
#
# This directive is required for cs_inhabit_registered to load correctly.
# If this is not defined, the module will not load.
# Main purpose is to indicate to other modules chanserv may be in channels.
#
#CSInhabitRegistered

# OneClientPerChan [OPTIONAL]
# Module: cs_inhabit_registered and serv_inhabit
#
# If this is defined, only one services client will be allowed to be in a
# channel at the same time. When assigning a new client to inhabit a channel,
# the previous client will be unassigned.
# When this is set and a channel already have more then one services client
# they will all join the channel, however when assigning a new bot to that channel
# all already assigned bots will be unassigned.
# If this is set when both cs_inhabit_registered and serv_inhabit are loaded,
# cs_inhabit_registered takes priority and only ChanServ will remain in all channels.
#
# Note that the unassigning of the BotServ bots cannot be undone. They have to be manually
# re-assigned for each channel.
#
#OneClientPerChan

 *
 **/

#include "module.h"

/*------------------------------Configuration Block----------------------------*/

/**
 * Undefining this will make you lose all support for cs_inhabit_registered!!!
 *
 * This module checks whether unsupported modules are enabled/loaded in anope or
 * whether possibly incompatible 3rd party modules are present.
 * If this is the case, cs_inhabit_registered will unload itself if it s a config directive,
 * or disable all of its functionality if it is a 3rd party module.
 * These measures have been added to protect users against themselves and prevent
 * the author of this module from being confronted with support queries about situations that
 * would not occor during normal use (for example when using RAW or having 2 modules both
 * trying to put ChanServ in a channel).
 *
 * If you undef this directive, these checks will be disabled, however you WILL LOSE ALL SUPPORT!!!
 *
 * To undefine this replace "#define" by "#undef" or simply comment the line out.
 **/
#define SUPPORTED


/**
 * If this is defined, ChanServ will take revenge on whoever kicked it from the channel.
 **/
#define REVENGE
 
/**
 * If this is defined, ChanServ will be given invisible mode (+i: Not shown in /WHO searches)
 * on IRCDs that support it.
 *
 * *** Reserved for Anope-Ad Hoc. Not possible with standard anope. ***
 **/
#define INVISIBLE

/*-------------------------End of Configuration Block--------------------------*/


#define AUTHOR "Viper"
#define VERSION "1.2"

/* Language defines */
#define LANG_NUM_STRINGS 					1

#define LANG_ASSIGN_DISABLED				0

#define UMODE_i_def 		0x00000004
#define UMODE_i_plexus3 	0x00000001

/* Variables */
int Enabled;
int OneClientPerChan;
int supported;


/* Functions */
void cs_join_all(void);
void cs_part_all(void);
void part_all_botserv_clients(void);
int cs_check_join(int ac, char **av);
int cs_check_part(int ac, char **av);
int cs_check_logoff(int ac, char **av);
int cs_check_kick_part(int ac, char **av);
int cs_expire(int ac, char **av);
int cs_check_register(int ac, char **av);

int do_drop(User * u);
int do_forbid(User * u);
int do_suspend(User * u);
int do_assign(User * u);

int cs_check_rejoin(char *source, int ac, char **av);

void cs_do_join(ChannelInfo *ci);

int check_load(void);
void update_version(void);

void load_config(void);
int reload_config(int argc, char **argv);
void add_languages(void);

/* ------------------------------------------------------------------------------- */

/**
 * Create the command, and tell anope about it.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv) {
	Command *c;
	Message *msg;
	EvtHook *hook;

	alog("[\002cs_inhabit_registered\002] Loading module...");

	moduleAddAuthor(AUTHOR);
	/* This one is not needed, but the modules site requires it... */
	moduleAddVersion(VERSION);

	Enabled = 1;

#ifdef SUPPORTED
	if (!moduleMinVersion(1,8,5,3037)) {
		alog("[\002cs_inhabit_registered\002] Your version of Anope isn't supported. Please update to a newer release.");
		return MOD_STOP;
	}
	supported = 1;
#else
	supported = 0;
#endif

	check_load();
	if (!Enabled)
		return MOD_STOP;


	/* Hook to some events.. */
	hook = createEventHook(EVENT_JOIN_CHANNEL, cs_check_join);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002cs_inhabit_registered\002] Can't hook to EVENT_JOIN_CHANNEL event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_PART_CHANNEL, cs_check_part);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002cs_inhabit_registered\002] Can't hook to EVENT_PART_CHANNEL event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_CHAN_EXPIRE, cs_expire);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002cs_inhabit_registered\002] Can't hook to EVENT_CHAN_EXPIRE event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_USER_LOGOFF, cs_check_logoff);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002cs_inhabit_registered\002] Can't hook to EVENT_USER_LOGOFF event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_CHAN_KICK, cs_check_kick_part);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002cs_inhabit_registered\002] Can't hook to EVENT_CHAN_KICK event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_CHAN_REGISTERED, cs_check_register);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002cs_inhabit_registered\002] Can't hook to EVENT_CHAN_REGISTERED event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_RELOAD, reload_config);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002cs_inhabit_registered\002] Can't hook to EVENT_RELOAD event");
		return MOD_STOP;
	}

	/* Because anopes event system is pretty worthless, we have to hook to the commands.. */
	c = createCommand("DROP", do_drop, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(CHANSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002cs_inhabit_registered\002] Cannot hook to DROP command...");
		return MOD_STOP;
	}

	c = createCommand("FORBID", do_forbid, is_services_admin, -1, -1, -1, -1, -1);
	if (moduleAddCommand(CHANSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002cs_inhabit_registered\002] Cannot hook to FORBID command...");
		return MOD_STOP;
	}

	c = createCommand("SUSPEND", do_suspend, is_services_oper, -1, -1, -1, -1, -1);
	if (moduleAddCommand(CHANSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002cs_inhabit_registered\002] Cannot hook to SUSPEND command...");
		return MOD_STOP;
	}

	c = createCommand("ASSIGN", do_assign, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(BOTSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002cs_inhabit_registered\002] Cannot hook to ASSIGN command...");
		return MOD_STOP;
	}

	/* Hook to raw incoming messages... */
	msg = createMessage("KICK", cs_check_rejoin);
	if (moduleAddMessage(msg, MOD_TAIL) != MOD_ERR_OK) {
		alog("[\002cs_inhabit_registered\002] Cannot hook to KICK message...");
		return MOD_STOP;
	}

	load_config();
	add_languages();
	update_version();

	alog("[\002cs_inhabit_registered\002] Module loaded successfully...");

	cs_join_all();

	return MOD_CONT;
}


/**
 * Unload the module
 **/
void AnopeFini(void) {
	if (Enabled)
		cs_part_all();

	alog("[\002cs_inhabit_registered\002] Unloading module...");
}


/* ------------------------------------------------------------------------------- */

void cs_join_all(void) {
	ChannelInfo *ci;
	int i = 0;

	if (!Enabled)
		return;

	for (i = 0; i < 256; i++) {
		for (ci = chanlists[i]; ci; ci = ci->next) {
			if ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED) || (ci->botflags & BS_NOBOT))
				continue;

			/* We don't join if channel doesn't exist or doesn't meet minimum usercount.. */
			if (!(ci ->c) || ci->c->usercount < BSMinUsers)
				continue;

			cs_do_join(ci);
		}
	}

	alog("[cs_inhabit_registered] %s has joined all channels.", s_ChanServ);
}


void cs_part_all(void) {
	ChannelInfo *ci;
	int i = 0;

	for (i = 0; i < 256; i++) {
		for (ci = chanlists[i]; ci; ci = ci->next) {
			if ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED) || (ci->botflags & BS_NOBOT))
				continue;

			anope_cmd_part(s_ChanServ, ci->name, "Parting all channels..");

		}
	}

	alog("[cs_inhabit_registered] %s has parted all channels.", s_ChanServ);
}

void part_all_botserv_clients(void) {
	ChannelInfo *ci;
	int i = 0;

	if (!Enabled)
		return;

	for (i = 0; i < 256; i++) {
		for (ci = chanlists[i]; ci; ci = ci->next) {
			if ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED) || (ci->botflags & BS_NOBOT))
				continue;

			if (ci->bi) {
				if (ci->c && ci->c->usercount >= BSMinUsers)
					anope_cmd_part(ci->bi->nick, ci->name, "UNASSIGN triggered by Network Administration.");
				ci->bi->chancount--;
				ci->bi = NULL;
			}
		}
	}

	alog("[cs_inhabit_registered] All BotServ bots have been unassigned from all channels.");
}

int cs_check_join(int ac, char **av) {
	User *u;
	ChannelInfo *ci;

	if (!Enabled)
		return MOD_CONT;

	if (ac != 3)
		return MOD_CONT;

	/* Join ChanServ after the user has been added to channel... */
	if (stricmp(av[0], EVENT_STOP))
		return MOD_CONT;

	/* Some sanity checks.. */
	if (!(u = finduser(av[1])))
		return MOD_CONT;
	if (!(ci = cs_findchan(av[2])))
		return MOD_CONT;
	if (!(ci->c))
		return MOD_CONT;
	if ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED) || (ci->botflags & BS_NOBOT))
		return MOD_CONT;

	/* Check that usercount is equal to the minimum users to join a bot.
	 * If it s above, ChanServ should already be there, if it s less, we don't join.. */
	if (ci->c->usercount != BSMinUsers)
		return MOD_CONT;

	cs_do_join(ci);

	if (debug)
		alog("[cs_inhabit_registered] debug: %s has joined %s", s_ChanServ, ci->name);

	return MOD_CONT;
}

int cs_check_part(int ac, char **av) {
	User *u;
	ChannelInfo *ci;

	if (!Enabled)
		return MOD_CONT;

	if (ac != 3)
		return MOD_CONT;

	/* Part ChanServ before the user has been deleted from the channel...
	 * So the channel still exists and usercount hasn't been decremented yet. */
	if (stricmp(av[0], EVENT_START))
		return MOD_CONT;

	/* Some sanity checks.. */
	if (!(u = finduser(av[1])))
		return MOD_CONT;
	if (!(ci = cs_findchan(av[2])))
		return MOD_CONT;
	if (!(ci->c))
		return MOD_CONT;
	if ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED) || (ci->botflags & BS_NOBOT))
		return MOD_CONT;

	/* Check that usercount will be equal to the minimum users for a bot to be there.
	 * If it s above, ChanServ can stay there, if it s below, we shouldn't be there.. */
	if ((ci->c->usercount-1) >= BSMinUsers)
		return MOD_CONT;

	anope_cmd_part(s_ChanServ, ci->name, NULL);

	if (debug)
		alog("[cs_inhabit_registered] debug: %s has parted %s (Triggered by Part)", s_ChanServ, ci->name);

	return MOD_CONT;
}

int cs_check_kick_part(int ac, char **av) {
	ChannelInfo *ci;

	if (!Enabled)
		return MOD_CONT;

	if (ac != 2)
		return MOD_CONT;

	/* Some sanity checks.. */
	if (!(ci = cs_findchan(av[1])))
		return MOD_CONT;
	if (!(ci->c))
		return MOD_CONT;
	if ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED) || (ci->botflags & BS_NOBOT))
		return MOD_CONT;

	/* If the channel doesn't meet the minimum requirements after the kick.. */
	if ((ci->c->usercount-1) < BSMinUsers) {
		anope_cmd_part(s_ChanServ, ci->name, NULL);

		if (debug)
			alog("[cs_inhabit_registered] debug: %s has parted %s (Triggered by Kick)", s_ChanServ, ci->name);
	}

	return MOD_CONT;
}

int cs_check_logoff(int ac, char **av) {
	User *u;
	Channel *c;
	struct u_chanlist *uc;

	if (!Enabled)
		return MOD_CONT;

	if (ac != 1)
		return MOD_CONT;

	/* Some sanity checks.. */
	if (!(u = finduser(av[0])))
		return MOD_CONT;

	for (uc = u->chans; uc; uc = uc->next) {
		if ((c = uc->chan)) {
			if (!(c->ci))
				continue;
			if ((c->ci->flags & CI_VERBOTEN) || (c->ci->flags & CI_SUSPENDED) || (c->ci->botflags & BS_NOBOT))
				continue;
			if ((c->usercount-1) >= BSMinUsers)
				continue;

			anope_cmd_part(s_ChanServ, c->name, NULL);

			if (debug)
				alog("[cs_inhabit_registered] debug: %s has parted %s (Triggered by LogOff)", s_ChanServ, c->name);
		}
	}

	return MOD_CONT;
}

int cs_expire(int ac, char **av) {
	ChannelInfo *ci;

	if (!Enabled)
		return MOD_CONT;

	if (ac != 1)
		return MOD_CONT;

	/* Some sanity checks.. */
	if (!(ci = cs_findchan(av[0])))
		return MOD_CONT;
	if (!(ci->c))
		return MOD_CONT;
	if ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED) || (ci->botflags & BS_NOBOT))
		return MOD_CONT;

	/* There are too few users in channel for ChanServ to be there.. */
	if (ci->c->usercount < BSMinUsers)
		return MOD_CONT;

	anope_cmd_part(s_ChanServ, ci->name, "Channel has expired.");

	if (debug)
		alog("[cs_inhabit_registered] debug: %s has parted %s (Channel Expired)", s_ChanServ, ci->name);

	return MOD_CONT;
}

int cs_check_register(int ac, char **av) {
	ChannelInfo *ci;

	if (!Enabled)
		return MOD_CONT;

	if (ac != 1)
		return MOD_CONT;

	/* Some sanity checks.. */
	if (!(ci = cs_findchan(av[0])))
		return MOD_CONT;
	if (!(ci->c))
		return MOD_CONT;
	if ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED) || (ci->botflags & BS_NOBOT))
		return MOD_CONT;

	if (ci->c->usercount < BSMinUsers)
		return MOD_CONT;

	cs_do_join(ci);

	if (debug)
		alog("[cs_inhabit_registered] debug: %s has joined %s (Channel newly registered.)", s_ChanServ, ci->name);

	return MOD_CONT;
}


/* ------------------------------------------------------------------------------- */

int do_drop(User * u) {
	ChannelInfo *ci;
	char *buffer, *chan;

	if (!Enabled)
		return MOD_CONT;

	buffer = moduleGetLastBuffer();
	chan = myStrGetToken(buffer, ' ', 0);

	if (!(readonly && !is_services_admin(u)) && (chan) && (ci = cs_findchan(chan)) &&
			!(ci->flags & CI_VERBOTEN) && !(ci->flags & CI_SUSPENDED) && (is_services_admin(u)
			|| (ci->flags & CI_SECUREFOUNDER ? !is_real_founder(u, ci) : is_founder(u, ci)))) {
		if (!(ci->botflags & BS_NOBOT)) {
			if (ci->c && ci->c->usercount >= BSMinUsers) {
				/* Channel meets requirements so should have ChanServ..
				 * Part it... */
				anope_cmd_part(s_ChanServ, chan, "Channel has been dropped.");

				if (debug)
					alog("[cs_inhabit_registered] debug: %s has parted %s (Channel Dropped)",
							s_ChanServ, chan);
			}
		}
	}

	if (chan) free(chan);
	return MOD_CONT;
}

int do_forbid(User * u) {
	ChannelInfo *ci;
	char *buffer, *chan;

	if (!Enabled)
		return MOD_CONT;

	buffer = moduleGetLastBuffer();
	chan = myStrGetToken(buffer, ' ', 0);

	/* Assumes that permission checking has already been done. */
	if ((chan) && (*chan == '#') && anope_valid_chan(chan)) {
		if ((ci = cs_findchan(chan))) {
			if (!((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED) || (ci->botflags & BS_NOBOT))) {
				/* Channel is registered so may contain ChanServ.. */
				if (ci->c && ci->c->usercount >= BSMinUsers) {
					/* Channel meets minimum requirements so should have ChanServ
					 * Part it... */
					anope_cmd_part(s_ChanServ, chan, "Channel has been forbidden");

					if (debug)
						alog("[cs_inhabit_registered] debug: %s has parted %s (Channel Forbidden)",
								s_ChanServ, chan);
				}
			}
		}
	}

	if (chan) free(chan);
	return MOD_CONT;
}

int do_suspend(User * u) {
	ChannelInfo *ci;
	char *buffer, *chan;

	if (!Enabled)
		return MOD_CONT;

	buffer = moduleGetLastBuffer();
	chan = myStrGetToken(buffer, ' ', 0);

	/* Assumes that permission checking has already been done. */
	if ((chan) && (chan[0] == '#') && (ci = cs_findchan(chan)) && !(ci->flags & CI_VERBOTEN) ) {
		if (!((ci->flags & CI_SUSPENDED) || (ci->botflags & BS_NOBOT))) {
			/* Channel is registered so may contain ChanServ.. */
			if (ci->c && ci->c->usercount >= BSMinUsers) {
				/* Channel meets minimum requirements so should have ChanServ
				 * Part it... */
				anope_cmd_part(s_ChanServ, chan, "Channel has been suspended");

				if (debug)
					alog("[cs_inhabit_registered] debug: %s has parted %s (Channel Suspended)",
							s_ChanServ, chan);
			}
		}
	}

	if (chan) free(chan);
	return MOD_CONT;
}

int do_assign(User * u) {
	if (!Enabled)
		return MOD_CONT;

	if (OneClientPerChan) {
		moduleNoticeLang(s_BotServ, u, LANG_ASSIGN_DISABLED);
		return MOD_STOP;
	}
	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

int cs_check_rejoin(char *source, int ac, char **av) {
	User *u;
	ChannelInfo *ci;

	if (!Enabled)
		return MOD_CONT;

	if (ac != 3)
		return MOD_CONT;

	/* Some sanity checks.. */
	if (!(u = finduser(source)))
		return MOD_CONT;
	if (!(ci = cs_findchan(av[0])))
		return MOD_CONT;
	if ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED) || (ci->botflags & BS_NOBOT))
		return MOD_CONT;

	/* If ChanServ got kicked, continue.. */
	if (stricmp(av[1], s_ChanServ))
		return MOD_CONT;

	alog("[cs_inhabit_registered] %s was kicked from %s by %s .. Rejoining...", s_ChanServ, ci->name, source);

	cs_do_join(ci);

#ifdef REVENGE
	alog("[cs_inhabit_registered] %s taking revenge on %s...", s_ChanServ, source);
	kill_user(s_ChanServ, source, "Is that all you've got? I win!");
#endif

	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */


void cs_do_join(ChannelInfo *ci) {
	if (BSSmartJoin) {
		if (ci->c && ci->c->bans && ci->c->bans->count > 0) {
			char cs_mask[BUFSIZE], buf[BUFSIZE];
			Entry *ban, *next;
			char *av[4];
			int ac;

			snprintf(cs_mask, sizeof(cs_mask), "%s!%s", s_ChanServ, ServiceUser);

			if (ircdcap->tsmode) {
				snprintf(buf, BUFSIZE - 1, "%ld", (long int) time(NULL));
				av[0] = ci->name;
				av[1] = buf;
				av[2] = "-b";
				ac = 4;
			} else {
				av[0] = ci->name;
				av[1] = "-b";
				ac = 3;
			}

			for (ban = ci->c->bans->entries; ban; ban = next) {
				next = ban->next;
				if (entry_match_mask(ban, cs_mask, 0)) {
					anope_cmd_mode(s_ChanServ, ci->name, "-b %s", ban->mask);
					if (ircdcap->tsmode)
						av[3] = ban->mask;
					else
						av[2] = ban->mask;

					do_cmode(s_ChanServ, ac, av);
				}
			}
		}
	}
	anope_cmd_join(s_ChanServ, ci->name, ci->c->creation_time);
	anope_cmd_bot_chan_mode(s_ChanServ, ci->name);
}

/* ------------------------------------------------------------------------------- */

int check_load(void) {
#ifdef SUPPORTED
	if (!DisableRaw || findModule("os_raw")) {
		alog("[\002cs_inhabit_registered\002] RAW has NOT been disabled! (This is fatal!)");
		Enabled = 0;
		return MOD_STOP;
	}

	if (findModule("ircd_init")) {
		alog("[\002cs_inhabit_registered\002] This module cannot be loaded alongside ircd_init. (This is fatal!)");
		Enabled = 0;
		return MOD_STOP;
	}

	if (findModule("cs_join")) {
		alog("[\002cs_inhabit_registered\002] This module cannot be loaded alongside cs_join. (This is fatal!)");
		Enabled = 0;
		return MOD_STOP;
	}

	if (supported) {
		if (findModule("bs_logchanmon")) {
			alog("[\002cs_inhabit_registered\002] This module is unsupported in combination with bs_logchanmon.");
			supported = 0;
		} else if (findModule("ircd_gameserv")) {
			alog("[\002cs_inhabit_registered\002] This module is unsupported in combination with ircd_gameserv.");
			supported = 0;
		}
	} else
		alog("[\002cs_inhabit_registered\002] Module operating in unsupported mode!");

#endif

	return MOD_CONT;
}

void update_version(void) {
	Module *m;
	char tmp[BUFSIZE];

	if (mod_current_module)
		m = mod_current_module;
	else
		m = findModule("cs_inhabit_registered");
	snprintf(tmp, BUFSIZE, "%s [%s]", VERSION, (((Enabled))?(((supported))?"S":"U"):"D"));

	if (m->version)
		free(m->version);
	m->version = sstrdup(tmp);
}

/* ------------------------------------------------------------------------------- */

/**
 * (Re)Load the configuration directives
 **/
void load_config(void) {
	int i,CSInhabitRegistered, OneClientPerChanTmp;

	Directive confvalues[][1] = {
		{{"CSInhabitRegistered", {{PARAM_SET, PARAM_RELOAD, &CSInhabitRegistered}}}},
		{{"OneClientPerChan", {{PARAM_SET, PARAM_RELOAD, &OneClientPerChanTmp}}}},
	};

	CSInhabitRegistered = 0;
	OneClientPerChan = 0;
	OneClientPerChanTmp = 0;

	for (i = 0; i < 2; i++)
		moduleGetConfigDirective(confvalues[i]);

	if (Enabled) {
		if (!CSInhabitRegistered) {
			alog("[\002cs_inhabit_registered\002] Disabling module... (Configuration directive missing.)");
			Enabled = 0;
			update_version();
		}
	}

	if (OneClientPerChanTmp && !OneClientPerChan) {
		OneClientPerChan = OneClientPerChanTmp;
		part_all_botserv_clients();
	}


	if (debug)
		alog ("[cs_inhabit_registered] debug: CSInhabitRegistered set to %d; OneClientPerChan set to %d",
				CSInhabitRegistered, OneClientPerChan);
}


/**
 * Upon /os reload call the routines for reloading the configuration directives
 **/
int reload_config(int argc, char **argv) {
	if (!Enabled)
		return MOD_CONT;

	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			alog("[cs_inhabit_registered]: Reloading configuration directives...");
			load_config();
		}
	}

	if (check_load() != MOD_CONT) {
		update_version();
		cs_part_all();
	}

	return MOD_CONT;
}


/* ------------------------------------------------------------------------------- */

/**
 * Add language strings to the module's language db.
 **/
void add_languages(void) {
	char *langtable_en_us[] = {
		/* LANG_ASSIGN_DISABLED */
		" This command is currently disabled.",
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}

/* EOF */
