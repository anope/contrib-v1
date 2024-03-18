/**
 * -----------------------------------------------------------------------------
 * Name    : bs_fantasy_vhost
 * Author  : Viper  <Viper@Anope.org>
 * Date    : 13/01/2012  (Last update: 18/01/2012)
 * Version : 1.0
 * -----------------------------------------------------------------------------
 * Requires   : Anope-1.8.7 + IRCd support vhosts + BotServ enabled
 * Tested     : Anope-1.8.7 + UnrealIRCd 3.2.8.1
 * Tested     : Anope-1.8.7 + InspIRCd 1.2
 * -----------------------------------------------------------------------------
 * This module adds 2 fantasy commands to botserv to allow users to set new vhosts
 * on themselves using !vhost or !groupvhost.
 *
 * This module is meant to replace n00bie's old ircd_vhostserv which can cause
 * desyncs and only works with UnrealIRCd. This rewrite is a slimmed down version
 * which utilises the botserv fantasy system instead of a dedicated client.
 * Some extra option were added to allow admins to limit who has access.
 *
 * Note that I am not personally a fan of this type of functionality as it allows
 * users to evade bans placed on them in channels!
 *
 * This module is released under the GPL 2 license.
 * -----------------------------------------------------------------------------
 * Changelog:
 *
 *    1.0    Initial release.
 *
 */

/**
 * TODO:
 *
 *    << hope there are no bugs left >>
 *
 **/

/**
 * Configuration directives that should be copy-pasted to services.conf

# vHostChannels [REQUIRED]
# Module: bs_fantasy_vhost
#
# Define in which channels the !vhost and !groupvhost commands will be enabled.
# Multiple channels can be specified separated by a space. Wildcards are accepted.
# (E.g.: "#*" will allow !vhost to be used in all channels.)
# Note that channels must be registered and assigned a BotServ bot.
#
vHostChannels "#help #vHost"

# vHostSetInterval [OPTIONAL - Recommended]
# Module: bs_fantasy_vhost
#
# Sets the period after setting a vhost during which a user cannot set a new vhost.
# When 0 or not set, users will be able to set their vhost without restrictions.
#
vHostSetInterval 24h

# vHostKickBanAfterSet [OPTIONAL]
# Module: bs_fantasy_vhost
#
# When defined, a user will be banned and kicked from the channel after
# successfully setting a vhost. Users will remain banned from all channels listed
# in vHostChannels until vHostSetInterval has expired or services are restarted.
# Users with access to the channel will not be kicked.
#
#vHostKickBanAfterSet

# vHostRestricted [OPTIONAL - Recommended]
# Module: bs_fantasy_vhost
#
# Define strings or wildcards that are not accepted in vIndents or vHosts.
# Multiple strings can be specified separated by a space.
#
vHostRestricted "*ircop* *admin* *root* *staff*"

# vHostRestrictedKickBan [OPTIONAL]
# Module: bs_fantasy_vhost
#
# When defined users who request a vhost matching a restricted string will
# be banned Dand kicked from all channels listed in vHostChannels for the
# duration specified in vHostSetInterval.
#
vHostRestrictedKickBan

# vHostSetRegisteredMin [OPTIONAL - Recommended]
# Module: bs_fantasy_vhost
#
# Sets the period after setting a vhost during which a user cannot set a new vhost.
# When 0 or not set, there is no minimum to how long users must be registered before
# being allowed to set their own vhost.
#
vHostSetRegisteredMin 28d

 *
 **/

#include "module.h"

/*------------------------------Configuration Block----------------------------*/

/**
 * When defined the module operates in supported mode.
 * This means it will stop functioning if incompatible modules are
 * detected.
 * When commenting this or undefining, these checks will be disabled,
 * however no further support will be provided by the author in case
 * problems arise.
 **/
#define SUPPORTED

/*-------------------------End of Configuration Block--------------------------*/

#define AUTHOR "Viper"
#define VERSION "1.0"


/* Language defines */
#define LANG_NUM_STRINGS 					16

#define LANG_CMD_DISABLED					0
#define LANG_VHOST_SYNTAX					1
#define LANG_GROUPVHOST_SYNTAX				2
#define LANG_VIDENT_VHOST_SYNTAX			3
#define LANG_VIDENT_GROUPVHOST_SYNTAX		4
#define LANG_VHOST_SYNTAX_EXT				5
#define LANG_GROUPVHOST_SYNTAX_EXT			6
#define LANG_VIDENT_VHOST_SYNTAX_EXT		7
#define LANG_VIDENT_GROUPVHOST_SYNTAX_EXT	8
#define LANG_VHOST_SET_INTERVAL_NOT_EXPIRED	9
#define LANG_VHOST_NOT_ALLOWED				10
#define LANG_NICK_VHOST_SET					11
#define LANG_NICK_VIDENT_VHOST_SET			12
#define LANG_GROUP_VHOST_SET				13
#define LANG_GROUP_VIDENT_VHOST_SET			14
#define LANG_MINIUM_TIME_REGISTERED			15


