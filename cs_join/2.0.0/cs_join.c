#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: cs_join.c v2.0.0 09-09-2007 n00bie $"

/*
************************************************************************************************
** Module	: cs_join.c
** Version	: 2.0.0
** Author	: n00bie (n00bie@rediffmail.com)
** Release	: 11th August, 2006
** Update	: 09th September, 2007
************************************************************************************************
** <Description>
**		This module will make ChanServ JOIN / PART a channel & will automatically
**		set correct modes for itself.
**
**		This module is designed for ppl who wanted to have @ChanServ sits on their
**		channel along with a BotServ @Bots; and without the need of using ugly RAW commands for
**		making ChanServ join a channel.
**
**		It is recommended not to use this module with cs_fantasy.c or
**		cs_inhabitregistered.c loaded as those modules already does the job ;)
**
**		Also, when using this module i'd like to suggest the SYMBIOSIS mode turned off
**		(if there is a bot assigned on a channel). See /BotServ HELP SET SYMBIOSIS for reasons.
** </Description>
**
** NOTE: Only channel founder and services admin can use this command.
************************************************************************************************
** Tested: 1.7.19, Unreal3.2.7
**
** Providing Command:
**
** /msg ChanServ HELP JOIN|PART
** /msg ChanServ JOIN #channel
** /msg ChanServ PART #channel
**
************************************************************************************************
** This program is free software; you can redistribute it and/or modify it under the
** terms of the GNU General Public License as published by the Free Software
** Foundation; either version 1, or (at your option) any later version. 
**
** This program is distributed in the hope that it will be useful, but WITHOUT ANY
** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS for
** a PARTICULAR PURPOSE. THIS MODULE IS DISTRIBUTED 'AS IS'. NO WARRANTY OF ANY
** KIND IS EXPRESSED OR IMPLIED. YOU USE AT YOUR OWN RISK. THE MODULE AUTHOR WILL 
** NOT BE RESPONSIBLE FOR DATA LOSS, DAMAGES, LOSS OF PROFITS OR ANY OTHER KIND OF 
** LOSS WHILE USING OR MISUSING THIS MODULE. 
**
** See the GNU General Public License for more details.
************************************************************************************************
** Changelog:
**
** v1.0.0 - First public release
**
** v1.0.2 - 1) Removed strtok();
**          2) Fixed mem leak
**
** v1.0.3 - Changed mode +oq/+oa/+o to correct modes depending on the IRCd
**
** v2.0.0 - 1) Cleaned up minor codes
**          2) Added check for EVENT_PART_CHANNEL to prevent from an invite bug :o
**          3) Added and will now respect 'BSMinUsers' directives from services.conf ;)
**          4) Added command support for Services Admin (suggested by Katsklaw) ^^
**
** Still on the TODO - CS_JOIN_ALREADY :">
************************************************************************************************
** This module have no configurable option.
*/

int do_cs_join(User *u);
int do_cs_part(User *u);
int do_cs_help_join(User *u);
int do_cs_help_part(User *u);
int do_kick_rejoin(char *source, int ac, char **av);
int do_cs_check(int ac, char **av);
void myChanServHelp(User *u);
int AnopeInit(int argc, char **argv)
{
	EvtHook *hook = NULL;
	Message *msg = NULL;
	Command *c;
	int status = 0;
	c = createCommand("JOIN", do_cs_join, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, do_cs_help_join);
	moduleSetChanHelp(myChanServHelp);
	status = moduleAddCommand(CHANSERV, c, MOD_TAIL);
	if (status == MOD_ERR_OK) {
		alog("%s: cs_join: Added command 'JOIN", s_ChanServ);
	} else {
		return MOD_STOP;
	}
	c = createCommand("PART", do_cs_part, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, do_cs_help_part);
	status = moduleAddCommand(CHANSERV, c, MOD_TAIL);
	if (status == MOD_ERR_OK) {
		alog("%s: cs_join: Added command 'PART'", s_ChanServ);
	} else {
		return MOD_STOP;
	}
	hook = createEventHook(EVENT_PART_CHANNEL, do_cs_check);
    status = moduleAddEventHook(hook);
	if (status == MOD_ERR_OK) {
		alog("%s: cs_join: Successfully hooked to EVENT_PART_CHANNEL", s_ChanServ);
	} else {
		alog("%s: cs_join: Unable to bind to EVENT_PART_CHANNEL", s_ChanServ);
        return MOD_STOP;
    }
	msg = createMessage("KICK", do_kick_rejoin);
	status = moduleAddMessage(msg, MOD_TAIL);
	alog("%s: cs_join: Module loaded and active.", s_ChanServ);
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

int do_cs_join(User *u)
{
	ChannelInfo *ci;
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	if (!chan) {
		notice(s_ChanServ, u->nick, "You did not specify a channel to join.");
		notice(s_ChanServ, u->nick, "Syntax: \2JOIN\2 \037#CHANNEL\037");
	} else if (!(ci = cs_findchan(chan))) {
		notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
	} else if (!nick_identified(u)) {
		notice_lang(s_ChanServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
	} else if ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED)) {
		notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
	} else if ((!is_founder(u, ci) && !is_services_admin(u))) {
		notice_lang(s_ChanServ, u, PERMISSION_DENIED);
		if (LogChannel) {
			alog("%s: access denied for \2%s\2 with command JOIN %s", s_ChanServ, u->nick, chan);
		}
	} else if (ci->c && ci->c->usercount < BSMinUsers) {
		notice_user(s_ChanServ, u, "Can join only when there are %d user(s) in %s.", BSMinUsers, chan);
	} else {
		anope_cmd_join(s_ChanServ, chan, time(NULL));
		anope_cmd_bot_chan_mode(s_ChanServ, ci->name);
	}
	if (chan)
		free(chan);
	return MOD_CONT;
}

