/* hs_request.c - Add request and activate functionality to HostServ,
 *                along with adding +req as optional param to HostServ list.
 *
 * (C) 2003-2005 Anope Team
 * Contact us at info@anope.org
 *
 * Based on the original module by Rob <rob@anope.org>
 * Included in the Anope module pack since Anope 1.7.11
 * Anope Coder: GeniusDex <geniusdex@anope.org>
 *
 * Modified by zefie to add OperPM Ability
 *
 * Please read COPYING and README for further details.
 *
 * Send bug reports to the Anope Coder instead of the module
 * author, because any changes since the inclusion into anope
 * are not supported by the original author.
 */

#include "module.h"

#define AUTHOR "Rob, modified by zefie"
#define VERSION "1.1"

/* Configuration variables */
int HSRequestMemoUser = 0;
int HSRequestMemoOper = 0;
int HSRequestMemoSetters = 0;
int HSSendOperPM = 0;
char *HSSendOperPMDest = NULL;
char *HSRequestDBName = NULL;

#define HSREQ_DEFAULT_DBNAME "hs_request.db"

/* Language defines */
#define LNG_NUM_STRINGS 21

#define LNG_REQUEST_SYNTAX		0
#define LNG_REQUESTED			1
#define LNG_REQUEST_WAIT		2
#define LNG_REQUEST_MEMO		3
#define LNG_ACTIVATE_SYNTAX		4
#define LNG_ACTIVATED			5
#define LNG_ACTIVATE_MEMO		6
#define LNG_REJECT_SYNTAX		7
#define LNG_REJECTED			8
#define LNG_REJECT_MEMO			9
#define LNG_REJECT_MEMO_REASON	10
#define LNG_NO_REQUEST			11
#define LNG_HELP				12
#define LNG_HELP_SETTER			13
#define LNG_HELP_REQUEST		14
#define LNG_HELP_ACTIVATE		15
#define LNG_HELP_ACTIVATE_MEMO	16
#define LNG_HELP_REJECT			17
#define LNG_HELP_REJECT_MEMO	18
#define LNG_WAITING_SYNTAX		19
#define LNG_HELP_WAITING		20

int hs_do_request(User * u);
int hs_do_activate(User * u);
int hs_do_reject(User * u);
int hs_do_list_out(User * u);

int hs_help_request(User * u);
int hs_help_activate(User * u);
int hs_help_reject(User * u);
int hs_help_waiting(User * u);
void hs_help(User * u);

void my_add_host_request(char *nick, char *vIdent, char *vhost,
                         char *creator, int32 tmp_time);
int my_isvalidchar(const char c);
void my_memo_lang(User * u, char *name, int z, int number, ...);
void req_send_memos(User * u, char *vHost);
void show_list(User * u);
int hs_do_waiting(User * u);
int ns_do_drop(User * u);

void hsreq_save_db(void);
void hsreq_load_db(void);
int hsreqevt_db_saving(int argc, char **argv);

void my_load_config(void);
void my_add_languages(void);

HostCore *hs_request_head;

int AnopeInit(int argc, char **argv)
{
    Command *c;
    EvtHook *hook;

    c = createCommand("request", hs_do_request, nick_identified, -1, -1,
                      -1, -1, -1);
    moduleAddHelp(c, hs_help_request);
    moduleAddCommand(HOSTSERV, c, MOD_HEAD);

    c = createCommand("activate", hs_do_activate, is_host_setter, -1, -1,
                      -1, -1, -1);
    moduleAddHelp(c, hs_help_activate);
    moduleAddCommand(HOSTSERV, c, MOD_HEAD);

    c = createCommand("reject", hs_do_reject, is_host_setter, -1, -1, -1,
                      -1, -1);
    moduleAddHelp(c, hs_help_reject);
    moduleAddCommand(HOSTSERV, c, MOD_HEAD);

    c = createCommand("waiting", hs_do_waiting, is_host_setter, -1, -1, -1,
                      -1, -1);
    moduleAddHelp(c, hs_help_waiting);
    moduleAddCommand(HOSTSERV, c, MOD_HEAD);

    c = createCommand("list", hs_do_list_out, is_services_oper, -1, -1, -1,
                      -1, -1);
    moduleAddCommand(HOSTSERV, c, MOD_HEAD);

    c = createCommand("drop", ns_do_drop, NULL, -1, -1, -1, -1, -1);
    moduleAddCommand(NICKSERV, c, MOD_HEAD);

    hook = createEventHook(EVENT_DB_SAVING, hsreqevt_db_saving);
    moduleAddEventHook(hook);

    moduleSetHostHelp(hs_help);
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);

    my_load_config();
    my_add_languages();
    hs_request_head = NULL;

    if (debug)
        alog("[hs_request] Loading database...");
    hsreq_load_db();
    alog("hs_request loaded");
    return MOD_CONT;
}

