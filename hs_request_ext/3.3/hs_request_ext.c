/**
 * -----------------------------------------------------------------------------
 * Name    : hs_request_ext
 * Author  : Viper <Viper@Anope.org>
 * Date    : 02/02/2008 (Last update: 20/01/2009)
 * Version : 3.3
 * -----------------------------------------------------------------------------
 * Requires    : Anope-1.8.0
 * Tested      : Anope 1.8.2 SVN + InspIRCd 1.2.0
 * -----------------------------------------------------------------------------
 * This module is a modified version of the hs_request that is shipped with Anope.
 * It provides some extra usefull features such as reporting requests to a channel
 * (other then logchan) and the ability to request a vhost through fantasy commands
 * (unlike ircd_vhostserv which sets them instead of requesting them).
 *
 * !!! This module REPLACES hs_request so should not be loaded alongside it !!!
 *
 * I originally never meant to release this publically, but a few people
 * have asked for functionality like this online, so i ll just publish it .
 *
 * Based on the original module by Rob <rob@anope.org>
 * Based on the version included in the Anope module pack
 * This module also uses parts of Trystan's hs_request_fant module.
 *
 * Please note that this extension to hs_request is released as a third party module 
 * and therefor not supported by the Anope Dev Crew.
 * -----------------------------------------------------------------------------
 *
 * Changelog:
 *
 *   3.3    Initial public release.
 *          Updated to work with and require Anope 1.8.
 *          Updated to use events. (We were still doing it the old way..)
 *          Added flags, some sanity checking and a supported mode.
 *
 *   3.2    Fixed numerous small bugs.
 *
 *   3.1    Private Release.
 *          Added some documentation.
 *
 *   3.0    Rewrite based on hs_request included in Anope.
 *
 * -----------------------------------------------------------------------------
 **/

/**
 * TODO:
 *
 *    - think about adding support for a filter or list with forbidden vhosts..
 *
 **/

/**
 * Configuration directives that should be copy-pasted to services.conf in 
 * addition to those already in the example.conf for the hs_request that
 * ships with anope..

# HSRequestSendChan [OPTIONAL]
# Module: hs_request_ext
#
# Set this to the channel you want services to report hostserv requests to.
# If not given, services will not report requests to a channel.
#
#HSRequestSendChan "#opers"

# HSRequestChan [OPTIONAL]
# Module: hs_request_ext
#
# Set this to the channel you want services to listen in for requests.
# If not given, services will not listen for requests, it will however still send help.
# If you want services to listen in all channels, leave this set, but empty (HSRequestChan "")
#
#HSRequestChan "#vhost"

# HSSendHelp [OPTIONAL]
# Module: hs_request_ext
#
# Set this if you want services to send help on a user types !vhost.
# Otherwise comment this out.
#
#HSSendHelp

 */

#include "module.h"

/*------------------------------ Configuration Block ----------------------------*/

/**
 * Undefining this will make you lose all support for hs_request_ext!
 *
 * This module checks whether unsupported modules are enabled/loaded in anope and
 * whether the anope version meets minimum requirements to load.
 * If an incompatible module is found, hs_request_ext will either unload or disable
 * itself (ex: os_raw), or continue in unsupported mode (ex: ircd_init).
 *
 * These measures have been added to protect users against themselves and prevent
 * the author of this module from being confronted with support queries about situations that
 * would not occor during normal use (for example when using RAW).
 *
 * If you undef this directive, these checks will be disabled, however you WILL LOSE ALL SUPPORT!!!
 * If you use the module in unsupported module ayways (because of a conflicting 3rd party module
 * for example) this may also be desirable.
 *
 * To undefine this replace "#define" by "#undef" or simply comment the line out.
 * Note that you may need to delete hs_request_ext.o and .so (*nix) to force a recompile.
 **/
#define SUPPORTED

/*------------------------- End of Configuration Block --------------------------*/


#define AUTHOR "Viper"
#define VERSION "3.3"

/* Configuration variables */
int Enabled;
int supported;
HostCore *hs_request_head;

int HSRequestMemoUser = 0;
int HSRequestMemoOper = 0;
int HSRequestMemoSetters = 0;
int HSSendHelp = 0;
char *HSRequestDBName = NULL;
char *HSRequestSendChan = NULL;
char *HSRequestChan = NULL;

#define HSREQ_DEFAULT_DBNAME "hs_request.db"


/* Language defines */
#define LNG_NUM_STRINGS 22

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
#define LNG_EXPL_HS				21


/* Functions */
int hs_do_request(User * u);
int do_fantasy(int ac, char **av);
int hs_request(User * u, char *cur_buffer);

int msg_vhost_help(User *u);
void myPrivateLang(char *source, User * u, int number, ...);

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
int hsreqevt_db_backup(int argc, char **argv);

int check_load(void);
void update_version(void);

void my_load_config(void);
int hsReloadConf(int argc, char **argv);

void my_add_languages(void);

/* ------------------------------------------------------------------------------- */

