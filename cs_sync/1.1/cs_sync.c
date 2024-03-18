/**
 * -----------------------------------------------------------------------------
 * Name    : cs_sync
 * Author  : Viper <Viper@Absurd-IRC.net>
 * Date    : 02/02/2008 (Last update: 08/02/2008)
 * Version : 1.1
 * -----------------------------------------------------------------------------
 * Requires    : Anope-1.7.21
 * Tested      : Anope 1.7.21 + UnrealIRCd 3.2.6
 * -----------------------------------------------------------------------------
 * This module will synchronize a channels' userlist with the channels accesslist
 * giving/taking user privileges according to the access list.
 *
 * The SYNC command is restricted to channel founders.
 * -----------------------------------------------------------------------------
 *
 * Changelog:
 *
 *   1.1  -  Fixed a few minor typos in the different languages.
 *
 *   1.0  -  Initial Release
 *
 * -----------------------------------------------------------------------------
 **/


#include "module.h"
#define AUTHOR "Viper"
#define VERSION "1.1"


/* Language defines */
#define LANG_NUM_STRINGS 					5

#define LANG_SYNC_DESC						0
#define LANG_SYNC_SYNTAX					1
#define LANG_SYNC_SYNTAX_EXT				2
#define LANG_SYNC_NOCHAN					3
#define LANG_SYNC_DONE						4

/* Functions */
void do_help_list(User *u);
int do_help(User *u);
int do_sync(User *c);
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

	alog("[\002cs_sync\002] Loading module...");

	if (!moduleMinVersion(1,7,21,1341)) {
		alog("[\002cs_sync\002] Your version of Anope isn't supported. Please update to a newer release.");
		return MOD_STOP;
	}

	/* Create AJOIN command.. */
	c = createCommand("SYNC", do_sync, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(CHANSERV,c,MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002cs_sync\002] Cannot create SYNC command...");
		return MOD_STOP;
	}
	moduleAddHelp(c,do_help);
	moduleSetChanHelp(do_help_list);

	add_languages();

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	alog("[\002cs_sync\002] Module loaded successfully...");

	return MOD_CONT;
}


/**
 * Unload the module
 **/
void AnopeFini(void) {
	alog("[\002cs_sync\002] Unloading module...");
}

/* ------------------------------------------------------------------------------- */

/**
 * Add the AJOIN command to the NickServ HELP listing.
 **/
void do_help_list(User *u) {
	moduleNoticeLang(s_ChanServ, u, LANG_SYNC_DESC);
}


/**
 * Show the extended help on the AJOIN command.
 **/
int do_help(User *u) {
	moduleNoticeLang(s_ChanServ, u, LANG_SYNC_SYNTAX_EXT);
	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

int do_sync(User *u) {
	Channel *c;
	ChannelInfo *ci;
	char *buffer, *chan;

	buffer = moduleGetLastBuffer();
	chan = myStrGetToken(buffer, ' ', 0);

	if (!chan) {
		moduleNoticeLang(s_ChanServ, u, LANG_SYNC_NOCHAN);
		moduleNoticeLang(s_ChanServ, u, LANG_SYNC_SYNTAX);
		return MOD_CONT;
	}

	if (!(c = findchan(chan))) {
		notice_lang(s_ChanServ, u, CHAN_X_NOT_IN_USE, chan);
	} else if (!(ci = c->ci)) {
		notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
	} else if (ci->flags & CI_VERBOTEN) {
		notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
	} else if (ci->flags & CI_SUSPENDED) {
		notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
	} else if (!u || !is_founder(u, ci)) {
		notice_lang(s_ChanServ, u, ACCESS_DENIED);
	} else {
		struct c_userlist *cu, *next;

		cu = c->users;
		while (cu) {
			uint32 flags = 0;

			/* We store the next user just in case current user ends up getting kicked off the channel. */
			next = cu->next;

			if (cu->user->na && cu->user->na->nc) {
				flags = cu->user->na->nc->flags;
				cu->user->na->nc->flags &= ~NI_AUTOOP;
			}

			chan_set_correct_modes(cu->user, c, 1);

			if (cu->user->na && cu->user->na->nc)
				cu->user->na->nc->flags = flags;

			cu = next;
		}

		moduleNoticeLang(s_ChanServ, u, LANG_SYNC_DONE, ci->name);
	}

	free(chan);
	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

void add_languages(void) {
	char *langtable_en_us[] = {
		/* LANG_SYNC_DESC */
		" SYNC       Give all users the status the access list grants them.",
		/* LANG_SYNC_SYNTAX */
		" Syntax: SYNC [\037channel\037]",
		/* LANG_SYNC_SYNTAX_EXT */
		" Syntax: \002SYNC \037channel\037\002\n"
		" \n"
		" This command will give all users currently in the channel the level\n"
		" they are granted by the channels' access list. Users who have a level\n"
		" greater then the one they are supposed to have will be demoted.\n"
		" \n"
		" The use ofthis command is restricted to the Channel Founder.",
		/* LANG_SYNC_NOCHAN */
		" No channel specified.",
		/* LANG_SYNC_DONE */
		" Synchronized userlist with channel accesslist for %s.",
	};

	char *langtable_nl[] = {
		/* LANG_SYNC_DESC */
		" SYNC       Geef alle gebruikers de status die de access list hen toestaat.",
		/* LANG_SYNC_SYNTAX */
		" Gebruik: SYNC [\037kanaal\037]",
		/* LANG_SYNC_SYNTAX_EXT */
		" Gebruik: \002SYNC \037kanaal\037\002\n"
		" \n"
		" Dit commando geeft alle gebruikers die zich in het kanaal bevinden het niveau\n"
		" waartoe ze volgens de access list van het kanaal toegang hebben.\n"
		" Gebruikers die een niveau hebben dat hoger is dan hetgeen zo toegang hebben worden\n"
		" gedegradeerd tot het niveau waartoe ze recht hebben.\n"
		" \n"
		" Het gebruik van dit commando is gelimiteerd tot de Kanaal founder/stichter.",
		/* LANG_SYNC_NOCHAN */
		" Geen kanaal opgegeven.",
		/* LANG_SYNC_DONE */
		"Gebruikerslijst met toegangslijst gesynchroniseerd voor %s.",
	};

	/* Russian translation provided by Kein */
	char *langtable_ru[] = {
		/* LANG_SYNC_DESC */
		"    SYNC       ¬ыставл€ет статусы прописанным пользовател€м канала.",
		/* LANG_SYNC_SYNTAX */
		" —интаксис: SYNC [\037#канал\037]",
		/* LANG_SYNC_SYNTAX_EXT */
		" —интаксис: \002SYNC \037#канал\037\002\n"
		" \n"
		" »спользование данной команды позвол€ет автоматически выдать все статусы\n"
		" тем пользовател€м канала, которые присутствуют в его списке доступа.\n"
		" ѕосетители, текущий канальный статус которых превышает прописанный им\n"
		" в списке доступа канала, будут понижены в полномочи€х.\n"
		" \n"
		" ƒоступ к данной команде ограничен до владельца канала.",
		/* LANG_SYNC_NOCHAN */
		" ¬ы не указали канал.",
		/* LANG_SYNC_DONE */
		" —татусы пользователей канала %s успешно синхронизированы со списком доступа.",
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
	moduleInsertLanguage(LANG_NL, LANG_NUM_STRINGS, langtable_nl);
	moduleInsertLanguage(LANG_RU, LANG_NUM_STRINGS, langtable_ru);
}

/* EOF */
