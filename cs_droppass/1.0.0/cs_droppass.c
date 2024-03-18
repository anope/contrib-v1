#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: cs_droppass.c v1.0.0 12-08-2007 n00bie $"

/****************************************************************************************
** Module	: cs_droppass.c
** Version	: 1.0.0
** Author	: n00bie [n00bie@rediffmail.com]
** Release	: 12th August, 2007
**
** Description: Using this module, user must supply a channel password in order to drop
** a channel. However, Services Admins can still drop channel without a password. This
** module use 'BadPassLimit' directive from services.conf which means that if a user
** failed to identify the channel several (BadPassLimit count) times they will be killed
** by services.
** 
** Providing command:
** /msg ChanServ DROP #channel [channelpass]
**
** Tested: Anope-1.7.18, 1.7.19
*****************************************************************************************
** This program is free software; you can redistribute it and/or modify it under the 
** terms of the GNU General Public License as published by the Free Software 
** Foundation; either version 1, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful, but WITHOUT ANY 
** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** for a PARTICULAR PURPOSE. See the GNU General Public License for more details.
*****************************************************************************************
** This module have no configurable option.
****************************************************************************************/

int m_do_drop(User *u);
int myChanServHelp(User *u);
int AnopeInit(int argc, char **argv)
{
	Command *c;
	int status = 0;
	c = createCommand("DROP", m_do_drop, NULL, -1, -1, -1, -1, -1);
    status = moduleAddCommand(CHANSERV, c, MOD_HEAD);
	moduleAddHelp(c, myChanServHelp);
	if (status != MOD_ERR_OK) {
		return MOD_STOP;
	} else {
		alog("cs_droppass: Successfully loaded module.");
	}
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("cs_droppass: Module Unloaded.");
}

int myChanServHelp(User *u)
{
	if (is_services_admin(u)) {
		notice_lang(s_ChanServ, u, CHAN_SERVADMIN_HELP_DROP);
	} else {
		notice(s_ChanServ, u->nick, "Syntax: \2DROP \037channel\037 \037password\037\2");
		notice(s_ChanServ, u->nick, " ");
		notice(s_ChanServ, u->nick, "Unregisters the named channel.  Can only be used by");
		notice(s_ChanServ, u->nick, "\2channel founder\2, who must use the \2IDENTIFY\2 command first.");
	}
	return MOD_STOP;
}

int m_do_drop(User *u)
{
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	char *pass = myStrGetToken(buf, ' ', 1);
	ChannelInfo *ci;
	int is_servadmin = is_services_admin(u);
	if (readonly && !is_servadmin) {
        notice_lang(s_ChanServ, u, CHAN_DROP_DISABLED);
		if (chan)
			free(chan);
		if (pass)
			free(pass);
        return MOD_STOP;
    }
	if (!chan) {
		if (is_servadmin) {
			syntax_error(s_ChanServ, u, "DROP", CHAN_DROP_SYNTAX);
		} else {
			notice(s_ChanServ, u->nick, "Syntax: \2DROP \037channel\037 \037password\037\2");
			notice(s_ChanServ, u->nick, "\2/msg %s HELP DROP\2 for more information.", s_ChanServ);
		}
	} else if (!is_servadmin && !pass) {
		notice(s_ChanServ, u->nick, "Syntax: \2DROP \037channel\037 \037password\037\2");
		notice(s_ChanServ, u->nick, "\2/msg %s HELP DROP\2 for more information.", s_ChanServ);	
	} else if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else if (!is_servadmin && (ci->flags & CI_VERBOTEN)) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    } else if (!is_servadmin && (ci->flags & CI_SUSPENDED)) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    } else if (!is_servadmin && (ci->flags & CI_SECUREFOUNDER ? !is_real_founder(u, ci) : !is_founder(u, ci))) {
        notice_lang(s_ChanServ, u, ACCESS_DENIED);
    } else {
		if (is_servadmin) {
			if (ci->c) {
				if (ircd->regmode) {
					ci->c->mode &= ~ircd->regmode;
					anope_cmd_mode(whosends(ci), ci->name, "-r");
				}
			}
			if (ircd->chansqline && (ci->flags & CI_VERBOTEN)) {
				anope_cmd_unsqline(ci->name);
			}
			alog("%s: Channel %s dropped by %s!%s@%s (founder: %s)",
				s_ChanServ, ci->name, u->nick, u->username,
				u->host, (ci->founder ? ci->founder->display : "(none)"));
			delchan(ci);
			if (WallDrop) {
				anope_cmd_global(s_ChanServ, "\2%s\2 used DROP on channel \2%s\2", u->nick, chan);
			}
			notice_lang(s_ChanServ, u, CHAN_DROPPED, chan);
			send_event(EVENT_CHAN_DROP, 1, chan);
		} else {
			if (stricmp(pass, ci->founderpass) == 0) {
				if (ci->c) {
					if (ircd->regmode) {
						ci->c->mode &= ~ircd->regmode;
						anope_cmd_mode(whosends(ci), ci->name, "-r");
					}
				}
				if (ircd->chansqline && (ci->flags & CI_VERBOTEN)) {
					anope_cmd_unsqline(ci->name);
				}
				alog("%s: Channel %s dropped by %s!%s@%s (founder: %s)",
					s_ChanServ, ci->name, u->nick, u->username,
					u->host, (ci->founder ? ci->founder->display : "(none)"));
				delchan(ci);
				notice_lang(s_ChanServ, u, CHAN_DROPPED, chan);
				send_event(EVENT_CHAN_DROP, 1, chan);
			} else {
				notice(s_ChanServ, u->nick, "Invalid Password.");
				bad_password(u);
			}
		}
	}
	if (chan)
		free(chan);
	if (pass)
		free(pass);
	return MOD_STOP;
}

/* EOF */
