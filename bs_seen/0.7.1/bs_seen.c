/***************************************************************************
 **  bs_seen.c  ********************* Author: GeniusDex ** Version: 0.7.1 **
 ***************************************************************************
 *                                                                         *
 * Service:     BotServ                                                    *
 * Module:      Improved !seen                                             *
 * Version:     0.7.1                                                      *
 * License:     GPL [GNU Public License]                                   *
 * Author:      GeniusDex                                                  *
 * E-mail:      geniusdex@anope.org                                        *
 * Description: Make the !seen command more general                        *
 *                                                                         *
 *   This module improves the BotServ !seen command by adding lookups for  *
 *   all registered users allover the network. It's not limited to people  *
 *   on your channel access list anymore; not the tracking, nor the usage. *
 *                                                                         *
 ***************************************************************************
 * Languages:                                                              *
 *   English     GeniusDex   <geniusdex@anope.org>                         *
 *   Spanish     DrStein     <gacevedo@anope.org>                          *
 *   Portugese   MightyRaven                                               *
 *   French      hexa        <hexa@insiderz.org>                           *
 *   Turkish     guru                                                      *
 *   Italian     Hal9000     <hal9000@musichat.net>                        *
 *   German      Certus      <certus@anope.org>                            *
 *   Catalan                                                               *
 *   Greek                                                                 *
 *   Dutch       GeniusDex   <geniusdex@anope.org>                         *
 *   Russian                                                               *
 * All open languages default to English                                   *
 ***************************************************************************
 **  CHANGES  *****************************************  VERSION HISTORY  **
 ***************************************************************************
 *** 0.7.1 ************************************************** 06/01/2007 ***
 * -Fixed a crash when no target was passed to !seen                       8
 *** 0.7.0 ************************************************** 01/10/2006 ***
 * -General code review and cleanup                                        *
 * -Minor fixes to make it work right with Anope 1.7.15                    *
 *** 0.6.0 ************************************************** 01/09/2005 ***
 * -Converted language system to use Anope's built-in system               *
 * -Converted fantasy parser to use events instead of a PRIVMSG handler    *
 *** 0.5.0 ************************************************** 23/08/2004 ***
 * -Fixed compile errors for anope 1.7.6                                   *
 *** 0.4.1 ************************************************** 20/07/2004 ***
 * -Added French language                                                  *
 *** 0.4.0 ************************************************** 20/07/2004 ***
 * -Should now work on win32                                               *
 *** 0.3.1 ************************************************** 17/05/2004 ***
 * -Added Italian language                                                 *
 * -Fixed a bug causing incorrect searches due to some memory stuff blabla *
 *** 0.3.0 ************************************************** 17/04/2004 ***
 * -Made DeMiNi0 happy....                                                 *
 * -Added Turkish language                                                 *
 *** 0.2.0 ************************************************** 11/04/2004 ***
 * -BS_UPDATE_LAST_VISIT_ON_PRIVMSG did effecitvely nothing, so removed    *
 * -Implemented multilingual support. Some languages are still missing and *
 *     thus default to English. Currently supported: English, Dutch,       *
 *     Spanish, German, Portugese.                                         *
 * -Fixed a segfault when ommiting target                                  *
 * -Fixed a bug with botserv handling his channel strings for kickers      *
 *** 0.1.0 ************************************************** 08/04/2004 ***
 * -First testing release                                                  *
 ***************************************************************************
 ***************************************************************************
 **  CONFIGURATION  *************************************  CONFIGURATION  **
 ***************************************************************************
 * Scaringly empty...                                                      *
 ***************************************************************************
 ** END OF CONFIGURATION *************************** END OF CONFIGURATION **
 ****************** Don't change anything below this line ******************
 **************************************************************************/

#include "module.h"

#define AUTHOR "GeniusDex"
#define VERSION "0.7.1"

#define LNG_NUM_STRINGS    11

