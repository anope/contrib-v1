/***************************************************************************************/
/* Anope Module : os_svsnick_sra.c : v1.x                                              */
/* Scott Seufert - katsklaw@ircmojo.net                                                */
/*                                                                                     */
/* Anope (c) 2000-2002 Anope.org                                                       */
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
#include <module.h>

#define AUTHOR "katsklaw"
#define VERSION "1.0.0-FINAL"

int do_svsnick(User * u);
void myOperServHelp(User * u);

int AnopeInit(int argc, char **argv)
{
    Command *c;

    c = createCommand("SVSNICK", do_svsnick, is_services_root, OPER_HELP_SVSNICK, -1, -1, -1, -1);
    moduleAddCommand(OPERSERV, c, MOD_HEAD);

    moduleSetOperHelp(myOperServHelp);
    
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);

	if (findModule("os_raw")) {
		alog("I found os_raw loaded!, Sorry it didn't work out.");
		return MOD_STOP;
	}

    if (!ircd->svsnick) {
        return MOD_STOP;
    }
    return MOD_CONT;
}

void AnopeFini(void)
{
	/* Nothing needed here - Win32 complains sometimes */
}


void myOperServHelp(User * u)
{
        notice_lang(s_OperServ, u, OPER_HELP_CMD_SVSNICK);
}

int do_svsnick(User * u)
{
    char *nick = strtok(NULL, " ");
    char *newnick = strtok(NULL, " ");

    NickAlias *na;
    char *c;

    if (!nick || !newnick) {
        syntax_error(s_OperServ, u, "SVSNICK", OPER_SVSNICK_SYNTAX);
        return MOD_CONT;
    }

    /* Truncate long nicknames to NICKMAX-2 characters */
    if (strlen(newnick) > (NICKMAX - 2)) {
        notice_lang(s_NickServ, u, NICK_X_TRUNCATED,
                    newnick, NICKMAX - 2, newnick);
        newnick[NICKMAX - 2] = '\0';
    }

    /* Check for valid characters */
    if (*newnick == '-' || isdigit(*newnick)) {
        notice_lang(s_OperServ, u, NICK_X_ILLEGAL, newnick);
        return MOD_CONT;
    }
    for (c = newnick; *c && (c - newnick) < NICKMAX; c++) {
        if (!isvalidnick(*c)) {
            notice_lang(s_OperServ, u, NICK_X_ILLEGAL, newnick);
            return MOD_CONT;
        }
    }

    if (!finduser(nick)) {
        notice_lang(s_OperServ, u, NICK_X_NOT_IN_USE, nick);
    } else if (finduser(newnick)) {
        notice_lang(s_NickServ, u, NICK_X_IN_USE, newnick);
    } else if ((na = findnick(newnick)) && (na->status & NS_VERBOTEN)) {
        notice_lang(s_NickServ, u, NICK_X_FORBIDDEN, newnick);
    } else {
        notice_lang(s_OperServ, u, OPER_SVSNICK_NEWNICK, nick, newnick);
        anope_cmd_global(s_OperServ, "%s used SVSNICK to change %s to %s",
                         u->nick, nick, newnick);
        anope_cmd_svsnick(nick, newnick, time(NULL));
    }
    return MOD_CONT;
}
