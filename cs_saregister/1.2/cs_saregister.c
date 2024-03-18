/**
 * -----------------------------------------------------------------------------
 * Name    : cs_saregister
 * Author  : Viper <Viper@Anope.org>
 * Date    : 21/10/2009 (Last update: 15/01/2011)
 * Version : 1.2
 * -----------------------------------------------------------------------------
 * Requires    : Anope-1.8.5
 * Tested      : Anope 1.8.5 SVN + InspIRCd 1.2.0
 * -----------------------------------------------------------------------------
 * This module will allow Services Admins to register channels for another user.
 * Note that when using SAREGISTER much less restrictions will be enforced upon
 * the registration of a channel: the number of channels already registered under
 * the target users' account will not be restricted, user does not have to be
 * online/on thet channel, etc...
 *
 * This module is particularly useful in combination with cs_register_operonly.
 * -----------------------------------------------------------------------------
 *
 * Changelog:
 *
 *   1.2  -  Fixed buffer overflow when generating password.
 *           Fixed a couple of memleaks.
 *
 *   1.1  -  Removed references to enc_encrypt_in_place() which is no longer supported by 1.8.5.
 *
 *   1.0  -  Initial Release.
 *
 * -----------------------------------------------------------------------------
 **/

#include "module.h"
#define AUTHOR "Viper"
#define VERSION "1.2"


/* Language defines */
#define LANG_NUM_STRINGS 					6

#define LANG_SAREG_DESC						0
#define LANG_SAREG_SYNTAX					1
#define LANG_SAREG_SYNTAX_EXT				2
#define LANG_SAREG_REGISTERED				3
#define LANG_SAREG_MEMO						4
#define LANG_TARGET_REGGED					5


/* Functions */
void do_help_list(User *u);
int do_help(User *u);

int do_saregister(User * u);
void send_reg_memo(User * u, char *target, char *chan, char *pass);

void add_languages(void);

/**
 * Create the off command, and tell anope about it.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv) {
	Command *c;

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	alog("[\002cs_saregister\002] Loading module...");

	if (!moduleMinVersion(1,8,5,3037)) {
		alog("[\002cs_saregister\002] Your version of Anope isn't supported. Please update to a newer release.");
		return MOD_STOP;
	}

	c = createCommand("SAREGISTER", do_saregister, is_services_admin, -1, -1, -1, -1, -1);
	if (moduleAddCommand(CHANSERV, c, MOD_UNIQUE) != MOD_ERR_OK) {
		alog("[\002cs_saregister\002] Cannot create SAREGISTER command...");
		return MOD_STOP;
	}
	moduleAddHelp(c, do_help);
	moduleSetChanHelp(do_help_list);
	c = createCommand("SAREG", do_saregister, is_services_admin, -1, -1, -1, -1, -1);
	if (moduleAddCommand(CHANSERV, c, MOD_UNIQUE) != MOD_ERR_OK) {
		alog("[\002cs_saregister\002] Cannot create SAREG command...");
	}

	add_languages();

	alog("[\002cs_saregister\002] Module loaded successfully...");
	return MOD_CONT;
}

/**
 * Unload the module
 **/
void AnopeFini(void) {
	alog("[\002cs_saregister\002] Unloading module...");
}

/* ------------------------------------------------------------------------------- */

/**
 * Add the SAREGISTER command to the ChanServ HELP listing.
 * @param u The user who is requesting help.
 **/
void do_help_list(User *u) {
	if (is_services_admin(u))
		moduleNoticeLang(s_ChanServ, u, LANG_SAREG_DESC);
}


/**
 * Show the extended help on the SAREGISTER command.
 **/