/* Constants */
char *ModDataKey = "vhostsetexpiry";


/* Variables */
int supported;
int vHostSetInterval, vHostKickBanAfterSet, vHostRestrictedKickBan, vHostSetRegisteredMin;
int vHostChannelsNr, vHostRestrictedNr;
char **vHostChannels, **vHostRestricted;


/* Functions */
int do_fantasy_vhost(int ac, char **av);
int is_vhost_channel(char *channel);
void do_set_vhost(ChannelInfo *ci, User *u, char *rawhostmask, int setgroup);
int est_core_registered(NickCore *nc);
int is_vhost_restricted(char *vhost);
static void show_modinfo(User *u, ChannelInfo *ci);

void addTempBan(Channel *c, time_t expires, char *banmask);
int delTempBan(int argc, char **argv);
char *maketime(int seconds);

int check_modules(void);
int event_check_module(int argc, char **argv);
int event_check_cmd(int argc, char **argv);

char* get_flags();
void update_version(void);

void load_config(void);
int reload_config(int argc, char **argv);

void add_languages(void);


/* Externals */
extern int do_hs_sync(NickCore * nc, char *vIdent, char *hostmask, char *creator, time_t time);


/* ------------------------------------------------------------------------------- */

/**
 * Create the command, and tell anope about it.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv) {
	EvtHook *hook = NULL;

	alog("[\002bs_fantasy_vhost\002] Loading module...");

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	supported = 1;

	if (!moduleMinVersion(1,8,7,3089)) {
		alog("[\002bs_fantasy_vhost\002] Your version of Anope isn't supported. Please update to a newer release.");
		return MOD_STOP;
	}
	if (!ircd->vhost) {
		alog("[\002bs_fantasy_vhost\002] ERROR: Your IRCd does not support vhosts!");
		return MOD_STOP;
	}
	if (!s_BotServ) {
		alog("[\002bs_fantasy_vhost\002] ERROR: BotServ is not enabled!");
		return MOD_STOP;
	}

	check_modules();
	if (supported == 0) {
		alog("[\002bs_fantasy_vhost\002] Warning: Module continuing in unsupported mode!");
	} else if (supported == -1) {
		alog("[\002bs_fantasy_vhost\002] Unloading module due to incompatibilities!");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_BOT_FANTASY, do_fantasy_vhost);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_vhost\002] Can't hook to EVENT_BOT_FANTASY event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_BOT_FANTASY_NO_ACCESS, do_fantasy_vhost);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_vhost\002] Can't hook to EVENT_BOT_FANTASY_NO_ACCESS event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_RELOAD, reload_config);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_vhost\002] Can't hook to EVENT_RELOAD event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_MODLOAD, event_check_module);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_vhost\002] Can't hook to EVENT_MODLOAD event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_ADDCOMMAND, event_check_cmd);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_vhost\002] Can't hook to EVENT_ADDCOMMAND event.");
		return MOD_STOP;
	}

	load_config();
	add_languages();

	/* Update version info.. */
	update_version();

	alog("[\002bs_fantasy_vhost\002] Module loaded successfully...");

	return MOD_CONT;
}

/**
 * Unload the module
 **/
void AnopeFini(void) {
	int i;

	for (i = 0; i < vHostChannelsNr; i++)
		if (vHostChannels[i])
			free(vHostChannels[i]);
	if (vHostChannels)
		free(vHostChannels);

	for (i = 0; i < vHostRestrictedNr; i++)
		if (vHostRestricted[i])
			free(vHostRestricted[i]);
	if (vHostRestricted)
		free(vHostRestricted);

	alog("[\002bs_fantasy_vhost\002] Unloading module...");
}

/* ------------------------------------------------------------------------------- */

/**
 * Parses the fantasy commands looking for !vhost and !groupvhost.
 **/
