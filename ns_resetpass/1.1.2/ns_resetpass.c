/* ns_resetpass
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

/* Module information */
#define MYNAME "ns_resetpass"
#define AUTHOR "DrStein"
#define VERSION "1.1.1"

/*************************************************************************/

#define MODULEDATAKEY "passcode"
#define PASSCODELEN   11

/*************************************************************************/

/* Multi-language stuff */
#define RESETPASS_MAIL_SUBJECT  "Nickname Password Reset (%s)"
#define RESETPASS_MAIL_HEAD     "Hi,"
#define RESETPASS_MAIL_LINE_1   "A password reset has been requested for the nickname %s."
#define RESETPASS_MAIL_LINE_2   "Please type \" /msg %s ENTERCODE %s \" when using the nickname to reset the password."
#define RESETPASS_MAIL_LINE_3   "Note this passcode will expire, so reset your password as soon as posible."
#define RESETPASS_MAIL_LINE_4   "If you don't know why this mail is sent to you, please ignore it silently."
#define RESETPASS_MAIL_LINE_5   "PLEASE DON'T ANSWER TO THIS MAIL!"
#define RESETPASS_MAIL_LINE_6   "%s administrators."

#define LOG_RESETPASS_REQUESTED  "%s: user %s has just requested a password reset for %s."
#define LOG_COULDNT_SENDMAIL     "%s: couldn't send message to %s's e-mail address"
#define LOG_CREATE_COMMAND_ERROR "%s: an error ocurred when creating commands!"
#define LOG_PASSCODE_EXPIRED     "%s: passcode for %s has just expired"
#define LOG_CONFIG_LOADED        "[%s] Configuration directives loaded..."

#define LANG_NUM_STRINGS             15
#define RESETPASS_ALREADY_REQUESTED1  0
#define RESETPASS_ALREADY_REQUESTED2  1
#define RESETPASS_SUCCESS             2
#define RESETPASS_INSTRUC             3
#define RESETPASS_HELP                4
#define RESETPASS_HELP_CMD            5
#define RESETPASS_SYNTAX              6
#define RESETPASS_NOT_REQUESTED       7

#define ENTERCODE_WRONG_PASSCODE        8
#define ENTERCODE_SYNTAX                9
#define ENTERCODE_HELP                 10
#define ENTERCODE_HELP_CMD             11

#define RESENDCODE_SYNTAX            12
#define RESENDCODE_HELP              13
#define RESENDCODE_HELP_CMD          14

/*************************************************************************/

/* configuration variables */
int RestrictReset = 0;
int ExpirePassCode = 3600;

/*************************************************************************/

int do_resetpass(User * u);
int do_confirm(User * u);
int do_resendcode(User * u);
int do_sendmail(User * u, NickAlias * na);
int EntercodeResetHelp(User * u);
int ResetPassHelp(User * u);
int ResendPassCodeHelp(User * u);
int mLoadConfig(int argc, char **argv);
int expirePassCode(int argc, char **argv);
void mMainNickHelp(User * u);
void ns_setpassword(User * u, NickCore * nc);
void generatePassCode(char passcode[PASSCODELEN]);
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
    EvtHook *hook = NULL;
    Command *c;

    int status = 0;

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    moduleSetType(THIRD);

    /* Load configuration directives from services.conf */
    alog("[%s] Loading configuration directives", MYNAME);
    if (mLoadConfig(0, NULL))
        return MOD_STOP;

    hook = createEventHook(EVENT_RELOAD, mLoadConfig);
    status += moduleAddEventHook(hook);

    c = createCommand("RESETPASS", do_resetpass, NULL, -1, -1, -1, -1, -1);
    moduleAddHelp(c, ResetPassHelp);
    status += moduleAddCommand(NICKSERV, c, MOD_HEAD);

    c = createCommand("ENTERCODE", do_confirm, NULL, -1, -1, -1, -1, -1);
    moduleAddHelp(c, EntercodeResetHelp);
    status += moduleAddCommand(NICKSERV, c, MOD_HEAD);

    c = createCommand("RESENDCODE", do_resendcode, NULL, -1, -1, -1, -1,
                      -1);
    moduleAddHelp(c, ResendPassCodeHelp);
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

