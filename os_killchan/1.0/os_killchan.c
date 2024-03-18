/**
 * -----------------------------------------------------------------------------
 * Name    : os_killchan
 * Author  : Viper <Viper@Anope.org>
 * Date    : 02/12/2011 (Last update: 24/12/2011)
 * Version : 1.0
 * -----------------------------------------------------------------------------
 * Requires    : Anope-1.8.7
 * Tested      : Anope 1.8.7 + UnrealIRCd 3.2.8
 * -----------------------------------------------------------------------------
 * This module allows Services Administrators to define "kill channels" which
 * when joined by a normal user will result in anope akilling said user.
 *
 * The main reason for writing this module is the passing of IRC Defender which
 * I until now have always used to fullfil this function.
 *
 * This module is released under the GPL 2 license.
 * -----------------------------------------------------------------------------
 *
 * Changelog:
 *
 *   1.0  -  Initial Release
 *
 * -----------------------------------------------------------------------------
 * TODO:
 *
 *   << hope there are no more bugs.. >>
 *
 **/


/**
 * Configuration directives that should be copy-pasted to services.conf

# KillChanDB [OPTIONAL]
# Module: os_killchan
#
# Use the given filename as database to store the KILLCHANs.
# If not given, the default of "os_killchan.db" will be used.
#
#KillChanDB "os_killchan.db"

# KillChanRegistered [OPTIONAL]
# Module: os_killchan
#
# Defines whether registered and identified users joining a killchan
# should be akilled. If defined registered users will be akilled, 
# if commented out only unregistered users will be killed.
#
# Note that opers and clients on ulined servers are always exempt.
#
#KillChanRegistered

# KillChanAKillTime [OPTIONAL]
# Module: os_killchan
#
# This determines how long users will be Akilled.
# Please note this is subject to ExpireTimeout.
# If this is not specified a default value of 1 day will be used instead.
#
#KillChanAKillTime 1d

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
#define KILLCHANDBVERSION 1

/* Language defines */
#define LANG_NUM_STRINGS 					22

#define LANG_KILLCHAN_DESC					0
#define LANG_KILLCHAN_SYNTAX				1
#define LANG_KILLCHAN_SYNTAX_EXT			2
#define LANG_KILLCHAN_DISABLED				3
#define LANG_NO_LOCAL_CHAN					4
#define LANG_CHAN_SYMB_REQUIRED				5
#define CHAN_X_REGISTERED					6
#define LANG_KILLCHAN_ADDED					7
#define LANG_KILLCHAN_UPDATED				8
#define LANG_KILLCHAN_ENFORCED				9
#define LANG_KILLCHAN_NO_SUCH_ENTRY			10
#define LANG_KILLCHAN_DELETED				11
#define LANG_KILLCHAN_LIST_EMPTY			12
#define LANG_KILLCHAN_ENTRY					13
#define LANG_KILLCHAN_ENTRIES				14
#define LANG_KILLCHAN_NAME					15
#define LANG_KILLCHAN_REASON				16
#define LANG_KILLCHAN_NO_REASON				17
#define LANG_KILLCHAN_LIST_CLEARED			18
#define LANG_UNKWN_KILLCHAN_OPTION			19
#define LANG_KILLED							20
#define LANG_KILLED_NO_REASON				21


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
#define MAXKEYLEN 64
#define MAXVALLEN 256

/* Create a simple hash of the trigger to begin lookup.. */
#define HASH(chan)				((tolower((chan)[0])&31)<<5 | (tolower((chan)[1])&31))


/* Structs */
typedef struct db_file_ DBFile;
typedef struct killchan_ KillChan;

struct db_file_ {
	FILE *fptr;             /* Pointer to the opened file */
	int db_version;         /* The db version of the datafiles (only needed for reading) */
	int core_db_version;    /* The current db version of this anope source */
	char service[256];      /* StatServ/etc. */
	char filename[256];     /* Filename of the database */
	char temp_name[262];    /* Temp filename of the database */
};

