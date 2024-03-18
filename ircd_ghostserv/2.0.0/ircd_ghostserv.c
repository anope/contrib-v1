#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: ircd_ghostserv.c v2.0.0 18-01-2007 n00bie $"

/* ------------------------------------------------------------------------------
 * Name		: ircd_ghostserv.c
 * Author	: n00bie [n00bie@rediffmail.com]
 * Version	: 2.0.0
 * Date		: 22-10-2006
 * Update	: 18-01-2008
 * ----------------------------------------------------------------------------------
 * Description:
 *
 * This module create a new services client/bot. The bot will take care of all
 * NickServ GHOST's command and users will be killed by the bot instead of the
 * default GHOST killed.
 * ----------------------------------------------------------------------------------
 * Tested: Unreal3.2.7, Anope-1.7.21
 * ----------------------------------------------------------------------------------
 * Providing commands:
 *
 * /msg GhostServ HELP
 * /msg GhostServ GHOST [nick] [password]
 * /msg GhostServ JOIN #Channel
 * /msg GhostServ PART #Channel
 * ----------------------------------------------------------------------------------
 * Providing IRCd handler for: KICK, KILL
 *
 * Note: All NickServ GHOST command are handled and KILLED by the Bot, so if you
 * do not like this, simply unload or do not use this module.
 * ----------------------------------------------------------------------------------
 * Changelog:
 * v1.0.0	- First Public Release
 * v2.0.0	- 
 *			  • Minor code cleaned up
 *			  • Bot will not be automatically opped anymore upon joining a channel
 *			    or when kicked
 *			  • Added and will now respect 'BSMinUsers' directives from services.conf
 *			  • Fixed compiling error on enc_check_password
 * ----------------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 1, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * ----------------------------------------------------------------------------------

INSTALLATION:
=============
Before you load this module, you need to put these lines into your services.conf
=====================================================================================

# Module: ircd_ghostserv
#
# GSNick [REQUIRED]
# The nickname of the bot.

GSNick "GhostBuster"

# GSIdent [REQUIRED]
# The identd of the bot.

GSIdent "GS"

# GSHost [OPTIONAL]
# The vhost or hostmask of the bot.

GSHost "MyNet.Ghost.Buster.Bot"

# GSReal [REQUIRED]
# The realname of the Bot.

GSReal "/msg GhostBuster HELP"

# GSModes [OPTIONAL]
# Modes the bot will have.

GSModes "+SqB"

# GSQuitMsg [OPTIONAL]
# The bot quit msg when the module gets unloaded.

GSQuitMsg "Chasing Ghosts!"

# GSAutoJoin [OPTIONAL]
# Define this if you want the bot to automatically
# join your services log channel.

GSAutoJoin

# End of module: ircd_ghostserv

=====================================================================================
*/

CommandHash *Ghostserv_cmdTable[MAX_CMD_HASH];
char *GSNick;
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
		kill_user(NULL, GSNick, "This nick is reserved for services");
		if (!GSModes || !GSHost) {
			anope_cmd_bot_nick(GSNick, GSIdent, ServiceHost, GSReal, ircd->botservmode);
		} else {
			anope_cmd_bot_nick(GSNick, GSIdent, GSHost, GSReal, GSModes);
		}
		addMessageList();
		if (GSAutoJoin && LogChannel) {
			anope_cmd_join(GSNick, LogChannel, time(NULL));
			anope_cmd_bot_chan_mode(GSNick, LogChannel);
		}
		alog("ircd_ghostserv: Successfully loaded module.");
		alog("ircd_ghostserv: \2/msg %s HELP\2 for more information.", GSNick);
	}
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void delClient(char *nick)
{
	if (!GSQuitMsg) {
		anope_cmd_quit(GSNick, "Module Unloaded");
	} else {
		anope_cmd_quit(GSNick, "Quit: %s", GSQuitMsg);
	}
}

