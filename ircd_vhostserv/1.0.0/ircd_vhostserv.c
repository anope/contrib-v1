#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: ircd_vhostserv.c v1.0.0 02-10-2006 n00bie $"

/* ------------------------------------------------------------------------------------
 * Name		: ircd_vhostserv.c
 * Author	: n00bie
 * Version	: 1.0.0
 * Date		: 2nd October, 2006
 * ------------------------------------------------------------------------------------
 * Description:
 * This module create a new services client/bot. The bot have its own
 * fantasy command !vhost and !hs. The bot will answer the fantasy commands ONLY
 * on its own channel.
 * ------------------------------------------------------------------------------------
 * Providing Fantasy Commands:
 *
 * !vhost	[some.vhost.here]	- set a vhost for users
 * !hs		[on|off]		- turn ON or OFF a vhost
 * ------------------------------------------------------------------------------------
 * Tested only on UnrealIRCd3.2.5, Ultimate-3.0(01) and Anope-1.7.13, 1.7.14, 1.7.15
 * This module should work on other IRCd'd too.
 * ------------------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 1, or (at your option) any later version.
 * ------------------------------------------------------------------------------------
 * This module have 8 configurable options.
 */

/*************** Start of Configuration ********************/
/************* NOTE: All fields are required ***************/

// Define your Network vHost channel
#define HostChan "#vHost"

// Define what nick the Bot will have
// Do NOT use HostServ as your bot nick
#define s_vHostServ "vHostServ"

// Define your Bot Hostname/vhost
#define BotHost "MyNet.IRC.Network.HostBot"

// Define your Bot Identd
#define BotIdent "Bot"

// Define your Bot Realname
#define BotReal "Don't PM me honey! I'm just a Bot!"

// Define what user modes the bot will have
#define BotModes "+SqB"

// Define the Bot quit message
#define BotQmsg "Servicing Off! I'll be back!"

// Define whether the Bot will automatically join
// the network services log channel
// 0 = The Bot will not join services log channel
// 1 = The Bot will automatically join services log channel
#define BotAutoJoin 1

/*************** End of Configuration ***********************/
/***** Do NOT EDIT ANYTHING BELOW UNLESS YOU KNOW ***********/
/************* WHAT YOU ARE DOING! **************************/
CommandHash *Vhostserv_cmdTable[MAX_CMD_HASH];
int do_privmsg(char *source, int ac, char **av);
int do_kill_rejoin(char *source, int ac, char **av);
int do_help(User *u);
void addMessageList(void);
void delClient(char *nick);
void Vhostserv(User *u, char *buf);
void Vhostchanmsgs(User *u, ChannelInfo *ci, char *buf);
int AnopeInit(int argc, char **argv)
{
	Message *msg = NULL;
	int status;
	msg = createMessage("PRIVMSG", do_privmsg);
	status = moduleAddMessage(msg, MOD_HEAD);
	msg = createMessage("KILL", do_kill_rejoin);
	status = moduleAddMessage(msg, MOD_HEAD);
	if (status == MOD_ERR_OK) {
		kill_user(NULL, s_vHostServ, "This nick is reserved for services");
		anope_cmd_bot_nick(s_vHostServ, BotIdent, BotHost, BotReal, BotModes);
		addMessageList();
	}
	alog("ircd_vhostserv: Successfully loaded \2%s\2 module!", s_vHostServ);
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	if (HostChan) {
		anope_cmd_join(s_vHostServ, HostChan, time(NULL));
		if (!stricmp(IRCDModule, "inspircd") || !stricmp(IRCDModule, "plexus") ||
			!stricmp(IRCDModule, "ptlink") || !stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "ultimate2") || !stricmp(IRCDModule, "unreal32") ||
			!stricmp(IRCDModule, "viagra")) {
				anope_cmd_mode(s_vHostServ, HostChan, "+ao %s %s", s_vHostServ, s_vHostServ);
		} else {
			anope_cmd_mode(s_vHostServ, HostChan, "+o %s", s_vHostServ);
		}
	}
	if (BotAutoJoin < 1) {
		return MOD_CONT;
	} else {
		anope_cmd_join(s_vHostServ, LogChannel, time(NULL));
		if (!stricmp(IRCDModule, "inspircd") || !stricmp(IRCDModule, "plexus") ||
			!stricmp(IRCDModule, "ptlink") || !stricmp(IRCDModule, "inspircd") ||
			!stricmp(IRCDModule, "ultimate2") || !stricmp(IRCDModule, "unreal32") ||
			!stricmp(IRCDModule, "viagra")) {
				anope_cmd_mode(s_vHostServ, LogChannel, "+ao %s %s", s_vHostServ, s_vHostServ);
		} else {
			anope_cmd_mode(s_vHostServ, LogChannel, "+o %s", s_vHostServ);
		}
	}
	return MOD_CONT;
}

