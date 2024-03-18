/**
 * -----------------------------------------------------------------------------
 * Name    : bs_fantasy_karma
 * Author  : Viper  <Viper@Anope.org>
 * Date    : 21/08/2011  (Last update: 24/12/2011)
 * Version : 1.0
 * -----------------------------------------------------------------------------
 * Requires   : Anope-1.8.7
 * Tested     : Anope-1.8.7 + UnrealIRCd 3.2.8.1
 * -----------------------------------------------------------------------------
 * This module adds a karma system to the botserv functionality.
 * Karma is stored on a channel by channel basis and the system must be
 * turned on by the channels owner.
 *
 * Commands:
 *     !set karma on/off     Turns the karma system on and off for the channel.
 *     !karma [<nick>]       Show karma attributed to nick or current user.
 *     !karma <nick> ALL     Show karma attributed to nick, including who gave it.
 *     <nick>++              Increments karma for given user.
 *     <nick>--              Decrements karma for given user.
 *
 * Karma can only be given to registered nicknames and can only be given
 * by registered users. Karma can be given until the limit defined in the
 * services.conf is reached - by default 1 - and it is possible to define
 * a minimum interval required between successive karma increases/decreases.
 *
 * Note that this module should be placed in ModuleDelayedAutoload,
 * and NOT in ModuleAutoload or one of the CoreModules directives.
 *
 * This module is released under the GPL 2 license.
 * -----------------------------------------------------------------------------
 * Translations:
 *
 *     - English and Dutch Languages provided and maintained by module author.
 *
 * -----------------------------------------------------------------------------
 *
 * Changelog:
 *
 *    1.0    Fixed SUPPORTED pre-compile configuration setting.
 *           Fixed DB loading routine aborting over missing(/expired) nicks or channels.
 *           Fixed crash when !karma <nick> all is issued by an unregistered user.
 *
 *    0.4    Changed how the validation code is calculated.
 *           Fixed a bug in the code validation on loading the DB.
 *           Fixed SAs not being able to use !set karma on.
 *           Fixed missing event hooks.
 *           Fixed flags not getting updated on reload.
 *
 *    0.3    Added a reply showing a users new karma after giving karma.
 *           Added ability to contribute karma to users up to a configured max.
 *           Added ability to define a minimum interval between consecutive contributions.
 *           Added simple checksum to validate DB records.
 *           Changed turning karma on/off sending the reply to the channel.
 *           Changed access lvl required for querying with the ALL param to voice.
 *           Fixed replies to !karma to go to the channel; errors are still a notice.
 *
 *    0.2    Added Dutch language strings.
 *           Ignore lines without a nick ('++'). (Reported by Adam)
 *           Fixed typos/spelling mistakes. (Reported by Adam)
 *           Fixed buffer overflow when nicklenght is above 33. (Reported by Adam)
 *
 *    0.1    Initial development release.
 *
 * -----------------------------------------------------------------------------
 **/

/**
 * TODO:
 *
 *   <<< No idea.. hope there are no bugs left.. >>>
 *
 **/

/**
 * Configuration directives that should be copy-pasted to services.conf

# BSKarmaDB [OPTIONAL]
# Module: bs_fantasy_karma
#
# Use the given filename as database to store the karma data.
# If not given, the default of "bs_karma.db" will be used.
#
#BSKarmaDB "bs_karma.db"

# BSKarmaGiveMaxTotal [OPTIONAL - Recommended]
# Module: bs_fantasy_karma
#
# Maximum karma a user can give to another user.
# If BSKarmaGiveInterval is not defined, this is fixed to 1 and thus users
# will only be able to give karma to other users once. (Undoing previously
# given karma is still possible though).
# When set, users will be able to give karma multiple times, until they reach
# the defined limit. (The limit defines a max positive and negative value.)
#
#BSKarmaGiveMaxTotal 25

# BSKarmaGiveInterval [OPTIONAL - Recommended]
# Module: bs_fantasy_karma
#
# Minimum interval between successive - same sign - karma contributions to
# the same user. This is only useful when BSKarmaGiveMaxTotal is set
# and has a value greater then 1. (Without this, karma can only
# be given once in any case.)
# When set, users will be able to give karma until the limit is reached, provided
# that BSKarmaGiveInterval time has elapsed since last giving karma.
# Undoing the previously given karma within this timeframe is possible however.
#
#BSKarmaGiveInterval 4h

 *
 **/

#include "module.h"

/*------------------------------Configuration Block----------------------------*/

/**
 * When defined the module operates in supported mode.
 * This means it will stop functioning if incompatible modules are
 * detected.
 * When commenting this or undefining, these checks will be disabled,
 * however no further support will be provided by the author in case 
 * problems arise.
 **/
#define SUPPORTED

/*-------------------------End of Configuration Block--------------------------*/

#define AUTHOR "Viper"
#define VERSION "1.0"
#define KARMADBVERSION 1

/* Language defines */
#define LANG_NUM_STRINGS 					19

#define LANG_HELP_HEADER					0
#define LANG_KARMA_DESC						1
#define LANG_KARMA_SYNTAX					2
#define LANG_KARMA_SYNTAX_EXT				3
#define LANG_SET_KARMA_DESC					4
#define LANG_SET_KARMA_SYNTAX				5
#define LANG_SET_KARMA_SYNTAX_EXT			6
#define LANG_CMD_NOT_AVAILABLE				7
#define LANG_SET_KARMA_ON					8
#define LANG_SET_KARMA_OFF					9
#define LANG_HAS_NOKARMA					10
#define LANG_HAS_KARMA						11
#define LANG_KARMA_BY_POS					12
#define LANG_KARMA_BY_NEG					13
#define LANG_KARMA_UPDATED					14
#define LANG_KARMA_MAX						15
#define LANG_KARMA_NO_SELF					16
#define LANG_KARMA_UNDOLAST					17
#define LANG_KARMA_WAIT						18


/* Database seperators */
#define SEPARATOR  '\011'          /* End of a key, seperates keys from values */
#define BLOCKEND   '\015'          /* End of a block, e.g. a whole nick/channel or a subblock */
#define VALUEEND   '\012'          /* End of a value */

/* Database reading return values */
#define DB_READ_SUCCESS			0
#define DB_READ_ERROR			1
#define DB_EOF_ERROR			2
#define DB_VERSION_ERROR		3
#define DB_READ_BLOCKEND		4

#define DB_WRITE_SUCCESS		0
#define DB_WRITE_ERROR			1
#define DB_WRITE_NOVAL			2

/* Database Key, Value max length */
#define MAXKEYLEN 32
#define MAXVALLEN 64               /* This means we support a maximum nick length of 64! */

/* Create a simple hash of the trigger to begin lookup.. */
#define HASH(nick)				((tolower((nick)[0])&31)<<5 | (tolower((nick)[1])&31))

/* Structs */
typedef struct db_file_ DBFile;
typedef struct chankarma_ ChanKarma;
typedef struct karmaentry_ KarmaEntry;
typedef struct karmadetail_ KarmaDetail;
typedef struct nickcorewrapper_ NickCoreWrapper;
typedef struct karmadetailwrapper_ KarmaDetailWrapper;

struct db_file_ {
	FILE *fptr;             /* Pointer to the opened file */
	int db_version;         /* The db version of the datafiles (only needed for reading) */
	int core_db_version;    /* The current db version of this anope source */
	char service[256];      /* StatServ/etc. */
	char filename[256];     /* Filename of the database */
	char temp_name[262];    /* Temp filename of the database */
};

struct chankarma_ {
	ChanKarma *prev, *next;
	char name[CHANMAX];
	int count;
	KarmaEntry *entries[1024];
	int enabled;
};

struct karmaentry_ {
	KarmaEntry *prev, *next;
	ChanKarma *ck;
	char *nick;
	int karma_p;
	int karma_n;
	KarmaDetail *pos, *pend;    /* Detail entries with karma >= 0 */
	KarmaDetail *neg, *nend;           /* Detail entries with karma < 0 */
};

struct karmadetail_ {
	KarmaDetail *prev, *next;
	KarmaEntry *ke;
	NickCore *nc;
	int karma;
	time_t last_update;         /* Track when karma was last given, to check when karma can be given next time.. */
	int last_karma;             /* Track what was given as this can be undone within the timeframe.. */
	KarmaDetailWrapper *kdw;    /* Reference to tracking instance.. */
};

/* Structs to track to which users a user has given karma..
 * (Enables more efficient purging of all given karma when a core is dropped by anope 
 * at the cost of some extra memory usage.) */
struct nickcorewrapper_ {
	NickCoreWrapper *prev, *next;
	NickCore *nc;
	int count;
	KarmaDetailWrapper *kdw;
};

struct karmadetailwrapper_ {
	KarmaDetailWrapper *next, *prev;
	KarmaDetail *kd;
};


/* Constants */
char *DefKarmaDB = "bs_karma.db";


/* Variables */
int supported;
char *KarmaDB;
int BSKarmaGiveInterval;
int BSKarmaGiveMaxTotal;

ChanKarma *channels[256];
NickCoreWrapper *nickcores[1024];


/* Functions */
int do_karma(char *source, int ac, char **av);
void format_period(char *buf, int size, int time);
int do_fantasy(int ac, char **av);
static void show_modinfo(User *u, ChannelInfo *ci);

ChanKarma *createChanKarma(char *chan);
ChanKarma *getChanKarma(char *chan);
void delChanKarma(char *chan);
void freeChanKarma(ChanKarma *ck);
void clearKarmaEntries(ChanKarma *ck);

KarmaEntry *createKarmaEntry(ChanKarma *ck, NickCore *nc);
KarmaEntry *getKarmaEntry(ChanKarma *ck, NickCore *nc);
KarmaEntry *getKarmaEntryByStr(ChanKarma *ck, char *nick);
void delKarma(ChanKarma *ck, NickCore *nc);
void delKarmaEntry(KarmaEntry *ke);
void freeKarmaEntry(KarmaEntry *ke);