int do_fantasy_vhost(int ac, char **av) {
	User *u;
	ChannelInfo *ci;
	Channel *c;
	char *botnick = NULL;
	int ret = MOD_CONT;

	/* Some basic error checking... should never match */
	if (ac < 3)
		return MOD_CONT;

	if (supported < 0)
		return MOD_CONT;
	if (!(ci = cs_findchan(av[2])))
		return MOD_CONT;
	if (!(u = finduser(av[1])))
		return MOD_CONT;
	if (!(c = findchan(ci->name)))
		return MOD_CONT;

	/* Check if we are supposed to be listening in the channel.. */
	if (!is_vhost_channel(ci->name))
		return MOD_CONT;

	if (ci->bi)
		botnick = ci->bi->nick;
	else
		botnick = s_ChanServ;

	if (!stricmp(av[0], "vhost")) {
		if (ac < 3)
			moduleNoticeLang(botnick, u, ircd->vident ? LANG_VIDENT_VHOST_SYNTAX : LANG_VHOST_SYNTAX, BSFantasyCharacter);
		else {
			char *vhost;
			vhost = myStrGetToken(av[3], ' ', 0);
			do_set_vhost(ci, u, vhost, 0);
			free(vhost);
		}
	} else if (!stricmp(av[0], "groupvhost")) {
		if (ac < 3)
			moduleNoticeLang(botnick, u, ircd->vident ? LANG_VIDENT_GROUPVHOST_SYNTAX : LANG_GROUPVHOST_SYNTAX, BSFantasyCharacter);
		else {
			char *vhost;
			vhost = myStrGetToken(av[3], ' ', 0);
			do_set_vhost(ci, u, vhost, 1);
			free(vhost);
		}
	} else if (!stricmp(av[0], "minfo")) {
		show_modinfo(u, ci);
	} else if (!stricmp(av[0], "help")) {
		if (ac > 3) {
			char *cmd;
			cmd = myStrGetToken(av[3], ' ', 0);
			if (!stricmp(cmd, "vhost")) {
				moduleNoticeLang(botnick, u, ircd->vident ? LANG_VIDENT_VHOST_SYNTAX_EXT : LANG_VHOST_SYNTAX_EXT, BSFantasyCharacter);
				ret = MOD_STOP;
			} else if (!stricmp(cmd, "groupvhost")) {
				moduleNoticeLang(botnick, u, ircd->vident ? LANG_VIDENT_GROUPVHOST_SYNTAX_EXT : LANG_GROUPVHOST_SYNTAX_EXT, BSFantasyCharacter);
				ret = MOD_STOP;
			}
			free(cmd);
		}
	}
	return ret;
}

int is_vhost_channel(char *channel) {
	int idx;

	for (idx = 0; idx < vHostChannelsNr; idx++) {
		if (match_wild_nocase(vHostChannels[idx], channel)) {
			return 1;
		}
	}
	return 0;
}

/**
 * Validate the requested vhost and apply.
 * @param ci Channel in which the fantasy command was issued.
 * @param u User who issued the command.
 * @param rawhostmask Requested vHost - may include vIdent.
 * @param setgroup Boolean indicating whether to set for all nicks in the group.
 **/