/*************************************************************************/

int do_resetpass(User * u)
{
    char *buffer = NULL;
    char *nick = NULL;
    char passcode[PASSCODELEN];
    char *callBackinfo[1];
    NickAlias *na = NULL;

    if ((RestrictReset == 1) && !(is_oper(u))) {
        notice_lang(s_NickServ, u, ACCESS_DENIED);
        return MOD_CONT;
    }

    buffer = moduleGetLastBuffer();
    if (buffer)
        nick = myStrGetToken(buffer, ' ', 0);

    if (!nick) {
        moduleNoticeLang(s_NickServ, u, RESETPASS_SYNTAX);
    } else if (!anope_valid_nick(nick)) {
        notice_lang(s_NickServ, u, NICK_X_FORBIDDEN, nick);
    } else if (!(na = findnick(nick))) {
        notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, nick);
    } else if (nick_identified(na->u)) {
        notice_lang(s_NickServ, u, NICK_ALREADY_IDENTIFIED);
    } else if (moduleGetData(&na->nc->moduleData, MODULEDATAKEY)) {
        moduleNoticeLang(s_NickServ, u, RESETPASS_ALREADY_REQUESTED1,
                         na->nick);
        moduleNoticeLang(s_NickServ, u, RESETPASS_ALREADY_REQUESTED2);
    } else {
        generatePassCode(passcode);
        moduleAddData(&na->nc->moduleData, MODULEDATAKEY, passcode);
        if (do_sendmail(u, na) != 0) {
            alog(LOG_COULDNT_SENDMAIL, MYNAME, na->nick);
            moduleDelData(&na->nc->moduleData, MODULEDATAKEY);
        } else {
            callBackinfo[0] = na->nick;
            moduleAddCallback(MYNAME, time(NULL) + ExpirePassCode, expirePassCode, 1, callBackinfo);
            alog(LOG_RESETPASS_REQUESTED, MYNAME, u->nick, na->nick);
            moduleNoticeLang(s_NickServ, u, RESETPASS_SUCCESS, na->nick);
            moduleNoticeLang(s_NickServ, u, RESETPASS_INSTRUC);
        }
    }

    return MOD_CONT;
}

int do_confirm(User * u)
{
    char *buffer = NULL;
    char *passcode = NULL;
    char *moduledata = NULL;
    NickAlias *na = NULL;

    buffer = moduleGetLastBuffer();

    if (buffer)
        passcode = myStrGetToken(buffer, ' ', 0);

    if (!passcode) {
        moduleNoticeLang(s_NickServ, u, ENTERCODE_SYNTAX);
    } else if (nick_identified(u)) {
        notice_lang(s_NickServ, u, NICK_ALREADY_IDENTIFIED);
    } else if (!(na = findnick(u->nick))) {
        notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, u->nick);
    } else
        if (!
            (moduledata =
             moduleGetData(&na->nc->moduleData, MODULEDATAKEY))) {
        moduleNoticeLang(s_NickServ, u, RESETPASS_NOT_REQUESTED, u->nick);
    } else if (strcmp(passcode, moduledata) != 0) {
        moduleNoticeLang(s_NickServ, u, ENTERCODE_WRONG_PASSCODE);
        free(moduledata);
    } else {
        ns_setpassword(u, na->nc);
        moduleDelData(&na->nc->moduleData, MODULEDATAKEY);
        free(moduledata);
    }

    return MOD_CONT;
}