KarmaDetail *giveKarma(KarmaEntry *ke, NickCore *nc, int karma);
void undoLastKarma(KarmaDetail *kd);
KarmaDetail *addKarma(KarmaEntry *ke, NickCore *nc, int karma);
KarmaDetail *createKarmaDetail(KarmaEntry *ke, NickCore *nc);
void updatePosKarmaDetail(KarmaDetail *kd, int karma_old);
KarmaDetail *getKarmaDetail(KarmaEntry *ke, NickCore *nc);
void delKarmaDetail(KarmaDetail *kd);
void freeKarmaDetail(KarmaDetail *kd);

NickCoreWrapper *createNickCoreWrapper(NickCore *nc);
NickCoreWrapper *getNickCoreWrapper(NickCore *nc, int create);
NickCoreWrapper *getNickCoreWrapperByStr(char *display, int create);
void delNickCoreWrapper(NickCoreWrapper *ncw);
void clear_db();

int do_cs_drop(int argc, char **argv);
int do_core_dropped(int argc, char **argv);
int do_core_newdisplay(int argc, char **argv);

void load_karma_db(void);
void save_karma_db(void);
int save_karmas(DBFile *dbptr, ChanKarma *ck);
int get_verif_code(KarmaEntry *ke, KarmaDetail *kd);
int check_verif_code(KarmaEntry *ke, KarmaDetail *kd, int code);

int do_save(int argc, char **argv);
int db_backup(int argc, char **argv);

int check_modules(void);
int event_check_module(int argc, char **argv);
int event_check_cmd(int argc, char **argv);

char *get_flags();
void update_version(void);

void load_config(void);
int reload_config(int argc, char **argv);
void add_languages(void);

int new_open_db_read(DBFile *dbptr, char **key, char **value);
int new_open_db_write(DBFile *dbptr);
void new_close_db(FILE *fptr, char **key, char **value);
int new_read_db_entry(char **key, char **value, FILE * fptr);
int new_write_db_entry(const char *key, DBFile *dbptr, const char *fmt, ...);
int new_write_db_endofblock(DBFile *dbptr);
void fill_db_ptr(DBFile *dbptr, int version, int core_version, char service[256], char filename[256]);

/* ------------------------------------------------------------------------------- */

/**
 * Hook to all the events, read the config and load the database.
 *
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv) {
	Message *msg;
	EvtHook *hook;

	alog("[\002bs_fantasy_karma\002] Loading module...");

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	supported = 1;

	if (!moduleMinVersion(1,8,7,3089)) {
		alog("[\002bs_fantasy_karma\002] Your version of Anope isn't supported. Please update to a newer release.");
		return MOD_STOP;
	}

	check_modules();
	if (supported == 0) {
		alog("[\002bs_fantasy_karma\002] Warning: Module continuing in unsupported mode!");
	} else if (supported == -1) {
		alog("[\002bs_fantasy_karma\002] Unloading module due to incompatibilities!");
		return MOD_STOP;
	}

	msg = createMessage("PRIVMSG", do_karma);
	if (moduleAddMessage(msg, MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_karma\002] Can't parse incoming msgs...");
		return MOD_STOP;
	}
	if (UseTokens) {
		msg = createMessage("!", do_karma);
		if (moduleAddMessage(msg, MOD_HEAD) != MOD_ERR_OK) {
			alog("[\002bs_fantasy_karma\002] Can't parse incoming token msgs...");
			return MOD_STOP;
		}
	}

	hook = createEventHook(EVENT_BOT_FANTASY, do_fantasy);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_karma\002] Can't hook to EVENT_BOT_FANTASY event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_BOT_FANTASY_NO_ACCESS, do_fantasy);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_karma\002] Can't hook to EVENT_BOT_FANTASY_NO_ACCESS event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_CHAN_DROP, do_cs_drop);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_karma\002] Can't hook to EVENT_CHAN_DROP event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_CHAN_EXPIRE, do_cs_drop);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_karma\002] Can't hook to EVENT_CHAN_EXPIRE event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_CORE_DROPPED, do_core_dropped);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_karma\002] Can't hook to EVENT_CORE_DROPPED event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_CORE_NEWDISPLAY, do_core_newdisplay);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_karma\002] Can't hook to EVENT_CORE_NEWDISPLAY event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_RELOAD, reload_config);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_karma\002] Can't hook to EVENT_RELOAD event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_DB_SAVING, do_save);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_karma\002] Can't hook to EVENT_DB_SAVING event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_DB_BACKUP, db_backup);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_karma\002] Can't hook to EVENT_DB_BACKUP event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_MODLOAD, event_check_module);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_karma\002] Can't hook to EVENT_MODLOAD event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_ADDCOMMAND, event_check_cmd);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_karma\002] Can't hook to EVENT_ADDCOMMAND event.");
		return MOD_STOP;
	}

	load_config();
	add_languages();
	load_karma_db();

	/* Update version info.. */
	update_version();

	alog("[\002bs_fantasy_karma\002] Module loaded successfully...");
	return MOD_CONT;
}


/**
 * Unload the module..
 **/
void AnopeFini(void) {
	/* Save the database.. */
	save_karma_db();

	/* Clear the memory.... */
	clear_db();

	alog("[\002bs_fantasy_karma\002] Unloading module...");
}

/* ------------------------------------------------------------------------------- */

int do_karma(char *source, int ac, char **av) {
	User *u;
	ChannelInfo *ci;
	ChanKarma *ck;
	KarmaEntry *ke;
	KarmaDetail *kd;
	NickAlias *na;
	char *botnick = NULL;
	int len;

	if (ac != 2)
		return MOD_CONT;
	if (supported < 0)
		return MOD_CONT;

	if (UseTS6 && ircd->ts6) {
		u = find_byuid(source);
		if (!u)
			u = finduser(source);
	} else
		u = finduser(source);
	if (!u)
		return MOD_CONT;

	if (*av[0] == '#' && av[1]) {
		if (s_BotServ && (ci = cs_findchan(av[0])) && !(ci->flags & CI_VERBOTEN) && ci->c && ci->bi) {
			if (ci->bi)
				botnick = ci->bi->nick;
			else
				botnick = s_ChanServ;

			/* Check whether the karma system is enabled for the channel.. */
			if (((ck = getChanKarma(ci->name))) && ck->enabled) {
				/* Ensure it s a single word on the line, anything else is not worth parsing.. */
				len = strlen(av[1]);
				if (len > 2 && !strstr(av[1], " ") && av[1][len - 1] == av[1][len - 2] && (
						av[1][len - 1] == '+' || av[1][len - 1] == '-') && len < NICKMAX + 2) {
					char nick[NICKMAX];
					int karma = av[1][len - 1] == '+' ? 1 : -1;

					memset(nick, 0, NICKMAX);
					strncpy(nick, av[1], len - 2);
					if (!(na = findnick(nick)) || !na->nc)
						notice_lang(botnick, u, NICK_X_NOT_REGISTERED, nick);
					else if (!nick_identified(u))
						notice_lang(botnick, u, NICK_NOT_REGISTERED);
					else if (u->na->nc == na->nc) 
						moduleNoticeLang(botnick, u, LANG_KARMA_NO_SELF);
					else {
						ke = getKarmaEntry(ck, na->nc);
						if (!ke) {
							ke = createKarmaEntry(ck, na->nc);
							kd = giveKarma(ke, u->na->nc, karma);
							karma = ke->karma_p + ke->karma_n;
							moduleNoticeLang(botnick, u, LANG_KARMA_UPDATED, na->nc->display, kd->karma > 0 ? "+" : "", kd->karma);
							moduleNoticeLang(botnick, u, LANG_HAS_KARMA, ke->nick, karma > 0 ? "+" : "", karma, ke->karma_p, abs(ke->karma_n));
						} else if ((kd = getKarmaDetail(ke, u->na->nc))) {
							/* Enforce a maximum given to a single user.. */
							if (kd->karma + karma > BSKarmaGiveMaxTotal)
								moduleNoticeLang(botnick, u, LANG_KARMA_MAX, kd->karma > 0 ? "+" : "", kd->karma, na->nc->display);

							/* Check time rstrictions.. */
							else if (BSKarmaGiveInterval && kd->last_update && kd->last_update + BSKarmaGiveInterval > time(NULL)) {
								/* Alow undoing the karma that was last given within the timeframe.. */
								if (karma == -kd->last_karma) {
									undoLastKarma(kd);
									karma = ke->karma_p + ke->karma_n;
									moduleNoticeLang(botnick, u, LANG_KARMA_UNDOLAST, na->nc->display, kd->karma > 0 ? "+" : "", kd->karma);
									moduleNoticeLang(botnick, u, LANG_HAS_KARMA, ke->nick, karma > 0 ? "+" : "", karma, ke->karma_p, abs(ke->karma_n));
								} else {
									char buf[512];
									memset(buf, 0, 512);
									format_period(buf, 512, kd->last_update + BSKarmaGiveInterval - time(NULL));
									moduleNoticeLang(botnick, u, LANG_KARMA_WAIT, na->nc->display, buf);
								}
							} else {
								kd = giveKarma(ke, u->na->nc, karma);
								karma = ke->karma_p + ke->karma_n;
								moduleNoticeLang(botnick, u, LANG_KARMA_UPDATED, na->nc->display, kd->karma > 0 ? "+" : "", kd->karma);
								moduleNoticeLang(botnick, u, LANG_HAS_KARMA, ke->nick, karma > 0 ? "+" : "", karma, ke->karma_p, abs(ke->karma_n));
							}
						} else {
							kd = giveKarma(ke, u->na->nc, karma);
							karma = ke->karma_p + ke->karma_n;
							moduleNoticeLang(botnick, u, LANG_KARMA_UPDATED, na->nc->display, kd->karma > 0 ? "+" : "", kd->karma);
							moduleNoticeLang(botnick, u, LANG_HAS_KARMA, ke->nick, karma > 0 ? "+" : "", karma, ke->karma_p, abs(ke->karma_n));
						}
					}
				}
			}
		}
	}

	return MOD_CONT;
}

void format_period(char *buf, int size, int time) {
	int days, hrs, mins;

	if (time >= 86400) {
		days = time / 86400;
		snprintf(buf + strlen(buf), size - strlen(buf), "%d%s", days, "d");
		time -= days * 86400;
	}

	if (time >= 3600) {
		hrs = time / 3600;
		snprintf(buf + strlen(buf), size - strlen(buf), "%d%s", hrs, "h");
		time -= hrs * 3600;
	}

	if (time >= 60) {
		mins = time / 60;
		snprintf(buf + strlen(buf), size - strlen(buf), "%d%s", mins, "m");
		time -= mins * 60;
	}

	if (time > 0)
		snprintf(buf + strlen(buf), size - strlen(buf), "%d%s", (int)time, "s");
}

