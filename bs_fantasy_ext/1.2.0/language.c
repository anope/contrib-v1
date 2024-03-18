/**
 * Method that calls the language specific logic. - Source
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
 * Last Updated   : 25/01/2009
 *
 **/

/**
 * Languages & Translations:
 * --------------------------
 *
 * English        : Viper <Viper@Anope.org>
 * French         : Seb Inconnu    [ Translation outdated!]
 *
 * Other translations are always welcome. Pls check the README.
 **/

#include "language.h"
#include "index.h"
 
int mod_lang_init() {
	int i, j, n = 0;
	ModDefLanguage = -1;

	/* Load available languages.. */
	mod_load_lang(LANG_EN_US, "en_us");

	/* Go over all loaded languages, add them to the list and check their integrity. */
	for (i = 0; i < MOD_NUM_LANGS; i++) {
		if (modlangtexts[modlanglist[i]] != NULL) {
			modlangnames[modlanglist[i]] = modlangtexts[modlanglist[i]][LANG_NAME];
			modlanglist[n++] = modlanglist[i];
			for (j = 0; j < MOD_NUM_STRINGS; j++) {
				if (!modlangtexts[modlanglist[i]][j]) {
					modlangtexts[modlanglist[i]][j] = sstrdup(modlangtexts[ModDefLanguage][j]);
				}
			}
		}
	}

	/* Set the modules' default language.
	 * Use NickServ default if available, if not fall back to modules' hardcoded default. */
	for (i = 0; i < MOD_NUM_LANGS; i++)
		if (modlanglist[i] == NSDefLanguage)
			ModDefLanguage = NSDefLanguage;
	if (ModDefLanguage < 0)
		ModDefLanguage = MOD_DEF_LANG;

	/* If the default language isn't available we have a problem..*/
	if (!modlangtexts[ModDefLanguage]) {
		alog("[bs_fantasy_ext] Unable to load default language.");
		return MOD_STOP;
	}

	mod_lang_sanitize();
	return MOD_CONT;
}

void mod_lang_unload() {
	int i, j;

	/* Go over all supported languages.. */
	for (i = 0; i < MOD_NUM_LANGS; i++) {
		if (modlangtexts[modlanglist[i]] != NULL) {
			/* Go over all language entries.. */
			for (j = 0; j < MOD_NUM_STRINGS; j++) {
				if (modlangtexts[modlanglist[i]][j])
					free(modlangtexts[modlanglist[i]][j]);
			}
			free(modlangtexts[modlanglist[i]]);
			modlangtexts[modlanglist[i]] = NULL;
		}
	}
}

/* ------------------------------------------------------------------------------- */

