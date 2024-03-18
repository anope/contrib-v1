/***************************************************************************
 **  ns_greet_normalization.c  ****** Author: GeniusDex ** Version: 1.0.2 **
 ***************************************************************************
 *                                                                         *
 * Service:     NickServ                                                   *
 * Module:      Normalize Greets                                           *
 * Version:     1.0.2                                                      *
 * License:     GPL [GNU Public License]                                   *
 * Author:      GeniusDex                                                  *
 * E-mail:      geniusdex@anope.org                                        *
 * Description: Normalize all greet messages set with NickServ             *
 *                                                                         *
 *   When this module is loaded, all greet messages being set via NickServ *
 *   SET GREET will be stripped from color, bold, underline, and other     *
 *   control codes, making sure that they won't exist in the greets.       *
 *                                                                         *
 *   No configuration needed, it just works (tm).                          *
 *                                                                         *
 ***************************************************************************
 **  CHANGES  *****************************************  VERSION HISTORY  **
 ***************************************************************************
 *** 1.0.2 ************************************************** 03/09/2005 ***
 * -Few minor cleanups                                                     *
 *** 1.0.1 ************************************************** 24/04/2005 ***
 * -Fixed a small bug causing a crash on invalid input                     *
 *** 1.0.0 ************************************************** 23/04/2005 ***
 * -Initial release                                                        *
 ***************************************************************************
 ****************** Don't change anything below this line ******************
 **************************************************************************/

#include "module.h"

#define AUTHOR "GeniusDex"
#define VERSION "1.0.2"

/* Our function which will hook NickServ SET */
int my_ns_set(User *u);

int AnopeInit(int argc, char **argv)
{
        Command *c;
        int status;

        /* Create the command hook for NickServ SET */
        c = createCommand("set", my_ns_set, NULL, -1, -1, -1, -1, -1);
        if ((status = moduleAddCommand(NICKSERV, c, MOD_HEAD))) {
                alog("[ns_greet_normalization] Unable to hook to NS SET (%d)", status);
                return MOD_STOP;
        }

        /* Add author and version to the modinfo */
        moduleAddAuthor(AUTHOR);
        moduleAddVersion(VERSION);

        /* And all done */
        return MOD_CONT;
}

void AnopeFini(void)
{
        /* Nothing to do here */
}

int my_ns_set(User *u)
{
        char *space_one;
        char *space_two;
        char new_buffer[BUFSIZE];

        /* First we find the second paramenter and isolate it for comparision */
        space_one = strchr(mod_current_buffer, ' ');
        if (!space_one)
                return MOD_CONT;
        space_one++;
        space_two = strchr(space_one, ' ');
        if (!space_two)
                return MOD_CONT;
        *space_two = '\0';

        /* We have a valid second paramenter now, see if it is a greet */
        if (stricmp(space_one, "greet") != 0) {
                *space_two = ' ';
                return MOD_CONT;
        }

        space_two++;

        /* We are setting a greet. Currently space_two points to the greet we're
         * setting, so this needs to be normalized. The rest is still in the
         * mod_current_buffer we started with, but mod_current_buffer is now
         * NULL-terminated after 'greet', so we can just append a space and the
         * normalized buffer behind it.
         */
        snprintf(new_buffer, BUFSIZE, "%s %s", mod_current_buffer, normalizeBuffer(space_two));

        /* Then we free our old buffer, and activate the new buffer. We need to
         * update mod_current_buffer so the modules won't complain, and we need the
         * strtok there so the core knows what happened as well.
         */
        free(mod_current_buffer);
        mod_current_buffer = sstrdup(new_buffer);
        strtok(mod_current_buffer, " ");

        return MOD_CONT;
}

/* EOF */
