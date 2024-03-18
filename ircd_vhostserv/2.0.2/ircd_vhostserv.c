#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: ircd_vhostserv.c v2.0.2 04-03-2008 n00bie $"

/* ------------------------------------------------------------------------------------
 * Name		: ircd_vhostserv.c
 * Author	: n00bie [n00bie@rediffmail.com]
 * Version	: 2.0.2
 * Date		: 2nd October, 2006
 * Update	: 4th March, 2008
 * ------------------------------------------------------------------------------------
 * Description:
 *
 * This module create a new services client/bot. The bot have its own fantasy command
 * !vhost and !groupvhost. The bot will answer the fantasy commands ONLY on its own channel.
 * ------------------------------------------------------------------------------------
 * Providing Fantasy Commands:
 *
 * !vhost		some.vhost.here		- set a vhost for users
 * !groupvhost	group.vhost.here	- set vhost for all users in group
 * !vhost restricted				- list all restricted vhosts
 * ------------------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * ------------------------------------------------------------------------------------
 * Change Log:
 * v1.0.0 - First Public Release
 *
 * v1.0.1 -
 * a) Fixed a few mem leaks
 * b) Fixed a bug on vIdent
 *
 * v1.0.2 -
 * a) Minor code update
 * b) Added modload prevention for unsupported IRCd's. The module will not load on
 *    unsupported IRCd's (bahamut, dreamforge, hybrid, ratbox).
 * c) Added Tokens support
 * d) Added checked for vHost channel before modload. If the vHost channel is not
 *	  registered, the module will not load.
 *
 * v2.0.0 - Fixed a crash bug when the line starts with a space in it.
 *
 * v2.0.1 -
 * a) Added !groupvhost fantasy command for setting all users vhost in a group
 * b) Added configuration directive support
 * c) Added vhost flood protection; will now kick/ban users after a vhost is set
 * d) Remove !hs on|off fantasy command
 * e) Module will not load on version less than 1.7.21
 *
 * v2.0.2 -
 * a) Added check for unallowed vhosts.
 * b) Added options to kick-ban users who request unallowed vhosts.
 * c) Added fantasy command "!vhost restricted" to list all restricted hosts so that
 *	  a user can be warned first before kickban.
 * ------------------------------------------------------------------------------------
 * Module made possible using vhost fantasy module by Trystan
 * ------------------------------------------------------------------------------------
 * This module have 10 configurable options.
 *
 * Copy/Paste below on services.conf

# BotNick <nickname> [REQUIRED]
# Module: ircd_vhostserv
#	Define what nick the bot will have.
#
BotNick "vHostServ"

# BotIdent <ident> [REQUIRED]
#	Define the bot identd.
#
BotIdent "Bot"

# BotHost <hostmask> [REQUIRED]
#	Define the bot hostname or vhost.
#
BotHost "LocalHost-IRC.Host.Bot"

# BotReal <realname> [REQUIRED]
#	Define the bot realname.
#
BotReal "/msg vHostServ Help"

# vHostChannel <channel> [REQUIRED]
#	Define your network vhost channel.
#
vHostChannel "#vHost"

# BanClearTime <time> [REQUIRED]
#	Sets the timeout period after which a particular ban set by
#	the bot will be removed.
#
BanClearTime 5h

# BotModes <modes> [OPTIONAL]
#	Define what modes the bot will have.
#
BotModes "+SqB"

# JoinLogChannel [OPTIONAL]
#	Define this if you want the bot to automatically
#	join services log channel.
#
JoinLogChannel

# RestrictHosts <unallowed hosts> [RECOMMENDED]
#	Define unallowed vhosts which will be match using 
#	wildcards. This is not case-sensitive.
#
RestrictHosts "*ircop* *admin* *root* *staff* *some.wild.card*"

# KickBanRestrictHosts [OPTIONAL]
#	Define this to kick banned users who request
#	restricted hosts. If this option is enabled
#	RestrictHosts setting is required.
#
KickBanRestrictHosts

# End of config: ircd_vhostserv
*/