struct killchan_ {
	KillChan *prev, *next;
	char name[CHANMAX];
	char *reason;
	int kills;
};


/* Constants */
char *DefKillChanDB = "os_killchans.db";



/* Variables */
int supported;
char *KillChanDB;
int KillChanRegistered, KillChanAKillTime;

KillChan *channels[256];


/* Functions */
void do_help_list(User *u);
int do_help(User *u);
int do_killchan(User *c);
void enforce_kill(KillChan *kc);
void enforce_kill_all();

int check_join(int argc, char **argv);

KillChan *createKillChan(char *chan, char *reason);
KillChan *getKillChan(char *chan);
void delKillChan(KillChan *kc);
void freeKillChan(KillChan *kc);
void clear_db();

void load_killchan_db(void);
void save_killchan_db(void);
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
 * Create the command, and tell anope about it.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv) {
	Command *c;
	EvtHook *hook;

	alog("[\002os_killchan\002] Loading module...");

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	supported = 1;

	if (!moduleMinVersion(1, 8, 7, 3089)) {
		alog("[\002os_killchan\002] Your version of Anope isn't supported. Please update to a newer release.");
		return MOD_STOP;
	}

	check_modules();
	if (supported == 0) {
		alog("[\002os_killchan\002] Warning: Module continuing in unsupported mode!");
	} else if (supported == -1) {
		alog("[\002os_killchan\002] Unloading module due to incompatibilities!");
		return MOD_STOP;
	}

	/* Create KILLCHAN command.. */
	c = createCommand("KILLCHAN", do_killchan, is_services_admin, -1, -1, -1, -1, -1);
	if (moduleAddCommand(OPERSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002os_killchan\002] Cannot create KILLCHAN command...");
		return MOD_STOP;
	}
	moduleAddHelp(c, do_help);
	moduleSetOperHelp(do_help_list);

	hook = createEventHook(EVENT_JOIN_CHANNEL, check_join);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002os_killchan\002] Can't hook to EVENT_JOIN_CHANNEL event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_RELOAD, reload_config);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002os_killchan\002] Can't hook to EVENT_RELOAD event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_DB_SAVING, do_save);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002os_killchan\002] Can't hook to EVENT_DB_SAVING event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_DB_BACKUP, db_backup);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002os_killchan\002] Can't hook to EVENT_DB_BACKUP event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_MODLOAD, event_check_module);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002os_killchan\002] Can't hook to EVENT_MODLOAD event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_ADDCOMMAND, event_check_cmd);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002os_killchan\002] Can't hook to EVENT_ADDCOMMAND event.");
		return MOD_STOP;
	}

	load_config();
	add_languages();
	load_killchan_db();
	enforce_kill_all();

	/* Update version info.. */
	update_version();

	alog("[\002os_killchan\002] Module loaded successfully...");
	return MOD_CONT;
}

/**
 * Unload the module
 **/
void AnopeFini(void) {
	/* Save the database.. */
	save_killchan_db();

	/* Clear the memory.... */
	clear_db();
	free(KillChanDB);

	alog("[\002os_killchan\002] Unloading module...");
}

/* ------------------------------------------------------------------------------- */

/**
 * Add the KILLCHAN command to the OperServ HELP listing.
 **/
void do_help_list(User *u) {
	if (is_services_admin(u))
		moduleNoticeLang(s_OperServ, u, LANG_KILLCHAN_DESC);
}


/**
 * Show the extended help on the KILLCHAN command.
 **/
