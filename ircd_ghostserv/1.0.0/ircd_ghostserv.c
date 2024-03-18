#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: ircd_ghostserv.c v1.0.0 22-10-2006 n00bie $"

/* ------------------------------------------------------------------------------
 * Name		: ircd_ghostserv.c
 * Author	: n00bie
 * Version	: 1.0.0
 * Date		: 22-10-2006
 * ----------------------------------------------------------------------------------
 * Description:
 *
 * This module create a new services client/bot. The bot will take care of all
 * NickServ GHOST's command and users will be killed by the bot instead of the
 * default GHOST killed.
 * ----------------------------------------------------------------------------------
 * Tested: Unreal3.2.5, Anope-1.7.17
 * ----------------------------------------------------------------------------------
 * Providing commands:
 *
 * /msg BotNick HELP
 * /msg BotNick GHOST [nick] [password]
 * /msg BotNick JOIN #Channel
 * /msg BotNick PART #Channel
 * ----------------------------------------------------------------------------------
 * Providing IRCd handler for: KICK, KILL
 *
 * Note: All NickServ GHOST command are handled and KILLED by the Bot, so if you
 * do not like that simply unload or unuse this module.
 * ----------------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 1, or (at your option) any later version.
 * ----------------------------------------------------------------------------------

INSTALLATION:
=============
Before you load this module, you need to put these lines into your services.conf
=====================================================================================

# GSNick [REQUIRED]
# The nickname of the bot.

GSNick "GhostServ"

# GSIdent [REQUIRED]
# The identd of the bot.

GSIdent "GS"

# GSHost [OPTIONAL]
# The vhost or hostmask of the bot.

GSHost "MyNet.Ghost.Buster.Bot"

# GSReal [REQUIRED]
# The realname of the Bot.

GSReal "/msg GhostServ HELP"

# GSModes [OPTIONAL]
# Modes the bot will have.

GSModes "+oSqB"

# GSQuitMsg [OPTIONAL]
# The bot quit msg when the module gets unloaded.

GSQuitMsg "Chasing Ghosts!"

# GSAutoJoin [OPTIONAL]
# Define this if you want the bot to automatically
# join your services log channel.

GSAutoJoin

=====================================================================================
*/

CommandHash *Ghostserv_cmdTable[MAX_CMD_HASH];
char *s_GhostServ;
char *GSIdent;
char *GSHost;
char *GSReal;
char *GSModes;
char *GSQuitMsg;

void delClient(char *nick);
void addMessageList(void);
void ghostserv(User *u, char *buf);

int GSAutoJoin;
int do_ghost(User *u);
int do_help(User *u);
int do_gs_join(User *u);
int do_gs_part(User *u);
int load_config(void);
int do_reload(int argc, char **argv);
int do_kill_rejoin(char *source, int argc, char **argv);
int do_kick_rejoin(char *source, int argc, char **argv);
int my_privmsg(char *source, int argc, char **argv);

