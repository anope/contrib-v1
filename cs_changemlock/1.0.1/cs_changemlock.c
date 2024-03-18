/* cs_changemlock -- main source file.
 *
 * (C) 2005 Chris Hogben
 * Contact: heinz@anope.org
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
 */

/* Modes to set upon register */
char *changemlock = "-ilk";

/* Include the needed files */
#include "module.h"

#define AUTHOR "heinz"
#define VERSION "1.0.1"
#define MYNAME "cs_changemlock"

/* Main functions */
int changemlock_register(char *chan);

int AnopeInit(int argc, char **argv)
{
    EvtHook *hook = NULL;
    hook = createEventHook(EVENT_CHAN_REGISTERED, changemlock_register);
    moduleAddEventHook(hook);
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    alog("[%s] Module Loaded Successfully!", MYNAME);
    return MOD_CONT;
}

int changemlock_register(char *chan) {

    int add = -1;               /* 1 if adding, 0 if deleting, -1 if neither */
    unsigned char mode;
    char *tmpmodes;
    CBMode *cbm;
    Channel *c;
    ChannelInfo *ci;

    if (!(c = findchan(chan))) {
        alog ("[%s] DEBUG: Cannot find channel %s", MYNAME, chan);
        return MOD_CONT;
    }

    ci = c->ci;

    if (checkDefCon(DEFCON_NO_MLOCK_CHANGE)) {
        alog("[%s] DEBUG: Defcon states no mlock changes..", MYNAME);
        return MOD_CONT;
    }

     if (ircd->chanreg) {
        ci->mlock_on = ircd->regmode;
     } else {
        ci->mlock_on = 0;
     }
     ci->mlock_off = ci->mlock_limit = 0;
     ci->mlock_key = NULL;
     if (ircd->fmode) {
        ci->mlock_flood = NULL;
     }
     if (ircd->Lmode) {
         ci->mlock_redirect = NULL;
     }

     tmpmodes = sstrdup(changemlock);

     while ((mode = *tmpmodes++)) {
        switch (mode) {
         case '+':
             add = 1;
             continue;
         case '-':
             add = 0;
             continue;
         default:
             if (add < 0)
                 continue;
         }

         if ((int) mode < 128 && (cbm = &cbmodes[(int) mode])->flag != 0) {
             if (add) {
                 ci->mlock_on |= cbm->flag;
                 ci->mlock_off &= ~cbm->flag;
                 if (cbm->cssetvalue)
                     cbm->cssetvalue(ci, strtok(NULL, " "));
             } else {
                 ci->mlock_off |= cbm->flag;
                 if (ci->mlock_on & cbm->flag) {
                     ci->mlock_on &= ~cbm->flag;
                     if (cbm->cssetvalue)
                         cbm->cssetvalue(ci, NULL);
                 }
             }
         }
     }

     if (ircd->Lmode) {
         if ((ci->mlock_on & ircd->chan_lmode) && !(ci->mlock_on & CMODE_l)) {
             ci->mlock_on &= ~ircd->chan_lmode;
             free(ci->mlock_redirect);
         }
     }
     if (ircd->noknock && ircd->knock_needs_i) {
         if ((ci->mlock_on & ircd->noknock) && !(ci->mlock_on & CMODE_i)) {
             ci->mlock_on &= ~ircd->noknock;
         }
     }

     if (c)
         check_modes(c);

     return MOD_CONT;

}