int do_help(User *u) {
	if (is_services_admin(u))
		moduleNoticeLang(s_OperServ, u, LANG_KILLCHAN_SYNTAX_EXT);
	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

int do_killchan(User *u) {
	KillChan *kc;
	ChannelInfo *ci;
	char *buffer, *cmd, *chan, *reason;

	buffer = moduleGetLastBuffer();
	cmd = myStrGetToken(buffer, ' ', 0);
	chan = myStrGetToken(buffer, ' ', 1);
	reason = myStrGetTokenRemainder(buffer, ' ', 2);

	if (!cmd)
		moduleNoticeLang(s_OperServ, u, LANG_KILLCHAN_SYNTAX);

	else if (!stricmp(cmd, "ADD")) {
		if (readonly)
			moduleNoticeLang(s_OperServ, u, LANG_KILLCHAN_DISABLED);
		else if (!chan)
			moduleNoticeLang(s_OperServ, u, LANG_KILLCHAN_SYNTAX);
		else if (*chan == '&')
			moduleNoticeLang(s_OperServ, u, LANG_NO_LOCAL_CHAN);
		else if (*chan != '#')
			moduleNoticeLang(s_OperServ, u, LANG_CHAN_SYMB_REQUIRED);
		else if (!anope_valid_chan(chan))
			notice_lang(s_OperServ, u, CHAN_X_INVALID, chan);
		else if ((ci = cs_findchan(chan)) && !(ci->flags & CI_VERBOTEN) && !(ci->flags & CI_SUSPENDED))
			moduleNoticeLang(s_OperServ, u, CHAN_X_REGISTERED, chan);
		else if (ci && ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED)))
			notice_lang(s_OperServ, u, CHAN_X_FORBIDDEN, chan);
		else {
			/* Add or update channel in the list.. */
			if ((kc = getKillChan(chan))) {
				if (kc->reason)
					free(kc->reason);
				if (reason)
					kc->reason = sstrdup(reason);
				else
					kc->reason = NULL;
				moduleNoticeLang(s_OperServ, u, LANG_KILLCHAN_UPDATED, chan);
				alog("[os_killchan] %s updated the KillChan record for %s (%s).",
						u->nick, chan, reason ? reason : "");
			} else {
				kc = createKillChan(chan, reason);
				moduleNoticeLang(s_OperServ, u, LANG_KILLCHAN_ADDED, chan);
				anope_cmd_global(s_OperServ, "%s added %s to the KillChan list. (%s)",
						u->nick, chan, reason ? reason : "");
				alog("[os_killchan] %s added %s to the KillChan list. (%s)",
						u->nick, chan, reason ? reason : "");

				/* Enforce the killchan.. */
				enforce_kill(kc);
				if (kc->kills)
					moduleNoticeLang(s_OperServ, u, LANG_KILLCHAN_ENFORCED, chan);
			}
		}
	} else if (!stricmp(cmd, "DEL")) {
		if (readonly)
			moduleNoticeLang(s_OperServ, u, LANG_KILLCHAN_DISABLED);
		else if (!chan)
			moduleNoticeLang(s_OperServ, u, LANG_KILLCHAN_SYNTAX);
		else if (!(kc = getKillChan(chan)))
			moduleNoticeLang(s_OperServ, u, LANG_KILLCHAN_NO_SUCH_ENTRY, chan);
		else {
			delKillChan(kc);
			moduleNoticeLang(s_OperServ, u, LANG_KILLCHAN_DELETED, chan);
			anope_cmd_global(s_OperServ, "%s deleted %s from the KillChan list.",
					u->nick, chan);
			alog("[os_killchan] %s deleted %s from the KillChan list.", u->nick, chan);
		}
	} else if (!stricmp(cmd, "LIST")) {
		int i, counter = 0;

		for (i = 0; i < 256; i++) {
			for (kc = channels[i]; kc; kc = kc->next) {
				moduleNoticeLang(s_OperServ, u, LANG_KILLCHAN_ENTRY, kc->name, kc->kills,
						kc->reason ? kc->reason : "");
				counter++;
			}
			i++;
		}

		if (counter == 0)
			moduleNoticeLang(s_OperServ, u, LANG_KILLCHAN_LIST_EMPTY);
		else
			moduleNoticeLang(s_OperServ, u, LANG_KILLCHAN_ENTRIES, counter);
	} else if (!stricmp(cmd, "VIEW")) {
		if (!chan)
			moduleNoticeLang(s_OperServ, u, LANG_KILLCHAN_SYNTAX);
		else if (!(kc = getKillChan(chan)))
			moduleNoticeLang(s_OperServ, u, LANG_KILLCHAN_NO_SUCH_ENTRY, chan);
		else {
			moduleNoticeLang(s_OperServ, u, LANG_KILLCHAN_NAME, kc->name, kc->kills);
			if (kc->reason)
				moduleNoticeLang(s_OperServ, u, LANG_KILLCHAN_REASON, kc->reason);
			else
				moduleNoticeLang(s_OperServ, u, LANG_KILLCHAN_NO_REASON);
		}
	} else if (!stricmp(cmd, "CLEAR")) {
		if (readonly)
			moduleNoticeLang(s_OperServ, u, LANG_KILLCHAN_DISABLED);
		else {
			clear_db();
			moduleNoticeLang(s_OperServ, u, LANG_KILLCHAN_LIST_CLEARED);
			anope_cmd_global(s_OperServ, "%s cleared the KillChan list.", u->nick);
			alog("[os_killchan] %s cleared the KillChan list.", u->nick);
		}
	} else {
		moduleNoticeLang(s_OperServ, u, LANG_UNKWN_KILLCHAN_OPTION);
		moduleNoticeLang(s_OperServ, u, LANG_KILLCHAN_SYNTAX);
	}

	if (cmd) free(cmd);
	if (chan) free(chan);
	if (reason) free(reason);

	return MOD_CONT;
}

