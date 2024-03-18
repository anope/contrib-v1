#include "module.h"
#define AUTHOR "Jobe"
#define VERSION "0.2"

/*************************************************************************
* Module: os_chgswhois                                                   *
* Author: Jobe <Jobe@invictachat.net>                                    *
* Date: 04th Jul 2008                                                    *
**************************************************************************
*                                                                        *
* Allows IRC Operators to change A users SWHOIS                          *
*                                                                        *
* /msg OperServ CHGSWHOIS nick swhois                                    *
*                                                                        *
**************************************************************************
* Version History:                                                       *
*                                                                        *
* 0.1 -                                                                  *
*     Completed Code.                                                    *
*                                                                        *
* 0.2 -                                                                  *
*     Removed moduleSetType() call on the advice of katsklaw             *
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

int do_swhois_user(User *u);
int chgswhois_help(User *u);
void my_chgswhois(User *u);

/**
 * Main module init routine.
 * @param argc Argument count.
 * @param argv Argument list.
 **/
int AnopeInit(int argc, char **argv) 
{ 
    int status = 0;
    Command *c;

    c = createCommand("CHGSWHOIS", do_swhois_user, is_services_admin, -1, -1, -1, -1, -1);
    status += moduleAddCommand(OPERSERV, c, MOD_HEAD);

    moduleAddHelp(c, chgswhois_help);
    moduleSetOperHelp(my_chgswhois);

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);

    return MOD_CONT;
}

/*************************************************************************/

/**
 * Anope unload routine.
 **/
void AnopeFini(void) {
}

/*************************************************************************/

int do_swhois_user(User *u)
{
    User *u2;
    char *cmd = moduleGetLastBuffer();
    char *target = NULL;
    char *swhois = NULL;

    target = myStrGetToken(cmd, ' ', 0);
    swhois = myStrGetTokenRemainder(cmd, ' ', 1);

    if ((u2 = finduser(target))) { 
        if (!swhois) {
            anope_cmd_swhois(s_OperServ, target, "");
        } else
            anope_cmd_swhois(s_OperServ, target, swhois);
    } else
        notice(s_OperServ, u->nick, "%s is not on this network", target);

    if (target) free(target);
    if (swhois) free(swhois);

    return MOD_CONT;
}

/*************************************************************************/

int chgswhois_help(User *u)
{
    if (is_oper(u)) {
        notice(s_OperServ, u->nick, "Syntax: \002CHGSWHOIS nick [swhois]\002");
        notice(s_OperServ, u->nick, " ");
        notice(s_OperServ, u->nick, "Allows Services Admins to change a users SWHOIS");
    }

    return MOD_CONT;
}

/*************************************************************************/

void my_chgswhois(User *u)
{
    if (is_oper(u)) {
        notice(s_OperServ, u->nick, "    CHGSWHOIS  Change a users SWHOIS.");
    }
}

/* EOF */
