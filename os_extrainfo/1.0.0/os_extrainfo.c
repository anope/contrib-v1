#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: os_extrainfo.c v1.0.0 17-08-2007 n00bie $"

/****************************************************************************************
** Module	: os_extrainfo.c
** Version	: 1.0.0
** Author	: n00bie [n00bie@rediffmail.com]
** Release	: 17th August, 2007
**
** Description: This module will display extra info for nicknames and channels.
** The info command if given with a nickname param will display total registered, 
** forbidden, noexpired, private, suspended and unconfirmed nicknames. Likely, if 
** given with a channel param will display total registered, forbidden, noexpired, 
** private and suspended channels.
** 
** Providing command:
** /msg OperServ INFO {nickname|channel}
** /msg OperServ HELP INFO
**
** Tested: 1.7.18, 1.7.19
*****************************************************************************************
** This program is free software; you can redistribute it and/or modify it under the 
** terms of the GNU General Public License as published by the Free Software 
** Foundation; either version 1, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful, but WITHOUT ANY 
** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** for a PARTICULAR PURPOSE. See the GNU General Public License for more details.
*****************************************************************************************
** Module requested by a friend.
** This module have no configurable option.
****************************************************************************************/

int do_extrainfo(User *u);
int myOperServInfoOperHelp(User *u);
void myOperServHelp(User *u);
int AnopeInit(int argc, char **argv)
{
	Command *c;
	int status = 0;
	c = createCommand("INFO", do_extrainfo, is_services_oper, -1, -1, -1, -1, -1);
	moduleAddHelp(c, myOperServInfoOperHelp);
	moduleSetOperHelp(myOperServHelp);
	status = moduleAddCommand(OPERSERV, c, MOD_TAIL);
	if (status != MOD_ERR_OK) {
		alog("%s: os_extrainfo: Something isn't init right.", s_OperServ);
		return MOD_STOP;
	} else {
		alog("%s: os_extrainfo: Successfully loaded module.", s_OperServ);
		alog("%s: os_extrainfo: See \2/msg %s HELP INFO\2", s_OperServ, s_OperServ);
	}
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("%s: os_extrainfo: Module unloaded.", s_OperServ);
}

void myOperServHelp(User *u)
{
	notice(s_OperServ, u->nick, "    INFO        List extra info for nicknames and channels");
}

int myOperServInfoOperHelp(User *u)
{
	notice(s_OperServ, u->nick, "Syntax: \2INFO {nickname|channel}\2");
	notice(s_OperServ, u->nick, " ");
	notice(s_OperServ, u->nick, "The info command if given with a \2nickname\2 param will");
	notice(s_OperServ, u->nick, "display total registered, forbidden, noexpired, private, ");
	notice(s_OperServ, u->nick, "suspended and unconfirmed nicknames. Likely, if given with");
	notice(s_OperServ, u->nick, "a \2channel\2 param will display total registered, forbidden,");
	notice(s_OperServ, u->nick, "noexpired, private and suspended channels.");
	notice(s_OperServ, u->nick, " ");
	notice(s_OperServ, u->nick, "Limited to \2Services Operator\2.");
	return MOD_CONT;
}

int do_extrainfo(User *u)
{
	char *param = moduleGetLastBuffer();
	NickAlias *na;
	NickRequest *nr;
	ChannelInfo *ci;
	int i, count = 0, forbid = 0, noexpire = 0, private = 0, unconfirmed = 0, suspended = 0;
	if (!param) {
		notice(s_OperServ, u->nick, "Syntax: \2INFO {nickname|channel}\2");
		return MOD_CONT;
	}
	if (stricmp(param, "nickname") == 0) {
		for (i = 0; i < 1024; i++) {
			for (na = nalists[i]; na; na = na->next) {
				count++;
				if (na->status & NS_VERBOTEN) {
					forbid++;
				}
				if (na->status & NS_NO_EXPIRE) {
					noexpire++;
				}
				if (na->nc->flags & NI_PRIVATE) {
					private++;
				}
				if (na->nc->flags & NI_SUSPENDED) {
					suspended++;
				}
			}
		}
		for (i = 0; i < 1024; i++) {
			for (nr = nrlists[i]; nr; nr = nr->next) {
				unconfirmed++;
			}
		}
		notice(s_OperServ, u->nick, "Extra info for Nicknames:");
		notice(s_OperServ, u->nick, "-------------------------");
		notice(s_OperServ, u->nick, "Registered nick  : %2d", count);
		notice(s_OperServ, u->nick, "Forbidden nick   : %2d", forbid);
		notice(s_OperServ, u->nick, "Noexpired nick   : %2d", noexpire);
		notice(s_OperServ, u->nick, "Private nick     : %2d", private);
		notice(s_OperServ, u->nick, "Suspended nick   : %2d", suspended);
		notice(s_OperServ, u->nick, "Unconfirmed nick : %2d", unconfirmed);
		notice(s_OperServ, u->nick, "-------------------------");
	} else if (stricmp(param, "channel") == 0) {
		for (i = 0; i < 256; i++) {
			for (ci = chanlists[i]; ci; ci = ci->next) {
				count++;
				if (ci->flags & CI_VERBOTEN) {
					forbid++;
				}
				if (ci->flags & CI_PRIVATE) {
							private++;
				}
				if (ci->flags & CI_NO_EXPIRE) {
					noexpire++;
				}
				if (ci->flags & CI_SUSPENDED) {
					suspended++;
				}
			}
		}
		notice(s_OperServ, u->nick, "Extra info for Channels:");
		notice(s_OperServ, u->nick, "-------------------------");
		notice(s_OperServ, u->nick, "Registered channel  : %2d", count);
		notice(s_OperServ, u->nick, "Forbidden channel   : %2d", forbid);
		notice(s_OperServ, u->nick, "Noexpired channel   : %2d", noexpire);
		notice(s_OperServ, u->nick, "Private channel     : %2d", private);
		notice(s_OperServ, u->nick, "Suspended channel   : %2d", suspended);
		notice(s_OperServ, u->nick, "-------------------------");
	} else {
		notice(s_OperServ, u->nick, "Syntax: \2INFO {nickname|channel}\2");
	}
	if (param)
		free(param);
	return MOD_CONT;
}
/* EOF */