void enforce_kill(KillChan *kc) {
	Channel *c;
	struct c_userlist *cu, *next;
	char mask[USERMAX + HOSTMAX + 2];

	if (!kc)
		return;
	if (!(c = findchan(kc->name)))
		return;

	for (cu = c->users; cu; cu = next) {
		next = cu->next;
		if (is_oper(cu->user) || is_ulined(cu->user->server->name) ||
				(!KillChanRegistered && nick_identified(cu->user)))
			continue;

		(void) strncpy(mask, "*@", 3);
		strncat(mask, cu->user->host, HOSTMAX);
		if (kc->reason)
			moduleNoticeLang(s_OperServ, cu->user, LANG_KILLED, kc->name, kc->reason);
		else
			moduleNoticeLang(s_OperServ, cu->user, LANG_KILLED_NO_REASON, kc->name);
		add_akill(NULL, mask, s_OperServ, time(NULL) + KillChanAKillTime, kc->reason ? kc->reason : "User joined a KillChan.");
		check_akill(cu->user->nick, cu->user->username, cu->user->host, NULL, NULL);
		kc->kills++;
		alog("[os_killchan] Adding %s to AKILL list for joining %s (%s).",
				mask, kc->name, kc->reason ? kc->reason : "");
	}
}

void enforce_kill_all() {
	int i;
	KillChan *kc;

	for (i = 0; i < 256; i++)
		for (kc = channels[i]; kc; kc = kc->next)
			enforce_kill(kc);
}

/* ------------------------------------------------------------------------------- */

