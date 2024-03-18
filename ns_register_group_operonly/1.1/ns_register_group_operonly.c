#include "module.h"
#define AUTHOR "ScoopFinder"
#define VERSION "1.0b"

/*
===================================================================================
= Module	: ns_register_group_operonly.c [english-french version]
= Version	: 1.1
= Author	: ScoopFinder [scoopfinder@epiknet.org]
= Date		: 09/05/2008
= Update	: 09/05/2008
===================================================================================
= Description:
= This module will restrict the NICKSERV REGISTER, GROUP and DROP command to IRCops.
= [ENGLISH-FRENCH VERSION]
=
= Requires: Anope-1.7.13+
= Tested: Anope-1.7.17
====================================================================================
= Inspired by Viper with his cs_register_operonly.c module.
= This module have no configurable option.
*/

/*------------------------- Source - Don't change below --------------------------*/

#include "module.h"
#define AUTHOR "ScoopFinder"
#define VERSION "1.1"


/* Language defines */
#define LANG_NUM_STRINGS 				1

#define LANG_REGISTER_RESTRICTED			0


/* Functions */
int oper_only_nick_reg(User * u);
void add_languages(void);


/* ------------------------------------------------------------------------------- */


int AnopeInit(int argc, char **argv) {
	Command *c;
	int status;

	alog("[ns_register_group_operonly] Loading module...");
	add_languages();

	c = createCommand("REGISTER", oper_only_nick_reg, NULL,-1,-1,-1,-1,-1);
	status = moduleAddCommand(NICKSERV, c, MOD_HEAD);
	c = createCommand("GROUP", oper_only_nick_reg, NULL,-1,-1,-1,-1,-1);
	status = moduleAddCommand(NICKSERV, c, MOD_HEAD);

	alog("[ns_register_group_operonly] Yayness!(tm) - MODULE LOADED [Status: %d]", status);

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	return MOD_CONT;
}


/**
 * Unload the module
 **/
void AnopeFini(void) {
	alog("[ns_register_group_operonly] Unloading module...");
}

/* ------------------------------------------------------------------------------- */

/**
 * Deny registration if necessary and send notice if needed.
 **/
int oper_only_nick_reg(User * u) {
	if (!(is_oper(u))) {
		moduleNoticeLang(s_NickServ, u, LANG_REGISTER_RESTRICTED);

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
		" Sorry, you must REGISTER and GROUP your nick with our website: http://www.my-website.com \n"
		" If you need support, join the help channel \n"
		" Good Tchat.",
	};

	char *langtable_fr[] = {
		/* LANG_REGISTER_RESTRICTED */
		" Désolé, l'enregistrement et le groupage de pseudos ne peuvent se faire que par le site web : http://www.votre-site-ici.net \n"
		" Pour tout problème veuillez rejoindre le cnala d'aide du serveur.\n"
		" Merci pour votre comprhéension.",
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
	moduleInsertLanguage(LANG_FR, LANG_NUM_STRINGS, langtable_fr);
}


/* EOF */
