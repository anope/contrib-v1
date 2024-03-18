/*****************************************************************************/
/* Anope Module : ircd_tssync.c : v 1.0                                      */
/* Amanda F. (amanda@anope.org)                                              */
/*                                                                           */
/* BASED ON WORK BY:														 */
/*                                                                           */
/* Anope Module : os_tssync.c : v1.0                                         */
/* Scott Seufert (katsklaw@ircmojo.net)                                      */
/*                                                                           */
/* Proxy module for Trystan, DO NOT CONTACT TRYSTAN FOR SUPPORT, YOU WILL BE */
/* IGNORED!                                                                  */
/*                                                                           */
/* OS TSSYNC                                                                 */
/*                                                                           */
/* Original work by Certus <certus@europnet.org>                             */
/*           and by <nenolod@noderebellion.net>                              */
/*                                                                           */
/* This program is free software; you can redistribute it and/or modify it   */
/* under the terms of the GNU General Public License as published by the     */
/* Free Software Foundation; either version 1, or (at your option) any later */
/* version.                                                                  */
/*                                                                           */
/*  This program is distributed in the hope that it will be useful, but      */
/* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY*/
/* or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License  */
/* for more details.                                                         */
/*                                                                           */
/*****************************************************************************/

#include "module.h"
#define AUTHOR "Perihelion"
#define VERSION "1.0"

int AutoSync = 0;
int AutoSyncTime = 0;

/* Allow us to use help :D */
int my_os_tssync_help(User *u);
void my_operserv_help(User * u);

int valid_ircd(void);
int do_sync_ts(User *u);
int sync_callback(int ac, char **av);

void load_config(void);

int AnopeInit(int argc, char **argv)
{
    Command *c;

    if (!valid_ircd()) {
        alog("[ircd_tssync] Your IRCd is not supported. This module currently only supports UnrealIRCd and ShadowIRCd.");
        return MOD_STOP;
    }

    if (!moduleMinVersion(1,7,10,810)) {
        alog("[ircd_tssync] You are using an unsupported version of Anope.");
        return MOD_STOP;
    }
    load_config();

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);

    c = createCommand("TSSYNC",do_sync_ts,is_services_admin,-1,-1,-1,-1,-1);
	moduleAddHelp(c, my_os_tssync_help);
	moduleSetOperHelp(my_operserv_help);

    if (!moduleAddCommand(OPERSERV,c,MOD_HEAD)) {
        alog("Loading module os_tssync.%s [Status: working]", MODULE_EXT);
        alog("New command: /msg operserv TSSYNC");
        if (AutoSync) {
            moduleAddCallback("sync_callback",time(NULL) + AutoSyncTime, sync_callback,0,NULL);
        }
        return MOD_CONT;
    } else {
        alog("[ircd_tssync] Unable to load ircd_tssync [Status: failed]");
        alog("[ircd_tssync] Unable to add command to OperServ's command list.");
        return MOD_STOP;
    }
}

void AnopeFini(void)
{
    alog("[ircd_tssync] Module unloaded.%s", MODULE_EXT);
}

int do_sync_ts(User *u)
{
    time_t now = time(NULL);
    if (!stricmp(IRCDModule, "shadowircd")) {
        send_cmd(NULL, "tssync %ld", (long int) now);
    } else if (!stricmp(IRCDModule, "unreal32")) {
        send_cmd(NULL, "tsctl svstime %ld", (long int) now);
    } else {
        return MOD_CONT;
    }
    alog("Time synced (TS: %ld) by %s", (long int) now, u->nick);
    anope_cmd_global(s_OperServ, "\2%s\2 used TSSYNC to sync all server's TS (TS: %ld)", u->nick, (long int) now);
    notice(s_OperServ, u->nick, "Time synced (TS: %ld)", (long int) now);
    return MOD_CONT;
}

int sync_callback(int ac, char **av)
{
    time_t now = time(NULL);

    if (!stricmp(IRCDModule, "shadowircd")) {
        send_cmd(NULL, "tssync %ld", (long int) now);
    } else if (!stricmp(IRCDModule, "unreal32")) {
        send_cmd(NULL, "tsctl svstime %ld", (long int) now);
    } else {
        return MOD_CONT;
    }
    alog("Time synced (TS: %ld)", (long int) now);
    anope_cmd_global(s_OperServ, "Automated TSSYNC (TS: %ld)", (long int) now);
    moduleAddCallback("sync_callback",time(NULL) + AutoSyncTime, sync_callback,0,NULL);
    return MOD_CONT;
}

int valid_ircd(void)
{
    if (!stricmp(IRCDModule, "unreal32")) {
        return 1;
    }
    if (!stricmp(IRCDModule, "shadowircd")) {
        return 1;
    }
    return 0;
}

void load_config(void)
{
	int i;
	
    Directive confvalues[][1] = {
		{{"AutoSync", {{PARAM_SET, PARAM_RELOAD, &AutoSync}}}},
		{{"AutoSyncTime", {{PARAM_TIME, PARAM_RELOAD, &AutoSyncTime}}}}
    };
	
	for (i = 0; i < 2; i++)
    	moduleGetConfigDirective(confvalues[i]);

    if (AutoSync && !AutoSyncTime) {
        AutoSyncTime = dotime("24h");
    }

    if (debug)
        alog("debug: [ircd_tssync] Set config vars: AutoSync=%d AutoSyncTime=%d", AutoSync, AutoSyncTime);
}

/* Explain what TSSYNC does when used. Appears in OperServ Help */
int my_os_tssync_help(User *u) {
    notice(s_OperServ, u->nick, "Syntax: \002TSSYNC\002");
    notice(s_OperServ, u->nick, " ");
    notice(s_OperServ, u->nick, "Sync the clocks between servers.");
}

/* Add the command to help */
void my_operserv_help(User *u) {
    if (is_services_root(u))
       notice(s_OperServ, u->nick, "    TSSYNC        Sync the clocks between servers.");
    else if (is_services_admin(u))
       notice(s_OperServ, u->nick, "    TSSYNC        Sync the clocks between servers.");
    else if (is_services_oper(u))
       notice(s_OperServ, u->nick, "    TSSYNC        Sync the clocks between servers.");
	else
       notice(s_OperServ, u->nick, "    TSSYNC        Sync the clocks between servers.");
}