void AnopeFini(void)
{
    if (debug)
        alog("[hs_request] Saving database...");
    hsreq_save_db();

    /* Clean up all open host requests */
    while (hs_request_head)
        hs_request_head = deleteHostCore(hs_request_head, NULL);

    free(HSRequestDBName);
    alog("hs_request un-loaded");
}

int hs_do_request(User * u)
{
    char *cur_buffer;
    char *nick;
    char *rawhostmask;
    char hostmask[HOSTMAX];
    NickAlias *na;
    int32 tmp_time;
    char *s;
    char *vIdent = NULL;
    time_t now = time(NULL);

    cur_buffer = moduleGetLastBuffer();
    nick = u->nick;
    rawhostmask = myStrGetToken(cur_buffer, ' ', 0);

    if (!nick || !rawhostmask) {
        if (rawhostmask)
            free(rawhostmask);
        moduleNoticeLang(s_HostServ, u, LNG_REQUEST_SYNTAX);
        return MOD_STOP;
    }

    vIdent = myStrGetOnlyToken(rawhostmask, '@', 0);    /* Get the first substring, @ as delimiter */
    if (vIdent) {
        rawhostmask = myStrGetTokenRemainder(rawhostmask, '@', 1);      /* get the remaining string */
        if (!rawhostmask) {
            moduleNoticeLang(s_HostServ, u, LNG_REQUEST_SYNTAX);
            free(vIdent);
            return MOD_STOP;
        }
        if (strlen(vIdent) > USERMAX - 1) {
            notice_lang(s_HostServ, u, HOST_SET_IDENTTOOLONG, USERMAX);
            free(vIdent);
            free(rawhostmask);
            return MOD_STOP;
        } else {
            for (s = vIdent; *s; s++) {
                if (!my_isvalidchar(*s)) {
                    notice_lang(s_HostServ, u, HOST_SET_IDENT_ERROR);
                    free(vIdent);
                    free(rawhostmask);
                    return MOD_STOP;
                }
            }
        }
        if (!ircd->vident) {
            notice_lang(s_HostServ, u, HOST_NO_VIDENT);
            free(vIdent);
            free(rawhostmask);
            return MOD_STOP;
        }
    }
    if (strlen(rawhostmask) < HOSTMAX - 1) {
        snprintf(hostmask, HOSTMAX, "%s", rawhostmask);
    } else {
        notice_lang(s_HostServ, u, HOST_SET_TOOLONG, HOSTMAX);
        if (vIdent)
            free(vIdent);
        free(rawhostmask);
        return MOD_STOP;
    }

    if (!isValidHost(hostmask, 3)) {
        notice_lang(s_HostServ, u, HOST_SET_ERROR);
        if (vIdent)
            free(vIdent);
        free(rawhostmask);
        return MOD_STOP;
    }

    tmp_time = time(NULL);
    if ((na = findnick(nick))) {
        if (HSRequestMemoOper || HSRequestMemoSetters) {
            if (MSSendDelay > 0 && u
                && u->lastmemosend + MSSendDelay > now) {
                moduleNoticeLang(s_HostServ, u, LNG_REQUEST_WAIT,
                                 MSSendDelay);
                u->lastmemosend = now;
                if (vIdent)
                    free(vIdent);
                free(rawhostmask);
                return MOD_STOP;
            }
        }

		if (HSSendOperPM) {
			privmsg(s_HostServ,HSSendOperPMDest,"[VHOST] New vHost Request: Username: %s vHost: %s", nick, hostmask);
			privmsg(s_HostServ,HSSendOperPMDest,"[VHOST] To allow this vHost: /msg %s ACTIVATE %s ~ To reject this vHost: /msg %s REJECT %s", s_HostServ, nick, s_HostServ, nick);
		}

        my_add_host_request(nick, vIdent, hostmask, u->nick, tmp_time);
        moduleNoticeLang(s_HostServ, u, LNG_REQUESTED);
        req_send_memos(u, hostmask);
        alog("New vHost Requested by %s", nick);
    } else {
        notice_lang(s_HostServ, u, HOST_NOREG, nick);
    }

    if (vIdent)
        free(vIdent);
    free(rawhostmask);

    return MOD_STOP;
}