/**
 * Create the command, and tell anope about it.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv) {
	Command *c;
	EvtHook *hook;

	alog("[\002hs_request_ext\002] Loading module...");

	Enabled = 1;
#ifdef SUPPORTED
	if (!moduleMinVersion(1,8,0,1899)) {
		alog("[\002hs_request_ext\002] Your version of Anope isn't supported. Please update to a newer release.");
		return MOD_STOP;
	}
	supported = 1;
#else
	supported = -1;
#endif

	check_load();
	if (!Enabled)
		return MOD_STOP;

	c = createCommand("request", hs_do_request, nick_identified, -1, -1, -1, -1, -1);
	moduleAddHelp(c, hs_help_request);
	moduleAddCommand(HOSTSERV, c, MOD_HEAD);

	c = createCommand("activate", hs_do_activate, is_host_setter, -1, -1, -1, -1, -1);
	moduleAddHelp(c, hs_help_activate);
	moduleAddCommand(HOSTSERV, c, MOD_HEAD);

	c = createCommand("reject", hs_do_reject, is_host_setter, -1, -1, -1, -1, -1);
	moduleAddHelp(c, hs_help_reject);
	moduleAddCommand(HOSTSERV, c, MOD_HEAD);

	c = createCommand("waiting", hs_do_waiting, is_host_setter, -1, -1, -1, -1, -1);
	moduleAddHelp(c, hs_help_waiting);
	moduleAddCommand(HOSTSERV, c, MOD_HEAD);

	c = createCommand("list", hs_do_list_out, is_services_oper, -1, -1, -1, -1, -1);
	moduleAddCommand(HOSTSERV, c, MOD_HEAD);

	c = createCommand("drop", ns_do_drop, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(NICKSERV, c, MOD_HEAD);

	hook = createEventHook(EVENT_BOT_FANTASY, do_fantasy);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002hs_request_ext\002] Can't hook to EVENT_BOT_FANTASY event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_BOT_FANTASY_NO_ACCESS, do_fantasy);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002hs_request_ext\002] Can't hook to EVENT_BOT_FANTASY_NO_ACCESS event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_RELOAD, hsReloadConf);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002hs_request_ext\002] Can't hook to EVENT_RELOAD event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_DB_SAVING, hsreqevt_db_saving);
	moduleAddEventHook(hook);

	hook = createEventHook(EVENT_DB_BACKUP, hsreqevt_db_backup);
	moduleAddEventHook(hook);

	moduleSetHostHelp(hs_help);
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	my_load_config();
	my_add_languages();
	hs_request_head = NULL;

	if (debug)
		alog("[hs_request_ext] Loading database...");
	hsreq_load_db();

	update_version();

	alog("[\002hs_request_ext\002] Module loaded successfully...");
	return MOD_CONT;
}

void AnopeFini(void) {
	if (debug)
		alog("[hs_request_ext] Saving database...");
	hsreq_save_db();

	if (HSRequestSendChan)
		privmsg(s_HostServ, HSRequestSendChan, "[hs_request_ext] Module Unloading... no longer handling requests.");

	/* Clean up all open host requests */
	while (hs_request_head)
		hs_request_head = deleteHostCore(hs_request_head, NULL);

	free(HSRequestDBName);
	free(HSRequestSendChan);
	free(HSRequestChan);
	alog("hs_request_ext un-loaded");
}

/* ------------------------------------------------------------------------------- */

int hs_do_request(User * u) {
	char *cur_buffer;

	if (!Enabled)
		return MOD_CONT;

	cur_buffer = moduleGetLastBuffer();

	if (cur_buffer)
		return hs_request(u, cur_buffer);
	else
		return MOD_CONT;
}

/**
 * Handles all fantasy commands.
 * Here we ll identify the command and call the right routines.
 **/
int do_fantasy(int ac, char **av) {
	User *u;
	ChannelInfo *ci;
	Channel *c;
	int ret = MOD_CONT;

	if (!Enabled || !HSRequestChan)
		return MOD_CONT;

	/* Some basic error checking... should never match */
	if (ac < 3)
		return MOD_CONT;

	if (!(ci = cs_findchan(av[2])))
		return MOD_CONT;
	if (!(u = finduser(av[1])))
		return MOD_CONT;
	if (!(c = findchan(ci->name)))
		return MOD_CONT;

	if (stricmp(av[0], "vhost") == 0) {
		/* If executed without any params, show help */
		if (ac == 3)
			msg_vhost_help(u);
		else {
			if (((stricmp(ci->name, HSRequestChan)) == 0) || (stricmp(HSRequestChan, "")) == 0) {
				char *vhost = myStrGetToken(av[3],' ',0);
				ret = hs_request(u, vhost);
				free(vhost);
			} else 
				msg_vhost_help(u);
		}
	}

	/* Continue processig event.. maybe other modules want it too */
	return ret;
}

