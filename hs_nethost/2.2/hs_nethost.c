/**
 * -----------------------------------------------------------------------------
 * Name        : hs_nethost
 * Author      : Viper  <Viper@Absurd-IRC.net>
 * Date        : 12/05/2008
 * Last update : 30/05/2008
 * Version     : 2.2
 * -----------------------------------------------------------------------------
 * Tested      : Anope-1.7.21 + UnrealIRCd 3.2.6
 * Requires    : Anope-1.7.21
 * -----------------------------------------------------------------------------
 * This module will automatically set a default vhost upon registration, supporting
 * customization based upon the user's nickname.
 *
 * This module is based on a fork of hs_nethost 1.2 by Trystan Scott Lee when that
 * module was no longer supported. Most code however has been rewritten to make use
 * of the events system and allow for better customization of the NetworkHost.
 * Credit also goes to the original author Certus, and Strike, the author of hs_autohost.
 *
 * This module is published under GPLv2.
 * -----------------------------------------------------------------------------
 * Translations:    *** Does not currently apply ***
 *
 *     - English and Dutch Languages provided and maintained by myself.
 *
 * -----------------------------------------------------------------------------
 *
 * Changelog:
 *
 *    2.2    Added sanity check on vIdent & vHost
 *           Fixed a critical bug in str_replace() on FreeBSD systems.
 *               (Rewrote it entirely and should be a lot faster now as well..)
 *           Fixed a few memory leaks.
 *           Fixed vIdent and vHost checking
 *
 *    2.1    Fixed segfault crash occuring when no vIdent is set.
 *
 *    2.0    Added support for more customizable network vhosts.
 *           Added windows support.
 *           Re-wrote most code and re-releasing module under my name.
 *           Module forked off hs_nethost 1.2 by Trystan.
 *
 * -----------------------------------------------------------------------------
 **/

/**
 * TODO:
 *
 *    - shrink vIdents that are too long and remove invalid chars from them before that..
 *
 **/

/**
 * Configuration directives that should be copy-pasted to services.conf

# NetworkHost [REQUIRED by hs_nethost]
# Module: hs_nethost
#
# When set this vHost (and vIdent) will automatically be set for users
# when they register a new nick with NickServ.
# Note that no ident must be given.
#
# Some variables may be used for further customization:
# "$nick$" will be replaced by the nickname that was just registered and
# can be used multiple times in both the ident and host portion.
# $network$ will be replaced by the configured NetworkName.
#
# Note that only letters and numbers are allowed to be used in the NetworkHost.
#
# Some examples are:
#NetworkHost "$nick$@users.network.net"
#NetworkHost "$nick$.users.network.net"
#NetworkHost "$nick$@$nick$.users.network.net"
#NetworkHost "$nick$.users.$network$"
#NetworkHost "$nick$@$nick$.users.$network$"

NetworkHost "$nick$@users.$network$"

 *
 **/


/* ------------------------------------------------------------------------------- */

#include "module.h"

/*------------------------------Configuration Block----------------------------*/

/**
 * Undefining this will make you lose all support for hs_nethost!
 *
 * This module checks whether unsupported modules are enabled/loaded in anope and
 * whether the anope version meets minimum requirements to load.
 * If an incompatible module is found, hs_nethost will either unload or disable
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
 * Note that you may need to delete hs_nethost.o and .so (*nix) to force a recompile.
 **/
#define SUPPORTED

/*-------------------------End of Configuration Block--------------------------*/

#define AUTHOR "Viper"
#define VERSION "2.2"


/* Language defines - not currently being used.. */
#define LANG_NUM_STRINGS 					0

#define LANG_								0


/* Constants */
char *rep_nick = "$nick$";
char *rep_network = "$network$";


/* Variables */
char *NetworkHost;
int Enabled;
int supported;


/* Functions */
int set_vhost(int argc, char **argv);

void vident_valid(char *s);
void vhost_valid(char *s);
int check_vIdent(char *vIdent);
int check_vHost(char *vHost);
char *str_replace(char *replace_target, char *replace_with, char *source);

int check_load(void);
void update_version(void);

int load_config(void);
int reload_config(int argc, char **argv);
void add_languages(void);

/* ------------------------------------------------------------------------------- */

/**
 * Create the command, and tell anope about it.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv) {
	EvtHook *hook;
	Enabled = 1;

	alog("[\002hs_nethost\002] Loading module...");

	moduleAddAuthor(AUTHOR);
	/* This one is not needed, but the modules site requires it... */
	moduleAddVersion(VERSION);

