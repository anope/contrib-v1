/**
 * -----------------------------------------------------------------------------
 * Name    : ns_sagroup
 * Author  : Viper  <Viper@Absurd-IRC.net>
 * Date    : 08/02/2008  (Last update: 16/02/2008)
 * Version : 1.0
 * -----------------------------------------------------------------------------
 * Tested  : Anope-1.7.21 + UnrealIRCd 3.2.6
 * -----------------------------------------------------------------------------
 * This module adds the SAGROUP command to NickServ allowing Services Administrators
 * and Services Root Administrators to remotely group nicknames.
 * This module is particulary useful in combination with ns_group_operonly and can
 * also be used to grant users more nicks then NSMaxAliases.
 *
 * SAGROUP will never work on registered users that have Services Root!
 *
 * Note that this command overrides several restrictions enforced by the
 * regular GROUP command, including NSNoGroupChange and NSMaxAliases.
 * !!! USE WITH CAUTION !!!
 * -----------------------------------------------------------------------------
 * Translations:
 *
 *     - English and Dutch Languages provided and maintained by Myself.
 *
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
 *  <<Hope there are no more bugs ;-) >>..
 *
 **/

#include "module.h"
#define AUTHOR "Viper"
#define VERSION "1.0"


/* Language defines */
#define LANG_NUM_STRINGS 					6

#define LANG_SAGROUP_DESC					0
#define LANG_SAGROUP_SYNTAX					1
#define LANG_SAGROUP_SYNTAX_EXT				2
#define LANG_GROUP_JOINED					3
#define LANG_NICK_SAGROUPED					4
#define LANG_GROUP_SAME						5


/* Functions */
void do_help_list(User * u);
int do_help(User *u);

int do_sagroup(User * u);
NickAlias *makealias(const char *nick, NickCore * nc);

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

	alog("[\002ns_sagroup\002] Loading module...");

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	c = createCommand("SAGROUP", do_sagroup, is_services_admin, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV, c, MOD_UNIQUE) != MOD_ERR_OK) {
		alog("[\002ns_sagroup\002] Cannot create SAGROUP command...");
		return MOD_STOP;
	}
	moduleAddHelp(c,do_help);
	moduleSetNickHelp(do_help_list);

	add_languages();

	alog("[\002ns_sagroup\002] Module loaded successfully...");

	return MOD_CONT;
}

/**
 * Unload the module
 **/
void AnopeFini(void) {
	alog("[\002ns_sagroup\002] Unloading module...");
}

/* ------------------------------------------------------------------------------- */

/**
 * Add the command to anopes /ns help output.
 * @param u The user who is requesting help
 **/
void do_help_list(User *u) {
    moduleNoticeLang(s_NickServ, u, LANG_SAGROUP_DESC);
}

/**
 * Show the extended help on the GROUP command.
 **/
