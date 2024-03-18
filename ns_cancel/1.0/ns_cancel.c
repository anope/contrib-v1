/* 
 * Author: culex <culex@xertion.org>
 * Date: Oct 9, 2011
 *
 * Thanks for a lot of help, Adam.
 *
 * ns_cancel: Drop a pending nick using /ns cancel, in case a user typoed their
 * e-mail address or something like that.
 *
 * Configuration directrives: None
 */
#include "module.h"

#define AUTHOR "culex"
#define VERSION "1.0"

#define LNG_NUM_STRINGS 7

#define LNG_NICK_HELP              0
#define LNG_NICK_HELP_CANCEL       1
#define LNG_NICK_HELP_CANCEL_SA    2
#define LNG_CANCEL_SYNTAX          3
#define LNG_CANCEL_SYNTAX_SA       4
#define LNG_CANCEL_SUCCESS         5
#define LNG_CANCEL_NO_REQUEST      6

/* my_ to prevent some silly upstream function from intervening */
void do_help_list(User *u);
int do_help(User *u);
int my_ns_cancel(User *u);
void my_add_languages(void);

int AnopeInit(int argc, char **argv)
{
    Command *c;
    int status;
    if(!NSEmailReg) {
        alog("[ns_cancel] NSEmailReg disabled, this module is pointless. Cowardly refusing to load.");
        return MOD_STOP;
    }

    c = createCommand("CANCEL", my_ns_cancel, NULL, -1, -1, -1, -1, -1);
    moduleAddCommand(NICKSERV, c, MOD_HEAD);
    if((status = moduleAddCommand(CHANSERV, c, MOD_HEAD))) {
        alog("[ns_cancel] Unable to create CANCEL command: %d", status);
        return MOD_STOP;
    }

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);

    moduleAddHelp(c, do_help);
    moduleSetNickHelp(do_help_list);

    my_add_languages();

    alog("[ns_cancel] Loaded successfully");

    return MOD_CONT;
}

void AnopeFini(void)
{
}

void do_help_list(User *u)
{
    moduleNoticeLang(s_NickServ, u, LNG_NICK_HELP);
}

int do_help(User *u)
{
    int is_sa = is_services_admin(u);
    if(is_sa)
        moduleNoticeLang(s_NickServ, u, LNG_CANCEL_SYNTAX_SA);
    else
        moduleNoticeLang(s_NickServ, u, LNG_CANCEL_SYNTAX);
    notice_user(s_NickServ, u, " ");
    if(is_sa)
        moduleNoticeLang(s_NickServ, u, LNG_NICK_HELP_CANCEL_SA);
    else
        moduleNoticeLang(s_NickServ, u, LNG_NICK_HELP_CANCEL);
    return MOD_STOP;
}

int my_ns_cancel(User *u)
{
    char *buf;
    char *arg;
    int is_sa = is_services_admin(u);
    NickRequest *nr = NULL;

    buf = moduleGetLastBuffer();
    arg = myStrGetToken(buf, ' ', 0);
    if(!arg) {
        if(is_sa)
            moduleNoticeLang(s_NickServ, u, LNG_CANCEL_SYNTAX_SA);
        else
            moduleNoticeLang(s_NickServ, u, LNG_CANCEL_SYNTAX);

        notice_lang(s_NickServ, u, MORE_INFO, s_NickServ, "CANCEL");

        return MOD_CONT;
    }

    /* If they're SA, their nick is obviously registered and they probably
     * mean to cancel someone else's nick.
     */
    if(is_sa)
        nr = findrequestnick(arg);
    else
        nr = findrequestnick(u->nick);

    if(nr == NULL) {
        moduleNoticeLang(s_NickServ, u, LNG_CANCEL_NO_REQUEST, is_sa ? arg : u->nick);
    } else if(!enc_check_password(arg, nr->password) && !is_sa) {
        notice_lang(s_NickServ, u, PERMISSION_DENIED);
    } else {
        if(is_sa)
            alog("[ns_cancel] Nick request for %s cancelled by oper %s",
                    nr->nick, u->nick);
        else 
            alog("[ns_cancel] Nick request for %s cancelled", nr->nick);
        delnickrequest(nr);
        moduleNoticeLang(s_NickServ, u, LNG_CANCEL_SUCCESS);
    }

    free(arg);
    return MOD_CONT;
}

void my_add_languages(void)
{
    /* We don't even HAVE more than one language, but correctness for the sake
     * of correctness...
     */
    /* English (US) */
    char *langtable_en_us[] = {
        /* LNG_NICK_HELP */
        "    CANCEL     Cancel a pending nick registration",
        /* LNG_NICK_HELP_CANCEL */
        "Deletes your pending nickname so you can re-register with a corrected "
           "e-mail address.",
        /* LNG_NICK_HELP_CANCEL_SA */
        "Deletes the given pending nickname so the user can re-register with a corrected "
           "e-mail address.",
        /* LNG_CANCEL_SYNTAX */
        "Syntax: CANCEL password",
        /* LNG_CANCEL_SYNTAX_SA */
        "Syntax: CANCEL nick",
        /* LNG_CANCEL_SUCCESS */
        "Nick successfully cancelled. The nick can now be re-registered.",
        /* LNG_CANCEL_NO_REQUEST */
        "There is no request pending for %s."
    };
    moduleInsertLanguage(LANG_EN_US, LNG_NUM_STRINGS, langtable_en_us);
}