void my_memo_lang(User * u, char *name, int z, int number, ...)
{
    va_list va;
    char buffer[4096], outbuf[4096];
    char *fmt = NULL;
    int lang = LANG_EN_US;
    char *s, *t, *buf;
    User *u2;

    if ((mod_current_module_name)
        && (!mod_current_module
            || strcmp(mod_current_module_name, mod_current_module->name)))
        mod_current_module = findModule(mod_current_module_name);

    u2 = finduser(name);
    /* Find the users lang, and use it if we cant */
    if (u2 && u2->na && u2->na->nc)
        lang = u2->na->nc->language;

    /* If the users lang isnt supported, drop back to enlgish */
    if (mod_current_module->lang[lang].argc == 0)
        lang = LANG_EN_US;

    /* If the requested lang string exists for the language */
    if (mod_current_module->lang[lang].argc > number) {
        fmt = mod_current_module->lang[lang].argv[number];

        buf = sstrdup(fmt);
        s = buf;
        while (*s) {
            t = s;
            s += strcspn(s, "\n");
            if (*s)
                *s++ = '\0';
            strscpy(outbuf, t, sizeof(outbuf));

            va_start(va, number);
            vsnprintf(buffer, 4095, outbuf, va);
            va_end(va);
            memo_send(u, name, buffer, z);
        }
        free(buf);
    } else {
        alog("%s: INVALID language string call, language: [%d], String [%d]", mod_current_module->name, lang, number);
    }
}


void req_send_memos(User * u, char *vHost)
{
    int i;
    int z = 2;

    if (checkDefCon(DEFCON_NO_NEW_MEMOS))
        return;

    if (HSRequestMemoOper == 1) {
        for (i = 0; i < servopers.count; i++) {
            my_memo_lang(u, (((NickCore *) servopers.list[i])->display), z,
                         LNG_REQUEST_MEMO, vHost);
        }
        for (i = 0; i < servadmins.count; i++) {
            my_memo_lang(u, (((NickCore *) servadmins.list[i])->display),
                         z, LNG_REQUEST_MEMO, vHost);
        }
        for (i = 0; i < RootNumber; i++) {
            my_memo_lang(u, ServicesRoots[i], z, LNG_REQUEST_MEMO, vHost);
        }
    }
    if (HSRequestMemoSetters == 1) {
        for (i = 0; i < HostNumber; i++) {
            my_memo_lang(u, HostSetters[i], z, LNG_REQUEST_MEMO, vHost);
        }
    }
}

int ns_do_drop(User * u)
{
    HostCore *tmp;
    boolean found = false;
    NickAlias *na;

    na = findnick(u->nick);
    tmp = findHostCore(hs_request_head, u->nick, &found);

    if (found && na)
        hs_request_head = deleteHostCore(hs_request_head, tmp);

    return MOD_CONT;
}

int hs_do_reject(User * u)
{
    char *cur_buffer;
    char *nick;
    char *reason;
    HostCore *tmp, *hc;
    boolean found = false;

    cur_buffer = moduleGetLastBuffer();
    nick = myStrGetToken(cur_buffer, ' ', 0);
    reason = myStrGetTokenRemainder(cur_buffer, ' ', 1);

    if (!nick) {
        moduleNoticeLang(s_HostServ, u, LNG_REJECT_SYNTAX);
        if (reason)
            free(reason);
        return MOD_STOP;
    }

    tmp = findHostCore(hs_request_head, nick, &found);
    if (found) {
        if (!tmp)
            hc = hs_request_head;
        else
            hc = tmp->next;

        if (HSRequestMemoUser) {
            if (reason)
                my_memo_lang(u, hc->nick, 2, LNG_REJECT_MEMO_REASON,
                             reason);
            else
                my_memo_lang(u, hc->nick, 2, LNG_REJECT_MEMO);
        }

	    if (HSSendOperPM) {
			if(reason)
				privmsg(s_HostServ,HSSendOperPMDest,"[VHOST] %s has REJECTED %s's vHost (%s)", u->nick, hc->nick, reason);
			else
				privmsg(s_HostServ,HSSendOperPMDest,"[VHOST] %s has REJECTED %s's vHost (No Reason Given)", u->nick, hc->nick);
	    }

        hs_request_head = deleteHostCore(hs_request_head, tmp);
        moduleNoticeLang(s_HostServ, u, LNG_REJECTED, nick);
        alog("Host Request for %s rejected by %s (%s)", nick, u->nick,
             reason ? reason : "");
    } else {
        moduleNoticeLang(s_HostServ, u, LNG_NO_REQUEST, nick);
    }

    free(nick);
    if (reason)
        free(reason);

    return MOD_STOP;
}

