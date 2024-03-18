/* -------------------------------------------------------------------------
 * Name		: ircd_init.c
 * Author	: n00bie [n00bie@rediffmail.com]
 * Changed by   : Maddin [maddin_b@web.de] and Evil.2000 [evil.2000@gmx.de]
 * Version	: 2.0.1
 * Date		: 23rd Sept, 2006
 * Update	: 28th Mar, 2008
 * -------------------------------------------------------------------------
 * Tested	: Anope-1.7.21
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
 *    It will join a channel as soon as it gets registered by someone.
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
 * v2.0.0 : 
 *          • Cleaned up minor codes.
 *          • Services clients and botserv bots will not join anymore
 *            if LogChannel is not defined on services.conf
 * v2.0.1 : Patched by Maddin & Evil.2000
 *         • ChanServ will join on newly register channel
 *         •  ChanServ will now part a channel if it is DROPPED
 * -----------------------------------------------------------------------
 * This module have no configurable option.
 */

#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: ircd_init.c v2.0.1 28-03-2008 n00bie, Maddin, Evil.2000 $"
void load_chanserv();
void load_services();
void load_bots();
int do_register(int ac, char **av);
int do_drop(int ac, char **av);
int cs_kick_rejoin(char *source, int ac, char **av);

int AnopeInit(int argc, char **argv)
{
	Message *msg = NULL;
	EvtHook *hook;

	msg = createMessage("KICK", cs_kick_rejoin);
	moduleAddMessage(msg, MOD_TAIL);
	if (LogChannel) {
		logchan = 1;
		load_services();
		load_bots();
	}
	load_chanserv();
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	/**
	 * This will execute "do_registered" function if a new channel is registered
	 */
        hook = createEventHook(EVENT_CHAN_REGISTERED, do_register);
        if (moduleAddEventHook(hook) != MOD_ERR_OK) {
                alog("ircd_init%s: Can't hook to EVENT_CHAN_REGISTERED event. Stopping.", MODULE_EXT);
                return MOD_STOP;
	}
	/**
	 * Execute do_drop on channel drop.
	 */
	hook = createEventHook(EVENT_CHAN_DROP, do_drop);
        if (moduleAddEventHook(hook) != MOD_ERR_OK) {
                alog("ircd_init%s: Can't hook to EVENT_CHAN_DROP event. Stopping.", MODULE_EXT);
                return MOD_STOP;
        }
	
	alog("ircd_init%s: Successfully loaded module.", MODULE_EXT);
	return MOD_CONT;
}

/**
 * Try to join a newly registered channel.
 *
 * @param int ac - argument count
 * @param array av - agrument values
 */
int do_register(int ac, char **av) {
        alog("%s: The channel %s gots newly registered. I will join it now.", s_ChanServ, av[0]);

	/* Do a join into the new chan */
	anope_cmd_join(s_ChanServ, av[0], time(NULL));
	/* Set the correct modes to ChanServ */
	anope_cmd_bot_chan_mode(s_ChanServ, av[0]);
        
	/* return "Module Continue" */
	return MOD_CONT;
}

/**
 * Try to leave a dropped channel.
 *
 * @param int ac - argument count
 * @param array av - agrument values
 */
int do_drop(int ac, char **av) {
        alog("%s: The channel %s gots dropped. I will leave it too.", s_ChanServ, av[0]);

        /* Leave the dropped chan */
        anope_cmd_part(s_ChanServ, av[0], NULL);

        /* return "Module Continue" */
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
			anope_cmd_bot_chan_mode(s_ChanServ, ci->name);
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
	anope_cmd_bot_chan_mode(s_ChanServ, av[0]);
	anope_cmd_mode(s_ChanServ, av[0], "+b %s", source);
	anope_cmd_kick(s_ChanServ, av[0], source, "Abusing Services");
	return MOD_CONT;
}