char *s_vHostServ;
char *BotIdent;
char *BotHost;
char *BotReal;
char *BotModes;
char *BotChan;
char *RestrictHosts;
char **RestrictHostList;
int RestrictHostNum;
int KickBanRestrictHosts;
int vHostServBanClearTime;
int JoinLogChannel;
int check_banned_vhost(char *vhost);
int mEventReload(int argc, char **argv);
int mLoadConfig(void);
int do_privmsg(char *source, int argc, char **argv);
int vhost_fantasy(User *u, char *cur_buffer);
int vhost_fantasy_group(User *u, char *cur_buffer);
int do_help(User *u);
int delBan(int argc, char **argv);
void addClient(void);
void delClient(void);
void addMessageList(void);
void addBan(Channel *c, time_t timeout, char *nick);
void Vhostserv(User *u, char *buf);
void VBotChanmsgs(User *u, ChannelInfo *ci, char *buf);
CommandHash *Vhostserv_cmdTable[MAX_CMD_HASH];
extern int do_hs_sync(NickCore *nc, char *vIdent, char *hostmask, char *creator, time_t time);
int AnopeInit(int argc, char **argv)
{
	Message *msg = NULL;
	EvtHook *hook = NULL;
	ChannelInfo *ci;
	int status = 0;
	msg = createMessage("PRIVMSG", do_privmsg);
	status = moduleAddMessage(msg, MOD_HEAD);
	if (status != MOD_ERR_OK) {
		alog("ircd_vhostserv: Unable to bind to PRIVMSG");
        return MOD_STOP;
    }
	hook = createEventHook(EVENT_RELOAD, mEventReload);
    status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("ircd_vhostserv: Unable to hook to EVENT_RELOAD");
        return MOD_STOP;
    }
	if (UseTokens) {
		msg = createMessage("!", do_privmsg);
		status = moduleAddMessage(msg, MOD_HEAD);
		if (status != MOD_ERR_OK) {
			alog("ircd_vhostserv: Unable to bind to PRIVMSG");
			return MOD_STOP;
		}
	}
	if (!ircd->vhost) {
		alog("ircd_vhostserv: \2IRCd not supported.\2");
		return MOD_STOP;
	}
	if (!moduleMinVersion(1,7,21,1341)) {
		alog("ircd_vhostserv: Your version of Anope isn't supported. This module require Anope-1.7.21 (1341) or later. Please update to a newer release.");
		return MOD_STOP;
	}
	if (mLoadConfig()) {
        return MOD_STOP;
    }
	addClient();
	addMessageList();
	if (!(ci = cs_findchan(BotChan))) {
		alog("ircd_vhostserv: Error: \2vHostChannel\2 is not defined in services configuration file or vHostChannel is not registered. You need to defined or register the vHostChannel first for this module to work.");
		return MOD_STOP;
	} else {
		anope_cmd_join(s_vHostServ, ci->name, time(NULL));
		anope_cmd_bot_chan_mode(s_vHostServ, ci->name);
	}
	if (LogChannel) {
		if (JoinLogChannel) {
			anope_cmd_join(s_vHostServ, LogChannel, time(NULL));
			anope_cmd_bot_chan_mode(s_vHostServ, LogChannel);
		}
	}
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void)
{
	int idx;
	delClient();

	if (s_vHostServ)
		free(s_vHostServ);

	if (BotIdent)
		free(BotIdent);

	if (BotHost)
		free(BotHost);

	if (BotReal)
		free(BotReal);

	if (BotModes)
		free(BotModes);

	if (BotChan)
		free(BotChan);

	if (RestrictHostNum) {
		for (idx = 0; idx < RestrictHostNum; idx++) {
			free(RestrictHostList[idx]);
		}
	}

	alog("ircd_vhostserv: Module Unloaded.");
}

void addClient(void)
{
    if (BotModes) {
		anope_cmd_bot_nick(s_vHostServ, BotIdent, BotHost, BotReal, BotModes);
    } else {
        anope_cmd_bot_nick(s_vHostServ, BotIdent, BotHost, BotReal, ircd->botservmode);
    }
}

