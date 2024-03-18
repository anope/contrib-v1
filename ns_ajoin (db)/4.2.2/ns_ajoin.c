/**
 * -----------------------------------------------------------------------------
 * Name    : ns_ajoin
 * Author  : Viper  <Viper@Anope.org>
 * Date    : 23/10/2006  (Last update: 02/12/2009)
 * Version : 4.2.2
 * -----------------------------------------------------------------------------
 * Limitations: Any IRCd which supports SVSJOIN
 * Tested     : Anope-1.8.0 + UnrealIRCd 3.2.6
 * Requires   : Anope-1.8.0
 * -----------------------------------------------------------------------------
 * This module adds the AJOIN command to nickserv allowing users to maintain
 * a server side auto-join list.
 * The ajoin settings can be configured for each nick group through SET AJOIN.
 *
 * This module requires the IRCd to support SVSJOIN.
 * At the time of writing this module will only properly work with following IRCDs:
 *   - InspIRCd
 *   - Plexus 3.0 [or later]
 *   - PTLink
 *   - UltimateIRCd 2.8.2 [or later] [Excluding 3.x]
 *   - Unreal 3.1.1 [or later]
 *   - Unreal 3.2 [or later]
 *   - ViagraIRCd 1.3.x [or later]
 *
 * This module regulary checks whether other incompatible modules are loaded.
 * There are 3 possible modes: no problems, conflicting modules are found, 
 * however the module will continue functionning in unsupported mode, and
 * a fatal conflict is found. In this last case the module will disable itself. 
 * These checks can be disabled by undefining the SUPPORTED setting in the 
 * modules' source configuration header after which the module needs to be recompiled.
 * For windows users this means they will have to compile the module themselves
 * as I do not want to distribute an unsupported version myself.
 * Note that to undo entering unsupported mode services have to be restarted.
 *
 * I wrote this module because the other 2 alternatives (addon module by
 * DukePyrolator and ns_ajoin by Scott) are outdated  and require MySQL, which
 * I - like many other people probably - do not use / want to use.
 *
 * The database subroutines have been ported from swhois by Trystan,
 * I wouldn't have been able to pull this off without those...
 *
 * Note that this module should be placed in ModuleDelayedAutoload,
 * and NOT in ModuleAutoload or one of the CoreModules directives.
 * -----------------------------------------------------------------------------
 * Translations:
 *
 *     - English and Dutch Languages provided and maintained by myself.
 *     - German translation     : SNU, Han`
 *     - Turkish translation    : ice
 *     - Russian translation    : Kein
 * -----------------------------------------------------------------------------
 *
 * Changelog:
 *
 *    4.2.2  Added default autojoinlist and ajoinunregistered options.
 *           Added supported/unsupported mode.
 *           Added extended version info to MODLIST and MODINFO.
 *           Updated module to work with Anope-1.7.23b and later.
 *           Fixed typo in dutch language strings.
 *           Fixed bans being removed even though the user is excepted.
 *
 *    4.2.1  Fixed AJOIN not requiring password identify (Reported by ysfm).
 *           Fixed ajoining suspended channels on identify.
 *           Fixed adding forbidden/suspended channels to the ajoin list.
 *           Fixed ChanServ replying to AJOIN ADD if the channel is not registered.
 *
 *    4.2.0  Going to next minor version and considering stable.
 *           Added ability to delete and list entries by number/list.
 *           Added russian translation (Provided by Kein)
 *           Fixed a few typos.
 *           Fixed crashbug when autojoining to dropped/expired channels. (Thanks to "Aded")
 *           Increased default number of entries allowed in AutoJoin list to 12
 *
 *    4.1.15 Fixed module causing a crash when ajoin issued by an unregistered user. (Thanks to "aerolife")
 *
 *    4.1.14 Fixed ajoin failing under several conditions on a channel with +k..
 *           Added ban testing and unbanning if possible...
 *           Because UnrealIRCd is the only IRCd supporting SVSJOIN'ing to +k chans
 *               we now invite before joining on all IRCd's except unreal.
 *           We no longer unban the user if he is excepted...
 *           Added hooks for NS ID.
 *
 *    4.1.13 Limited channels on ajoin list to registered channels only.
 *               This should resolve issues with invalid channel names. (Reported by "Avenger")
 *           Added check before executing the SVSJOIN to make sure channel isn't forbidden.
 *           Fixed bug causing remaining ajoins to be aborted if user is already on a ajoin chan.
 *
 *    4.1.12 Removed remaining bit of debug code
 *           Fixed some minor language details  (Complained about by "SNU" ;-) )
 *           Added german translation (Provided by SNU)
 *
 *    4.1.11 Fixed possible segfault when loading is aborted because of unsupported ircd
 *           Fixed missing help for SET AJOIN.
 *
 *    4.1.10 Fixed segfault when saving DB after nick was dropped/expired. (Reported by "hexa")
 *               (Rewrote part of DB saving routine and the way AjoinEntries are addressed.(ugly!))
 *           Added InspIRCd as supported IRCd. (Their DOCs say it implements SVSJOIN so..)
 *           Added turkish translation (Provided by ice)
 *
 *    4.1.9  Added win32 support
 *           Fixed segfault when AJOIN is used by an unregistered user
 *
 *    4.1.8  Fixed vhost being set after ajoining (Reported by "CuttingEdge")
 *           Now also performing ajoin on UPDATE
 *
 *    4.1.7  Fixed crashes in ajoin add/del
 *           Added Anope version check
 *           Default ajoin flags moved to a const
 *           No longer storing empty entries with default configuration to the DB
 *
 *    4.1.6  Added SET AJOIN option
 *           Changed database scheme to include version info.
 *           Fixed a memleak in the AjoinEntry handlers.
 *
 *    4.1.5  Fixed segfaults on load & unload
 *           AnopeFini now goes over the ajoinTable.. should speed unloading up.
 *           Anope now also backs the ajoin database up.
 *
 *    4.1.4  Finished runtime storing and database writing and reading.
 *           Implemented ajoining on identify.
 *
 *    4.1.3  Replaced hardcoded replies with langtables.
 *
 *    4.1.2  Replaced database system
 *
 *    4.1.1  First 'working' Alpha Version
 *
 *    4.1    Module Development taken over by me (Viper)
 *           Beginning with clean development version.
 *
 * -----------------------------------------------------------------------------
 **/

/**
 * TODO:
 *
 *    << hope there are no bugs left >>
 *
 **/


/**
 * Configuration directives that should be copy-pasted to services.conf

# AJoinDB [OPTIONAL]
# Module: ns_ajoin
#
# Use the given filename as database to store the AJOINs.
# If not given, the default of "ns_ajoin.db" will be used.
#
#AJoinDB "ns_ajoin.db"

# AJoinDefChans [OPTIONAL]
# Module: ns_ajoin
#
# Space seperated list of default ajoin channels.
# When a new nick is registered these will be automatically
# added to the users' AJOIN list.
#
#AJoinDefChans "#anope #denora"

# AJoinUnRegistered [OPTIONAL]
# Module: ns_ajoin
#
# When set ns_ajoin will AJOIN all connecting users
# who aren't using a registered nick (they don't need 
# to be identified) to the default ajoin channels.
# Note that users will be unable to join channels with
# a key or invitemode set. 
#
# Be aware the most users will experience this as very
# abusive and it will likely give your network a bad 
# reputation.
# This was added only because under some  circumstances
# it has a legit use.
#
#AJoinUnRegistered

 *
 **/

#include "module.h"

/*------------------------------Configuration Block----------------------------*/

/**
 * When defined the module operates in supported mode.
 * This means it will stop functionning if incompatible modules are
 * detected.
 * When commenting this or undefining, these checks will be disabled,
 * however no further support will be provided by the author in case 
 * problems arise.
 **/
#define SUPPORTED


/**
 * Determines the maximum entries in an ajoin list.
 *
 * If this value is changed, it will not affect AjoinEntries allready in the
 * database, however now ones can't be added if the limit is exceeded.
 **/

int NSAjoinMax = 15;


/*-------------------------End of Configuration Block--------------------------*/

#define AUTHOR "Viper"
#define VERSION "4.2.2"
#define AJOINDBVERSION 1


/* Language defines */
#define LANG_NUM_STRINGS 					32

#define LANG_AJOIN_DESC						0
#define LANG_AJOIN_SYNTAX					1
#define LANG_AJOIN_SYNTAX_EXT				2
#define LANG_AJOIN_DISABLED					3
#define LANG_NO_LOCAL_CHAN					4
#define LANG_CHAN_SYMB_REQUIRED				5
#define LANG_CHAN_UPDATED					6
#define LANG_AJOIN_LIST_FULL				7
#define LANG_CHAN_ADDED						8
#define LANG_NO_AJOINS						9
#define LANG_AJOIN_LIST_EMPTY				10
#define LANG_NO_ENTRY						11
#define LANG_CHAN_DELETED					12
#define LANG_AJOIN_ENTRY					13
#define LANG_AJOIN_ENTRIES					14
#define LANG_AJOIN_LIST_CLEARED				15
#define LANG_UNKWN_AJOIN_OPTION				16
#define LANG_AJOINING						17
#define LANG_AJOINING_FAILED				18
#define LANG_SET_AJOIN_DESC					19
#define LANG_SET_AJOIN_SYNTAX				20
#define LANG_SET_AJOIN_SYNTAX_EXT			21
#define LANG_SET_AJOIN_ON					22
#define LANG_SET_AJOIN_SILENT				23
#define LANG_SET_AJOIN_OFF					24
#define LANG_AJOINING_SUM_SUCCESS			25
#define LANG_AJOINING_SUM_FAILED			26
#define LANG_AJOIN_DELETED_NR_1				27
#define LANG_AJOIN_DELETED_NR				28
#define LANG_AJOIN_DEFAULT_ENTRIES_ADDED	29
#define LANG_AJOIN_UNREGISTERED				30
#define LANG_AJOIN_NOT_AVAILABLE			31


/* Flags to set in database */
#define AJOIN_ON        (1 << 0)
#define AJOIN_SILENT    (1 << 1)

/* Database seperators */
#define SEPARATOR  ':'          /* End of a key, seperates keys from values */
#define BLOCKEND   '\n'         /* End of a block, e.g. a whole nick/channel or a subblock */
#define VALUEEND   '\000'       /* End of a value */
#define SUBSTART   '\010'       /* Beginning of a new subblock, closed by a BLOCKEND */

/* Database reading return values */
#define DB_READ_SUCCESS   0
#define DB_READ_ERROR     1
#define DB_EOF_ERROR      2
#define DB_VERSION_ERROR  3
#define DB_READ_BLOCKEND  4
#define DB_READ_SUBSTART  5

#define DB_WRITE_SUCCESS  0
#define DB_WRITE_ERROR    1
#define DB_WRITE_NOVAL    2

