/*************************************************************************
* Module: os_flood                                                       *
* Author: mooncup <mooncup@anonnet.org>                                  *
* Date: 26th June 2010                                                   *
*                                                                        *
* Vaguely based on os_kill by Tom65789. Props to him.                    *
**************************************************************************
*                                                                        *
* Removes fake lag from users for the current session. (UnrealIRCD Only) *
*                                                                        *
* /msg OperServ FLOOD nick		                                         *
*                                                                        *
**************************************************************************
*                                                                        *
* This program is free software; you can redistribute it and/or modify   *
* it under the terms of the GNU General Public License as published by   *
* the Free Software Foundation; either version 2 of the License, or      *
* (at your option) any later version.                                    *
*                                                                        *
*************************************************************************/

#include "module.h"
#define AUTHOR "mooncup"
#define VERSION "1.0"

int flood(User *u);
int flood_help(User *u);
void doflood(User *u);

int AnopeInit(int argc, char **argv) 
{
    int status = 0;
    Command *c;

    c = createCommand("FLOOD", flood, is_oper, -1, -1, -1, -1, -1);
    status += moduleAddCommand(OPERSERV, c, MOD_HEAD);

    moduleAddHelp(c, flood_help);
    moduleSetOperHelp(doflood);
	
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    moduleSetType(THIRD);

    return MOD_CONT;
}

void AnopeFini(void) {
}

int flood(User *u)
{
    User *tu;
    char *cmd = moduleGetLastBuffer();
    char *target = NULL;

    target = myStrGetToken(cmd, ' ', 0);

        if ((tu = finduser(target))) {
                send_cmd(NULL, "SVS2NOLAG + %s", target);
                anope_cmd_global(s_OperServ, "\2%s\2 used FLOOD on \2%s\2", u->nick,target);
        } else
            notice(s_OperServ, u->nick, "%s is not on this network", target);

	if (target) free(target);
    return MOD_CONT;
}

int flood_help(User *u)
{
    if (is_oper(u)) {
        notice(s_OperServ, u->nick, "Syntax: \002FLOOD nick\002");
        notice(s_OperServ, u->nick, " ");
        notice(s_OperServ, u->nick, "Removes fake lag from the specified user");
        notice(s_OperServ, u->nick, "for the current session");
    }
    return MOD_CONT;
}

void doflood(User *u)
{
    if (is_oper(u)) {
        notice(s_OperServ, u->nick, "    FLOOD       Remove fake lag from a user");
    }
}