void mod_load_lang(int index, const char *filename) {
	char buf[256];
	FILE *f;
	int32 num, i;

	if (debug)
		alog("[bs_fantasy_ext] debug: Loading language %d from file `languages/%s/%s'", index,
				LANG_SUBDIR_NAME, filename);

	snprintf(buf, sizeof(buf), "languages/%s/%s", LANG_SUBDIR_NAME, filename);
#ifndef _WIN32
	if (!(f = fopen(buf, "r"))) {
#else
	if (!(f = fopen(buf, "rb"))) {
#endif
		log_perror("[bs_fantasy_ext] Failed to load language %d (%s)", index, filename);
		return;
	} else if (mod_read_int32(&num, f) < 0) {
		alog("[bs_fantasy_ext] Failed to read number of strings for language %d (%s)", index, filename);
		return;
	} else if (num != MOD_NUM_STRINGS) {
		alog("[bs_fantasy_ext] Warning: Bad number of strings (%d, wanted %d) for language %d (%s)",
				num, MOD_NUM_STRINGS, index, filename);
	}
	modlangtexts[index] = scalloc(sizeof(char *), MOD_NUM_STRINGS);

	if (num > MOD_NUM_STRINGS)
		num = MOD_NUM_STRINGS;

	for (i = 0; i < num; i++) {
		int32 pos, len;
		fseek(f, i * 8 + 4, SEEK_SET);
		if (mod_read_int32(&pos, f) < 0 || mod_read_int32(&len, f) < 0) {
			alog("[bs_fantasy_ext] Failed to read entry %d in language %d (%s) TOC",
					i, index, filename);
			while (--i >= 0) {
				if (modlangtexts[index][i])
					free(modlangtexts[index][i]);
			}
			free(modlangtexts[index]);
			modlangtexts[index] = NULL;
			return;
		}
		if (len == 0) {
			modlangtexts[index][i] = NULL;
		} else if (len >= 65536) {
			alog("[bs_fantasy_ext] Entry %d in language %d (%s) is too long (over 64k)--"
					"corrupt TOC?", i, index, filename);
			while (--i >= 0) {
				if (modlangtexts[index][i])
					free(modlangtexts[index][i]);
			}
			free(modlangtexts[index]);
			modlangtexts[index] = NULL;
			return;
		} else if (len < 0) {
			alog("[bs_fantasy_ext] Entry %d in language %d (%s) has negative length-- corrupt TOC?",
					i, index, filename);
			while (--i >= 0) {
				if (modlangtexts[index][i])
					free(modlangtexts[index][i]);
			}
			free(modlangtexts[index]);
			modlangtexts[index] = NULL;
			return;
		} else {
			modlangtexts[index][i] = scalloc(len + 1, 1);
			fseek(f, pos, SEEK_SET);
			if (fread(modlangtexts[index][i], 1, len, f) != len) {
				alog("[bs_fantasy_ext] Failed to read string %d in language %d (%s)", i, index, filename);
				while (--i >= 0) {
					if (modlangtexts[index][i])
						free(modlangtexts[index][i]);
				}
				free(modlangtexts[index]);
				modlangtexts[index] = NULL;
				return;
			}
			modlangtexts[index][i][len] = 0;
		}
	}
	fclose(f);
}

void mod_lang_sanitize() {
	int i = 0, j = 0;
	int len = 0;
	char tmp[65536];
	
	/* Go over all supported languages.. */
	for (i = 0; i < MOD_NUM_LANGS; i++) {
		/* Go over all language entries.. */
		for (j = 0; j < MOD_NUM_STRINGS; j++) {
			if (strstr(modlangtexts[modlanglist[i]][j], "%R")) {
				len = strlen(modlangtexts[modlanglist[i]][j]);
				strscpy(tmp, modlangtexts[modlanglist[i]][j], sizeof(tmp));
				if (UseStrictPrivMsg) {
					strnrepl(tmp, sizeof(tmp), "%R", "/");
				} else {
					strnrepl(tmp, sizeof(tmp), "%R", "/msg ");
				}
				free(modlangtexts[modlanglist[i]][j]);
				modlangtexts[modlanglist[i]][j] = sstrdup(tmp);
			}
			if (strstr(modlangtexts[modlanglist[i]][j], "%F")) {
				len = strlen(modlangtexts[modlanglist[i]][j]);
				strscpy(tmp, modlangtexts[modlanglist[i]][j], sizeof(tmp));
				strnrepl(tmp, sizeof(tmp), "%F", BSFantasyCharacter);
				free(modlangtexts[modlanglist[i]][j]);
				modlangtexts[modlanglist[i]][j] = sstrdup(tmp);
			}
			if (strstr(modlangtexts[modlanglist[i]][j], "%NS")) {
				len = strlen(modlangtexts[modlanglist[i]][j]);
				strscpy(tmp, modlangtexts[modlanglist[i]][j], sizeof(tmp));
				strnrepl(tmp, sizeof(tmp), "%NS", s_NickServ);
				free(modlangtexts[modlanglist[i]][j]);
				modlangtexts[modlanglist[i]][j] = sstrdup(tmp);
			}
			if (strstr(modlangtexts[modlanglist[i]][j], "%BS")) {
				len = strlen(modlangtexts[modlanglist[i]][j]);
				strscpy(tmp, modlangtexts[modlanglist[i]][j], sizeof(tmp));
				strnrepl(tmp, sizeof(tmp), "%BS", s_BotServ);
				free(modlangtexts[modlanglist[i]][j]);
				modlangtexts[modlanglist[i]][j] = sstrdup(tmp);
			}
			if (strstr(modlangtexts[modlanglist[i]][j], "%OS")) {
				len = strlen(modlangtexts[modlanglist[i]][j]);
				strscpy(tmp, modlangtexts[modlanglist[i]][j], sizeof(tmp));
				strnrepl(tmp, sizeof(tmp), "%OS", s_OperServ);
				free(modlangtexts[modlanglist[i]][j]);
				modlangtexts[modlanglist[i]][j] = sstrdup(tmp);
			}
			if (strstr(modlangtexts[modlanglist[i]][j], "%CS")) {
				len = strlen(modlangtexts[modlanglist[i]][j]);
				strscpy(tmp, modlangtexts[modlanglist[i]][j], sizeof(tmp));
				strnrepl(tmp, sizeof(tmp), "%CS", s_ChanServ);
				free(modlangtexts[modlanglist[i]][j]);
				modlangtexts[modlanglist[i]][j] = sstrdup(tmp);
			}
			if (s_HostServ && strstr(modlangtexts[modlanglist[i]][j], "%HS")) {
				len = strlen(modlangtexts[modlanglist[i]][j]);
				strscpy(tmp, modlangtexts[modlanglist[i]][j], sizeof(tmp));
				strnrepl(tmp, sizeof(tmp), "%HS", s_HostServ);
				free(modlangtexts[modlanglist[i]][j]);
				modlangtexts[modlanglist[i]][j] = sstrdup(tmp);
			}
			if (s_MemoServ && strstr(modlangtexts[modlanglist[i]][j], "%MS")) {
				len = strlen(modlangtexts[modlanglist[i]][j]);
				strscpy(tmp, modlangtexts[modlanglist[i]][j], sizeof(tmp));
				strnrepl(tmp, sizeof(tmp), "%MS", s_MemoServ);
				free(modlangtexts[modlanglist[i]][j]);
				modlangtexts[modlanglist[i]][j] = sstrdup(tmp);
			}

		}
	}
}
/* ------------------------------------------------------------------------------- */

/**
 * Send a message in the user's selected language to the user using NOTICE.
 * @param source Orgin of the Message
 * @param u User Struct
 * @param int Index of the Message
 * @param ... any number of parameters
 * @return void
 */
void noticeLang(char *source, User * dest, int message, ...) {
	va_list args;
	char buf[4096];             /* because messages can be really big */
	char *s, *t;
	const char *fmt;

	if (!dest || !message) {
		return;
	}
	va_start(args, message);
	fmt = getLangString(dest, message);

	if (!fmt)
		return;
	memset(buf, 0, 4096);
	vsnprintf(buf, sizeof(buf), fmt, args);
	s = buf;
	while (*s) {
		t = s;
		s += strcspn(s, "\n");
		if (*s)
			*s++ = 0;

		/* Send privmsg instead of notice if:
		 * - UsePrivmsg is enabled
		 * - The user is not registered and NSDefMsg is enabled
		 * - The user is registered and has set /ns set msg on
		 */
		if (UsePrivmsg && ((!dest->na && (NSDefFlags & NI_MSG)) || (dest->na
				&& (dest->na->nc->flags & NI_MSG)))) {
			anope_cmd_privmsg2(source, dest->nick, *t ? t : " ");
		} else {
			anope_cmd_notice2(source, dest->nick, *t ? t : " ");
		}
	}
	va_end(args);
}

/* ------------------------------------------------------------------------------- */

char *getLangString(User *u, int index) {
	int i;
	char *s = NULL;

	if (u->na && u->na->nc && !(u->na->status & NS_VERBOTEN)) {
		if (debug)
			alog("[bs_fantasy_ext] Looking for language string %d in language %d.",
					index, u->na->nc->language);

		/* Check to make sure the users language is supported */
		for (i = 0; i < MOD_NUM_LANGS; i++) {
			if (modlanglist[i] == u->na->nc->language) {
				if (modlangtexts[modlanglist[i]] != NULL)
					s = modlangtexts[modlanglist[i]][index];
				break;
			}
		}
	}

	/* If the string isn't assigned yet, use default language.. */
	if (!s)
		s = modlangtexts[ModDefLanguage][index];

	return s;
}

int mod_read_int32(int32 * ptr, FILE * f) {
	int a = fgetc(f);
	int b = fgetc(f);
	int c = fgetc(f);
	int d = fgetc(f);
	if (a == EOF || b == EOF || c == EOF || d == EOF)
		return -1;
	*ptr = a << 24 | b << 16 | c << 8 | d;
	return 0;
}

/* EOF */
