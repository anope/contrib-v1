#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: ms_chanowner.c v1.0.0 10-08-2007 n00bie $"

/*
===================================================================================
= Module: ms_chanowner.c
= Version: 1.0.0
= Author: n00bie
= Date: 10th August, 2007
=
= Description: This module will send a memo-text to all channel owner.
=
= Providing command for Services Admin:
= /msg MemoServ CHANOWNER memo-text
= /msg MemoServ HELP CHANOWNER
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
= Original Ideas by Keeper (ms_coowner.c)
= This module have no configurable option.
*/

int ms_send_chanowner(User *u);
int do_help_send_chanowner(User *u);
void myMemoServHelp(User *u);
int AnopeInit(int argc, char **argv)
{
	Command *c;
	int status = 0;
	c = createCommand("CHANOWNER", ms_send_chanowner, is_services_admin, -1, -1, -1, -1, -1);
	status = moduleAddCommand(MEMOSERV, c, MOD_TAIL);
	moduleAddHelp(c, do_help_send_chanowner);
	moduleSetMemoHelp(myMemoServHelp);
	if (status != MOD_ERR_OK) {
		return MOD_STOP;
	} else {
		alog("ms_chanowner: Successfully loaded module.");
		alog("ms_chanowner: \2/msg %s HELP CHANOWNER\2", s_MemoServ);
	}
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("ms_chanowner: Module Unloaded.");
}

void myMemoServHelp(User *u)
{
	notice(s_MemoServ, u->nick, "    CHANOWNER  Send a memo to all channel owner");
}

int do_help_send_chanowner(User *u)
{
	if (is_services_admin(u)) {
		notice(s_MemoServ, u->nick, "Syntax: \2CHANOWNER \037memo-text\037\02");
		notice(s_MemoServ, u->nick, " ");
		notice(s_MemoServ, u->nick, "Sends all channel founder a memo containing \037memo-text\037.");
		notice(s_MemoServ, u->nick, "Limited to \2Services Admin\2.");
	}
	return MOD_CONT;
}

int ms_send_chanowner(User *u)
{
	int i, z = 1, count = 0, match_count = 0;
    NickCore *nc;
	char *text = moduleGetLastBuffer();
	if (readonly) {
        notice_lang(s_MemoServ, u, MEMO_SEND_DISABLED);
		if (text)
			free(text);
        return MOD_CONT;
    } else if (checkDefCon(DEFCON_NO_NEW_MEMOS)) {
        notice_lang(s_MemoServ, u, OPER_DEFCON_DENIED);
		if (text)
			free(text);
        return MOD_CONT;
    } else if (!text) {
		notice(s_MemoServ, u->nick, "Syntax: \2CHANOWNER \037memo-text\037\02");
        return MOD_CONT;
    }
	for (i = 0; i < 1024; i++) {
		for (nc = nclists[i]; nc; nc = nc->next) {
			count++;
			if ((stricmp(u->nick, nc->display) != 0) && (nc->channelcount > 0)) {
				memo_send(u, nc->display, text, z);
				match_count++;
				free(text);
			}
		}
	}
	notice(s_MemoServ, u->nick, "Memo to all channel founder sent. [%d/%d]", match_count, count);
	return MOD_CONT;
}

/* EOF */