int do_help(User *u) {
	if (is_services_admin(u))
		moduleNoticeLang(s_ChanServ, u, LANG_SAREG_SYNTAX_EXT, s_ChanServ);
	else
		notice_lang(s_ChanServ, u, PERMISSION_DENIED);
	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

/**
 * The /cs register command.
 * @param u The user who issued the command
 * @param MOD_CONT to continue processing other modules, MOD_STOP to stop processing.
 **/
int do_saregister(User *u) {
	char *buffer, *chan, *target, *desc;
	Channel *c;
	ChannelInfo *ci;
	User *u2;
	NickAlias *na;
	NickCore *nc;
	struct u_chaninfolist *uc;
	int is_servadmin = is_services_admin(u);
	char pass[11];
	int idx, min = 1, max = 62;
	int chars[] = { ' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
		'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
		'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
		'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
		'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
	};

	if (readonly) {
		notice_lang(s_ChanServ, u, CHAN_REGISTER_DISABLED);
		return MOD_CONT;
	}

	if (checkDefCon(DEFCON_NO_NEW_CHANNELS)) {
		notice_lang(s_ChanServ, u, OPER_DEFCON_DENIED);
		return MOD_CONT;
	}
	
	if (!is_servadmin) {
		notice_lang(s_ChanServ, u, PERMISSION_DENIED);
		return MOD_CONT;
	}

	buffer = moduleGetLastBuffer();
	chan = myStrGetToken(buffer, ' ', 0);
	target = myStrGetToken(buffer, ' ', 1);
	desc = myStrGetTokenRemainder(buffer, ' ', 2);

	if (!chan || !target || !desc) {
		moduleNoticeLang(s_ChanServ, u, LANG_SAREG_SYNTAX);
	} else if (*chan == '&') {
		notice_lang(s_ChanServ, u, CHAN_REGISTER_NOT_LOCAL);
	} else if (*chan != '#') {
		notice_lang(s_ChanServ, u, CHAN_SYMBOL_REQUIRED);
	} else if (!anope_valid_chan(chan)) {
		notice_lang(s_ChanServ, u, CHAN_X_INVALID, chan);
	} else if (!(na = findnick(target)) || !(nc = na->nc)) {
		moduleNoticeLang(s_ChanServ, u, LANG_TARGET_REGGED);
	} else if ((ci = cs_findchan(chan)) != NULL) {
		if (ci->flags & CI_VERBOTEN) {
			alog("%s: Attempt to register FORBIDden channel %s by %s!%s@%s", s_ChanServ, ci->name, u->nick, u->username, u->host);
			notice_lang(s_ChanServ, u, CHAN_MAY_NOT_BE_REGISTERED, chan);
		} else {
			notice_lang(s_ChanServ, u, CHAN_ALREADY_REGISTERED, chan);
		}
	} else if (!(ci = makechan(chan))) {
		alog("%s: makechan() failed for REGISTER %s", s_ChanServ, chan);
		notice_lang(s_ChanServ, u, CHAN_REGISTRATION_FAILED);
	} else {
		/* We don't require the channel to be open when we register it.. */
		c = findchan(chan);
		u2 = finduser(target);
		/* If the user isn't identified, don't notice him anything sensitive.. */
		if (u2 && !nick_identified(u2)) u2 = NULL;

		/* Generate password.. */
		memset(pass, 0, 11);
		for (idx = 0; idx < 9; idx++) {
			pass[idx] =
				chars[(1 +
					   (int) (((float) (max - min)) * getrandom16() /
							  (65535 + 1.0)) + min)];
		} pass[idx] = '\0';

		if (enc_encrypt(pass, strlen(pass), ci->founderpass, PASSMAX - 1) < 0) {
			alog("%s: Couldn't encrypt password for %s (REGISTER)", s_ChanServ, chan);
			notice_lang(s_ChanServ, u, CHAN_REGISTRATION_FAILED);
			delchan(ci);
		} else {
			if (c) c->ci = ci;
			ci->c = c;
			ci->bantype = CSDefBantype;
			ci->flags = CSDefFlags;
			ci->mlock_on = ircd->defmlock;
			ci->memos.memomax = MSMaxMemos;
			ci->last_used = ci->time_registered;
			ci->founder = nc;

			ci->desc = sstrdup(desc);
			if (c && c->topic) {
				ci->last_topic = sstrdup(c->topic);
				strscpy(ci->last_topic_setter, c->topic_setter, NICKMAX);
				ci->last_topic_time = c->topic_time;
			} else {
				/* Set this to something, otherwise it will maliform the topic */
				strscpy(ci->last_topic_setter, s_ChanServ, NICKMAX);
			}
			ci->bi = NULL;
			ci->botflags = BSDefFlags;
			ci->founder->channelcount++;
			alog("%s: Channel '%s' registered by %s!%s@%s on behalf of %s(%s)", s_ChanServ, chan,
				 u->nick, u->username, u->host, target, nc->display);
			moduleNoticeLang(s_ChanServ, u, LANG_SAREG_REGISTERED, chan, target);
			
			/* If the target user / owner is online, send him the notice 
			 * as well as the channel password. */
			if (u2) {
				notice_lang(s_ChanServ, u2, CHAN_REGISTERED, chan, target);
				notice_lang(s_ChanServ, u2, CHAN_PASSWORD_IS, pass);

				/* Auto-ID the user to the newly regged channel.. */
				uc = scalloc(sizeof(*uc), 1);
				uc->next = u2->founder_chans;
				uc->prev = NULL;
				if (u2->founder_chans)
					u2->founder_chans->prev = uc;
				u2->founder_chans = uc;
				uc->chan = ci;
			}
			/* Send the password to the channel owner by memo.. */
			send_reg_memo(u, target, chan, pass);

			/* Remove the plain text password from out memory.. */
			memset(pass, 0, strlen(pass));

			/* Implement new mode lock */
			check_modes(c);
			/* On most ircds you do not receive the admin/owner mode till its registered */
			if (u2 && ircd->admin) {
				anope_cmd_mode(s_ChanServ, chan, "%s %s", ircd->adminset, u2->nick);
			}
			if (u2 && ircd->owner && ircd->ownerset) {
				anope_cmd_mode(s_ChanServ, chan, "%s %s", ircd->ownerset, u2->nick);
			}
			send_event(EVENT_CHAN_REGISTERED, 1, chan);
		}
	}
	if (chan) free(chan);
	if (target) free(target);
	if (desc) free(desc);
	return MOD_CONT;
}

/**
 * Sends a memo to the user informing him the channel has been registered.
 * Also contains the channels password.
 **/
void send_reg_memo(User * u, char *target, char *chan, char *pass) {
	char *fmt = NULL;
	int lang = LANG_EN_US;
	char buf[4096];
	NickAlias *na;

	if ((mod_current_module_name)
		&& (!mod_current_module
			|| strcmp(mod_current_module_name, mod_current_module->name)))
		mod_current_module = findModule(mod_current_module_name);

	na = findnick(target);
	/* Find the users lang, and use it if we cant */
	if (na && na->nc)
		lang = na->nc->language;

	/* If the users lang isnt supported, drop back to enlgish */
	if (mod_current_module->lang[lang].argc == 0)
		lang = LANG_EN_US;

	/* If the requested lang string exists for the language */
	if (mod_current_module->lang[lang].argc > LANG_SAREG_MEMO) {
		fmt = mod_current_module->lang[lang].argv[LANG_SAREG_MEMO];

		snprintf(buf, sizeof(buf), fmt, chan, u->nick, pass);
		memo_send(u, target, buf, 2);
	} else
		alog("[\002cs_saregister\002] Error: Could not find memo lang string!");
}

/* ------------------------------------------------------------------------------- */

void add_languages(void) {
	char *langtable_en_us[] = {
		/* LANG_SAREG_DESC */
		" SAREGISTER       Register a channel for another user.",
		/* LANG_SAREG_SYNTAX */
		" Syntax: SAREGISTER \037channel\037 \037nick\037 \037description\037",
		/* LANG_SAREG_SYNTAX_EXT */
		" Syntax: \002SAREGISTER \037channel\037 \037nick\037 \037description\037\002\n"
		" \n"
		" Registers a channel with %s on behalf of another user.\n"
		" \n"
		" The user on behalf of whom the channel is being registered\n"
		" must have own a registered nick.\n"
		" A password is randomly generated and send to the target user\n"
		" by notice (if online) and memo.\n"
		" The last parameter, which must be included, is a\n"
		" general description of the channel's purpose.\n"
		" \n"
		" When you register the channel through SAREGISTER, the target\n"
		" user will be recorded as the founder of the channel.",
		/* LANG_SAREG_REGISTERED */
		" Channel \002%s\002 registered under the nickname \002%s\002.",
		/* LANG_SAREG_MEMO */
		" Channel %s was regged under your account by %s. A temprorary password was randomly generated: \002%s\002.",
		/* LANG_TARGET_REGGED */
		" Given nick is not a registered.",
	};

	char *langtable_nl[] = {
		/* LANG_SAREG_DESC */
		" SAREGISTER       Registreer een kanaal voor een andere gebruiker.",
		/* LANG_SAREG_SYNTAX */
		" Gebruik: SAREGISTER \037kanaal\037 \037nick\037 \037beschrijving\037",
		/* LANG_SAREG_SYNTAX_EXT */
		" Gebruik: \002SAREGISTER \037kanaal\037 \037nick\037 \037beschrijving\037\002\n"
		" \n"
		" Registreert een kanaal bij %s in naam van een andere gebruiker.\n"
		" \n"
		" De gebruiker voor wie het kanaal wordt geregistreert moet een\n"
		" geregistreerde nick zijn."
		" Een wachtwoord wordt willekeurig gegenereerd en \n"
		" naar de beoogde eigenaar van het kanaal verstuurd via notice\n"
		" (als gebruiker online is) en via memo.\n"
		" De laatste paramenter, die moet worden opgegeven, is een\n"
		" algemene beschrijving van het doel van het kanaal.\n"
		" \n"
		" Wanneer je een kanaal registreert via SAREGISTER wordt\n"
		" de doel-gebruiker ingesteld als stichter van het kanaal.\n",
		/* LANG_SAREG_REGISTERED */
		" Kanaal \002%s\002 is geregistreerd onder de nickname: \002%s\002.",
		/* LANG_SAREG_MEMO */
		" Kanaal %s is geregistreerd in jouw naam door %s. Een tijdelijk wachtwoord was willekeurig gegenereerd: \002%s\002.",
		/* LANG_TARGET_REGGED */
		" De opgegeven nick is niet geregistreerd.",
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
	moduleInsertLanguage(LANG_NL, LANG_NUM_STRINGS, langtable_nl);
}

/* EOF */

