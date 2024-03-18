#include "module.h"
#define AUTHOR "Tom65789"
#define VERSION "1.05"

/*************************************************************************
* Module: os_qakill                                                      *
* Author: Tom65789 <Tom65789@T65789.co.uk>                               *
* Date: 17th August 2006                                                 *
**************************************************************************
*                                                                        *
* Allows service admins to akill a nick.                                 *
*                                                                        *
* /msg OperServ QAKILL nick                                              *
*                                                                        *
* Original work by SGR                                                   *
*                                                                        *
**************************************************************************
* Version History:                                                       *
*                                                                        *
* 1.03 -                                                                 *
*     Updated code and cleaned up for Anope 1.7.15                       *
*                                                                        *
* 1.04 -                                                                 *
*     Fixed a memory leak.                                               *
*                                                                        *
* 1.05 -                                                                 *
*     Changed configuration to services.conf                             *
*                                                                        *
**************************************************************************
*                                                                        *
* This program is free software; you can redistribute it and/or modify   *
* it under the terms of the GNU General Public License as published by   *
* the Free Software Foundation; either version 2 of the License, or      *
* (at your option) any later version.                                    *
*                                                                        *
***************************** Configuration ******************************
Place these into your serivces.conf:
------------------------------------------

# DisableLowerQAKill [OPTIONAL]
#
# When defined services admins will not be able to use the command on the 
# nicks of other identified services admins or roots. (comment to disable)

DisableLowerQAKill

-------------------------------------------
- END -
************************** End Of Configuration! *************************
*       DO NOT EDIT BELOW HERE - UNLESS YOU KNOW WHAT YOU ARE DOING      *
*************************************************************************/

/**
 * Multi language crap
 **/
#define LANG_NUM_STRINGS                 7

#define LANG_QAKILL                      0
#define LANG_QAKILL_HELP                 1
#define LANG_QAKILL_HELP2                2
#define LANG_NAKILL_SYNTAX               3
#define LANG_ACCESS_DENIED               4
#define LANG_PERMISSION_DENIED           5
#define SEPERATOR                        6

/*************************************************************************/

static int do_qakill(User * u);
void do_qakill_help(User *u);
int do_qakill_help_full(User *u);
void AddLanguages(void);
int do_reload(int argc, char **argv);
int load_config();

char *DisableLowerQAKill;

/**
 * Main module init routine.
 * @param argc Argument count.
 * @param argv Argument list.
 **/
int AnopeInit(int argc, char **argv) 
{
    int status = 0;
    Command *c;
    EvtHook *hook = NULL;

    /* create the command QAKILL */
    c = createCommand("QAKILL", do_qakill, is_services_admin,-1,-1,-1,-1,-1);
    moduleAddCommand(OPERSERV, c, MOD_HEAD);

    /* create hook for EVENT_RELOAD */
    hook = createEventHook(EVENT_RELOAD, do_reload);
    status = moduleAddEventHook(hook);
    if (status != MOD_ERR_OK) {
        alog("[os_qakill%s] unable to bind to EVENT_RELOAD error [%d]", MODULE_EXT, status);
        return MOD_STOP;
    }

    AddLanguages();
    load_config();

    moduleAddHelp(c, do_qakill_help_full);
    moduleSetOperHelp(do_qakill_help);

    alog("Loading module os_qakill.so [Status: %d]", status);

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    moduleSetType(THIRD);

    return MOD_CONT; 
}

/*************************************************************************/

/**
 * Unload Routine
 **/
void AnopeFini(void) 
{
    if (DisableLowerQAKill) {
        free(DisableLowerQAKill);
    }
    alog("Unloading os_qakill.so");
}

/*************************************************************************/

/**
 * Main Function
 **/