void do_set_vhost(ChannelInfo *ci, User *u, char *rawhostmask, int setgroup) {
	char *botnick = NULL, *vIdent = NULL, *hostmask = NULL, *tmp = NULL, *s = NULL;
	int32 tmp_time = time(NULL);

	if (!ci || !ci->c || !u || !rawhostmask)
		return;
	if (ci->bi)
		botnick = ci->bi->nick;
	else
		botnick = s_ChanServ;

	if (readonly) {
		moduleNoticeLang(botnick, u, LANG_CMD_DISABLED);
		return;
	} else if (!u->na) {
		notice_lang(botnick, u, NICK_NOT_REGISTERED);
		return;
	} else if (!nick_identified(u)) {
		notice_lang(botnick, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
		return;
	}

	/* Ensure the user has been registered long enough to allow the use of !vhost. */
	if (vHostSetRegisteredMin && !is_services_oper(u)) {
		/* We have to go over all nicks in the group because the time registered isn't
		 * stored in the nickcore.. HMPF!!! */
		if (est_core_registered(u->na->nc) > tmp_time - vHostSetRegisteredMin) {
			char *min_reg;
			min_reg = maketime(vHostSetRegisteredMin);
			moduleNoticeLang(botnick, u, LANG_MINIUM_TIME_REGISTERED, min_reg);
			free(min_reg);
			return;
		}
	}

	/* Check whether the user is allowed to set another vhost at this time.. */
	if (!is_services_oper(u) && (tmp = moduleGetData(&u->na->nc->moduleData, ModDataKey))) {
		if (atoi(tmp) > tmp_time) {
			char *left;
			left = maketime(atoi(tmp) - tmp_time);
			/* User is not allowed to set another vhost yet.. */
			moduleNoticeLang(botnick, u, LANG_VHOST_SET_INTERVAL_NOT_EXPIRED, left);
			if (vHostKickBanAfterSet && !get_access(u, ci)) {
				char mask[BUFSIZE], buf[BUFSIZE];
				char *av[3];

				get_idealban(ci, u, mask, sizeof(mask));
				snprintf(buf, BUFSIZE, "You must wait before setting another vhost. (Time left: %s)", left);
				av[0] = ci->name;
				av[1] = GET_USER(u);
				av[2] = buf;

				addTempBan(ci->c, atoi(tmp), mask);
				anope_cmd_kick(botnick, ci->name, u->nick, buf);
				do_kick(botnick, 3, av);
			}
			free(left);
			return;
		} else
			moduleDelData(&u->na->nc->moduleData, ModDataKey);
	}

	vIdent = myStrGetOnlyToken(rawhostmask, '@', 0);    /* Get the first substring, @ as delimiter */
	if (vIdent) {
		rawhostmask = myStrGetTokenRemainder(rawhostmask, '@', 1);      /* get the remaining string */
		if (!rawhostmask) {
			if (!setgroup)
				moduleNoticeLang(botnick, u, ircd->vident ? LANG_VIDENT_VHOST_SYNTAX : LANG_VHOST_SYNTAX, BSFantasyCharacter);
			else
				moduleNoticeLang(botnick, u, ircd->vident ? LANG_VIDENT_GROUPVHOST_SYNTAX : LANG_GROUPVHOST_SYNTAX, BSFantasyCharacter);
			free(vIdent);
			return;
		}
		if (strlen(vIdent) > USERMAX - 1) {
			notice_lang(botnick, u, HOST_SET_IDENTTOOLONG, USERMAX);
			free(vIdent);
			free(rawhostmask);
			return;
		} else {
			for (s = vIdent; *s; s++) {
				if (!isvalidchar(*s)) {
					notice_lang(botnick, u, HOST_SET_IDENT_ERROR);
					free(vIdent);
					free(rawhostmask);
					return;
				}
			}
		}
		if (!ircd->vident) {
			notice_lang(botnick, u, HOST_NO_VIDENT);
			free(vIdent);
			free(rawhostmask);
			return;
		}
	}

	hostmask = smalloc(HOSTMAX);
	if (strlen(rawhostmask) < HOSTMAX)
		snprintf(hostmask, HOSTMAX, "%s", rawhostmask);
	else {
		notice_lang(botnick, u, HOST_SET_TOOLONG, HOSTMAX);
		if (vIdent) {
			free(vIdent);
			free(rawhostmask);
		}
		free(hostmask);
		return;
	}

	if (!isValidHost(hostmask, 3)) {
		notice_lang(botnick, u, HOST_SET_ERROR);
		if (vIdent) {
			free(vIdent);
			free(rawhostmask);
		}
		free(hostmask);
		return;
	}

	/* First check if it is restricted.. */
	if (!is_services_oper(u) && ((vIdent && is_vhost_restricted(vIdent))
			|| is_vhost_restricted(hostmask))) {
		if (vHostRestrictedKickBan && !get_access(u, ci)) {
			char mask[BUFSIZE], buf[BUFSIZE];
			char *av[3], *banned;

			snprintf(buf, BUFSIZE, "%d", tmp_time + vHostSetInterval);
			moduleNoticeLang(botnick, u, LANG_VHOST_NOT_ALLOWED);
			moduleAddData(&u->na->nc->moduleData, ModDataKey, buf);

			get_idealban(ci, u, mask, sizeof(mask));
			banned = maketime(vHostSetInterval);
			snprintf(buf, BUFSIZE, "The vhost you requested is not allowed. Banned for %s.", banned);
			av[0] = ci->name;
			av[1] = GET_USER(u);
			av[2] = buf;

			addTempBan(ci->c, tmp_time + vHostSetInterval, mask);
			anope_cmd_kick(botnick, ci->name, u->nick, buf);
			do_kick(botnick, 3, av);
			free(banned);
		} else
			moduleNoticeLang(botnick, u, LANG_VHOST_NOT_ALLOWED);

		if (vIdent) {
			free(vIdent);
			free(rawhostmask);
		}
		free(hostmask);
		return;
	}

	/* We ve passed basic validation so set the timeout.. */
	if (vHostSetInterval) {
		char buf[BUFSIZE];
		snprintf(buf, BUFSIZE, "%d", tmp_time + vHostSetInterval);
		moduleAddData(&u->na->nc->moduleData, ModDataKey, buf);
	}

	if (setgroup)
		do_hs_sync(u->na->nc, vIdent, hostmask, u->nick, tmp_time);
	else
		addHostCore(u->nick, vIdent, hostmask, u->nick, tmp_time);
	anope_cmd_vhost_on(u->nick, vIdent, hostmask);
	set_lastmask(u);

	if (setgroup) {
		if (vIdent && ircd->vident) {
			moduleNoticeLang(botnick, u, LANG_GROUP_VIDENT_VHOST_SET, u->nick, vIdent, hostmask);
			alog("[bs_fantasy_vhost] vHost for all nicks in group \002%s\002 set to \002%s@%s\002.", u->nick, vIdent, hostmask);
		} else {
			moduleNoticeLang(botnick, u, LANG_GROUP_VHOST_SET, u->nick, hostmask);
			alog("[bs_fantasy_vhost] vHost for all nicks in group \002%s\002 set to \002%s\002.", u->nick, hostmask);
		}
	} else {
		if (vIdent && ircd->vident) {
			moduleNoticeLang(botnick, u, LANG_NICK_VIDENT_VHOST_SET, u->nick, vIdent, hostmask);
			alog("[bs_fantasy_vhost] vHost for user \002%s\002 set to \002%s@%s\002.", u->nick, vIdent, hostmask);
		} else {
			moduleNoticeLang(botnick, u, LANG_NICK_VHOST_SET, u->nick, hostmask);
			alog("[bs_fantasy_vhost] vHost for user \002%s\002 set to \002%s\002.", u->nick, hostmask);
		}
	}

	if (vHostKickBanAfterSet && !get_access(u, ci)) {
		char mask[BUFSIZE], buf[BUFSIZE];
		char *av[3], *banned;

		get_idealban(ci, u, mask, sizeof(mask));
		banned = maketime(vHostSetInterval);
		snprintf(buf, BUFSIZE, "Banned until you are allowed to set your next vhost. (Time left: %s)", banned);
		av[0] = ci->name;
		av[1] = GET_USER(u);
		av[2] = buf;

		addTempBan(ci->c, tmp_time + vHostSetInterval, mask);
		anope_cmd_kick(botnick, ci->name, u->nick, buf);
		do_kick(botnick, 3, av);
		free(banned);
	}
}

/**
 * Guess when the nickcore was registered by getting the registered time of the oldest nick..
 **/
int est_core_registered(NickCore *nc) {
	int i;
	NickAlias *na;
	time_t time_created = 0;

	for (i = 0; i < 1024; i++) {
		for (na = nalists[i]; na; na = na->next) {
			if (na->nc && na->nc == nc && (!time_created || na->time_registered < time_created))
				time_created = na->time_registered;
		}
	}

	return time_created;
}

/**
 * Check whether the given vhost matches a wildcarded string in the restriction list.
 **/
int is_vhost_restricted(char *vhost) {
	int idx;

	for (idx = 0; idx < vHostRestrictedNr; idx++) {
		if (match_wild_nocase(vHostRestricted[idx], vhost)) {
			return 1;
		}
	}
	return 0;
}

/* ------------------------------------------------------------------------------- */

/**
 * Sends the user the information on this module.
 **/
static void show_modinfo(User *u, ChannelInfo *ci) {
	char *botnick = NULL;
	char *flags = get_flags();

	if (ci->bi)
		botnick = ci->bi->nick;
	else
		botnick = s_ChanServ;

	notice(botnick, u->nick, "vHost triggers provided by \002bs_fantasy_vhost\002. [Author: \002%s\002] [Version: \002%s\002] [Flags: \002%s\002]",
			AUTHOR, VERSION, flags);

	free(flags);
}

/* ------------------------------------------------------------------------------- */

/**
 * Adds a ban in a channel for the given mask and adds a callback for it
 * to be removed after the given period of time.
 **/
void addTempBan(Channel *c, time_t expires, char *banmask) {
	char *av[3];
	char *cb[2];

	cb[0] = c->name;
	cb[1] = banmask;

	av[0] = sstrdup("+b");
	av[1] = banmask;

	anope_cmd_mode(c->ci->bi->nick, c->name, "+b %s", av[1]);
	chan_set_modes(c->ci->bi->nick, c, 2, av, 1);

	free(av[0]);
	moduleAddCallback("vhostban", expires, delTempBan, 2, cb);
}


/**
 * Callback function to remove a given ban from a channel.
 **/
int delTempBan(int argc, char **argv) {
	char *av[3];
	Channel *c;

	av[0] = sstrdup("-b");
	av[1] = argv[1];

	if ((c = findchan(argv[0])) && c->ci) {
		anope_cmd_mode(whosends(c->ci), c->name, "-b %s", av[1]);
		chan_set_modes(whosends(c->ci), c, 2, av, 1);
	}

	free(av[0]);
	return MOD_CONT;
}

/**
 * Turn a given number of seconds into a more readable time format.
 **/
char *maketime(int seconds) {
	int d, h, m;
	char buf[BUFSIZE], *end, *t;

	*buf = 0;
	end = buf;

	if (seconds >= 86400) {
		d = seconds / 86400;
		end += snprintf(end, sizeof(buf) - (end - buf), "%dd", d);
		seconds -= d * 86400;
	}

	if (seconds >= 3600) {
		h = seconds / 3600;
		end += snprintf(end, sizeof(buf) - (end - buf), "%dh", h);
		seconds -= h * 3600;
	}

	if (seconds >= 60) {
		m = seconds / 60;
		end += snprintf(end, sizeof(buf) - (end - buf), "%dm", m);
		seconds -= m * 60;
	}

	if (seconds)
		end += snprintf(end, sizeof(buf) - (end - buf), "%ds", seconds);

	t = sstrdup(buf);
	return t;
}

/* ------------------------------------------------------------------------------- */

/**
 * Checks whether the right conditions to continue loading are met.
 **/
int check_modules(void) {
#ifdef SUPPORTED
	if (supported >= 0) {
		if (findModule("os_raw")) {
			alog("[\002bs_fantasy_vhost\002] Unsupported module found: os_raw.. (This is fatal!)");
			supported = -1;
		}
		if (findCommand(OPERSERV, "RAW")) {
			alog("[\002bs_fantasy_vhost\002] Unsupported module found: os_raw.. (This is fatal!)");
			supported = -1;
		}
		if (!DisableRaw) {
			alog("[\002bs_fantasy_vhost\002] RAW has NOT been disabled! (This is fatal!)");
			supported = -1;
		}
		if (findModule("ircd_vhostserv")) {
			alog("[\002bs_fantasy_vhost\002] Unsupported module found: ircd_vhostserv.. (This is fatal!)");
			supported = -1;
		}
	}

	if (supported >= 0) {
		if (findModule("ircd_init")) {
			alog("[\002bs_fantasy_vhost\002] This module is unsupported in combination with ircd_init.");
			supported = 0;
		}

		if (findModule("cs_join")) {
			alog("[\002bs_fantasy_vhost\002] This module is unsupported in combination with cs_join.");
			supported = 0;
		}

		if (findModule("bs_logchanmon")) {
			alog("[\002bs_fantasy_vhost\002] This module is unsupported in combination with bs_logchanmon.");
			supported = 0;
		}

		if (findModule("ircd_gameserv")) {
			alog("[\002bs_fantasy_vhost\002] This module is unsupported in combination with ircd_gameserv.");
			supported = 0;
		}

		if (findModule("os_psuedo_cont")) {
			alog("[\002bs_fantasy_vhost\002] This module is unsupported in combination with os_psuedo_cont.");
			supported = 0;
		}
	}
#endif
	return supported;
}

/**
 * When a module is loaded, check whether it s compatible.
 **/
int event_check_module(int argc, char **argv) {
	int old_supported = supported;
#ifdef SUPPORTED
	if (argc != 1 || !argv[0])
		return MOD_CONT;

	if (supported >= 0) {
		if (!stricmp(argv[0], "os_raw") || !stricmp(argv[0], "ircd_vhostserv")) {
			alog("[\002bs_fantasy_vhost\002] Unsupported module found: %s.. (This is fatal!)", argv[0]);
			supported = -1;
		}
		if (!stricmp(argv[0], "ircd_init") || !stricmp(argv[0], "cs_join") || !stricmp(argv[0], "bs_logchanmon")
				|| !stricmp(argv[0], "ircd_gameserv") || !stricmp(argv[0], "os_psuedo_cont")) {
			alog("[\002bs_fantasy_vhost\002] This module is unsupported in combination with %s.", argv[0]);
			supported = 0;
		}

		if (supported != old_supported) {
			if (supported == 0) {
				alog("[\002bs_fantasy_vhost\002] Warning: Module continuing in unsupported mode!");
			} else if (supported == -1) {
				alog("[\002bs_fantasy_vhost\002] Disabling module due to incompatibilities!");
				return MOD_STOP;
			}
			update_version();
		}
	}
#endif
	return MOD_CONT;
}

/**
 * When a command is created, check whether it s compatible.
 **/
int event_check_cmd(int argc, char **argv) {
	int old_supported = supported;
#ifdef SUPPORTED
	if (argc != 1 || !argv[0])
		return MOD_CONT;

	if (supported >= 0) {
		if (!stricmp(argv[0], "RAW")) {
			alog("[\002bs_fantasy_vhost\002] Unsupported command found: %s.. (This is fatal!)", argv[0]);
			supported = -1;
		}

		if (supported != old_supported) {
			if (supported == 0) {
				alog("[\002bs_fantasy_vhost\002] Warning: Module continuing in unsupported mode!");
			} else if (supported == -1) {
				alog("[\002bs_fantasy_vhost\002] Disabling module due to incompatibilities!");
				return MOD_STOP;
			}
			update_version();
		}
	}
#endif
	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

/**
 * Returns pointer to a string of flags.. Must be free'd!!
 **/
char* get_flags() {
	char tmp[BUFSIZE];
	const char version_flags[] = " " VER_DEBUG VER_OS VER_MYSQL VER_MODULE;
	char *flags;

#ifdef SUPPORTED
	snprintf(tmp, BUFSIZE, "%s-%s-I%d-KBs%d-KBr%d-RM%d", version_flags, ((supported == -1) ? "U" :
			(supported == 0) ? "u" : "S"), vHostSetInterval, vHostKickBanAfterSet,
			vHostRestrictedKickBan, vHostSetRegisteredMin);
#else
	snprintf(tmp, BUFSIZE, "%s-%s-I%d-KBs%d-KBr%d-RM%d", version_flags, vHostSetInterval,
			vHostKickBanAfterSet, vHostRestrictedKickBan, vHostSetRegisteredMin);
#endif
	flags = sstrdup(tmp);
	return flags;
}

/**
 * Updates the version info shown in modlist and modinfo.
 **/
void update_version(void) {
	Module *m;
	char tmp[BUFSIZE];
	char *flags = get_flags();

	if (mod_current_module)
		m = mod_current_module;
	else
		m = findModule("bs_fantasy_vhost");

	snprintf(tmp, BUFSIZE, "%s %s [%s]", AUTHOR, VERSION, flags);

	if (m->version)
		free(m->version);
	m->version = sstrdup(tmp);
	free(flags);
}

/* ------------------------------------------------------------------------------- */

/**
 * (Re)Load the configuration directives
 **/
void load_config(void) {
	int i;
	char *chans = NULL, *restricted = NULL;

	Directive confvalues[][1] = {
		{{"vHostChannels", {{PARAM_STRING, PARAM_RELOAD, &chans}}}},
		{{"vHostSetInterval", {{PARAM_TIME, PARAM_RELOAD, &vHostSetInterval}}}},
		{{"vHostKickBanAfterSet", {{PARAM_SET, PARAM_RELOAD, &vHostKickBanAfterSet}}}},
		{{"vHostRestricted", {{PARAM_STRING, PARAM_RELOAD, &restricted}}}},
		{{"vHostRestrictedKickBan", {{PARAM_SET, PARAM_RELOAD, &vHostRestrictedKickBan}}}},
		{{"vHostSetRegisteredMin", {{PARAM_TIME, PARAM_RELOAD, &vHostSetRegisteredMin}}}},
	};

	vHostSetInterval = 0;
	vHostKickBanAfterSet = 0;
	vHostRestrictedKickBan = 0;
	vHostSetRegisteredMin = 0;

	for (i = 0; i < vHostChannelsNr; i++)
		if (vHostChannels[i])
			free(vHostChannels[i]);
	if (vHostChannels)
		free(vHostChannels);
	vHostChannelsNr = 0;
	vHostChannels = NULL;

	for (i = 0; i < vHostRestrictedNr; i++)
		if (vHostRestricted[i])
			free(vHostRestricted[i]);
	if (vHostRestricted)
		free(vHostRestricted);
	vHostRestrictedNr = 0;
	vHostRestricted = NULL;

	for (i = 0; i < 6; i++)
		moduleGetConfigDirective(confvalues[i]);

	if (chans) {
		vHostChannels = buildStringList(chans, &vHostChannelsNr);

		for (i = 0; i < vHostChannelsNr; i++) {
			if (!vHostChannels[i])
				continue;

			if (*vHostChannels[i] != '#') {
				alog("[bs_fantasy_vhost] Invalid syntax. %s could not be added to the vHostChannels list.", vHostChannels[i]);
				free(vHostChannels[i]);
				vHostChannels[i] = NULL;
			} else if (debug)
				alog("[bs_fantasy_vhost] debug: Added '%s' to the vHostChannels list.", vHostChannels[i]);
		}
		free(chans);
	}

	if (vHostKickBanAfterSet && !vHostSetInterval) {
		vHostKickBanAfterSet = 0;
		alog("[bs_fantasy_vhost] vHostKickBanAfterSet cannot be set if vHostSetInterval is not set! Disabling vHostKickBanAfterSet..");
	}

	if (restricted) {
		vHostRestricted = buildStringList(restricted, &vHostRestrictedNr);

		if (debug) {
			for (i = 0; i < vHostRestrictedNr; i++) {
				if (!vHostRestricted[i])
					continue;
				alog("[bs_fantasy_vhost] debug: Added '%s' to the vHostRestricted list.", vHostRestricted[i]);
			}
		}
		free(restricted);
	}

	if (vHostRestrictedKickBan && !vHostSetInterval) {
		vHostRestrictedKickBan = 0;
		alog("[bs_fantasy_vhost] vHostRestrictedKickBan cannot be set if vHostSetInterval is not set! Disabling vHostRestrictedKickBan..");
	}

	if (debug)
		alog ("[bs_fantasy_vhost] debug: vHostSetInterval=%d. vHostKickBanAfterSet=%d. vHostRestrictedKickBan=%d. vHostSetRegisteredMin=%d.",
				vHostSetInterval, vHostKickBanAfterSet, vHostRestrictedKickBan, vHostSetRegisteredMin);
}


/**
 * Upon /os reload call the routines for reloading the configuration directives
 **/
int reload_config(int argc, char **argv) {
	int old_supported = supported;

	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			if (debug)
				alog("[bs_fantasy_vhost] debug: Reloading configuration directives...");
			load_config();
		}
	}

	check_modules();
	if (supported != old_supported) {
		if (supported == 0) {
			alog("[\002bs_fantasy_vhost\002] Warning: Module continueing in unsupported mode!");
		} else if (supported == -1) {
			alog("[\002bs_fantasy_vhost\002] Disabling module due to incompatibilities!");
			return MOD_STOP;
		}
	}
	update_version();

	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

/**
 * Add language strings to the module's language db.
 **/
void add_languages(void) {
	char *langtable_en_us[] = {
		/* LANG_CMD_DISABLED */
		"This command has been disabled. Pls Contact Network Staff for more info.",
		/* LANG_VHOST_SYNTAX */
		"Syntax: %svhost your.desired.vHost",
		/* LANG_GROUPVHOST_SYNTAX */
		"Syntax: %sgroupvhost your.desired.vHost",
		/* LANG_VIDENT_VHOST_SYNTAX */
		"Syntax: %svhost [vIdent@]your.desired.vHost",
		/* LANG_VIDENT_GROUPVHOST_SYNTAX */
		"Syntax: %sgroupvhost [vIdent@]your.desired.vHost",
		/* LANG_VHOST_SYNTAX_EXT */
		"Syntax: %svhost your.desired.vHost\n"
		"\n"
		"Sets the vHost of your current nickname.",
		/* LANG_GROUPVHOST_SYNTAX_EXT */
		"Syntax: %sgroupvhost your.desired.vHost\n"
		"\n"
		"Sets the vHost of all nicknames in your account.",
		/* LANG_VIDENT_VHOST_SYNTAX_EXT */
		"Syntax: %svhost [vIdent@]your.desired.vHost\n"
		"\n"
		"Sets the vIdent and vHost of your current nickname.",
		/* LANG_VIDENT_GROUPVHOST_SYNTAX_EXT */
		"Syntax: %sgroupvhost [vIdent@]your.desired.vHost\n"
		"\n"
		"Sets the vIdent and vHost of all nicknames in your account.",
		/* LANG_VHOST_SET_INTERVAL_NOT_EXPIRED */
		"You must wait until you can set another vhost. (Time remaining: %s)",
		/* LANG_VHOST_NOT_ALLOWED */
		"The requested vhost is not allowed.",
		/* LANG_NICK_VHOST_SET */
		"vHost for nick %s set to %s and activated.",
		/* LANG_NICK_VIDENT_VHOST_SET */
		"vHost for nick %s set to %s@%s and activated.",
		/* LANG_GROUP_VHOST_SET */
		"vHost for all nicks in group %s set to %s and activated.",
		/* LANG_GROUP_VIDENT_VHOST_SET */
		"vHost for all nicks in group %s set to %s@%s and activated.",
		/* LANG_MINIUM_TIME_REGISTERED */
		"You must be registered for at least %s before you are allowed to set your own vhost.",
	};

	char *langtable_nl[] = {
		/* LANG_CMD_DISABLED */
		"Dit commando is niet actief. Contacteer de netwerk staf voor meer informatie.",
		/* LANG_VHOST_SYNTAX */
		"Syntax: %svhost de.gewenste.vHost",
		/* LANG_GROUPVHOST_SYNTAX */
		"Syntax: %sgroupvhost de.gewenste.vHost",
		/* LANG_VIDENT_VHOST_SYNTAX */
		"Syntax: %svhost [vIdent@]de.gewenste.vHost",
		/* LANG_VIDENT_GROUPVHOST_SYNTAX */
		"Syntax: %sgroupvhost [vIdent@]de.gewenste.vHost",
		/* LANG_VHOST_SYNTAX_EXT */
		"Syntax: %svhost de.gewenste.vHost\n"
		"\n"
		"Zet de vHost voor je huidige nickname.",
		/* LANG_GROUPVHOST_SYNTAX_EXT */
		"Syntax: %sgroupvhost de.gewenste.vHost\n"
		"\n"
		"Zet de vHost voor alle nicks in je nickgroup.",
		/* LANG_VIDENT_VHOST_SYNTAX_EXT */
		"Syntax: %svhost [vIdent@]de.gewenste.vHost\n"
		"\n"
		"Zet de vIdent en vHost voor je huidige nickname.",
		/* LANG_VIDENT_GROUPVHOST_SYNTAX_EXT */
		"Syntax: %sgroupvhost [vIdent@]de.gewenste.vHost\n"
		"\n"
		"Zet de vIdent en vHost voor alle nicks in je nickgroup.",
		/* LANG_VHOST_SET_INTERVAL_NOT_EXPIRED */
		"Je moet wachten voordat je een niewe vhost kunt zetten. (Tijd resterend: %s)",
		/* LANG_VHOST_NOT_ALLOWED */
		"De gevraagde vhost is niet toegelaten.",
		/* LANG_NICK_VHOST_SET */
		"vHost voor nick %s is ingesteld op %s en geactiveerd.",
		/* LANG_NICK_VIDENT_VHOST_SET */
		"vHost voor nick %s is ingesteld op %s@%s en geactiveerd.",
		/* LANG_GROUP_VHOST_SET */
		"vHosts voor alle nicks in groep %s zijn ingesteld op %s en geactiveerd.",
		/* LANG_GROUP_VIDENT_VHOST_SET */
		"vHosts voor alle nicks in groep %s zijn ingesteld op %s@%s en geactiveerd.",
		/* LANG_MINIUM_TIME_REGISTERED */
		"Je moet minstens gedurende %s geregistreerd zijn vooralleer je je eigen vhost kan zetten.",
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
	moduleInsertLanguage(LANG_NL, LANG_NUM_STRINGS, langtable_nl);
}

/* ------------------------------------------------------------------------------- */

/* EOF */
