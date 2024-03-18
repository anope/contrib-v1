#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: ns_noticeunregister.c v1.0.1 20-02-2008 n00bie $"

/*
**************************************************************************************************
** Module	: ns_noticeunregister.c
** Author	: n00bie [n00bie@rediffmail.com]
** Version	: 1.0.1
** Date		: 14th Feb, 2008
** Update	: 20th Feb, 2008
**************************************************************************************************
** Original Module by SGR
** Update/Compiled for Anope 1.7.21 on request of network
**************************************************************************************************
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
**************************************************************************************************
** This module have 2 configurable option.
** Copy/paste below on services.conf

# NoticeUnreg [OPTIONAL]
# Module: ns_noticeunregister
# Define this if you want NickServ to NOTICE 
# unregistered users when they connect. If you
# enable this setting, NoticeUnregMsg is required.
#
NoticeUnreg

# NoticeUnregMsg <message> [REQUIRED]
# Define the message which will be
# noticed to unregistered users.
#
NoticeUnregMsg "Your nickname does not appear to be registered on this network. For more information on how to register and the benefits of registration, use the commands: /NickServ HELP and /NickServ HELP REGISTER"

# End of module: ns_noticeunregister
*/

char *UnregNoticeMsg;
int UnregNotice;
int mLoadConfig(void);
int mEventReload(int argc, char **argv);
int doUnregNotice(int argc, char **argv);
int AnopeInit(int argc, char **argv)
{
	EvtHook *hook;
    int status;
	hook = createEventHook(EVENT_NEWNICK, doUnregNotice);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("ns_noticeunregister: Cannot hook to EVENT_NEWNICK.");
		return MOD_STOP;
	}
	hook = createEventHook(EVENT_RELOAD, mEventReload);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("ns_noticeunregister: Cannot hook to EVENT_RELOAD.");
		return MOD_STOP;
	}
	mLoadConfig();
	alog("ns_noticeunregister: module loaded and active.");
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    return MOD_CONT;
}

void AnopeFini(void)
{
	if (UnregNoticeMsg)
		free(UnregNoticeMsg);

	alog("ns_noticeunregister: Module Unloaded.");
}

int mLoadConfig(void)
{
    int i;
	int retval = 0;
	Directive directives[][1] = {
		{{"NoticeUnreg", {{PARAM_SET, 0, &UnregNotice}}}},
		{{"NoticeUnregMsg", {{PARAM_STRING, 0, &UnregNoticeMsg}}}},
	};
    for (i = 0; i < 2; i++)
        moduleGetConfigDirective(directives[i]);

	if (!UnregNoticeMsg) {
		alog("ns_noticeunregister: Error: \2NoticeUnregMsg\2 is missing on services configuration file");
		retval = 1;
	}
	return retval;
}

int mEventReload(int argc, char **argv)
{
	int ret = 0;
    if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			ret = mLoadConfig();
        }
	}
	if (ret) {
		alog("ns_noticeunregister: \2ERROR\2: An error has occured while reloading the configuration file!");
	}
	return MOD_CONT;
}

int doUnregNotice(int argc, char **argv)
{
    User *u;
    NickAlias *na;
	if (argc < 1) {
		return MOD_CONT;
	}
	if (!(u = finduser(argv[0]))) {
		return MOD_CONT;
	} else if ((na = findnick(u->nick))) {
		return MOD_CONT;
	} else {
		if (UnregNotice && UnregNoticeMsg)
			anope_cmd_notice(s_NickServ, u->nick, "%s", UnregNoticeMsg);
	}
	return MOD_CONT;
}

/* EOF */