void delClient(char *nick)
{
	anope_cmd_quit(s_vHostServ, "Quit: %s", BotQmsg);
}

void AnopeFini(void)
{
	alog("ircd_vhostserv: Module Unloaded.");
	delClient(s_vHostServ);
}

int do_kill_rejoin(char *source, int ac, char **av)
{
	if (ac != 2) {
		return MOD_STOP;
	}
	if (stricmp(av[0], s_vHostServ) == 0) {
		anope_cmd_bot_nick(s_vHostServ, BotIdent, BotHost, BotReal, BotModes);
		if (HostChan) {
			anope_cmd_join(s_vHostServ, HostChan, time(NULL));
			if (!stricmp(IRCDModule, "inspircd") || !stricmp(IRCDModule, "plexus") ||
				!stricmp(IRCDModule, "ptlink") || !stricmp(IRCDModule, "inspircd") ||
				!stricmp(IRCDModule, "ultimate2") || !stricmp(IRCDModule, "unreal32") ||
				!stricmp(IRCDModule, "viagra")) {
					anope_cmd_mode(s_vHostServ, HostChan, "+ao %s %s", s_vHostServ, s_vHostServ);
			} else {
				anope_cmd_mode(s_vHostServ, HostChan, "+o %s", s_vHostServ);
			}
		}
		if (BotAutoJoin < 1) {
			return MOD_CONT;
		} else {
			anope_cmd_join(s_vHostServ, LogChannel, time(NULL));
			if (!stricmp(IRCDModule, "inspircd") || !stricmp(IRCDModule, "plexus") ||
				!stricmp(IRCDModule, "ptlink") || !stricmp(IRCDModule, "inspircd") ||
				!stricmp(IRCDModule, "ultimate2") || !stricmp(IRCDModule, "unreal32") ||
				!stricmp(IRCDModule, "viagra")) {
					anope_cmd_mode(s_vHostServ, LogChannel, "+ao %s %s", s_vHostServ, s_vHostServ);
			} else {
				anope_cmd_mode(s_vHostServ, LogChannel, "+o %s", s_vHostServ);
			}
		}
	}
	return MOD_CONT;
}

int do_help(User *u)
{
	notice(s_vHostServ, u->nick, "Syntax: \2!vhost\2 \037Your.vHost.Here\037");
	notice(s_vHostServ, u->nick, " ");
	notice(s_vHostServ, u->nick, "Note that the \2!vhost\2 fantasy command");
	notice(s_vHostServ, u->nick, "works ONLY on \037%s\037 channel.", HostChan);
	notice(s_vHostServ, u->nick, " ");
	notice(s_vHostServ, u->nick, "Syntax: \2!hs ON|OFF\2");
	notice(s_vHostServ, u->nick, " ");
	notice(s_vHostServ, u->nick, "Activate or Deactivate your vHost.");
	return MOD_CONT;
}

int do_privmsg(char *source, int ac, char **av)
{
	ChannelInfo *ci;
	User *u;
	char *s;
	if (ac != 2) {
		return MOD_CONT;
	}
	if (!(u = finduser(source))) {
		return MOD_CONT;
	}
	if (!stricmp(HostChan, av[0])) {
		if (s_vHostServ && (ci = cs_findchan(HostChan))) {
			if (!(ci->flags & CI_VERBOTEN) && ci->c && s_vHostServ) {
				Vhostchanmsgs(u, ci, av[1]);
			}
		}
	}
	s = strchr(av[0], '@');
	if (s) {
		*s++ = 0;
		if (stricmp(s, ServerName) != 0) {
			return MOD_CONT;
		}
	}
	if ((stricmp(av[0], s_vHostServ)) == 0) {
		Vhostserv(u, av[1]);
		return MOD_CONT;
	} else {
		return MOD_CONT;
	}
	return MOD_CONT;
}

void addMessageList(void)
{
	Command *c;
	c = createCommand("HELP", do_help, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(Vhostserv_cmdTable, c, MOD_HEAD);
}

void Vhostserv(User * u, char *buf)
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
}