/* ------------------------------------------------------------------------------- */

/**
 * Handles the fantasy commands for querying karma and turning it on/off.
 **/
int do_fantasy(int ac, char **av) {
	User *u;
	NickAlias *na;
	ChannelInfo *ci;
	Channel *c;
	int ret = MOD_CONT, karma;
	char *botnick = NULL;
	char *cmd = NULL, *param = NULL;
	ChanKarma *ck;
	KarmaEntry *ke;
	KarmaDetail *kd;

	/* Some basic error checking... should never match */
	if (ac < 3)
		return MOD_CONT;

	if (!(ci = cs_findchan(av[2])))
		return MOD_CONT;
	if (!(u = finduser(av[1])))
		return MOD_CONT;
	if (!(c = findchan(ci->name)))
		return MOD_CONT;

	if (ci->bi)
		botnick = ci->bi->nick;
	else
		botnick = s_ChanServ;

	if (supported < 0) {
		moduleNoticeLang(botnick, u, LANG_CMD_NOT_AVAILABLE);
		return MOD_CONT;
	}

	if (ac > 3) {
		cmd = myStrGetToken(av[3],' ',0);
		param = myStrGetToken(av[3],' ',1);
	}

	if (!stricmp(av[0], "karma")) {
		if ((ck = getChanKarma(ci->name))) {
			if (!cmd && !nick_identified(u))
				notice_lang(botnick, u, NICK_NOT_REGISTERED);
			else if (!cmd && (!(ke = getKarmaEntry(ck, u->na->nc))))
				privmsg(botnick, c->name, moduleGetLangString(u, LANG_HAS_NOKARMA), u->na->nc->display);
			else if (cmd && (!(na = findnick(cmd)) || !na->nc))
				notice_lang(botnick, u, NICK_X_NOT_REGISTERED, cmd);
			else if (cmd && (!(ke = getKarmaEntry(ck, na->nc))))
				privmsg(botnick, c->name, moduleGetLangString(u, LANG_HAS_NOKARMA), na->nc->display);
			else if (!param) {
				karma = ke->karma_p + ke->karma_n;
				privmsg(botnick, c->name, moduleGetLangString(u, LANG_HAS_KARMA), ke->nick, karma > 0 ? "+" : "", karma, ke->karma_p, abs(ke->karma_n));
			} else if (param && !stricmp(param, "ALL")) {
				if (!u->na || !u->na->nc || (stricmp(ke->nick, u->na->nc->display) && !check_access(u, ci, CA_AUTOVOICE)))
					notice_lang(botnick, u, PERMISSION_DENIED);
				else {
					karma = ke->karma_p + ke->karma_n;
					privmsg(botnick, c->name, moduleGetLangString(u, LANG_HAS_KARMA), ke->nick, karma > 0 ? "+" : "", karma, ke->karma_p, abs(ke->karma_n));

					if (ke->karma_p > 0) {
						char kp[420], buf[64];
						memset(kp, 0, 420);

						/* Build the list of users who contributed.. */
						for (kd = ke->pos; kd; kd = kd->next) {
							snprintf(buf, 64, "%s (+%d)", kd->nc->display, kd->karma);
							if (strlen(buf) + 6 < 420) {
								if (strlen(kp) > 0)
									strcat(kp, ", ");
								strcat(kp, buf);
							} else
								strcat(kp, ", ...");
						}
						privmsg(botnick, c->name, moduleGetLangString(u, LANG_KARMA_BY_POS), kp);
					}

					if (ke->karma_n < 0) {
						char kn[420], buf[64];
						memset(kn, 0, 420);

						/* Build the list of users who contributed.. */
						for (kd = ke->neg; kd; kd = kd->next) {
							snprintf(buf, 64, "%s (%d)", kd->nc->display, kd->karma);
							if (strlen(buf) + 6 < 420) {
								if (strlen(kn) > 0)
									strcat(kn, ", ");
								strcat(kn, buf);
							} else
								strcat(kn, ", ...");
						}
						privmsg(botnick, c->name, moduleGetLangString(u, LANG_KARMA_BY_NEG), kn);
					}
				}
			} else
				moduleNoticeLang(botnick, u, LANG_KARMA_SYNTAX, BSFantasyCharacter);
		}
	} else if (!stricmp(av[0], "set")) {
		if (ac == 3)
			moduleNoticeLang(botnick, u, LANG_SET_KARMA_SYNTAX, BSFantasyCharacter);
		else {
			if (!stricmp(cmd, "karma")) {
				if (!param)
					moduleNoticeLang(botnick, u, LANG_SET_KARMA_SYNTAX, BSFantasyCharacter);
				else if (!is_founder(u, ci) && !is_services_admin(u))
					notice_lang(botnick, u, PERMISSION_DENIED);
				else {
					if (!stricmp(param, "ON")) {
						if (!(ck = getChanKarma(ci->name)))
							ck = createChanKarma(ci->name);
						ck->enabled = 1;
						privmsg(botnick, c->name, moduleGetLangString(u, LANG_SET_KARMA_ON), ci->name);
					} else if (!stricmp(param, "OFF")) {
						delChanKarma(ci->name);
						privmsg(botnick, c->name, moduleGetLangString(u, LANG_SET_KARMA_OFF), ci->name);
					} else
						moduleNoticeLang(botnick, u, LANG_SET_KARMA_SYNTAX, BSFantasyCharacter);
				}
				ret = MOD_STOP;
			}
		}
	} else if (!stricmp(av[0], "minfo")) {
		show_modinfo(u, ci);
	} else if (!stricmp(av[0], "help")) {
		if (ac == 3) {
			moduleNoticeLang(botnick, u, LANG_HELP_HEADER);
			moduleNoticeLang(botnick, u, LANG_KARMA_DESC, BSFantasyCharacter);
		} else {
			if (!stricmp(cmd, "set")) {
				if (param && !stricmp(param, "karma")) {
					moduleNoticeLang(botnick, u, LANG_SET_KARMA_SYNTAX_EXT, BSFantasyCharacter);
					ret = MOD_STOP;
				} else {
					moduleNoticeLang(botnick, u, LANG_HELP_HEADER);
					moduleNoticeLang(botnick, u, LANG_SET_KARMA_DESC);
				}
			} else if (!stricmp(cmd, "karma")) {
				moduleNoticeLang(botnick, u, LANG_KARMA_SYNTAX_EXT, BSFantasyCharacter);
				ret = MOD_STOP;
			}
		}
	}

	if (cmd) free(cmd);
	if (param) free(param);
	return ret;
}


/**
 * Sends the user the information on this module.
 **/
static void show_modinfo(User *u, ChannelInfo *ci) {
	char *botnick = NULL;
	char *flags = get_flags();

	if (ci->bi)
		botnick = ci->bi->nick;
	else
		botnick = s_ChanServ;

	notice(botnick, u->nick, "Karma functionality provided by \002bs_fantasy_karma\002. [Author: \002%s\002] [Version: \002%s\002] [Flags: \002%s\002]",
			AUTHOR, VERSION, flags);

	free(flags);
}

/* ------------------------------------------------------------------------------- *
 * ----------------------- Functions for managing internal DB --------------------
 * ------------------------------------------------------------------------------- */

/*
 * Control functions for the karma data stored about channels (on/off; users)..
 */
 
ChanKarma *createChanKarma(char *chan) {
	ChanKarma *prev, *ptr, *ck;

	if (!chan)
		return NULL;

	ck = getChanKarma(chan);
	if (ck)
		return NULL;

	ck = scalloc(1, sizeof(ChanKarma));
	strscpy(ck->name, chan, CHANMAX);
	ck->count = 0;
	ck->enabled = 0;

	/* Go to the right position to insert.. */
	for (prev = NULL, ptr = channels[(unsigned char) tolower(chan[1])];
			ptr != NULL && stricmp(ptr->name, chan) < 0;
			prev = ptr, ptr = ptr->next);

	/* Insert.. */
	ck->prev = prev;
	ck->next = ptr;
	if (!prev)
		channels[(unsigned char) tolower(chan[1])] = ck;
	else
		prev->next = ck;
	if (ptr)
		ptr->prev = ck;

	return ck;
}

ChanKarma *getChanKarma(char *chan) {
	ChanKarma *ck;

	for (ck = channels[(unsigned char) tolower(chan[1])]; 
			ck != NULL && (stricmp(ck->name, chan));
			ck = ck->next);

	return ck;
}

void delChanKarma(char *chan) {
	ChanKarma *ck;

	if (!chan)
		return;

	ck = getChanKarma(chan);
	if (!ck)
		return;

	/* Remove record from the list.. */
	if (ck->next)
		ck->next->prev = ck->prev;
	if (ck->prev)
		ck->prev->next = ck->next;
	else
		channels[(unsigned char) tolower(chan[1])] = ck->next;

	/* No references remain, now free the record.. */
	freeChanKarma(ck);
}

void freeChanKarma(ChanKarma *ck) {
	clearKarmaEntries(ck);
	free(ck);
}

void clearKarmaEntries(ChanKarma *ck) {
	int i;
	KarmaEntry *ke, *next;

	for(i = 0; i < 1024; i++) {
		for (ke = ck->entries[i]; ke; ke = next) {
			next = ke->next;
			freeKarmaEntry(ke);
		}
		ck->entries[i] = NULL;
	}
	ck->count = 0;
}

/*
 * Control functions for the karma data stored about a specific user on a channel..
 */

/**
 * Creates and inserts a KarmaEntry for a user in the channels list.
 **/