#ifdef SUPPORTED
	if (!moduleMinVersion(1,7,21,1341)) {
		alog("[\002hs_nethost\002] Your version of Anope isn't supported. Please update to a newer release.");
		return MOD_STOP;
	}
	supported = 1;
#else
	supported = -1;
#endif

	check_load();
	if (!Enabled)
		return MOD_STOP;

	/* Hook to some events.. */
	hook = createEventHook(EVENT_NICK_REGISTERED, set_vhost);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002hs_nethost\002] Can't hook to EVENT_NICK_REGISTERED event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_GROUP, set_vhost);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002hs_nethost\002] Can't hook to EVENT_GROUP event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_RELOAD, reload_config);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002hs_nethost\002] Can't hook to EVENT_RELOAD event");
		return MOD_STOP;
	}

	if (!load_config()) {
		alog("[\002hs_nethost\002] NetworkHost is not set in the config. (This is fatal)");
		return MOD_STOP;
	}

	update_version();

	alog("[\002hs_nethost\002] Module loaded successfully...");

	return MOD_CONT;
}

void AnopeFini(void) {
	alog("[\002hs_nethost\002] Unloading module...");

	if (NetworkHost)
		free(NetworkHost);
}

/* ------------------------------------------------------------------------------- */

int set_vhost(int argc, char **argv) {
	int32 tmp_time = time(NULL);
	NickAlias *na;
	char *cust_host, *vIdent, *vHost, *tmp;

	if (!Enabled)
		return MOD_CONT;
	if (argc != 1)
		return MOD_CONT;
	if (!NetworkHost)
		return MOD_CONT;
	if (!(na = findnick(argv[0])))
		return MOD_CONT;

	/* Create the customized host for the user.. */
	cust_host = str_replace(rep_nick, na->nick, NetworkHost);
	tmp = cust_host;
	cust_host = str_replace(rep_network, NetworkName, cust_host);
	free(tmp);

	vIdent = myStrGetOnlyToken(cust_host, '@', 0);
	if (!vIdent) {
		vHost = strdup(cust_host);
	} else {
		vHost = myStrGetTokenRemainder(cust_host, '@', 1);
	}

	/* Try to make the vIdent and vHost valid.. */
	if (vIdent) vident_valid(vIdent);
	vhost_valid(vHost);

	/* Now check if they are valid... */
	if (check_vIdent(vIdent) == MOD_STOP || check_vHost(vHost) == MOD_STOP) {
		alog("[hs_nethost] Cannot set vHost for nick %s (%s). (Could not create valid vhost!)", na->nick, cust_host);

		if (vHost) free(vHost);
		if (vIdent) free(vIdent);
		free(cust_host);
		return MOD_CONT;
	}

	/* Create the new vhost... */
	alog("[hs_nethost] Automatically setting vhost for nick %s (%s).", na->nick, cust_host);
	addHostCore(na->nick, vIdent, vHost, s_HostServ, tmp_time);

	if (na->u) {
		notice_lang(s_HostServ, na->u, HOST_SET, s_HostServ, cust_host);

		/* Make sure the vhost is also set. */
		if (nick_identified(na->u))
			do_on_id(na->u);
	}

	free(cust_host);
	free(vHost);
	if (vIdent) free(vIdent);

	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

/**
 * Checks and makes sure the final vIdent is valid..
 **/
void vident_valid(char *s) {
	register int c;

	while ((c = *s)) {
		if (!isalnum(c)) {
			*s = '-';
		}
		s++;
	}
	return;
}

/**
 * Replaces non-letters and non-numbers by .s.
 **/
void vhost_valid(char *s) {
	register int c;
	int count = 0, ok = 0;

	while ((c = *s)) {
		if (!isalnum(c) && c != '-') {
			if (!ok) {
				*s = 'x';
			} else
				*s = '.';
		} else
			ok = 1;

		s++; count++;
	}
	return;
}

int check_vIdent(char *vIdent) {
	char *s;

	if (!vIdent)
		return MOD_CONT;

	if (strlen(vIdent) > USERMAX - 1)
		return MOD_STOP;
	if (!ircd->vident)
		return MOD_STOP;

	for (s = vIdent; *s; s++) {
		if (!isvalidchar(*s))
			return MOD_STOP;
	}

	return MOD_CONT;
}

int check_vHost(char *vHost) {
	if (!vHost)
		return MOD_STOP;
    if (strlen(vHost) >= HOSTMAX - 1)
        return MOD_STOP;
    if (!isValidHost(vHost, 3))
        return MOD_STOP;

	return MOD_CONT;
}

/**
 * This will replace all occurances of replace_target by replace_with in the source string.
 * Returns pointer to the new string.
 *
 * Note that all passed strings should be terminated.
 * Also note we return a NEW string that will need to be free'd.
 **/
char *str_replace(char *replace_target, char *replace_with, char *source) {
	char *result, *tmp, *end_r, *end_s;
	int count = 0, length, diff;

	if (!replace_target || !replace_with || !source)
		return NULL;

	/* See how many there are.. */
	end_s = source;
	while((tmp = strstr(end_s, replace_target))) {
		end_s = tmp + strlen(replace_target);
		count++;
	}

	/* Alocate the space we ll need.. */
	diff = strlen(replace_with) - strlen(replace_target);
	length = strlen(source) + count * diff;
	result = malloc(sizeof(char*) * length + 1);

	end_s = source;
	end_r = result;

	/* Copy the data to the new string.. */
	while((tmp = strstr(end_s, replace_target))) {
		memcpy(end_r, end_s, tmp - end_s);
		end_r += tmp - end_s;

		memcpy(end_r, replace_with, strlen(replace_with));
		end_r += strlen(replace_with);

		end_s = tmp + strlen(replace_target);
	}

	memcpy(end_r, end_s, strlen(end_s));
	memset(result + length, 0, 1);

	return result;
}

/* ------------------------------------------------------------------------------- */

int check_load(void) {
#ifdef SUPPORTED
	if (!DisableRaw || findModule("os_raw")) {
		alog("[\002hs_nethost\002] RAW has NOT been disabled! (This is fatal!)");
		Enabled = 0;
		return MOD_STOP;
	}

	if (supported) {
		if (findModule("bs_logchanmon")) {
			alog("[\002hs_nethost\002] This module is unsupported in combination with bs_logchanmon.");
			supported = 0;
		} else if (findModule("ircd_gameserv")) {
			alog("[\002hs_nethost\002] This module is unsupported in combination with ircd_gameserv.");
			supported = 0;
		} else if (findModule("ircd_init")) {
			alog("[\002hs_nethost\002] This module is unsupported in combination with ircd_init.");
			supported = 0;
		} else if (findModule("cs_join")) {
			alog("[\002hs_nethost\002] This module is unsupported in combination with cs_join.");
			supported = 0;
		}
	} else
		alog("[\002hs_nethost\002] Module operating in unsupported mode!");

#endif

	return MOD_CONT;
}

void update_version(void) {
	Module *m;
	char tmp[BUFSIZE];

	if (mod_current_module)
		m = mod_current_module;
	else
		m = findModule("hs_nethost");
	snprintf(tmp, BUFSIZE, "%s [%s]", VERSION, (((Enabled))?((supported >= 0)?((supported == 0)?"u":"S"):"U"):"D"));

	if (m->version)
		free(m->version);
	m->version = sstrdup(tmp);
}

/* ------------------------------------------------------------------------------- */

/**
 * (Re)Load the configuration directives
 **/
int load_config(void) {
	int i;

	Directive confvalues[][1] = {
		{{"NetworkHost", {{PARAM_STRING, PARAM_RELOAD, &NetworkHost}}}},
	};

	if (NetworkHost) free(NetworkHost);
	NetworkHost = NULL;

	for (i = 0; i < 1; i++)
		moduleGetConfigDirective(confvalues[i]);

	if (NetworkHost) {
		alog("[hs_nethost] NetworkHost set to %s", NetworkHost);
		return 1;
	} else {
		alog("[hs_nethost] NetworkHost NOT Set, please set NetworkHost in the services.conf");
		return 0;
	}
}

/**
 * Upon /os reload call the routines for reloading the configuration directives
 **/
int reload_config(int argc, char **argv) {
	if (!Enabled)
		return MOD_CONT;

	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			alog("[hs_nethost]: Reloading configuration directives...");
			load_config();
		}
	}

	if (check_load() != MOD_CONT) {
		update_version();
	}


	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

/**
 * At this stage, this is not being used..
 **/
void add_languages(void) {
	char *langtable_en_us[] = {
		"",
	};

	char *langtable_nl[] = {
		"",
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
	moduleInsertLanguage(LANG_NL, LANG_NUM_STRINGS, langtable_nl);
}

/* EOF */
