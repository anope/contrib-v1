#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: os_restrictmode.c v2.0.0 14-02-2008 n00bie $"

/*****************************************************************************
** Module	: os_restrictmode.c
** Author	: n00bie
** Version	: 2.0.0
** Update	: 14th Feb, 2008
******************************************************************************
** Description: Limit the use of OperServ MODE command on a particular
** channel.
******************************************************************************
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
******************************************************************************
** Changelog:
** v1.0.0	- First initial release
** v1.0.1	- Added configuration directive
** v2.0.0	- Added support for multiple channels
**			- First public release
******************************************************************************
** Installation/Usage:
** Copy/Paste below on services.conf
**

# RestrictOSMode <channels> [REQUIRED]
# Module: os_restrictmode
#
#	Use this to define channels where
#	the use of OperServ MODE command
#	will be restricted.
#
RestrictOSMode "#chan1 #chan2 #chan3 #etc."

# End of module: os_restrictmode
*/

char *RestrictOSMode;
char **RestrictOSModes;
int RestrictOSModeNum;
int mOSMode(User *u);
int mLoadConfig();
int mEventReload(int argc, char **argv);
int AnopeInit(int argc, char **argv)
{
	Command *c;
	EvtHook *hook = NULL;
	int status;
	hook = createEventHook(EVENT_RELOAD, mEventReload);
    status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("%s: os_restrictmode: Cannot hook to EVENT_RELOAD.", s_OperServ);
		return MOD_STOP;
	}
	c = createCommand("MODE", mOSMode, is_services_oper, -1, -1, -1, -1, -1);
	status = moduleAddCommand(OPERSERV, c, MOD_HEAD);
	if (status != MOD_ERR_OK) {
		return MOD_STOP;
	}
	if (mLoadConfig()) {
        return MOD_STOP;
    }
	moduleSetType(THIRD);
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void)
{
	int idx;

	if (RestrictOSModeNum) {
		for (idx = 0; idx < RestrictOSModeNum; idx++) {
			free(RestrictOSModes[idx]);
		}
	}

	alog("%s: Module unloaded.", s_OperServ);
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
		alog("os_restrictmode: \2ERROR\2: An error has occured while reloading the configuration file!");
	}
	return MOD_CONT;
}

int mLoadConfig(void)
{
	int idx;
	int retval = 0;
	char *s;
	Directive directivas[] = {
		{"RestrictOSMode", {{PARAM_STRING, PARAM_RELOAD, &RestrictOSMode}}},
	};
    Directive *d = &directivas[0];
	moduleGetConfigDirective(d);
	if (RestrictOSModeNum) {
		for (idx = 0; idx < RestrictOSModeNum; idx++) {
			free(RestrictOSModes[idx]);
		}
	}
	if (RestrictOSMode) {
        RestrictOSModeNum = 0;
        s = strtok(RestrictOSMode, " ");
        do {
            if (s) {
                RestrictOSModeNum++;
                RestrictOSModes = realloc(RestrictOSModes, sizeof(char *) * RestrictOSModeNum);
                RestrictOSModes[RestrictOSModeNum - 1] = sstrdup(s);
            }
        } while ((s = strtok(NULL, " ")));
	} else {
		alog("os_restrictmode: Error: \2RestrictOSMode\2 is missing or not defined on services configuration file!");
		retval = 1;
	}
	return retval;
}

int mOSMode(User *u)
{
	int idx;
	Channel *c;
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	if (!chan) {
		syntax_error(s_OperServ, u, "MODE", OPER_MODE_SYNTAX);
		return MOD_STOP;
	}
	if (!(c = findchan(chan))) {
		notice_lang(s_OperServ, u, CHAN_X_NOT_IN_USE, chan);
		free(chan);
		return MOD_STOP;
	}
	for (idx = 0; idx < RestrictOSModeNum; idx++) {
        if (stricmp(chan, RestrictOSModes[idx]) == 0) {
			anope_cmd_notice(s_OperServ, u->nick, "This command is useless in %s.", chan);
			free(chan);
			return MOD_STOP;
		}
    }
	if (chan)
		free(chan);
	return MOD_CONT;
}

/* EOF */