KarmaEntry *createKarmaEntry(ChanKarma *ck, NickCore *nc) {
	KarmaEntry *ke;
	int index;

	if (!ck || !nc)
		return NULL;

	/* Make sure it doesn't already exist.. */
	ke = getKarmaEntry(ck, nc);
	if (ke)
		return NULL;

	ke = scalloc(1, sizeof(KarmaEntry));
	ke->ck = ck;
	ke->nick = sstrdup(nc->display);
	ke->karma_p = 0;
	ke->karma_n = 0;
	ke->pos = NULL;
	ke->pend = NULL;
	ke->neg = NULL;
	ke->nend = NULL;

	index = HASH(nc->display);
	ke->prev = NULL;
	ke->next = ck->entries[index];
	if (ke->next)
		ke->next->prev = ke;
	ck->entries[index] = ke;
	ck->count++;

	return ke;
}

KarmaEntry *getKarmaEntry(ChanKarma *ck, NickCore *nc) {
	KarmaEntry *ke;

	if (!ck || !nc)
		return NULL;

	for (ke = ck->entries[HASH(nc->display)]; ke; ke = ke->next) {
		if (!stricmp(ke->nick, nc->display))
			return ke;
	}

	return NULL;
}

KarmaEntry *getKarmaEntryByStr(ChanKarma *ck, char *nick) {
	KarmaEntry *ke;

	if (!ck || !nick)
		return NULL;

	for (ke = ck->entries[HASH(nick)]; ke; ke = ke->next) {
		if (!stricmp(ke->nick, nick))
			return ke;
	}

	return NULL;
}

void delKarma(ChanKarma *ck, NickCore *nc) {
	KarmaEntry *ke;

	if (!ck || !nc)
		return;

	ke = getKarmaEntry(ck, nc);
	if (!ke)
		return;

	delKarmaEntry(ke);
}

void delKarmaEntry(KarmaEntry *ke) {
	if (!ke)
		return;

	/* Remove us from the list.. */
	if (ke->next)
		ke->next->prev = ke->prev;
	if (ke->prev)
		ke->prev->next = ke->next;
	else
		ke->ck->entries[HASH(ke->nick)] = ke->next;
	ke->ck->count--;

	freeKarmaEntry(ke);
}

void freeKarmaEntry(KarmaEntry *ke) {
	KarmaDetail *kd, *next;

	for (kd = ke->pos; kd; kd = next) {
		next = kd->next;
		freeKarmaDetail(kd);
	}
	for (kd = ke->neg; kd; kd = next) {
		next = kd->next;
		freeKarmaDetail(kd);
	}

	/* Free memory.. */
	free(ke->nick);
	free(ke);
}


/*
 * Control functions for giving karma to a user...
 */
 
KarmaDetail *giveKarma(KarmaEntry *ke, NickCore *nc, int karma) {
	KarmaDetail *kd;

	if (!ke || !nc || karma == 0)
		return NULL;

	kd = addKarma(ke, nc, karma);
	kd->last_update = time(NULL);
	kd->last_karma = karma;

	return kd;
}

void undoLastKarma(KarmaDetail *kd) {
	int karma_old;

	if (!kd)
		return;

	karma_old = kd->karma;
	kd->karma -= kd->last_karma;
	updatePosKarmaDetail(kd, karma_old);

	kd->last_update = 0;
	kd->last_karma = 0;
}

KarmaDetail *addKarma(KarmaEntry *ke, NickCore *nc, int karma) {
	KarmaDetail *kd;
	int karma_old;

	if (!ke || !nc || karma == 0)
		return NULL;

	/* If it already exists, add the karma to the existing entry.. 
	 * If it doesn't exist, first create a new record.. */
	kd = getKarmaDetail(ke, nc);
	if (!kd)
		kd = createKarmaDetail(ke, nc);

	karma_old = kd->karma;
	kd->karma += karma;
	updatePosKarmaDetail(kd, karma_old);

	return kd;
}

KarmaDetail *createKarmaDetail(KarmaEntry *ke, NickCore *nc) {
	KarmaDetail *kd;
	NickCoreWrapper *ncw;
	KarmaDetailWrapper *kdw;

	if (!ke || !nc)
		return NULL;

	kd = scalloc(1, sizeof(KarmaDetail));
	kd->ke = ke;
	kd->nc = nc;
	kd->karma = 0;
	kd->last_update = 0;
	kd->last_karma = 0;

	/* Upon creation we append this to the end of the karma details list.. */
	kd->next = NULL;
	if (!ke->pos) {
		ke->pos = kd;
		ke->pend = kd;
	} else {
		ke->pend->next = kd;
		kd->prev = ke->pend;
		ke->pend = kd;
	}

	/* Now we need to add the karmadetail entry to the tracking list for the nickcore
	 * so we can easily delete all given karma when the core is dropped.. */
	ncw = getNickCoreWrapper(nc, 1);
	kdw = scalloc(1, sizeof(KarmaDetailWrapper));
	kdw->kd = kd;
	kdw->prev = NULL;
	kdw->next = ncw->kdw;
	if (kdw->next)
		kdw->next->prev = kdw;
	ncw->kdw = kdw;
	ncw->count++;
	kd->kdw = kdw;

	return kd;
}

void updatePosKarmaDetail(KarmaDetail *kd, int karma_old) {
	KarmaDetail *ptr;
	KarmaDetail **head, **tail, **head_old, **tail_old;
	int diff;

	if (!kd)
		return;

	/* Determine the list the record will end up in.. 
	 * Also store the old list in case we need to move from one list to another.. */
	if (kd->karma >= 0) {
		head = &kd->ke->pos;
		tail = &kd->ke->pend;
		head_old = &kd->ke->neg;
		tail_old = &kd->ke->nend;
	} else {
		head = &kd->ke->neg;
		tail = &kd->ke->nend;
		head_old = &kd->ke->pos;
		tail_old = &kd->ke->pend;
	}

	/* Check if we have to delete from the current list and add it to the other...
	 * This is a different logic followed.. */
	if ((kd->karma >= 0 && karma_old < 0) || (kd->karma < 0 && karma_old >= 0)) {
		/* Delete from the list.. (karma switched plus / minus sign) */
		if (kd->prev)
			kd->prev->next = kd->next;
		else if (*head_old == kd)
			*head_old = kd->next;
		if (kd->next)
			kd->next->prev = kd->prev;
		else if (*tail_old == kd)
			*tail_old = kd->prev;
		kd->prev = NULL;
		kd->next = NULL;

		if (*head && abs(kd->karma) <= abs((*head)->karma)) {
			/* We aren't the first record..
			 * Go over the list to find the right position to insert.. */
			for (ptr = *head; ptr->next; ptr = ptr->next)
				if (abs(kd->karma) <= abs(ptr->karma) && abs(kd->karma) > abs(ptr->next->karma))
					break;

			/* Insert AFTER the record ptr is pointing to.. */
			kd->prev = ptr;
			kd->next = ptr->next;
			kd->prev->next = kd;
			if (kd->next)
				kd->next->prev = kd;
			else
				*tail = kd;
		} else if (*head) {
			/* We are bigger then the current head.. replace it.. */
			kd->next = *head;
			kd->next->prev = kd;
			*head = kd;
		} else
			*head = *tail = kd;

		/* Update total karma count.. */
		if (karma_old >= 0) {
			kd->ke->karma_p -= karma_old;
			kd->ke->karma_n += kd->karma;
		} else {
			kd->ke->karma_n -= karma_old;
			kd->ke->karma_p += kd->karma;
		}
	} else {
		/* Check if we have to move it up the list.. */
		if (kd->prev && abs(kd->karma) > abs(kd->prev->karma)) {
			for (ptr = kd->prev; ptr->prev; ptr = ptr->prev)
				if (abs(kd->karma) > abs(ptr->karma) && abs(kd->karma) <= abs(ptr->prev->karma))
					break;

			kd->prev->next = kd->next;
			if (kd->next)
				kd->next->prev = kd->prev;
			else
				*tail = kd->prev;

			/* Insert BEFORE the record ptr is pointing to.. */
			kd->prev = ptr->prev;
			kd->next = ptr;
			if (kd->prev)
				kd->prev->next = kd;
			else
				*head = kd;

		/* Check if we have to move it down the list.. */
		} else if (kd->next && abs(kd->karma) < abs(kd->next->karma)) {
			for (ptr = kd->next; ptr->next; ptr = ptr->next)
				if (abs(kd->karma) <= abs(ptr->karma) && abs(kd->karma) > abs(ptr->next->karma))
					break;

			if (kd->prev)
				kd->prev->next = kd->next;
			else
				*head = kd->next;
			kd->next->prev = kd->prev;

			/* Insert AFTER the record ptr is pointing to.. */
			kd->prev = ptr;
			kd->next = ptr->next;
			kd->prev->next = kd;
			if (kd->next)
				kd->next->prev = kd;
			else
				*tail = kd;
		}

		/* In any case, update the total karma counters.. */
		diff = kd->karma - karma_old;
		if (kd->karma >= 0)
			kd->ke->karma_p += diff;
		else
			kd->ke->karma_n += diff;
	}
}

KarmaDetail *getKarmaDetail(KarmaEntry *ke, NickCore *nc) {
	KarmaDetail *kd;

	if (!ke || !nc)
		return NULL;

	for (kd = ke->pos; kd; kd = kd->next) {
		if (kd->nc == nc)
			return kd;
	}
	for (kd = ke->neg; kd; kd = kd->next) {
		if (kd->nc == nc)
			return kd;
	}

	return NULL;
}

/**
 * This function deletes a karma detail record:
 *    - The detail record itself is removed from the list for that item.
 *    - The total karma of the item is updated.
 *    - All reference to the karma detail is deleted from the user who gave it.
 **/
void delKarmaDetail(KarmaDetail *kd) {
	if (!kd)
		return;

	/* Remove us from the list.. */
	if (kd->prev)
		kd->prev->next = kd->next;
	else {
		if (kd->ke->pos == kd)
			kd->ke->pos = kd->next;
		else
			kd->ke->neg = kd->next;
	}
	if (kd->next)
		kd->next->prev = kd->prev;

	/* Adjust total karma.. */
	if (kd->karma >= 0)
		kd->ke->karma_p -= kd->karma;
	else
		kd->ke->karma_n -= kd->karma;

	freeKarmaDetail(kd);
}