int check_join(int argc, char **argv) {
	KillChan *kc;

	if (argc != 3)
		return MOD_CONT;
	if (stricmp(argv[0], EVENT_STOP))
		return MOD_CONT;
	if (!(kc = getKillChan(argv[2])))
		return MOD_CONT;

	enforce_kill(kc);

	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

/*
 * Control functions for managing the killchan list..
 */

KillChan *createKillChan(char *chan, char *reason) {
	KillChan *prev, *ptr, *kc;

	if (!chan)
		return NULL;

	kc = getKillChan(chan);
	if (kc)
		return NULL;

	kc = scalloc(1, sizeof(KillChan));
	strscpy(kc->name, chan, CHANMAX);
	if (reason)
		kc->reason = sstrdup(reason);
	else
		kc->reason = NULL;
	kc->kills = 0;

	/* Go to the right position to insert.. */
	for (prev = NULL, ptr = channels[(unsigned char) tolower(chan[1])];
			ptr != NULL && stricmp(ptr->name, chan) < 0;
			prev = ptr, ptr = ptr->next);

	/* Insert.. */
	kc->prev = prev;
	kc->next = ptr;
	if (!prev)
		channels[(unsigned char) tolower(chan[1])] = kc;
	else
		prev->next = kc;
	if (ptr)
		ptr->prev = kc;

	return kc;
}

KillChan *getKillChan(char *chan) {
	KillChan *kc;

	for (kc = channels[(unsigned char) tolower(chan[1])]; 
			kc != NULL && (stricmp(kc->name, chan));
			kc = kc->next);

	return kc;
}

void delKillChan(KillChan *kc) {
	if (!kc)
		return;

	/* Remove record from the list.. */
	if (kc->next)
		kc->next->prev = kc->prev;
	if (kc->prev)
		kc->prev->next = kc->next;
	else
		channels[(unsigned char) tolower(kc->name[1])] = kc->next;

	/* No references remain, now free the record.. */
	freeKillChan(kc);
}

void freeKillChan(KillChan *kc) {
	if (kc->reason) free(kc->reason);
	free(kc);
}

void clear_db() {
	int i;
	KillChan *kc, *next;

	for (i = 0; i < 256; i++) {
		for (kc = channels[i]; kc; kc = next) {
			next = kc->next;
			freeKillChan(kc);
		}
		channels[i] = NULL;
	}
}

/* ------------------------------------------------------------------------------- */


void load_killchan_db(void) {
	DBFile *dbptr = scalloc(1, sizeof(DBFile));
	char *key, *value;
	KillChan *kc = NULL;
	int retval, error = 1;

	fill_db_ptr(dbptr, 0, KILLCHANDBVERSION, s_OperServ, KillChanDB);

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
			if (kc) {
				kc = NULL;
			} else
				alog("[\002os_killchan\002] Warning: Encountered unexpected BLOCKEND in database!");
		} else {              /* DB_READ_SUCCESS */
			if (!*key || !*value)
				continue;

			if (!strcmp(key, "KC")) {
				if (kc) {
					alog("[\002os_killchan\002] ERROR: KC: Previous KillChan entry still open!");
					alog("[\002os_killchan\002] Aborting database loading...");
					break;
				}
				kc = createKillChan(value, NULL);
			} else if (!strcmp(key, "KCr")) {
				if (!kc) {
					alog("[\002os_killchan\002] ERROR: Encountered unexpected 'KCr' entry in database!");
					alog("[\002os_killchan\002] Aborting database loading...");
					break;
				}
				kc->reason = sstrdup(value);
			} else if (!strcmp(key, "KCk")) {
				if (!kc) {
					alog("[\002os_killchan\002] ERROR: Encountered unexpected 'KCk' entry in database!");
					alog("[\002os_killchan\002] Aborting database loading...");
					break;
				}
				kc->kills = atoi(value);
			} else if (!strcmp(key, "KILLCHAN DB VERSION")) {
				if ((int)atoi(value) != KILLCHANDBVERSION) {
					alog("[\002os_killchan\002] ERROR: Database version does not match any versions supported by this module.");
					alog("[\002os_killchan\002] Aborting database loading...");
					break;
				}
			} else if (!stricmp(key, "EOF")) {
				error = 0;
				break;
			} else {
				alog("[\002os_killchan\002] ERROR: Database element '%s' not recognized.", key);
				break;
			}
		} 					/* else */
	}					/* while */

	/* If we've encountered an error, clean the DB and start with a fresh one..*/
	if (error) {
		clear_db();
		alog("[\002os_killchan\002] Starting with fresh database...");
	}

	free(dbptr);
}