int AnopeInit(int argc, char **argv)
{
	Command *c;
	Message *msg = NULL;
	EvtHook *hook = NULL;
	int status = 0;
	c = createCommand("GHOST", do_ghost, NULL, -1, -1 ,-1, -1, -1);
	status = moduleAddCommand(NICKSERV, c, MOD_HEAD);
	msg = createMessage("PRIVMSG", my_privmsg);
	status = moduleAddMessage(msg, MOD_HEAD);
	msg = createMessage("KILL", do_kill_rejoin);
	status = moduleAddMessage(msg, MOD_HEAD);
	msg = createMessage("KICK", do_kick_rejoin);
	status = moduleAddMessage(msg, MOD_HEAD);
	hook = createEventHook(EVENT_RELOAD, do_reload);
	status = moduleAddEventHook(hook);
	if (UseTokens) {
		msg = createMessage("!", my_privmsg);
		status = moduleAddMessage(msg, MOD_HEAD);
		if (status != MOD_ERR_OK) {
			alog("ircd_ghostserv: Unable to bind to PRIVMSG. Unloading module [%d]", status);
			return MOD_STOP;
		}
	}
	if (!(load_config())) {
		return MOD_STOP;
	}
	if (status == MOD_ERR_OK) {
		kill_user(NULL, s_GhostServ, "This nick is reserved for services");
		if (!GSModes || !GSHost) {
			anope_cmd_bot_nick(s_GhostServ, GSIdent, ServiceHost, GSReal, ircd->botservmode);
		} else {
			anope_cmd_bot_nick(s_GhostServ, GSIdent, GSHost, GSReal, GSModes);
		}
		addMessageList();
		if (GSAutoJoin && LogChannel) {
			anope_cmd_join(s_GhostServ, LogChannel, time(NULL));
			if (!stricmp(IRCDModule, "inspircd") || 
				!stricmp(IRCDModule, "plexus") || 
				!stricmp(IRCDModule, "ptlink") || 
				!stricmp(IRCDModule, "inspircd") || 
				!stricmp(IRCDModule, "ultimate2") || 
				!stricmp(IRCDModule, "unreal32") || 
				!stricmp(IRCDModule, "viagra")) {
					anope_cmd_mode(s_GhostServ, LogChannel, "+ao %s %s", s_GhostServ, s_GhostServ);
			} else {
				anope_cmd_mode(s_GhostServ, LogChannel, "+o %s", s_GhostServ);
			}
		}
		alog("ircd_ghostserv: Successfully loaded module.");
		alog("ircd_ghostserv: \2/msg %s HELP\2 for more information.", s_GhostServ);
	}
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void delClient(char *nick)
{
	if (!GSQuitMsg) {
		anope_cmd_quit(s_GhostServ, "Module Unloaded");
	} else {
		anope_cmd_quit(s_GhostServ, "Quit: %s", GSQuitMsg);
	}
}

void AnopeFini(void)
{
	alog("ircd_ghostserv: Module unloaded.");
	delClient(s_GhostServ);
	if (s_GhostServ)
		free(s_GhostServ);
	if (GSIdent)
		free(GSIdent);
	if (GSHost)
		free(GSHost);
	if (GSModes)
		free(GSModes);
	if (GSReal)
		free(GSReal);
	if (GSQuitMsg)
		free(GSQuitMsg);
}

void addMessageList(void)
{
    Command *c;
    c = createCommand("HELP", do_help, NULL, -1, -1, -1, -1, -1);
    moduleAddCommand(Ghostserv_cmdTable, c, MOD_HEAD);
	c = createCommand("GHOST", do_ghost, NULL, -1, -1, -1, -1, -1);
    moduleAddCommand(Ghostserv_cmdTable, c, MOD_HEAD);
	c = createCommand("JOIN", do_gs_join, NULL, -1, -1, -1, -1, -1);
    moduleAddCommand(Ghostserv_cmdTable, c, MOD_HEAD);
	c = createCommand("PART", do_gs_part, NULL, -1, -1, -1, -1, -1);
    moduleAddCommand(Ghostserv_cmdTable, c, MOD_HEAD);
}

void ghostserv(User *u, char *buf)
{
	char *cmd, *s;
	cmd = strtok(buf, " ");
	if (!cmd) {
		return;
	} else if (stricmp(cmd, "\1PING") == 0) {
		if (!(s = strtok(NULL, " ")))
			s = " ";
		anope_cmd_ctcp(s_GhostServ, u->nick, "\1PING %s", s);
	} else if (skeleton) {
		notice_lang(s_GhostServ, u, SERVICE_OFFLINE, s_GhostServ);
	} else {
		mod_run_cmd(s_GhostServ, u, Ghostserv_cmdTable, cmd);
	}
}

int my_privmsg(char *source, int argc, char **argv)
{
	User *u;
	char *s;
	if (argc != 2)
		return MOD_CONT;
	if (!(u = finduser(source))) {
		return MOD_CONT;
	}
	if (*argv[0] == '#') {
		return MOD_CONT;
	}
    s = strchr(argv[0], '@');
	if (s) {
		*s++ = 0;
		if (stricmp(s, ServerName) != 0)
			return MOD_CONT;
	}
	if ((stricmp(argv[0], s_GhostServ)) == 0) {
		ghostserv(u, argv[1]);
		return MOD_STOP;
	} else {
		return MOD_CONT;
	}
}

int do_kill_rejoin(char *source, int argc, char **argv) 
{
	if (argc != 2) {
		return MOD_STOP;
	}
	if (stricmp(argv[0], s_GhostServ) == 0) {
		alog("%s: I've got KILLED by a stupid Operator, \2I'll Be Back!\2", s_GhostServ);
		if (WallOSGlobal) {
			anope_cmd_global(s_OperServ, "\2%s\2 got KILLED by a stupid Operator.", s_GhostServ);
		}
		kill_user(NULL, s_GhostServ, "This nick is reserved for services");
		if (!GSModes || !GSHost) {
			anope_cmd_bot_nick(s_GhostServ, GSIdent, ServiceHost, GSReal, ircd->botservmode);
		} else {
			anope_cmd_bot_nick(s_GhostServ, GSIdent, GSHost, GSReal, GSModes);
		}
		if (GSAutoJoin && LogChannel) {
			anope_cmd_join(s_GhostServ, LogChannel, time(NULL));
			if (!stricmp(IRCDModule, "inspircd") || 
				!stricmp(IRCDModule, "plexus") || 
				!stricmp(IRCDModule, "ptlink") || 
				!stricmp(IRCDModule, "inspircd") || 
				!stricmp(IRCDModule, "ultimate2") || 
				!stricmp(IRCDModule, "unreal32") || 
				!stricmp(IRCDModule, "viagra")) {
					anope_cmd_mode(s_GhostServ, LogChannel, "+ao %s %s", s_GhostServ, s_GhostServ);
			} else {
				anope_cmd_mode(s_GhostServ, LogChannel, "+o %s", s_GhostServ);
			}
		}
	}
	return MOD_CONT;
}

int do_kick_rejoin(char *source, int argc, char **argv)
{
	if ((!(stricmp(argv[1], s_GhostServ) == 0))) {
		return MOD_CONT;
	}
	alog("%s got kicked from '%s' by %s (Auto re-joining)", s_GhostServ, argv[0], source);
	anope_cmd_join(s_GhostServ, argv[0], time(NULL));
	if (!stricmp(IRCDModule, "inspircd") || 
		!stricmp(IRCDModule, "plexus") || 
		!stricmp(IRCDModule, "ptlink") || 
		!stricmp(IRCDModule, "inspircd") || 
		!stricmp(IRCDModule, "ultimate2") || 
		!stricmp(IRCDModule, "unreal32") || 
		!stricmp(IRCDModule, "viagra")) {
			anope_cmd_mode(s_GhostServ, argv[0], "+ao %s %s", s_GhostServ, s_GhostServ);
	} else {
		anope_cmd_mode(s_GhostServ, argv[0], "+o %s", s_GhostServ);
	}
	return MOD_CONT;
}

int do_gs_join(User *u)
{
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	if (!chan) {
		notice(s_ChanServ, u->nick, "You did not specify a channel to join.");
		notice(s_ChanServ, u->nick, "Syntax: \2JOIN\2 \037#CHANNEL\037");
	} else if (!is_oper(u)) {
		notice(s_GhostServ, u->nick, "Permission denied.");
		if (LogChannel) {
			alog("%s: Access denied for \2%s\2 with command JOIN %s", s_GhostServ, u->nick, chan);
		}
	} else {
		anope_cmd_join(s_GhostServ, chan, time(NULL));
		if (!stricmp(IRCDModule, "inspircd") || 
			!stricmp(IRCDModule, "plexus") || 
			!stricmp(IRCDModule, "ptlink") || 
			!stricmp(IRCDModule, "inspircd") || 
			!stricmp(IRCDModule, "ultimate2") || 
			!stricmp(IRCDModule, "unreal32") || 
			!stricmp(IRCDModule, "viagra")) {
				anope_cmd_mode(s_GhostServ, chan, "+ao %s %s", s_GhostServ, s_GhostServ);
		} else {
			anope_cmd_mode(s_GhostServ, chan, "+o %s", s_GhostServ);
		}
	}
	if (chan)
		free(chan);
	return MOD_CONT;
}

int do_gs_part(User *u)
{
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	if (!chan) {
		notice(s_ChanServ, u->nick, "Syntax: \2PART\2 \037#CHANNEL\037");
	} else if (!is_oper(u)) {
		notice(s_GhostServ, u->nick, "Permission denied.");
		if (LogChannel) {
			alog("%s: Access denied for \2%s\2 with command PART %s", s_GhostServ, u->nick, chan);
		}
	} else {
		anope_cmd_part(s_GhostServ, chan, "PART command from %s", u->nick);
	}
	if (chan)
		free(chan);
	return MOD_CONT;
}

int do_help(User *u)
{
	notice(s_GhostServ, u->nick, "Syntax: \2GHOST \037nickname\037 \037password\037\2");
	notice(s_GhostServ, u->nick, " ");
	notice(s_GhostServ, u->nick, "Terminates a 'ghost' IRC session using your nick.  A");
	notice(s_GhostServ, u->nick, "'ghost' session is one which is not actually connected,");
	notice(s_GhostServ, u->nick, "but which the IRC server believes is still online for one");
	notice(s_GhostServ, u->nick, "reason or another.  Typically, this happens if your");
	notice(s_GhostServ, u->nick, "computer crashes or your Internet or modem connection");
	notice(s_GhostServ, u->nick, "goes down while you're on IRC.");
	notice(s_GhostServ, u->nick, " ");
	notice(s_GhostServ, u->nick, "In order to use the \2GHOST\2 command for a nick, your");
	notice(s_GhostServ, u->nick, "current address as shown in /WHOIS must be on that nick's");
	notice(s_GhostServ, u->nick, "access list, you must be identified and in the group of");
	notice(s_GhostServ, u->nick, "that nick, or you must supply the correct password for");
	notice(s_GhostServ, u->nick, "the nickname.");
	notice(s_GhostServ, u->nick, " ");
	return MOD_CONT;
}

int do_reload(int argc, char **argv)
{
    load_config();
    return MOD_CONT;
}

int load_config(void)
{
	int i;
	int retval = 1;

	Directive confvalues[][1] = {
		{{ "GSNick", {{ PARAM_STRING, PARAM_RELOAD, &s_GhostServ }} }},
		{{ "GSIdent", {{ PARAM_STRING, PARAM_RELOAD, &GSIdent }} }},
		{{ "GSHost", {{ PARAM_STRING, PARAM_RELOAD, &GSHost }} }},
		{{ "GSReal", {{ PARAM_STRING, PARAM_RELOAD, &GSReal }} }},
		{{ "GSModes", {{ PARAM_STRING, PARAM_RELOAD, &GSModes }} }},
		{{ "GSQuitMsg", {{ PARAM_STRING, PARAM_RELOAD, &GSQuitMsg }} }},
        {{ "GSAutoJoin", {{ PARAM_SET, PARAM_RELOAD, &GSAutoJoin }} }},
	};

    for (i = 0; i < 7; i++)
        moduleGetConfigDirective(confvalues[i]);

    if (!s_GhostServ) {
        alog("Config Error: \2GSNick\2 missing");
		retval = 0;
	}

    if (!GSIdent) {
        alog("Config Error: \2GSIdent\2 missing");
		retval = 0;
	}

    if (!GSReal) {
        alog("Config Error: \2GSReal\2 missing");
		retval = 0;
	}
	return retval;
}

int do_ghost(User *u)
{
	User *u2;

	char *buf = moduleGetLastBuffer();
	char *nick = myStrGetToken(buf, ' ', 0);
    char *pass = myStrGetToken(buf, ' ', 1);
    int res;
    NickAlias *na;

	if (nickIsServices(nick, 1)) {
		notice(s_GhostServ, u->nick, "Are you Serious? You can't ghost services!");
		return MOD_STOP;
    }

    if (!nick || !pass) {
		notice(s_GhostServ, u->nick, "Syntax: \2GHOST \037nickname\037 \037password\037\2");
		notice(s_GhostServ, u->nick, "\2/msg %s HELP\2 for more information.", s_GhostServ);
    } else if (!(u2 = finduser(nick))) {
		notice_lang(s_GhostServ, u, NICK_X_NOT_IN_USE, nick);
	} else if (!(na = u2->na)) {
		notice_lang(s_GhostServ, u, NICK_X_NOT_REGISTERED, nick);
	} else if (na->status & NS_VERBOTEN) {
		notice_lang(s_GhostServ, u, NICK_X_FORBIDDEN, na->nick);
	} else if (na->nc->flags & NI_SUSPENDED) {
		notice_lang(s_GhostServ, u, NICK_X_SUSPENDED, na->nick);
	} else if (stricmp(nick, u->nick) == 0) {
		notice_lang(s_GhostServ, u, NICK_NO_GHOST_SELF);
	} else if (pass) {
		int res = check_password(pass, na->nc->pass);
		if (res == 1) {
			char mbuf[NICKMAX + 32];
			snprintf(mbuf, sizeof(mbuf), "Killed (Ghost nick %s busted by %s)", nick, u->nick);
			if (LimitSessions) {
				del_session(u2->host);
			}
			kill_user(s_GhostServ, nick, mbuf);
			notice_lang(s_GhostServ, u, NICK_GHOST_KILLED, nick);
        } else {
			notice_lang(s_GhostServ, u, ACCESS_DENIED);
            if (res == 0) {
				alog("%s: GHOST: invalid password for %s by %s!%s@%s",
					s_GhostServ, nick, u->nick, u->username, u->host);
				bad_password(u);
			}
		}
	} else {
		if (group_identified(u, na->nc) || (!(na->nc->flags & NI_SECURE) && is_on_access(u, na->nc))) {
			char mbuf[NICKMAX + 32];
			snprintf(mbuf, sizeof(mbuf), "Killed (Ghost nick %s busted by %s)", nick, u->nick);
			if (LimitSessions) {
				del_session(u2->host);
			}
			kill_user(s_GhostServ, nick, mbuf);
			notice_lang(s_GhostServ, u, NICK_GHOST_KILLED, nick);
        } else {
			notice_lang(s_GhostServ, u, ACCESS_DENIED);
			if (res == 0) {
                alog("%s: GHOST: invalid password for %s by %s!%s@%s",
					s_GhostServ, nick, u->nick, u->username, u->host);
				bad_password(u);
			}
		}
	}
	if (nick)
		free(nick);
	if (pass)
		free(pass);
    return MOD_STOP;
}

/* EOF */