/* Database Key, Value max length */
#define MAXKEYLEN 128
#define MAXVALLEN 1024


/* Structs */
typedef struct db_file_ DBFile;
typedef struct ajoinchan_ AjoinChan;
typedef struct ajoinentry_ AjoinEntry;

struct db_file_ {
	FILE *fptr;             /* Pointer to the opened file */
	int db_version;         /* The db version of the datafiles (only needed for reading) */
	int core_db_version;    /* The current db version of this anope source */
	char service[256];      /* StatServ/etc. */
	char filename[256];     /* Filename of the database */
	char temp_name[262];    /* Temp filename of the database */
};

struct ajoinchan_ {
	AjoinChan *prev, *next;
	char *channel;
	char *key;
	int nr;
};

struct ajoinentry_ {
	/* Specifies storage location in ajoinTable */
	int row, col;
	AjoinEntry *prev, *next;
	int ajchannels;
	AjoinChan *channels;
	/* Flags store AJOIN settings.
	 * Bits are used as followed:
	 *   User wants to be autojoined            {bit 0}
	 *   User wants autojoin to be silent       {bit 1}
	 */
	uint16 flags;
	/* Used to check during the cleanup whether the entry is still being used.
	 * This is always 0 except when the DB saving function has completed going
	 * over all NickCore's. The ajoin entries with in_use set to 0 at that time
	 * are no longer used and cleared. In the other entries in_use is reset to 0. */
	int in_use;
};


/* Constants */
char *DefAJoinDB = "ns_ajoin.db";
char *ModDataKey = "ajoin";
uint16 DefAjoinFlags = 1;


/* Variables */
int supported;
char *AJoinDB;
int AJoinDefChansNr;
char **AJoinDefChans;
int AJoinUnRegistered;

/* This stores the last used row in teh ajoinTable so the module can attempt to
 * spread the entries out as evenly as possible... */
int counter;

/* We still need a table cause we won't be able to reach the AjoinEntry if
 * the nick is dropped... this sucks i know, but it s just a list of references */
/* Table will be cleaned of invalid entries every time the database is saved */
AjoinEntry *ajoinTable[1024];


/* Functions */
void do_help_list(User *u);
int do_help(User *u);
int add_ajoin_option(User *u);
int do_set_ajoin_help(User *u);

int do_ajoin(User *u);
int do_identify(User *u);
int ns_set(User *u);
int null_func(User *u);
int is_banned(ChannelInfo *ci, User *u);

int new_open_db_read(DBFile *dbptr, char **key, char **value);
int new_open_db_write(DBFile *dbptr);
void new_close_db(FILE *fptr, char **key, char **value);
int new_read_db_entry(char **key, char **value, FILE * fptr);
int new_write_db_entry(const char *key, DBFile *dbptr, const char *fmt, ...);
int new_write_db_endofblock(DBFile *dbptr);
void fill_db_ptr(DBFile *dbptr, int version, int core_version, char service[256], char filename[256]);

static int ajoin_del_callback(User * u, int num, va_list args);
static int ajoin_list_callback(User * u, int num, va_list args);

static AjoinChan *addAjoinChan(AjoinEntry *ae, char *chan, char *key);
AjoinChan *findAjoinChan(AjoinEntry *ae, char *chan);
AjoinChan *findAjoinChanNr(AjoinEntry *ae, int nr);
static int deleteAjoinChan(AjoinEntry *ae, AjoinChan *ac);
static int clearAjoinChans(AjoinEntry *ae);
static int numberAjoinChans(AjoinEntry *ae);

static AjoinEntry *createAjoinEntry(NickCore *nc);
AjoinEntry *getAjoinEntry(NickCore *nc);
static int deleteAjoinEntry(AjoinEntry *ae);
void freeUnusedEntries();

int do_save(int argc, char **argv);
int db_backup(int argc, char **argv);
void load_ajoin_db(void);
void save_ajoin_db(void);
AjoinEntry *go_to_next_entry(uint16 skipped, AjoinEntry *next, AjoinEntry *ae);

int nick_registered(int argc, char **argv);
int client_connect(int argc, char **argv);

int valid_ircd(void);
int check_modules(void);

char* get_flags();
void update_version(void);

void load_config(void);
int reload_config(int argc, char **argv);
int backup_ajoin_db(int argc, char **argv);
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
	EvtHook *hook;

	alog("[\002ns_ajoin\002] Loading module...");

	counter = 0;
	supported = 1;

	if (!valid_ircd()) {
		alog("[\002ns_ajoin\002] ERROR: IRCd not supported by this module");
		alog("[\002ns_ajoin\002] Unloading module...");
		return MOD_STOP;
	}

	if (!moduleMinVersion(1,8,0,1899)) {
		alog("[\002ns_ajoin\002] Your version of Anope isn't supported. Please update to a newer release.");
		return MOD_STOP;
	}

	check_modules();
	if (supported == 0) {
		alog("[\002ns_ajoin\002] Warning: Module continueing in unsupported mode!");
	} else if (supported == -1) {
		alog("[\002ns_ajoin\002] Unloading module due to incompatibilities!");
		return MOD_STOP;
	}

	/* Create AJOIN command.. */
	c = createCommand("AJOIN", do_ajoin, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV,c,MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002ns_ajoin\002] Cannot create AJOIN command...");
		return MOD_STOP;
	}
	moduleAddHelp(c,do_help);
	moduleSetNickHelp(do_help_list);


	/* Hook to the IDENTIFY and UPDATE command */
	c = createCommand("IDENTIFY", do_identify, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV,c,MOD_TAIL) != MOD_ERR_OK) {
		alog("[\002ns_ajoin\002] Cannot hook to IDENTIFY command...");
		return MOD_STOP;
	}

	c = createCommand("ID", do_identify, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV,c,MOD_TAIL) != MOD_ERR_OK) {
		alog("[\002ns_ajoin\002] Cannot hook to ID command...");
		return MOD_STOP;
	}

	c = createCommand("UPDATE", do_identify, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV,c,MOD_TAIL) != MOD_ERR_OK) {
		alog("[\002ns_ajoin\002] Cannot hook to UPDATE command...");
		return MOD_STOP;
	}


	/* Create the SET AJOIN command, its help and add it to cmd listing.. */
	c = createCommand("SET", ns_set, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002ns_ajoin\002] Cannot create SET AJOIN command...");
		return MOD_STOP;
	}

	c = createCommand("SET AJOIN", NULL, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV, c, MOD_TAIL) != MOD_ERR_OK) {
		alog("[\002ns_ajoin\002] Cannot create help for the SET AJOIN command...");
		return MOD_STOP;
	}
	moduleAddHelp(c, do_set_ajoin_help);

	c = createCommand("SET", null_func, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV, c, MOD_TAIL) != MOD_ERR_OK) {
		alog("[\002ns_ajoin\002] Cannot add AJOIN to SET options...");
		return MOD_STOP;
	}
	moduleAddHelp(c, add_ajoin_option);


	/* Hook to some events.. */
	hook = createEventHook(EVENT_NICK_REGISTERED, nick_registered);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002ns_ajoin\002] Can't hook to EVENT_NICK_REGISTERED event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_NEWNICK, client_connect);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002ns_ajoin\002] Can't hook to EVENT_NEWNICK event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_RELOAD, reload_config);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002ns_ajoin\002] Can't hook to EVENT_RELOAD event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_DB_SAVING, do_save);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002ns_ajoin\002] Can't hook to EVENT_DB_SAVING event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_DB_BACKUP, db_backup);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002ns_ajoin\002] Can't hook to EVENT_DB_BACKUP event");
		return MOD_STOP;
	}

	load_config();
	add_languages();
	load_ajoin_db();

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	
	/* Update version info.. */
	update_version();

	alog("[\002ns_ajoin\002] Module loaded successfully...");

	return MOD_CONT;
}


/**
 * Unload the module
 **/
void AnopeFini(void) {
	AjoinEntry *ae = NULL, *next = NULL;
	int i;

	if (AJoinDB)
		save_ajoin_db();

	/* clear the memory.... */
	for (i = 0; i < 1024; i++) {
		for (ae = ajoinTable[i]; ae; ae = next) {
			next = ae->next;
			deleteAjoinEntry(ae);
		}
	}

	if (AJoinDB)
		free(AJoinDB);
	for ( i = 0; i < AJoinDefChansNr; i++)
		if (AJoinDefChans[i])
			free(AJoinDefChans[i]);
	if (AJoinDefChans)
		free(AJoinDefChans);

	alog("[\002ns_ajoin\002] Unloading module...");
}


/* ------------------------------------------------------------------------------- */


/**
 * Add the AJOIN command to the NickServ HELP listing.
 **/
void do_help_list(User *u) {
	if (supported >= 0)
		moduleNoticeLang(s_NickServ, u, LANG_AJOIN_DESC);
}


/**
 * Show the extended help on the AJOIN command.
 **/
int do_help(User *u) {
	if (supported < 0)
		moduleNoticeLang(s_NickServ, u, LANG_AJOIN_NOT_AVAILABLE);
	else
		moduleNoticeLang(s_NickServ, u, LANG_AJOIN_SYNTAX_EXT);
	return MOD_CONT;
}


/**
 * Add the AJOIN option to the end of the SET options listing.
 **/
int add_ajoin_option(User *u) {
	if (supported >= 0)
		moduleNoticeLang(s_NickServ, u, LANG_SET_AJOIN_DESC);
	return MOD_CONT;
}


/**
 * Show the extended help on the SET AJOIN command.
 **/
int do_set_ajoin_help(User *u) {
	if (supported < 0)
		moduleNoticeLang(s_NickServ, u, LANG_AJOIN_NOT_AVAILABLE);
	else
		moduleNoticeLang(s_NickServ, u, LANG_SET_AJOIN_SYNTAX_EXT);
	return MOD_CONT;
}


/* ------------------------------------------------------------------------------- */

