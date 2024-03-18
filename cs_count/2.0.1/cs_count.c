#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: cs_count.c v2.0.1 25-02-2008 n00bie $"

/*
************************************************************************************************
** Module	: cs_count.c
** Version	: 2.0.1
** Author	: n00bie (n00bie@rediffmail.com)
** Release	: 30th August, 2007
** Update	: 25th February, 2008
************************************************************************************************
** Description:
**
** This command will display the total number of VOPs, HOPs, AOPs, SOPs, AKICKs, 
** badwords list and current users in a channel. Based on dalservices (DALnet).
************************************************************************************************
**
** Providing Command:
**
** /msg ChanServ HELP COUNT
** /msg ChanServ COUNT #channel
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
** v1.0.0 - First Public Release
** v2.0.0 - I've mistakenly put NickServ on the /CS HELP output ^^ which was now fixed (thanks Armadillo)
** v2.0.1 - Services administrators can now use the command too
************************************************************************************************
** This module have no configurable option.
*/

int myCount(User *u);
int do_count_help(User *u);
void myChanServHelp(User *u);
int AnopeInit(int argc, char **argv)
{
	Command *c;
	int status = 0;
	c = createCommand("COUNT", myCount, NULL, -1, -1, -1, -1, -1);
	status = moduleAddCommand(CHANSERV, c, MOD_TAIL);
	moduleAddHelp(c, do_count_help);
	moduleSetChanHelp(myChanServHelp);
	if (status != MOD_ERR_OK) {
		return MOD_STOP;
	} else {
		alog("cs_count: Successfully loaded module.");
		alog("cs_count: \2/msg %s HELP COUNT\2", s_ChanServ);
	}
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("cs_count: Module Unloaded.");
}

void myChanServHelp(User *u)
{
	notice(s_ChanServ, u->nick, "    COUNT      Display no. of AOPs, SOPs, AKICKs etc. in a channel");
}

int do_count_help(User *u)
{
	notice(s_ChanServ, u->nick, "Syntax: \2COUNT\2 \037#channel\037");
	notice(s_ChanServ, u->nick, " ");
	notice(s_ChanServ, u->nick, "This command will display the total number of VOPs, HOPs,");
	notice(s_ChanServ, u->nick, "AOPs, SOPs, AKICKs, badwords list and current users in a channel.");
	notice(s_ChanServ, u->nick, " ");
	notice(s_ChanServ, u->nick, "Command is limited to \2AOP\2 levels or higher.");
	return MOD_CONT;
}

int myCount(User *u)
{
	Channel *c;
	ChannelInfo *ci;
	AutoKick *akick;
	ChanAccess *access;
	int i;
	int is_servadmin = is_services_admin(u);
	int akcount = 0, total_acc = 0, total_vop = 0, total_hop = 0, total_aop = 0, total_sop = 0, bwords = 0, c_users = 0;
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	if (!chan) {
		notice(s_ChanServ, u->nick, "Syntax: \2COUNT\2 \037#channel\037");
	} else if (!nick_identified(u)) {
		notice_lang(s_ChanServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
	} else if (!(c = findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_IN_USE, chan);
    } else if (!(ci = c->ci)) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else if (ci->flags & CI_VERBOTEN) {
		notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
	} else if (ci->flags & CI_SUSPENDED) {
		notice(s_ChanServ, u->nick, "Channel \2%s\2 is currently suspended.", chan);
	} else if (!(check_access(u, ci, CA_OPDEOP) || check_access(u, ci, CA_OPDEOPME) || is_servadmin)) {
		notice_lang(s_ChanServ, u, ACCESS_DENIED);
	} else {
		struct c_userlist *cu, *next;
		for (i = 0, akick = ci->akick; i < ci->akickcount; akick++, i++) {
			if (akick->flags & AK_USED) {
				akcount++;
			}
		}
		for (i = 0, access = ci->access; i < ci->accesscount; access++, i++) {
			if (access->in_use) {
				total_acc++;
			}
		}
		for (i = 0, access = ci->access; i < ci->accesscount; access++, i++) {
			if (ci->access[i].in_use && ci->access[i].level == ACCESS_VOP) {
				total_vop++;
			} else if (ci->access[i].in_use && ci->access[i].level == ACCESS_HOP) {
				total_hop++;
			} else if (ci->access[i].in_use && ci->access[i].level == ACCESS_AOP) {
				total_aop++;
			} else if (ci->access[i].in_use && ci->access[i].level == ACCESS_SOP) {
				total_sop++; 
			} 
		}
		notice(s_ChanServ, u->nick, "*** Count for %s ***", ci->name);
		notice(s_ChanServ, u->nick, " Auto-voice      : %2d", total_vop);
		notice(s_ChanServ, u->nick, " Half-operator   : %2d", total_hop);
		notice(s_ChanServ, u->nick, " Auto-operator   : %2d", total_aop);
		notice(s_ChanServ, u->nick, " Super-operator  : %2d", total_sop);
		notice(s_ChanServ, u->nick, "          Total  : %2d", total_acc);
		notice(s_ChanServ, u->nick, "***********************");
		if (ci->bi) {
			for (i = 0; i < ci->bwcount; i++) {
				if (ci->badwords[i].in_use)
					bwords++;
			}
			notice(s_ChanServ, u->nick, " Badwords list   : %2d", bwords);
		}
		notice(s_ChanServ, u->nick, " Auto-kick list  : %2d", akcount);
		for (cu = c->users; cu; cu = next) {
			next = cu->next;
			c_users++;
		}
		notice(s_ChanServ, u->nick, " Current users   : %2d", c_users);
		notice(s_ChanServ, u->nick, "***********************");
	}
	if (chan)
		free(chan);
	return MOD_CONT;
}
/* EOF */
