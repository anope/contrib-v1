/* 
 * Author: culex <culex@xertion.org>
 * Date: Nov 5, 2011
 *
 * ns_gnick: Get the display nick of any given nick.
 * Parts copied from os_access_levels by Adam.
 *
 * Configuration directrives:
 * NSGNickAccess 0-4
 * Determines access to gnick.
 * 0 = no checks (everyone)
 * 1 = IRCop only
 * 2 = Services Operator only
 * 3 = Services Administrators only
 * 4 = Services Roots only
 * Defaults to 0 (Seeing how users can get the display nick from adding
 * access on channel anyway.
 *
 * Changelog:
 * 1.2: Fixed a memory leak.
 * 1.1: Dynamic permission check, see config.
 * 1.0: Pretends to work, successfully
 */
#include "module.h"

#define AUTHOR "culex"
#define VERSION "1.2"

#define LNG_NUM_STRINGS 4

#define LNG_NICK_HELP              0
#define LNG_NICK_HELP_GNICK        1
#define LNG_GNICK_SYNTAX           2
#define LNG_GNICK_FOUND            3

static int do_reload(int argc, char **argv);
void do_help_list(User *u);
int do_help(User *u);
int my_ns_gnick(User *u);
void do_config(void);
void my_add_languages(void);

Command *cmd;

int AnopeInit(int argc, char **argv)
{
    EvtHook *evt;
    int status;

    cmd = createCommand("GNICK", my_ns_gnick, NULL, -1, -1, -1, -1, -1);
    status = moduleAddCommand(NICKSERV, cmd, MOD_HEAD);
    if(status) {
        alog("[ns_gnick] Unable to create GNICK command: %d", status);
        return MOD_STOP;
    }

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);

    moduleAddHelp(cmd, do_help);
    moduleSetNickHelp(do_help_list);

    my_add_languages();
    do_config();

    evt = createEventHook(EVENT_RELOAD, do_reload);
    moduleAddEventHook(evt);

    alog("[ns_gnick] Loaded successfully");

    return MOD_CONT;
}

void AnopeFini(void)
{
}

static int do_reload(int argc, char **argv)
{
    do_config();
    return MOD_CONT;
}

void do_help_list(User *u)
{
    moduleNoticeLang(s_NickServ, u, LNG_NICK_HELP);
}

int do_help(User *u)
{
    moduleNoticeLang(s_NickServ, u, LNG_GNICK_SYNTAX);
    notice_user(s_NickServ, u, " ");
    moduleNoticeLang(s_NickServ, u, LNG_NICK_HELP_GNICK);
    return MOD_STOP;
}

int my_ns_gnick(User *u)
{
    char *buf;
    char *arg;
    NickAlias *na = NULL;

    buf = moduleGetLastBuffer();
    arg = myStrGetToken(buf, ' ', 0);
    if(!arg) {
        moduleNoticeLang(s_NickServ, u, LNG_GNICK_SYNTAX);

        notice_lang(s_NickServ, u, MORE_INFO, s_NickServ, "GNICK");

        return MOD_CONT;
    }

    na = findnick(arg);
    if(na == NULL) {
        notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, arg);

        free(arg);
        return MOD_CONT;
    }

    moduleNoticeLang(s_NickServ, u, LNG_GNICK_FOUND, arg, na->nc->display);

    free(arg);
    return MOD_CONT;
}

void do_config(void)
{
    int level = 0;
    Directive confvalues[][1] = {
        {{"NSGNickAccess", {{PARAM_INT, PARAM_RELOAD, &level}}}}
    };
    moduleGetConfigDirective(confvalues[0]);

    if(debug)
        alog("debug: [ns_gnick] NSGNickAccess set to %d", level);

    if(level == 0)
        cmd->has_priv = NULL;
    else if(level == 1)
        cmd->has_priv = is_oper;
    else if(level == 2)
        cmd->has_priv = is_services_oper;
    else if(level == 3)
        cmd->has_priv = is_services_admin;
    else if(level == 4)
        cmd->has_priv = is_services_root;
}

void my_add_languages(void)
{
    /* We don't even HAVE more than one language, but correctness for the sake
     * of correctness...
     */
    /* English (US) */
    char *langtable_en_us[] = {
        /* LNG_NICK_HELP */
        "    GNICK      Get someone's display nick",
        /* LNG_NICK_HELP_GNICK */
        "Gets the display nick of the given nick",
        /* LNG_GNICK_SYNTAX */
        "Syntax: GNICK nick",
        /* LNG_GNICK_FOUND */
        "GNICK %s %s"
    };
    moduleInsertLanguage(LANG_EN_US, LNG_NUM_STRINGS, langtable_en_us);
}