#define LANG_SEEN_BOT            0
#define LANG_SEEN_YOU            1
#define LANG_SEEN_ON_CHANNEL     2
#define LANG_SEEN_ON_CHANNEL_AS  3
#define LANG_SEEN_ONLINE         4
#define LANG_SEEN_ONLINE_AS      5
#define LANG_SEEN_WAS_ONLINE     6
#define LANG_SEEN_WAS_ONLINE_AS  7
#define LANG_SEEN_NEVER          8
#define LANG_SEEN_UNKNOWN        9
#define LANG_SEEN_FAILED         10

int do_fantasy(int ac, char **av);
void do_seen(User * u, ChannelInfo * ci, char *target);
void my_add_languages(void);

/* Initialize the module; make all the needed info known to the Anope core */
int AnopeInit(int argc, char **argv)
{
	int i;
	EvtHook *hook;
	
	my_add_languages();
	
	/* Hook to the fantasy event (for !* channel messages) */
	hook = createEventHook(EVENT_BOT_FANTASY, do_fantasy);
	if ((i = moduleAddEventHook(hook))) {
		alog("[bs_seen] Unable to hook to fantasy events (%d)", i);
		return MOD_STOP;
	}
	
	/* Let Anope know who we are */
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	
	/* All done nicely, tell Anope we're fine to go */
	return MOD_CONT;
}

/* Do any unloading steps if required, but none are required. This
 * function is only here to fix compiling on win32.
 */
void AnopeFini(void)
{
	/* Empty! I told you! */
}

/* Fantasy commands handler */
int do_fantasy(int ac, char **av)
{
	User *u;
	ChannelInfo *ci;
	char *target;
	
	/* See if we have enough arguments (command, user, channel, target) */
	if (ac < 4)
		return MOD_CONT;
	
	/* Only handle !seen commands */
	if (stricmp(av[0], "seen") == 0) {
		u = finduser(av[1]);
		ci = cs_findchan(av[2]);
		
		if (!u || !ci)
			return MOD_CONT;
		
		/* Get the target we want to inspect */
		target = myStrGetToken(av[3], ' ', 0);
		
		if (!target)
			return MOD_CONT;
		
		/* Run the actual seen code */
		do_seen(u, ci, target);
		free(target);
		
		/* We return MOD_STOP here to avoid double answers on !seen
		 * requests; this includes the !seen from the Anope core
		 */
		return MOD_STOP;
	}
	
	return MOD_CONT;
}

/* Actual seen handler. This function handles the seen requests by the given
 * user u on the channel ci for the given target. It uses the buf string to
 * save the current response in.
 */