int hs_request(User * u, char *cur_buffer) {
	char *nick;
	char *rawhostmask;
	char hostmask[HOSTMAX];
	NickAlias *na;
	int32 tmp_time;
	char *s;
	char *vIdent = NULL;
	time_t now = time(NULL);

	nick = u->nick;
	rawhostmask = myStrGetToken(cur_buffer, ' ', 0);

	if (!nick || !rawhostmask) {
		if (rawhostmask)
			free(rawhostmask);
		moduleNoticeLang(s_HostServ, u, LNG_REQUEST_SYNTAX);
		return MOD_CONT;
	}

	vIdent = myStrGetOnlyToken(rawhostmask, '@', 0);    /* Get the first substring, @ as delimiter */
	if (vIdent) {
		rawhostmask = myStrGetTokenRemainder(rawhostmask, '@', 1);      /* get the remaining string */
		if (!rawhostmask) {
			moduleNoticeLang(s_HostServ, u, LNG_REQUEST_SYNTAX);
			free(vIdent);
			return MOD_CONT;
		}
		if (strlen(vIdent) > USERMAX - 1) {
			notice_lang(s_HostServ, u, HOST_SET_IDENTTOOLONG, USERMAX);
			free(vIdent);
			free(rawhostmask);
			return MOD_CONT;
		} else {
			for (s = vIdent; *s; s++) {
				if (!my_isvalidchar(*s)) {
					notice_lang(s_HostServ, u, HOST_SET_IDENT_ERROR);
					free(vIdent);
					free(rawhostmask);
					return MOD_CONT;
				}
			}
		}
		if (!ircd->vident) {
			notice_lang(s_HostServ, u, HOST_NO_VIDENT);
			free(vIdent);
			free(rawhostmask);
			return MOD_CONT;
		}
	}
	if (strlen(rawhostmask) < HOSTMAX - 1) {
		snprintf(hostmask, HOSTMAX, "%s", rawhostmask);
	} else {
		notice_lang(s_HostServ, u, HOST_SET_TOOLONG, HOSTMAX);
		if (vIdent)
			free(vIdent);
		free(rawhostmask);
		return MOD_CONT;
	}

	if (!isValidHost(hostmask, 3)) {
		notice_lang(s_HostServ, u, HOST_SET_ERROR);
		if (vIdent)
			free(vIdent);
		free(rawhostmask);
		return MOD_CONT;
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
				return MOD_CONT;
			}
		}
		my_add_host_request(nick, vIdent, hostmask, u->nick, tmp_time);

		moduleNoticeLang(s_HostServ, u, LNG_REQUESTED);
		req_send_memos(u, hostmask);
		alog("New vHost Requested by %s", nick);

		if (HSRequestSendChan) {
			if (vIdent) {
				privmsg(s_HostServ, HSRequestSendChan, "[\2vHost\2] New vhost request by %s [ %s@%s ]", nick, vIdent, rawhostmask);
			} else {
				privmsg(s_HostServ, HSRequestSendChan, "[\2vHost\2] New vhost request by %s [ %s ]", nick, rawhostmask);
			}
		}
	} else {
		notice_lang(s_HostServ, u, HOST_NOREG, nick);
	}

	if (vIdent)
		free(vIdent);
	free(rawhostmask);

	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

int msg_vhost_help(User *u) {
	if (HSSendHelp)
		myPrivateLang(s_HostServ, u, LNG_EXPL_HS);
	return MOD_CONT;
}

void myPrivateLang(char *source, User * u, int number, ...) {
	va_list va;
	char buffer[4096], outbuf[4096];
	char *fmt = NULL;
	int lang = NSDefLanguage;
	char *s, *t, *buf;

	if ((mod_current_module_name) && (!mod_current_module || strcmp(mod_current_module_name, mod_current_module->name))) {
		mod_current_module = findModule(mod_current_module_name);
	}

	/* Find the users lang, and use it if we can */
	if (u && u->na && u->na->nc) {
		lang = u->na->nc->language;
	}

	/* If the users lang isnt supported, drop back to English */
	if (mod_current_module->lang[lang].argc == 0) {
		lang = LANG_EN_US;
	}

	/* If the requested lang string exists for the language */
	if (mod_current_module->lang[lang].argc > number) {
		fmt = mod_current_module->lang[lang].argv[number];

		buf = sstrdup(fmt);
		va_start(va, number);
		vsnprintf(buffer, 4095, buf, va);
		va_end(va);
		s = buffer;
		while (*s) {
			t = s;
			s += strcspn(s, "\n");
			if (*s)
				*s++ = '\0';
			strscpy(outbuf, t, sizeof(outbuf));
			privmsg(source, u->nick, outbuf);
		}
		free(buf);
	} else {
		alog("%s: INVALID language string call, language: [%d], String [%d]", mod_current_module->name, lang, number);
	}
}

/* ------------------------------------------------------------------------------- */

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

/* ------------------------------------------------------------------------------- */