void delClient(void)
{
	anope_cmd_quit(s_vHostServ, "Quit: Module Unloaded!");
}

int do_help(User *u)
{
	anope_cmd_notice(s_vHostServ, u->nick, "Syntax: \2!vhost \037Your.vHost.Here\037\2");
	anope_cmd_notice(s_vHostServ, u->nick, " ");
	anope_cmd_notice(s_vHostServ, u->nick, "Sets a vhost.");
	anope_cmd_notice(s_vHostServ, u->nick, " ");
	anope_cmd_notice(s_vHostServ, u->nick, "Syntax: \2!groupvhost \037Group.vHost.Here\037\2");
	anope_cmd_notice(s_vHostServ, u->nick, " ");
	anope_cmd_notice(s_vHostServ, u->nick, "This command allows user to set the vhost of their");
	anope_cmd_notice(s_vHostServ, u->nick, "CURRENT nick to be the vhost for all nicks in the same");
	anope_cmd_notice(s_vHostServ, u->nick, "group.");
	anope_cmd_notice(s_vHostServ, u->nick, " ");
	anope_cmd_notice(s_vHostServ, u->nick, "Syntax: \2!vhost \037restricted\037\2");
	anope_cmd_notice(s_vHostServ, u->nick, " ");
	anope_cmd_notice(s_vHostServ, u->nick, "This command allows user to see all hosts which");
	anope_cmd_notice(s_vHostServ, u->nick, "are not allowed to used. If a restricted hosts");
	anope_cmd_notice(s_vHostServ, u->nick, "is chosen %s will trigger a \2kick-ban\2", s_vHostServ);
	anope_cmd_notice(s_vHostServ, u->nick, "combination depending on the network settings.");
	if (ircd->vident) {
		anope_cmd_notice(s_vHostServ, u->nick, " ");
		anope_cmd_notice(s_vHostServ, u->nick, "Note: \2vIdentd\2 is supported by the IRCd.");
	} else {
		anope_cmd_notice(s_vHostServ, u->nick, " ");
		anope_cmd_notice(s_vHostServ, u->nick, "Note: This Network does not support vIdentd.");
	}
	return MOD_CONT;
}

int do_privmsg(char *source, int argc, char **argv)
{
	ChannelInfo *ci;
	User *u;
	char *s;
	if (argc != 2) {
		return MOD_CONT;
	}
	if (!(u = finduser(source))) {
		return MOD_CONT;
	}
	if (!stricmp(BotChan, argv[0])) {
		if (s_vHostServ && (ci = cs_findchan(BotChan))) {
			if (!(ci->flags & CI_VERBOTEN) && ci->c && s_vHostServ) {
				VBotChanmsgs(u, ci, argv[1]);
			}
		}
	}
	s = strchr(argv[0], '@');
	if (s) {
		*s++ = 0;
		if (stricmp(s, ServerName) != 0) {
			return MOD_CONT;
		}
	}
	if ((stricmp(argv[0], s_vHostServ)) == 0) {
		Vhostserv(u, argv[1]);
		return MOD_STOP;
	} else {
		return MOD_CONT;
	}
}

void addMessageList(void)
{
	Command *c;
	c = createCommand("HELP", do_help, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(Vhostserv_cmdTable, c, MOD_HEAD);
}

void Vhostserv(User *u, char *buf)
{
	char *cmd, *s;
	cmd = myStrGetToken(buf, ' ', 0);
	s = myStrGetToken(buf, ' ', 1);
	if (!cmd) {
		return;
	} else if (stricmp(cmd, "\1PING") == 0) {
		if (!s)
			s = "\1";
		anope_cmd_ctcp(s_vHostServ, u->nick, "\1PING %s", s);
	} else if (skeleton) {
		notice_lang(s_vHostServ, u, SERVICE_OFFLINE, s_vHostServ);
	} else {
		mod_run_cmd(s_vHostServ, u, Vhostserv_cmdTable, cmd);
	}
	if (cmd) free(cmd);
	if (s) free(s);
}

int mEventReload(int argc, char **argv)
{
	int ret = 0;
    if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			ret = mLoadConfig();
        }
	}
	if (ret) {
		alog("ircd_vhostserv: \2ERROR\2: An error has occured while reloading the configuration file!");
	}
	return MOD_CONT;
}