int hs_do_activate(User * u)
{
    char *cur_buffer;
    char *nick;
    NickAlias *na;
    HostCore *tmp, *hc;
    boolean found = false;

    cur_buffer = moduleGetLastBuffer();
    nick = myStrGetToken(cur_buffer, ' ', 0);

    if (!nick) {
        moduleNoticeLang(s_HostServ, u, LNG_ACTIVATE_SYNTAX);
        return MOD_STOP;
    }

    if ((na = findnick(nick))) {
        tmp = findHostCore(hs_request_head, nick, &found);
        if (found) {
            if (!tmp)
                hc = hs_request_head;
            else
                hc = tmp->next;

            addHostCore(hc->nick, hc->vIdent, hc->vHost, u->nick,
                        time(NULL));

            if (HSRequestMemoUser)
                my_memo_lang(u, hc->nick, 2, LNG_ACTIVATE_MEMO);

		    if (HSSendOperPM)
				privmsg(s_HostServ,HSSendOperPMDest,"[VHOST] %s has ACTIVATED %s's vHost", u->nick, hc->nick);

            hs_request_head = deleteHostCore(hs_request_head, tmp);
            moduleNoticeLang(s_HostServ, u, LNG_ACTIVATED, nick);
            alog("Host Request for %s activated by %s", nick, u->nick);
        } else {
            moduleNoticeLang(s_HostServ, u, LNG_NO_REQUEST, nick);
        }
    } else {
        /* Should be "User Not Found" instead */
        moduleNoticeLang(s_HostServ, u, LNG_ACTIVATE_SYNTAX);
    }

    free(nick);
    return MOD_STOP;
}


void my_add_host_request(char *nick, char *vIdent, char *vhost,
                         char *creator, int32 tmp_time)
{
    HostCore *tmp;
    boolean found = false;

    if (!hs_request_head) {
        hs_request_head =
            createHostCorelist(hs_request_head, nick, vIdent, vhost,
                               creator, tmp_time);
    } else {
        tmp = findHostCore(hs_request_head, nick, &found);
        if (!found) {
            hs_request_head =
                insertHostCore(hs_request_head, tmp, nick, vIdent, vhost,
                               creator, tmp_time);
        } else {
            hs_request_head = deleteHostCore(hs_request_head, tmp);     /* delete the old entry */
            my_add_host_request(nick, vIdent, vhost, creator, tmp_time);        /* recursive call to add new entry */
        }
    }
}

int my_isvalidchar(const char c)
{
    if (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'))
        || ((c >= '0') && (c <= '9')) || (c == '.') || (c == '-'))
        return 1;
    else
        return 0;
}

int hs_do_list_out(User * u)
{
    char *key;

    key = moduleGetLastBuffer();
    if (!key)
        return MOD_CONT;

    if (stricmp(key, "+req") != 0)
        return MOD_CONT;

    show_list(u);

    return MOD_STOP;
}

int hs_do_waiting(User * u)
{
    show_list(u);

    return MOD_CONT;
}

void show_list(User * u)
{
    struct tm *tm;
    char buf[BUFSIZE];
    int counter = 1;
    int from = 0, to = 0;
    int display_counter = 0;
    HostCore *current;

    current = hs_request_head;
    while (current) {
        if ((((counter >= from) && (counter <= to))
             || ((from == 0) && (to == 0)))
            && (display_counter < NSListMax)) {
            display_counter++;
            tm = localtime(&current->time);
            strftime(buf, sizeof(buf),
                     getstring(NULL, STRFTIME_DATE_TIME_FORMAT), tm);
            if (current->vIdent)
                notice_lang(s_HostServ, u, HOST_IDENT_ENTRY, counter,
                            current->nick, current->vIdent, current->vHost,
                            current->creator, buf);
            else
                notice_lang(s_HostServ, u, HOST_ENTRY, counter,
                            current->nick, current->vHost,
                            current->creator, buf);
        }
        counter++;
        current = current->next;
    }
    notice_lang(s_HostServ, u, HOST_LIST_FOOTER, display_counter);
}

