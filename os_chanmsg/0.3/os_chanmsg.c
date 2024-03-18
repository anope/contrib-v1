#include "module.h"
#define AUTHOR "Tom65789"
#define VERSION "0.3"

/*************************************************************************
* Module: os_chanmsg                                                     *
* Author: Tom65789 <Tom65789@T65789.co.uk>                               *
* Date: 11th May 2006                                                    *
**************************************************************************
*                                                                        *
* This module makes ChanServ sends a notice to EVERY channel on the      *
* network with a msg.                                                    *
*                                                                        *
* /msg OperServ CHANMSG message                                          *
*                                                                        *
**************************************************************************
* Version History:                                                       *
*                                                                        *
* 0.1 -                                                                  *
*     Completed Code.                                                    *
*                                                                        *
* 0.2 -                                                                  *
*     Fixed Little Typo.                                                 *
*                                                                        *
* 0.3 -                                                                  *
*     Added a header to the message and who the chanmsg was from         *
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

int do_chanmsg(User *u);
int os_chanmsg_help(User *u);
void my_operserv_help(User *u);

/**
 * Main module init routine.
 * @param arg Argument count.
 * @param argv Argument list.
 **/
int AnopeInit(int arg, char **argv) 
{
    Command *c;

    c = createCommand("CHANMSG", do_chanmsg, is_oper, -1, -1, -1, -1, -1);
    moduleAddCommand(OPERSERV, c, MOD_HEAD);

    moduleAddHelp(c, os_chanmsg_help);
    moduleSetOperHelp(my_operserv_help);

    moduleAddAuthor(AUTHOR);    
    moduleAddVersion(VERSION);
    moduleSetType(THIRD);

    return MOD_CONT;
}

/*************************************************************************/

void AnopeFini(void) {
}

/*************************************************************************/

int do_chanmsg(User *u)
{
    char *msg = moduleGetLastBuffer();
    Channel *c;
    int carryon = 0;
    int i;

    if (!msg) {
        notice(s_OperServ, u->nick, "Syntax: \002CHANMSG TEXT\002 [You need to include some text]");
    }

    if (WallOSGlobal) {
        wallops(s_OperServ, "\2%s\2 just used CHANMSG command.", u->nick); 
    }

    carryon = 0;
    for (i = 0; i < 1024; i++) {
        for (c = chanlist[i]; c; c = c->next) {
            notice (s_ChanServ, c->name, "( %s Global Channel Notice From %s ) %s", NetworkName, u->nick, msg);
        }
    }

    return MOD_CONT;
}

/*************************************************************************/

/**
 * Detailed Help
 **/
int os_chanmsg_help(User *u) 
{
    if (is_oper(u)) {
        notice(s_OperServ, u->nick, "Syntax: \002CHANMSG TEXT\002");
        notice(s_OperServ, u->nick, " ");
        notice(s_OperServ, u->nick, "Make ChanServ Send a notice to all The");
        notice(s_OperServ, u->nick, "registered Channels on your network!");
        notice(s_OperServ, u->nick, "----------------------------------");
        notice(s_OperServ, u->nick, "This is available for ircops only.");
    }
    return MOD_CONT;                       
}

/*************************************************************************/

/**
 * Add to the list of commands in HELP
 **/
void my_operserv_help(User *u) {
    if (is_oper(u)) {
         notice(s_OperServ, u->nick, "    CHANMSG     Send a Notice To all Registered Channels!");
    }
}

/* EOF */
