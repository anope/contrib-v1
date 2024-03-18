#include "module.h"
#define AUTHOR "Tom65789"
#define VERSION "0.2"

/*************************************************************************
* Module: os_killuser                                                    *
* Author: Tom65789 <Tom65789@T65789.co.uk>                               *
* Date: 11th May 2006                                                    *
**************************************************************************
*                                                                        *
* Allows IRC Operators to kill a user with/without reason from the       *
* network using OperServ.                                                *
*                                                                        *
* /msg OperServ KILLUSER nick reason                                     *
*                                                                        *
**************************************************************************
* Version History:                                                       *
*                                                                        *
* 0.1 -                                                                  *
*     Completed Code.                                                    *
*                                                                        *
* 0.2 -                                                                  *
*     Fixed Little Typo.                                                 *
*     Got rid of strtok.                                                 *
*     Added reason for the killuser.                                     *
*                                                                        *
**************************************************************************
*                                                                        *
* This program is free software; you can redistribute it and/or modify   *
* it under the terms of the GNU General Public License as published by   *
* the Free Software Foundation; either version 2 of the License, or      *
* (at your option) any later version.                                    *
*                                                                        *
***************************** Configuration *****************************/

/* There are no options */

/************************* End Of Configuration! *************************
*       DO NOT EDIT BELOW HERE - UNLESS YOU KNOW WHAT YOU ARE DOING      *
*************************************************************************/

int do_kill_user(User *u);
int killuser_help(User *u);
void killuser(User *u);

/**
 * Main module init routine.
 * @param argc Argument count.
 * @param argv Argument list.
 **/
int AnopeInit(int argc, char **argv) 
{ 
    int status = 0;
    Command *c;

    c = createCommand("KILLUSER", do_kill_user, is_oper, -1, -1, -1, -1, -1);
    status += moduleAddCommand(OPERSERV, c, MOD_HEAD);

    moduleAddHelp(c, killuser_help);
    moduleSetOperHelp(killuser);

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    moduleSetType(THIRD);

    return MOD_CONT;
}

/*************************************************************************/

/**
 * Anope unload routine.
 **/
void AnopeFini(void) {
}

/*************************************************************************/

int do_kill_user(User *u)
{
    User *u2;
    char *cmd = moduleGetLastBuffer();
    char *target = NULL;
    char *reason = NULL;

    target = myStrGetToken(cmd, ' ', 0);
    reason = myStrGetTokenRemainder(cmd, ' ', 1);

    if(is_oper(u)) {
        if ((u2 = finduser(target))) { 
            if (!reason) {
                kill_user(s_OperServ, target, "Killed (Reason Unknown)");
            } else
                kill_user(s_OperServ, target, reason);
        } else
            notice(s_OperServ, u->nick, "%s is not on this network", target);
    } else
        notice(s_OperServ, u->nick, "Permission Denied");

    if (target) free(target);
    if (reason) free(reason);

    return MOD_CONT;
}

/*************************************************************************/

int killuser_help(User *u)
{
    if (is_oper(u)) {
        notice(s_OperServ, u->nick, "Syntax: \002KILLUSER nick [reason]\002");
        notice(s_OperServ, u->nick, " ");
        notice(s_OperServ, u->nick, "Allows IRC Operators to kill a user off the");
        notice(s_OperServ, u->nick, "network with or without a reason");
    }

    return MOD_CONT;
}

/*************************************************************************/

void killuser(User *u)
{
    if (is_oper(u)) {
        notice(s_OperServ, u->nick, "    KILLUSER   Kill a user of the network.");
    }
}

/* EOF */
