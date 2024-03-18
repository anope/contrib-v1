/**
 * Language variables and headers for filling
 *
 ***********
 * Module Name    : bs_fantasy_ext
 * Author         : Viper <Viper@Anope.org>
 * Creation Date  : 21/07/2006
 *
 * More info on http://modules.anope.org and http://forum.anope.org
 ***********
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***********
 *
 * Last Updated   : 21/11/2011
 *
 **/

#define getstring(na,index) \
	(langtexts[((na)&&((NickAlias*)na)->nc&&!(((NickAlias*)na)->status & NS_VERBOTEN)?((NickAlias*)na)->nc->language:NSDefLanguage)][(index)])


/*** Language Configuration Block ***/
/* Directory where we store our lang data..*/
#define LANG_SUBDIR_NAME			"bs_fantasy_ext"
 
/* Number of languages the mod is available in */
#define MOD_NUM_LANGS				3

/* Hardcoded default language. */
#define MOD_DEF_LANG				LANG_EN_US

/*** End Of Language Configuration Block ***/


/* Variables */
/* The list of lists of messages. */
char **modlangtexts[NUM_LANGS];

/* The list of names of languages. */
char *modlangnames[NUM_LANGS];

/* Indexes of available languages: */
int modlanglist[MOD_NUM_LANGS];

/* Store the default language used by the module */
int ModDefLanguage;

/* Index of languages supported by this module. */
int modlanglist[MOD_NUM_LANGS] = {
	LANG_EN_US,                 /* English (US) */
	LANG_FR,                    /* French */
	LANG_ES,                    /* Spanish */
};

/* ------------------------------------------------------------------------------- */

int mod_lang_init();
void mod_lang_unload();
int mod_read_int32(int32 * ptr, FILE * f);
void mod_load_lang(int index, const char *filename);
void mod_lang_sanitize();
char *getLangString(User *u, int message);
void noticeLang(char *source, User * dest, int message, ...);

/* ------------------------------------------------------------------------------- */

/* Language Variables */
int xop_msgs[4][14] = {
	{	CHAN_AOP_SYNTAX, CHAN_AOP_DISABLED, CHAN_AOP_NICKS_ONLY, CHAN_AOP_ADDED, CHAN_AOP_MOVED,
		CHAN_AOP_NO_SUCH_ENTRY, CHAN_AOP_NOT_FOUND, CHAN_AOP_NO_MATCH, CHAN_AOP_DELETED, CHAN_AOP_DELETED_ONE,
		CHAN_AOP_DELETED_SEVERAL, CHAN_AOP_LIST_EMPTY, CHAN_AOP_LIST_HEADER, CHAN_AOP_CLEAR
	},
	{	CHAN_SOP_SYNTAX, CHAN_SOP_DISABLED, CHAN_SOP_NICKS_ONLY, CHAN_SOP_ADDED, CHAN_SOP_MOVED,
		CHAN_SOP_NO_SUCH_ENTRY, CHAN_SOP_NOT_FOUND, CHAN_SOP_NO_MATCH, CHAN_SOP_DELETED, CHAN_SOP_DELETED_ONE,
		CHAN_SOP_DELETED_SEVERAL, CHAN_SOP_LIST_EMPTY, CHAN_SOP_LIST_HEADER, CHAN_SOP_CLEAR
	},
	{	CHAN_VOP_SYNTAX, CHAN_VOP_DISABLED, CHAN_VOP_NICKS_ONLY, CHAN_VOP_ADDED, CHAN_VOP_MOVED,
		CHAN_VOP_NO_SUCH_ENTRY, CHAN_VOP_NOT_FOUND, CHAN_VOP_NO_MATCH, CHAN_VOP_DELETED, CHAN_VOP_DELETED_ONE,
		CHAN_VOP_DELETED_SEVERAL, CHAN_VOP_LIST_EMPTY, CHAN_VOP_LIST_HEADER, CHAN_VOP_CLEAR
	},
	{	CHAN_HOP_SYNTAX, CHAN_HOP_DISABLED, CHAN_HOP_NICKS_ONLY, CHAN_HOP_ADDED, CHAN_HOP_MOVED,
		CHAN_HOP_NO_SUCH_ENTRY, CHAN_HOP_NOT_FOUND, CHAN_HOP_NO_MATCH, CHAN_HOP_DELETED, CHAN_HOP_DELETED_ONE,
		CHAN_HOP_DELETED_SEVERAL, CHAN_HOP_LIST_EMPTY, CHAN_HOP_LIST_HEADER, CHAN_HOP_CLEAR
	}
};

/* EOF */
