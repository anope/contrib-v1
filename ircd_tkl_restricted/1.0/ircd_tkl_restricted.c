/**
 * -----------------------------------------------------------------------------
 * Name    : ircd_tkl_restricted
 * Author  : Viper  <Viper@Absurd-IRC.net>
 * Date    : 15/01/2008  (Last update: 16/01/2008)
 * Version : 1.0
 * -----------------------------------------------------------------------------
 * Limitations: Any IRCd which uses TKL banning syntax similar to UnrealIRCd
 * Tested     : Anope-1.7.21 + UnrealIRCd 3.2.6
 * Requires   : Anope-1.7.21
 * -----------------------------------------------------------------------------
 * This module will cause - when activated - services to unset global TKL's set
 * by any server other then services. Thus the only way to ban someone
 * globally would be through services.
 *
 * Module requested by "zero".
 * -----------------------------------------------------------------------------
 *
 * Changelog:
 *
 *    1.0    Initial Release
 *
 * -----------------------------------------------------------------------------
 **/

/**
 * TODO:
 *
 *    - add option to block TKL removals.
 *    - add option to except SRAs
 *    - enable serverlist checking, but this can only be supported as of anope 1.7.22
 *    - consider adding the ability to block TKL's on a server by server basis.
 *
 **/


/*------------------------------Configuration Block----------------------------*/


/**
 * This is the default value of restricttkl after the module is loaded.
 * Set this to 1 to be active by default and to 0 to be disabled by default.
 **/
int tkl_restricted_def = 0;


/*-------------------------End of Configuration Block--------------------------*/


#include "module.h"

#define AUTHOR "Viper"
#define VERSION "1.0"


/* Language defines */
#define LANG_NUM_STRINGS 					5

#define LANG_TKL_RESTRICTED_DESC			0
#define LANG_TKL_RESTRICTED_SYNTAX			1
#define LANG_TKL_RESTRICTED_SYNTAX_EXT		2
#define LANG_TKL_RESTRICTED_ON				3
#define LANG_TKL_RESTRICTED_OFF				4


/* Variables */
int tkl_restricted;

/* Functions */
int add_restricttkl_option(User *u);
int do_set_restricttkl_help(User *u);
int null_func(User *u);

int do_set(User * u);
int parse_tkl(char *source, int ac, char **av);

void add_languages(void);


/* ------------------------------------------------------------------------------- */

/**
 * Create the command, and tell anope about it.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv) {
	Command *c;
	Message *msg;

	if (stricmp(IRCDModule, "unreal32")) {
		alog("[\002ircd_tkl_restricted\002] This IRCd is not supported!");
		return MOD_STOP;
	}

	msg = createMessage("TKL", parse_tkl);
	if (moduleAddMessage(msg, MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002ircd_tkl_restricted\002] Can't listen for TKL messages!");
		return MOD_STOP;
	}

	c = createCommand("SET", do_set, is_services_root, -1, -1, -1, -1, -1);
	if (moduleAddCommand(OPERSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002ircd_tkl_restricted\002] Can't hook to SET command!");
		return MOD_STOP;
	}

	c = createCommand("SET RESTRICTTKL", NULL, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(OPERSERV, c, MOD_TAIL) != MOD_ERR_OK) {
		alog("[\002ircd_tkl_restricted\002] Cannot create help for the SET RESTRICTTKL command...");
		return MOD_STOP;
	}
	moduleAddHelp(c, do_set_restricttkl_help);

	c = createCommand("SET", null_func, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(OPERSERV, c, MOD_TAIL) != MOD_ERR_OK) {
		alog("[\002ircd_tkl_restricted\002] Cannot add RESTRICTTKL to SET options...");
		return MOD_STOP;
	}
	moduleAddHelp(c, add_restricttkl_option);

	tkl_restricted = tkl_restricted_def;
	add_languages();

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	alog("[\002ircd_tkl_restricted\002] Module loaded successfully...");

	return MOD_CONT;
}


/**
 * Unload the module
 **/
void AnopeFini(void) {
	alog("[\002ircd_tkl_restricted\002] Unloading module...");
}


/* ------------------------------------------------------------------------------- */

/**
 * Add the RESTRICTTKL option to the end of the SET options listing.
 **/
int add_restricttkl_option(User *u) {
	moduleNoticeLang(s_OperServ, u, LANG_TKL_RESTRICTED_DESC);
	return MOD_CONT;
}


/**
 * Show the extended help on the SET RESTRICTTKL command.
 **/
int do_set_restricttkl_help(User *u) {
	moduleNoticeLang(s_OperServ, u, LANG_TKL_RESTRICTED_SYNTAX_EXT);
	return MOD_CONT;
}