int do_ajoin(User *u) {
	ChannelInfo *ci;
	char *buffer, *cmd, *chan, *key;

	if (supported < 0) {
		moduleNoticeLang(s_NickServ, u, LANG_AJOIN_NOT_AVAILABLE);
		return MOD_CONT;
	}

	buffer = moduleGetLastBuffer();
	cmd = myStrGetToken(buffer, ' ', 0);
	chan = myStrGetToken(buffer, ' ', 1);
	key = myStrGetToken(buffer, ' ', 2);

	if (!cmd)
		moduleNoticeLang(s_NickServ, u, LANG_AJOIN_SYNTAX);

	else if (stricmp(cmd, "ADD") == 0) {
		if (readonly)
			moduleNoticeLang(s_NickServ, u, LANG_AJOIN_DISABLED);
		else if (!u->na)
			notice_lang(s_NickServ, u, NICK_NOT_REGISTERED);
		else if (!nick_identified(u))
			notice_lang(s_NickServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
		else if (!chan)
			moduleNoticeLang(s_NickServ, u, LANG_AJOIN_SYNTAX);
		else if (*chan == '&')
			moduleNoticeLang(s_NickServ, u, LANG_NO_LOCAL_CHAN);
		else if (*chan != '#')
			moduleNoticeLang(s_NickServ, u, LANG_CHAN_SYMB_REQUIRED);
		else if (!anope_valid_chan(chan))
			notice_lang(s_NickServ, u, CHAN_X_INVALID, chan);
		else if (!(ci = cs_findchan(chan)))
			notice_lang(s_NickServ, u, CHAN_X_NOT_REGISTERED, chan);
		else if (ci->flags & CI_VERBOTEN)
			notice_lang(s_NickServ, u, CHAN_X_FORBIDDEN, chan);
		else if (ci->flags & CI_SUSPENDED)
			notice_lang(s_NickServ, u, CHAN_X_FORBIDDEN, chan);
		else {
			AjoinEntry *ae = NULL;
			AjoinChan *ac = NULL;

			ae = getAjoinEntry(u->na->nc);
			ac = findAjoinChan(ae, chan);

			if (!ae)
				ae = createAjoinEntry(u->na->nc);

			if (ac) {
				/* update existing entry */
				if (!key && ac->key) {
					free(ac->key);
					ac->key = NULL;
				} else if (key) {
					free(ac->key);
					ac->key = sstrdup(key);
				}
				moduleNoticeLang(s_NickServ, u, LANG_CHAN_UPDATED);
			} else {
				if (ae->ajchannels == NSAjoinMax)
					moduleNoticeLang(s_NickServ, u, LANG_AJOIN_LIST_FULL);
				else {
					addAjoinChan(ae, chan, key);
					moduleNoticeLang(s_NickServ, u, LANG_CHAN_ADDED, chan);
				}
			}
		}

	} else if (stricmp(cmd, "DEL") == 0) {
		AjoinEntry *entry;

		if (readonly)
			moduleNoticeLang(s_NickServ, u, LANG_AJOIN_DISABLED);
		else if (!u->na)
			notice_lang(s_NickServ, u, NICK_NOT_REGISTERED);
		else if (!chan)
			moduleNoticeLang(s_NickServ, u, LANG_AJOIN_SYNTAX);
		else if (!(entry = getAjoinEntry(u->na->nc)))
			moduleNoticeLang(s_NickServ, u, LANG_NO_AJOINS);
		else {
			/* we r a valid chan, delete us if we r present... */
			if (entry->ajchannels == 0)
				moduleNoticeLang(s_NickServ, u, LANG_AJOIN_LIST_EMPTY);

			else {
				/* Check if the given target is a number/list.
				 * Only  search if it isn't. */
				if (isdigit(*chan) && strspn(chan, "1234567890,-") == strlen(chan)) {
					int deleted, count;
					deleted = process_numlist(chan, &count, ajoin_del_callback, u, entry);
					numberAjoinChans(entry);

					if (deleted == 1)
						moduleNoticeLang(s_NickServ, u, LANG_AJOIN_DELETED_NR_1, deleted);
					else
						moduleNoticeLang(s_NickServ, u, LANG_AJOIN_DELETED_NR, deleted);
				} else {
					if (*chan != '#')
						moduleNoticeLang(s_NickServ, u, LANG_CHAN_SYMB_REQUIRED);
					else {
						AjoinChan *ac = findAjoinChan(entry, chan);

						if (!ac)
							moduleNoticeLang(s_NickServ, u, LANG_NO_ENTRY, chan);

						else {
							deleteAjoinChan(entry, ac);
							numberAjoinChans(entry);
							moduleNoticeLang(s_NickServ, u, LANG_CHAN_DELETED, chan);
						}
					}
				}
			}
		}

	} else if (stricmp(cmd, "LIST") == 0) {
		int i = 0;
		AjoinEntry *ae = NULL;
		AjoinChan *ac = NULL;

		if (!u->na) {
			notice_lang(s_NickServ, u, NICK_NOT_REGISTERED);
		} else {
			ae = getAjoinEntry(u->na->nc);

			if (!ae)
				moduleNoticeLang(s_NickServ, u, LANG_NO_AJOINS);

			else {
				if (ae->ajchannels == 0)
					moduleNoticeLang(s_NickServ, u, LANG_AJOIN_LIST_EMPTY);

				else {
					/* Check if the given target is a number/list. */
					if (chan && isdigit(*chan) && strspn(chan, "1234567890,-") == strlen(chan)) {
						int count, found;
						found = process_numlist(chan, &count, ajoin_list_callback, u, ae);
						moduleNoticeLang(s_NickServ, u, LANG_AJOIN_ENTRIES, found);
					} else {
						for (ac = ae->channels; ac; ac = ac->next) {
							if (chan && !match_wild_nocase(chan, ac->channel))
								continue;
							moduleNoticeLang(s_NickServ, u, LANG_AJOIN_ENTRY, ac->nr, ac->channel,
									((ac->key) ? ac->key:""));
							i++;
						}

						moduleNoticeLang(s_NickServ, u, LANG_AJOIN_ENTRIES, i);
					}
				}
			}
		}

	} else if (stricmp(cmd, "CLEAR") == 0) {
		AjoinEntry *ae;

		if (readonly)
			moduleNoticeLang(s_NickServ, u, LANG_AJOIN_DISABLED);
		else if (!u->na)
			notice_lang(s_NickServ, u, NICK_NOT_REGISTERED);
		else if (!(ae = getAjoinEntry(u->na->nc)))
			moduleNoticeLang(s_NickServ, u, LANG_AJOIN_LIST_EMPTY);
		else {
			clearAjoinChans(ae);
			moduleNoticeLang(s_NickServ, u, LANG_AJOIN_LIST_CLEARED);
		}

	} else {
		moduleNoticeLang(s_NickServ, u, LANG_UNKWN_AJOIN_OPTION);
		moduleNoticeLang(s_NickServ, u, LANG_AJOIN_SYNTAX);
	}

	if (cmd)
		free(cmd);
	if (chan)
		free(chan);
	if (key)
		free(key);

	return MOD_CONT;
}


int do_identify(User *u) {
	int i = 0, ok, needinvite, counter_succ = 0, counter_f = 0;

	if (supported < 0)
		return MOD_CONT;

	if (nick_identified(u) && u->na->nc) {
		AjoinEntry *ae = NULL;

		ae = getAjoinEntry(u->na->nc);

		if (ae && (ae->flags & AJOIN_ON) && (ae->ajchannels > 0)) {
			AjoinChan *ac = NULL;

			ac = ae->channels;
			for (i = 0; i < ae->ajchannels; i++) {
				Channel *c = NULL;
				ChannelInfo *ci;
				ok = 1, needinvite = 0;

				if (!ac)
					break;

				/* If the channel is no longer registered we won't complain..
				 * We are sure the name is valid because it was registered once.
				 * We do complain when the channel is forbidden.. no point in joining. */
				/* We do the same when it s suspended. We do NOT automatically remove channels though. */
				ci = cs_findchan(ac->channel);
				if (ci && (ci->flags & CI_VERBOTEN)) {
					notice_lang(s_NickServ, u, CHAN_X_FORBIDDEN, ac->channel);
					ok = 0;
				}
				if (ci && (ci->flags & CI_SUSPENDED)) {
					notice_lang(s_NickServ, u, CHAN_X_FORBIDDEN, ac->channel);
					ok = 0;
				}

				/* Check if channel already exists...
				 * We can't use c->ci here because we don't require the chan to still be registered */
				if (ok && ((c = findchan(ac->channel)))) {
					if (is_on_chan(c, u)) {
						ac = ac->next;
						continue;
					}

					else {
						if (c->key) {
							if (!ac->key || (stricmp(ac->key, c->key) != 0)) {
								if (ci && check_access(u, ci, CA_GETKEY)) {
									if (ac->key)
										free(ac->key);
									ac->key = sstrdup(c->key);

								} else
									needinvite = 1;
							}

							/* Any IRCd other then unreal needs invite cause they don't support
							 * SVSJOIN with a key.. */
							if (stricmp(IRCDModule,"unreal32"))
								needinvite = 1;
						}

						/* Check if user is banned and if so, remove is possible.
						 * If we cannot remove it, try to invite */
						if (ci && is_banned(ci, u) && !is_excepted(ci, u)) {
							if (check_access(u, ci, CA_UNBAN))
								common_unban(ci, u->nick);
							else
								needinvite = 1;
						}

						if ((c->mode & anope_get_invite_mode()) || needinvite) {
							if (ci && check_access(u, ci, CA_INVITE))
								anope_cmd_invite(s_NickServ, ac->channel, u->nick);
							else
								ok = 0;
						}
					}
				}

				if (ok) {
					if (ae->flags & AJOIN_SILENT)
						counter_succ++;
					else
						moduleNoticeLang(s_NickServ, u, LANG_AJOINING, ac->channel);

					anope_cmd_svsjoin(s_NickServ, u->nick, ac->channel, ac->key);
				} else {
					if (ae->flags & AJOIN_SILENT)
						counter_f++;
					else
						moduleNoticeLang(s_NickServ, u, LANG_AJOINING_FAILED, ac->channel);
				}

				ac = ac->next;
			}

			if (ae->flags & AJOIN_SILENT) {
				if (counter_succ > 0)
					moduleNoticeLang(s_NickServ, u, LANG_AJOINING_SUM_SUCCESS, counter_succ);
				if (counter_f > 0)
					moduleNoticeLang(s_NickServ, u, LANG_AJOINING_SUM_FAILED, counter_f);
			}
		}
	}

	return MOD_CONT;
}


int ns_set(User *u) {
	char *args, *cmd, *option;

	/* If readonly, let the core ns_set handle it.. */
	if (readonly || supported < 0)
		return MOD_CONT;

	args = moduleGetLastBuffer();
	if (!args)
		return MOD_CONT;

	cmd = myStrGetToken(args, ' ', 0);
	option = myStrGetToken(args, ' ', 1);

	if (cmd && !stricmp(cmd, "AJOIN")) {
		if (!option)
			moduleNoticeLang(s_NickServ, u, LANG_SET_AJOIN_SYNTAX);
		else if (!u->na)
			notice_lang(s_NickServ, u, NICK_NOT_REGISTERED);
		else if (!nick_identified(u))
			notice_lang(s_NickServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
		else {
			AjoinEntry *ae = NULL;
			ae = getAjoinEntry(u->na->nc);

			if (!ae)
				ae = createAjoinEntry(u->na->nc);

			if (!stricmp(option, "ON")) {
				/* Turn off silent bit */
				ae->flags &= ~AJOIN_SILENT;
				/* Turn on use autojoin bit */
				ae->flags |= AJOIN_ON;
				moduleNoticeLang(s_NickServ, u, LANG_SET_AJOIN_ON);

			} else if (!stricmp(option, "SILENT")) {
				/* Turn on silent bit */
				ae->flags |= AJOIN_SILENT;
				/* Turn on use autojoin bit */
				ae->flags |= AJOIN_ON;
				moduleNoticeLang(s_NickServ, u, LANG_SET_AJOIN_SILENT);

			} else if (!stricmp(option, "OFF")) {
				/* Turn off silent bit */
				ae->flags &= ~AJOIN_SILENT;
				/* Turn off use autojoin bit */
				ae->flags &= ~AJOIN_ON;
				moduleNoticeLang(s_NickServ, u, LANG_SET_AJOIN_OFF);

			} else
				moduleNoticeLang(s_NickServ, u, LANG_SET_AJOIN_SYNTAX);
		}

		free(cmd);
		if (option) free (option);
		return MOD_STOP;
	}

	if (cmd) free(cmd);
	if (option) free (option);

	return MOD_CONT;
}


int null_func(User *u) {
	return MOD_CONT;
}


int is_banned(ChannelInfo *ci, User *u) {
	if (!ci->c)
		return 0;

	if (elist_match_user(ci->c->bans, u) && !elist_match_user(ci->c->excepts, u))
		return 1;
	
	return 0;
}

/* ------------------------------------------------------------------------------- */

int nick_registered(int argc, char **argv) {
	User *u;
	NickAlias *na;
	AjoinEntry *ae = NULL;
	int i, added = 0;

	if (argc != 1)
		return MOD_CONT;

	if (!AJoinDefChansNr || !AJoinDefChans || supported < 0)
		return MOD_CONT;

	/* Verify the nickalias exists.. */
	if (!(na = findnick(argv[0])) || !na->nc)
		return MOD_CONT;

	if (AJoinDefChansNr) {
		ae = getAjoinEntry(na->nc);
		if (!ae)
			ae = createAjoinEntry(na->nc);
	}

	for (i = 0; i < AJoinDefChansNr; i++) {
		if (AJoinDefChans[i]) {
			AjoinChan *ac = NULL;

			ac = findAjoinChan(ae, AJoinDefChans[i]);
			/* it shouldn't exist yet..  */
			if (!ac) {
				added = 1;
				addAjoinChan(ae, AJoinDefChans[i], NULL);
			}
		}
	}

	/* Send a message to the user, if he exists. 
	 * (Maybe another module regged a nick and send the event. */
	u = finduser(argv[0]);
	if (u && added)
		moduleNoticeLang(s_NickServ, u, LANG_AJOIN_DEFAULT_ENTRIES_ADDED);

	return MOD_CONT;
}

int client_connect(int argc, char **argv) {
	User *u;
	int i;

	if (argc != 1)
		return MOD_CONT;

	if (!AJoinUnRegistered || !AJoinDefChansNr || !AJoinDefChans || supported < 0)
		return MOD_CONT;

	/* Verify the user exists and isn't regged.. */
	if (!(u = finduser(argv[0])) || findnick(argv[0]))
		return MOD_CONT;

	for (i = 0; i < AJoinDefChansNr; i++)
		if (AJoinDefChans[i])
			anope_cmd_svsjoin(s_NickServ, u->nick, AJoinDefChans[i], NULL);

	moduleNoticeLang(s_NickServ, u, LANG_AJOIN_UNREGISTERED);

	return MOD_CONT;
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

static int ajoin_del_callback(User * u, int num, va_list args) {
	AjoinEntry *ae = va_arg(args, AjoinEntry *);
	AjoinChan *ac = findAjoinChanNr(ae, num);

	if (!ac)
		return 0;

	moduleNoticeLang(s_NickServ, u, LANG_CHAN_DELETED, ac->channel);
	deleteAjoinChan(ae, ac);

	return 1;
}

static int ajoin_list_callback(User * u, int num, va_list args) {
	AjoinEntry *ae = va_arg(args, AjoinEntry *);
	AjoinChan *ac = findAjoinChanNr(ae, num);

	if (!ac)
		return 0;

	moduleNoticeLang(s_NickServ, u, LANG_AJOIN_ENTRY, ac->nr, ac->channel,
			((ac->key) ? ac->key:""));

	return 1;
}



/* ------------------------------------------------------------------------------- */

/**
 * We add an AjoinChan to the list and update the nr of the channels.
 **/
static AjoinChan *addAjoinChan(AjoinEntry *ae, char *chan, char *key) {
	AjoinChan *ac = NULL, *current = NULL, *previous = NULL;

	if (!ae || !chan)
		return NULL;

	if ((ac = malloc(sizeof(AjoinChan))) == NULL) {
		fatal("Out Of Memory!");
	}

	for (current = ae->channels; current; current = current->next) {
		/* search for the position to insert the new channel */
		if (stricmp(chan, current->channel) > 0)
			previous = current;
		else
			break;
	}

	ac->next = current;
	ac->prev = previous;

	ac->channel = sstrdup(chan);
	if (key)
		ac->key = sstrdup(key);
	else
		ac->key = NULL;

	if (previous == NULL) {
		ae->channels = ac;
		ac->nr = 1;
	} else {
		previous->next = ac;
		ac->nr = previous->nr + 1;
	}

	if (current)
		current->prev = ac;

	while (current) {
		current->nr++;
		current = current->next;
	}

	ae->ajchannels++;

	return ac;
}


AjoinChan *findAjoinChan(AjoinEntry *ae, char *chan) {
	AjoinChan *current;

	if (!ae)
		return NULL;

	for (current = ae->channels; current; current = current->next) {
		if (!stricmp(chan, current->channel))
			return current;
	}

	return NULL;
}


AjoinChan *findAjoinChanNr(AjoinEntry *ae, int nr) {
	AjoinChan *current;

	if (!nr)
		return NULL;

	for (current = ae->channels; current; current = current->next) {
		if (current->nr == nr)
			return current;
	}

	return NULL;
}

/**
 * We delete an AjoinChan from the list, but we do not update the numbering.
 * This needs to be done later by calling numberAjoinChans() because numbers shouldn't
 * change while deleting ajoinchans by number.
 **/
static int deleteAjoinChan(AjoinEntry *ae, AjoinChan *ac) {
	if (!ae || !ac)
		return 0;

	if (ac->prev)
		ac->prev->next = ac->next;
	else
		ae->channels = ac->next;

	if (ac->next)
		ac->next->prev = ac->prev;

	if (ac->channel)
		free(ac->channel);
	if (ac->key)
		free(ac->key);

	free(ac);
	ae->ajchannels--;

	return 1;
}


static int clearAjoinChans(AjoinEntry *ae) {
	AjoinChan *ac = NULL, *next = NULL;

	if (!ae)
		return 0;

	for (ac = ae->channels; ac; ac = next) {
		if (ac->next)
			next = ac->next;
		else
			next = NULL;

		if (ac->channel)
			free(ac->channel);
		if (ac->key)
			free(ac->key);

		free(ac);
	}

	ae->channels = NULL;
	ae->ajchannels = 0;

	return 1;
}

static int numberAjoinChans(AjoinEntry *ae) {
	AjoinChan *current;
	int nr = 1;

	if (!ae)
		return 0;

	for (current = ae->channels; current; current = current->next) {
		current->nr  = nr++;
	}

	return 1;
}


/* ------------------------------------------------------------------------------- */

static AjoinEntry *createAjoinEntry(NickCore *nc) {
	AjoinEntry *ae = NULL;
	char buf[BUFSIZE];

	if (!nc)
		return NULL;

	if ((ae = malloc(sizeof(AjoinEntry))) == NULL) {
		fatal("Out Of Memory!");
	}

	ae->prev = NULL;
	ae->next = NULL;
	ae->row = counter;
	ae->ajchannels = 0;
	ae->in_use = 0;
	ae->flags = DefAjoinFlags;
	ae->channels = NULL;

	/* add it to the ajoinTable */
	if (ajoinTable[counter] == NULL) {
		ajoinTable[counter] = ae;
		ae->col = 0;
	} else {
		int i = 0, last = -1;
		AjoinEntry *t, *prev = NULL;

		for (t = ajoinTable[counter]; t; t = t->next) {
			if (t->col != last + 1)
				break;

			last = t->col;
			i++;
			prev = t;
		}
		ae->col = i;

		if (prev) {
			if (prev->next) {
				ae->next = prev->next;
				prev->next->prev = ae;
			}
			prev->next = ae;
		}
		ae->prev = prev;
	}

	/* add to the nc, so it can be addressed relative quickly */
	snprintf(buf, BUFSIZE, "%d;%d", ae->row, ae->col);
	moduleAddData(&nc->moduleData, ModDataKey, buf);

	if (counter < 1023)
		counter++;
	else
		counter = 0;

	return ae;
}

AjoinEntry *getAjoinEntry(NickCore *nc) {
	char *t, *x, *y;
	int row, col;
	AjoinEntry *ae = NULL;
	if (!nc)
		return NULL;

	t = moduleGetData(&nc->moduleData, ModDataKey);

	if (t == NULL) {
		return NULL;
	}
	x = myStrGetToken(t, ';', 0);
	y = myStrGetToken(t, ';', 1);

	row = atoi(x);
	col = atoi(y);

	for (ae = ajoinTable[row]; ae; ae = ae->next) {
		if (ae->col == col)
			break;
	}

	free(t);
	free(x);
	free(y);

	return ae;
}

static int deleteAjoinEntry(AjoinEntry *ae) {
	if (!ae)
		return 0;

	if (ajoinTable[ae->row] == ae)
		ajoinTable[ae->row] = ae->next;

	if (ae->prev)
		ae->prev->next = ae->next;

	if (ae->next)
		ae->next->prev = ae->prev;

	clearAjoinChans(ae);
	ae->prev = NULL;
	ae->next = NULL;

	free(ae);

	return 1;
}

/**
 * When we are called, we assume used entries have in_use set to 1 (done while the
 * DB is saved) and reset it to 0 for all other entries.
 * Entries with in_use to 0 are free()'d and removed from the AjoinTable since we
 * assume all other references to it have allready been removed.
 **/
void freeUnusedEntries() {
	AjoinEntry *next = NULL, *ae = NULL;
	int i;

	for (i = 0; i < 1024; i++) {
		for (ae = ajoinTable[i]; ae; ae = next) {
			if (ae->in_use == 1) {
				/* Entry is still being used... resetting */
				ae->in_use = 0;
				next = ae->next;
				continue;
			}

			/* Entry is no longer being used... free it */
			next = ae->next;
			deleteAjoinEntry(ae);
		}
	}
}


/* ------------------------------------------------------------------------------- */

/**
 * When anope saves her databases, we do the same.
 **/
int do_save(int argc, char **argv) {
	int old_supported = supported;
	
	if ((argc >= 1) && (!stricmp(argv[0], EVENT_STOP)))
		save_ajoin_db();

	check_modules();
	if (supported != old_supported) {
		if (supported == 0) {
			alog("[\002ns_ajoin\002] Warning: Module continueing in unsupported mode!");
		} else if (supported == -1) {
			alog("[\002ns_ajoin\002] Disabling module due to incompatibilities!");
			return MOD_STOP;
		}
	}
	update_version();

	return MOD_CONT;
}


/**
 * When anope backs her databases up, we do the same.
 **/
int db_backup(int argc, char **argv) {
	if ((argc >= 1) && (!stricmp(argv[0], EVENT_STOP))) {
		alog("[ns_ajoin] Backing up AJOIN database...");
		ModuleDatabaseBackup(AJoinDB);

		/* When anope backs her databases up, we also clear unused entries from the AjoinTable */
		freeUnusedEntries();
	}
	return MOD_CONT;
}


/**************************************************************************
 *                DataBase Handling
 **************************************************************************/

void load_ajoin_db(void) {
	DBFile *dbptr = scalloc(1, sizeof(DBFile));
	AjoinEntry *ae = NULL;
	NickCore *nc = NULL;
	char *key, *value;
	int retval = 0;

	fill_db_ptr(dbptr, 0, AJOINDBVERSION, s_NickServ, AJoinDB);

	/* let's remove existing temp files here, because we only load dbs on startup */
	remove(dbptr->temp_name);

	/* Open the db, fill the rest of dbptr and allocate memory for key and value */
	if (new_open_db_read(dbptr, &key, &value)) {
		free(dbptr);
		return;                 /* Bang, an error occurred */
	}

	while (1) {
		/* read a new entry and fill key and value with it -Certus */
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
			/* prevent chans from one block ending up in another... */
			ae = NULL;
		} else {		 /* DB_READ_SUCCESS */
			if (!*key || !*value)
				continue;

			/* nick */
			if (!stricmp(key, "ncd")) {
				if ((nc = findcore(value))) {
					ae = createAjoinEntry(nc);
				} else
					ae = NULL;

			/* flags */
			} else if (!stricmp(key, "flgs") && ae) {
				ae->flags = (uint16) atoi(value);

			/* channel entry */
			} else if (!stricmp(key, "che") && ae) {
				char *chan = myStrGetToken(value, ' ', 0);
				char *chkey = myStrGetToken(value, ' ', 1);

				addAjoinChan(ae, chan, chkey);

				free(chan);
				if (chkey)
					free(chkey);
			} else if (!stricmp(key, "AJOIN DB VERSION")) {
				if ((int)atoi(value) != AJOINDBVERSION) {
					alog("[\002ns_ajoin\002] Database version does not match any database versions supported by this module.");
					alog("[\002ns_ajoin\002] Continuing with clean database...");
					break;
				}
			}
		} 					/* else */
	}					/* while */

	free(dbptr);
}


void save_ajoin_db(void) {
	DBFile *dbptr = scalloc(1, sizeof(DBFile));
	NickCore *nc;
	int i;

	fill_db_ptr(dbptr, 0, AJOINDBVERSION, s_NickServ, AJoinDB);

	/* time to backup the old db */
	rename(AJoinDB, dbptr->temp_name);

	if (new_open_db_write(dbptr)) {
		rename(dbptr->temp_name, AJoinDB);
		free(dbptr);
		return;                /* Bang, an error occurred */
	}

	/* Store the version of the DB in the DB as well...
	 * This will make stuff a lot easier if the database scheme needs to modified. */
	new_write_db_entry("AJOIN DB VERSION", dbptr, "%d", AJOINDBVERSION);
	new_write_db_endofblock(dbptr);

	/* Go over the entire NickCore list and check whether each one has an ajoin entry */
	for (i = 0; i < 1024; i++) {
		for (nc = nclists[i]; nc; nc = nc->next) {
			AjoinEntry *ae = NULL;
			AjoinChan *ac = NULL;

			ae = getAjoinEntry(nc);
			if (!ae)
				continue;

			/* If there are no entries in the list and the configuration is set to the default
			 * value, there is no need to save it to the db. */
			if (ae->ajchannels == 0 && ae->flags == DefAjoinFlags) {
				/* Delete the entry first since it s just eating up space... */
				moduleDelData(&nc->moduleData, ModDataKey);
				deleteAjoinEntry(ae);
				continue;
			}

			new_write_db_entry("ncd", dbptr, "%s", nc->display);
			new_write_db_entry("flgs", dbptr, "%d", ae->flags);

			if (ae->ajchannels > 0) {
				for (ac = ae->channels; ac; ac = ac->next) {
					new_write_db_entry("che", dbptr, "%s %s", ac->channel, (ac->key ? ac->key : ""));
				}
			}

			new_write_db_endofblock(dbptr);

			/* Marking the entry as in use... */
			ae->in_use = 1;
		}
	}

	if (dbptr) {
		new_close_db(dbptr->fptr, NULL, NULL);  /* close file */
		remove(dbptr->temp_name);       /* saved successfully, no need to keep the old one */
		free(dbptr);           /* free the db struct */
	}
}

/* ------------------------------------------------------------------------------- */

/**
 * We check if the ircd is supported
 **/
int valid_ircd(void) {
	if (!stricmp(IRCDModule, "unreal31"))
		return 1;

	if (!stricmp(IRCDModule, "unreal32"))
		return 1;

	if (!stricmp(IRCDModule, "viagra"))
		return 1;

	if (!stricmp(IRCDModule, "ptlink"))
		return 1;

	if (!stricmp(IRCDModule, "ultimate2"))
		return 1;

	if (!stricmp(IRCDModule, "plexus3"))
		return 1;

	if (!stricmp(IRCDModule, "inspircd10"))
		return 1;

	if (!stricmp(IRCDModule, "inspircd11"))
		return 1;

	return 0;
}


/**
 * Checks whether the right conditions to continue loading are met.
 **/
int check_modules(void) {
#ifdef SUPPORTED
	if (supported >= 0) {
		if (findModule("os_raw")) {
			alog("[\002ns_ajoin\002] Unsupported module found: os_raw.. (This is fatal!)");
			supported = -1;
		}

		if (!DisableRaw) {
			alog("[\002ns_ajoin\002] RAW has NOT been disabled! (This is fatal!)");
			supported = -1;
		}
	}

	if (supported) {
		if (findModule("ircd_init")) {
			alog("[\002ns_ajoin\002] This module is unsupported in combination with ircd_init.");
			supported = 0;
		}

		if (findModule("cs_join")) {
			alog("[\002ns_ajoin\002] This module is unsupported in combination with cs_join.");
			supported = 0;
		}

		if (findModule("bs_logchanmon")) {
			alog("[\002ns_ajoin\002] This module is unsupported in combination with bs_logchanmon.");
			supported = 0;
		}

		if (findModule("ircd_gameserv")) {
			alog("[\002ns_ajoin\002] This module is unsupported in combination with ircd_gameserv.");
			supported = 0;
		}
	}
#endif
	return supported;
}

/* ------------------------------------------------------------------------------- */

/**
 * Returns pointer to a string of flags.. Must be free'd!!
 **/
char* get_flags() {
	char tmp[BUFSIZE];
	const char version_flags[] = " " VER_DEBUG VER_OS VER_MYSQL VER_MODULE;
	char *flags;
	
#ifdef SUPPORTED
	snprintf(tmp, BUFSIZE, "%s-%s-ADC%d-FU%d", version_flags, ((supported == -1) ? "U" : 
			(supported == 0) ? "u" : "S"), AJoinDefChansNr, AJoinUnRegistered);
#else
	snprintf(tmp, BUFSIZE, "%s-HU-ADC%d-FU%d", version_flags, AJoinDefChansNr, AJoinUnRegistered);
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
		m = findModule("ns_ajoin");

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
	char *defajoinchans = NULL;

	Directive confvalues[][1] = {
		{{"AJoinDB", {{PARAM_STRING, PARAM_RELOAD, &AJoinDB}}}},
		{{"AJoinDefChans", {{PARAM_STRING, PARAM_RELOAD, &defajoinchans}}}},
		{{"AJoinUnRegistered", {{PARAM_SET, PARAM_RELOAD, &AJoinUnRegistered}}}},
	};

	if (AJoinDB)
		free(AJoinDB);
	AJoinDB = NULL;
	for ( i = 0; i < AJoinDefChansNr; i++)
		if (AJoinDefChans[i])
			free(AJoinDefChans[i]);
	if (AJoinDefChans)
		free(AJoinDefChans);
	AJoinDefChans = NULL;
	AJoinDefChansNr = 0;
	AJoinUnRegistered = 0;

	for (i = 0; i < 3; i++)
		moduleGetConfigDirective(confvalues[i]);

	if (!AJoinDB)
		AJoinDB = sstrdup(DefAJoinDB);

	if ((defajoinchans)) {
		ChannelInfo *ci;

		AJoinDefChans = buildStringList(defajoinchans, &AJoinDefChansNr);

		for (i = 0; i < AJoinDefChansNr; i++) {
			if (!AJoinDefChans[i])
				continue;

			if (*AJoinDefChans[i] != '#' || !anope_valid_chan(AJoinDefChans[i]) ||
					!(ci = cs_findchan(AJoinDefChans[i])) || 
					ci->flags & CI_VERBOTEN || ci->flags & CI_SUSPENDED ) {
				alog("[ns_ajoin] %s could not be added to the default ajoin list.", AJoinDefChans[i]);
				free(AJoinDefChans[i]);
				AJoinDefChans[i] = NULL;
			} else if (debug)
				alog("[ns_ajoin] debug: Added '%s' to the default AJOIN list.", AJoinDefChans[i]);
		}
		
		free(defajoinchans);
	}

	if (debug)
		alog ("[ns_ajoin] debug: AJoinDB set to %s. AJoinUnRegistered is %s.", 
				AJoinDB, (AJoinUnRegistered ? "set" : "not set"));
}


/**
 * Upon /os reload call the routines for reloading the configuration directives
 **/
int reload_config(int argc, char **argv) {
	int old_supported = supported;

	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			if (debug)
				alog("[ns_ajoin] debug: Reloading configuration directives...");
			load_config();
		}
	}

	check_modules();
	if (supported != old_supported) {
		if (supported == 0) {
			alog("[\002ns_ajoin\002] Warning: Module continueing in unsupported mode!");
		} else if (supported == -1) {
			alog("[\002ns_ajoin\002] Disabling module due to incompatibilities!");
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
		/* LANG_AJOIN_DESC */
		" AJOIN       Manipulate the AutoJoin list.",
		/* LANG_AJOIN_SYNTAX */
		" Syntax: AJOIN { ADD | DEL | LIST | CLEAR } [\037channel\037 | \037entry-list\037] [\037key\037]",
		/* LANG_AJOIN_SYNTAX_EXT */
		" Syntax: \002AJOIN ADD \037channel\037 \037key\037\002 \n"
		"         \002AJOIN DEL {\037channel\037 | \037entry-nr\037 | \037list\037}\002\n"
		"         \002AJOIN LIST [\037mask\037 | \037list\037]\002 \n"
		"         \002AJOIN CLEAR\002 \n"
		" \n"
		" Maintains the \002AutoJoin list\002 for nick group. \n"
		" If a user identifies to his nickname, he will \n"
		" automatically join the listed channels. \n"
		" \n"
		" The \002AJOIN ADD\002 command adds the given channel\n"
		" to the AutoJoin list or updates the existing entry, \n"
		" if it is allready present on the AutoJoin list. \n"
		" If no key is given and an entry for the channel is already\n"
		" present, the key allready set (if any) will be unset. \n"
		" Only \002registered channels\002 are accepted! \n"
		" \n"
		" The \002AJOIN DEL\002 command removes the given channel \n"
		" from the AutoJoin list. If a list of entry numbers is given,\n"
		" those entries are deleted.\n"
		" \n"
		" The \002AJOIN LIST\002 command displays the AutoJoin list.\n"
		" If a wildcard mask is given, only those entries matching the\n"
		" mask are displayed. If a list of entry numbers is given, only\n"
		" those entries are shown.\n"
		" \n"
		" The \002AJOIN CLEAR\002 command clears all entries on the \n"
		" AutoJoin list.",
		/* LANG_AJOIN_DISABLED */
		" Sorry, AutoJoin list modification is temporarily disabled.",
		/* LANG_NO_LOCAL_CHAN */
		" Cannot add local channels to ajoin list.",
		/* LANG_CHAN_SYMB_REQUIRED */
		" The channel symbol '\002#\002' is required.",
		/* LANG_CHAN_UPDATED */
		" Ajoin entry updated.",
		/* LANG_AJOIN_LIST_FULL */
		" The AutoJoin list is full.",
		/* LANG_CHAN_ADDED */
		" \002%s\002 has been added to the ajoin list.",
		/* LANG_NO_AJOINS */
		" No AutoJoins have been set.",
		/* LANG_AJOIN_LIST_EMPTY */
		" The AutoJoins list is empty.",
		/* LANG_NO_ENTRY */
		" \002%s\002 is not in the AutoJoin list.",
		/* LANG_CHAN_DELETED */
		" \002%s\002 has been deleted from the AutoJoin list.",
		/* LANG_AJOIN_ENTRY */
		" %d - Channel: %s  -  Key: %s",
		/* LANG_AJOIN_ENTRIES */
		" Found %d entries",
		/* LANG_AJOIN_LIST_CLEARED */
		" AJOIN list cleared.",
		/* LANG_UNKWN_AJOIN_OPTION */
		" Unknown AJOIN option.",
		/* LANG_AJOINING */
		" Automatically joining %s .",
		/* LANG_AJOINING_FAILED */
		" Could not automatically join %s .",
		/* LANG_SET_AJOIN_DESC */
		" \n"
		" Commands Provided by module ns_ajoin: \n"
		" AJOIN      Change your autojoin settings.",
		/* LANG_SET_AJOIN_SYNTAX */
		" Syntax: SET AJOIN { ON | OFF | SILENT }",
		/* LANG_SET_AJOIN_SYNTAX_EXT */
		" Syntax: \002SET AJOIN { ON | OFF | SILENT }\002 \n"
		" \n"
		" Toggles whether or not your AutoJoin list is performed when \n"
		" you IDENTIFY or UPDATE.\n"
		" When set to ON, you will receive a notification for each channel\n"
		" you are automatically joined to. When set to SILENT, you will receive\n"
		" one message indicating you are automatically joining channels. \n"
		" When set to OFF, you will not join any channels upon identifying.\n"
		" \n"
		" The default setting is \002ON\002.",
		/* LANG_SET_AJOIN_ON */
		" AutoJoin is now \002ON\002.",
		/* LANG_SET_AJOIN_SILENT */
		" AutoJoin is now \002ON\002 and set to \002SILENT\002 mode.",
		/* LANG_SET_AJOIN_OFF */
		" AutoJoin is now \002OFF\002.",
		/* LANG_AJOINING_SUM_SUCCESS */
		" Successfully joined %d channel(s).",
		/* LANG_AJOINING_SUM_FAILED */
		" Failed joining %d channel(s).",
		/* LANG_AJOIN_DELETED_NR_1 */
		" Deleted %d entry from the AutoJoin list.",
		/* LANG_AJOIN_DELETED_NR */
		" Deleted %d entries from the AutoJoin list.",
		/* LANG_AJOIN_DEFAULT_ENTRIES_ADDED */
		" Default AJOIN channels have been added to your auto-join list.\n"
		" Use \002AJOIN LIST\002 to get a list of all channels currently on your\n"
		" autojoin list. ",
		/* LANG_AJOIN_UNREGISTERED */
		" You are being auto-joined to this networks default autojoin channels.\n"
		" If you want to avoid this, register your nickname and delete these channels\n"
		" from your autojoin list.",
		/* LANG_AJOIN_NOT_AVAILABLE */
		" Command is currently disabled.",
	};

	char *langtable_nl[] = {
		/* LANG_AJOIN_DESC */
		" AJOIN       Beheer de autojoin lijst.",
		/* LANG_AJOIN_SYNTAX */
		" Gebruik: AJOIN {ADD | DEL | LIST | CLEAR} [\037kanaal\037 | \037lijst\037] [\037wachtwoord\037]",
		/* LANG_AJOIN_SYNTAX_EXT */
		" Gebruik: \002AJOIN ADD \037kanaal\037 \037wachtwoord\037\002 \n"
		"          \002AJOIN DEL {\037kanaal\037 | \037nummer\037 | \037lijst\037}\002 \n"
		"          \002AJOIN LIST [\037mask\037 | \037lijst\037]\002 \n"
		"          \002AJOIN CLEAR\002 \n"
		" \n"
		" Onderhoud de \002AutoJoin lijst\002 voor een nick groep. \n"
		" Als een gebruiker zich voor zijn nickname identificeerd,\n"
		" zal hij automatisch de kanalen in de lijst joinen.\n"
		" \n"
		" Het \002AJOIN ADD\002 commando voegt het gegeven kanaal toe \n"
		" aan de AutoJoin lijst of werkt de bestaande entry bij, \n"
		" indien het kanaal reeds aanwezig is op de AutoJoin lijst. \n"
		" Als er geen wachtwoord is opgegeven en het kanaal reeds \n"
		" aanwezig is op de lijst, zal het wachtwoord dat reeds is \n"
		" ingesteld (indien aanwezig) verwijderd worden. \n"
		" Enkel \002geregistreerde kanalen\002 worden aanvaard!. \n"
		" \n"
		" Het \002AJOIN DEL\002 commando verwiderd het gegeven kanaal \n"
		" van de AutoJoin lijst. Als een lijst met entry-nummers is\n"
		" gegeven, worden deze verwijderd.\n"
		" \n"
		" Het \002AJOIN LIST\002 commando geeft de AutoJoin lijst weer.\n"
		" Als een wildcard mask is opgegeven worden alleen de overeenkomsten\n"
		" weergegeven. Als een lijst van entry-nummers in gegeven wordt,\n"
		" worden alleen die weergegeven\n"
		" \n"
		" Het \002AJOIN CLEAR\002 commando maakt de AutoJoin lijst leeg.",
		/* LANG_AJOIN_DISABLED */
		" Sorry,het wijzigen van de AutoJoin lijst is tijdelijk uitgeschakeld.",
		/* LANG_NO_LOCAL_CHAN */
		" Kan geen lokale kanalen  toevoegen aan de ajoin list.",
		/* LANG_CHAN_SYMB_REQUIRED */
		" Het kanaal symbool '\002#\002' is vereist.",
		/* LANG_CHAN_UPDATED */
		" AutoJoin entry bijgewerkt.",
		/* LANG_AJOIN_LIST_FULL */
		" De AutoJoin lijst is vol.",
		/* LANG_CHAN_ADDED */
		" \002%s\002 is aan de autojoin lijst toegevoegd.",
		/* LANG_NO_AJOINS */
		" Er zijn geen AutoJoins ingesteld.",
		/* LANG_AJOIN_LIST_EMPTY */
		" De AutoJoin lijst is leeg.",
		/* LANG_NO_ENTRY */
		" \002%s\002 bestaat niet in de AutoJoin lijst.",
		/* LANG_CHAN_DELETED */
		" \002%s\002 is van de AutoJoin lijst verwijderd.",
		/* LANG_AJOIN_ENTRY */
		" %d - Kanaal: %s  -  Wachtwoord: %s",
		/* LANG_AJOIN_ENTRIES */
		" %d plaatsen gevonden.",
		/* LANG_AJOIN_LIST_CLEARED */
		" AutoJoin lijst is leeggemaakt.",
		/* LANG_UNKWN_AJOIN_OPTION */
		" Onbekende AJOIN parameter.",
		/* LANG_AJOINING */
		" Automatisch %s joinen",
		/* LANG_AJOINING_FAILED */
		" Kon %s niet automatisch joinen.",
		/* LANG_SET_AJOIN_DESC */
		" \n"
		" Commandos toegevoegd door module ns_ajoin: \n"
		" AJOIN      Wijzig de instellingen van je autojoin.",
		/* LANG_SET_AJOIN_SYNTAX */
		" Gebruik: SET AJOIN { ON | OFF | SILENT }",
		/* LANG_SET_AJOIN_SYNTAX_EXT */
		" Gebruik: \002SET AJOIN { ON | OFF | SILENT }\002 \n"
		" \n"
		" Configureert of je AutoJoin lijst al dan niet wordt uitgevoerd bij \n"
		" een nickserv IDENTIFY of UPDATE.\n"
		" Wanneer deze optie op ON staat zal je een melding krijgen voor elk kanaal\n"
		" dat je automatisch joint. Wanneer het op SILENT staat, krijg je ййn melding\n"
		" die aangeeft dat je automatisch kanalen joint. \n"
		" Wanneer deze optie op OFF staat zal je niet automatisch kanalen joinen.\n"
		" \n"
		" De standaard waarde is \002ON\002.",
		/* LANG_SET_AJOIN_ON */
		" AutoJoin staat nu \002OAAN\002.",
		/* LANG_SET_AJOIN_SILENT */
		" AutoJoin staat nu \002AAN\002 in \002STILLE\002 modus.",
		/* LANG_SET_AJOIN_OFF */
		" AutoJoin staat nu \002UIT\002.",
		/* LANG_AJOINING_SUM_SUCCESS */
		" Succesvol %d kanaal/kanalen gejoint.",
		/* LANG_AJOINING_SUM_FAILED */
		" Kon %d kanaal/kanalen niet joinen.",
		/* LANG_AJOIN_DELETED_NR_1 */
		" %d kanaal uit de AutoJoin lijst verwijderd.",
		/* LANG_AJOIN_DELETED_NR */
		" %d kanalen uit de AutoJoin lijst verwijderd.",
		/* LANG_AJOIN_DEFAULT_ENTRIES_ADDED */
		" Standaard AJOIN kanalen zijn aan je auto-join lijst toegevoegd.\n"
		" Gebruik \002AJOIN LIST\002 om een lijst te krijgen van alle kanalen die\n"
		" momenteel op je autojoin lijst staan. ", 
		/* LANG_AJOIN_UNREGISTERED */
		" Je wordt automatisch in enkele standaard kanalen van dit netwerk geplaatst.\n"
		" Indien je dit wilt vermijden in de toekomst registreer dan je nicknaam en \n"
		" verwijder deze kanalen van je autojoin lijst.",
		/* LANG_AJOIN_NOT_AVAILABLE */
		" Dit Commando is momenteel niet beschikbaar.",
	};

	char *langtable_de[] = {
		/* LANG_AJOIN_DESC */
		" AJOIN       AutoJoin-Liste bearbeiten.",
		/* LANG_AJOIN_SYNTAX */
		" Syntax: AJOIN { ADD | DEL | LIST | CLEAR } [\037channel\037 | \037entry-list\037] [\037key\037]",
		/* LANG_AJOIN_SYNTAX_EXT */
		" Syntax: \002AJOIN ADD \037channel\037 \037key\037\002 \n"
		"         \002AJOIN DEL {\037channel\037 | \037entry-nr\037 | \037list\037}\002 \n"
		"         \002AJOIN LIST [\037mask\037 | \037list\037]\002 \n"
		"         \002AJOIN CLEAR\002 \n"
		" \n"
		" Verwaltet die \002AutoJoin Liste\002 fьr eine Nick-Gruppe. \n"
		" Wenn ein user sich mit seinem Nicknamen identifiziert, so \n"
		" betritt er automatisch die Rдume aus der Liste. \n"
		" \n"
		" \002AJOIN ADD\002 fьgt den angegebenen Raum \n"
		" der AutoJoin-Liste hinzu oder aktualisiert den \n"
		" Eintrag, wenn der angegebene Raum schon vorhanden ist. \n"
		" Wenn kein Raum-Schlьssel angegeben wird und es bereits \n"
		" einen Listeneintrag fьr diesen Raum gibt, wo ein Schlьssel \n"
		" angegeben wurde, dann wird dieser Schlьssel zurьckgesetzt. \n"
		" Nur \002registrierte Rдume\002 sind erlaubt! \n"
		" \n"
		" \002AJOIN DEL\002 entfernt den angegebenen Raum \n"
		" aus der AutoJoin-Liste. Wurden die Nummern der Eintrдge angegeben,\n"
		" werden die dazugehцrigen Eintrдge gelцscht.\n"
		" \n"
		" \002AJOIN LIST\002 zeigt die AutoJoin-liste.\n"
		" If a wildcard mask is given, only those entries matching the\n"
		" mask are displayed. Wurden die Nummern der Listeneintrдge angegeben,\n"
		" werden nur diese Eintrдge angezeigt.\n"
		" \n"
		" \002AJOIN CLEAR\002 lцscht alle Eintrдge aus der \n"
		" AutoJoin-Liste.",
		/* LANG_AJOIN_DISABLED */
		" Sorry, das Bearbeiten der AutoJoin-Liste ist vorrьbergehend deaktiviert.",
		/* LANG_NO_LOCAL_CHAN */
		" Kann keine lokalen Rдume zur AutoJoin-Liste hinzufьgen.",
		/* LANG_CHAN_SYMB_REQUIRED */
		" Das Raumsymbol '\002#\002' wird benцtigt.",
		/* LANG_CHAN_UPDATED */
		" AutoJoin Eintrag aktualisiert.",
		/* LANG_AJOIN_LIST_FULL */
		" Die AutoJoin-Liste ist voll.",
		/* LANG_CHAN_ADDED */
		" \002%s\002 wurde zur AutoJoin-Liste hinzugefьgt.",
		/* LANG_NO_AJOINS */
		" Es wurden noch keine AutoJoins festgelegt.",
		/* LANG_AJOIN_LIST_EMPTY */
		" Die AutoJoin-Liste ist leer.",
		/* LANG_NO_ENTRY */
		" \002%s\002 ist nicht in der AutoJoin-liste.",
		/* LANG_CHAN_DELETED */
		" \002%s\002 wurde aus der AutoJoin-Liste gelцscht.",
		/* LANG_AJOIN_ENTRY */
		" %d - Channel: %s  -  Key: %s",
		/* LANG_AJOIN_ENTRIES */
		" Habe %d Eintrдge gefunden ",
		/* LANG_AJOIN_LIST_CLEARED */
		" AutoJoin-Liste gesдubert.",
		/* LANG_UNKWN_AJOIN_OPTION */
		" Unbekannte AutoJoin-Option.",
		/* LANG_AJOINING */
		" AutoJoining %s .",
		/* LANG_AJOINING_FAILED */
		" Konnte %s nicht AutoJoinen .",
		/* LANG_SET_AJOIN_DESC */
		" \n"
		" Unterstьtzte Kommandos von Modul ns_ajoin: \n"
		" AJOIN      AutoJoin-Settings дndern.",
		/* LANG_SET_AJOIN_SYNTAX */
		" Syntax: SET AJOIN { ON | OFF | SILENT }",
		/* LANG_SET_AJOIN_SYNTAX_EXT */
		" Syntax: \002SET AJOIN { ON | OFF | SILENT }\002 \n"
		" \n"
		" Stellt ein, ob die AutoJoin-Liste abgearbeitet wird, wenn \n"
		" IDENTIFY oder UPDATE ausgefьhrt wird.\n"
		" Wenn ON eingestellt wird, bekommst du fьr jeden Raum, den du AutoJoinst \n"
		" eine Nachricht. Bei Einstellung SILENT, bekommst du \n"
		" eine Nachricht, welche dir mitteilt, dass du AutoJoinst. \n"
		" Die Einstellung OFF bewirkt, dass du keinen Raum beim IDENTIFY Befehl betrittst. \n"
		" \n"
		" Voreinstellung ist: \002ON\002.",
		/* LANG_SET_AJOIN_ON */
		" AutoJoin ist jetzt \002ON\002.",
		/* LANG_SET_AJOIN_SILENT */
		" AutoJoin ist jetzt \002ON\002 und auf \002SILENT\002 eingestellt.",
		/* LANG_SET_AJOIN_OFF */
		" AutoJoin ist jetzt \002OFF\002.",
		/* LANG_AJOINING_SUM_SUCCESS */
		" Erfolgreich %d Channel(s) automatisch betreten.",
		/* LANG_AJOINING_SUM_FAILED */
		" Fehler beim AutoJoinen von %d channel(s).",
		/* LANG_AJOIN_DELETED_NR_1 */
		" %d Eintrag wurde aus der AutoJoin-Liste entfernt.",
		/* LANG_AJOIN_DELETED_NR */
		" %d Eintrдge wurden aus der AutoJoin-Liste entfernt.",
		/* LANG_AJOIN_DEFAULT_ENTRIES_ADDED */
		" Deine Standard AJOIN-Channels wurden zu Deiner auto-join Liste geadded.\n"
		" Benutze \002AJOIN LIST\002 um eine Liste all Deiner Channels der\n"
		" Autojoinliste zu bekommen. ", 
		/* LANG_AJOIN_UNREGISTERED */
		" Du wurdest in alle Auto-join Channels dieses Networks gejoint.\n"
		" Wenn Du das vermeiden mцchtest, registriere Deinen Nick und lцsch die\n"
		" Chans aus der Autojoinliste.",
		/* LANG_AJOIN_NOT_AVAILABLE */
		" Dieser Befehl ist zur Zeit nicht nutzbar.",
	};

	char *langtable_tr[] = {
		/* LANG_AJOIN_DESC */
		" AJOIN       Otojoin listesinizi yцnetebilirsiniz.",
		/* LANG_AJOIN_SYNTAX */
		" Kullanэmэ: AJOIN { ADD | DEL | LIST | CLEAR } [\037#kanaladi\037 | \037entry-list\037] [\037anahtar\037]",
		/* LANG_AJOIN_SYNTAX_EXT */
		" Kullanэmэ: \002AJOIN ADD \037#kanaladi\037 \037anahtar\037\002 \n"
		"            \002AJOIN DEL {\037#kanaladi\037 | \037entry-nr\037 | \037list\037}\002 \n"
		"            \002AJOIN LIST [\037mask\037 | \037list\037]\002 \n"
		"            \002AJOIN CLEAR\002 \n"
		" \n"
		" Nick grubunuzun \002otojoin list\002 dьzenini gцrьntьler. \n"
		" Nickinizi identify ettikten sonra, otomatik \n"
		" olarak eklediрiniz kanallara giriю yaparsэnэz. \n"
		" \n"
		" Otojoin listeniz hazэrda var ise \002AJOIN ADD\002 komutu\n"
		" ile kanal ekleyebilir veya otojoin  \n"
		" listesini dьzenlersiniz. \n"
		" Eрer kanal eklerken anahtar belirtilmediyse ve kanalda \n"
		" anahtar var ise, kanala girmek iзin anahtar olmucak.\n"
		" Sadece \002kayэtlэ kanal\002 kabul edilir! \n"
		" \n"
		" \002AJOIN DEL\002 komutu eklediрiniz kanalэ otojoin \n"
		" listesinden зэkartэr. Eрerki listede ki giriю numaralarэ verilirse,\n"
		" bu giriюler silinir.\n"
		" \n"
		" \002AJOIN LIST\002 komutu otojoin listenizi gцrьntьler.\n"
		" Eрerki mask belirtilirse, sadece belirtrilen bu masklar\n"
		" gцrьntьlenir. Eрerki sadece listede ki giriю numaralarэ belirtilirse, sadece\n"
		" bunlar gцzьkьr.\n"
		" \n"
		" \002AJOIN CLEAR\002 komutu bьtьn ekli olan kanallarэnэzэ \n"
		" otojoin listenizden siler.",
		/* LANG_AJOIN_DISABLED */
		" Ьzgьnьm, ajoin komutu kullanэm dэюэ bэrakэlmэюtэr.",
		/* LANG_NO_LOCAL_CHAN */
		" Lokal kanal ekleyemezsiniz.",
		/* LANG_CHAN_SYMB_REQUIRED */
		" '\002#\002' iюaretini kulanmanэz gerekmektedir.",
		/* LANG_CHAN_UPDATED */
		" Otojoin giriюleriniz gьncellendi.",
		/* LANG_AJOIN_LIST_FULL */
		" Otojoin listiniz dolmuюtur.",
		/* LANG_CHAN_ADDED */
		" \002%s\002 kanali otojoin listenize eklendi.",
		/* LANG_NO_AJOINS */
		" Herhangi bir otojoin kullanэlmadэ.",
		/* LANG_AJOIN_LIST_EMPTY */
		" Otojoin listiniz boю.",
		/* LANG_NO_ENTRY */
		" \002%s\002 kanali otojoin listenizde deрildir.",
		/* LANG_CHAN_DELETED */
		" \002%s\002 kanali otojoin listenizden silindi.",
		/* LANG_AJOIN_ENTRY */
		" %d - Kanaladi: %s  -  Anahtar: %s",
		/* LANG_AJOIN_ENTRIES */
		" %d giriю bulundu",
		/* LANG_AJOIN_LIST_CLEARED */
		" Otojoin listeniz temizlendi.",
		/* LANG_UNKWN_AJOIN_OPTION */
		" Bilinmeyen otojoin komutu.",
		/* LANG_AJOINING */
		" %s kanalэna otomatik olarak giriю yaptэnэz .",
		/* LANG_AJOINING_FAILED */
		" %s kanalэna otomatik giriю yapamadэnэz .",
		/* LANG_SET_AJOIN_DESC */
		" \n"
		" ns_ajoin modulu hakkэnda: \n"
		" AJOIN      Otojoin ayarlarэnэzэ dьzenler.",
		/* LANG_SET_AJOIN_SYNTAX */
		" Kullanэmэ: SET AJOIN { ON | OFF | SILENT }",
		/* LANG_SET_AJOIN_SYNTAX_EXT */
		" Kullanэmэ: \002SET AJOIN { ON | OFF | SILENT }\002 \n"
		" \n"
		" Otojoin listiniz hakkэnda IDENTIFY ve UPDATE edildiginde \n"
		" geniю bilgi sunar.\n"
		" ON olduрunda otojoin listesinizdeki kanallara giriю\n"
		" yapmanэzэ saрlar. Bu ayar SILENT olduрunda, sadece bir tane\n"
		" otomatik giriю yaptэрэnэz kanallardan giriю mesajэnэz alэrsэnэz. \n"
		" Bu ayar OFF olduрunda, hiзbir kanala otojoin yapamazsэnэz.\n"
		" \n"
		" Belirlenmiю ayar \002ON\002.",
		/* LANG_SET_AJOIN_ON */
		" Юuan otojoin \002ON\002.",
		/* LANG_SET_AJOIN_SILENT */
		" Otojoin юuan \002ON\002 ve \002SILENT\002 modundadэr.",
		/* LANG_SET_AJOIN_OFF */
		" Otojoin юuan \002OFF\002.",
		/* LANG_AJOINING_SUM_SUCCESS */
		" %d kanallarэna baюarэlэ юekilde giriю yaptэnэz.",
		/* LANG_AJOINING_SUM_FAILED */
		" %d kanallarэna giriю yapamadэnэz.",
		/* LANG_AJOIN_DELETED_NR_1 */
		" %d giriю otojoin listesinden silindi.",
		/* LANG_AJOIN_DELETED_NR */
		" %d giriюler otojoin listesinden silindi.",
		/* LANG_AJOIN_DEFAULT_ENTRIES_ADDED */
		" Default AJOIN channels have been added to your auto-join list.\n"
		" Use \002AJOIN LIST\002 to get a list of all channels currently on your\n"
		" autojoin list. ", 
		/* LANG_AJOIN_UNREGISTERED */
		" You are being auto-joined to this networks default ajoin channels.\n"
		" If you want to avoid this, register your nickname and delete these channels\n"
		" from your autojoin list.",
		/* LANG_AJOIN_NOT_AVAILABLE */
		" Command is currently disabled.",
	};

	char *langtable_ru[] = {
		/* LANG_AJOIN_DESC */
		"    AJOIN      Управляет AJOIN-списком (автозаход).",
		/* LANG_AJOIN_SYNTAX */
		"Синтаксис: AJOIN { ADD | DEL | LIST | CLEAR } [\037#канал\037 | \037список_записей\037] [\037ключ\037]",
		/* LANG_AJOIN_SYNTAX_EXT */
		"Синтаксис: \002AJOIN ADD \037#канал\037 \037ключ\037\002 \n"
		"           \002AJOIN DEL {\037#канал\037 | \037номер_записи\037 | \037список_записей\037}\002\n"
		"           \002AJOIN LIST [\037маска\037 | \037список\037]\002 \n"
		"           \002AJOIN CLEAR\002 \n"
		" \n"
		"Позволяет управлять списком \002автозахода\002 на каналы. \n"
		"Как только пользователь идентифицируется к своему нику, он будет \n"
		"автоматически присоединен к указанным в списке каналам. \n"
		" \n"
		"Команда \002AJOIN ADD\002 позволяет добавить канал в список \n"
		"автозахода или обновить информацию для уже существующего в \n"
		"в списке канала. \n"
		"Если при обновлении записи для какого либо канала, вы не укажете \n"
		"дополнительный параметр 'ключ' - текущий установленный для канала \n"
		"ключ будет удален (при условии что он стоял, конечно) \n"
		"Список автозахода может содержать только \002зарегистрированные каналы\002 \n"
		" \n"
		"Команда \002AJOIN DEL\002 позволяет удалить указанный канал из \n"
		"списка автозахода. В качестве параметра вы можете указать список \n"
		"записей или конкретный номер записи ajoin-списка \n"
		" \n"
		"Команда \002AJOIN LIST\002 отображает текущий список автозахода.\n"
		"Допустимо использование подстановочных символов (глобальных масок)\n"
		"в качестве дополнительного параметра. Так же, в качестве уловия листинга\n"
		"каналов, вы можете указать список записей.\n"
		" \n"
		"Команда \002AJOIN CLEAR\002 полностью удаляет \002все\002 записи из \n"
		"списка автозахода.",
		/* LANG_AJOIN_DISABLED */
		"Извините, но модификация списка автозахода временно недоступна.",
		/* LANG_NO_LOCAL_CHAN */
		"Вы не можете добавить локальные каналы в список автозахода.",
		/* LANG_CHAN_SYMB_REQUIRED */
		"Вы должны указать символ канала: '\002#\002' .",
		/* LANG_CHAN_UPDATED */
		"Обновлена одна запись в списке автозахода.",
		/* LANG_AJOIN_LIST_FULL */
		"Список автозахода переполнен.",
		/* LANG_CHAN_ADDED */
		"Канал \002%s\002 успешно добавлен в список автозахода.",
		/* LANG_NO_AJOINS */
		"Список автозахода для вашего ника не настроен.",
		/* LANG_AJOIN_LIST_EMPTY */
		"Список автозахода пуст.",
		/* LANG_NO_ENTRY */
		"Канал \002%s\002 не присутствует в списке автозахода.",
		/* LANG_CHAN_DELETED */
		"Канал \002%s\002 успешно удален из списка автозахода.",
		/* LANG_AJOIN_ENTRY */
		" %d - Канал: %s  -  Ключ: %s",
		/* LANG_AJOIN_ENTRIES */
		"Всего записей: %d",
		/* LANG_AJOIN_LIST_CLEARED */
		"Список автозахода полностью очищен.",
		/* LANG_UNKWN_AJOIN_OPTION */
		"Неизвестная AJOIN-опция.",
		/* LANG_AJOINING */
		"Успешный автозаход на %s .",
		/* LANG_AJOINING_FAILED */
		"Неудачный автозаход на %s .",
		/* LANG_SET_AJOIN_DESC */
		" \n"
		"Команды, предоставляемые модулем ns_ajoin: \n"
		"    AJOIN      Позволяет настроить режим автозахода.",
		/* LANG_SET_AJOIN_SYNTAX */
		"Синтаксис: SET AJOIN { ON | OFF | SILENT }",
		/* LANG_SET_AJOIN_SYNTAX_EXT */
		"Синтаксис: \002SET AJOIN { ON | OFF | SILENT }\002 \n"
		" \n"
		"Позволяет включить или выключить автозаход на каналы как только \n"
		"вы идентифицируетесь к нику или используете команду UPDATE. \n"
		"Установка в ON активирует автоматическое уведомление об автозаходе на \n"
		"каждый канал из вашего списка автозахода. Установка в SILENT активирует \n"
		"лишь единственное уведомление о том, к какому количеству каналов из вашего \n"
		"списка автозахода вы успешно/неуспешно присоединены. \n"
		"Установка опции в OFF отключит возможность автозахода вообще. \n"
		" \n"
		"Установка по-умолчанию: \002ON\002.",
		/* LANG_SET_AJOIN_ON */
		"Режим автозахода \002ВКЛЮЧЕН\002.",
		/* LANG_SET_AJOIN_SILENT */
		"Режим автозахода \002ВКЛЮЧЕН\002, будет использован \002SILENT\002-режим (минимум уведомлений).",
		/* LANG_SET_AJOIN_OFF */
		"Режим автозахода \002ВЫКЛЮЧЕН\002.",
		/* LANG_AJOINING_SUM_SUCCESS */
		"Успешный автозаход на %d канал(а/ов).",
		/* LANG_AJOINING_SUM_FAILED */
		"Неудачный автозаход на %d канал(а/ов).",
		/* LANG_AJOIN_DELETED_NR_1 */
		"Запись под номером %d удалена из списка автозахода.",
		/* LANG_AJOIN_DELETED_NR */
		"Из списка автозахода успешно удалены %d записей.",
		/* LANG_AJOIN_DEFAULT_ENTRIES_ADDED */
		"В ваш личный список автозахода автоматически добавлены общесетевые каналы\n"
		"указанные в конфиге сервисов.\n"
		"Чтобы просмотреть ваш текущий список автозахода используйте команду \002AJOIN LIST\002", 
		/* LANG_AJOIN_UNREGISTERED */
		"Вы были принудительно подключены к общесетевым каналам данной сети.\n"
		"Если вас раздражает подобное самоуправство со стороны сервисов -\n"
		"зарегистрируйте свой ник и удалите эти каналы из вашего списка автозахода.",
		/* LANG_AJOIN_NOT_AVAILABLE */
		"Запрошенная вами команда временно недоступна.",
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
	moduleInsertLanguage(LANG_NL, LANG_NUM_STRINGS, langtable_nl);
	moduleInsertLanguage(LANG_DE, LANG_NUM_STRINGS, langtable_de);
	moduleInsertLanguage(LANG_TR, LANG_NUM_STRINGS, langtable_tr);
	moduleInsertLanguage(LANG_RU, LANG_NUM_STRINGS, langtable_ru);
}


/* EOF */
