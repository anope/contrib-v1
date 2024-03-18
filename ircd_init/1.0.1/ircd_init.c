/* -------------------------------------------------------------------------
 * Name		: ircd_init.c
 * Author	: n00bie
 * Version	: 1.0.1
 * Date		: 23rd Sept, 2006
 * Update	: 27th Sept, 2006
 * -------------------------------------------------------------------------
 * Tested	: Anope-1.7.13, 1.7.14, 1.7.15
 * -------------------------------------------------------------------------
 * Supported IRCd's:
 * All IRCd's listed on services.conf are supported by this module.
 * -------------------------------------------------------------------------
 * Installation / Usage:
 * Add this file 'ircd_init' on the 'ModuleDelayedAutoload' section
 * in your services.conf making sure that you enable ModuleDelayedAutoload.
 * -------------------------------------------------------------------------
 * Description / Features:
 *
 * 1) This module will automatically turned ON or ENABLE log channel
 *    when services get restarted or as soon as this module is loaded.
 *
 * 2) ChanServ will join all registered channels as soon as this module
 *    is loaded or whenever services get restarted.
 *    (except for suspended or forbidden channels)
 *
 * 3) All services clients (ChanServ, NickServ, BotServ, OperServ etc.)
 *    will join services log channel (e.g. #Services) as soon as this
 *    module is loaded or whenever services get restarted.
 *
 * 4) All BotServ bots (if available) will join services log channel
 *    as soon as this module is loaded or whenever services get restarted.
 *
 * 5) If some stupid oper KICK ChanServ from a Channel, ChanServ will
 *    rejoin the channel and kick/banned the user.
 * -----------------------------------------------------------------------
 * Changes:
 * v1.0.0 : First Public Release.
 * v1.0.1 : Fixed a missing correct mode.
 * -----------------------------------------------------------------------
 * This module have no configurable option.
 */

#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: ircd_init.c v1.0.1 27-09-2006 spa $"
void load_chanserv();
void load_services();
void load_bots();
int cs_kick_rejoin(char *source, int ac, char **av);
int AnopeInit(int argc, char **argv)
{
	Message *msg = NULL;
	msg = createMessage("KICK", cs_kick_rejoin);
	moduleAddMessage(msg, MOD_TAIL);
	logchan = 1;
	load_chanserv();
	load_services();
	load_bots();
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	alog("ircd_init%s: Successfully loaded module.", MODULE_EXT);
	return MOD_CONT;
}

void load_chanserv()

{
	ChannelInfo *ci;
	int i = 0;
	for (i = 0; i < 256; i++) {
		for (ci = chanlists[i]; ci; ci = ci->next) {
			if ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED)) {
				continue;
			}
			anope_cmd_join(s_ChanServ, ci->name, time(NULL));
			if (!stricmp(IRCDModule, "inspircd") ||
				!stricmp(IRCDModule, "plexus") ||
				!stricmp(IRCDModule, "ptlink") ||
				!stricmp(IRCDModule, "inspircd") ||
				!stricmp(IRCDModule, "ultimate2") ||
				!stricmp(IRCDModule, "unreal32") ||
				!stricmp(IRCDModule, "viagra")) {
					anope_cmd_mode(s_ChanServ, ci->name, "+ao %s %s", s_ChanServ, s_ChanServ);
				} else {
					anope_cmd_mode(s_ChanServ, ci->name, "+o %s", s_ChanServ);
			}
		}
	}
	return;
}

int cs_kick_rejoin(char *source, int ac, char **av)
{
	if ((!(stricmp(av[1], s_ChanServ) == 0))) {
		return MOD_CONT;
	}
	if (LogChannel) {
		alog("%s got kicked from '%s' by %s (Auto re-joining)", s_ChanServ, av[0], source);
	}
	anope_cmd_join(s_ChanServ, av[0], time(NULL));
	if (!stricmp(IRCDModule, "inspircd") ||
		!stricmp(IRCDModule, "plexus") ||
		!stricmp(IRCDModule, "ptlink") ||
		!stricmp(IRCDModule, "inspircd") ||
		!stricmp(IRCDModule, "ultimate2") ||
		!stricmp(IRCDModule, "unreal32") ||
		!stricmp(IRCDModule, "viagra")) {
			anope_cmd_mode(s_ChanServ, av[0], "+ao %s %s", s_ChanServ, s_ChanServ);
		} else {
			anope_cmd_mode(s_ChanServ, av[0], "+o %s", s_ChanServ);
	}
	anope_cmd_mode(s_ChanServ, av[0], "+b %s", source);
	anope_cmd_kick(s_ChanServ, av[0], source, "Abusing Services");
	return MOD_CONT;
}