int mLoadConfig(void)
{
	int i, idx;
	int retval = 0;
	char *s;
	Directive directives[][1] = {
		{{"BotNick", {{PARAM_STRING, 0, &s_vHostServ}}}},
		{{"BotIdent", {{PARAM_STRING, 0, &BotIdent}}}},
		{{"BotHost", {{PARAM_STRING, 0, &BotHost}}}},
		{{"BotReal", {{PARAM_STRING, 0, &BotReal}}}},
		{{"BotModes", {{PARAM_STRING, 0, &BotModes}}}},
		{{"vHostChannel", {{PARAM_STRING, 0, &BotChan}}}},
		{{"JoinLogChannel", {{PARAM_SET, 0, &JoinLogChannel}}}},
		{{"BanClearTime", {{PARAM_TIME, 0, &vHostServBanClearTime}}}},
		{{"RestrictHosts", {{PARAM_STRING, 0, &RestrictHosts}}}},
		{{"KickBanRestrictHosts", {{PARAM_SET, 0, &KickBanRestrictHosts}}}},
	};
    for (i = 0; i < 10; i++)
        moduleGetConfigDirective(directives[i]);

	if (!s_vHostServ) {
		alog("ircd_vhostserv: Error: \2BotNick\2 is missing on services configuration file");
		retval = 1;
	}

	if (!BotIdent) {
		alog("ircd_vhostserv: Error: \2BotIdent\2 is missing on services configuration file");
		retval = 1;
	}

	if (!BotHost) {
		alog("ircd_vhostserv: Error: \2BotHost\2 is missing on services configuration file");
		retval = 1;
	}

	if (!BotReal) {
		alog("ircd_vhostserv: Error: \2BotReal\2 is missing on services configuration file");
		retval = 1;
	}

	if (!BotChan) {
		alog("ircd_vhostserv: Error: \2vHostChannel\2 is missing on services configuration file");
		retval = 1;
	}

	if (!vHostServBanClearTime) {
		alog("ircd_vhostserv: Error: \2BanClearTime\2 is missing on services configuration file");
		retval = 1;
	}
	
	if (RestrictHostNum) {
		for (idx = 0; idx < RestrictHostNum; idx++) {
			free(RestrictHostList[idx]);
		}
	}

	if (RestrictHosts) {
		RestrictHostNum = 0;
        s = strtok(RestrictHosts, " ");
        do {
            if (s) {
                RestrictHostNum++;
                RestrictHostList = realloc(RestrictHostList, sizeof(char *) * RestrictHostNum);
                RestrictHostList[RestrictHostNum - 1] = sstrdup(s);
            }
        } while ((s = strtok(NULL, " ")));
	}

	return retval;
}

void VBotChanmsgs(User *u, ChannelInfo *ci, char *buf)
{
	int idx, count = 0;
	char *cmd = myStrGetToken(buf, ' ', 0);
	char *param = myStrGetToken(buf, ' ', 1);
	if (!cmd)
		return;
	if (!stricmp(cmd, "!vhost")) {
		if (!param) {
			anope_cmd_notice(s_vHostServ, u->nick, "Syntax: \2!vhost\2 \037Your.vHost.Here\037");
		} else if (!stricmp(param, "restricted")) {
			anope_cmd_notice(s_vHostServ, u->nick, "The following *hosts* are not allowed:");
			anope_cmd_notice(s_vHostServ, u->nick, "\037Num\037  \037Restricted\037");
			for (idx = 0; idx < RestrictHostNum; idx++) {
				count++;
				anope_cmd_notice(s_vHostServ, u->nick, "%3d  %s", count, RestrictHostList[idx]);
			}
			anope_cmd_notice(s_vHostServ, u->nick, "End of List.");
		} else {
			vhost_fantasy(u, param);
		}
	}
	if (!stricmp(cmd, "!groupvhost")) {
		if (!param) {
			anope_cmd_notice(s_vHostServ, u->nick, "Syntax: \2!groupvhost\2 \037Group.vHost.Here\037");
		} else if (!stricmp(param, "restricted")) {
			anope_cmd_notice(s_vHostServ, u->nick, "See \2!vhost restricted\2");
		} else {
			vhost_fantasy_group(u, param);
		}
	}
	if (cmd)
		free(cmd);
	if (param)
		free(param);
}