void load_services()
{
	if (s_ChanServ) {
		anope_cmd_join(s_ChanServ, LogChannel, time(NULL));
		anope_cmd_bot_chan_mode(s_ChanServ, LogChannel);
	}
	if (s_MemoServ) {
		anope_cmd_join(s_MemoServ, LogChannel, time(NULL));
		anope_cmd_bot_chan_mode(s_MemoServ, LogChannel);
	}
	if (s_NickServ) {
		anope_cmd_join(s_NickServ, LogChannel, time(NULL));
		anope_cmd_bot_chan_mode(s_NickServ, LogChannel);
	}
	if (s_BotServ) {
		anope_cmd_join(s_BotServ, LogChannel, time(NULL));
		anope_cmd_bot_chan_mode(s_BotServ, LogChannel);
	}
	if (s_HostServ) {
		anope_cmd_join(s_HostServ, LogChannel, time(NULL));
		anope_cmd_bot_chan_mode(s_HostServ, LogChannel);
	}
	if (s_OperServ) {
		anope_cmd_join(s_OperServ, LogChannel, time(NULL));
		anope_cmd_bot_chan_mode(s_OperServ, LogChannel);
	}
	if (s_DevNull) { // Do we really need DevNull? anyway, carry on...
		anope_cmd_join(s_DevNull, LogChannel, time(NULL));
		anope_cmd_bot_chan_mode(s_DevNull, LogChannel);
	}
	if (s_HelpServ) {
		anope_cmd_join(s_HelpServ, LogChannel, time(NULL));
		anope_cmd_bot_chan_mode(s_HelpServ, LogChannel);
	}
	if (s_GlobalNoticer) { // Global?
		anope_cmd_join(s_GlobalNoticer, LogChannel, time(NULL));
		anope_cmd_bot_chan_mode(s_GlobalNoticer, LogChannel);
	}
	if (s_NickServAlias) {
		anope_cmd_join(s_NickServAlias, LogChannel, time(NULL));
		anope_cmd_bot_chan_mode(s_NickServAlias, LogChannel);
	}
	if (s_ChanServAlias) {
		anope_cmd_join(s_ChanServAlias, LogChannel, time(NULL));
		anope_cmd_bot_chan_mode(s_ChanServAlias, LogChannel);
	}
	if (s_BotServAlias) {
		anope_cmd_join(s_BotServAlias, LogChannel, time(NULL));
		anope_cmd_bot_chan_mode(s_BotServAlias, LogChannel);
	}
	if (s_MemoServAlias) {
		anope_cmd_join(s_MemoServAlias, LogChannel, time(NULL));
		anope_cmd_bot_chan_mode(s_MemoServAlias, LogChannel);
	}
	if (s_HelpServAlias) {
		anope_cmd_join(s_HelpServAlias, LogChannel, time(NULL));
		anope_cmd_bot_chan_mode(s_HelpServAlias, LogChannel);
	}
	if (s_OperServAlias) {
		anope_cmd_join(s_OperServAlias, LogChannel, time(NULL));
		anope_cmd_bot_chan_mode(s_OperServAlias, LogChannel);
	}
	if (s_NickServAlias) {
		anope_cmd_join(s_NickServAlias, LogChannel, time(NULL));
		anope_cmd_bot_chan_mode(s_NickServAlias, LogChannel);
	}
	if (s_DevNullAlias) { // Again DevNull? nevermind...
		anope_cmd_join(s_DevNullAlias, LogChannel, time(NULL));
		anope_cmd_bot_chan_mode(s_DevNullAlias, LogChannel);
	}
	if (s_HostServAlias) {
		anope_cmd_join(s_HostServAlias, LogChannel, time(NULL));
		anope_cmd_bot_chan_mode(s_HostServAlias, LogChannel);
	}
	if (s_GlobalNoticerAlias) { // At last...
		anope_cmd_join(s_GlobalNoticerAlias, LogChannel, time(NULL));
		anope_cmd_bot_chan_mode(s_GlobalNoticerAlias, LogChannel);
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
				anope_cmd_bot_chan_mode(bi->nick, LogChannel);
  			}
		}
	}
}

void AnopeFini(void)
{
        ChannelInfo *ci;
        int i = 0;
        for (i = 0; i < 256; i++) {
                for (ci = chanlists[i]; ci; ci = ci->next) {
                        if ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED)) {
                                continue;
                        }
                        anope_cmd_part(s_ChanServ, ci->name, NULL);
                }
        }
	alog("ircd_init%s: module unloaded.", MODULE_EXT);
}

/* EOF */
