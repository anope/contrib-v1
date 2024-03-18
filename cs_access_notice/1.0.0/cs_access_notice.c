#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: cs_access_notice.c v1.0.0 29-02-2008 n00bie $"

/*
************************************************************************************************
** Module	: cs_access_notice.c
** Version	: 1.0.0
** Author	: n00bie (n00bie@rediffmail.com)
** Release	: 29th February, 2008
************************************************************************************************
** Description:
**
** Whenever a user is added to the channel access list or, his access being changed or his
** access deleted from the channel access list, ChanServ will send them a notice informing about
** their access status (whether (s)he is added, changed or deleted, by whom, which channel and at
** what levels). If the user is offline during the events, a memo will be sent to the user 
** informing about the events.
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
** This module have 1 configurable option.
** Copy/Paste below on services.conf

# SendMemoEventAccess [OPTIONAL]
# Module: cs_access_notice
#
#	Define this to send memo to an offline user whenever his/her
#	access gets added, changed or deleted on a channel.
#
SendMemoEventAccess

*/

void my_memo(User *u, char *nick, char *fmt, ...);
int SendMemoEventAccess;
int mLoadConfig(void);
int mEventReload(int argc, char **argv);
int mEventAccessAdd(int argc, char **argv);
int mEventAccessChange(int argc, char **argv);
int mEventAccessDel(int argc, char **argv);
int AnopeInit(int argc, char **argv)
{
	EvtHook *hook = NULL;
	int status;
	hook = createEventHook(EVENT_ACCESS_ADD, mEventAccessAdd);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("cs_access_notice: Cannot hook to EVENT_ACCESS_ADD.");
		return MOD_STOP;
	}
	hook = createEventHook(EVENT_ACCESS_CHANGE, mEventAccessChange);
	if (status != MOD_ERR_OK) {
		alog("cs_access_notice: Cannot hook to EVENT_ACCESS_CHANGE.");
		return MOD_STOP;
	}
	status = moduleAddEventHook(hook);
	hook = createEventHook(EVENT_ACCESS_DEL, mEventAccessDel);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("cs_access_notice: Cannot hook to EVENT_ACCESS_DEL.");
		return MOD_STOP;
	}
	hook = createEventHook(EVENT_RELOAD, mEventReload);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("cs_access_notice: Cannot hook to EVENT_RELOAD.");
		return MOD_STOP;
	}
	mLoadConfig();
	alog("cs_access_notice: module loaded and active.");
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("cs_access_notice: module unloaded.");
}

int mLoadConfig(void)
{
    Directive d[] = {
        {"SendMemoEventAccess", {{PARAM_SET, PARAM_RELOAD, &SendMemoEventAccess}}},
    };
    moduleGetConfigDirective(d);
    return MOD_CONT;
}

int mEventReload(int argc, char **argv)
{
	mLoadConfig();
	return MOD_CONT;
}

int mEventAccessAdd(int argc, char **argv)
{
	User *u, *u2; 
	if (argc < 3) {
		return MOD_CONT;
	}
	if (!(u = finduser(argv[1]))) {
		return MOD_CONT;
	}
	if (stricmp(u->nick, argv[2]) == 0) {
		return MOD_CONT;
	}
	if (!(u2 = finduser(argv[2]))) {
		if (SendMemoEventAccess)
			my_memo(u, argv[2], "[auto memo] You were added to the access list of %s by \2%s\2 at the level of \2%s\2", argv[0], u->nick, argv[3]);
	} else {
		anope_cmd_notice(s_ChanServ, u2->nick, "You were added to the access list of %s by \2%s\2 at the level of \2%s\2", argv[0], u->nick, argv[3]);
	}
	return MOD_CONT;
}

int mEventAccessChange(int argc, char **argv)
{
	User *u, *u2;
	if (argc < 3) {
		return MOD_CONT;
	}
	if (!(u = finduser(argv[1]))) {
		return MOD_CONT;
	}
	if (stricmp(u->nick, argv[2]) == 0) {
		return MOD_CONT;
	}
	if (!(u2 = finduser(argv[2]))) {
		if (SendMemoEventAccess)
			my_memo(u, argv[2], "[auto memo] \2%s\2 change your access level on %s to level \2%s\2", argv[1], argv[0], argv[3]);
	} else {
		anope_cmd_notice(s_ChanServ, u2->nick, "\2%s\2 change your access level on %s to level \2%s\2", argv[1], argv[0], argv[3]);
	}
	return MOD_CONT;
}

int mEventAccessDel(int argc, char **argv)
{
	User *u, *u2;
	if (argc < 2) {
		return MOD_CONT;
	}
	if (!(u = finduser(argv[1]))) {
		return MOD_CONT;
	}
	if (stricmp(u->nick, argv[2]) == 0) {
		return MOD_CONT;
	}
	if (!(u2 = finduser(argv[2]))) {
		if (SendMemoEventAccess)
			my_memo(u, argv[2], "[auto memo] You were deleted from the access list of %s by \2%s\2", argv[0], argv[1]);
	} else {
		anope_cmd_notice(s_ChanServ, u2->nick, "You were deleted from the access list of %s by \2%s\2", argv[0], argv[1]);
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

/* EOF */