void addBan(Channel *c, time_t timeout, char *nick)
{
    char *av[3];
    char *cb[2];
    cb[0] = c->name;
    cb[1] = nick;
    av[0] = sstrdup("+b");
	av[1] = nick;
    anope_cmd_mode(s_vHostServ, c->name, "+b %s", av[1]);
    chan_set_modes(s_vHostServ, c, 2, av, 1);
    free(av[0]);
	moduleAddCallback("ircd_vhostserv: delBan", time(NULL) + timeout, delBan, 2, cb);
}

int delBan(int argc, char **argv)
{
    char *av[3];
    Channel *c;
    av[0] = sstrdup("-b");
    av[1] = argv[1];
    if ((c = findchan(BotChan)) && c->ci) {
        anope_cmd_mode(s_vHostServ, c->name, "-b %s", av[1]);
        chan_set_modes(s_vHostServ, c, 2, av, 1);
    }
    free(av[0]);
    return MOD_CONT;
}

int vhost_fantasy(User *u, char *cur_buffer)
{
	Channel *c;
	User *u2 = NULL;
	char *nick = u->nick;
	char *rawhostmask = cur_buffer;
	char *hostmask = smalloc(HOSTMAX);
	NickAlias *na;
	int32 tmp_time;
	char *s;
	int idx;
	char *vIdent = NULL;
    if (!nick || !rawhostmask) {
        return MOD_STOP;
    }
	if (!(c = findchan(BotChan))) {
		return MOD_STOP;
	}
	if (!(u2 = finduser(u->nick))) {
		return MOD_STOP;
	}
    vIdent = myStrGetOnlyToken(rawhostmask, '@', 0);    /* Get the first substring, @ as delimiter */
	if (vIdent) {
		rawhostmask = myStrGetTokenRemainder(rawhostmask, '@', 1);      /* get the remaining string */
        if (!rawhostmask) {
			return MOD_STOP;
		}
        if (strlen(vIdent) > USERMAX - 1) {
			anope_cmd_privmsg(s_vHostServ, BotChan,
				"The Ident is too long, please use an ident shorter than %d characters.", USERMAX);
			if (vIdent)
				free(vIdent);
			return MOD_STOP;
		} else {
			for (s = vIdent; *s; s++) {
				if (!isvalidchar(*s)) {
					anope_cmd_privmsg(s_vHostServ, BotChan,
						"A vhost ident must be in the format of a valid ident.");
					if (vIdent)
						free(vIdent);
					return MOD_STOP;
				}
			}
		}
        if (!ircd->vident) {
			anope_cmd_privmsg(s_vHostServ, BotChan, "This Network does not support vIdentd.");
			if (vIdent)
				free(vIdent);
			return MOD_STOP;
		}
	}
    if (strlen(rawhostmask) < HOSTMAX - 1) {
        snprintf(hostmask, HOSTMAX - 1, "%s", rawhostmask);
    } else {
        anope_cmd_privmsg(s_vHostServ, BotChan,
			"The vhost is too long, please use a host shorter than \2%d\2 characters.", HOSTMAX);
		if (vIdent)
			free(vIdent);
		return MOD_STOP;
	}
	if (!isValidHost(hostmask, 3)) {
		anope_cmd_privmsg(s_vHostServ, BotChan, "A vhost must be in the format of a valid hostmask.");
		if (vIdent)
			free(vIdent);
		free(hostmask);
		return MOD_STOP;
	}
	if (!check_banned_vhost(hostmask)) {
		if (KickBanRestrictHosts && RestrictHosts) {
			if (vHostServBanClearTime > 0) {
				addBan(c, vHostServBanClearTime, nick);
				if (is_on_chan(c, u2)) {
					anope_cmd_kick(s_vHostServ, c->ci->name, nick, 
						"The vhost you requested is not allowed. Banned for %d seconds", vHostServBanClearTime);
				}
			}
		} else {
			anope_cmd_privmsg(s_vHostServ, BotChan, "The requested vhost is not allowed, please choose another.");
		}
		if (vIdent) {
            free(vIdent);
        }
        free(hostmask);
        return MOD_STOP;
    }
	tmp_time = time(NULL);
	if ((na = findnick(nick))) {
		if (na->status & NS_VERBOTEN) {
			anope_cmd_privmsg(s_vHostServ, BotChan, "Cannot set vhost for forbidden nick %s", nick);
			if (vIdent)
				free(vIdent);
			if (hostmask)
				free(hostmask);
			return MOD_STOP;
		}
		if (!nick_identified(u)) {
			notice_lang(s_vHostServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
			if (vIdent)
				free(vIdent);
			if (hostmask)
				free(hostmask);
			return MOD_STOP;
		}
		if (vIdent && ircd->vident) {
			alog("vHost for user \2%s\2 set to \2%s@%s\2 by %s", nick, vIdent, hostmask, s_vHostServ);
		} else {
			alog("vHost for user \2%s\2 set to \2%s\2 by %s", nick, hostmask, s_vHostServ);
		}
		addHostCore(nick, vIdent, hostmask, u->nick, tmp_time);
		anope_cmd_vhost_on(nick, vIdent, hostmask);
		set_lastmask(u2);
		if (vHostServBanClearTime > 0) {
			addBan(c, vHostServBanClearTime, nick);
			if (is_on_chan(c, u2)) {
				anope_cmd_kick(s_vHostServ, c->ci->name, nick, "Done. You can request a new vhost after %d seconds from your last one. Banned for %d seconds", vHostServBanClearTime, vHostServBanClearTime);
			}
		}
		if (vIdent && ircd->vident) {
			anope_cmd_notice(s_vHostServ, nick, "vHost for \2%s\2 set to \2%s@%s\2 and activated.", nick, vIdent, hostmask);
		} else {
			anope_cmd_notice(s_vHostServ, nick, "vHost for \2%s\2 set to \2%s\2 and activated.", nick, hostmask);
		}
	} else {
		anope_cmd_privmsg(s_vHostServ, BotChan, "\2%s\2, please register your nick first.", nick);
	}
    if (vIdent)
		free(vIdent);
	if (hostmask)
		free(hostmask);
    return MOD_CONT;
}

int vhost_fantasy_group(User *u, char *cur_buffer)
{
	char *nick = u->nick;
	char *rawhostmask = cur_buffer;
	char *hostmask = smalloc(HOSTMAX);
	User *u2 = NULL;
	Channel *c;
	NickAlias *na;
	int32 tmp_time;
	char *s;
	char *vIdent = NULL;
    if (!nick || !rawhostmask) {
        return MOD_STOP;
    }
	if (!(c = findchan(BotChan))) {
		return MOD_CONT;
	}
	if (!(u2 = finduser(u->nick))) {
		return MOD_CONT;
	}
	if (!(na = findnick(u->nick))) {
		return MOD_CONT;
	}
    vIdent = myStrGetOnlyToken(rawhostmask, '@', 0);    /* Get the first substring, @ as delimiter */
    if (vIdent) {
		rawhostmask = myStrGetTokenRemainder(rawhostmask, '@', 1);      /* get the remaining string */
        if (!rawhostmask) {
			return MOD_STOP;
		}
        if (strlen(vIdent) > USERMAX - 1) {
			anope_cmd_privmsg(s_vHostServ, BotChan,
				"The Ident is too long, please use an ident shorter than %d characters.", USERMAX);
			if (vIdent)
				free(vIdent);
			return MOD_STOP;
		} else {
			for (s = vIdent; *s; s++) {
				if (!isvalidchar(*s)) {
					anope_cmd_privmsg(s_vHostServ, BotChan,
						"A vhost ident must be in the format of a valid ident.");
					if (vIdent)
						free(vIdent);
					return MOD_STOP;
				}
			}
		}
        if (!ircd->vident) {
			anope_cmd_privmsg(s_vHostServ, BotChan, "This Network does not support vIdentd.");
			if (vIdent)
				free(vIdent);
			return MOD_STOP;
		}
	}
    if (strlen(rawhostmask) < HOSTMAX - 1) {
        snprintf(hostmask, HOSTMAX - 1, "%s", rawhostmask);
    } else {
        anope_cmd_privmsg(s_vHostServ, BotChan,
			"The vhost is too long, please use a host shorter than \2%d\2 characters.", HOSTMAX);
		if (vIdent)
			free(vIdent);
		return MOD_STOP;
	}
	if (!isValidHost(hostmask, 3)) {
		anope_cmd_privmsg(s_vHostServ, BotChan, "A vhost must be in the format of a valid hostmask.");
		if (vIdent) {
            free(vIdent);
        }
        free(hostmask);
        return MOD_STOP;
	}
	if (!check_banned_vhost(hostmask)) {
		if (KickBanRestrictHosts && RestrictHosts) {
			if (vHostServBanClearTime > 0) {
				addBan(c, vHostServBanClearTime, nick);
				if (is_on_chan(c, u2)) {
					anope_cmd_kick(s_vHostServ, c->ci->name, nick, 
						"The vhost you requested is not allowed. Banned for %d seconds", vHostServBanClearTime);
				}
			}
		} else {
			anope_cmd_privmsg(s_vHostServ, BotChan, "The requested vhost is not allowed, please choose another.");
		}
		if (vIdent) {
            free(vIdent);
        }
        free(hostmask);
        return MOD_STOP;
    }
	tmp_time = time(NULL);
	if ((na = findnick(nick))) {
		if (na->status & NS_VERBOTEN) {
			anope_cmd_privmsg(s_vHostServ, BotChan, "Cannot set vhost for forbidden nick %s", nick);
			if (vIdent)
				free(vIdent);
			if (hostmask)
				free(hostmask);
            return MOD_STOP;
		}
		if (!nick_identified(u)) {
			notice_lang(s_vHostServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
			if (vIdent)
				free(vIdent);
			if (hostmask)
				free(hostmask);
			return MOD_STOP;
		}
		if (vIdent && ircd->vident) {
            alog("vHost for all nicks in group \2%s\2 set to \2%s@%s\2 by %s", nick, vIdent, hostmask, s_vHostServ);
		} else {
			alog("vHost for all nicks in group \2%s\2 set to \2%s\2 by %s", nick, hostmask, s_vHostServ);
        }
		do_hs_sync(na->nc, vIdent, hostmask, u->nick, tmp_time);
		anope_cmd_vhost_on(nick, vIdent, hostmask);
		set_lastmask(u2);
		if (vHostServBanClearTime > 0) {
			addBan(c, vHostServBanClearTime, nick);
			if (is_on_chan(c, u2)) {
				anope_cmd_kick(s_vHostServ, c->ci->name, nick, "Done. You can request a new vhost after %d seconds from your last one. Banned for %d seconds", vHostServBanClearTime, vHostServBanClearTime);
			}
		}
		if (vIdent && ircd->vident) {
			anope_cmd_notice(s_vHostServ, nick, "vHost for all nicks in group \2%s\2 set to \2%s@%s\2 and activated.", nick, vIdent, hostmask);
		} else {
			anope_cmd_notice(s_vHostServ, nick, "vHost for all nicks in group \2%s\2 set to \2%s\2 and activated.", nick, hostmask);
		}
	} else {
		anope_cmd_privmsg(s_vHostServ, BotChan, "\2%s\2, please register your nick first.", nick);
	}
    if (vIdent)
		free(vIdent);
	if (hostmask)
		free(hostmask);
    return MOD_CONT;
}

int check_banned_vhost(char *vhost)
{
    int idx;
	
    for (idx = 0; idx < RestrictHostNum; idx++) {
        if (match_wild_nocase(RestrictHostList[idx], vhost)) {
            return 0;
        }
    }
    return 1;
}

/* EOF */
