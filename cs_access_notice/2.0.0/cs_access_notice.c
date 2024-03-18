#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: cs_access_notice.c v2.0.0 25-01-2009 n00bie $"
/*
************************************************************************************************
** Module	: cs_access_notice.c
** Version	: 2.0.0
** Author	: n00bie (n00bie@rediffmail.com)
** Release	: 29th February, 2008
** Update	: 25th January, 2009
************************************************************************************************
** Description:
**
** Whenever a user is added to the channel access list or, his access being changed or his
** access deleted from the channel access list, ChanServ will send them a notice informing about
** their access status (whether (s)he is added, changed or deleted, by whom, which channel and at
** what levels). If the user is offline during the events, a memo will be sent to the user 
** informing about the events (if it is enabled on services.conf).
************************************************************************************************
** Changelog:
**
** v1.0.0 - First initial release
** v2.0.0 - • The module now looks for user in the nick aliases/group list. So if a user access is
**			  added/deleted or changed the user who is ONLINE amongst the group will receive notices.
**			• Added languages support.
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
** Module made possible using the memo_send routine by Trystan
**
** This module have 1 optional configurable option.
** Copy/Paste below on services.conf

# SendMemoEventAccess [OPTIONAL]
# Module: cs_access_notice
#
#	Define this to send memo to an offline user whenever his/her
#	access gets added, changed or deleted on a channel.
#
SendMemoEventAccess

*/

#define LANG_NUM_STRINGS	6
#define LANG_MEMO_ADD		0
#define LANG_MEMO_DEL		1
#define LANG_MEMO_CHANGE	2
#define LANG_NOTICE_ADD		3
#define LANG_NOTICE_DEL		4
#define LANG_NOTICE_CHANGE	5

char *str;
void my_memo(User *u, char *nick, char *fmt, ...);
void mAddLanguages(void);
void mLoadConfig(void);
int i, user_online = 0;
int SendMemoEventAccess;
int mEventReload(int argc, char **argv);
int mEventAccessAdd(int argc, char **argv);
int mEventAccessChange(int argc, char **argv);
int mEventAccessDel(int argc, char **argv);
int AnopeInit(int argc, char **argv)
{
	EvtHook *hook = NULL;
	hook = createEventHook(EVENT_ACCESS_ADD, mEventAccessAdd);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("cs_access_notice: Cannot hook to EVENT_ACCESS_ADD.");
		return MOD_STOP;
	}
	hook = createEventHook(EVENT_ACCESS_CHANGE, mEventAccessChange);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("cs_access_notice: Cannot hook to EVENT_ACCESS_CHANGE.");
		return MOD_STOP;
	}
	hook = createEventHook(EVENT_ACCESS_DEL, mEventAccessDel);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("cs_access_notice: Cannot hook to EVENT_ACCESS_DEL.");
		return MOD_STOP;
	}
	hook = createEventHook(EVENT_RELOAD, mEventReload);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("cs_access_notice: Cannot hook to EVENT_RELOAD.");
		return MOD_STOP;
	}
	mLoadConfig();
	mAddLanguages();
	alog("cs_access_notice: module loaded and active.");
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("cs_access_notice: module unloaded.");
}

void mLoadConfig(void)
{
    Directive confvalues[][1] = {
		{{"SendMemoEventAccess", {{PARAM_SET, PARAM_RELOAD, &SendMemoEventAccess}}}},
    };
	SendMemoEventAccess = 0;
    for (i = 0; i < 1; i++)
		moduleGetConfigDirective(confvalues[i]);
}

int mEventReload(int argc, char **argv)
{
	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			mLoadConfig();
		}
	}
	return MOD_CONT;
}