int hs_help_request(User * u)
{
    moduleNoticeLang(s_HostServ, u, LNG_REQUEST_SYNTAX);
    notice(s_HostServ, u->nick, " ");
    moduleNoticeLang(s_HostServ, u, LNG_HELP_REQUEST);

    return MOD_CONT;
}

int hs_help_activate(User * u)
{
    if (is_host_setter(u)) {
        moduleNoticeLang(s_HostServ, u, LNG_ACTIVATE_SYNTAX);
        notice(s_HostServ, u->nick, " ");
        moduleNoticeLang(s_HostServ, u, LNG_HELP_ACTIVATE);
        if (HSRequestMemoUser)
            moduleNoticeLang(s_HostServ, u, LNG_HELP_ACTIVATE_MEMO);
    } else {
        notice_lang(s_HostServ, u, NO_HELP_AVAILABLE, "ACTIVATE");
    }

    return MOD_CONT;
}

int hs_help_reject(User * u)
{
    if (is_host_setter(u)) {
        moduleNoticeLang(s_HostServ, u, LNG_REJECT_SYNTAX);
        notice(s_HostServ, u->nick, " ");
        moduleNoticeLang(s_HostServ, u, LNG_HELP_REJECT);
        if (HSRequestMemoUser)
            moduleNoticeLang(s_HostServ, u, LNG_HELP_REJECT_MEMO);
    } else {
        notice_lang(s_HostServ, u, NO_HELP_AVAILABLE, "REJECT");
    }

    return MOD_CONT;
}

int hs_help_waiting(User * u)
{
    if (is_host_setter(u)) {
        moduleNoticeLang(s_HostServ, u, LNG_WAITING_SYNTAX);
        notice(s_HostServ, u->nick, " ");
        moduleNoticeLang(s_HostServ, u, LNG_HELP_WAITING);
    } else {
        notice_lang(s_HostServ, u, NO_HELP_AVAILABLE, "WAITING");
    }

    return MOD_CONT;
}

void hs_help(User * u)
{
    moduleNoticeLang(s_HostServ, u, LNG_HELP);
    if (is_host_setter(u))
        moduleNoticeLang(s_HostServ, u, LNG_HELP_SETTER);
}
void hsreq_load_db(void)
{
    FILE *fp;
    char *filename;
    char readbuf[1024];
    char *nick, *vident, *vhost, *creator, *tmp;
    int32 tmp_time;
    char *buf;

    if (HSRequestDBName)
        filename = HSRequestDBName;
    else
        filename = HSREQ_DEFAULT_DBNAME;

    fp = fopen(filename, "r");
    if (!fp) {
        alog("[hs_request] Unable to open database ('%s') for reading",
             filename);
        return;
    }

    while (fgets(readbuf, 1024, fp)) {
        buf = normalizeBuffer(readbuf);
        if (buf || *buf) {
            nick = myStrGetToken(buf, ':', 0);
            vident = myStrGetToken(buf, ':', 1);
            vhost = myStrGetToken(buf, ':', 2);
            tmp = myStrGetToken(buf, ':', 3);
            if (tmp) {
                tmp_time = strtol(tmp, (char **) NULL, 16);
                free(tmp);
            } else {
                tmp_time = 0;
            }
            creator = myStrGetToken(buf, ':', 4);
            if (!nick || !vident || !vhost || !creator) {
                alog("[hs_request] Error while reading database, skipping record");
                continue;
            }
            if (stricmp(vident, "(null)") == 0) {
                free(vident);
                vident = NULL;
            }
            my_add_host_request(nick, vident, vhost, creator, tmp_time);
            free(nick);
            free(vhost);
            free(creator);
            if (vident)
                free(vident);
        }
        free(buf);
    }

    fclose(fp);

    if (debug)
        alog("[hs_request] Succesfully loaded database");
}

void hsreq_save_db(void)
{
    FILE *fp;
    char *filename;
    char *vident;
    HostCore *current;

    if (HSRequestDBName)
        filename = HSRequestDBName;
    else
        filename = HSREQ_DEFAULT_DBNAME;

    fp = fopen(filename, "w");
    if (!fp) {
        alog("[hs_request] Unable to open database ('%s') for writing",
             filename);
        return;
    }

    current = hs_request_head;
    while (current) {
        vident = (current->vIdent ? current->vIdent : "(null)");
        fprintf(fp, "%s:%s:%s:%X:%s\n", current->nick, vident,
                current->vHost, (uint32) current->time, current->creator);
        current = current->next;
    }

    fclose(fp);

    if (debug)
        alog("[hs_request] Succesfully saved database");
}