void Vhostchanmsgs(User *u, ChannelInfo *ci, char *buf)
{
	char *cmd = myStrGetToken(buf, ' ', 0);
	char *param = myStrGetToken(buf, ' ', 1);
	char *nick = u->nick;
	char *hostmask = smalloc(HOSTMAX);
	NickAlias *na;
	int32 tmp_time;
	char *s;
	char *vHost;
	char *vIdent = NULL;

	if (!stricmp(cmd, "!vhost")) {
		if (!param) {
			notice(s_vHostServ, u->nick, "Syntax: \2!vhost\2 \037Your.vHost.Here\037");
		} else {
			vIdent = myStrGetOnlyToken(param, '@', 0);
			if (vIdent) {
				param = myStrGetTokenRemainder(param, '@', 1);
				if (strlen(vIdent) > USERMAX - 1) {
					anope_cmd_privmsg(s_vHostServ, HostChan,
						"Error! The Ident is too long, please use an ident shorter than %d characters.", USERMAX);
					if (vIdent)
						free(vIdent);
					return;
				} else {
					for (s = vIdent; *s; s++) {
						if (!isvalidchar(*s)) {
							anope_cmd_privmsg(s_vHostServ, HostChan,
								"A vhost ident must be in the format of a valid ident.");
							if (vIdent)
								free(vIdent);
							return;
						}
					}
				}
				if (!ircd->vident) {
					anope_cmd_privmsg(s_vHostServ, HostChan, "Error! This Network does not support vIdentd.");
					if (vIdent)
						free(vIdent);
					return;
				}
			}
			if (strlen(param) < HOSTMAX - 1) {
				snprintf(hostmask, HOSTMAX - 1, "%s", param);
			} else {
				anope_cmd_privmsg(s_vHostServ, HostChan,
					"Error! The vhost is too long, please use a host shorter than \2%d\2 characters.", HOSTMAX);
				if (hostmask)
					free(hostmask);
				return;
			}
			if (!isValidHost(hostmask, 3)) {
				anope_cmd_privmsg(s_vHostServ, HostChan, "A vhost must be in the format of a valid hostmask.");
				if (hostmask)
					free(hostmask);
				return;
			}
			tmp_time = time(NULL);
			if ((na = findnick(nick))) {
				if (na->status & NS_VERBOTEN) {
					anope_cmd_privmsg(s_vHostServ, HostChan, "Cannot set vhost for forbidden nick %s", nick);
					return;
				}
				if (!nick_identified(u)) {
					notice(s_vHostServ, u->nick, "Password authentication required for that command.");
					notice(s_vHostServ, u->nick, "Retry after type \2/msg %s IDENTIFY password\2", s_NickServ);
					return;
				}
				if (vIdent && ircd->vident) {
					alog("vHost for user \2%s\2 set to \2%s@%s\2 by %s", nick, vIdent, hostmask, s_vHostServ);
				} else {
					alog("vHost for user \2%s\2 set to \2%s\2 by %s", nick, hostmask, s_vHostServ);
				}
				addHostCore(nick, vIdent, hostmask, u->nick, tmp_time);
				if (vIdent) {
					anope_cmd_privmsg(s_vHostServ, HostChan, "vHost for \2%s\2 set to \2%s@%s\2", nick, vIdent, hostmask);
					anope_cmd_privmsg(s_vHostServ, HostChan, "Type \2!hs ON\2 to activate your new vhost.");
				} else {
					anope_cmd_privmsg(s_vHostServ, HostChan, "vHost for \2%s\2 set to \2%s\2", nick, hostmask);
					anope_cmd_privmsg(s_vHostServ, HostChan, "Type \2!hs ON\2 to activate your new vhost.");
				}
			} else {
				anope_cmd_privmsg(s_vHostServ, HostChan, "\2%s\2, please register your nick first.", nick);
			}
		}
	}
	if (!stricmp(cmd, "!hs")) {
		if (!param) {
			notice(s_vHostServ, u->nick, "Syntax: \2!hs ON|OFF\2");
		} else if (!stricmp(param, "ON")) {
			if ((na = findnick(nick))) {
				if (na->status & NS_IDENTIFIED) {
					vHost = getvHost(nick);
					vIdent = getvIdent(nick);
					if (vHost == NULL) {
						notice(s_vHostServ , u->nick, "Please contact an IRCop to assigned a vHost for you.");
					} else {
						if (vIdent) {
							notice(s_vHostServ, u->nick, "Your vHost of \2%s@%s\2 is now activated.", vIdent, vHost);
						} else {
							notice(s_vHostServ, u->nick, "Your vHost of \2%s\2 is now activated.", vHost);
						}
						anope_cmd_vhost_on(u->nick, vIdent, vHost);
						set_lastmask(u);
					}
				} else {
					notice_lang(s_vHostServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
				}
			} else {
				notice_lang(s_vHostServ, u, NICK_NOT_REGISTERED);
			}
		} else if (!stricmp(param, "OFF")) {
			if ((na = findnick(nick))) {
				if (na->status & NS_IDENTIFIED) {
					vHost = getvHost(u->nick);
					vIdent = getvIdent(u->nick);
					if (vHost == NULL && vIdent == NULL) {
						notice(s_vHostServ , u->nick, "Please contact an IRCop to assigned a vHost for you.");
					} else {
						anope_cmd_vhost_off(u);
					}
				} else {
					notice_lang(s_vHostServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
				}
			} else {
				notice_lang(s_vHostServ, u, NICK_NOT_REGISTERED);
			}
		} else {
			notice(s_vHostServ, u->nick, "Syntax: \2!hs ON|OFF\2");
		}
	}
	if (param) free(param);
	if (vIdent) free(vIdent);
	if (hostmask) free(hostmask);
	return;
}

/* EOF */



