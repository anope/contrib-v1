#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: ns_clist.c v2.0.0 30-07-2007 n00bie $"

/*
===================================================================================
= Module: ns_clist.c
= Version: 2.0.0
= Author: n00bie
= Date: 30th July, 2007
=
= Description: This module will display all the channels that are registered by you.
=
= Providing command:
= /msg NickServ CLIST 
= /msg NickServ HELP CLIST
= 
= Command for Services Admin:
= /msg NickServ CLIST [nickname]
=
= Tested: Anope-1.7.18, 1.7.19
====================================================================================
= This program is free software; you can redistribute it and/or modify it under the
= terms of the GNU General Public License as published by the Free Software
= Foundation; either version 1, or (at your option) any later version. 

= This program is distributed in the hope that it will be useful, but WITHOUT ANY
= WARRANTY; without even the implied warranty of MERCHANTABILITY or 
= FITNESS for a PARTICULAR PURPOSE.  
= See the GNU General Public License for more details.
=====================================================================================
= Change log:
= v1.0.0 - First public release
= v2.0.0 - Clean up minor codes and add parameter for Services Admin
=====================================================================================
= Module suggested by [dx]
*/

int do_clist(User *u);
int do_clist_help(User *u);
void myNickServHelp(User *u);
int AnopeInit(int argc, char **argv)
{
	Command *c;
	int status = 0;
	c = createCommand("CLIST", do_clist, NULL, -1, -1, -1, -1, -1);
	status = moduleAddCommand(NICKSERV, c, MOD_TAIL);
	moduleAddHelp(c, do_clist_help);
	moduleSetNickHelp(myNickServHelp);
	if (status != MOD_ERR_OK) {
		return MOD_STOP;
	} else {
		alog("ns_clist: Successfully loaded module.");
		alog("ns_clist: \2/msg %s HELP CLIST\2", s_NickServ);
	}
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("ns_clist: Module Unloaded.");
}

void myNickServHelp(User *u)
{
	notice(s_NickServ, u->nick, "    CLIST      List all channels registered by you");
}

int do_clist_help(User *u)
{
	if (is_services_admin(u)) {
		notice(s_NickServ, u->nick, "Syntax: \2CLIST [\037nickname\037]\2");
		notice(s_NickServ, u->nick, " ");
		notice(s_NickServ, u->nick, "With no parameters, lists all channels registered by you.");
		notice(s_NickServ, u->nick, "Giving a nickname parameter will lists the channels that");
		notice(s_NickServ, u->nick, "\2nickname\2 have registered. Channels that have the \037NOEXPIRE\037");
		notice(s_NickServ, u->nick, "option set will be prefixed by an exclamation mark.");
		notice(s_NickServ, u->nick, " ");
		notice(s_NickServ, u->nick, "Limited to \2Services admins\2.");
	} else {
		notice(s_NickServ, u->nick, "Syntax: \2CLIST\2");
		notice(s_NickServ, u->nick, " ");
		notice(s_NickServ, u->nick, "List all channels that are registered by you.");
		notice(s_NickServ, u->nick, "Channels that have the \037NOEXPIRE\037 option set will be");
		notice(s_NickServ, u->nick, "prefixed by an exclamation mark.");
	}
	return MOD_CONT;
}

int do_clist(User * u)
{
	
    char *nick = NULL;
	ChannelInfo *ci;
    NickAlias *na;
	int is_servadmin = is_services_admin(u);
    int i, count = 0, match_count = 0;
	if (!is_servadmin) {
		na = u->na;
	} else {
		nick = moduleGetLastBuffer();
		if (nick) {
			na = findnick(nick);
		} else {
			na = u->na;
		}
	}
	if (!nick_identified(u)) {
		notice_lang(s_NickServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
	} else if (is_servadmin && nick && !na) {
		notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, nick);
	} else if (na->status & NS_VERBOTEN) {
		notice_lang(s_NickServ, u, NICK_X_FORBIDDEN, na->nick);
	} else {
		notice(s_NickServ, u->nick, 
			nick ? "Channels that \2%s\2 have registered:" : "Channels that you have registered:", 
			na->nick);
		notice(s_NickServ, u->nick, "\037Num\037  \037Channel\037");
		for (i = 0; i < 256; i++) {
			for (ci = chanlists[i]; ci; ci = ci->next) {
				count++;
				if (na && na->nc == ci->founder) {
					match_count++;
					notice(s_NickServ, u->nick, "%3d %c%-20s", match_count, 
						((ci->flags & CI_NO_EXPIRE) ? '!' : ' '), ci->name);
				}
			}
		}
		notice(s_NickServ, u->nick, "End of list - %d/%d channels shown.", match_count, count);
	}
	/*if (nick)
		free(nick);*/
	return MOD_CONT;
}
/* EOF */