int hsreqevt_db_saving(int argc, char **argv)
{
    if ((argc >= 1) && (stricmp(argv[0], EVENT_START) == 0))
        hsreq_save_db();

    return MOD_CONT;
}

void my_load_config(void)
{
    int i;
    char *tmp = NULL;

    Directive confvalues[][1] = {
        {{"HSRequestMemoUser",
          {{PARAM_SET, PARAM_RELOAD, &HSRequestMemoUser}}}},
        {{"HSRequestMemoOper",
          {{PARAM_SET, PARAM_RELOAD, &HSRequestMemoOper}}}},
        {{"HSRequestMemoSetters",
          {{PARAM_SET, PARAM_RELOAD, &HSRequestMemoSetters}}}},
        {{"HSSendOperPM",
          {{PARAM_SET, PARAM_RELOAD, &HSSendOperPM}}}},
        {{"HSSendOperPMDest", {{PARAM_STRING, PARAM_RELOAD, &HSSendOperPMDest}}}},
        {{"HSRequestDBName", {{PARAM_STRING, PARAM_RELOAD, &tmp}}}}
    };

    for (i = 0; i < 6; i++)
        moduleGetConfigDirective(confvalues[i]);

    if (tmp) {
        if (HSRequestDBName)
            free(HSRequestDBName);
        HSRequestDBName = sstrdup(tmp);
    } else {
        HSRequestDBName = sstrdup(HSREQ_DEFAULT_DBNAME);
    }

    if (debug)
        alog("debug: [hs_request] Set config vars: MemoUser=%d MemoOper=%d MemoSetters=%d OperPM=%d OperPMDest='%s' DBName='%s'", HSRequestMemoUser, HSRequestMemoOper, HSRequestMemoSetters, HSSendOperPM, HSSendOperPMDest, HSRequestDBName);
}