void do_seen(User * u, ChannelInfo * ci, char *target)
{
	char buf[BUFSIZE];
	char durastr[192];
	char *nick;
	int i;
	time_t lastseen;
	
	User *u2;
	NickAlias *na;
	NickAlias *na2;
	NickAlias *na3;
	NickCore *nc;
	
	if (debug)
		alog("debug: [bs_seen] Doing a seen for '%s' on '%s' (Requested by '%s')", ci->name, u->nick, target);
	
	buf[0] = 0;
	
	if (!stricmp(ci->bi->nick, target)) {
		/* The user is looking for the channel bot */
		snprintf(buf, sizeof(buf), moduleGetLangString(u, LANG_SEEN_BOT), u->nick);
	} else if (!(na = findnick(target)) || (na->status & NS_VERBOTEN)) {
		/* The user is looking for a forbidden or non-existing nick */
		snprintf(buf, sizeof(buf), moduleGetLangString(u, LANG_SEEN_UNKNOWN), target);
	} else if ((u2 = nc_on_chan(ci->c, na->nc))) {
		/* The user is looking for someone currently on the channel. This
		 * is either theirselves or someone on the channel. In the last case,
		 * they could also be in disguise using another nick.
		 */
		if (u == u2 || (u->na && (u->na->nc == na->nc)))
			snprintf(buf, sizeof(buf), moduleGetLangString(u, LANG_SEEN_YOU), u->nick);
		else if (!stricmp(u2->nick, target))
			snprintf(buf, sizeof(buf), moduleGetLangString(u, LANG_SEEN_ON_CHANNEL), u2->nick);
		else
			snprintf(buf, sizeof(buf), moduleGetLangString(u, LANG_SEEN_ON_CHANNEL_AS), target, u2->nick);
	} else if (na->u) {
		/* The user is looking for someone online, but not currently on the channel */
		snprintf(buf, sizeof(buf), moduleGetLangString(u, LANG_SEEN_ONLINE), na->u->nick);
	} else {
		/* We have no result so far. We'll first check if the user is online as
		 * someone else.
		 */
		nc = na->nc;
		for (i = 0; i < nc->aliases.count; i++) {
			if ((u2 = ((NickAlias *) nc->aliases.list[i])->u))
				snprintf(buf, sizeof(buf), moduleGetLangString(u, LANG_SEEN_ONLINE_AS), target, u2->nick);
		}
		if (!buf[0]) {
			/* There is still no result. We'll compare all the NickAliases of
			 * the person we're looking for and search the highest last seen
			 * time. This one will be the nick we'll report back to the user
			 */
			na3 = NULL;
			nick = NULL;
			lastseen = 0;
			for (i = 0; i < nc->aliases.count; i++) {
				na2 = (NickAlias *) nc->aliases.list[i];
				if (na2->last_seen > lastseen) {
					na3 = na2;
					nick = na2->nick;
					lastseen = na2->last_seen;
				}
			}
			if (nick) {
				duration(u->na, durastr, sizeof(durastr), time(NULL) - lastseen);
				if (!stricmp(nick, target))
					snprintf(buf, sizeof(buf), moduleGetLangString(u, LANG_SEEN_WAS_ONLINE), target, durastr);
				else
					snprintf(buf, sizeof(buf), moduleGetLangString(u, LANG_SEEN_WAS_ONLINE_AS), target, durastr, nick);
			}
		}
	}
	
	/* Check if we have a result. If not, claim we never saw the target */
	if (!buf[0])
		snprintf(buf, sizeof(buf), moduleGetLangString(u, LANG_SEEN_NEVER), target);
	
	/* Send the result back to the channel */
	anope_cmd_privmsg(ci->bi->nick, ci->name, buf);
}

