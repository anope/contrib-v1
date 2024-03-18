/**
 * -----------------------------------------------------------------------------
 * Name        : ircd_restrict_oper
 * Author      : Viper  <Viper@Absurd-IRC.net>
 * Date        : 21/06/2008
 * Last update : 21/06/2008
 * Version     : 1.0
 * -----------------------------------------------------------------------------
 * Limitations : Currently only works on UnrealIRC (InspIRCd & UltimateIRCd planned)
 * Tested      : Anope-1.7.21 + UnrealIRCd 3.2.6
 * Requires    : Anope-1.7.21
 * -----------------------------------------------------------------------------
 * This module will restrict oper modes (incl +q) to a certain services level.
 * In addition it requires users to be using a registered nick to /oper.
 * For example if a server admin with services oper access would decide to promote
 * himself to netadmin, the +Na modes would be removed by services.
 *
 * If an oper is found to be invalid, services will remove the umodes granted by oper.
 * Note that if anope uses SVSMODE instead of SVS2MODE the user will not be informed.
 *
 * The module imposes following restrictions:
 *         IRC Operator Level            Required Services Access      Identified
 *      Network Administrator (N)         Services Administrator            x
 *      Services Administrator (a)        Services Administrator            x
 *        Server (Co)Admin (A/C)             Services Operator              x
 *        Global IRC Operator (o)                <none>                     x
 *        Local IRC Operator (O)                 <none>
 *
 *            User Mode q               Services Root Administrator         x
 *
 * This module can be usefull for networks using a relatively flexible open link
 * policy, allowing them to restrict new opers' access through services.
 *
 * I wrote this module mainly for use on my net, it is being released in case
 * others have a use for it.
 * This module is published under GPLv2.
 * -----------------------------------------------------------------------------
 *
 * Changelog:
 *
 *    1.0    Initial release
 *
 * -----------------------------------------------------------------------------
 **/

/**
 * TODO:
 *
 *    - add support for inspircd and ultimateircd
 *    - make it more configurable.. (list of nicks with q, NA, SA, A, O access)
 *          in addition to current restrictions
 *
 **/

/**
 * Configuration directives that should be copy-pasted to services.conf

 *
 **/


/* ------------------------------------------------------------------------------- */

#include "module.h"

#define AUTHOR "Viper"
#define VERSION "1.0"


/* Language defines - not currently being used.. */
#define LANG_NUM_STRINGS 					1

#define LANG_OPER_NOTID								0


/* Constants */


/* Variables */


/* Functions */
int my_oper_check(char *source, int ac, char **av);

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
	Message *msg;

	alog("[\002ircd_restrict_oper\002] Loading module...");

	if (!moduleMinVersion(1,7,21,1341)) {
		alog("[\002ircd_restrict_oper\002] Your version of Anope isn't supported. Please update to a newer release.");
		return MOD_STOP;
	}

	if (stricmp(IRCDModule,"unreal32")) {
		alog("[\002ircd_restrict_oper\002] Your IRCd is currently not supported.");
		return MOD_STOP;
	}

	/* Hook to mode changes by server.. */
	msg = createMessage("UMODE2", my_oper_check);
	if (moduleAddMessage(msg, MOD_TAIL) != MOD_CONT) {
		alog("[\002ircd_restrict_oper\002] Could not hook to incoming UMODE2 messages.");
		return MOD_STOP;
	}

	msg = createMessage("|", my_oper_check);
	if (moduleAddMessage(msg, MOD_TAIL) != MOD_CONT) {
		alog("[\002ircd_restrict_oper\002] Could not hook to incoming TOKEN messages.");
		return MOD_STOP;
	}

	add_languages();

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	alog("[\002ircd_restrict_oper\002] Module loaded successfully...");

	return MOD_CONT;
}

void AnopeFini(void) {
	alog("[\002ircd_restrict_oper\002] Unloading module...");
}

/* ------------------------------------------------------------------------------- */

int my_oper_check(char *source, int ac, char **av) {
	User *u;
	int add = 1, i = 0;
	char *modes;
	char rem[10];

	if (ac != 1)
		return MOD_CONT;

	if (!(u = finduser(source)))
		return MOD_CONT;

	modes = av[0];
	rem[i++] = '-';
	while (*modes) {
		switch (*modes++) {
			case '+':
				add = 1;
				break;

			case '-':
				add = 0;
				break;

			case 'N':
				if (add && !is_services_admin(u))
					rem[i++] = 'N';
				break;

			case 'a':
				if (add && !is_services_admin(u))
					rem[i++] = 'a';
				break;

			case 'A':
				if (add && !is_services_oper(u))
					rem[i++] = 'A';
				break;

			case 'C':
				if (add && !is_services_oper(u))
					rem[i++] = 'C';
				break;

			case 'o':
				if (add && !nick_identified(u))
					rem[i++] = 'o';
				break;

			case 'q':
				if (add && !is_services_root(u))
					rem[i++] = 'q';
				break;
		}
	}

	/* Send all mode changes in one command. */
	if (i > 1) {
		common_svsmode(u, rem, NULL);

		/* Inform the user if he wasn't identified.. */
		if (!nick_identified(u))
			moduleNoticeLang(s_OperServ, u, LANG_OPER_NOTID);

	}

	return MOD_CONT;
}


/* ------------------------------------------------------------------------------- */


/**
 * (Re)Load the configuration directives
 **/
int load_config(void) {
	int i;

	Directive confvalues[][1] = {
	};

	for (i = 0; i < 0; i++)
		moduleGetConfigDirective(confvalues[i]);

	return 1;
}

/**
 * Upon /os reload call the routines for reloading the configuration directives
 **/
int reload_config(int argc, char **argv) {
	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			alog("[ircd_restrict_oper]: Reloading configuration directives...");
			load_config();
		}
	}

	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

/**
 * At this stage, this is not being used..
 **/
void add_languages(void) {
	char *langtable_en_us[] = {
		/* LANG_OPER_NOTID */
		" You need to be identified to a registered nickname before you can become a Global IRC Operator.",
	};

	char *langtable_nl[] = {
		/* LANG_OPER_NOTID */
		" Je moet jezelf aanmelden voor een geregistreerde nicknaam voordat je een Globale IRC beheerder wordt.",
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
	moduleInsertLanguage(LANG_NL, LANG_NUM_STRINGS, langtable_nl);
}

/* EOF */