void save_killchan_db(void) {
	DBFile *dbptr = scalloc(1, sizeof(DBFile));
	KillChan *kc;
	int i;

	if (!KillChanDB)
		return;

	fill_db_ptr(dbptr, 0, KILLCHANDBVERSION, s_OperServ, KillChanDB);

	/* time to backup the old db */
	rename(KillChanDB, dbptr->temp_name);

	if (new_open_db_write(dbptr)) {
		rename(dbptr->temp_name, KillChanDB);
		free(dbptr);
		return;                /* Bang, an error occurred */
	}

	/* Store the version of the DB in the DB as well...
	 * This will make stuff a lot easier if the database scheme needs to modified. */
	new_write_db_entry("KILLCHAN DB VERSION", dbptr, "%d", KILLCHANDBVERSION);

	/* Go over all channels..*/
	for (i = 0; i < 256; i++) {
		for (kc = channels[i]; kc; kc = kc->next) {
			new_write_db_entry("KC", dbptr, "%s", kc->name);
			if (kc->reason)
				new_write_db_entry("KCr", dbptr, "%s", kc->reason);
			if (kc->kills)
				new_write_db_entry("KCk", dbptr, "%d", kc->kills);
			new_write_db_endofblock(dbptr);
		}
	}

	new_write_db_entry("EOF", dbptr, "");

	if (dbptr) {
		new_close_db(dbptr->fptr, NULL, NULL);  /* close file */
		remove(dbptr->temp_name);       /* saved successfully, no need to keep the old one */
		free(dbptr);           /* free the db struct */
	}
}

/* ------------------------------------------------------------------------------- */

/**
 * When anope saves her databases, we do the same.
 **/
int do_save(int argc, char **argv) {
	if ((argc >= 1) && (!stricmp(argv[0], EVENT_STOP)))
		save_killchan_db();

	return MOD_CONT;
}


/**
 * When anope backs her databases up, we do the same.
 **/