int do_cs_part(User *u)
{
	ChannelInfo *ci;
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	if (!chan) {
		notice(s_ChanServ, u->nick, "Syntax: \2PART\2 \037#CHANNEL\037");
	} else if (!(ci = cs_findchan(chan))) {
		notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
	} else if (!nick_identified(u)) {
		notice_lang(s_ChanServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
	} else if ((!is_founder(u, ci) && !is_services_admin(u))) {
		notice_lang(s_ChanServ, u, PERMISSION_DENIED);
		if (LogChannel) {
			alog("%s: access denied for \2%s\2 with command PART %s", s_ChanServ, u->nick, chan);
		}
	} else {
		anope_cmd_part(s_ChanServ, chan, "PART command from %s", u->nick);
	}
	if (chan)
		free(chan);
	return MOD_CONT;
}

int do_kick_rejoin(char *source, int ac, char **av)
{
	if ((!(stricmp(av[1], s_ChanServ) == 0))) {
		return MOD_CONT;
	}
	if (LogChannel) {
		alog("%s got kicked from '%s' by %s (Auto re-joining)", s_ChanServ, av[0], source);
	}
	anope_cmd_join(s_ChanServ, av[0], time(NULL));
	anope_cmd_bot_chan_mode(s_ChanServ, av[0]);
	return MOD_CONT;
}

int do_cs_check(int ac, char **av)
{
	ChannelInfo *ci;
	if (ac < 5) {
		if (stricmp(av[0], EVENT_STOP)) {
			if ((ci = cs_findchan(av[2]))) {
				if (ci->c && ci->c->usercount <= BSMinUsers) {
					anope_cmd_part(s_ChanServ, ci->name, NULL);
				}
			}
		}
	}
	return MOD_CONT;
}

int do_cs_help_join(User *u)
{
	notice(s_ChanServ, u->nick, "Syntax: \2JOIN\2 \037#Channel\037");
	notice(s_ChanServ, u->nick, " ");
	notice(s_ChanServ, u->nick, "This command will make \2%s\2 join the channel you specified.", s_ChanServ);
	notice(s_ChanServ, u->nick, "Note that when %s joins the channel, it will automatically Opped", s_ChanServ);
	notice(s_ChanServ, u->nick, "itselves on the channel and will remain on the channel untill parted");
	notice(s_ChanServ, u->nick, "using the \2PART\2 command or will part the channel when there are");
	notice(s_ChanServ, u->nick, "less than \2%d\2 user(s) on the channel.", BSMinUsers);
	notice(s_ChanServ, u->nick, " ");
	notice(s_ChanServ, u->nick, "\2NOTE\2: Command is limited to Channel Founder.", s_ChanServ);
	return MOD_CONT;
}

int do_cs_help_part(User *u)
{
	notice(s_ChanServ, u->nick, "Syntax: \2PART\2 \037#Channel\037");
	notice(s_ChanServ, u->nick, " ");
	notice(s_ChanServ, u->nick, "This command will make \2%s\2 part the channel you specified.", s_ChanServ);
	return MOD_CONT;
}

void myChanServHelp(User *u)
{
	notice(s_ChanServ, u->nick, "    JOIN       Makes %s join a channel", s_ChanServ);
	notice(s_ChanServ, u->nick, "    PART       Makes %s part a channel", s_ChanServ);
}

void AnopeFini(void)
{
	alog("%s: cs_join: module unloaded.", s_ChanServ);
}

/* EOF */

