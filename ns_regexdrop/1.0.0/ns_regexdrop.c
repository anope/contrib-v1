/* ns_regexdrop
 *
 * (C) 2007 Gabriel Acevedo
 * Contact me at drstein@anope.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, write to the
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Please read COPYING and README for further details.
 *
 */

/*************************************************************************/

/* Include the needed files */
#include "module.h"
#include <regex.h>

/* Module information */
#define MYNAME "ns_regexdrop"
#define AUTHOR "DrStein"
#define VERSION "1.0.0"

/*************************************************************************/

#define LOG_CREATE_COMMAND_ERROR "%s: an error ocurred when creating commands!"
#define LOG_NICK_DROPPED "%s: %s!%s@%s dropped nickname %s (group %s) (e-mail: %s)"

#define LANG_NUM_STRINGS             6
#define REGEXDROP_HELP               0
#define REGEXDROP_HELP_CMD           1
#define REGEXDROP_SYNTAX             2
#define REGEXDROP_NUM_DROPPED        3
#define REGEXDROP_NUM_NOT_DROPPED    4
#define REGEXDROP_PERMISSION_DENIED  5

/*************************************************************************/

int do_regexdrop(User *u);
int match(const char *string, char *pattern);
int regexDropHelp(User * u);
void mMainNickHelp(User * u);
void m_AddLanguages(void);

/*************************************************************************/

/**
 * AnopeInit is called when the module is loaded
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv)
{
    Command *c;
    int status = 0;

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    moduleSetType(THIRD);

    c = createCommand("REGEXDROP", do_regexdrop, is_services_admin, -1, -1, -1, -1, -1);
    moduleAddHelp(c, regexDropHelp);
    status += moduleAddCommand(NICKSERV, c, MOD_HEAD);

    moduleSetNickHelp(mMainNickHelp);

    m_AddLanguages();

    if (status != MOD_ERR_OK) {
        alog(LOG_CREATE_COMMAND_ERROR, MYNAME);
        return MOD_STOP;
    }

    return MOD_CONT;
}

/**
 * Unload the module
 **/
void AnopeFini(void)
{
}

int do_regexdrop(User *u)
{
    char *buffer = NULL;
    char *regex = NULL;
    char *verbose = NULL;
    NickAlias *na, *next;
    int i, use_verbose = 0;
    int count_deleted = 0;
    int count_not_deleted = 0;

    if (!(is_services_admin(u))) {
        notice_lang(s_NickServ, u, ACCESS_DENIED);
        return MOD_CONT;
    }

    buffer = moduleGetLastBuffer();
    if (buffer) {
        regex = myStrGetToken(buffer, ' ', 0);
        verbose = myStrGetToken(buffer, ' ', 1);
    }

    if (!regex) {
        moduleNoticeLang(s_NickServ, u, REGEXDROP_SYNTAX);
    } else {
        if (verbose && stricmp(verbose, "verbose") == 0)
            use_verbose = 1;

        if (readonly)
            notice_lang(s_NickServ, u, READ_ONLY_MODE);

        for (i = 0; i < 1024; i++) {
            for (na = nalists[i]; na; na = next) {
                next = na->next;
                if(match(na->nick, regex) == 1) {
                    if (NSSecureAdmins && nick_is_services_admin(na->nc) && !is_services_root(u)) {
                        if (use_verbose == 1)
                            moduleNoticeLang(s_NickServ, u, REGEXDROP_PERMISSION_DENIED, na->nick);
                        count_not_deleted++;
                    } else {
                        if (ircd->sqline && (na->status & NS_VERBOTEN)) {
                            anope_cmd_unsqline(na->nick);
                        }

                        alog(LOG_NICK_DROPPED, s_NickServ, u->nick, u->username, u->host,
                             na->nick, na->nc->display,
                             (na->nc->email ? na->nc->email : "none"));
                        send_event(EVENT_NICK_DROPPED, 1, na->nick);
                        if (use_verbose == 1)
                            notice_lang(s_NickServ, u, NICK_X_DROPPED, na->nick);
                        delnick(na);
                        count_deleted++;
                    }
                }
            }
        }
        moduleNoticeLang(s_NickServ, u, REGEXDROP_NUM_DROPPED, count_deleted);
        if (count_not_deleted > 0)
            moduleNoticeLang(s_NickServ, u, REGEXDROP_NUM_NOT_DROPPED, count_not_deleted);
    }

    return MOD_CONT;
}

/*************************************************************************/

/*
 * Copyright © 1997 The Open Group
 *
 * Match string against the extended regular expression in
 * pattern, treating errors as no match.
 *
 * return 1 for match, 0 for no match
 */
int match(const char *string, char *pattern)
{
    int status;
    regex_t re;

    if (!string || !pattern)
        return 0;

    if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0) {
        return 0;      /* report error */
    }
    status = regexec(&re, string, (size_t) 0, NULL, 0);
    regfree(&re);
    if (status != 0) {
        return 0;      /* report error */
    }
    return 1;
}

/*************************************************************************/

int regexDropHelp(User * u)
{
    if (is_services_admin(u))
        moduleNoticeLang(s_NickServ, u, REGEXDROP_HELP);

    return MOD_CONT;
}

/* This help will be added to the main NickServ list */
void mMainNickHelp(User * u)
{
    if (is_services_admin(u))
        moduleNoticeLang(s_NickServ, u, REGEXDROP_HELP_CMD);
}

/*************************************************************************/

/**
 * manages the multilanguage stuff
 **/
void m_AddLanguages(void)
{
    char *langtable_en_us[] = {
        /* REGEXDROP_HELP */
        "Syntax: REGEXDROP regular-expresion [verbose]\n"
            "Drop all the nicknames matching the given regular expresion.\n"
            "For further information about regular expressions, please\n"
            "visit http://en.wikipedia.org/wiki/Regular_expression",
        /* REGEXDROP_HELP_CMD */
        "    REGEXDROP  Drop all the nicknames matching the given regular expresion.",
        /* REGEXDROP_SYNTAX */
        "Syntax: REGEXDROP regular-expresion [verbose]\n",
        /* REGEXDROP_NUM_DROPPED */
        "A total of %d nicknames have been dropped.",
        /* REGEXDROP_NUM_NOT_DROPPED */
        "There are %d nicknames which couln't be deleted.",
        /* REGEXDROP_PERMISSION_DENIED */
        "Permision denied when trying to drop nickname %s"
    };

    moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}

/*************************************************************************/

