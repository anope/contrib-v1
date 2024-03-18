#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "1.0.0"

/***********************************************************************************
** Module: bs_uptime.c
** Author: n00bie
** Version: 1.0.0
** Released: Thursday June 14, 2007
** Description: BotServ fantasy command ' !uptime ' providing services uptime.
**
** Usage: !uptime
** Tested: Anope-1.7.18, 1.7.19, Unreal3.2.6, FreeBSD/Windows
*************************************************************************************
** This program is free software; you can redistribute it and/or modify it under the
** terms of the GNU General Public License as published by the Free Software
** Foundation; either version 1, or (at your option) any later version. 
**
** This program is distributed in the hope that it will be useful, but WITHOUT ANY
** WARRANTY; without even the implied warranty of MERCHANTABILITY or 
** FITNESS for a PARTICULAR PURPOSE.  
** See the GNU General Public License for more details.
*************************************************************************************
*/

int do_fantasy(int argc, char **argv);
int AnopeInit(int argc, char **argv)
{
	EvtHook *hook = NULL;
	int status = 0;
	hook = createEventHook(EVENT_BOT_FANTASY, do_fantasy);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("%s: Unable to bind to EVENT_BOT_FANTASY.", s_BotServ);
		alog("%s: Unloading module.", s_BotServ);
		return MOD_STOP;
	} else {
		alog("%s: Successfully hooked to EVENT_BOT_FANTASY.", s_BotServ);
	}
	hook = createEventHook(EVENT_BOT_FANTASY_NO_ACCESS, do_fantasy);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("%s: Unable to bind to EVENT_BOT_FANTASY_NO_ACCESS.", s_BotServ);
		alog("%s: Unloading module.", s_BotServ);
		return MOD_STOP;
	} else {
		alog("%s: Successfully hooked to EVENT_BOT_FANTASY_NO_ACCESS.", s_BotServ);
	}
	alog("%s: Success loading module.", s_BotServ);
	alog("%s: New fantasy command: \2!uptime\2", s_BotServ);
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    return MOD_CONT;
}

void AnopeFini(void)
{
	alog("%s: bs_uptime: Module Unloaded.", s_BotServ);
}

int do_fantasy(int ac, char **av)
{
    User *u;
    ChannelInfo *ci;
    Channel *c;
	if (ac < 3) {
        return MOD_CONT;
    }
	if (!(ci = cs_findchan(av[2])))
		return MOD_CONT;
	if (!(u = finduser(av[1])))
		return MOD_CONT;
	if (!(c = findchan(ci->name)))
		return MOD_CONT;
	if (stricmp(av[0], "uptime") == 0) {
		time_t uptime = time(NULL) - start_time;
		int days = uptime / 86400, hours = (uptime / 3600) % 24, mins = (uptime / 60) % 60, secs = uptime % 60;
		if (days > 1) {
			anope_cmd_privmsg(whosends(ci), ci->name, "Services up for %d days, %02d:%02d", days, hours, mins, secs);
		} else if (days == 1) {
			anope_cmd_privmsg(whosends(ci), ci->name, "Services up for %d day, %02d:%02d", days, hours, mins, secs);
		} else {
			if (hours > 1) {
				if (mins != 1) {
					if (secs != 1) {
						anope_cmd_privmsg(whosends(ci), ci->name, "Services up for %d hours, %d minutes", hours, mins, secs);
					} else {
						anope_cmd_privmsg(whosends(ci), ci->name, "Services up for %d hours, %d minutes", hours, mins, secs);
					}
				} else {
					if (secs != 1) {
						anope_cmd_privmsg(whosends(ci), ci->name, "Services up for %d hours, %d minute", hours, mins, secs);
					} else {
						anope_cmd_privmsg(whosends(ci), ci->name, "Services up for %d hours, %d minute", hours, mins, secs);
					}
				}
			} else if (hours == 1) {
				if (mins != 1) {
					if (secs != 1) {
						anope_cmd_privmsg(whosends(ci), ci->name, "Services up for %d hour, %d minutes", hours, mins, secs);
					} else {
						anope_cmd_privmsg(whosends(ci), ci->name, "Services up for %d hour, %d minutes", hours, mins, secs);
					}
				} else {
					if (secs != 1) {
						anope_cmd_privmsg(whosends(ci), ci->name, "Services up for %d hour, %d minute", hours, mins, secs);
					} else {
						anope_cmd_privmsg(whosends(ci), ci->name, "Services up for %d hour, %d minute", hours, mins, secs);
					}
				}
			} else {
				if (mins != 1) {
					if (secs != 1) {
						anope_cmd_privmsg(whosends(ci), ci->name, "Services up for %d minutes, %d seconds", mins, secs);
					} else {
						anope_cmd_privmsg(whosends(ci), ci->name, "Services up for %d minutes, %d second", mins, secs);
					}
				} else {
					if (secs != 1) {
						anope_cmd_privmsg(whosends(ci), ci->name, "Services up for %d minute, %d seconds", mins, secs);
					} else {
						anope_cmd_privmsg(whosends(ci), ci->name, "Services up for %d minute, %d second", mins, secs);
					}
				}
			}
		}
	}
	return MOD_CONT;
}

/* EOF */	