int do_help(User *u) {
	moduleNoticeLang(s_NickServ, u, LANG_SAGROUP_SYNTAX_EXT);
	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

/**
 * The /ns sagroup command.
 * @param u The user who issued the command
 * @param MOD_CONT to continue processing other modules, MOD_STOP to stop processing.
 **/
int do_sagroup(User * u) {
	NickAlias *na, *target;
	User *u2;
	char *buffer, *nick, *tnick;
	int i;

	buffer = moduleGetLastBuffer();
	nick = myStrGetToken(buffer, ' ', 0);
	tnick = myStrGetToken(buffer, ' ', 1);

	if (readonly) {
		notice_lang(s_NickServ, u, NICK_GROUP_DISABLED);
	} else if (!nick || !tnick) {
		moduleNoticeLang(s_NickServ, u, LANG_SAGROUP_SYNTAX);
	} else if (NSEmailReg && (findrequestnick(nick))) {
		notice_lang(s_NickServ, u, NICK_REQUESTED);
	} else if (!(target = findnick(tnick))) {
		notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, tnick);
	} else if ((na = findnick(nick)) && (na->status & NS_VERBOTEN)) {
		notice_lang(s_NickServ, u, NICK_X_FORBIDDEN, nick);
	} else if (na && (na->nc->flags & NI_SUSPENDED)) {
		notice_lang(s_NickServ, u, NICK_X_SUSPENDED, nick);
	} else if (target->nc->flags & NI_SUSPENDED) {
		notice_lang(s_NickServ, u, NICK_X_SUSPENDED, tnick);
	} else if (target->status & NS_VERBOTEN) {
		notice_lang(s_NickServ, u, NICK_X_FORBIDDEN, tnick);
	} else if (na && target->nc == na->nc) {
		moduleNoticeLang(s_NickServ, u, LANG_GROUP_SAME, nick, tnick);
	} else if (na && (na->nc->flags & NI_SERVICES_ROOT)) {
		notice_lang(s_NickServ, u, PERMISSION_DENIED);
	} else {
		/* If the nick is already registered, drop it.
		 * If not, check that it is valid. */
		if (na) {
			delnick(na);
		} else {
			int prefixlen = strlen(NSGuestNickPrefix);
			int nicklen = strlen(nick);

			if (nicklen <= prefixlen + 7 && nicklen >= prefixlen + 1 && stristr(nick, NSGuestNickPrefix)
					== nick && strspn(nick + prefixlen, "1234567890") == nicklen - prefixlen) {
				notice_lang(s_NickServ, u, NICK_CANNOT_BE_REGISTERED, nick);
				free(nick);
				free(tnick);
				return MOD_CONT;
			}
		}
		na = NULL;
		na = makealias(nick, target->nc);

		if (na) {
			/* Copy the data here from the group we are putting the nick in..
			 * Anope doesn't like these entries to be NULL ;) */
			na->last_usermask = sstrdup(target->last_usermask);
			na->last_realname = sstrdup(target->last_realname);
			na->time_registered = time(NULL);
			na->last_seen = target->last_seen;
			na->status &= ~(NS_IDENTIFIED | NS_RECOGNIZED);

			if (!(na->nc->flags & NI_SERVICES_ROOT)) {
				for (i = 0; i < RootNumber; i++) {
					if (!stricmp(ServicesRoots[i], nick)) {
						na->nc->flags |= NI_SERVICES_ROOT;
						break;
					}
				}
			}

			if ((u2 = finduser(nick))) {
				u2->na = na;
				na->u = u2;

				/* If some1 is currently using this nick, inform them and enforce. */
				moduleNoticeLang(s_NickServ, u2, LANG_NICK_SAGROUPED, target->nc->display, u->nick);
				validate_user(u2);
			}

			send_event(EVENT_GROUP, 1, nick);
			alog("%s: %s!%s@%s used SAGROUP to make %s join group of %s (%s)", s_NickServ, u->nick, u->username, u->host, nick, target->nick, target->nc->display);
			moduleNoticeLang(s_NickServ, u, LANG_GROUP_JOINED, nick, tnick, target->nc->display);

		} else {
			alog("%s: makealias(%s) failed", s_NickServ, nick);
			notice_lang(s_NickServ, u, NICK_GROUP_FAILED);
		}
	}

	if (nick) free(nick);
	if (tnick) free(tnick);

	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

/**
 * Creates a new alias in NickServ database.
 **/
NickAlias *makealias(const char *nick, NickCore * nc) {
	NickAlias *na;

	/* Just need to make the alias */
	na = scalloc(1, sizeof(NickAlias));
	na->nick = sstrdup(nick);
	na->nc = nc;
	slist_add(&nc->aliases, na);
	alpha_insert_alias(na);
	return na;
}

/* ------------------------------------------------------------------------------- */

/**
 * Add language strings to the module's language db.
 **/
void add_languages(void) {
	char *langtable_en_us[] = {
		/* LANG_SAGROUP_DESC */
		" SAGROUP      Force a nick to join a group",
		/* LANG_SAGROUP_SYNTAX */
		" Syntax: SAGROUP \037nick\037 \037target nick\037",
		/* LANG_SAGROUP_SYNTAX_EXT */
		" Syntax: \002SAGROUP \037nick\037 \037target nick\037\002\n"
		" This command forces the nickname to join the target nicknames' group.\n"
		" \n"
		" It is recommended to use this command with a non-registered nick\n"
		" because it will be registered automatically when issueing the command.\n"
		" You may use it with a registered nick, however the nick will be\n"
		" dropped first and then added to the new group.\n"
		" \n"
		" A nick can only be in one group at a time.\n"
		" Group merging is not possible.\n"
		" \n"
		" Note: All nicknames of a group have the same password.",
		/* LANG_GROUP_JOINED */
		" %s has now joined the group of %s(%s).",
		/* LANG_NICK_SAGROUPED */
		" This nick has been forced to join the group of %s by %s. \n"
		" Please contact Network Staff for more information.",
		/* LANG_GROUP_SAME */
		" %s is already a member of the group of %s.",
	};

	char *langtable_nl[] = {
		/* LANG_SAGROUP_DESC */
		" SAGROUP      Forceer een nick in een groep",
		/* LANG_SAGROUP_SYNTAX */
		" Syntax: SAGROUP \037nick\037 \037doel nick\037",
		/* LANG_SAGROUP_SYNTAX_EXT */
		" Syntax: \002SAGROUP \037nick\037 \037doel nick\037\002\n"
		" Dit commando plaatst de nicknaam in de groep van de doel nick.\n"
		" \n"
		" Het is aangeraden om dit commando enkel met niet geregistreerde nicks te\n"
		" gebruiket vermits de nick toch automatisch wordt geregistreerd.\n"
		" Indien  het toch wordt gebruikt op een reeds geregistreerde nick, wordt\n"
		" deze eerst verwijderd alvorens hij toegevoegd wordt aan de doelgroep.\n"
		" \n"
		" Een nick kan slechts in één groep tegelijkertijd zijn.\n"
		" Groepen samenvoegen is niet mogelijk.\n"
		" \n"
		" Note: Alle nicknamen van de groep hebben hetzelfde wachtwoord.",
		/* LANG_GROUP_JOINED */
		" %s is nu in de groep van %s(%s) geplaatst.",
		/* LANG_NICK_SAGROUPED */
		" Deze nickname is in de nick groep van %s geplaatst door %s. \n"
		" Contacteer de Netwerk Administratie voor meer informatie.",
		/* LANG_GROUP_SAME */
		" %s is reeds lid van de groep van %s.",
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
	moduleInsertLanguage(LANG_NL, LANG_NUM_STRINGS, langtable_nl);
}

/* EOF */