void AnopeFini(void)
{
	alog("ircd_ghostserv: Module unloaded.");
	delClient(GSNick);
	if (GSNick)
		free(GSNick);
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
	cmd = myStrGetToken(buf, ' ', 0);
	s = myStrGetToken(buf, ' ', 1);
	if (!cmd) {
		return;
	} else if (stricmp(cmd, "\1PING") == 0) {
		if (!s)
			s = "\1";
		anope_cmd_ctcp(GSNick, u->nick, "\1PING %s", s);
	} else if (skeleton) {
		notice_lang(GSNick, u, SERVICE_OFFLINE, GSNick);
	} else {
		mod_run_cmd(GSNick, u, Ghostserv_cmdTable, cmd);
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
	if ((stricmp(argv[0], GSNick)) == 0) {
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
	if (stricmp(argv[0], GSNick) == 0) {
		alog("%s: I've got KILLED by a stupid Operator, \2I'll Be Back!\2", GSNick);
		if (WallOSGlobal) {
			anope_cmd_global(s_OperServ, "\2%s\2 got KILLED by a stupid Operator.", GSNick);
		}
		kill_user(NULL, GSNick, "This nick is reserved for services");
		if (!GSModes || !GSHost) {
			anope_cmd_bot_nick(GSNick, GSIdent, ServiceHost, GSReal, ircd->botservmode);
		} else {
			anope_cmd_bot_nick(GSNick, GSIdent, GSHost, GSReal, GSModes);
		}
		if (GSAutoJoin && LogChannel) {
			anope_cmd_join(GSNick, LogChannel, time(NULL));
			anope_cmd_bot_chan_mode(GSNick, LogChannel);
		}
	}
	return MOD_CONT;
}

int do_kick_rejoin(char *source, int argc, char **argv)
{
	if ((!(stricmp(argv[1], GSNick) == 0))) {
		return MOD_CONT;
	}
	alog("%s got kicked from '%s' by %s (Auto re-joining)", GSNick, argv[0], source);
	anope_cmd_join(GSNick, argv[0], time(NULL));
	return MOD_CONT;
}

int do_gs_join(User *u)
{
	ChannelInfo *ci;
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	if (!chan) {
		notice(GSNick, u->nick, "You did not specify a channel to join.");
		notice(GSNick, u->nick, "Syntax: \2JOIN\2 \037#CHANNEL\037");
	} else if (!is_oper(u)) {
		notice(GSNick, u->nick, "Permission denied.");
		if (LogChannel) {
			alog("%s: Access denied for \2%s\2 with command JOIN %s", GSNick, u->nick, chan);
		}
	} else if (!(ci = cs_findchan(chan))) {
		notice_lang(GSNick, u, CHAN_X_NOT_REGISTERED, chan);
	} else if ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED)) {
		notice_lang(GSNick, u, CHAN_X_FORBIDDEN, chan);
	} else if (ci->c && ci->c->usercount < BSMinUsers) {
		notice_user(GSNick, u, "Can join only when there are %d user(s) in %s.", BSMinUsers, chan);
	} else {
		anope_cmd_join(GSNick, chan, time(NULL));
	}
	if (chan)
		free(chan);
	return MOD_CONT;
}

int do_gs_part(User *u)
{
	ChannelInfo *ci;
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	if (!chan) {
		notice(GSNick, u->nick, "Syntax: \2PART\2 \037#CHANNEL\037");
	} else if (!(ci = cs_findchan(chan))) {
		notice_lang(GSNick, u, CHAN_X_NOT_REGISTERED, chan);
	} else if (!is_oper(u)) {
		notice(GSNick, u->nick, "Permission denied.");
		if (LogChannel) {
			alog("%s: Access denied for \2%s\2 with command PART %s", GSNick, u->nick, chan);
		}
	} else {
		anope_cmd_part(GSNick, chan, "PART command from %s", u->nick);
	}
	if (chan)
		free(chan);
	return MOD_CONT;
}

int do_help(User *u)
{
	notice(GSNick, u->nick, "Syntax: \2GHOST \037nickname\037 \037password\037\2");
	notice(GSNick, u->nick, " ");
	notice(GSNick, u->nick, "Terminates a 'ghost' IRC session using your nick.  A");
	notice(GSNick, u->nick, "'ghost' session is one which is not actually connected,");
	notice(GSNick, u->nick, "but which the IRC server believes is still online for one");
	notice(GSNick, u->nick, "reason or another.  Typically, this happens if your");
	notice(GSNick, u->nick, "computer crashes or your Internet or modem connection");
	notice(GSNick, u->nick, "goes down while you're on IRC.");
	notice(GSNick, u->nick, " ");
	notice(GSNick, u->nick, "In order to use the \2GHOST\2 command for a nick, your");
	notice(GSNick, u->nick, "current address as shown in /WHOIS must be on that nick's");
	notice(GSNick, u->nick, "access list, you must be identified and in the group of");
	notice(GSNick, u->nick, "that nick, or you must supply the correct password for");
	notice(GSNick, u->nick, "the nickname.");
	notice(GSNick, u->nick, " ");
	notice(GSNick, u->nick, "Additional Commands:");
	notice(GSNick, u->nick, " ");
	notice(GSNick, u->nick, "/msg %s JOIN #channel", GSNick);
	notice(GSNick, u->nick, "/msg %s PART #channel", GSNick);
	notice(GSNick, u->nick, " ");
	notice(GSNick, u->nick, "Note: join/part command is limited to \2IRC Operators\2 only.");
	return MOD_CONT;
}