void freeKarmaDetail(KarmaDetail *kd) {
	NickCoreWrapper *ncw;

	if (!kd)
		return;
	if (!(ncw = getNickCoreWrapper(kd->nc, 0)))
		return;

	/* Delete the tracking entry.. */
	if (kd->kdw->prev)
		kd->kdw->prev->next = kd->kdw->next;
	else
		ncw->kdw = kd->kdw->next;
	if (kd->kdw->next)
		kd->kdw->next->prev = kd->kdw->prev;

	/* Adjust counter tracking karma given by user.. */
	ncw->count--;

	/* Free memory.. */
	free(kd->kdw);
	free(kd);
}


/*
 * Control functions accessing the wrappers..
 */

NickCoreWrapper *createNickCoreWrapper(NickCore *nc) {
	NickCoreWrapper *ncw;

	if (!nc)
		return NULL;

	/* If it doesn't exist yet, silently create it.. */
	ncw = scalloc(1, sizeof(NickCoreWrapper));
	ncw->nc = nc;
	ncw->count = 0;
	ncw->kdw = NULL;
	ncw->prev = NULL;
	if (nickcores[HASH(nc->display)]) {
		ncw->next = nickcores[HASH(nc->display)];
		ncw->next->prev = ncw;
	} else
		ncw->next = NULL;
	nickcores[HASH(nc->display)] = ncw;

	return ncw;
}

NickCoreWrapper *getNickCoreWrapper(NickCore *nc, int create) {
	NickCoreWrapper *ncw;

	if (!nc)
		return NULL;

	for (ncw = nickcores[HASH(nc->display)]; ncw; ncw = ncw->next)
		if (ncw->nc == nc)
			return ncw;

	if (create)
		return createNickCoreWrapper(nc);

	return NULL;
}


NickCoreWrapper *getNickCoreWrapperByStr(char *display, int create) {
	NickCore *nc;
	NickCoreWrapper *ncw;

	if (!display)
		return NULL;

	for (ncw = nickcores[HASH(display)]; ncw; ncw = ncw->next)
		if (!stricmp(ncw->nc->display, display))
			return ncw;

	if (create && (nc = findcore(display)))
		return createNickCoreWrapper(nc);

	return NULL;
}


/**
 * Deletes a core wrapper and all data contained within it.
 * This includes deleting all karma given by the user.
 **/
void delNickCoreWrapper(NickCoreWrapper *ncw) {
	KarmaDetailWrapper *kdw, *next;

	if (!ncw)
		return;

	/* Remove us from the list.. */
	if (ncw->prev)
		ncw->prev->next = ncw->next;
	else
		nickcores[HASH(ncw->nc->display)] = ncw->next;
	if (ncw->next)
		ncw->next->prev = ncw->prev;

	/* Now delete all karma given.. */
	for (kdw = ncw->kdw; kdw; kdw = next) {
		next = kdw->next;

		/* Remove karma from the list.. */
		if (kdw->kd->prev)
			kdw->kd->prev->next = kdw->kd->next;
		else {
			if (kdw->kd->ke->pos == kdw->kd)
				kdw->kd->ke->pos = kdw->kd->next;
			else
				kdw->kd->ke->neg = kdw->kd->next;
		}
		if (kdw->kd->next)
			kdw->kd->next->prev = kdw->kd->prev;

		/* Adjust total karma.. */
		if (kdw->kd->karma >= 0)
			kdw->kd->ke->karma_p -= kdw->kd->karma;
		else
			kdw->kd->ke->karma_n -= kdw->kd->karma;

		/* Free memory.. */
		free(kdw->kd);
		free(kdw);
	}

	/* Free the wrapper itself.. */
	free(ncw);
}

void clear_db() {
	int i, j;
	ChanKarma *ck, *nck;
	KarmaEntry *ke, *nke;
	KarmaDetail *kd, *nkd;
	NickCoreWrapper *ncw, *nncw;
	KarmaDetailWrapper *kdw, *nkdw;

	for (i = 0; i < 256; i++) {
		for (ck = channels[i]; ck; ck = nck) {
			nck = ck->next;

			for (j = 0; j < 1024; j++) {
				for (ke = ck->entries[j]; ke; ke = nke) {
					nke = ke->next;

					for (kd = ke->pos; kd; kd = nkd) {
						nkd = kd->next;
						free(kd);
					}
					for (kd = ke->neg; kd; kd = nkd) {
						nkd = kd->next;
						free(kd);
					}

					free(ke->nick);
					free(ke);
				}
			}

			free(ck);
		}
		channels[i] = NULL;
	}

	for (i = 0; i < 1024; i++) {
		for (ncw = nickcores[i]; ncw; ncw = nncw) {
			nncw = ncw->next;

			for (kdw = ncw->kdw; kdw; kdw = nkdw) {
				nkdw = kdw->next;
				free(kdw);
			}

			free(ncw);
		}
		nickcores[i] = NULL;
	}
}

/* ------------------------------------------------------------------------------- *
 * ------- Functions for updating the internal DB on events send by the core -----
 * ------------------------------------------------------------------------------- */

/**
 * If anope drops a channel, delete all karma data for that channel.
 **/
int do_cs_drop(int argc, char **argv) {
	if (argc != 1)
		return MOD_CONT;

	delChanKarma(argv[0]);
	return MOD_CONT;
}


/**
 * When a NickCore is going to be dropped by the core, ensure we clear all data
 * relating to that core first. (karma entries for the user & karma given by the user)
 **/
int do_core_dropped(int argc, char **argv) {
	NickCore *nc = NULL;
	ChanKarma *ck = NULL;
	KarmaEntry *ke = NULL;
	NickCoreWrapper *ncw = NULL;
	int i;

	if (argc != 1)
		return MOD_CONT;
	if (!(nc = findcore(argv[0])))
		return MOD_CONT;

	/* Go over all channels and delete karma entries for the core.. */
	for (i = 0; i < 256; i++) {
		for (ck = channels[i]; ck; ck = ck->next) {
			for (ke = ck->entries[HASH(argv[0])]; ke; ke = ke->next) {
				if (!stricmp(ke->nick, argv[0])) {
					delKarmaEntry(ke);
					break;
				}
			}
		}
	}

	/* Delete all karma the user has given.. */
	if ((ncw = getNickCoreWrapper(nc, 0)))
		delNickCoreWrapper(ncw);

	return MOD_CONT;
}


/**
 * When a NickCore is given a new display nickname, ensure the modules internal DB
 * is updated to use the new display nick as key for all records regarding the core.
 **/