/**
 * Just an empty function to return MOD_CONT
 **/
int null_func(User *u) {
  return MOD_CONT;
}


/* ------------------------------------------------------------------------------- */

/**
 * The /os set restricttkl command.
 * @param u The user who issued the command
 * @param MOD_CONT to continue processing other modules, MOD_STOP to stop processing.
 **/
int do_set(User * u) {
	char *buffer, *option, *param;
	int index;

	buffer = moduleGetLastBuffer();
	option = myStrGetToken(buffer, ' ', 0);
	param = myStrGetToken(buffer, ' ', 1);

	if (option && stricmp(option, "RESTRICTTKL") == 0) {
		if (param && !stricmp(param, "ON")) {
			tkl_restricted = 1;
			moduleNoticeLang(s_OperServ, u, LANG_TKL_RESTRICTED_ON);
		} else if (param && !stricmp(param, "OFF")) {
			tkl_restricted = 0;
			moduleNoticeLang(s_OperServ, u, LANG_TKL_RESTRICTED_OFF);
		} else
			moduleNoticeLang(s_OperServ, u, LANG_TKL_RESTRICTED_SYNTAX);
		free(option);
		if (param) free(param);
		return MOD_STOP;
	} else if (stricmp(option, "LIST") == 0) {
		index = (tkl_restricted ? OPER_SET_LIST_OPTION_ON : OPER_SET_LIST_OPTION_OFF);
		notice_lang(s_OperServ, u, index, "RESTRICTTKL");
	}

	if (option)	free(option);
	if (param)	free(param);
	return MOD_CONT;
}


/**
 * This functions receives the TKL's set and determines whether they need to be unset.
 *
 * Parameters:
 *  --- irc.VIPs.bounceme.net TKL + G *test *test Viper!Viper@Try.To.Get.My.IP 0 1200410993 :no reason ---
 * ac = number of parameters passed along by the IRCd. (8 when adding - 5 when removing)
 * av[0] = action: add or remove ('+' or '-') [ For further av's we supposed add]
 * av[1] = type of TKL (G: GLINE; Z: GZLINE; s: SHUN)
 * av[2] = target ident
 * av[3] = target hostmask (GLINE) or ipmask (GZLINE)
 * av[4] = Setter (nick!ident@host)
 * av[5] = time when it expires
 * av[6] = time set
 * av[7] = reason (With spaces as '_')
 **/
int parse_tkl(char *source, int ac, char **av) {
	Server *serv;

	if (!tkl_restricted)
		return MOD_CONT;

	if (ac <= 5)
		return MOD_CONT;

	if (debug)
		alog("[ircd_tkl_restricted] debug: Received incoming TKL: %s %s %s %s %s %s %s %s",
				av[0], av[1], av[2], av[3], av[4], av[5], av[6], av[7]);

	/* For safety reasons we will make sure the message comes from a server... */
	serv = first_server(-1);
	do {
		if (!stricmp(serv->name, source)) {
			break;
		}
		serv = next_server(-1);
	} while (serv);

	/* Disabling for now because next_server() is broken in the core
	if (!serv) {
		return MOD_CONT;
	} */

	/* We only need to parse new TKLs... */
	if (!stricmp(av[0], "+")) {
		/* Check if we handle the this type of TKL... */
		if (!stricmp(av[1], "G") || !stricmp(av[1], "Z") || !stricmp(av[1], "s")) {
			send_cmd(NULL, "TKL - %s %s %s %s", av[1], av[2], av[3], s_OperServ);
			alog("Removed TKL (%s) set by %s on %s@%s", av[1], av[4], av[2], av[3]);
		}
	}

	return MOD_CONT;
}


/* ------------------------------------------------------------------------------- */

/**
 * Add language strings to the module's language db.
 **/
void add_languages(void) {
	char *langtable_en_us[] = {
		/* LANG_TKL_RESTRICTED_DESC */
		" RESTRICTTKL       Restrict the use of TKL commands to services.",
		/* LANG_TKL_RESTRICTED_SYNTAX */
		" Syntax: SET RESTRICTTKL { ON | OFF}",
		/* LANG_TKL_RESTRICTED_SYNTAX_EXT */
		" Syntax: \002SET RESTRICTTKL { ON | OFF }\002 \n"
		" \n"
		" When enabled all TKLs set by servers other then services will be undone.",
		/* LANG_TKL_RESTRICTED_ON */
		" Services will now block GLINEs, GZLINEs and SHUNs made by anyone other then services.",
		/* LANG_TKL_RESTRICTED_OFF */
		" Services will not block any TKLs.",
	};
	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}

/* EOF */