void my_add_languages(void)
{
    char *langtable_en_us[] = {
        /* LNG_REQUEST_SYNTAX */
        "Syntax: \002REQUEST \037vhost\037\002",
        /* LNG_REQUESTED */
        "Your vHost has been requested",
        /* LNG_REQUEST_WAIT */
        "Please wait %d seconds before requesting a new vHost",
        /* LNG_REQUEST_MEMO */
        "[auto memo] vHost \002%s\002 has been requested.",
        /* LNG_ACTIVATE_SYNTAX */
        "Syntax: \002ACTIVATE \037nick\037\002",
        /* LNG_ACTIVATED */
        "vHost for %s has been activated",
        /* LNG_ACTIVATE_MEMO */
        "[auto memo] Your requested vHost has been approved.",
        /* LNG_REJECT_SYNTAX */
        "Syntax: \002REJECT \037nick\037\002",
        /* LNG_REJECTED */
        "vHost for %s has been rejected",
        /* LNG_REJECT_MEMO */
        "[auto memo] Your requested vHost has been rejected.",
        /* LNG_REJECT_MEMO_REASON */
        "[auto memo] Your requested vHost has been rejected. Reason: %s",
        /* LNG_NO_REQUEST */
        "No request for nick %s found.",
        /* LNG_HELP */
        "    REQUEST     Request a vHost for your nick",
        /* LNG_HELP_SETTER */
        "    ACTIVATE    Approve the requested vHost of a user\n"
            "    REJECT      Reject the requested vHost of a user\n"
            "    WAITING     Convenience command for LIST +req",
        /* LNG_HELP_REQUEST */
        "Request the given vHost to be actived for your nick by the\n"
            "network administrators. Please be patient while your request\n"
            "is being considered.",
        /* LNG_HELP_ACTIVATE */
        "Activate the requested vHost for the given nick.",
        /* LNG_HELP_ACTIVATE_MEMO */
        "A memo informing the user will also be sent.",
        /* LNG_HELP_REJECT */
        "Reject the requested vHost for the given nick.",
        /* LNG_HELP_REJECT_MEMO */
        "A memo informing the user will also be sent.",
        /* LNG_WAITING_SYNTAX */
        "Syntax: \002WAITING\002",
        /* LNG_HELP_WAITING */
        "This command is provided for convenience. It is essentially\n"
            "the same as performing a LIST +req ."
    };

    char *langtable_nl[] = {
        /* LNG_REQUEST_SYNTAX */
        "Gebruik: \002REQUEST \037vhost\037\002",
        /* LNG_REQUESTED */
        "Je vHost is aangevraagd",
        /* LNG_REQUEST_WAIT */
        "Wacht %d seconden voor je een nieuwe vHost aanvraagt",
        /* LNG_REQUEST_MEMO */
        "[auto memo] vHost \002%s\002 is aangevraagd.",
        /* LNG_ACTIVATE_SYNTAX */
        "Gebruik: \002ACTIVATE \037nick\037\002",
        /* LNG_ACTIVATED */
        "vHost voor %s is geactiveerd",
        /* LNG_ACTIVATE_MEMO */
        "[auto memo] Je aangevraagde vHost is geaccepteerd.",
        /* LNG_REJECT_SYNTAX */
        "Gebruik: \002REJECT \037nick\037\002",
        /* LNG_REJECTED */
        "vHost voor %s is afgekeurd",
        /* LNG_REJECT_MEMO */
        "[auto memo] Je aangevraagde vHost is afgekeurd.",
        /* LNG_REJECT_MEMO_REASON */
        "[auto memo] Je aangevraagde vHost is afgekeurd. Reden: %s",
        /* LNG_NO_REQUEST */
        "Geen aanvraag voor nick %s gevonden.",
        /* LNG_HELP */
        "    REQUEST     Vraag een vHost aan voor je nick",
        /* LNG_HELP_SETTER */
        "    ACTIVATE    Activeer de aangevraagde vHost voor een gebruiker\n"
            "    REJECT      Keur de aangevraagde vHost voor een gebruiker af\n"
            "    WAITING     Snelkoppeling naar LIST +req",
        /* LNG_HELP_REQUEST */
        "Verzoek de gegeven vHost te activeren voor jouw nick bij de\n"
            "netwerk beheerders. Het kan even duren voordat je aanvraag\n"
            "afgehandeld wordt.",
        /* LNG_HELP_ACTIVATE */
        "Activeer de aangevraagde vHost voor de gegeven nick.",
        /* LNG_HELP_ACTIVATE_MEMO */
        "Een memo die de gebruiker op de hoogste stelt zal ook worden verstuurd.",
        /* LNG_HELP_REJECT */
        "Keur de aangevraagde vHost voor de gegeven nick af.",
        /* LNG_HELP_REJECT_MEMO */
        "Een memo die de gebruiker op de hoogste stelt zal ook worden verstuurd.",
        /* LNG_WAITING_SYNTAX */
        "Gebruik: \002WAITING\002",
        /* LNG_HELP_WAITING */
        "Dit commando is beschikbaar als handigheid. Het is simpelweg\n"
            "hetzelfde als LIST +req ."
    };

    char *langtable_pt[] = {
        /* LNG_REQUEST_SYNTAX */
        "Sintaxe: \002REQUEST \037vhost\037\002",
        /* LNG_REQUESTED */
        "Seu pedido de vHost foi encaminhado",
        /* LNG_REQUEST_WAIT */
        "Por favor, espere %d segundos antes de fazer um novo pedido de vHost",
        /* LNG_REQUEST_MEMO */
        "[Auto Memo] O vHost \002%s\002 foi solicitado.",
        /* LNG_ACTIVATE_SYNTAX */
        "Sintaxe: \002ACTIVATE \037nick\037\002",
        /* LNG_ACTIVATED */
        "O vHost para %s foi ativado",
        /* LNG_ACTIVATE_MEMO */
        "[Auto Memo] Seu pedido de vHost foi aprovado.",
        /* LNG_REJECT_SYNTAX */
        "Sintaxe: \002REJECT \037nick\037\002",
        /* LNG_REJECTED */
        "O vHost de %s foi recusado",
        /* LNG_REJECT_MEMO */
        "[Auto Memo] Seu pedido de vHost foi recusado.",
        /* LNG_REJECT_MEMO_REASON */
        "[Auto Memo] Seu pedido de vHost foi recusado. Motivo: %s",
        /* LNG_NO_REQUEST */
        "Nenhum pedido encontrado para o nick %s.",
        /* LNG_HELP */
        "    REQUEST     Request a vHost for your nick",
        /* LNG_HELP_SETTER */
        "    ACTIVATE    Aprova o pedido de vHost de um usu�rio\n"
            "    REJECT      Recusa o pedido de vHost de um usu�rio\n"
            "    WAITING     Comando para LISTAR +req",
        /* LNG_HELP_REQUEST */
        "Solicita a ativa��o do vHost fornecido em seu nick pelos\n"
            "administradores da rede. Por favor, tenha paci�ncia\n"
            "enquanto seu pedido � analisado.",
        /* LNG_HELP_ACTIVATE */
        "Ativa o vHost solicitado para o nick fornecido.",
        /* LNG_HELP_ACTIVATE_MEMO */
        "Um memo informando o usu�rio tamb�m ser� enviado.",
        /* LNG_HELP_REJECT */
        "Recusa o pedido de vHost para o nick fornecido.",
        /* LNG_HELP_REJECT_MEMO */
        "Um memo informando o usu�rio tamb�m ser� enviado.",
        /* LNG_WAITING_SYNTAX */
        "Sintaxe: \002WAITING\002",
        /* LNG_HELP_WAITING */
        "Este comando � usado por conveni�ncia. � essencialmente\n"
            "o mesmo que fazer um LIST +req"
    };

    char *langtable_it[] = {
        /* LNG_REQUEST_SYNTAX */
        "Sintassi: \002REQUEST \037vhost\037\002",
        /* LNG_REQUESTED */
        "Il tuo vHost � stato richiesto",
        /* LNG_REQUEST_WAIT */
        "Prego attendere %d secondi prima di richiedere un nuovo vHost",
        /* LNG_REQUEST_MEMO */
        "[auto memo] � stato richiesto il vHost \002%s\002.",
        /* LNG_ACTIVATE_SYNTAX */
        "Sintassi: \002ACTIVATE \037nick\037\002",
        /* LNG_ACTIVATED */
        "Il vHost per %s � stato attivato",
        /* LNG_ACTIVATE_MEMO */
        "[auto memo] Il vHost da te richiesto � stato approvato.",
        /* LNG_REJECT_SYNTAX */
        "Sintassi: \002REJECT \037nick\037\002",
        /* LNG_REJECTED */
        "Il vHost per %s � stato rifiutato",
        /* LNG_REJECT_MEMO */
        "[auto memo] Il vHost da te richiesto � stato rifiutato.",
        /* LNG_REJECT_MEMO_REASON */
        "[auto memo] Il vHost da te richiesto � stato rifiutato. Motivo: %s",
        /* LNG_NO_REQUEST */
        "Nessuna richiesta trovata per il nick %s.",
        /* LNG_HELP */
        "    REQUEST     Richiede un vHost per il tuo nick",
        /* LNG_HELP_SETTER */
        "    ACTIVATE    Approva il vHost richiesto di un utente\n"
            "    REJECT      Rifiuta il vHost richiesto di un utente\n"
            "    WAITING     Comando per LIST +req",
        /* LNG_HELP_REQUEST */
        "Richiede l'attivazione del vHost specificato per il tuo nick da parte\n"
            "degli amministratori di rete. Sei pregato di pazientare finch� la tua\n"
            "richiesta viene elaborata.",
        /* LNG_HELP_ACTIVATE */
        "Attiva il vHost richiesto per il nick specificato.",
        /* LNG_HELP_ACTIVATE_MEMO */
        "Viene inviato un memo per informare l'utente.",
        /* LNG_HELP_REJECT */
        "Rifiuta il vHost richiesto per il nick specificato.",
        /* LNG_HELP_REJECT_MEMO */
        "Viene inviato un memo per informare l'utente.",
        /* LNG_WAITING_SYNTAX */
        "Sintassi: \002WAITING\002",
        /* LNG_HELP_WAITING */
        "Questo comando � per comodit�. Praticamente � la stessa cosa che\n"
            "eseguire un LIST +req ."
    };
    moduleInsertLanguage(LANG_EN_US, LNG_NUM_STRINGS, langtable_en_us);
    moduleInsertLanguage(LANG_NL, LNG_NUM_STRINGS, langtable_nl);
    moduleInsertLanguage(LANG_PT, LNG_NUM_STRINGS, langtable_pt);
    moduleInsertLanguage(LANG_IT, LNG_NUM_STRINGS, langtable_it);
}

/* EOF */