int do_resendcode(User * u)
{
    char *buffer = NULL;
    char *nick = NULL;
    NickAlias *na = NULL;

    buffer = moduleGetLastBuffer();
    if (buffer)
        nick = myStrGetToken(buffer, ' ', 0);

    if (!nick) {
        moduleNoticeLang(s_NickServ, u, RESENDCODE_SYNTAX);
    } else if (!anope_valid_nick(nick)) {
        notice_lang(s_NickServ, u, NICK_X_FORBIDDEN, nick);
    } else if (!(na = findnick(nick))) {
        notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, nick);
    } else if (!(moduleGetData(&na->nc->moduleData, MODULEDATAKEY))) {
        moduleNoticeLang(s_NickServ, u, RESETPASS_NOT_REQUESTED, u->nick);
    } else {
        if (do_sendmail(u, na) != 0) {
            alog(LOG_COULDNT_SENDMAIL, MYNAME, na->nick);
        } else {
            moduleNoticeLang(s_NickServ, u, RESETPASS_SUCCESS, na->nick);
            moduleNoticeLang(s_NickServ, u, RESETPASS_INSTRUC);
        }
    }

    return MOD_CONT;
}

/*************************************************************************/

int do_sendmail(User * u, NickAlias * na)
{
    MailInfo *mail = NULL;
    char buf[BUFSIZE];

    if (!(na || u)) {
        return -1;
    }
    snprintf(buf, sizeof(buf), RESETPASS_MAIL_SUBJECT, na->nick);
    mail = MailBegin(u, na->nc, buf, s_NickServ);
    if (!mail) {
        return -1;
    }
    fprintf(mail->pipe, RESETPASS_MAIL_HEAD);
    fprintf(mail->pipe, "\n\n");
    fprintf(mail->pipe, RESETPASS_MAIL_LINE_1, na->nick);
    fprintf(mail->pipe, "\n\n");
    fprintf(mail->pipe, RESETPASS_MAIL_LINE_2, s_NickServ,
            moduleGetData(&na->nc->moduleData, MODULEDATAKEY));
    fprintf(mail->pipe, "\n\n");
    fprintf(mail->pipe, RESETPASS_MAIL_LINE_3);
    fprintf(mail->pipe, "\n\n");
    fprintf(mail->pipe, RESETPASS_MAIL_LINE_4);
    fprintf(mail->pipe, "\n\n");
    fprintf(mail->pipe, RESETPASS_MAIL_LINE_5);
    fprintf(mail->pipe, "\n\n");
    fprintf(mail->pipe, RESETPASS_MAIL_LINE_6, NetworkName);
    fprintf(mail->pipe, "\n.\n");
    MailEnd(mail);
    return 0;
}

void generatePassCode(char passcode[PASSCODELEN])
{
    int idx, min = 1, max = 62;
    int chars[] =
        { ' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
        'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
        'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
        'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
        'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
    };

    for (idx = 0; idx < 9; idx++) {
        passcode[idx] =
            chars[(1 +
                   (int) (((float) (max - min)) * getrandom16() /
                          (65535 + 1.0)) + min)];
    }

    passcode[idx] = '\0';
}

void ns_setpassword(User * u, NickCore * nc)
{
    int len = 0;
    char tmp_pass[PASSMAX];
    char password[PASSCODELEN];

    if (nc->pass)
        free(nc->pass);

    nc->pass = smalloc(PASSMAX);

    /* we'll generate the password just like we
     * did for the passcode */
    generatePassCode(password);

    len = strlen(password);

    if (enc_encrypt(password, len, nc->pass, PASSMAX) < 0) {
        memset(password, 0, len);
        alog("%s: Failed to encrypt password for %s (set)", MYNAME,
             nc->display);
        notice_lang(s_NickServ, u, NICK_SET_PASSWORD_FAILED);
        return;
    }

    if(enc_decrypt(nc->pass, tmp_pass, PASSMAX) == 1) {
        notice_lang(s_NickServ, u, NICK_SET_PASSWORD_CHANGED_TO, nc->pass);
    } else {
        notice_lang(s_NickServ, u, NICK_SET_PASSWORD_CHANGED_TO, password);
    }

    memset(password, 0, len);

    alog("%s: %s!%s@%s (e-mail: %s) password has been changed.", MYNAME,
         u->nick, u->username, u->host, nc->email);

    return;
}

/*************************************************************************/

/**
 * manages the multilanguage stuff
 **/