int mEventAccessAdd(int argc, char **argv)
{
	User *u, *u2;
	NickAlias *na, *na2;	
	u = finduser(argv[1]);
	na = findnick(argv[2]);
	if (stricmp(u->nick, argv[2]) == 0)
		return MOD_CONT;	
	for (i = 0; i < na->nc->aliases.count; i++) {
		na2 = na->nc->aliases.list[i];
		if ((u2 = finduser(na2->nick)) && nick_identified(u2)) {
			if ((str = moduleGetLangString(u, LANG_NOTICE_ADD))) {
				anope_cmd_notice(s_ChanServ, na2->nick, str, argv[0], argv[1], argv[3]);
			}
			user_online = 1;
		}
	}
	if (!user_online) {
		if (SendMemoEventAccess) {
			if ((str = moduleGetLangString(u, LANG_MEMO_ADD))) {
				my_memo(u, argv[2], str, argv[0], argv[1], argv[3]);
			}
		}
	}
	return MOD_CONT;
}

int mEventAccessChange(int argc, char **argv)
{
	User *u, *u2;
	NickAlias *na, *na2;
	u = finduser(argv[1]);
	na = findnick(argv[2]);
	if (stricmp(u->nick, argv[2]) == 0)
		return MOD_CONT;
	for (i = 0; i < na->nc->aliases.count; i++) {
		na2 = na->nc->aliases.list[i];
		if ((u2 = finduser(na2->nick)) && nick_identified(u2)) {
			if ((str = moduleGetLangString(u, LANG_NOTICE_CHANGE))) {
				anope_cmd_notice(s_ChanServ, u2->nick, str, argv[1], argv[0], argv[3]);
			}
			user_online = 1;
		}
	}
	if (!user_online) {
		if (SendMemoEventAccess) {
			if ((str = moduleGetLangString(u, LANG_MEMO_CHANGE))) {
				my_memo(u, argv[2], str, argv[1], argv[0], argv[3]);
			}
		}
	}
	return MOD_CONT;
}

int mEventAccessDel(int argc, char **argv)
{
	User *u, *u2;
	NickAlias *na, *na2;
	u = finduser(argv[1]);
	na = findnick(argv[2]);
	if (stricmp(u->nick, argv[2]) == 0)
		return MOD_CONT;
	for (i = 0; i < na->nc->aliases.count; i++) {
		na2 = na->nc->aliases.list[i];
		if ((u2 = finduser(na2->nick)) && nick_identified(u2)) {
			if ((str = moduleGetLangString(u, LANG_NOTICE_DEL))) {
				anope_cmd_notice(s_ChanServ, u2->nick, str, argv[0], argv[1]);
			}
			user_online = 1;
		}
	}
	if (!user_online) {
		if (SendMemoEventAccess) {
			if ((str = moduleGetLangString(u, LANG_MEMO_DEL))) {
				my_memo(u, argv[2], str, argv[0], argv[1]);
			}
		}
	}
	return MOD_CONT;
}

void my_memo(User *u, char *nick, char *fmt, ...)
{
	va_list args;
	char buf[BUFSIZE];
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	memo_send(u, nick, buf, 1);
}

void mAddLanguages(void)
{
	char *langtable_en_us[] = {
		/* LANG_MEMO_ADD */
		// Eg: You were added to the access list of #Help by n00bie at the level of 10
		"[auto memo] You were added to the access list of %s by \2%s\2 at the level of \2%s\2",

		/* LANG_MEMO_DEL */
		// Eg: You were deleted from the access list of #Help by n00bie
		"[auto memo] You were deleted from the access list of %s by \2%s\2",

		/* LANG_MEMO_CHANGE */
		// Eg: n00bie change your access level on #Help to level 5
		"[auto memo] \2%s\2 change your access level on %s to level \2%s\2",

		/* LANG_NOTICE_ADD */
		"You were added to the access list of %s by \2%s\2 at the level of \2%s\2",
		/* LANG_NOTICE_DEL */
		"You were deleted from the access list of %s by \2%s\2",
		/* LANG_NOTICE_CHANGE */
		"\2%s\2 change your access level on %s to level \2%s\2"
	};
	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}

/* EOF */