int do_reload(int argc, char **argv)
{
	if (GSNick)
		free(GSNick);
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

    load_config();
    return MOD_CONT;
}

int load_config(void)
{
	int i;
	int retval = 1;

	Directive confvalues[][1] = {
		{{ "GSNick", {{ PARAM_STRING, PARAM_RELOAD, &GSNick }} }},
		{{ "GSIdent", {{ PARAM_STRING, PARAM_RELOAD, &GSIdent }} }},
		{{ "GSHost", {{ PARAM_STRING, PARAM_RELOAD, &GSHost }} }},
		{{ "GSReal", {{ PARAM_STRING, PARAM_RELOAD, &GSReal }} }},
		{{ "GSModes", {{ PARAM_STRING, PARAM_RELOAD, &GSModes }} }},
		{{ "GSQuitMsg", {{ PARAM_STRING, PARAM_RELOAD, &GSQuitMsg }} }},
        {{ "GSAutoJoin", {{ PARAM_SET, PARAM_RELOAD, &GSAutoJoin }} }},
	};

    for (i = 0; i < 7; i++)
        moduleGetConfigDirective(confvalues[i]);

    if (!GSNick) {
		alog("ircd_ghostserv: Config Error: \2GSNick\2 missing");
		retval = 0;
	}

    if (!GSIdent) {
        alog("ircd_ghostserv: Config Error: \2GSIdent\2 missing");
		retval = 0;
	}

    if (!GSReal) {
        alog("ircd_ghostserv: Config Error: \2GSReal\2 missing");
		retval = 0;
	}
	return retval;
}

int do_ghost(User *u)
{
	User *u2;
	NickAlias *na;
	char *buf = moduleGetLastBuffer();
	char *nick = myStrGetToken(buf, ' ', 0);
    char *pass = myStrGetToken(buf, ' ', 1);
   
	if (nickIsServices(nick, 1)) {
		notice(GSNick, u->nick, "Are you Serious? You can't ghost services!");
		if (nick) free(nick);
		if (pass) free(pass);
		return MOD_STOP;
    }

    if (!nick || !pass) {
		notice(GSNick, u->nick, "Syntax: \2GHOST \037nickname\037 [\037password\037]\2");
		notice(GSNick, u->nick, "\2/msg %s HELP\2 for more information.", GSNick);
    } else if (!(u2 = finduser(nick))) {
		notice_lang(GSNick, u, NICK_X_NOT_IN_USE, nick);
	} else if (!(na = u2->na)) {
		notice_lang(GSNick, u, NICK_X_NOT_REGISTERED, nick);
	} else if (na->status & NS_VERBOTEN) {
		notice_lang(GSNick, u, NICK_X_FORBIDDEN, na->nick);
	} else if (na->nc->flags & NI_SUSPENDED) {
		notice_lang(GSNick, u, NICK_X_SUSPENDED, na->nick);
	} else if (stricmp(nick, u->nick) == 0) {
		notice_lang(GSNick, u, NICK_NO_GHOST_SELF);
	} else if (pass) {
		int res = enc_check_password(pass, na->nc->pass);
		if (res == 1) {
			char mbuf[NICKMAX + 32];
			snprintf(mbuf, sizeof(mbuf), "Killed (Ghost nick \2%s\2 busted by %s)", nick, u->nick);
			if (LimitSessions) {
				del_session(u2->host);
			}
			kill_user(GSNick, nick, mbuf);
			notice_lang(GSNick, u, NICK_GHOST_KILLED, nick);
        } else {
			notice_lang(GSNick, u, ACCESS_DENIED);
            if (res == 0) {
				alog("%s: GHOST: invalid password for %s by %s!%s@%s",
					GSNick, nick, u->nick, u->username, u->host);
				bad_password(u);
			}
		}
	} else {
		int res = enc_check_password(pass, na->nc->pass);
		if (group_identified(u, na->nc) || (!(na->nc->flags & NI_SECURE) && is_on_access(u, na->nc))) {
			char mbuf[NICKMAX + 32];
			snprintf(mbuf, sizeof(mbuf), "Killed (Ghost nick \2%s\2 busted by %s)", nick, u->nick);
			if (LimitSessions) {
				del_session(u2->host);
			}
			kill_user(GSNick, nick, mbuf);
			notice_lang(GSNick, u, NICK_GHOST_KILLED, nick);
        } else {
			notice_lang(GSNick, u, ACCESS_DENIED);
			if (res == 0) {
                alog("%s: GHOST: invalid password for %s by %s!%s@%s",
					GSNick, nick, u->nick, u->username, u->host);
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