void m_AddLanguages(void)
{
    char *langtable_en_us[] = {
        /* RESETPASS_ALREADY_REQUESTED1 */
        "A password reset for %s has already been requested.",
        /* RESETPASS_ALREADY_REQUESTED2 */
        "If you didn't get the e-mail in a reasonable time, please use RESENDCODE command",
        /* RESETPASS_SUCCESS */
        "A passcode has been sent to %s's e-mail address.",
        /* RESETPASS_INSTRUC */
        "Please read and follow the instructions provided in the message.",
        /* RESETPASS_HELP */
        "Syntax: RESETPASS nick\n"
            "Allow you to reset your password in case you have it forgotten.\n"
            "A passcode shall be sent to the configured e-mail address of the\n"
            "given nickname. Once you receive the message, follow the included\n"
            "instructions in order to reset your password.",
        /* RESETPASS_HELP_CMD */
        "    RESETPASS  Reset the password of the given nickname",
        /* RESETPASS_SYNTAX */
        "Syntax: RESETPASS nick",
        /* RESETPASS_NOT_REQUESTED */
        "Nobody has requested a password reset for %s.",
        /* ENTERCODE_WRONG_PASSCODE */
        "The passcode you have just entered is wrong. Please verify the e-mail again.",
        /* ENTERCODE_SYNTAX */
        "Syntax: ENTERCODE passcode",
        /* ENTERCODE_HELP */
        "Syntax: ENTERCODE passcode\n"
            "Allow you to enter the passcode you received when trying to reset\n"
            "the password for you nickname.",
        /* ENTERCODE_HELP_CMD */
        "    ENTERCODE  Confirm your password reset operation.",
        /* RESENDCODE_SYNTAX */
        "Syntax: RESENDCODE nick",
        /* RESENDCODE_HELP */
        "Syntax: RESENDCODE nick\n"
            "Send the passcode for password reset again.\n"
            "Make sure the e-mail hasn't been marked as spam.",
        /* RESENDCODE_HELP_CMD */
        "    RESENDCODE Send the passcode for password reset again."
    };

    moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}

/*************************************************************************/

int ResetPassHelp(User * u)
{
    moduleNoticeLang(s_NickServ, u, RESETPASS_HELP);

    return MOD_CONT;
}

int EntercodeResetHelp(User * u)
{
    moduleNoticeLang(s_NickServ, u, ENTERCODE_HELP);

    return MOD_CONT;
}

int ResendPassCodeHelp(User * u)
{
    moduleNoticeLang(s_NickServ, u, RESENDCODE_HELP);

    return MOD_CONT;
}

/* This help will be added to the main NickServ list */
void mMainNickHelp(User * u)
{
    moduleNoticeLang(s_NickServ, u, RESETPASS_HELP_CMD);
    moduleNoticeLang(s_NickServ, u, ENTERCODE_HELP_CMD);
    moduleNoticeLang(s_NickServ, u, RESENDCODE_HELP_CMD);
}

/*************************************************************************/

int expirePassCode(int argc, char **argv)
{
    NickAlias *na = NULL;

    if ((na = findnick(argv[0]))) {
        alog(LOG_PASSCODE_EXPIRED, MYNAME, na->nick);
        moduleDelData(&na->nc->moduleData, MODULEDATAKEY);
    }

    return MOD_CONT;
}

/*************************************************************************/

/**
 * Load the configuration directives from Services configuration file.
 * @return 0 for success
 **/
int mLoadConfig(int argc, char **argv)
{
    int tmpRestrictReset = 0;
    int tmpExpirePassCode = -1;
    int i;

    Directive directivas[] = {
        {"RestrictReset", {{PARAM_SET, PARAM_RELOAD, &tmpRestrictReset}}},
        {"ExpirePassCode", {{PARAM_TIME, PARAM_RELOAD, &tmpExpirePassCode}}},
    };

    for (i = 0; i < 2; i++) {
        Directive *d = &directivas[i];
        moduleGetConfigDirective(d);
    }

    if (tmpRestrictReset == 1)
        RestrictReset = 1;
    else
        RestrictReset = 0;

    if (tmpExpirePassCode != -1) {
        ExpirePassCode = tmpExpirePassCode;
    }

    alog(LOG_CONFIG_LOADED, MYNAME);

    return MOD_CONT;
}

/*************************************************************************/
