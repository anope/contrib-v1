#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: cs_unban.c v1.0.0 04-07-2007 n00bie $"

/*******************************************************************************************************
** Name		: cs_unban.c v1.0.0
** Author	: n00bie
** Email	: n00bie@rediffmail.com
** Date		: July 04, 2007
********************************************************************************************************
** Description:
** 
** This module change the way of ChanServ UNBAN command. When giving the
** /ChanServ UNBAN command, instead of services.localhost.net sets mode: -b *!*@host,
** ChanServ or BotServ bot (if assigned) will unban or be displayed on the channel.
********************************************************************************************************
** This program is free software; you can redistribute it and/or modify it under the
** terms of the GNU General Public License as published by the Free Software 
** Foundation; either version 1, or (at your option) any later version.
********************************************************************************************************
** Module suggested by [dx]
** This module have no configurable option.
*******************************************************************************************************/

int do_cs_unban(User *u);
void m_common_unban(ChannelInfo *ci, char *nick);

/**
 * Create the command, and tell anope about it.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv)
{
	Command *c;
	c = createCommand("UNBAN", do_cs_unban, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(CHANSERV, c, MOD_HEAD);
	alog("%s: cs_unban: Module loaded successfully.", s_ChanServ);
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

/**
 * The /cs unban command.
 * @param u The user who issued the command
 * @param MOD_CONT to continue processing other modules, MOD_STOP to stop processing.
 **/
int do_cs_unban(User *u)
{
    Channel *c;
    ChannelInfo *ci;
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
    if (!chan) {
        syntax_error(s_ChanServ, u, "UNBAN", CHAN_UNBAN_SYNTAX);
		return MOD_STOP;
    } else if (!(c = findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_IN_USE, chan);
		free(chan);
		return MOD_STOP;
    } else if (!(ci = c->ci)) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
		free(chan);
		return MOD_STOP;
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
		free(chan);
		return MOD_STOP;
    } else if (!check_access(u, ci, CA_UNBAN)) {
        notice_lang(s_ChanServ, u, PERMISSION_DENIED);
		free(chan);
		return MOD_STOP;
    } else {
        m_common_unban(ci, u->nick);
        notice_lang(s_ChanServ, u, CHAN_UNBANNED, chan);
		free(chan);
    }
    return MOD_STOP;
}

/**
 * Unban the nick from a channel
 * @param ci channel info for the channel
 * @param nick to remove the ban for
 * @return void
 */
void m_common_unban(ChannelInfo *ci, char *nick)
{
    int count, i;
    char *av[3], **bans;
    User *u;
    char *host;
    int matchfound = 0;
    if (!ci || !ci->c || !nick) {
        return;
    }
    if (!(u = finduser(nick))) {
        return;
    }
    host = host_resolve(u->host);
    av[0] = ci->name;
	av[1] = sstrdup("-b");
	count = ci->c->bancount;
	bans = scalloc(sizeof(char *) * count, 1);
	memcpy(bans, ci->c->bans, sizeof(char *) * count);
	for (i = 0; i < count; i++) {
		if (match_usermask(bans[i], u)) {
			anope_cmd_mode(whosends(ci), ci->name, "-b %s", bans[i]);
			av[2] = bans[i];
			do_cmode(whosends(ci), 3, av);
			matchfound++;
		}
		if (host) {
			/* prevent multiple unbans if the first one was successful in
			locating the ban for us. This is due to match_userip() checks
			the vhost again, and thus can return false positive results
			for the function. but won't prevent thus from clearing out
			the bans against an IP address since the first would fail and
			the second would match - TSL
			*/
			if (!matchfound) {
				if (match_userip(bans[i], u, host)) {
					anope_cmd_mode(whosends(ci), ci->name, "-b %s", bans[i]);
					av[2] = bans[i];
					do_cmode(whosends(ci), 3, av);
				}
			}
		}
		matchfound = 0;
	}
	free(bans);
	free(av[1]);
	/* host_resolve() sstrdup us this info so we gotta free it */
	if (host) {
		free(host);
    }
}

void AnopeFini(void)
{
	alog("%s: cs_unban: module unloaded.", s_ChanServ);
}

/* EOF */