static int do_qakill(User *u)
{
    char *cmd = moduleGetLastBuffer();
    char *to_be_akilled, *reason;
    char reasonx[512];
    char mask[USERMAX + HOSTMAX + 2];
    User *target;

    to_be_akilled = myStrGetToken(cmd, ' ', 0);
    reason = myStrGetTokenRemainder(cmd, ' ', 1);

    if (is_services_admin(u)) { 
       if (to_be_akilled) {
           if (!reason) {
               reason = "This nick has been akilled - Reason Unknown!";
           }
           if (AddAkiller) {
               snprintf(reasonx, sizeof(reasonx), "[%s] %s", u->nick, reason);
           }
           if ((target = finduser(to_be_akilled))) {
               sprintf(mask, "*@%s", target->host);
               /* if defined prevent lowers using qakill on service admins/roots */
               if (DisableLowerQAKill) {
                   if ((is_services_admin(target)) && (!is_services_root(u))) {
                       moduleNoticeLang(s_OperServ, u, LANG_PERMISSION_DENIED);
                       if (WallOSAkill) {
                           wallops(s_OperServ, "%s attempted to QAKILL %s (%s)", u->nick, target->nick, reasonx);
                       } else {
                           anope_cmd_global(s_GlobalNoticer, "%s attempted to QAKILL %s (%s)", u->nick, target->nick, reasonx);
                       }
                       if(to_be_akilled) free(to_be_akilled);
                       if(reason) free(reason);
                       return MOD_STOP;
                   }
               } else {
                   add_akill(u, mask, u->nick, time(NULL)+AutokillExpiry, reason);
               }
               if (WallOSAkill) {
                   wallops(s_OperServ, "%s used QAKILL on %s (%s)", u->nick, target->nick, reasonx);
               } else {
                   anope_cmd_global(s_GlobalNoticer, "%s used QAKILL on %s (%s)", u->nick, target->nick, reasonx);
               }
               if (!AkillOnAdd) {
                   kill_user(s_OperServ, target->nick, reasonx);
               }
           } else
               notice_lang(s_OperServ, u, NICK_X_NOT_IN_USE, to_be_akilled);
       } else
           notice(s_OperServ, u->nick, "See \002/msg OperServ HELP QAKILL\002 for more info.");
    } else 
        moduleNoticeLang(s_OperServ, u, LANG_ACCESS_DENIED);

    /* free whats been assigned to memory */
    if(to_be_akilled) free(to_be_akilled);
    if(reason) free(reason);

    return MOD_CONT;
}

/*************************************************************************/

/**
 * Add command to help list
 **/
void do_qakill_help(User *u)
{
   if (is_services_admin(u)) {
       moduleNoticeLang(s_OperServ, u, LANG_QAKILL);
   }
}

/*************************************************************************/

/**
 * Help for QAKILL
 **/
int do_qakill_help_full(User *u)
{
    if (is_services_admin(u)) {
        moduleNoticeLang(s_OperServ, u, SEPERATOR);
        moduleNoticeLang(s_OperServ, u, LANG_QAKILL_HELP);
        if (DisableLowerQAKill) {
        moduleNoticeLang(s_OperServ, u, LANG_QAKILL_HELP2);
        }
        moduleNoticeLang(s_OperServ, u, SEPERATOR);
        return MOD_CONT;
    }

   return MOD_CONT;
}

/*************************************************************************/

/**
 * Add module langauge strings to anope's module langauge system.
 * This will allow users to get help in there own langauge.
 **/
void AddLanguages(void) {
    char *langtable_en_us[] = {
        /* LANG_QAKILL */
        "    QAKILL      AKILL a user by nick.",
        /* LANG_QAKILL_HELP */
        "Syntax: \002QAKILL Nick [Reason]\002\n"
        " \n"
        "Allows Services Admins to add an akill with just a nick. A reason\n"
        "is not mandatory, however it is often useful for you put a valid one.\n"
        "The akill placed will be in the form *@full.host.mask for the default",
        /* LANG_QAKILL_HELP2 */
        " \n"
        "You will not be able to QAKILL identifed services admins or services\n"
        "roots unless you are a services root.",
        /* LANG_QAKILL_SYNTAX */
        "Syntax: \002QAKILL Nick [Reason]\002",
        /* LANG_ACCESS_DENIED */
        "Access Denied",
        /* LANG_PERMISSION_DENIED */
        "Permission Denied",
        /* SEPERATOR */
        "-----------------------------------------------------------------------"
    };
    moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}

/*************************************************************************/

/**
 * Reload Config
 **/
int do_reload(int argc, char **argv)
{
    load_config();
    return MOD_CONT;
}

/*************************************************************************/

/**
 * Load Config
 **/
int load_config()
{
    int i;

    Directive directivas[][1] = {
        {{"DisableLowerQAKill", {{PARAM_SET, PARAM_RELOAD, &DisableLowerQAKill}}}},
    };

    for (i = 0; i < 1; i++)
        moduleGetConfigDirective(directivas[i]);

    if (!DisableLowerQAKill) {
        alog("You do not have DisableLowerQAKill in your services.conf - this means services admins will be able to use the command on the nicks of other identified services admins or roots.");
    } else {
        alog("os_qakill: Config: Disable Lower QAKILL: [%s]", DisableLowerQAKill);
    }

    return 0;
}

/* EOF */