/* Define and add the languages to the system */
void my_add_languages(void)
{
	char *langtable_en_us[] = {
		/* LANG_SEEN_BOT */
		"You found me, %s!",
		/* LANG_SEEN_YOU */
		"You might see yourself in the mirror, %s...",
		/* LANG_SEEN_ON_CHANNEL */
		"%s is on the channel right now!",
		/* LANG_SEEN_ON_CHANNEL_AS */
		"%s is on the channel right now (as %s) !",
		/* LANG_SEEN_ONLINE */
		"%s is online right now!",
		/* LANG_SEEN_ONLINE_AS */
		"%s is online right now (as %s) !",
		/* LANG_SEEN_WAS_ONLINE */
		"%s was last seen online %s ago.",
		/* LANG_SEEN_WAS_ONLINE_AS */
		"%s was last seen online %s ago (as %s) .",
		/* LANG_SEEN_NEVER */
		"I've never seen %s.",
		/* LANG_SEEN_UNKNOWN */
		"I don't know who %s is.",
		/* LANG_SEEN_FAILED */
		"Seen query failed."
	};

	char *langtable_es[] = {
		/* LANG_SEEN_BOT */
		"%s me has encontrado!",
		/* LANG_SEEN_YOU */
		"Te podrias mirar en un espejo, %s...",
		/* LANG_SEEN_ON_CHANNEL */
		"%s esta en el canal ahora mismo!",
		/* LANG_SEEN_ON_CHANNEL_AS */
		"%s esta ahora en el canal (como %s) !",
		/* LANG_SEEN_ONLINE */
		"%s esta ahora en linea!",
		/* LANG_SEEN_ONLINE_AS */
		"%s esta ahora en linea (como %s) !",
		/* LANG_SEEN_WAS_ONLINE */
		"%s fue visto en linea %s atras.",
		/* LANG_SEEN_WAS_ONLINE_AS */
		"%s fue visto en linea %s atras (como %s) .",
		/* LANG_SEEN_NEVER */
		"Nunca he visto a %s.",
		/* LANG_SEEN_UNKNOWN */
		"No se quien es %s.",
		/* LANG_SEEN_FAILED */
		"La busqueda ha fallado."
	};

	char *langtable_pt[] = {
		/* LANG_SEEN_BOT */
		"Voce me achou, %s!",
		/* LANG_SEEN_YOU */
		"Talves voce se veja no espelho, %s...",
		/* LANG_SEEN_ON_CHANNEL */
		"%s esta no canal agora!",
		/* LANG_SEEN_ON_CHANNEL_AS */
		"%s esta no canal agora (as %s) !",
		/* LANG_SEEN_ONLINE */
		"%s é online agora!",
		/* LANG_SEEN_ONLINE_AS */
		"%s é online agora (as %s) !",
		/* LANG_SEEN_WAS_ONLINE */
		"%s visto online para a ultima veis %s atrais.",
		/* LANG_SEEN_WAS_ONLINE_AS */
		"%s visto online para a ultima veis %s atrais (as %s) .",
		/* LANG_SEEN_NEVER */
		"Eu nunca viu %s.",
		/* LANG_SEEN_UNKNOWN */
		"Eu não sei quen %s é.",
		/* LANG_SEEN_FAILED */
		"Seen pergunta falhou."
	};

	char *langtable_fr[] = {
		/* LANG_SEEN_BOT */
		"Tu m'as trouv!, %s!",
		/* LANG_SEEN_YOU */
		"Tu pourrais te regarder toi-mme dans le miroir, %s.",
		/* LANG_SEEN_ON_CHANNEL */
		"%s est dans le chatroom en ce moment.",
		/* LANG_SEEN_ON_CHANNEL_AS */
		"%s est dans le chatroom comme %s en ce moment.",
		/* LANG_SEEN_ONLINE */
		"%s est en ligne",
		/* LANG_SEEN_ONLINE_AS */
		"%s est en ligne (comme %s)",
		/* LANG_SEEN_WAS_ONLINE */
		"%s tait en ligne il y a %s.",
		/* LANG_SEEN_WAS_ONLINE_AS */
		"%s tait en ligne comme %s il y a %s.",
		/* LANG_SEEN_NEVER */
		"Je n'ai jamais vu %s.",
		/* LANG_SEEN_UNKNOWN */
		"Je ne connais pas %s.",
		/* LANG_SEEN_FAILED */
		"Demande choue."
	};

	char *langtable_tr[] = {
		/* LANG_SEEN_BOT */
		"%s beni buldun!",
		/* LANG_SEEN_YOU */
		"%s kendini aynada da görebilirsin...",
		/* LANG_SEEN_ON_CHANNEL */
		"%s þu an zaten kanalda!",
		/* LANG_SEEN_ON_CHANNEL_AS */
		"%s þu an zaten kanalda (%s rumuzu ile) !",
		/* LANG_SEEN_ONLINE */
		"%s þu an çevrimiçi!",
		/* LANG_SEEN_ONLINE_AS */
		"%s þu an çevrimiçi (%s rumuzu ile) !",
		/* LANG_SEEN_WAS_ONLINE */
		"%s en son %s önce çevrimiçi görüldü.",
		/* LANG_SEEN_WAS_ONLINE_AS */
		"%s en son %s önce çevrimiçi görüldü (%s rumuzu ile) .",
		/* LANG_SEEN_NEVER */
		"%s rumuzlu kiþiyi hiç görmedim.",
		/* LANG_SEEN_UNKNOWN */
		"%s rumuzlu kiþinin kim olduðunu bilmiyorum.",
		/* LANG_SEEN_FAILED */
		"En son görülme sorgusu baþarýlamadý."
	};

	char *langtable_it[] = {
		/* LANG_SEEN_BOT */
		"Mi hai trovato, %s!",
		/* LANG_SEEN_YOU */
		"Potresti vederti nello specchio, %s...",
		/* LANG_SEEN_ON_CHANNEL */
		"%s si trova nel canale ora!",
		/* LANG_SEEN_ON_CHANNEL_AS */
		"%s si trova nel canale ora (come %s) !",
		/* LANG_SEEN_ONLINE */
		"%s è online ora!",
		/* LANG_SEEN_ONLINE_AS */
		"%s è online ora (come %s) !",
		/* LANG_SEEN_WAS_ONLINE */
		"%s è stato visto online %s fa.",
		/* LANG_SEEN_WAS_ONLINE_AS */
		"%s è stato visto online %s fa (come %s) .",
		/* LANG_SEEN_NEVER */
		"Non ho mai visto %s.",
		/* LANG_SEEN_UNKNOWN */
		"Non so chi sia %s.",
		/* LANG_SEEN_FAILED */
		"La ricerca non ha avuto esito."
	};

	char *langtable_de[] = {
		/* LANG_SEEN_BOT */
		"Du hast mich gefunden, %s!",
		/* LANG_SEEN_YOU */
		"Du könntest dich selbst im Spiegel sehen, %s...",
		/* LANG_SEEN_ON_CHANNEL */
		"%s ist gerade im Channel!",
		/* LANG_SEEN_ON_CHANNEL_AS */
		"%s ist gerade im Channel (als %s) !",
		/* LANG_SEEN_ONLINE */
		"%s ist gerade online!",
		/* LANG_SEEN_ONLINE_AS */
		"%s ist gerade online (als %s) !",
		/* LANG_SEEN_WAS_ONLINE */
		"%s wurde zuletzt vor %s online gesehen.",
		/* LANG_SEEN_WAS_ONLINE_AS */
		"%s wurde zuletzt vor %s online gesehen (als %s) .",
		/* LANG_SEEN_NEVER */
		"Ich habe %s nie gesehen.",
		/* LANG_SEEN_UNKNOWN */
		"Ich kenne %s nicht.",
		/* LANG_SEEN_FAILED */
		"Seen Anfrage fehlgeschlagen."
	};

	char *langtable_nl[] = {
		/* LANG_SEEN_BOT */
		"Je hebt me gevonden, %s!",
		/* LANG_SEEN_YOU */
		"Een blik in de spiegel zou geen kwaad kunnen, %s...",
		/* LANG_SEEN_ON_CHANNEL */
		"%s is nu op het kanaal!",
		/* LANG_SEEN_ON_CHANNEL_AS */
		"%s is nu op het kanaal (als %s) !",
		/* LANG_SEEN_ONLINE */
		"%s is nu online!",
		/* LANG_SEEN_ONLINE_AS */
		"%s is nu online (als %s) !",
		/* LANG_SEEN_WAS_ONLINE */
		"%s is %s geleden voor het laatst online gezien.",
		/* LANG_SEEN_WAS_ONLINE_AS */
		"%s is %s geleden voor het laatst online gezien (als %s) .",
		/* LANG_SEEN_NEVER */
		"Ik heb %s nooit gezien.",
		/* LANG_SEEN_UNKNOWN */
		"Ik weet niet wie %s is.",
		/* LANG_SEEN_FAILED */
		"Zoekopdracht mislukt."
	};

	moduleInsertLanguage(LANG_EN_US, LNG_NUM_STRINGS, langtable_en_us);
	moduleInsertLanguage(LANG_ES, LNG_NUM_STRINGS, langtable_es);
	moduleInsertLanguage(LANG_PT, LNG_NUM_STRINGS, langtable_pt);
	moduleInsertLanguage(LANG_FR, LNG_NUM_STRINGS, langtable_fr);
	moduleInsertLanguage(LANG_TR, LNG_NUM_STRINGS, langtable_tr);
	moduleInsertLanguage(LANG_IT, LNG_NUM_STRINGS, langtable_it);
	moduleInsertLanguage(LANG_DE, LNG_NUM_STRINGS, langtable_de);
	moduleInsertLanguage(LANG_NL, LNG_NUM_STRINGS, langtable_nl);
}

/* EOF */
