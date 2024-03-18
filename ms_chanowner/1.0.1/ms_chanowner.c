#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: ms_chanowner.c v1.0.1 03-02-2008 n00bie $"

/*
===================================================================================
= Module	: ms_chanowner.c
= Version	: 1.0.1
= Author	: n00bie [n00bie@rediffmail.com]
= Date		: 10th August, 2007
= Update	: 3rd Feb, 2008
===================================================================================
= Description: This module will send a memo-text to all channel owner.
=
= Providing command for Services Admin:
= /msg MemoServ CHANOWNER memo-text
= /msg MemoServ HELP CHANOWNER
= 
= Tested: Anope-1.7.21
====================================================================================
= This program is free software; you can redistribute it and/or modify it under the
= terms of the GNU General Public License as published by the Free Software
= Foundation; either version 1, or (at your option) any later version. 

= This program is distributed in the hope that it will be useful, but WITHOUT ANY
= WARRANTY; without even the implied warranty of MERCHANTABILITY or 
= FITNESS for a PARTICULAR PURPOSE.  
= See the GNU General Public License for more details.
=====================================================================================
= Changelog:
= v1.0.0	- First Public Release
= v1.0.1	- • Minor code update • Added Multi-lang support
=====================================================================================
= Original Ideas by Keeper (ms_coowner.c)
= This module have no configurable option.
*/

#define LANG_NUM_STRINGS	4
#define CHANOWNER_HELP		0
#define CHANOWNER_SYNTAX	1
#define CHANOWNER_HELP_CMD	2
#define CHANOWNER_SENT		3

int ms_send_chanowner(User *u);
int do_help_send_chanowner(User *u);
void myMemoServHelp(User *u);
void mAddLanguages(void);
int AnopeInit(int argc, char **argv)
{
	Command *c;
	int status = 0;
	c = createCommand("CHANOWNER", ms_send_chanowner, is_services_admin, -1, -1, -1, -1, -1);
	status = moduleAddCommand(MEMOSERV, c, MOD_TAIL);
	moduleAddAdminHelp(c, do_help_send_chanowner);
	moduleAddRootHelp(c, do_help_send_chanowner);
	moduleSetMemoHelp(myMemoServHelp);
	if (status != MOD_ERR_OK) {
		return MOD_STOP;
	} else {
		alog("ms_chanowner: Successfully loaded module.");
		alog("ms_chanowner: \2/msg %s HELP CHANOWNER\2", s_MemoServ);
	}
	mAddLanguages();
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
	if (is_services_admin(u)) {
		moduleNoticeLang(s_MemoServ, u, CHANOWNER_HELP);
	}
}

int do_help_send_chanowner(User *u)
{
	moduleNoticeLang(s_MemoServ, u, CHANOWNER_SYNTAX);
	notice(s_MemoServ, u->nick, " ");
	moduleNoticeLang(s_MemoServ, u, CHANOWNER_HELP_CMD);
	return MOD_CONT;
}

int ms_send_chanowner(User *u)
{
	int i, z = 1, count = 0, match_count = 0;
    NickCore *nc;
	char *buf = moduleGetLastBuffer();
	char *text = myStrGetTokenRemainder(buf, ' ', 0);
	if (readonly) {
        notice_lang(s_MemoServ, u, MEMO_SEND_DISABLED);
		if (text)
			free(text);
        return MOD_STOP;
    } else if (checkDefCon(DEFCON_NO_NEW_MEMOS)) {
        notice_lang(s_MemoServ, u, OPER_DEFCON_DENIED);
		if (text)
			free(text);
        return MOD_STOP;
    } else if (!text) {
		moduleNoticeLang(s_MemoServ, u, CHANOWNER_SYNTAX);
	} else {
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
		moduleNoticeLang(s_MemoServ, u, CHANOWNER_SENT, match_count, count);
	}
	return MOD_CONT;
}

void mAddLanguages(void)
{
    char *langtable_en_us[] = {
		/* CHANOWNER_HELP */
		"    CHANOWNER  Send a memo to all channel owner",
		/* CHANOWNER_SYNTAX */
		"Syntax: \2CHANOWNER \037memo-text\037\02",
		/* CHANOWNER_HELP_CMD */
		"Send all channel founder a memo containing \037memo-text\037.",
		/* CHANOWNER_SENT */
		"Memos to all channel founder sent. [%d/%d]"
	};
	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}

/* EOF */
