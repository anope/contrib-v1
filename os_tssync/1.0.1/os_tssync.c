/***************************************************************************************/
/* Anope Module : os_tssync.c : v1.0                                                   */
/* Scott Seufert (katsklaw@ircmojo.net)                                                */
/*                                                                                     */
/* Proxy module for Trystan, DO NOT CONTACT TRYSTAN FOR SUPPORT, YOU WILL BE IGNORED!  */
/*                                                                                     */
/* OS TSSYNC                                                                           */
/*                                                                                     */
/* Original work by Certus <certus@europnet.org>                                       */
/*           and by <nenolod@noderebellion.net>                                        */
/*                                                                                     */
/* This program is free software; you can redistribute it and/or modify it under the   */
/* terms of the GNU General Public License as published by the Free Software           */
/* Foundation; either version 1, or (at your option) any later version.                */
/*                                                                                     */
/*  This program is distributed in the hope that it will be useful, but WITHOUT ANY    */
/*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A    */
/*  PARTICULAR PURPOSE.  See the GNU General Public License for more details.          */
/*                                                                                     */
/***************************************************************************************/

#include "module.h"

#define AUTHOR "katsklaw"
#define VERSION "1.0.1"

int AutoSync = 0;
int AutoSyncTime = 0;

int valid_ircd(void);
int do_sync_ts(User *u);
int sync_callback(int ac, char **av);

void load_config(void);

int AnopeInit(int argc, char **argv)
{
    Command *c;

    if (!valid_ircd()) {
        alog("Your ircd is not supported.");
        return MOD_STOP;
    }

    if (!moduleMinVersion(1,7,10,810)) {
        alog("Your version of Anope isn't supported.");
        return MOD_STOP;
    }
    load_config();

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);

    c = createCommand("TSSYNC",do_sync_ts,is_services_admin,-1,-1,-1,-1,-1);

    if (!moduleAddCommand(OPERSERV,c,MOD_HEAD)) {
        alog("Loading module os_tssync.%s [Status: working]", MODULE_EXT);
        alog("New command: /msg operserv TSSYNC");
        if (AutoSync) {
            moduleAddCallback("sync_callback",time(NULL) + AutoSyncTime, sync_callback,0,NULL);
        }
        return MOD_CONT;
    } else {
        alog("Loading module os_tssync.so [Status: failed]");
        alog("Couldn't add command to operserv's cmdlist, this shouldn't have happened. Unloading....");
        return MOD_STOP;
    }
}

void AnopeFini(void)
{
    alog("Unloading os_tssync.%s", MODULE_EXT);
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
        alog("debug: [os_tssync] Set config vars: AutoSync=%d AutoSyncTime=%d", AutoSync, AutoSyncTime);
}



