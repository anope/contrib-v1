
/**
 * -----------------------------------------------------------------------------
 * Name: cs_register_operonly
 * Author: Viper  <Viper@Absurd-IRC.net>
 * Date: 21/10/2006  (Last update: 21/10/2006)
 * Version: 2.0
 * -----------------------------------------------------------------------------
 * Requires: Anope-1.7.13+
 * Tested: Anope-1.7.17 + UnrealIRCd 3.2.3
 * -----------------------------------------------------------------------------
 * This module will restrict the CHANSERV REGISTER command to IRCops.
 * This module should work on any ircd and any version of anope higher then 1.7.13,
 * but only version 1.7.17 and higher will be supported.
 *
 * This module is based on cs_operonly_register by SGR.
 * -----------------------------------------------------------------------------
 * Changes:
 *
 *  2.0  First release by me
 *       Rewrite to a 1.7 style module
 *       No longer supporting 1.6 branch
 *       Renamed module
 *
 *
 * -----------------------------------------------------------------------------
 **/


/*------------------------- Source - Don't change below --------------------------*/

#include "module.h"
#define AUTHOR "Viper"
#define VERSION "2.0"


/* Language defines */
#define LANG_NUM_STRINGS 					1

#define LANG_REGISTER_RESTRICTED			0


/* Functions */
int oper_only_chan_reg(User * u);
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
	int status;

	alog("[cs_register_operonly] Loading module...");
	add_languages();

	c = createCommand("REGISTER", oper_only_chan_reg, NULL,-1,-1,-1,-1,-1);
	status = moduleAddCommand(CHANSERV, c, MOD_HEAD);

	alog("[cs_register_operonly] Yayness!(tm) - MODULE LOADED [Status: %d]", status);

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	return MOD_CONT;
}


/**
 * Unload the module
 **/
void AnopeFini(void) {
	alog("[cs_register_operonly] Unloading module...");
}

/* ------------------------------------------------------------------------------- */

/**
 * Deny registration if necessary and send notice if needed.
 **/
int oper_only_chan_reg(User * u) {
	if (!(is_oper(u))) {
		moduleNoticeLang(s_ChanServ, u, LANG_REGISTER_RESTRICTED);

		return MOD_STOP;
	}
	return MOD_CONT;
}


/* ------------------------------------------------------------------------------- */

/**
 * Add language strings to the module's language db.
 **/
void add_languages(void) {
	char *langtable_en_us[] = {
		/* LANG_REGISTER_RESTRICTED */
		" Sorry, channel registration must be authorised by a \n"
		" network staff member. Please join the networks' support \n"
		" channel to have your channel registered. \n"
		" More information can be found using: /MOTD",
	};

	char *langtable_nl[] = {
		/* LANG_REGISTER_RESTRICTED */
		" Sorry, kanaal registratie moet toegelaten worden door \n"
		" een netwerk verantwoordelijke. Gelieve het help kanaal \n"
		" te joinen om je kanaal te laten registreren. \n"
		" Meer informatie kan gevonden worden met: /MOTD",
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
	moduleInsertLanguage(LANG_NL, LANG_NUM_STRINGS, langtable_nl);
}


/* EOF */