void load_services()
{
	if (s_ChanServ) {
		anope_cmd_join(s_ChanServ, LogChannel, time(NULL));
		if (!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "plexus") ||
			!stricmp(IRCDModule, "ptlink") ||
			!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "ultimate2") ||
			!stricmp(IRCDModule, "unreal32") ||
			!stricmp(IRCDModule, "viagra")) {
				anope_cmd_mode(s_ChanServ, LogChannel, "+ao %s %s", s_ChanServ, s_ChanServ);
		} else {
			anope_cmd_mode(s_ChanServ, LogChannel, "+o %s", s_ChanServ);
		}
	}
	if (s_MemoServ) {
		anope_cmd_join(s_MemoServ, LogChannel, time(NULL));
		if (!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "plexus") ||
			!stricmp(IRCDModule, "ptlink") ||
			!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "ultimate2") ||
			!stricmp(IRCDModule, "unreal32") ||
			!stricmp(IRCDModule, "viagra")) {
				anope_cmd_mode(s_MemoServ, LogChannel, "+ao %s %s", s_MemoServ, s_MemoServ);
		} else {
			anope_cmd_mode(s_MemoServ, LogChannel, "+o %s", s_MemoServ);
		}
	}
	if (s_NickServ) {
		anope_cmd_join(s_NickServ, LogChannel, time(NULL));
		if (!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "plexus") ||
			!stricmp(IRCDModule, "ptlink") ||
			!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "ultimate2") ||
			!stricmp(IRCDModule, "unreal32") ||
			!stricmp(IRCDModule, "viagra")) {
				anope_cmd_mode(s_NickServ, LogChannel, "+ao %s %s", s_NickServ, s_NickServ);
		} else {
			anope_cmd_mode(s_NickServ, LogChannel, "+o %s", s_NickServ);
		}
	}
	if (s_BotServ) {
		anope_cmd_join(s_BotServ, LogChannel, time(NULL));
		if (!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "plexus") ||
			!stricmp(IRCDModule, "ptlink") ||
			!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "ultimate2") ||
			!stricmp(IRCDModule, "unreal32") ||
			!stricmp(IRCDModule, "viagra")) {
				anope_cmd_mode(s_BotServ, LogChannel, "+ao %s %s", s_BotServ, s_BotServ);
		} else {
			anope_cmd_mode(s_BotServ, LogChannel, "+o %s", s_BotServ);
		}
	}
	if (s_HostServ) {
		anope_cmd_join(s_HostServ, LogChannel, time(NULL));
		if (!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "plexus") ||
			!stricmp(IRCDModule, "ptlink") ||
			!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "ultimate2") ||
			!stricmp(IRCDModule, "unreal32") ||
			!stricmp(IRCDModule, "viagra")) {
				anope_cmd_mode(s_HostServ, LogChannel, "+ao %s %s", s_HostServ, s_HostServ);
		} else {
			anope_cmd_mode(s_HostServ, LogChannel, "+o %s", s_HostServ);
		}
	}
	if (s_OperServ) {
		anope_cmd_join(s_OperServ, LogChannel, time(NULL));
		if (!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "plexus") ||
			!stricmp(IRCDModule, "ptlink") ||
			!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "ultimate2") ||
			!stricmp(IRCDModule, "unreal32") ||
			!stricmp(IRCDModule, "viagra")) {
				anope_cmd_mode(s_OperServ, LogChannel, "+ao %s %s", s_OperServ, s_OperServ);
		} else {
			anope_cmd_mode(s_OperServ, LogChannel, "+o %s", s_OperServ);
		}
	}
	if (s_DevNull) { // Do we really need DevNull? anyway, carry on...
		anope_cmd_join(s_DevNull, LogChannel, time(NULL));
		if (!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "plexus") ||
			!stricmp(IRCDModule, "ptlink") ||
			!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "ultimate2") ||
			!stricmp(IRCDModule, "unreal32") ||
			!stricmp(IRCDModule, "viagra")) {
				anope_cmd_mode(s_DevNull, LogChannel, "+ao %s %s", s_DevNull, s_DevNull);
		} else {
			anope_cmd_mode(s_DevNull, LogChannel, "+o %s", s_DevNull);
		}
	}
	if (s_HelpServ) {
		anope_cmd_join(s_HelpServ, LogChannel, time(NULL));
		if (!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "plexus") ||
			!stricmp(IRCDModule, "ptlink") ||
			!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "ultimate2") ||
			!stricmp(IRCDModule, "unreal32") ||
			!stricmp(IRCDModule, "viagra")) {
				anope_cmd_mode(s_HelpServ, LogChannel, "+ao %s %s", s_HelpServ, s_HelpServ);
		} else {
			anope_cmd_mode(s_HelpServ, LogChannel, "+o %s", s_HelpServ);
		}
	}
	if (s_GlobalNoticer) { // Global?
		anope_cmd_join(s_GlobalNoticer, LogChannel, time(NULL));
		if (!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "plexus") ||
			!stricmp(IRCDModule, "ptlink") ||
			!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "ultimate2") ||
			!stricmp(IRCDModule, "unreal32") ||
			!stricmp(IRCDModule, "viagra")) {
				anope_cmd_mode(s_GlobalNoticer, LogChannel, "+ao %s %s", s_GlobalNoticer, s_GlobalNoticer);
		} else {
			anope_cmd_mode(s_GlobalNoticer, LogChannel, "+o %s", s_GlobalNoticer);
		}
	}
	if (s_NickServAlias) {
		anope_cmd_join(s_NickServAlias, LogChannel, time(NULL));
		if (!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "plexus") ||
			!stricmp(IRCDModule, "ptlink") ||
			!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "ultimate2") ||
			!stricmp(IRCDModule, "unreal32") ||
			!stricmp(IRCDModule, "viagra")) {
				anope_cmd_mode(s_NickServAlias, LogChannel, "+ao %s %s", s_NickServAlias, s_NickServAlias);
		} else {
			anope_cmd_mode(s_NickServAlias, LogChannel, "+o %s", s_NickServAlias);
		}
	}
	if (s_ChanServAlias) {
		anope_cmd_join(s_ChanServAlias, LogChannel, time(NULL));
		if (!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "plexus") ||
			!stricmp(IRCDModule, "ptlink") ||
			!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "ultimate2") ||
			!stricmp(IRCDModule, "unreal32") ||
			!stricmp(IRCDModule, "viagra")) {
				anope_cmd_mode(s_ChanServAlias, LogChannel, "+ao %s %s", s_ChanServAlias, s_ChanServAlias);
		} else {
			anope_cmd_mode(s_ChanServAlias, LogChannel, "+o %s", s_ChanServAlias);
		}
	}
	if (s_BotServAlias) {
		anope_cmd_join(s_BotServAlias, LogChannel, time(NULL));
		if (!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "plexus") ||
			!stricmp(IRCDModule, "ptlink") ||
			!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "ultimate2") ||
			!stricmp(IRCDModule, "unreal32") ||
			!stricmp(IRCDModule, "viagra")) {
				anope_cmd_mode(s_BotServAlias, LogChannel, "+ao %s %s", s_BotServAlias, s_BotServAlias);
		} else {
			anope_cmd_mode(s_BotServAlias, LogChannel, "+o %s", s_BotServAlias);
		}
	}
	if (s_MemoServAlias) {
		anope_cmd_join(s_MemoServAlias, LogChannel, time(NULL));
		if (!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "plexus") ||
			!stricmp(IRCDModule, "ptlink") ||
			!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "ultimate2") ||
			!stricmp(IRCDModule, "unreal32") ||
			!stricmp(IRCDModule, "viagra")) {
				anope_cmd_mode(s_MemoServAlias, LogChannel, "+ao %s %s", s_MemoServAlias, s_MemoServAlias);
		} else {
			anope_cmd_mode(s_MemoServAlias, LogChannel, "+o %s", s_MemoServAlias);
		}
	}
	if (s_HelpServAlias) {
		anope_cmd_join(s_HelpServAlias, LogChannel, time(NULL));
		if (!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "plexus") ||
			!stricmp(IRCDModule, "ptlink") ||
			!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "ultimate2") ||
			!stricmp(IRCDModule, "unreal32") ||
			!stricmp(IRCDModule, "viagra")) {
				anope_cmd_mode(s_HelpServAlias, LogChannel, "+ao %s %s", s_HelpServAlias, s_HelpServAlias);
		} else {
			anope_cmd_mode(s_HelpServAlias, LogChannel, "+o %s", s_HelpServAlias);
		}
	}
	if (s_OperServAlias) {
		anope_cmd_join(s_OperServAlias, LogChannel, time(NULL));
		if (!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "plexus") ||
			!stricmp(IRCDModule, "ptlink") ||
			!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "ultimate2") ||
			!stricmp(IRCDModule, "unreal32") ||
			!stricmp(IRCDModule, "viagra")) {
				anope_cmd_mode(s_OperServAlias, LogChannel, "+ao %s %s", s_OperServAlias, s_OperServAlias);
		} else {
			anope_cmd_mode(s_OperServAlias, LogChannel, "+o %s", s_OperServAlias);
		}
	}
	if (s_NickServAlias) {
		anope_cmd_join(s_NickServAlias, LogChannel, time(NULL));
		if (!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "plexus") ||
			!stricmp(IRCDModule, "ptlink") ||
			!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "ultimate2") ||
			!stricmp(IRCDModule, "unreal32") ||
			!stricmp(IRCDModule, "viagra")) {
				anope_cmd_mode(s_NickServAlias, LogChannel, "+ao %s %s", s_NickServAlias, s_NickServAlias);
		} else {
			anope_cmd_mode(s_NickServAlias, LogChannel, "+o %s", s_NickServAlias);
		}
	}
	if (s_DevNullAlias) { // Again DevNull? nevermind...
		anope_cmd_join(s_DevNullAlias, LogChannel, time(NULL));
		if (!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "plexus") ||
			!stricmp(IRCDModule, "ptlink") ||
			!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "ultimate2") ||
			!stricmp(IRCDModule, "unreal32") ||
			!stricmp(IRCDModule, "viagra")) {
				anope_cmd_mode(s_DevNullAlias, LogChannel, "+ao %s %s", s_DevNullAlias, s_DevNullAlias);
		} else {
			anope_cmd_mode(s_DevNullAlias, LogChannel, "+o %s", s_DevNullAlias);
		}
	}
	if (s_HostServAlias) {
		anope_cmd_join(s_HostServAlias, LogChannel, time(NULL));
		if (!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "plexus") ||
			!stricmp(IRCDModule, "ptlink") ||
			!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "ultimate2") ||
			!stricmp(IRCDModule, "unreal32") ||
			!stricmp(IRCDModule, "viagra")) {
				anope_cmd_mode(s_HostServAlias, LogChannel, "+ao %s %s", s_HostServAlias, s_HostServAlias);
		} else {
			anope_cmd_mode(s_HostServAlias, LogChannel, "+o %s", s_HostServAlias);
		}
	}
	if (s_GlobalNoticerAlias) { // At last...
		anope_cmd_join(s_GlobalNoticerAlias, LogChannel, time(NULL));
		if (!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "plexus") ||
			!stricmp(IRCDModule, "ptlink") ||
			!stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "ultimate2") ||
			!stricmp(IRCDModule, "unreal32") ||
			!stricmp(IRCDModule, "viagra")) {
				anope_cmd_mode(s_GlobalNoticerAlias, LogChannel, "+ao %s %s", s_GlobalNoticerAlias, s_GlobalNoticerAlias);
		} else {
			anope_cmd_mode(s_GlobalNoticerAlias, LogChannel, "+o %s", s_GlobalNoticerAlias);
		}
	}
}

void load_bots()
{
	int i;
	BotInfo *bi;
	if (!nbots) {
		return;
	} else {
		for (i = 0; i < 256; i++) {
			for (bi = botlists[i]; bi; bi = bi->next) {
    			anope_cmd_join(bi->nick, LogChannel, time(NULL));
				if (!stricmp(IRCDModule, "inspircd") ||
					!stricmp(IRCDModule, "plexus") ||
					!stricmp(IRCDModule, "ptlink") ||
					!stricmp(IRCDModule, "inspircd") ||
					!stricmp(IRCDModule, "ultimate2") ||
					!stricmp(IRCDModule, "unreal32") ||
					!stricmp(IRCDModule, "viagra")) {
						anope_cmd_mode(bi->nick, LogChannel, "+ao %s %s", bi->nick, bi->nick);
				} else {
					anope_cmd_mode(bi->nick, LogChannel, "+o %s", bi->nick);
				}
  			}
		}
	}
}

void AnopeFini(void)
{
	alog("ircd_init%s: Successfully unloaded module.", MODULE_EXT);
}

/* EOF */