int db_backup(int argc, char **argv) {
	if ((argc >= 1) && (!stricmp(argv[0], EVENT_STOP))) {
		alog("[os_killchan] Backing up killchan database...");
		ModuleDatabaseBackup(KillChanDB);
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
			alog("[\002os_killchan\002] Unsupported module found: os_raw.. (This is fatal!)");
			supported = -1;
		}
		if (findCommand(OPERSERV, "RAW")) {
			alog("[\002os_killchan\002] Unsupported command found: RAW.. (This is fatal!)");
			supported = -1;
		}
		if (!DisableRaw) {
			alog("[\002os_killchan\002] RAW has NOT been disabled! (This is fatal!)");
			supported = -1;
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
			alog("[\002os_killchan\002] Unsupported module found: %s.. (This is fatal!)", argv[0]);
			supported = -1;
		}

		if (supported != old_supported) {
			if (supported == 0) {
				alog("[\002os_killchan\002] Warning: Module continuing in unsupported mode!");
			} else if (supported == -1) {
				alog("[\002os_killchan\002] Disabling module due to incompatibilities!");
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
			alog("[\002os_killchan\002] Unsupported command found: %s.. (This is fatal!)", argv[0]);
			supported = -1;
		}

		if (supported != old_supported) {
			if (supported == 0) {
				alog("[\002os_killchan\002] Warning: Module continuing in unsupported mode!");
			} else if (supported == -1) {
				alog("[\002os_killchan\002] Disabling module due to incompatibilities!");
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
	snprintf(tmp, BUFSIZE, "%s-%s", version_flags, ((supported == -1) ? "U" : 
			(supported == 0) ? "u" : "S"));
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
		m = findModule("os_killchan");

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
		{{"OSKillChanDB", {{PARAM_STRING, PARAM_RELOAD, &KillChanDB}}}},
		{{"KillChanRegistered", {{PARAM_SET, PARAM_RELOAD, &KillChanRegistered}}}},
		{{"KillChanAKillTime", {{PARAM_TIME, PARAM_RELOAD, &KillChanAKillTime}}}},
	};

	if (KillChanDB)
		free(KillChanDB);
	KillChanDB = NULL;
	KillChanRegistered = 0;
	KillChanAKillTime = 0;

	for (i = 0; i < 3; i++)
		moduleGetConfigDirective(confvalues[i]);

	if (!KillChanDB)
		KillChanDB = sstrdup(DefKillChanDB);
	if (!KillChanAKillTime)
		KillChanAKillTime = 86400;		 /* 1 day */
	else if (KillChanAKillTime < 60) {
		KillChanAKillTime = 86400;		 /* 1 day */
		alog("[\002os_killchan\002] Warning: KillChanAKillTime needs to be at least 60. Using default values...");
	}

	if (debug)
		alog ("[os_killchan] debug: KillChanDB set to '%s'.; KillChanRegistered is %sSET; KillChanAKillTime is %ds.",
				KillChanDB, KillChanRegistered ? "" : "NOT ", KillChanAKillTime);
}


/**
 * Upon /os reload call the routines for reloading the configuration directives
 **/
int reload_config(int argc, char **argv) {
	int old_supported = supported;

	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			if (debug)
				alog("[os_killchan] debug: Reloading configuration directives...");
			load_config();
		}
	}

	check_modules();
	if (supported != old_supported) {
		if (supported == 0) {
			alog("[\002os_killchan\002] Warning: Module continuing in unsupported mode!");
		} else if (supported == -1) {
			alog("[\002os_killchan\002] Disabling module due to incompatibilities!");
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
		/* LANG_KILLCHAN_DESC */
		"KILLCHAN       Manipulate the KillChan list.",
		/* LANG_KILLCHAN_SYNTAX */
		"Syntax: KILLCHAN { ADD | DEL | LIST | VIEW | CLEAR } [\037channel\037] [\037reason\037]",
		/* LANG_KILLCHAN_SYNTAX_EXT */
		"Syntax: \002KILLCHAN ADD \037channel\037 [\037reason\037]\002\n"
		"        \002KILLCHAN DEL \037channel\037\002\n"
		"        \002KILLCHAN LIST\002 \n"
		"        \002KILLCHAN VIEW \037channel\037\002\n"
		"        \002KILLCHAN CLEAR\002 \n"
		"\n"
		"This command allows services admins to manage a list of channels which will\n"
		"operate as a death trap. Services will akill all cients who join the channel.\n"
		"Note that opers, clients on ulined servers and - depending on configuration -\n"
		"registered users are also exempt.",
		/* LANG_KILLCHAN_DISABLED */
		"Sorry, KillChan list modification is temporarily disabled.",
		/* LANG_NO_LOCAL_CHAN */
		"Cannot add local channels to the KillChan list.",
		/* LANG_CHAN_SYMB_REQUIRED */
		"The channel symbol '\002#\002' is required.",
		/* CHAN_X_REGISTERED */
		"Registered channels cannot be used as kill channels.",
		/* LANG_KILLCHAN_ADDED */
		"\002%s\002 has been added to the KillChan list.",
		/* LANG_KILLCHAN_UPDATEDLANG_KILLCHAN_UPDATED */
		"KillChan record for \002%s\002 has been updated.",
		/* LANG_KILLCHAN_ENFORCED */
		"All unauthorized clients in \002%s\002 have been akilled.",
		/* LANG_KILLCHAN_NO_SUCH_ENTRY */
		"\002%s\002 is not in the KillChan list.",
		/* LANG_KILLCHAN_DELETED */
		"\002%s\002 has been deleted from the KillChan list.",
		/* LANG_KILLCHAN_LIST_EMPTY */
		"The KillChan list is empty.",
		/* LANG_KILLCHAN_ENTRY */
		"Channel: %s  (Kills: %d; Reason: %s)",
		/* LANG_KILLCHAN_ENTRIES */
		"Found %d entries",
		/* LANG_KILLCHAN_NAME */
		"Channel: %s - Kills: %d",
		/* LANG_KILLCHAN_REASON */
		"Reason: %s",
		/* LANG_KILLCHAN_NO_REASON */
		"No reason has been given.",
		/* LANG_KILLCHAN_LIST_CLEARED */
		"KillChan list cleared.",
		/* LANG_UNKWN_KILLCHAN_OPTION */
		"Unknown KILLCHAN option.",
		/* LANG_KILLED */
		"You are boing Auto-Killed for joining a forbidden channel %s. (%s)",
		/* LANG_KILLED_NO_REASON */
		"You are boing Auto-Killed for joining a forbidden channel %s.",
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
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