int do_core_newdisplay(int argc, char **argv) {
	ChanKarma *ck = NULL;
	KarmaEntry *ke = NULL;
	NickCoreWrapper *ncw = NULL;
	int i, index;

	if (argc != 2)
		return MOD_CONT;

	/* Move all the KarmaEntry in all channels.. */
	for (i = 0; i < 256; i++) {
		for (ck = channels[i]; ck; ck = ck->next) {
			if ((ke = getKarmaEntryByStr(ck, argv[0]))) {
				/* Delete from current list*/
				if (ke->next)
					ke->next->prev = ke->prev;
				if (ke->prev)
					ke->prev->next = ke->next;
				else
					ck->entries[HASH(ke->nick)] = ke->next;

				/* Insert in new location.. */
				index = HASH(argv[1]);
				ke->prev = NULL;
				ke->next = ck->entries[index];
				if (ke->next)
					ke->next->prev = ke;
				ck->entries[index] = ke;

				/* Update nick in KarmaEntry.. */
				free(ke->nick);
				ke->nick = sstrdup(argv[1]);
			}
		}
	}

	/* Move the NickCoreWrapper to the new location in the array.. */
	ncw = getNickCoreWrapperByStr(argv[0], 0);
	if (ncw) {
		/* Unlink it from its current position.. */
		if (ncw->next)
			ncw->next->prev = ncw->prev;
		if (ncw->prev)
			ncw->prev->next = ncw->next;
		else
			nickcores[HASH(argv[0])] = ncw->next;

		/* Insert at new position.. */
		index = HASH(argv[1]);
		ncw->prev = NULL;
		ncw->next = nickcores[index];
		if (ncw->next)
			ncw->next->prev = ncw;
		nickcores[index] = ncw;
	}

	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

void load_karma_db(void) {
	DBFile *dbptr = scalloc(1, sizeof(DBFile));
	char *key, *value, *nick, *karma, *last_update, *last_karma, *check;
	ChanKarma *ck = NULL;
	KarmaEntry *ke = NULL;
	KarmaDetail *kd = NULL;
	NickCore *nc = NULL;
	NickAlias *na = NULL;
	int retval, error = 1, pos, i = 0, k = 0, skip_r = 0, skip_c = 0;

	fill_db_ptr(dbptr, 0, KARMADBVERSION, s_BotServ, KarmaDB);

	/* let's remove existing temp files here, because we only load dbs on startup */
	remove(dbptr->temp_name);

	/* Open the db, fill the rest of dbptr and allocate memory for key and value */
	if (new_open_db_read(dbptr, &key, &value)) {
		free(dbptr);
		return;                 /* Bang, an error occurred */
	}

	while (1) {
		/* read a new entry and fill key and value with it - Certus */
		retval = new_read_db_entry(&key, &value, dbptr->fptr);

		if (retval == DB_READ_ERROR) {
			new_close_db(dbptr->fptr, &key, &value);
			free(dbptr);
			return;

		} else if (retval == DB_EOF_ERROR) {
			new_close_db(dbptr->fptr, &key, &value);
			free(dbptr);
			return;
		} else if (retval == DB_READ_BLOCKEND) {        /* DB_READ_BLOCKEND */
			if (skip_r)
				skip_r = 0;
			else if (skip_c)
				skip_c = 0;
			else if (ke) {
				ke = NULL;
			} else if (ck)
				ck = NULL;
			else
				alog("[\002bs_fantasy_karma\002] Warning: Encountered unexpected BLOCKEND in database!");
		} else {              /* DB_READ_SUCCESS */
			if (!*key || !*value || skip_r)
				continue;

			/* The K+/K- records are checked first followed by KE and CK* as these are likely more common and this will speed up parsing.. */
			if (!stricmp(key, "K+") || !stricmp(key, "K-")) {
				if (!ke) {
					alog("[\002bs_fantasy_karma\002] ERROR: Encountered unexpected '%s' entry in database!", key);
					alog("[\002bs_fantasy_karma\002] Aborting database loading...");
					break;
				}
				pos = !stricmp(key, "K+");
				nick = myStrGetToken(value,' ', 0);
				karma = myStrGetToken(value,' ', 1);
				last_update = myStrGetToken(value,' ', 2);
				last_karma = myStrGetToken(value,' ', 3);
				check = myStrGetToken(value,' ', 4);

				if (!nick || !karma || !last_update || !last_karma || !check) {
					alog("[\002bs_fantasy_karma\002] ERROR: Encountered malformed '%s' entry in database!", key);
					alog("[\002bs_fantasy_karma\002] Aborting database loading...");
					break;
				} else {
					k = atoi(karma);
					if (k > BSKarmaGiveMaxTotal) {
						alog("[\002bs_fantasy_karma\002] Warning: Karma attributed by %s to %s on %s exceeeds BSKarmaGiveMaxTotal (%d) and will be truncated!",
								value, ke->nick, ck->name, BSKarmaGiveMaxTotal);
						k = BSKarmaGiveMaxTotal;
					}
					if ((na = findnick(nick))) {
						kd = addKarma(ke, na->nc, k);
						kd->last_update = atoi(last_update);
						kd->last_karma = atoi(last_karma);

						if (!check_verif_code(ke, kd, atoi(check))) {
							alog("[\002bs_fantasy_karma\002] ERROR: Corrupt entry (%s: %s on %s) in database!", ke->nick, na->nc->display, ck->name);
							alog("[\002bs_fantasy_karma\002] Aborting database loading...");
							break;
						}
					} else if (debug)
						alog("[\002bs_fantasy_karma\002] Warning: Skipping karma attributed by unknown user '%s'!", value);
				}
				if (nick) free(nick);
				if (karma) free(karma);
				if (last_update) free(last_update);
				if (last_karma) free(last_karma);
				if (check) free(check);
			} else if (!stricmp(key, "KE")) {
				if (skip_c) {
					skip_r = 1;
					continue;
				}
				if (!ck) {
					alog("[\002bs_fantasy_karma\002] ERROR: Encountered unexpected 'KE' entry in database!");
					alog("[\002bs_fantasy_karma\002] Aborting database loading...");
					break;
				}

				if ((nc = findcore(value))) {
					ke = createKarmaEntry(ck, nc);
					i++;
				} else {
					if (debug)
						alog("[\002bs_fantasy_karma\002] Warning: Skipping karma entry for unknown user '%s'!", value);
					skip_r = 1;
				}
			} else if (!stricmp(key, "Kp")) {
				if (!ke) {
					alog("[\002bs_fantasy_karma\002] ERROR: Encountered unexpected 'Kp' entry in database!");
					alog("[\002bs_fantasy_karma\002] Aborting database loading...");
					break;
				}
				if (debug && ke->karma_p != atoi(value))
					alog("[\002bs_fantasy_karma\002] Warning: Positive karma attributed to %s (%d) does not match the expected karma (%s)!",
							ke->nick, ke->karma_p, value);
			} else if (!stricmp(key, "Kn")) {
				if (!ke) {
					alog("[\002bs_fantasy_karma\002] ERROR: Encountered unexpected 'Kn' entry in database!");
					alog("[\002bs_fantasy_karma\002] Aborting database loading...");
					break;
				}
				if (debug && ke->karma_n != atoi(value))
					alog("[\002bs_fantasy_karma\002] Warning: Negative karma attributed to %s (%d) does not match the expected karma (%s)!",
							ke->nick, ke->karma_n, value);
			} else if (!stricmp(key, "CK")) {
				if (ck) {
					alog("[\002bs_fantasy_karma\002] ERROR: CK: Channel Karma entry still open!");
					alog("[\002bs_fantasy_karma\002] Aborting database loading...");
					break;
				}
				if (cs_findchan(value))
					ck = createChanKarma(value);
				else {
					alog("[\002bs_fantasy_karma\002] Warning: Skipping karma entries for unknown channel '%s'!", value);
					skip_c = 1;
				}
			} else if (!stricmp(key, "CKe")) {
				if (skip_c)
					continue;
				if (!ck) {
					alog("[\002bs_fantasy_karma\002] ERROR: Encountered unexpected 'CKe' entry in database!");
					alog("[\002bs_fantasy_karma\002] Aborting database loading...");
					break;
				}
				ck->enabled = atoi(value);
			} else if (!stricmp(key, "CKc")) {
				if (skip_c)
					continue;
				if (!ck) {
					alog("[\002bs_fantasy_karma\002] ERROR: Encountered unexpected 'CKc' entry in database!");
					alog("[\002bs_fantasy_karma\002] Aborting database loading...");
					break;
				}
				if (debug && ck->count != atoi(value))
					alog("[\002bs_fantasy_karma\002] Warning: Total number of karma records for %s (%d) does not match the expected number (%s)!",
							ck->name, ck->count, value);
			} else if (!stricmp(key, "KARMA DB VERSION")) {
				if ((int)atoi(value) != KARMADBVERSION) {
					alog("[\002bs_fantasy_karma\002] ERROR: Database version does not match any versions supported by this module.");
					alog("[\002bs_fantasy_karma\002] Aborting database loading...");
					break;
				}
			} else if (!stricmp(key, "EOF")) {
				if ((int)atoi(value) != i) {
					alog("[\002bs_fantasy_karma\002] Warning: Not all karma records could be loaded.. (This could be due to expired nicks or channels!)");
				}
				error = 0;
				break;
			} else {
				alog("[\002bs_fantasy_karma\002] ERROR: Database element '%s' not recognized.", key);
				break;
			}
		} 					/* else */
	}					/* while */

	/* If we've encountered an error, clean the DB and start with a fresh one..*/
	if (error) {
		clear_db();
		alog("[\002bs_fantasy_karma\002] Starting with fresh database...");
	}

	free(dbptr);
}


void save_karma_db(void) {
	DBFile *dbptr = scalloc(1, sizeof(DBFile));
	ChanKarma *ck;
	int i, j, k = 0;

	if (!KarmaDB)
		return;

	fill_db_ptr(dbptr, 0, KARMADBVERSION, s_BotServ, KarmaDB);

	/* time to backup the old db */
	rename(KarmaDB, dbptr->temp_name);

	if (new_open_db_write(dbptr)) {
		rename(dbptr->temp_name, KarmaDB);
		free(dbptr);
		return;                /* Bang, an error occurred */
	}

	/* Store the version of the DB in the DB as well...
	 * This will make stuff a lot easier if the database scheme needs to modified. */
	new_write_db_entry("KARMA DB VERSION", dbptr, "%d", KARMADBVERSION);

	/* Go over all channels..*/
	for (i = 0; i < 256; i++) {
		for (ck = channels[i]; ck; ck = ck->next) {
			new_write_db_entry("CK", dbptr, "%s", ck->name);
			new_write_db_entry("CKe", dbptr, "%d", ck->enabled);
			j = save_karmas(dbptr, ck);
			new_write_db_entry("CKc", dbptr, "%d", j);
			new_write_db_endofblock(dbptr);
			if (j != ck->count)
				alog("[\002bs_fantasy_karma\002] Warning: Saved karma records for channel %s (%d) differs from expected count (%d) !", ck->name, j, ck->count);
			k += j;
		}
	}

	/* Write DB end tag.. */
	new_write_db_entry("EOF", dbptr, "%d", k);

	if (dbptr) {
		new_close_db(dbptr->fptr, NULL, NULL);  /* close file */
		remove(dbptr->temp_name);       /* saved successfully, no need to keep the old one */
		free(dbptr);           /* free the db struct */
	}
}

int save_karmas(DBFile *dbptr, ChanKarma *ck) {
	int i, j, k, l;
	KarmaEntry *ke, *ken;
	KarmaDetail *kd, *kdn;

	for (i = 0, j = 0; i < 1024; i++) {
		for (ke = ck->entries[i]; ke; ke = ken) {
			ken = ke->next;

			/* If this is a karma entry without anydetail records, purge it from the system.. */
			if (ke->karma_p == 0 && ke->karma_n == 0) {
				delKarmaEntry(ke);
				continue;
			}

			new_write_db_entry("KE", dbptr, "%s", ke->nick);
			for (kd = ke->pos, k = 0; kd; kd = kdn) {
				kdn = kd->next;

				/* Don't save details records with karma 0 and no active interval..
				 * Additionally, remove these from the list.. */
				if (kd->karma == 0 && (kd->last_update + BSKarmaGiveInterval < time(NULL))) {
					delKarmaDetail(kd);
					continue;
				}

				new_write_db_entry("K+", dbptr, "%s %d %d %d %d", kd->nc->display, kd->karma, kd->last_update, kd->last_karma, get_verif_code(ke, kd));
				k += kd->karma;
			}
			if (k != ke->karma_p)
				alog("[\002bs_fantasy_karma\002] Warning: Positive karma for %s in %s (%d) differs from expected value (%d) !", ke->nick, ke->ck->name, k, ke->karma_p);

			for (kd = ke->neg, l = 0; kd; kd = kd->next) {
				new_write_db_entry("K-", dbptr, "%s %d %d %d %d", kd->nc->display, kd->karma, kd->last_update, kd->last_karma, get_verif_code(ke, kd));
				l += kd->karma;
			}
			if (l != ke->karma_n)
				alog("[\002bs_fantasy_karma\002] Warning: Negative karma for %s in %s (%d) differs from expected value (%d) !", ke->nick, ke->ck->name, l, ke->karma_n);

			new_write_db_entry("Kp", dbptr, "%d", k);
			new_write_db_entry("Kn", dbptr, "%d", l);
			new_write_db_endofblock(dbptr);
			j++;
		}
	}

	return j;
}

int get_verif_code(KarmaEntry *ke, KarmaDetail *kd) {
	int i, res;
	NickAlias *na, *na2;

	na = findnick(ke->nick);
	na2 = findnick(kd->nc->display);
	if (kd->karma >= 0)
		i = kd->karma + 1;
	else
		i = kd->karma;

	res = (((na->time_registered >> 2) + (na2->time_registered >> 2)) ^ i) / i;
	res %= 10000;
	return res;
}

int check_verif_code(KarmaEntry *ke, KarmaDetail *kd, int code) {
	int i, res;
	NickAlias *na, *na2;

	na = findnick(ke->nick);
	na2 = findnick(kd->nc->display);
	if (kd->karma >= 0)
		i = kd->karma + 1;
	else
		i = kd->karma;

	res = (((na->time_registered >> 2) + (na2->time_registered >> 2)) ^ i) / i;
	res %= 10000;
	if (res == code)
		return 1;
	return 0;
}

/* ------------------------------------------------------------------------------- */

/**
 * When anope saves her databases, we do the same.
 **/
int do_save(int argc, char **argv) {
	if ((argc >= 1) && (!stricmp(argv[0], EVENT_STOP)))
		save_karma_db();

	return MOD_CONT;
}


/**
 * When anope backs her databases up, we do the same.
 **/
int db_backup(int argc, char **argv) {
	if ((argc >= 1) && (!stricmp(argv[0], EVENT_STOP))) {
		alog("[bs_fantasy_karma] Backing up karma database...");
		ModuleDatabaseBackup(KarmaDB);
	}
	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

/**
 * Checks whether the right conditions to continue loading are met.
 **/
int check_modules(void) {
#ifdef SUPPORTED
	if (supported >= 0) {
		if (findModule("os_raw")) {
			alog("[\002bs_fantasy_karma\002] Unsupported module found: os_raw.. (This is fatal!)");
			supported = -1;
		}
		if (findCommand(OPERSERV, "RAW")) {
			alog("[\002bs_fantasy_karma\002] Unsupported command found: RAW.. (This is fatal!)");
			supported = -1;
		}
		if (!DisableRaw) {
			alog("[\002bs_fantasy_karma\002] RAW has NOT been disabled! (This is fatal!)");
			supported = -1;
		}
	}

	if (supported >= 0) {
		if (findModule("ircd_init")) {
			alog("[\002bs_fantasy_karma\002] This module is unsupported in combination with ircd_init.");
			supported = 0;
		}

		if (findModule("cs_join")) {
			alog("[\002bs_fantasy_karma\002] This module is unsupported in combination with cs_join.");
			supported = 0;
		}

		if (findModule("bs_logchanmon")) {
			alog("[\002bs_fantasy_karma\002] This module is unsupported in combination with bs_logchanmon.");
			supported = 0;
		}

		if (findModule("ircd_gameserv")) {
			alog("[\002bs_fantasy_karma\002] This module is unsupported in combination with ircd_gameserv.");
			supported = 0;
		}

		if (findModule("os_psuedo_cont")) {
			alog("[\002bs_fantasy_karma\002] This module is unsupported in combination with os_psuedo_cont.");
			supported = 0;
		}
	}
#endif
	return supported;
}

/**
 * When a module is loaded, check whether it s compatible.
 **/
int event_check_module(int argc, char **argv) {
	int old_supported = supported;
#ifdef SUPPORTED
	if (argc != 1 || !argv[0])
		return MOD_CONT;

	if (supported >= 0) {
		if (!stricmp(argv[0], "os_raw")) {
			alog("[\002bs_fantasy_karma\002] Unsupported module found: %s.. (This is fatal!)", argv[0]);
			supported = -1;
		}
		if (!stricmp(argv[0], "ircd_init") || !stricmp(argv[0], "cs_join") || !stricmp(argv[0], "bs_logchanmon")
				|| !stricmp(argv[0], "ircd_gameserv") || !stricmp(argv[0], "os_psuedo_cont")) {
			alog("[\002bs_fantasy_karma\002] This module is unsupported in combination with %s.", argv[0]);
			supported = 0;
		}

		if (supported != old_supported) {
			if (supported == 0) {
				alog("[\002bs_fantasy_karma\002] Warning: Module continuing in unsupported mode!");
			} else if (supported == -1) {
				alog("[\002bs_fantasy_karma\002] Disabling module due to incompatibilities!");
				return MOD_STOP;
			}
			update_version();
		}
	}
#endif
	return MOD_CONT;
}

/**
 * When a command is created, check whether it s compatible.
 **/
int event_check_cmd(int argc, char **argv) {
	int old_supported = supported;
#ifdef SUPPORTED
	if (argc != 1 || !argv[0])
		return MOD_CONT;

	if (supported >= 0) {
		if (!stricmp(argv[0], "RAW")) {
			alog("[\002bs_fantasy_karma\002] Unsupported command found: %s.. (This is fatal!)", argv[0]);
			supported = -1;
		}

		if (supported != old_supported) {
			if (supported == 0) {
				alog("[\002bs_fantasy_karma\002] Warning: Module continuing in unsupported mode!");
			} else if (supported == -1) {
				alog("[\002bs_fantasy_karma\002] Disabling module due to incompatibilities!");
				return MOD_STOP;
			}
			update_version();
		}
	}
#endif
	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

/**
 * Returns pointer to a string of flags.. Must be free'd!!
 **/
char *get_flags() {
	char tmp[BUFSIZE];
	const char version_flags[] = " " VER_DEBUG VER_OS VER_MYSQL VER_MODULE;
	char *flags;

#ifdef SUPPORTED
	snprintf(tmp, BUFSIZE, "%s-%s-I=%ds-M=%d", version_flags, ((supported == -1) ? "U" : 
			(supported == 0) ? "u" : "S"), BSKarmaGiveInterval, BSKarmaGiveMaxTotal);
#else
	snprintf(tmp, BUFSIZE, "%s-HU", version_flags);
#endif
	flags = sstrdup(tmp);
	return flags;
}

/**
 * Updates the version info shown in modlist and modinfo.
 **/
void update_version(void) {
	Module *m;
	char tmp[BUFSIZE];
	char *flags = get_flags();

	if (mod_current_module)
		m = mod_current_module;
	else
		m = findModule("bs_fantasy_karma");

	snprintf(tmp, BUFSIZE, "%s %s [%s]", AUTHOR, VERSION, flags);

	if (m->version)
		free(m->version);
	m->version = sstrdup(tmp);
	free(flags);
}

/* ------------------------------------------------------------------------------- */

/**
 * (Re)Load the configuration directives
 **/
void load_config(void) {
	int i;

	Directive confvalues[][1] = {
		{{"BSKarmaDB", {{PARAM_STRING, PARAM_RELOAD, &KarmaDB}}}},
		{{"BSKarmaGiveInterval", {{PARAM_TIME, PARAM_RELOAD, &BSKarmaGiveInterval}}}},
		{{"BSKarmaGiveMaxTotal", {{PARAM_POSINT, PARAM_RELOAD, &BSKarmaGiveMaxTotal}}}},
	};

	if (KarmaDB)
		free(KarmaDB);
	KarmaDB = NULL;
	BSKarmaGiveInterval = 0;
	BSKarmaGiveMaxTotal = 1;

	for (i = 0; i < 3; i++)
		moduleGetConfigDirective(confvalues[i]);

	if (!KarmaDB)
		KarmaDB = sstrdup(DefKarmaDB);
	if (!BSKarmaGiveMaxTotal)
		BSKarmaGiveMaxTotal = 1;

	if (debug)
		alog ("[bs_fantasy_karma] debug: KarmaDB set to '%s'; BSKarmaGiveInterval set to '%d's; BSKarmaGiveMax set to %d.", KarmaDB, BSKarmaGiveInterval, BSKarmaGiveMaxTotal);
}


/**
 * Upon /os reload call the routines for reloading the configuration directives
 **/
int reload_config(int argc, char **argv) {
	int old_supported = supported;

	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			if (debug)
				alog("[bs_fantasy_karma] debug: Reloading configuration directives...");
			load_config();
		}
	}

	check_modules();
	if (supported != old_supported) {
		if (supported == 0) {
			alog("[\002bs_fantasy_karma\002] Warning: Module continuing in unsupported mode!");
		} else if (supported == -1) {
			alog("[\002bs_fantasy_karma\002] Disabling module due to incompatibilities!");
			return MOD_STOP;
		}
	}
	update_version();

	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

/**
 * Add language strings to the module's language db.
 **/
void add_languages(void) {
	char *langtable_en_us[] = {
		/* LANG_HELP_HEADER */
		" ----- Karma Commands:",
		/* LANG_KARMA_DESC */
		" %skarma - Query the karma attributed to a user.",
		/* LANG_KARMA_SYNTAX */
		" Syntax: %skarma [[nick] [ALL]]",
		/* LANG_KARMA_SYNTAX_EXT */
		" Syntax: %skarma [[nick] [ALL]]\n"
		" \n"
		" Query the given users karma. If no nickname is given, the own karma is displayed.\n"
		" When given the ALL parameter, a list of the users who contributed to the karma\n"
		" is shown. Note that this parameter is restricted to users looking up their own karma,\n"
		" or channel voices and above.\n"
		" \n"
		" Karma can be given with the commands nick++ and nick--.",
		/* LANG_SET_KARMA_DESC */
		" karma - Toggle the karma functionality on or off.",
		/* LANG_SET_KARMA_SYNTAX */
		" Syntax: %sset karma { ON | OFF }",
		/* LANG_SET_KARMA_SYNTAX_EXT */
		" Syntax: %sset karma { ON | OFF }\n"
		" \n"
		" Toggle the BotServ karma feature on or off for the current channel.\n"
		" Note that turning off the karma feature will purge all karma currently\n"
		" given in this channel.\n"
		" \n"
		" This command is restricted to the channel founder.",
		/* LANG_CMD_NOT_AVAILABLE */
		" This command is currently not available.",
		/* LANG_SET_KARMA_ON */
		" The karma system has been enabled in %s.",
		/* LANG_SET_KARMA_OFF */
		" The karma system has been disabled in %s. All karma data has been purged.",
		/* LANG_HAS_NOKARMA */
		" User %s has no karma on this channel.",
		/* LANG_HAS_KARMA */
		" User %s has a combined karma of %s%d. (%d Positive / %d Negative)",
		/* LANG_KARMA_BY_POS */
		" Gave positive karma: %s",
		/* LANG_KARMA_BY_NEG */
		" Gave negative karma: %s",
		/* LANG_KARMA_UPDATED */
		" Karma given to %s has been updated. You now contribute %s%d.",
		/* LANG_KARMA_MAX */
		" You have already given the maximum karma (%s%d) to user %s on this channel.",
		/* LANG_KARMA_NO_SELF */
		" You cannot give karma to yourself.",
		/* LANG_KARMA_UNDOLAST */
		" The karma last given to %s has been revoked. You now contribute %s%d.",
		/* LANG_KARMA_WAIT */
		" You have recently given karma to %s and must wait for another %s."
	};

	char *langtable_nl[] = {
		/* LANG_HELP_HEADER */
		" ----- Karma Commando's:",
		/* LANG_KARMA_DESC */
		" %skarma - Geef het karma van een gebruiker weer.",
		/* LANG_KARMA_SYNTAX */
		" Syntax: %skarma [[nicknaam] [ALL]]",
		/* LANG_KARMA_SYNTAX_EXT */
		" Syntax: %skarma [[nicknaam] [ALL]]\n"
		" \n"
		" Geeft het kazma van de opgegeven gebruiker weer. Als geen nicknaam is gespecificeerd, \n"
		" wordt het karma van de gebruiker zelf weergegeven.\n"
		" Als de parameter ALL is opgegeven, wordt een lijst weergegeven van wie karma heeft gegeven.\n"
		" Let dat toegang tot deze parameter is beperkt tot gebruikers die hun eigen karma opvragen,\n"
		" of tot personen met voice of hoger op het kanaal.\n"
		" \n"
		" Karma kan gegeven worden met de commando's nick++ en nick--.",
		/* LANG_SET_KARMA_DESC */
		" karma - Schakel de karma functionaliteit aan of uit.",
		/* LANG_SET_KARMA_SYNTAX */
		" Syntax: %sset karma { ON | OFF }",
		/* LANG_SET_KARMA_SYNTAX_EXT */
		" Syntax: %sset karma { ON | OFF }\n"
		" \n"
		" Schakel de BotServ karma functionaliteit aan of uit voor dit kanaal.\n"
		" Let dat het uitschakelen van de karma functionaliteit alle karma gevegens in dit kanaal zal verwijderen!\n"
		" \n"
		" Dit commando is enkel toegankelijk voor de eigenaar(s) van het kanaal.",
		/* LANG_CMD_NOT_AVAILABLE */
		" Dit commando is momenteel niet beschikbaar.",
		/* LANG_SET_KARMA_ON */
		" De karma functionaliteit is geactiveerd in %s.",
		/* LANG_SET_KARMA_OFF */
		" De karma functionaliteit is uitgeschakeld %s. Alle karma gegevens zijn verwijderd.",
		/* LANG_HAS_NOKARMA */
		" De gebruiker %s heeft geen karma in dit kanal.",
		/* LANG_HAS_KARMA */
		" De gebruiker %s heeft een totaal karma van %s%d. (%d Positief / %d Negatief)",
		/* LANG_KARMA_BY_POS */
		" Gaven positief karma: %s",
		/* LANG_KARMA_BY_NEG */
		" Gaven negatief karma: %s",
		/* LANG_KARMA_UPDATED */
		" Het karma gegeven aan %s werd bijgewerkt. U geeft nu %s%d.",
		/* LANG_KARMA_MAX */
		" U hebt reeds het maximum karma (%s%d) aan %s gegeven op dit kanaal.",
		/* LANG_KARMA_NO_SELF */
		" Je kan geen karma aan jezelf geven..",
		/* LANG_KARMA_UNDOLAST */
		" The karma last given to %s has been revoked. You now contribute %s%d.",
		/* LANG_KARMA_WAIT */
		" Je hebt recent karma gegeven aan %s en moet nog %s wachten vooraleer je opnieuw karma kan geven."
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
	moduleInsertLanguage(LANG_NL, LANG_NUM_STRINGS, langtable_nl);
}

/* ------------------------------------------------------------------------------- */

/**************************************************************************
 *         Generic DataBase Functions  (Taken from swhois by Trystan)
 **************************************************************************/

int new_open_db_read(DBFile *dbptr, char **key, char **value) {
	*key = malloc(MAXKEYLEN);
	*value = malloc(MAXVALLEN);

	if (!(dbptr->fptr = fopen(dbptr->filename, "rb"))) {
		if (debug) {
			alog("debug: Can't read %s database %s : errno(%d)", dbptr->service,
				dbptr->filename, errno);
		}
		free(*key);
		*key = NULL;
		free(*value);
		*value = NULL;
		return DB_READ_ERROR;
	}
	dbptr->db_version = fgetc(dbptr->fptr) << 24 | fgetc(dbptr->fptr) << 16
		| fgetc(dbptr->fptr) << 8 | fgetc(dbptr->fptr);

	if (ferror(dbptr->fptr)) {
		if (debug) {
			alog("debug: Error reading version number on %s", dbptr->filename);
		}
		free(*key);
		*key = NULL;
		free(*value);
		*value = NULL;
		return DB_READ_ERROR;
	} else if (feof(dbptr->fptr)) {
		if (debug) {
			alog("debug: Error reading version number on %s: End of file detected",
				dbptr->filename);
		}
		free(*key);
		*key = NULL;
		free(*value);
		*value = NULL;
		return DB_EOF_ERROR;
	} else if (dbptr->db_version < 1) {
		if (debug) {
			alog("debug: Invalid version number (%d) on %s", dbptr->db_version, dbptr->filename);
		}
		free(*key);
		*key = NULL;
		free(*value);
		*value = NULL;
		return DB_VERSION_ERROR;
	}
	return DB_READ_SUCCESS;
}


int new_open_db_write(DBFile *dbptr) {
	if (!(dbptr->fptr = fopen(dbptr->filename, "wb"))) {
		if (debug) {
			alog("debug: %s Can't open %s database for writing", dbptr->service, dbptr->filename);
		}
		return DB_WRITE_ERROR;
	}

	if (fputc(dbptr->core_db_version >> 24 & 0xFF, dbptr->fptr) < 0 ||
		fputc(dbptr->core_db_version >> 16 & 0xFF, dbptr->fptr) < 0 ||
		fputc(dbptr->core_db_version >> 8 & 0xFF, dbptr->fptr) < 0 ||
		fputc(dbptr->core_db_version & 0xFF, dbptr->fptr) < 0) {
		if (debug) {
			alog("debug: Error writing version number on %s", dbptr->filename);
		}
		return DB_WRITE_ERROR;
	}
	return DB_WRITE_SUCCESS;
}


void new_close_db(FILE *fptr, char **key, char **value) {
	if (key && *key) {
		free(*key);
		*key = NULL;
	}
	if (value && *value) {
		free(*value);
		*value = NULL;
	}

	if (fptr) {
		fclose(fptr);
	}
}


int new_read_db_entry(char **key, char **value, FILE *fptr) {
	char *string = *key;
	int character;
	int i = 0;

	**key = '\0';
	**value = '\0';

	while (1) {
		if ((character = fgetc(fptr)) == EOF) { /* a problem occurred reading the file */
			if (ferror(fptr)) {
				return DB_READ_ERROR;   /* error! */
			}
			return DB_EOF_ERROR;        /* end of file */
		} else if (character == BLOCKEND) {     /* END OF BLOCK */
			return DB_READ_BLOCKEND;
		} else if (character == VALUEEND) {     /* END OF VALUE */
			string[i] = '\0';   /* end of value */
			return DB_READ_SUCCESS;
		} else if (character == SEPARATOR) {    /* END OF KEY */
			string[i] = '\0';   /* end of key */
			string = *value;    /* beginning of value */
			i = 0;              /* start with the first character of our value */
		} else {
			if ((i == (MAXKEYLEN - 1)) && (string == *key)) {   /* max key length reached, continuing with value */
				string[i] = '\0';       /* end of key */
				string = *value;        /* beginning of value */
				i = 0;          /* start with the first character of our value */
			} else if ((i == (MAXVALLEN - 1)) && (string == *value)) {  /* max value length reached, returning */
				string[i] = '\0';
				return DB_READ_SUCCESS;
			} else {
				string[i] = character;  /* read string (key or value) */
				i++;
			}
		}
	}
}


int new_write_db_entry(const char *key, DBFile *dbptr, const char *fmt, ...) {
	char string[MAXKEYLEN + MAXVALLEN + 2], value[MAXVALLEN];   /* safety byte :P */
	va_list ap;
	unsigned int length;

	if (!dbptr) {
		return DB_WRITE_ERROR;
	}

	va_start(ap, fmt);
	vsnprintf(value, MAXVALLEN, fmt, ap);
	va_end(ap);

	if (!stricmp(value, "(null)")) {
		return DB_WRITE_NOVAL;
	}
	snprintf(string, MAXKEYLEN + MAXVALLEN + 1, "%s%c%s", key, SEPARATOR, value);
	length = strlen(string);
	string[length] = VALUEEND;
	length++;

	if (fwrite(string, 1, length, dbptr->fptr) < length) {
		if (debug) {
			alog("debug: Error writing to %s", dbptr->filename);
		}
		new_close_db(dbptr->fptr, NULL, NULL);
		if (debug) {
			alog("debug: Restoring backup.");
		}
		remove(dbptr->filename);
		rename(dbptr->temp_name, dbptr->filename);
		free(dbptr);
		dbptr = NULL;
		return DB_WRITE_ERROR;
	}
	return DB_WRITE_SUCCESS;
}


int new_write_db_endofblock(DBFile *dbptr) {
	if (!dbptr) {
		return DB_WRITE_ERROR;
	}
	if (fputc(BLOCKEND, dbptr->fptr) == EOF) {
		if (debug) {
			alog("debug: Error writing to %s", dbptr->filename);
		}
		new_close_db(dbptr->fptr, NULL, NULL);
		return DB_WRITE_ERROR;
	}
	return DB_WRITE_SUCCESS;
}


void fill_db_ptr(DBFile *dbptr, int version, int core_version,
				char service[256], char filename[256]) {
	dbptr->db_version = version;
	dbptr->core_db_version = core_version;
	if (!service)
		strcpy(dbptr->service, service);
	else
		strcpy(dbptr->service, "");

	strcpy(dbptr->filename, filename);
	snprintf(dbptr->temp_name, 261, "%s.temp", filename);
	return;
}

/* ------------------------------------------------------------------------------- */

/* EOF */