void req_send_memos(User * u, char *vHost) {
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

/* ------------------------------------------------------------------------------- */

int ns_do_drop(User * u) {
	HostCore *tmp;
	boolean found = false;
	NickAlias *na;

	na = findnick(u->nick);
	tmp = findHostCore(hs_request_head, u->nick, &found);

	if (found && na)
		hs_request_head = deleteHostCore(hs_request_head, tmp);

	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

int hs_do_reject(User * u) {
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
		return MOD_CONT;
	}

	tmp = findHostCore(hs_request_head, nick, &found);
	if (found) {
		if (!tmp)
			hc = hs_request_head;
		else
			hc = tmp->next;

		if (HSRequestMemoUser) {
			if (reason)
				my_memo_lang(u, hc->nick, 2, LNG_REJECT_MEMO_REASON, reason);
			else
				my_memo_lang(u, hc->nick, 2, LNG_REJECT_MEMO);
		}

		hs_request_head = deleteHostCore(hs_request_head, tmp);
		moduleNoticeLang(s_HostServ, u, LNG_REJECTED, nick);
		alog("Host Request for %s rejected by %s (%s)", nick, u->nick, reason ? reason : "");
		if (HSRequestSendChan) {
			privmsg(s_HostServ, HSRequestSendChan, "[\2vHost\2] request by %s has been rejected by %s ( %s )", 
					nick, u->nick, reason ? reason : "");
		}
	} else {
		moduleNoticeLang(s_HostServ, u, LNG_NO_REQUEST, nick);
	}

	free(nick);
	if (reason)
		free(reason);

	return MOD_CONT;
}

int hs_do_activate(User * u) {
	char *cur_buffer;
	char *nick;
	NickAlias *na;
	HostCore *tmp, *hc;
	boolean found = false;

	cur_buffer = moduleGetLastBuffer();
	nick = myStrGetToken(cur_buffer, ' ', 0);

	if (!nick) {
		moduleNoticeLang(s_HostServ, u, LNG_ACTIVATE_SYNTAX);
		return MOD_CONT;
	}

	if ((na = findnick(nick))) {
		tmp = findHostCore(hs_request_head, nick, &found);
		if (found) {
			if (!tmp)
				hc = hs_request_head;
			else
				hc = tmp->next;

			addHostCore(hc->nick, hc->vIdent, hc->vHost, u->nick, time(NULL));

			if (HSRequestMemoUser)
				my_memo_lang(u, hc->nick, 2, LNG_ACTIVATE_MEMO);

			hs_request_head = deleteHostCore(hs_request_head, tmp);
			moduleNoticeLang(s_HostServ, u, LNG_ACTIVATED, nick);
			alog("Host Request for %s activated by %s", nick, u->nick);
			if (HSRequestSendChan) {
				privmsg(s_HostServ, HSRequestSendChan, "[\2vHost\2] request by %s has been activated by %s", nick, u->nick);
			}
		} else {
			moduleNoticeLang(s_HostServ, u, LNG_NO_REQUEST, nick);
		}
	} else {
	notice_lang(s_HostServ, u, NICK_X_NOT_REGISTERED, nick);
	}

	free(nick);
	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

void my_add_host_request(char *nick, char *vIdent, char *vhost,
			char *creator, int32 tmp_time) {
	HostCore *tmp;
	boolean found = false;

	if (!hs_request_head) {
		hs_request_head = createHostCorelist(hs_request_head, nick, vIdent, vhost, creator, tmp_time);
	} else {
		tmp = findHostCore(hs_request_head, nick, &found);
		if (!found) {
			hs_request_head = insertHostCore(hs_request_head, tmp, nick, vIdent, vhost, creator, tmp_time);
		} else {
			hs_request_head = deleteHostCore(hs_request_head, tmp);     /* delete the old entry */
			my_add_host_request(nick, vIdent, vhost, creator, tmp_time);        /* recursive call to add new entry */
		}
	}
}

/* ------------------------------------------------------------------------------- */

int my_isvalidchar(const char c) {
	if (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'))
		|| ((c >= '0') && (c <= '9')) || (c == '.') || (c == '-'))
		return 1;
	else
		return 0;
}

int hs_do_list_out(User * u) {
	char *key;

	key = moduleGetLastBuffer();
	if (!key)
		return MOD_CONT;
	if (stricmp(key, "+req") != 0)
		return MOD_CONT;

	show_list(u);
	return MOD_CONT;
}

int hs_do_waiting(User * u) {
	show_list(u);
	return MOD_CONT;
}

void show_list(User * u) {
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

int hs_help_request(User * u) {
	moduleNoticeLang(s_HostServ, u, LNG_REQUEST_SYNTAX);
	notice(s_HostServ, u->nick, " ");
	moduleNoticeLang(s_HostServ, u, LNG_HELP_REQUEST);

	return MOD_CONT;
}

int hs_help_activate(User * u) {
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

int hs_help_reject(User * u) {
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

int hs_help_waiting(User * u) {
	if (is_host_setter(u)) {
		moduleNoticeLang(s_HostServ, u, LNG_WAITING_SYNTAX);
		notice(s_HostServ, u->nick, " ");
		moduleNoticeLang(s_HostServ, u, LNG_HELP_WAITING);
	} else {
		notice_lang(s_HostServ, u, NO_HELP_AVAILABLE, "WAITING");
	}

	return MOD_CONT;
}

void hs_help(User * u) {
	moduleNoticeLang(s_HostServ, u, LNG_HELP);
	if (is_host_setter(u))
		moduleNoticeLang(s_HostServ, u, LNG_HELP_SETTER);
}

/* ------------------------------------------------------------------------------- */

void hsreq_load_db(void) {
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
		alog("[hs_request] Unable to open database ('%s') for reading", filename);
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

void hsreq_save_db(void) {
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
		alog("[hs_request] Unable to open database ('%s') for writing", filename);
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

/* ------------------------------------------------------------------------------- */

int hsreqevt_db_saving(int argc, char **argv) {
	if ((argc >= 1) && (stricmp(argv[0], EVENT_START) == 0))
		hsreq_save_db();

	return MOD_CONT;
}

int hsreqevt_db_backup(int argc, char **argv) {
	if ((argc >= 1) && (stricmp(argv[0], EVENT_START) == 0)) {
		if (HSRequestDBName)
			ModuleDatabaseBackup(HSRequestDBName);
	}

	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

int check_load(void) {
	/* Make sure the normal hs_request isn't loaded.. */
	if (findModule("hs_request")) {
		alog("[\002hs_request_ext\002] This module cannot be used alongside hs_request. It is meant as a replacement! (This is fatal!)");
		Enabled = 0;
		return MOD_STOP;
	}

#ifdef SUPPORTED
	if (!DisableRaw || findModule("os_raw")) {
		alog("[\002hs_request_ext\002] RAW has NOT been disabled! (This is fatal!)");
		Enabled = 0;
		return MOD_STOP;
	}

	if (supported) {
		if (findModule("bs_logchanmon")) {
			alog("[\002hs_request_ext\002] This module is unsupported in combination with bs_logchanmon.");
			supported = 0;
		} else if (findModule("ircd_gameserv")) {
			alog("[\002hs_request_ext\002] This module is unsupported in combination with ircd_gameserv.");
			supported = 0;
		} else if (findModule("ircd_init")) {
			alog("[\002hs_request_ext\002] This module is unsupported in combination with ircd_init.");
			supported = 0;
		} else if (findModule("cs_join")) {
			alog("[\002hs_request_ext\002] This module is unsupported in combination with cs_join.");
			supported = 0;
		}
	} else
		alog("[\002hs_request_ext\002] Module operating in unsupported mode!");

#endif
	return MOD_CONT;
}

void update_version(void) {
	Module *m;
	char tmp[BUFSIZE];

	if (mod_current_module)
		m = mod_current_module;
	else
		m = findModule("hs_request_ext");
	snprintf(tmp, BUFSIZE, "%s [%s-%s%s%s]", VERSION, (((Enabled))?((supported >= 0)?((supported == 0)?"u":"S"):"U"):"D"), 
			HSRequestSendChan ? "S" : "", HSRequestChan ? "R" : "", HSSendHelp ? "H" : "");

	if (m->version)
		free(m->version);
	m->version = sstrdup(tmp);
}

/* ------------------------------------------------------------------------------- */

void my_load_config(void) {
	int i;
	char *tmp1 = NULL, *tmp2 = NULL, *tmp3 = NULL;

	Directive confvalues[][1] = {
		{{"HSRequestMemoUser", {{PARAM_SET, PARAM_RELOAD, &HSRequestMemoUser}}}},
		{{"HSRequestMemoOper", {{PARAM_SET, PARAM_RELOAD, &HSRequestMemoOper}}}},
		{{"HSRequestMemoSetters", {{PARAM_SET, PARAM_RELOAD, &HSRequestMemoSetters}}}},
		{{"HSRequestDBName", {{PARAM_STRING, PARAM_RELOAD, &tmp1}}}},
		{{"HSRequestSendChan", {{PARAM_STRING, PARAM_RELOAD, &tmp2}}}},
		{{"HSRequestChan", {{PARAM_STRING, PARAM_RELOAD, &tmp3}}}},
		{{"HSSendHelp", {{PARAM_SET, PARAM_RELOAD, &HSSendHelp}}}}
	};

	for (i = 0; i < 7; i++)
		moduleGetConfigDirective(confvalues[i]);

	if (tmp1) {
		if (HSRequestDBName)
			free(HSRequestDBName);
		HSRequestDBName = sstrdup(tmp1);
	} else {
		HSRequestDBName = sstrdup(HSREQ_DEFAULT_DBNAME);
	}

	/* This handles the channel to which we will report requests ~ Viper */
	if (HSRequestSendChan) {
		if (tmp2) {
			if (strcmp(tmp2, HSRequestSendChan) != 0) {
				privmsg(s_HostServ, HSRequestSendChan, "[hs_request_ext] No longer reporting requests to this channel.");
				alog("[hs_request_ext] No longer reporting requests to %s.", HSRequestSendChan);
				privmsg(s_HostServ, tmp2,"[hs_request_ext] Now reporting requests to this channel.");
				alog("[hs_request_ext] Now reporting requests to %s.", tmp2);
			}
		} else {
			privmsg(s_HostServ, HSRequestSendChan,"[hs_request_ext] Reporting has been turned off.");
			alog("[hs_request_ext] Reporting has been turned off.");
		}
		free(HSRequestSendChan);
	} else {
		if (tmp2) {
			privmsg(s_HostServ, tmp2,"[hs_request_ext] Now reporting requests to this channel.");
			alog("[hs_request_ext] Now reporting requests to %s.", tmp2);
		}
	}
	if (tmp2)
		HSRequestSendChan = sstrdup(tmp2);
	else
		HSRequestSendChan = NULL;

	if (HSRequestChan) free(HSRequestChan);
	if (tmp3)
		HSRequestChan = sstrdup(tmp3);
	else
		HSRequestChan = NULL;

	/* Show !vhost status.. */
	if (HSSendHelp) alog("[hs_request_ext] Help on !vhost is \002Active\002.");
	if (HSRequestChan) {
		if (stricmp(HSRequestChan,"") == 0)
			alog("[hs_request_ext] Request on !vhost is \002ON\002 in \002all\002 channels.");
		else
			alog("[hs_request_ext] Request on !vhost is \002ON\002 in \002%s\002.", HSRequestChan);
	}

	if (debug)
		alog("debug: [hs_request_ext] Set config vars: MemoUser=%d MemoOper=%d MemoSetters=%d SendHelp=%d DBName='%s' SendChan='%s' RequestChan='%s'", 
			HSRequestMemoUser, HSRequestMemoOper, HSRequestMemoSetters, HSSendHelp, HSRequestDBName, HSRequestSendChan, HSRequestChan);


	if (tmp1) free(tmp1);
	if (tmp2) free(tmp2);
	if (tmp3) free(tmp3);
}

int hsReloadConf(int argc, char **argv) {
	int ret = 0;

	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			alog("[hs_request_ext] Reloading configuration directives...");
			my_load_config();
			update_version();
		}
	}

	if (ret)
		alog("[hs_request_ext] ERROR: An error has occured while reloading the configuration file");

	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

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
		"the same as performing a LIST +req .",
		/* LNG_EXPL_HS */
		"Hello I am HostServ, a service that allows users to replace their host by a Virtual Host (vhost).\n"
		"Note that you need a registered nick before you can request a vhost.\n"
		"To request a vhost simply type '/hs REQUEST <vhost>' or reply to this message with 'REQUEST <vhost>' (without the 's)\n"
		"<vhost> can be a 'virtual-ident@virtual.host' or just a 'virtual.host'\n"
		"Once u have requested your vhost, the staff will be notified and when someone is available he will activate or reject the vhost.\n"
		"Check the networks rules for which vosts will be accepted on your network.\n"
		"For more help with HostServ, type help. Further replies go to the status window!"
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
		"hetzelfde als LIST +req .",
		/* LNG_EXPL_HS */
		"Hallo, ik ben HostServ, een service die gebruikers in staat stelt hun host te vervangen door een virtuele host (vhost).\n"
		"Voor u een vhost kunt aanvragen dient u eerst uw nick te registreren bij NickServ.\n"
		" \n"
		"Om een vhost aan te vragen typ '/hs REQUEST <vhost>' of beantwoord dit bericht met 'REQUEST <vhost>' (without the 's).\n"
		"<vhost> kan in de vorm zijn van 'virtuele-ident@virtuele.host' gewoon 'virtuele.host'\n"
		"Een aangevraagd zal de netwerk staf ingelicht worden waarna ze deze moeten goedkeuren voordat hij wordt geactiveerd.\n"
		"Controleer de netwerk reglementen om na te kijken welke vhosts aanvaard worden op dit netwerk.\n"
		"Voor meer help ivm HostServ, typ 'help'. Antwoorden gaan naar het status scherm!"
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
		"    ACTIVATE    Aprova o pedido de vHost de um usuбrio\n"
		"    REJECT      Recusa o pedido de vHost de um usuбrio\n"
		"    WAITING     Comando para LISTAR +req",
		/* LNG_HELP_REQUEST */
		"Solicita a ativaзгo do vHost fornecido em seu nick pelos\n"
		"administradores da rede. Por favor, tenha paciкncia\n"
		"enquanto seu pedido й analisado.",
		/* LNG_HELP_ACTIVATE */
		"Ativa o vHost solicitado para o nick fornecido.",
		/* LNG_HELP_ACTIVATE_MEMO */
		"Um memo informando o usuбrio tambйm serб enviado.",
		/* LNG_HELP_REJECT */
		"Recusa o pedido de vHost para o nick fornecido.",
		/* LNG_HELP_REJECT_MEMO */
		"Um memo informando o usuбrio tambйm serб enviado.",
		/* LNG_WAITING_SYNTAX */
		"Sintaxe: \002WAITING\002",
		/* LNG_HELP_WAITING */
		"Este comando й usado por conveniкncia. Й essencialmente\n"
		"o mesmo que fazer um LIST +req",
		/* LNG_EXPL_HS */
		"Hello I am HostServ, a service that allows users to replace their host by a Virtual Host (vhost).\n"
		"Note that you need a registered nick before you can request a vhost.\n"
		"To request a vhost simply type '/hs REQUEST <vhost>' or reply to this message with 'REQUEST <vhost>' (without the 's)\n"
		"<vhost> can be a 'virtual-ident@virtual.host' or just a 'virtual.host'\n"
		"Once u have requested your vhost, the staff will be notified and when someone is available he will activate or reject the vhost.\n"
		"Check the networks rules for which vosts will be accepted on your network.\n"
		"For more help with HostServ, type help (replies go to status window!)"
	};

	char *langtable_ru[] = {
		/* LNG_REQUEST_SYNTAX */
		"Синтаксис: \002REQUEST \037vHost\037\002",
		/* LNG_REQUESTED */
		"Ваш запрос на vHost отправлен.",
		/* LNG_REQUEST_WAIT */
		"Пожалуйста, подождите %d секунд, прежде чем запрашивать новый vHost",
		/* LNG_REQUEST_MEMO */
		"[авто-сообщение] Был запрошен vHost \002%s\002",
		/* LNG_ACTIVATE_SYNTAX */
		"Синтаксис: \002ACTIVATE \037ник\037\002",
		/* LNG_ACTIVATED */
		"vHost для %s успешно активирован",
		/* LNG_ACTIVATE_MEMO */
		"[авто-сообщение] Запрашиваемый вами vHost утвержден и активирован.",
		/* LNG_REJECT_SYNTAX */
		"Синтаксис: \002REJECT \037ник\037\002",
		/* LNG_REJECTED */
		"vHost для %s отклонен.",
		/* LNG_REJECT_MEMO */
		"[авто-сообщение] Запрашиваемый вами vHost отклонен.",
		/* LNG_REJECT_MEMO_REASON */
		"[авто-сообщение] Запрашиваемый вами vHost отклонен. Причина: %s",
		/* LNG_NO_REQUEST */
		"Запрос на vHost для ника %s не найден.",
		/* LNG_HELP */
		"    REQUEST     Запрос на vHost для вашего текущего ника",
		/* LNG_HELP_SETTER */
		"    ACTIVATE    Утвердить запрашиваемый пользователем  vHost\n"
		"    REJECT      Отклонить запрашиваемый пользователем  vHost\n"
		"    WAITING     Список запросов ожидающих обработки (аналог LIST +req)",
		/* LNG_HELP_REQUEST */
		"Отправляет запрос на активацию vHost, который будет рассмотрен одним из\n"
		"администраторов сети. Просьба проявить терпение, пока запрос\n"
		"рассматривается администрацией.",
		/* LNG_HELP_ACTIVATE */
		"Утвердить запрашиваемый vHost для указанного ника.",
		/* LNG_HELP_ACTIVATE_MEMO */
		"Пользователю будет послано авто-уведомление об активации его запроса.",
		/* LNG_HELP_REJECT */
		"Отклонить запрашиваемый vHost для указанного ника.",
		/* LNG_HELP_REJECT_MEMO */
		"Пользователю будет послано авто-уведомление об отклонении его запроса.",
		/* LNG_WAITING_SYNTAX */
		"Синтаксис: \002WAITING\002",
		/* LNG_HELP_WAITING */
		"Данная команда создана для удобства использования и выводит список запросов,\n"
		"ожидающих обработки. Аналогичная команда: LIST +req .",
		/* LNG_EXPL_HS */
		"Hello I am HostServ, a service that allows users to replace their host by a Virtual Host (vhost).\n"
		"Note that you need a registered nick before you can request a vhost.\n"
		"To request a vhost simply type '/hs REQUEST <vhost>' or reply to this message with 'REQUEST <vhost>' (without the 's)\n"
		"<vhost> can be a 'virtual-ident@virtual.host' or just a 'virtual.host'\n"
		"Once u have requested your vhost, the staff will be notified and when someone is available he will activate or reject the vhost.\n"
		"Check the networks rules for which vosts will be accepted on your network.\n"
		"For more help with HostServ, type help (replies go to status window!)"
	};

	char *langtable_it[] = {
		/* LNG_REQUEST_SYNTAX */
		"Sintassi: \002REQUEST \037vhost\037\002",
		/* LNG_REQUESTED */
		"Il tuo vHost и stato richiesto",
		/* LNG_REQUEST_WAIT */
		"Prego attendere %d secondi prima di richiedere un nuovo vHost",
		/* LNG_REQUEST_MEMO */
		"[auto memo] и stato richiesto il vHost \002%s\002.",
		/* LNG_ACTIVATE_SYNTAX */
		"Sintassi: \002ACTIVATE \037nick\037\002",
		/* LNG_ACTIVATED */
		"Il vHost per %s и stato attivato",
		/* LNG_ACTIVATE_MEMO */
		"[auto memo] Il vHost da te richiesto и stato approvato.",
		/* LNG_REJECT_SYNTAX */
		"Sintassi: \002REJECT \037nick\037\002",
		/* LNG_REJECTED */
		"Il vHost per %s и stato rifiutato",
		/* LNG_REJECT_MEMO */
		"[auto memo] Il vHost da te richiesto и stato rifiutato.",
		/* LNG_REJECT_MEMO_REASON */
		"[auto memo] Il vHost da te richiesto и stato rifiutato. Motivo: %s",
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
		"degli amministratori di rete. Sei pregato di pazientare finchи la tua\n"
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
		"Questo comando и per comoditа. Praticamente и la stessa cosa che\n"
		"eseguire un LIST +req .",
		/* LNG_EXPL_HS */
		"Hello I am HostServ, a service that allows users to replace their host by a Virtual Host (vhost).\n"
		"Note that you need a registered nick before you can request a vhost.\n"
		"To request a vhost simply type '/hs REQUEST <vhost>' or reply to this message with 'REQUEST <vhost>' (without the 's)\n"
		"<vhost> can be a 'virtual-ident@virtual.host' or just a 'virtual.host'\n"
		"Once u have requested your vhost, the staff will be notified and when someone is available he will activate or reject the vhost.\n"
		"Check the networks rules for which vosts will be accepted on your network.\n"
		"For more help with HostServ, type help (replies go to status window!)"
	};

	char *langtable_de[] = {
		/* LNG_REQUEST_SYNTAX */
		"Syntax: \002REQUEST \037vhost\037\002",
		/* LNG_REQUESTED */
		"Dein vHost wurde beantragt",
		/* LNG_REQUEST_WAIT */
		"Bitte warte %d Sekunden bevor Du einen neuen vHost beantragst",
		/* LNG_REQUEST_MEMO */
		"[auto memo] vHost \002%s\002 wurde beantragt.",
		/* LNG_ACTIVATE_SYNTAX */
		"Syntax: \002ACTIVATE \037nick\037\002",
		/* LNG_ACTIVATED */
		"vHost fьr %s wurde aktiviert",
		/* LNG_ACTIVATE_MEMO */
		"[auto memo] Dein beantragter vHost wurde freigegeben.",
		/* LNG_REJECT_SYNTAX */
		"Syntax: \002REJECT \037nick\037\002",
		/* LNG_REJECTED */
		"vHost fьr %s wurde abgelehnt",
		/* LNG_REJECT_MEMO */
		"[auto memo] Dein beantragter vHost wurde abgelehnt.",
		/* LNG_REJECT_MEMO_REASON */
		"[auto memo] Dein beantragter vHost wurde abgelehnt. Begrьndung: %s",
		/* LNG_NO_REQUEST */
		"Keine vHost Beantragung fьr Nick %s gefunden.",
		/* LNG_HELP */
		"    REQUEST     Beantrage einen vHost fьr Deinen Nick",
		/* LNG_HELP_SETTER */
		"    ACTIVATE    Freigeben eines beantragten vHostes von einem User\n"
		"    REJECT      Ablehnen eines beantragten vHostes von einem User\n"
		"    WAITING     Vereinfachter Befehl fьr LIST +req",
		/* LNG_HELP_REQUEST */
		"Beantragt den angegebenen vHost fьr Deinen Nick, um von den\n"
		"Network Administratoren aktiviert zu werden. Bitte gedulde Dich eine Weile,\n"
		"bis ьber Deine Anfrage entschieden wurde.",
		/* LNG_HELP_ACTIVATE */
		"Aktivert den beantragten vHost fьr den angegebenen Nick.",
		/* LNG_HELP_ACTIVATE_MEMO */
		"Eine Memo wird an den User gesendet, um ihn zu informieren.",
		/* LNG_HELP_REJECT */
		"Lehnt den beantragten vHost fьr den genannten Nick ab.",
		/* LNG_HELP_REJECT_MEMO */
		"Eine Memo wird an den User gesendet, um ihn zu informieren.",
		/* LNG_WAITING_SYNTAX */
		"Syntax: \002WAITING\002",
		/* LNG_HELP_WAITING */
		"Dieser Befehl ist Benutzerfreundlicher. Es ist genau derselbe\n"
		"als wenn man LIST +req benutzt.",
		/* LNG_EXPL_HS */
		"Hello I am HostServ, a service that allows users to replace their host by a Virtual Host (vhost).\n"
		"Note that you need a registered nick before you can request a vhost.\n"
		"To request a vhost simply type '/hs REQUEST <vhost>' or reply to this message with 'REQUEST <vhost>' (without the 's)\n"
		"<vhost> can be a 'virtual-ident@virtual.host' or just a 'virtual.host'\n"
		"Once u have requested your vhost, the staff will be notified and when someone is available he will activate or reject the vhost.\n"
		"Check the networks rules for which vosts will be accepted on your network.\n"
		"For more help with HostServ, type help (replies go to status window!)"
	};
	moduleInsertLanguage(LANG_EN_US, LNG_NUM_STRINGS, langtable_en_us);
	moduleInsertLanguage(LANG_NL, LNG_NUM_STRINGS, langtable_nl);
	moduleInsertLanguage(LANG_PT, LNG_NUM_STRINGS, langtable_pt);
	moduleInsertLanguage(LANG_RU, LNG_NUM_STRINGS, langtable_ru);
	moduleInsertLanguage(LANG_IT, LNG_NUM_STRINGS, langtable_it);
	moduleInsertLanguage(LANG_DE, LNG_NUM_STRINGS, langtable_de);
}

/* EOF */
