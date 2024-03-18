/**
 * -----------------------------------------------------------------------------
 * Name    : bs_fantasy_dt
 * Author  : Viper  <Viper@Anope.org>
 * Date    : 04/10/2009  (Last update: 24/12/2011)
 * Version : 1.0
 * -----------------------------------------------------------------------------
 * Requires   : Anope-1.8.7
 * Tested     : Anope-1.8.7 + UnrealIRCd 3.2.8.1
 * -----------------------------------------------------------------------------
 * This module makes it possible to use a botserv bot as an eggbot, infobot or zbot 
 * like client by allowing it to respond to programmable triggers which can be 
 * dynamically controlled.
 *
 * Note that this module should be placed in ModuleDelayedAutoload,
 * and NOT in ModuleAutoload or one of the CoreModules directives.
 *
 * This module is released under the GPL 2 license.
 * -----------------------------------------------------------------------------
 * List of commands provided by this module:
 *
 *     - !dt set dtchar <trigger>
 *     - !dt set privmsg on/off
 *     - !dt set level access lvl
 *     - !dt set level edit lvl
 *
 *     - !dt add <+lvl> trigger <+pos> reply
 *     - !dt app trigger <+nr> reply
 *     - !dt del trigger <nr>
 *     - !dt link trigger target
 *     - !dt dup trigger target
 *     - !dt set trigger operonly on/off
 *     - !dt set trigger list on/off
 *     - !dt set trigger level <lvl>
 *     - !dt set trigger cond <+nr> <condition>
 *     - !dt list
 *     - !dt show trigger
 *     - !dt clear 
 *     - !dt info
 *
 *     - !gdt add <+lvl> trigger <+pos> reply
 *     - !gdt app trigger <+nr> reply
 *     - !gdt del trigger <nr>
 *     - !gdt link trigger target
 *     - !gdt dup trigger target
 *     - !gdt set trigger operonly on/off
 *     - !gdt set trigger list on/off
 *     - !gdt set trigger level <lvl>
 *     - !gdt set trigger cond <+nr> <condition>
 *     - !gdt list
 *     - !gdt show trigger
 *     - !gdt clear
 *     - !gdt info
 *
 *     - <dtchar><trigger> <arguments>
 *     - <dtchar><trigger> <arguments> ><nick>
 *     - <dtchar><trigger> <arguments> > <nick>
 *     - <dtchar> <trigger> <arguments>
 *     - ...
 *
 *
 * Inline Conditionals:
 *    Triggers support the use of inline conditionals for checking existance and
 *    comparing with user supplied arguments. Below is example syntax.
 *       If exists               $1?
 *       If not exsts (or else)  $1!
 *       End If                  $1$
 *       If arg 1 is "test"      $1==test?
 *       else (if not "test")    $1==test!
 *       End If                  $1==test$
 *       If arg 1 is not "test"  $1!=test?
 *
 *    Supported comparison operations: ==, !=, >, >=, < and <=.
 *    Note that end or else statements are optional, however if missing, everything
 *    following the statement will be considered part of the condition.
 *    Note that the parsing engine will interpret $1==test! as true when no
 *    argument is supplied, yet $1!=test? will be false when no argument is given.
 *    Likewise $1==test? will be false when no argument is given, yet $1!=test! 
 *    will be true when no argument is given.
 *
 *    A few examples:
 *       - Check if trigger is called with 3 arguments:
 *            Trigger $3? called with 3 args $3! not called with 3 args $3$.
 *       - Check if trigger is called with 2 arguments and if first is greater than 5:
 *            Trigger $2? called with 2 args and arg1 $1>5? greater $1>5! smaller $1>5$than 5.$2! not called with 2 args.
 *
 *
 * Reply Conditionals:
 *    It is possible to define a condition on a reply level. Only if that condition
 *    translates as 'true' will the reply line be considered for output to the user.
 *    The following operators are accepted:
 *       ()       Parse content between ( and ) as a whole.
 *       !        Invert result of following statement or subsequent ().
 *       ||       OR
 *       &&       AND
 *
 *    The statements making up the conditions support the same comparison operations
 *    as inline conditions. The == and != operation differ in that they expect
 *    subsequent strings to be enclosed by "s.
 *
 *    A few examples:
 *       - Check whether there are 3 arguments and arg1 must be greater than 0 or arg2 must be "test"
 *            $3 && ( $1 > 0 || $2 == "test")
 *       - Check if arguments given are equal to "anope rulezzz" or none are given and trigger is called by "Viper"
 *            $2 && !$3 && $1- == "anope rulezzz" || !$1 && $n == "viper"
 *            !$3 && $1- == "anope rulezzz" || !$1 && $n == "viper"
 *            (!$3 && $1- == "anope rulezzz") || (!$1 && $n == "viper")
 *
 *
 * Variables:
 *    As demonstrated in the above example, this module supports a number of build-in
 *    variables that may be used in (both reply and inline) conditionals or simply
 *    in a reply. These variables are substitued by their appropriate values when
 *    the reply is returned.
 *    The following variables are supported:
 *      Variable  |  Replaced by
 *         $b        BotServ bot nickname. 
 *         $c        Channel name.
 *         $n        Nick of the user who called the trigger.
 *         $h        Visible user@host of the user who called the trigger.
 *         $d        Destination of the message. (Normally channel name, but in case of a redirect it can be a nick.)
 *         $t        Topic of the channel the trigger was called in.
 *         $x        The name of the trigger that was called..
 *         $me       When used at the beginning of the reply: Reply is always send as a PRIVMSG ACTION.
 *                   When used inside a trigger it is substituted the botnick (identical to 'b').
 * -----------------------------------------------------------------------------
 * Translations:
 *
 *     - English and Dutch Languages provided and maintained by module author.
 *
 * -----------------------------------------------------------------------------
 *
 * Changelog:
 *
 *    1.0    Fixed '%' symbol interpretation interfering with output.
 *           Fixed SUPPORTED pre-compile configuration setting.
 *           Fixed missing help for LINK and DUP.
 *
 *    0.7    Added windows support. (Thx to Adam for bothering.)
 *           Added support for $b, $c, $n, $h, $d $t and $x as inline variables and in conditions.
 *           Added support for $me as trigger for ACTIONs when used at the start of the reply.
 *           Added support for redirecting output from the channel to users.
 *           Added a basic flood protection system for redirects.
 *           Changed $me to be be reserved as the trigger for actions. (DB automatically converted!)
 *           Updated the !help to include all the advanced fancy stuff.
 *           Fixed some potentially uninitialized variables. (Spotted by Adam)
 *           Fixed errors in the help for APP.
 *           Fixed SET PRIVMSG OFF reporting a syntax error.
 *           Fixed reply nr 1 being randomly returned even when condition fails.
 *           Updated version requirements and module & command checker.
 *           Fixed database loading showing errors, but not aborting.
 *
 *    0.6    Added DT DUP command to completely duplicate existing triggers.
 *           Added support for escaping special characters like $ with a \.
 *           Added support for inline conditionals.
 *           Added support for defining conditions on trigger reply level.
 *           Changed syntax for arguments in triggers: we now use $1 and $1-, without the second $.
 *           Fixed MaxLines setting not being respected.
 *           Fixed !dt del to return an error when attempting to delete nonexisting reply.
 *           Fixed segfaults caused by accepting of insertions at positions beyond array size.
 *           Fixed segfault when changing settings for a channel with no trigger data.
 *           Fixed bug in memory allocation for tracking linked items. 
 *           Fixed memory leak in !dt set.
 *
 *    0.5    Added support for linked items.
 *           Added SET TRIGGER LEVEL command.
 *           Added support for $1$, $1-$, $2$, $2-$,.. variables in triggers.
 *           Added support for inserting replies at specific positions in the list.
 *           Changed !help to reply to specific !dt command help queries.
 *           Fixed message parsing on UnrealIRCd when using tokens.
 *           Fixed a number of compile warnings.
 *           Fixed a few memory leaks.
 *           Fixed a crashbug in SET LEVEL.
 *
 *    0.4    Added dutch language strings.
 *           Removed obsolete trl code.
 *           Fixed segfault during attempted DB save if loading is aborted.
 *           Fixed bug in check_modules. (Thx to Adam for spotting this!)
 *           Made a number of improvements to language strings.
 *
 *    0.3    Mostly working now..
 *           Limited release of dev version.
 *
 *    0.1    Initial development version.
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

# BSTriggerDB [OPTIONAL]
# Module: bs_fantasy_dt
#
# Use the given filename as database to store the trigger information.
# If not given, the default of "bs_trigger.db" will be used.
#
#BSTriggerDB "bs_trigger.db"

# BSTriggerCharacter [OPTIONAL]
# Module: bs_fantasy_dt
#
# Default character used to trigger a trigger lookup.
# This can still be changed on a per channel database.
# If not specified, will default to BSFantasyCharacter.
#
#BSTriggerCharacter "?"

# BSTriggerRedirMinSendInterval [OPTIONAL - Recommended]
# Module: bs_fantasy_dt
#
# Minimum interval between 2 subsequent redirects issued
# by a user (SOs and above are exempt).
# If not specified, protection is disabled.
#
#BSTriggerRedirMinSendInterval 3m

# BSTriggerRedirMinRecvInterval [OPTIONAL - Recommended]
# Module: bs_fantasy_dt
#
# Minimum interval between 2 trigger redirects to a user.
# This prevents services being abused for flooding.
# If not specified, protection is disabled.
#
#BSTriggerRedirMinRecvInterval 30s

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


/**
 * Determines the maximum number of entries in a trigger list for any
 * given channel. 
 *
 * If this value is changed, it will not affect entries already in the
 * database, however now ones can't be added if the limit is exceeded.
 **/

int MaxTriggers = 4096;

/**
 * Maximum number of lines or alternatives a reply can have.
 **/
int MaxLines = 5;

/**
 * Default level required to be able to create, modify and delete channel triggers.
 **/
int DefEditLevel = 5;

/*-------------------------End of Configuration Block--------------------------*/

#define AUTHOR "Viper"
#define VERSION "1.0"
#define TRIGGERDBVERSION 2


/* Language defines */
#define LANG_NUM_STRINGS 					135

#define LANG_TRIGGER_HELP					0
#define LANG_DT_DESC						1
#define LANG_GDT_DESC						2
#define LANG_DT_SYNTAX						3
#define LANG_GDT_SYNTAX						4
#define LANG_DT_SYNTAX_EXT					5
#define LANG_GDT_SYNTAX_EXT					6
#define LANG_DT_ADD_SYNTAX					7
#define LANG_DT_ADD_SYNTAX_EXT				8
#define LANG_DT_APP_SYNTAX					9
#define LANG_DT_APP_SYNTAX_EXT				10
#define LANG_DT_DEL_SYNTAX					11
#define LANG_DT_DEL_SYNTAX_EXT				12
#define LANG_DT_SET_SYNTAX					13
#define LANG_DT_SET_SYNTAX_EXT				14
#define LANG_DT_SET_TR_OPERONLY_SYNTAX		15
#define LANG_DT_SET_TR_LIST_SYNTAX			16
#define LANG_DT_SET_DTCHAR_SYNTAX			17
#define LANG_DT_SET_PRIVMSG_SYNTAX			18
#define LANG_DT_SET_LEVEL_SYNTAX			19
#define LANG_DT_LIST_SYNTAX					20
#define LANG_DT_LIST_SYNTAX_EXT				21
#define LANG_DT_SHOW_SYNTAX					22
#define LANG_DT_SHOW_SYNTAX_EXT				23
#define LANG_DT_CLEAR_SYNTAX				24
#define LANG_DT_CLEAR_SYNTAX_EXT			25
#define LANG_DT_INFO_SYNTAX					26
#define LANG_DT_INFO_SYNTAX_EXT				27
#define LANG_GDT_ADD_SYNTAX					28
#define LANG_GDT_ADD_SYNTAX_EXT				29
#define LANG_GDT_APP_SYNTAX					30
#define LANG_GDT_APP_SYNTAX_EXT				31
#define LANG_GDT_DEL_SYNTAX					32
#define LANG_GDT_DEL_SYNTAX_EXT				33
#define LANG_GDT_SET_SYNTAX					34
#define LANG_GDT_SET_SYNTAX_EXT				35
#define LANG_GDT_SET_TR_OPERONLY_SYNTAX		36
#define LANG_GDT_SET_TR_LIST_SYNTAX			37
#define LANG_GDT_LIST_SYNTAX				38
#define LANG_GDT_LIST_SYNTAX_EXT			39
#define LANG_GDT_SHOW_SYNTAX				40
#define LANG_GDT_SHOW_SYNTAX_EXT			41
#define LANG_GDT_CLEAR_SYNTAX				42
#define LANG_GDT_CLEAR_SYNTAX_EXT			43
#define LANG_GDT_INFO_SYNTAX				44
#define LANG_GDT_INFO_SYNTAX_EXT			45
#define LANG_ERROR							46
#define LANG_CMD_DISABLED					47
#define LANG_CMD_NOT_AVAILABLE				48
#define LANG_CANNOT_CREATE_TR				49
#define LANG_LEVEL_INVALID					50
#define LANG_DT_ADDED						51
#define LANG_GDT_ADDED						52
#define LANG_DT_NO_ENTRIES					53
#define LANG_DT_NO_SUCH_ENTRY				54
#define LANG_DT_APP_SPEC_ENTRY_NR			55
#define LANG_DT_APPENDED					56
#define LANG_DT_INVALID_NR					57
#define LANG_DT_DELETED						58
#define LANG_DT_DELETED_NR					59
#define LANG_DT_LIST_HEADER					60
#define LANG_GDT_LIST_HEADER				61
#define LANG_DT_LIST						62
#define LANG_GDT_LIST						63
#define LANG_DT_LIST_EMPTY					64
#define LANG_GDT_LIST_EMPTY					65
#define LANG_DT_SHOW_TRIGGER				66
#define LANG_GDT_SHOW_TRIGGER				67
#define LANG_DT_SHOW_CREATED				68
#define LANG_DT_SHOW_UPDATED				69
#define LANG_DT_SHOW_LEVEL					70
#define LANG_DT_INFO_OPT_LIST				71
#define LANG_DT_INFO_OPT_RAND				72
#define LANG_DT_OPT_OPERONLY				73
#define LANG_DT_SHOW_FLAGS					74
#define LANG_DT_SHOW_ENTRIES				75
#define LANG_DT_CLEARED						76
#define LANG_GDT_CLEARED					77
#define LANG_GDT_SET_UNKWN_TRIGGER			78
#define LANG_GDT_SET_UNKWN_OPTION			79
#define LANG_DT_SET_UNKWN_OPTION_TRIGGER	80
#define LANG_DT_SET_TR_OPERONLY_ON			81
#define LANG_DT_SET_TR_OPERONLY_OFF			82
#define LANG_DT_SET_TR_LIST					83
#define LANG_DT_SET_TR_RAND					84
#define LANG_DT_SET_PRIVMSG_ON				85
#define LANG_DT_SET_PRIVMSG_OFF				86
#define LANG_DT_SET_DTCHAR					87
#define LANG_DT_SET_LEVEL_A					88
#define LANG_DT_SET_LEVEL_E					89
#define LANG_DT_SET_ACCESS_INV_LVL			90
#define LANG_DT_INFO_COUNT					91
#define LANG_GDT_INFO_COUNT					92
#define LANG_DT_INFO_TCHAR					93
#define LANG_DT_INFO_LVL_ACCESS				94
#define LANG_DT_INFO_LVL_EDIT				95
#define LANG_DT_INFO_PRIVMSG_OFF			96
#define LANG_DT_INFO_PRIVMSG_ON				97
#define LANG_GDT_INFO_DEF_TCHAR				98
#define LANG_DT_LINK_SYNTAX					99
#define LANG_DT_LINK_SYNTAX_EXT				100
#define LANG_GDT_LINK_SYNTAX				101
#define LANG_GDT_LINK_SYNTAX_EXT			102
#define LANG_DT_EXISTS						103
#define LANG_DT_LINKED						104
#define LANG_GDT_LINKED						105
#define LANG_DT_CANNOT_EDIT_LINKED			106
#define LANG_DT_SET_TR_LVL_SYNTAX			107
#define LANG_GDT_SET_TR_LVL_SYNTAX			108
#define LANG_DT_SET_TR_LEVEL				109
#define LANG_DT_INV_POS						110
#define LANG_DT_MAX_REPLIES					111
#define LANG_DT_DUP_SYNTAX					112
#define LANG_DT_DUP_SYNTAX_EXT				113
#define LANG_GDT_DUP_SYNTAX					114
#define LANG_GDT_DUP_SYNTAX_EXT				115
#define LANG_DT_DUP							116
#define LANG_GDT_DUP						117
#define LANG_DT_INFO_TCHAR_DEF				118
#define LANG_DT_SHOW_ENTRY_COND				119
#define LANG_DT_SHOW_ENTRY_LIST_COND		120
#define LANG_DT_SET_COND_SYNTAX				121
#define LANG_GDT_SET_COND_SYNTAX			122
#define LANG_DT_SET_COND_SET				123
#define LANG_DT_SET_COND_CLEARED			124
#define LANG_DT_SET_COND_ENTRY_NR			125
#define LANG_DT_COND_SYNTAX_ERR				126
#define LANG_DT_REDIRECT_DENIED				127
#define LANG_DT_CONDITIONS					128
#define LANG_DT_CONDITIONS_INLINE			129
#define LANG_DT_CONDITIONS_REPLY			130
#define LANG_DT_VARIABLES					131
#define LANG_REDIRECT_MIN_SEND_INTERVAL		132
#define LANG_REDIRECT_MIN_RECV_INTERVAL		133
#define LANG_REDIRECT_COMPLETE				134


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
#define MAXKEYLEN 256
#define MAXVALLEN 8192

/* Create a simple hash of the trigger to begin lookup.. */
#define HASH(trigger)			((tolower((trigger)[0])&31)<<5 | (tolower((trigger)[1])&31))

/* Flags to set... */
/* TriggerEntries.. */
#define TE_OPERONLY				0x0001
#define TE_LIST					0x0002
/* TriggerData.. */
#define TD_USE_PRIVMSG			0x0001


/* Structs */
typedef struct db_file_ DBFile;
typedef struct triggerreply_ TriggerReply;
typedef struct triggerdata_ TriggerData;
typedef struct triggerentry_ TriggerEntry;

struct db_file_ {
	FILE *fptr;             /* Pointer to the opened file */
	int db_version;         /* The db version of the datafiles (only needed for reading) */
	int core_db_version;    /* The current db version of this anope source */
	char service[256];      /* StatServ/etc. */
	char filename[256];     /* Filename of the database */
	char temp_name[262];    /* Temp filename of the database */
};

struct triggerentry_ {
	TriggerEntry *prev, *next;
	char *trigger;
	TriggerReply **replies;
	char **cond;
	int count;
	char *creator;
	char *last_upd;
	time_t added;
	time_t last_updt;
	int16 level;
	uint16 flags;
	char *link;
	char **links;
	int linkcount;
};

struct triggerdata_ {
	TriggerData *prev, *next;
	char name[CHANMAX];
	int count;
	TriggerEntry *entries[1024];
	/* Flags provide the ability to store certain settings. */
	uint16 flags;
	int16 level_a; /* Level required to query triggers in this channel. */
	int16 level_e; /* Level required to create/edit/delete triggers in channel. */
	/* Define which symbol triggers us in this channel.. */
	char *tchar;
};

struct triggerreply_ {
	char *data;
	char *condition;
};


/* Constants */
char *DefTriggerDB = "bs_trigger.db";
char *ModDataLastSend = "DTLastSnd";
char *ModDataLastRecv = "DTLastRcv";
uint16 DefTeFlags = 0 | TE_LIST;
uint16 DefTdFlags = 0;


/* Variables */
int supported;
int gcount;
char *TriggerDB;
char *TriggerCharacter;
int BSTriggerRedirMinSendInterval;
int BSTriggerRedirMinRecvInterval;

TriggerEntry *gtriggers[1024];
TriggerData *channels[256];


/* Functions */
int do_trigger(char *source, int ac, char **av);
void do_reply(ChannelInfo *ci, User *u, TriggerData *td, TriggerEntry *te, int is_global, char *trigger, char *args);
void send_trigger(ChannelInfo *ci, User *u, TriggerData *td, TriggerEntry *te, int is_global, char *trigger, char *args, int nr, User *redir);
char **get_redirects(ChannelInfo *ci, char **args);
char *buildTriggerReply(char *txt, char *trigger, char *args, Channel *c, User *u, char *dest, int *action);
char *nextSpecialChar(char *ptr);
int validate_condition_syntax(char *cond);
int check_triggerreply_condition(char *cond, char *trigger, char *args, Channel *c, User *u, char *dest);
int check_triggerreply_condition_ext(char **ptr, char *trigger, char *args, int subcond, Channel *c, User *u, char *dest);
int parse_statement(char **stmt, char *trigger, char *args, Channel *c, User *u, char *dest);
int get_rand_reply(TriggerEntry *te, char *trigger, char *args, Channel *c, User *u, char *dest, int def);

int do_fantasy(int ac, char **av);
void send_help(char *bot, User *u, char *cmd, char *param, short int global);
void parse_dt(User *u, ChannelInfo *ci, int global, char *cmd, char *params);
static void show_modinfo(User *u, ChannelInfo *ci);
int do_cs_drop(int ac, char **av);

TriggerData *createTriggerData(char *chan);
TriggerData *getTriggerData(char *chan);
void delTriggerData(char *chan);
void freeTriggerData(TriggerData *td);
void clearTriggerEntries(TriggerData *td);
void clearGlobalTriggerEntries();
TriggerEntry *addTriggerEntry(TriggerData *td, char *trigger);
TriggerEntry *addGlobalTriggerEntry(char *trigger);
TriggerEntry *createTriggerEntry(TriggerEntry *list[], char *trigger);
void addToTriggerEntry(TriggerEntry *te, char *data);
void addToTriggerEntryAt(TriggerEntry *te, char *data, short int position);
void appToTriggerEntry(TriggerEntry *te, int nr, char *data);
TriggerEntry *getTriggerEntry(TriggerEntry *list[], char *trigger);
TriggerEntry *getRealTriggerEntry(TriggerEntry *list[], char *trigger);
TriggerEntry *getGlobalTriggerEntry(char *trigger);
TriggerEntry *getRealGlobalTriggerEntry(char *trigger);
void delTrigger(TriggerData *td, char *trigger);
void delGlobalTrigger(char *trigger);
void delTriggerDataNr(TriggerData *td, char *trigger, int nr);
void delGlobalTriggerDataNr(char *trigger, int nr);
void delTriggerEntry(TriggerEntry *list[], TriggerEntry *te);
void freeTriggerEntry(TriggerEntry *te);
void linkTriggerEntries(TriggerEntry *te, TriggerEntry *target);
void duplicateTriggerEntry(TriggerEntry *te, TriggerEntry *target);
void clear_db();

void load_trigger_db(void);
void convert_trigger_db(int version);
void save_trigger_db(void);
int save_triggerlist(DBFile *dbptr, TriggerEntry *list[]);
void save_triggerentry(DBFile *dbptr, TriggerEntry *te);

int do_save(int argc, char **argv);
int db_backup(int argc, char **argv);

int check_modules(void);
int event_check_module(int argc, char **argv);
int event_check_cmd(int argc, char **argv);

char* get_flags();
void update_version(void);

void load_config(void);
int reload_config(int argc, char **argv);
void add_languages(void);

char *myModuleGetLangString(User * u, int number);
void myModuleNoticeLang(char *source, User * u, int number, ...);

char *str_replace(char *replace_target, char *replace_with, char *source);

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
	Message *msg;
	EvtHook *hook;

	alog("[\002bs_fantasy_dt\002] Loading module...");

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	supported = 1;

	if (!moduleMinVersion(1,8,7,3089)) {
		alog("[\002bs_fantasy_dt\002] Your version of Anope isn't supported. Please update to a newer release.");
		return MOD_STOP;
	}

	check_modules();
	if (supported == 0) {
		alog("[\002bs_fantasy_dt\002] Warning: Module continuing in unsupported mode!");
	} else if (supported == -1) {
		alog("[\002bs_fantasy_dt\002] Unloading module due to incompatibilities!");
		return MOD_STOP;
	}

	msg = createMessage("PRIVMSG", do_trigger);
	if (moduleAddMessage(msg, MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_dt\002] Can't parse incoming msgs...");
		return MOD_STOP;
	}
	if (UseTokens) {
		msg = createMessage("!", do_trigger);
		if (moduleAddMessage(msg, MOD_HEAD) != MOD_ERR_OK) {
			alog("[\002bs_fantasy_dt\002] Can't parse incoming token msgs...");
			return MOD_STOP;
		}
	}

	hook = createEventHook(EVENT_BOT_FANTASY, do_fantasy);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_dt\002] Can't hook to EVENT_BOT_FANTASY event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_BOT_FANTASY_NO_ACCESS, do_fantasy);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_dt\002] Can't hook to EVENT_BOT_FANTASY_NO_ACCESS event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_CHAN_DROP, do_cs_drop);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_dt\002] Can't hook to EVENT_CHAN_DROP event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_CHAN_EXPIRE, do_cs_drop);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_dt\002] Can't hook to EVENT_CHAN_EXPIRE event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_RELOAD, reload_config);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_dt\002] Can't hook to EVENT_RELOAD event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_DB_SAVING, do_save);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_dt\002] Can't hook to EVENT_DB_SAVING event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_DB_BACKUP, db_backup);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_dt\002] Can't hook to EVENT_DB_BACKUP event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_MODLOAD, event_check_module);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_dt\002] Can't hook to EVENT_MODLOAD event.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_ADDCOMMAND, event_check_cmd);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002bs_fantasy_dt\002] Can't hook to EVENT_ADDCOMMAND event.");
		return MOD_STOP;
	}

	load_config();
	add_languages();
	load_trigger_db();

	/* Update version info.. */
	update_version();

	alog("[\002bs_fantasy_dt\002] Module loaded successfully...");

	return MOD_CONT;
}


/**
 * Unload the module
 **/
void AnopeFini(void) {
	save_trigger_db();
	clear_db();

	alog("[\002bs_fantasy_dt\002] Unloading module...");
}

/* ------------------------------------------------------------------------------- */

int do_trigger(char *source, int ac, char **av) {
	User *u;
	ChannelInfo *ci;
	char dilim = ' ';
	char *tchar, *trigger, *args;
	TriggerData *td;
	TriggerEntry *te;

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

	if (*av[0] == '#') {
		if (s_BotServ && (ci = cs_findchan(av[0])) && !(ci->flags & CI_VERBOTEN) && ci->c && ci->bi) {
			/* Look whether we have an entry for this chan.. */
			td = getTriggerData(av[0]);
			/* Check if the user has access and make sure the msg actually exists.. */
			if ((td && (get_access(u, ci) >= td->level_a)) && av[1]) {
				tchar = myStrGetToken(av[1], dilim, 0);
				if (!tchar)
					return MOD_CONT;

				/* Check whether we got a "? trigger" or a "?trigger" and make sure the trigger char is correct.
				 * If not, abort. */
				if (strlen(tchar) == 1 && ((td && td->tchar && tchar[0] == *td->tchar)  || ((!td || !td->tchar) && tchar[0] == *TriggerCharacter))) {
					trigger = myStrGetToken(av[1], dilim, 1);
					args = myStrGetTokenRemainder(av[1], dilim, 1);
				} else if (strlen(tchar) > 1 && ((td && td->tchar && tchar[0] == *td->tchar) || ((!td || !td->tchar) && tchar[0] == *TriggerCharacter))) {
					char *tmp = av[1];
					trigger = sstrdup(++tchar);
					args = myStrGetTokenRemainder(++tmp, dilim, 0);
					tchar--;
				} else {
					free(tchar);
					return MOD_CONT;
				}

				/* Get the global triggerentry, if it exists.. */
				if ((te = getRealTriggerEntry(gtriggers, trigger))) {
					/* Check to see if user has access to trigger.. */
					if (get_access(u, ci) >= te->level && (!(te->flags & TE_OPERONLY) || is_services_oper(u))) {
						do_reply(ci, u, td, te, 1, trigger, args);
					}
				}

				/* Now get the channel specific entry.*
				 * This means that it s possible 2 trigger replies will be send! */
				if (td && (te = getRealTriggerEntry(td->entries, trigger))) {
					/* Check to see if user has access to trigger.. */
					if (get_access(u, ci) >= te->level && (!(te->flags & TE_OPERONLY) || is_services_oper(u))) {
						do_reply(ci, u, td, te, 0, trigger, args);
					}
				}

				free(tchar);
				free(trigger);
				free(args);
			}
		}
	}

	return MOD_CONT;
}


/**
 * Check the trigger call to check for redirects and determine the recipients and send the reply based on trigger type.
 *
 * @param ci Channel in which the trigger was called.
 * @param u User who called the trigger.
 * @param td Pointer to the structure storing trigger settings for the channel.
 * @param te Pointer to the structure storing the data of the trigger that was called.
 * @param is_global True/false (1/0) indication whether the trigger is a local channel or a global trigger.
 * @param trigger Name as which the the trigger was called. This may be different from the name of the trigger entry for linked entries.
 * @param args Arguments passed along when the trigger was called. This may still include the redirect string at this point.
 **/
void do_reply(ChannelInfo *ci, User *u, TriggerData *td, TriggerEntry *te, int is_global, char *trigger, char *args) {
	User *u2;
	int i, j, r, cur_time;
	char *botnick = NULL, **dest = NULL, *RedLastSend = NULL, *RedLastRecv = NULL;

	if (!ci || !u || !te || !args)
		return;

	cur_time = time(NULL);
	if (ci->bi)
		botnick = ci->bi->nick;
	else
		botnick = s_ChanServ;

	/* Determine if we are redirecting the output to (a) nickname(s)..
	 * If there s a redirect, ensure the user has at least VOICE access on the channel.. */
	dest = get_redirects(ci, &args);
	if (dest && !check_access(u, ci, CA_VOICE) && !check_access(u, ci, CA_HALFOPME) && !is_services_oper(u))
		myModuleNoticeLang(botnick, u, LANG_DT_REDIRECT_DENIED);
	/* Ensure the user can only send one redirect every set interval (BSTriggerRedirMinSendInterval). */
	else if (dest && ((RedLastSend = moduleGetData(&u->moduleData, ModDataLastSend))) && 
			(atoi(RedLastSend) + BSTriggerRedirMinSendInterval > cur_time) && !is_services_oper(u))
		myModuleNoticeLang(botnick, u, LANG_REDIRECT_MIN_SEND_INTERVAL, BSTriggerRedirMinSendInterval,
				atoi(RedLastSend) + BSTriggerRedirMinSendInterval - cur_time);
	else if (te->flags & TE_LIST) {
		/* It s a list.. show all stored entries. */
		if (dest) {
			for (i = 0; dest[i]; i++) {
				if (!((u2 = finduser(dest[i]))))
					notice_lang(botnick, u, NICK_X_NOT_IN_USE, dest[i]);
				else if (!is_on_chan(ci->c, u2))
					notice_lang(botnick, u, NICK_X_NOT_ON_CHAN, dest[i], ci->name);
				/* Ensure the user doesn't receive redirects more often then allowed (BSTriggerRedirMinRecvInterval). */
				else if (!is_services_oper(u) && ((RedLastRecv = moduleGetData(&u2->moduleData, ModDataLastRecv))) &&
						(atoi(RedLastRecv) + BSTriggerRedirMinRecvInterval > cur_time))
					myModuleNoticeLang(botnick, u, LANG_REDIRECT_MIN_RECV_INTERVAL, BSTriggerRedirMinRecvInterval, dest[i],
							atoi(RedLastRecv) + BSTriggerRedirMinRecvInterval - cur_time);
				else {
					for (j = 0; j < te->count; j++)
						send_trigger(ci, u, td, te, is_global, trigger, args, j, u2);
				}
				if (RedLastRecv) free(RedLastRecv);
				RedLastRecv = NULL;
			}
		} else {
			for (i = 0; i < te->count; i++)
				send_trigger(ci, u, td, te, is_global, trigger, args, i, NULL);
		}
	} else {
		/* It s no list.. randomly pick a line to display.. 
		 * Note that if there are multiple recipients, each may get a diffenent message because
		 * the conditions on the reply have to be re-checked for each user.. */
		if (dest) {
			j = -1;
			for (i = 0; dest[i]; i++) {
				r = get_rand_reply(te, trigger, args, ci->c, u, NULL, j);
				if (j == -1)
					j = r;
				if (r >= 0) {
					if (!((u2 = finduser(dest[i]))))
						notice_lang(botnick, u, NICK_X_NOT_IN_USE, dest[i]);
					else if (!is_on_chan(ci->c, u2))
						notice_lang(botnick, u, NICK_X_NOT_ON_CHAN, dest[i], ci->name);
					/* Ensure the user doesn't receive redirects more often then allowed (BSTriggerRedirMinRecvInterval). */
					else if (!is_services_oper(u) && ((RedLastRecv = moduleGetData(&u2->moduleData, ModDataLastRecv))) &&
							(atoi(RedLastRecv) + BSTriggerRedirMinRecvInterval > cur_time))
						myModuleNoticeLang(botnick, u, LANG_REDIRECT_MIN_RECV_INTERVAL, BSTriggerRedirMinRecvInterval, dest[i],
								atoi(RedLastRecv) + BSTriggerRedirMinRecvInterval - cur_time);
					else
						send_trigger(ci, u, td, te, is_global, trigger, args, r, u2);
					if (RedLastRecv) free(RedLastRecv);
					RedLastRecv = NULL;
				}
			}
		} else {
			r = get_rand_reply(te, trigger, args, ci->c, u, NULL, -1);
			send_trigger(ci, u, td, te, is_global, trigger, args, r, NULL);
		}
	}

	/* Free the array containing the destinations list.. */
	if (dest) {
		for (i = 0; dest[i]; i++)
			if (dest[i]) free(dest[i]);
		free(dest);

		/* Send a confirmation message to the caller.. */
		myModuleNoticeLang(botnick, u, LANG_REDIRECT_COMPLETE);
	}
	if (RedLastSend) free(RedLastSend);
}


/**
 * Sends a trigger reply to a user or channel.
 *
 * @param ci Channel in which the trigger was called.
 * @param u User who called the trigger.
 * @param td Pointer to the structure storing trigger settings for the channel.
 * @param te Pointer to the structure storing the data of the trigger that was called.
 * @param is_global True/false (1/0) indication whether the trigger is a local channel or a global trigger.
 * @param trigger Name as which the the trigger was called. This may be different from the name of the trigger entry for linked entries.
 * @param args Arguments passed along when the trigger was called.
 * @param nr Index of the reply to be send to the user.
 * @param redir User to send the reply to in case of a redirect.
 **/
void send_trigger(ChannelInfo *ci, User *u, TriggerData *td, TriggerEntry *te, int is_global, char *trigger, char *args, int nr, User *redir) {
	int action = 0, i = 0;
	char *botnick, *msg, *dest;
	char tmp[BUFSIZE];

	if (redir)
		dest = redir->nick;
	else
		dest = ci->name;
	if (ci->bi)
		botnick = ci->bi->nick;
	else
		botnick = s_ChanServ;
	if (nr >= 0)
		i = nr;

	/* Check if the trigger reply matches the conditions set to to be send.. */
	if (!check_triggerreply_condition(te->replies[nr]->condition, trigger, args, ci->c, u, dest))
		return;

	/* Build the actual reply based on the given parameters.. */
	msg = buildTriggerReply(te->replies[nr]->data, trigger, args, ci->c, u, dest, &action);

	if (action)
		anope_cmd_action(botnick, dest, "%s", msg);
	else {
		/* Wrap the actual reply in something we can send to the client.. */
		memset(tmp, 0, BUFSIZE);
		if (redir)
			snprintf(tmp, BUFSIZE, "[%s] ", ci->name);
		if (is_global)
			snprintf(tmp + strlen(tmp), BUFSIZE, "[Global] ");
		if (te->count == 1)
			snprintf(tmp + strlen(tmp), BUFSIZE, "%s: %s", te->trigger, msg);
		else
			snprintf(tmp + strlen(tmp), BUFSIZE, "%s[%d]: %s", te->trigger, nr + 1, msg);

		if ((td && (td->flags & TD_USE_PRIVMSG)) || (!td && (DefTdFlags & TD_USE_PRIVMSG)))
			anope_cmd_privmsg(botnick, dest, "%s", tmp);
		else
			anope_cmd_notice(botnick, dest, "%s", tmp);
	}

	/* If this was a redirect, store flood protection data.. */
	if (redir) {
		i = time(NULL);
		snprintf(tmp, BUFSIZE, "%d", i);

		/* Store when the sender has last send a redirect.. */
		moduleAddData(&u->moduleData, ModDataLastSend, tmp);

		/* Store when the user has last received a redirect.. */
		moduleAddData(&redir->moduleData, ModDataLastRecv, tmp);
	}

	free(msg);
}


/**
 * Search for a redirect character ('>') in the arguments list and split the arguments at that point.
 * Return the nicks following the character. This is a space separated list of nicks to redirect to.
 * The redirect part of the arguments is removed from the arguments string to it doesn't interfer with processing.
 *
 * @param ci Channel in which the trigger was called.
 * @param args Strings of all arguments passed to the list.
 * @return Array containing the nicknames to redirect to. (Both the elements and the array need to be free'd.)
 **/
char **get_redirects(ChannelInfo *ci, char **args) {
	int count = 0;
	char *ptr, *redir = NULL, **ret = NULL;

	if (!args)
		return NULL;

	ptr = *args;
	while (ptr && ((ptr = strstr(ptr, ">")))) {
		ptr--;
		if (ptr[0] != ' ') {
			ptr++;
			continue;
		}
		ptr[0] = ptr[1] = 0;
		ptr += 2;
		break;
	}

	/* Skip any spaces after the '>'.. */
	while (ptr && ptr[0] == ' ')
		ptr++;
	redir = ptr;

	if (redir) {
		while ((ptr = myStrGetToken(redir, ' ', count))) {
			/* The last element is always a NULL pointer to indicate the end of the list.. */
			ret = realloc(ret, sizeof(char *) * (count + 2));
			ret[count] = ptr;
			count++;
			ret[count] = NULL;
		}
	}

	return ret;
}


/**
 * This function replaces needles and escape sequences.
 * 
 * @param txt String to replace the needles in.
 * @param trigger Trigger that was originally called.
 * @param args Arguments supplied to the trigger call.
 * @param c Channel in which the trigger was called.
 * @param u User calling the trigger.
 * @param dest Destination of the message. Normally the channel name, in case of a redirect, the nick to which the message is being redirected.
 * @param action Pointer to an integer indicating whether the reply should be send as an ACTION (always PRIVMSG) or using the default setting.
 * @return Newly allocated string with keys replaced.
 **/
char *buildTriggerReply(char *txt, char *trigger, char *args, Channel *c, User *u, char *dest, int *action) {
	short int size = 1; 
	char *ret, *ptr, *last;

	if (!txt || !c || !c->ci || !u || !dest)
		return NULL;
	*action = 0;

	/* Allocate memory for the return string.. */
	ret = scalloc(BUFSIZE * size, sizeof(char));

	/* Now look for needles and escape sequences.. */
	ptr = txt;
	last = txt;			/* Keeps track of the last position that was copied to the output string.. */
	while (ptr && (ptr = nextSpecialChar(ptr))) {
		short int cnt = 1, cont = 0;
		char buf[5];
		char *value = NULL, *tptr = NULL, *tmp = NULL;

		/* Check whether the character we found is a \, if it is, 
		 * copy the next character to the ouput string and move on.. */
		if (ptr[0] == '\\') {
			strncat(ret, last, ptr - last);
			ptr++;					/* Move on to the next character.. */
			strncat(ret, ptr, 1);	/* Copy escaped char to output string.. */
			ptr++;
			last = ptr;
			continue;		 		/* No more processing required.. */
		}

		/* Character is a '$' and needs parsing.. copy current block to output string 
		 * before continuing.. */
		/* Make sure the target buffer is big enough.. */
		while (BUFSIZE * size < (strlen(ret) + (ptr - last))) {
			size++;
			ret = srealloc(ret, BUFSIZE * size * sizeof(char));
		}
		strncat(ret, last, ptr - last);		/* Copy data from original to output string.. */
		last = ptr;
		ptr++;

		/* If it s followed by an int, it s an argument, if not, it may be a 
		 * predefined variable such as $me (these must not start with a number!). */
		if (ptr[0] >= '0' && ptr[0] <= '9') {
			while (ptr[cnt] >= '0' && ptr[cnt] <= '9') {
				/* Sanity check: don't allow more then 4 digits.. */
				if (cnt >= 4) {
					cont = 1;
					break;
				}
				cnt++;				/* count digits.. */
			}

			if (cont) continue;
			memset(buf, 0, 5);
			strncat(buf, ptr, cnt);
			ptr += cnt;

			if (ptr[0] == '-') {
				ptr++;
				value = myStrGetTokenRemainder(args, ' ', atoi(buf));
			} else {
				value = myStrGetToken(args, ' ', atoi(buf));
			}
		} else {
			/* It s a predefined variable.. 
			 *
			 * Currently support are...
			 *   b        BotServ bot nickname. 
			 *   c        Channel name.
			 *   n        Nick of the user who called the trigger.
			 *   h        Visible user@host of the user who called the trigger.
			 *   d        Destination of the message. (Normally channel name, but in case of a redirect it can be a nick.)
			 *   t        Topic of the channel the trigger was called in.
			 *   x        The name of the trigger that was called..
			 *   me       When used inside a trigger instead of at the beginning (ACTION), it represents the botnick.
			 */
			if (strstr(ptr, "me") == ptr) {
				ptr += 2;
				if (strlen(ret) == 0)
					*action = 1;
				else if (c->ci->bi)
					value = sstrdup(c->ci->bi->nick);
				else
					value = sstrdup(s_ChanServ);
			} else if (strstr(ptr, "b") == ptr) {
				ptr += 1;
				if (c->ci->bi)
					value = sstrdup(c->ci->bi->nick);
				else
					value = sstrdup(s_ChanServ);
			} else if (strstr(ptr, "c") == ptr) {
				ptr += 1;
				value = sstrdup(c->name);
			} else if (strstr(ptr, "n") == ptr) {
				ptr += 1;
				value = sstrdup(u->nick);
			} else if (strstr(ptr, "h") == ptr) {
				char mask[BUFSIZE];
				ptr += 1;
				snprintf(mask, BUFSIZE, "%s@%s", common_get_vident(u), common_get_vhost(u));
				value = sstrdup(mask);
			} else if (strstr(ptr, "d") == ptr) {
				ptr += 1;
				value = sstrdup(dest);
			} else if (strstr(ptr, "t") == ptr) {
				ptr += 1;
				value = sstrdup(((c->topic)) ? c->topic : "");
			} else if (strstr(ptr, "x") == ptr) {
				ptr += 1;
				value = sstrdup(trigger);
			}
		}

		/* The following can be done a lot more efficient, for now though, I ll do it this
		 * way for readability and simplicity.. */

		/* Now we have the argument.
		 * Next check whether we have to show the argument, or whether it is used to announce a conditional section. */
		/* These announces a custom condition.. parse the condition first.. */
		if ((ptr[0] == '=' || ptr[0] == '!') && ptr[1] == '=') {
			char *p = ptr, *f;
			ptr += 2;

			/* Statement is ended by either $, ? or !.. 
			 * Make sure it is not preceded by an escape character.. */
			tptr = ptr;
			while ((tptr = strstr(tptr, "$"))) {
				tptr--;
				if (tptr[0] != '\\') {
					tptr++;
					break;
				}
				tptr += 2;
			}

			tmp = ptr;
			while ((tmp = strstr(tmp, "?"))) {
				tmp--;
				if (tmp[0] != '\\') {
					tmp++;
					break;
				}
				tmp += 2;
			}
			if (tmp && (!tptr ||tmp < tptr))
				tptr = tmp;

			tmp = ptr;
			while ((tmp = strstr(tmp, "!"))) {
				tmp--;
				if (tmp[0] != '\\') {
					tmp++;
					break;
				}
				tmp += 2;
			}
			if (tmp && (!tptr ||tmp < tptr))
				tptr = tmp;

			if (!tptr) {
				if (value) free(value);
				continue;
			}
			tmp = scalloc((tptr - ptr) + 1, sizeof(char));
			strncat(tmp, ptr, tptr - ptr);
			/* We still have to filter out any possible escape sequences.. */
			while ((f = strstr(tmp, "\\")))
				memmove(f, f+1, ((tptr - ptr) + 1) - (f - tmp));

			/* Move pointer to end of statement.. */
			ptr = tptr;
			if (value && ((p[0] == '=' && strcmp(value, tmp)) || (p[0] == '!' && !strcmp(value, tmp)))) {
				/* Condition is false.. clear value as this is used to indicate true/false */
				free(value);
				value = NULL;
			}
			free(tmp);
			tmp = NULL;
		} else if (ptr[0] == '>' || ptr[0] == '<') { /* > >= and < <=  */
			char *p = ptr;
			int val, cmp;

			if (value) 
				val = atoi(value);
			else
				val = 0;

			if (ptr[1] == '=')
				ptr += 2;
			else 
				ptr++;

			tptr = NULL;
			/* Statement is ended by either $, ? or !.. */
			tptr = strstr(ptr, "$");
			if ((tmp = strstr(ptr, "?")) && tmp && (!tptr ||tmp < tptr))
				tptr = tmp;
			if ((tmp = strstr(ptr, "!")) && tmp && (!tptr ||tmp < tptr))
				tptr = tmp;
			if (!tptr) {
				if (value) free(value);
				continue;
			}
			tmp = scalloc((tptr - ptr) + 1, sizeof(char));
			strncat(tmp, ptr, tptr - ptr);
			cmp = atoi(tmp);
			ptr = tptr;

			/* Compare argument value with specified value.. */
			if (value && ((p[0] == '<' && p[1] == '=' && val > cmp) ||
					(p[0] == '<' && val >= cmp) ||
					(p[0] == '>' && val <= cmp) ||
					(val < cmp))) {
				free(value);
				value = NULL;
			}

			free(tmp);
			tmp = NULL;
		}

		/* Check whether it s a condition and of what type */
		if (ptr[0] == '?' || ptr[0] == '!' || ptr[0] == '$') {
			/* It's a conditional statement.. */
			if ((ptr[0] == '?' && value) || (ptr[0] == '!' && !value) || (ptr[0] == '$'))
				/* Simply continue if the statement is true or has no meaning.. */
				ptr++;
			else {
				/* Find the end of the statement (if any) */
				char ifstmt[128], elsestmt[128], endstmt[128];
				memset(ifstmt, 0, 128);
				memset(elsestmt, 0, 128);
				memset(endstmt, 0, 128);
				strncpy(ifstmt, last, ptr - last);
				strncpy(elsestmt, last, ptr - last);
				strncpy(endstmt, last, ptr - last);
				strncat(ifstmt, "?", 1);
				strncat(elsestmt, "!", 1);
				strncat(endstmt, "$", 1);

				tmp = ptr;
				ptr = NULL;
				/* Look for an if statement to jump to.. */
				if (tmp[0] == '!' && (tptr = strstr(tmp, ifstmt)))
					ptr = tptr;
				/* Look for an else statement to jump to.. */
				else if (tmp[0] == '?' && (tptr = strstr(tmp, elsestmt)))
					ptr = tptr;

				/* Look for an end statement to jump to.. */
				if ((tptr = strstr(tmp, endstmt)) && (!ptr || ptr > tptr))
					ptr = tptr;

				/* If there is no else or end termination statement, nothing more is appended to the reply */
				if (!ptr)
					tmp = NULL;
			} 
			/* Make sure the argument is not appended to the string.. */
			if (value) free(value);
			value = NULL;
		}

		/* Append the new string if it exists.. */
		if (value) {
			/* Make sure the target buffer is big enough.. */
			while (BUFSIZE * size < (strlen(ret) + strlen(value))) {
				size++;
				ret = srealloc(ret, BUFSIZE * size * sizeof(char));
			}
			strcat(ret, value);
			free(value);
		}
		last = ptr;
	}

	/* We still have to copy the part of the trigger behind the last needle (if any).. */
	if (last) {
		while (BUFSIZE * size < (strlen(ret) + strlen(last))) {
			size++;
			ret = srealloc(ret, BUFSIZE * size * sizeof(char));
		}
		strcat(ret, last);
	}

	return ret;
}

/**
 * Finds the first occurrence of \ or $ in given string.
 *
 * @param ptr String Haystack in which to look for a special character.
 * @return Returns pointer to the first occurrence of any special character in the string.
 **/
char *nextSpecialChar(char *ptr) {
	char *pos, *pos2;

	pos = strstr(ptr, "\\");
	pos2 = strstr(ptr, "$");

	if (pos && (!pos2 || pos < pos2))
		return pos;
	else if (pos2)
		return pos2;

	return NULL;
}

/* ------------------------------------------------------------------------------- */

/**
 * Performs a simple validation on the syntax of given condition by running it 
 * through the parser. The validation is done as if the trigger was called with no arguments.
 *
 * @param cond String The condition to validate.
 **/
int validate_condition_syntax(char *cond) {
	char **ptr = &cond;
	if (check_triggerreply_condition_ext(ptr, NULL, NULL, 0, NULL, NULL, NULL) == -1)
		return 0;
	return 1;
}

/**
 * Parses a condition based on given arguments.
 *
 * @param cond String The condition to parse.
 * @param trigger Trigger that was originally called.
 * @param args String Arguments passed to the trigger call.
 * @param c Channel in which the trigger was called.
 * @param u User calling the trigger.
 * @param dest Destination of the message. Normally the channel name, in case of a redirect, the nick to which the message is being redirected.
 * @return True (1) when trigger reply matches the conditions, false if not.
 **/
int check_triggerreply_condition(char *cond, char *trigger, char *args, Channel *c, User *u, char *dest) {
	char **ptr = &cond;
	int ret = check_triggerreply_condition_ext(ptr, trigger, args, 0, c, u, dest);
	return (ret <= 0 ? 0 : 1);
}

/**
 * Parses a condition based on given arguments.
 * This function works recursively by splitting up a condition into subconditions and substatements.
 * By the design of this function, if a ( is not closed, it s assumed closed at the end of the condition.
 *
 * @param cond String Pointer to the condition to parse.
 * @param trigger Trigger that was originally called.
 * @param args String Arguments passed to the trigger call.
 * @param subcond int Set to 1 when parsing a condition with a lower priority like &&. Will cause the function to return when encountering ||.
 * @param c Channel in which the trigger was called.
 * @param u User calling the trigger.
 * @param dest Destination of the message. Normally the channel name, in case of a redirect, the nick to which the message is being redirected.
 * @return True (1) when trigger reply matches the conditions, false if not. Returns -1 when an error occured.
 **/
int check_triggerreply_condition_ext(char **ptr, char *trigger ,char *args, int subcond, Channel *c, User *u, char *dest) {
	int ret = 0;
	short int invert = 0;		/* Store whether next statement to be inverted */
	short int operation = -1;	/* -1: first stmt; 0: none; 1: AND; 2: OR; */

	if (!*ptr)
		return 1;

	/* Go over the condition.. */
	while ((*ptr)[0]) {
		int result = 0;
		/* Spaces have no meaning.. */
		if ((*ptr)[0] ==  ' ') {
			*ptr += 1;
			continue;
		}

		/* These are parsed recursively.. */
		if ((*ptr)[0] ==  '(' && operation) {
			*ptr += 1;
			result = check_triggerreply_condition_ext(ptr, trigger, args, 0, c, u, dest);
			if (result == -1) return result;
			if (invert)				/* If the () followed an !, invert returned value. */
				result = (result ? 0 : 1);
			invert = 0;

			if (operation == -1)
				ret = result;
			else if (operation == 1)
				ret = ret && result;
			else if (operation == 2)		/* This will normally never happen as these are considered in a subcondition */
				ret = ret || result;
			operation = 0;
		} else 

		/* When current statement ends, always return.. */
		if ((*ptr)[0] ==  ')' && operation < 1) {
			*ptr += 1;
			return ret;
		} else

		/* NOT is allowed on a first statement or following an AND or OR, not after another statement without an operator
		 * or following another NOT. */
		if ((*ptr)[0] == '!' && operation && !invert) {
			invert = 1;
			*ptr += 1;
		} else

		/* If it s a statement, parse it, and if applicable invert returned value (always applies to entire statement) */
		if ((*ptr)[0] == '$' && operation) {
			result = parse_statement(ptr, trigger, args, c, u, dest);
			if (result == -1) return result;
			if (invert)				/* If the $ was preceded by an !, invert returned value. */
				result = (result ? 0 : 1);

			if (operation == -1)
				ret = result;
			else if (operation == 1)
				ret = ret && result;
			else if (operation == 2)		/* This will normally never happen as these are considered in a subcondition */
				ret = ret || result;

			/* Reset operators */
			invert = 0;
			operation = 0;
		} else

		if ((*ptr)[0] == '&' && (*ptr)[1] == '&' && !operation) {
			*ptr += 2;
			operation = 1;
		} else if ((*ptr)[0] == '|' && (*ptr)[1] == '|' && !operation) {
			/* If we are parsing a subcondition (ex: "B && C" in "A || B && C || D"), return */
			if (subcond)
				return ret;

			/* We parse ORs in another call for operator precedence.. */
			*ptr += 2;
			result = check_triggerreply_condition_ext(ptr, trigger, args, 1, c, u, dest);
			if (result == -1) return result;
			ret = ret || result;
		} else
			return -1;
	}

	return ret;
}

/**
 * Parses a truth statement.
 * This function is given a pointer to the statement in the condition. When the
 * end of the statement is reached the function returns.
 *
 * @param stmt String Pointer to the statement to parse.
 * @param trigger Trigger that was originally called.
 * @param args String Arguments passed to the trigger call.
 * @param c Channel in which the trigger was called.
 * @param u User calling the trigger.
 * @param dest Destination of the message. Normally the channel name, in case of a redirect, the nick to which the message is being redirected.
 * @return True (1) when trigger reply matches the conditions, false if not. Returns -1 when an error occured.
 **/
int parse_statement(char **stmt, char *trigger, char *args, Channel *c, User *u, char *dest) {
	short int cnt = 1, err = 0, operation = 0, ret = 0, tmp = 0;
	double val = 0, target = 0;
	char buf[32];
	char *value = NULL;

	if (!*stmt || (*stmt)[0] != '$')
		return -1;

	/* Get the value of the statement.. */
	*stmt += 1;
	if ((*stmt)[0] >= '0' && (*stmt)[0] <= '9') {
		while ((*stmt)[cnt] >= '0' && (*stmt)[cnt] <= '9') {
			/* Sanity check: don't allow more then 4 digits.. */
			if (cnt >= 4) {
				err = 1;
				break;
			}
			cnt++;				/* count digits.. */
		}
		if (err)
			return -1;
		memset(buf, 0, 32);
		strncat(buf, *stmt, cnt);
		*stmt += cnt;

		if ((*stmt)[0] == '-') {
			*stmt += 1;
			value = myStrGetTokenRemainder(args, ' ', atoi(buf));
		} else {
			value = myStrGetToken(args, ' ', atoi(buf));
		}
	} else {
		/* It s a predefined variable.. 
		 *
		 * Currently support are...
		 *   b        BotServ bot nickname. 
		 *   c        Channel name.
		 *   n        Nick of the user who called the trigger.
		 *   h        Visible user@host of the user who called the trigger.
		 *   d        Destination of the message. (Normally channel name, but in case of a redirect it can be a nick.)
		 *   t        Topic of the channel the trigger was called in.
		 *   x        The name of the trigger that was called..
		 */
		if (strstr(*stmt, "me") == *stmt) {
			*stmt += 2;
			if (c && c->ci && c->ci->bi)
				value = sstrdup(c->ci->bi->nick);
			else if (c)
				value = sstrdup(s_ChanServ);
		} else if (strstr(*stmt, "b") == *stmt) {
			*stmt += 1;
			if (c && c->ci && c->ci->bi)
				value = sstrdup(c->ci->bi->nick);
			else
				value = sstrdup(s_ChanServ);
		} else if (strstr(*stmt, "c") == *stmt) {
			*stmt += 1;
			if (c)
				value = sstrdup(c->name);
		} else if (strstr(*stmt, "n") == *stmt) {
			*stmt += 1;
			if (u)
				value = sstrdup(u->nick);
		} else if (strstr(*stmt, "h") == *stmt) {
			*stmt += 1;
			if (u) {
				char mask[BUFSIZE];
				snprintf(mask, BUFSIZE, "%s@%s", common_get_vident(u), common_get_vhost(u));
				value = sstrdup(mask);
			}
		} else if (strstr(*stmt, "d") == *stmt) {
			*stmt += 1;
			if (dest)
				value = sstrdup(dest);
		} else if (strstr(*stmt, "t") == *stmt) {
			*stmt += 1;
			if (c)
				value = sstrdup(((c->topic)) ? c->topic : "");
		} else if (strstr(*stmt, "x") == *stmt) {
			*stmt += 1;
			if (trigger)
				value = sstrdup(trigger);
		}
	}

	/* Get the comparison operator, if any.. If none, we simply check for existance of the argument. */
	while (*stmt && (*stmt)[0] ==  ' ')
		*stmt += 1;
	if (!*stmt || ((*stmt)[0] != '=' && (*stmt)[0] != '!' && (*stmt)[0] != '<' && (*stmt)[0] != '>')) {
		if (value) {
			ret = 1;
			free(value);
		}
		return ret;
	}

	/* The statement (or condition) continues.. Determine the operator. */
	if ((*stmt)[0] == '=' && (*stmt)[1] == '=')
		operation = 1;
	else if ((*stmt)[0] == '!' && (*stmt)[1] == '=')
		operation = 2;
	else if ((*stmt)[0] == '<' && (*stmt)[1] == '=')
		operation = 3;
	else if ((*stmt)[0] == '<')
		operation = 4;
	else if ((*stmt)[0] == '>' && (*stmt)[1] == '=')
		operation = 5;
	else if ((*stmt)[0] == '>')
		operation = 6;
	else {
		if (value) free(value);
		return -1;
	}

	/* Get the value after the operator.. this must be a number, or a string enclosed by "" (only supported on == and !=). */
	if (operation == 4 || operation == 6)
		*stmt += 1;
	else
		*stmt += 2;
	while (*stmt && *stmt[0] ==  ' ')
		*stmt += 1;
	if (!*stmt) {
		if (value) free(value);
		return -1;
	}

	if (operation < 3 && (*stmt)[0] == '"') {
		char *end, *target;
		*stmt += 1;
		while ((end = strstr(*stmt, "\""))) {
			end--;
			if (end[0] != '\\') {
				end++;
				break;
			}
			end++;
		}
		if (!end) {
			if (value) free(value);
			return -1;
		}
		end++;				/* Move pointer beyond string */
		target = scalloc(end - *stmt, sizeof(char));
		memset(target, 0, end - *stmt);
		strncat(target, *stmt, end - *stmt - 1);

		if (value && ((operation == 1 && !stricmp(value, target)) || (operation == 2 && stricmp(value, target))))
			ret = 1;
		free(target);
		if (value) free(value);
		*stmt = end;
		return ret;
	}

	/* Only remaining operations are numerical.. */
	cnt = strspn(*stmt, "0123456789.-");
	if (!cnt) {
		if (value) free(value);
		return -1;
	}
	memset(buf, 0, 32);
	if (cnt > 30)
		tmp = 30;
	else tmp = cnt;
	strncat(buf, *stmt, tmp);
	*stmt += cnt;

	/* Make sure the value is a number.. if not, return 0, not -1, as it s not a condition error */
	if (value) {
		cnt = strspn(value, "0123456789.-");
		if (!cnt || cnt != strlen(value)) {
			free(value);
			return 0;
		}

		val = atof(value);
		target = atof(buf);

		if ((operation == 1 && val == target)
				|| (operation == 2 && val != target)
				|| (operation == 3 && val <= target)
				|| (operation == 4 && val < target)
				|| (operation == 5 && val >= target)
				|| (operation == 6 && val > target))
			ret = 1;
		free(value);
	}
	return ret;
}

/**
 * Gets a random reply for given trigger.
 * Checks whether conditions to respond with given reply match.
 *
 * @param te TriggerEntry Trigger to search for a random reply which fulfills all conditions.
 * @param trigger Trigger that was originally called.
 * @param args String Arguments passed to the trigger call.
 * @param c Channel in which the trigger was called.
 * @param u User calling the trigger.
 * @param dest Destination of the message in case of a redirect.
 * @param def If conditions are valid for the reply at this index, it will be returned. (Useful to try to send the same random reply to multiple users..)
 * @return Index of a random trigger reply.
 **/
int get_rand_reply(TriggerEntry *te, char *trigger, char *args, Channel *c, User *u, char *dest, int def) {
	int matches = 0, i, r;
	int *matching = scalloc(sizeof(int), MaxLines);

	for (i = 0; i < te->count; i++) {
		if (check_triggerreply_condition(te->replies[i]->condition, trigger, args, c, u, dest ? dest : c->name)) {
			if (i == def) {
				free(matching);
				return def;
			}
			matching[matches] = i;
			matches++;
		}
	}
	if (matches > 0) {
		r = rand() % matches;
		r = matching[r];
	} else
		r = -1;

	free(matching);
	return r;
}

/* ------------------------------------------------------------------------------- */

/**
 * Handles the fantasy trigger control commands.
 * Triggers themselves are not handled here..
 **/
int do_fantasy(int ac, char **av) {
	User *u;
	ChannelInfo *ci;
	Channel *c;
	char *botnick = NULL;

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
		myModuleNoticeLang(botnick, u, LANG_CMD_NOT_AVAILABLE);
		return MOD_CONT;
	}

	if (!stricmp(av[0], "gdt")) {
		/* If executed without any params, show help */
		if (ac == 3) {
			TriggerData *td = getTriggerData(av[2]);
			myModuleNoticeLang(botnick, u, LANG_TRIGGER_HELP, td && td->tchar ? td->tchar : TriggerCharacter);
			notice(botnick, u->nick, " ");
			myModuleNoticeLang(botnick, u, LANG_GDT_SYNTAX);
		} else {
			if (is_services_oper(u)) {
				/* Get the command and parameters.. */
				char *cmd = myStrGetToken(av[3],' ',0);
				char *params = myStrGetTokenRemainder(av[3],' ',1);

				if (!cmd)
					myModuleNoticeLang(botnick, u, LANG_GDT_SYNTAX);
				else
					parse_dt(u, ci, 1,cmd, params);

				if (cmd) free(cmd);
				if (params) free(params);
			} else
				notice_lang(botnick, u, PERMISSION_DENIED);
		}
	} else if (!stricmp(av[0], "dt")) {
		/* If executed without any params, show help */
		if (ac == 3) {
			TriggerData *td = getTriggerData(av[2]);
			myModuleNoticeLang(botnick, u, LANG_TRIGGER_HELP, td && td->tchar ? td->tchar : TriggerCharacter);
			notice(botnick, u->nick, " ");
			myModuleNoticeLang(botnick, u, LANG_DT_SYNTAX);
		} else {
			/* Get the command and parameters.. */
			char *cmd = myStrGetToken(av[3],' ',0);
			char *params = myStrGetTokenRemainder(av[3],' ',1);

			if (!cmd)
				myModuleNoticeLang(botnick, u, LANG_DT_SYNTAX);
			else
				parse_dt(u, ci, 0,cmd, params);

			if (cmd) free(cmd);
			if (params) free(params);
		}
	} else if (!stricmp(av[0], "minfo")) {
		show_modinfo(u, ci);
	} else if (!stricmp(av[0], "help")) {
		if (ac > 3) {
			int ret = MOD_CONT;
			char *cmd, *param, *param2;
			cmd = myStrGetToken(av[3],' ',0);
			param = myStrGetToken(av[3],' ',1);
			param2 = myStrGetToken(av[3],' ',2);
			if (!stricmp(cmd, "dt")) {
				send_help(botnick, u, param, param2, 0);
				ret = MOD_STOP;
			} else if (!stricmp(cmd, "gdt")) {
				send_help(botnick, u, param, param2, 1);
				ret = MOD_STOP;
			}
			free(cmd);
			if (param) free(param);
			if (param2) free(param2);
			return ret;
		}
	}

	/* Continue processig event.. maybe other modules want it too */
	return MOD_CONT;
}

void send_help(char *bot, User *u, char *cmd, char *param, short int global) {
	if (!cmd)
		myModuleNoticeLang(bot, u, global ? LANG_GDT_SYNTAX_EXT : LANG_DT_SYNTAX_EXT);
	else if (!stricmp(cmd, "add"))
		myModuleNoticeLang(bot, u, global ? LANG_GDT_ADD_SYNTAX_EXT : LANG_DT_ADD_SYNTAX_EXT);
	else if (!stricmp(cmd, "app"))
		myModuleNoticeLang(bot, u, global ? LANG_GDT_APP_SYNTAX_EXT : LANG_DT_APP_SYNTAX_EXT);
	else if (!stricmp(cmd, "del"))
		myModuleNoticeLang(bot, u, global ? LANG_GDT_DEL_SYNTAX_EXT : LANG_DT_DEL_SYNTAX_EXT);
	else if (!stricmp(cmd, "link"))
		myModuleNoticeLang(bot, u, global ? LANG_GDT_LINK_SYNTAX_EXT : LANG_DT_LINK_SYNTAX_EXT);
	else if (!stricmp(cmd, "dup"))
		myModuleNoticeLang(bot, u, global ? LANG_GDT_DUP_SYNTAX_EXT : LANG_DT_DUP_SYNTAX_EXT);
	else if (!stricmp(cmd, "set"))
		myModuleNoticeLang(bot, u, global ? LANG_GDT_SET_SYNTAX_EXT : LANG_DT_SET_SYNTAX_EXT);
	else if (!stricmp(cmd, "list"))
		myModuleNoticeLang(bot, u, global ? LANG_GDT_LIST_SYNTAX_EXT : LANG_DT_LIST_SYNTAX_EXT);
	else if (!stricmp(cmd, "show"))
		myModuleNoticeLang(bot, u, global ? LANG_GDT_SHOW_SYNTAX_EXT : LANG_DT_SHOW_SYNTAX_EXT);
	else if (!stricmp(cmd, "clear"))
		myModuleNoticeLang(bot, u, global ? LANG_GDT_CLEAR_SYNTAX_EXT : LANG_DT_CLEAR_SYNTAX_EXT);
	else if (!stricmp(cmd, "info"))
		myModuleNoticeLang(bot, u, global ? LANG_GDT_INFO_SYNTAX_EXT : LANG_DT_INFO_SYNTAX_EXT);
	else if (!stricmp(cmd, "conditions")) {
		if (!param)
			myModuleNoticeLang(bot, u, LANG_DT_CONDITIONS);
		else if (!stricmp(param, "inline"))
			myModuleNoticeLang(bot, u, LANG_DT_CONDITIONS_INLINE);
		else if (!stricmp(param, "reply"))
			myModuleNoticeLang(bot, u, LANG_DT_CONDITIONS_REPLY);
		else
			myModuleNoticeLang(bot, u, LANG_DT_CONDITIONS);
	} else if (!stricmp(cmd, "variables"))
		myModuleNoticeLang(bot, u, LANG_DT_VARIABLES);
	else
		notice_lang(bot, u, UNKNOWN_COMMAND_HELP, cmd);
}

void parse_dt(User *u, ChannelInfo *ci, int global, char *cmd, char *params) {
	char *botnick = NULL;

	if (!u || !ci || !cmd)
		return;

	if (ci->bi)
		botnick = ci->bi->nick;
	else
		botnick = s_ChanServ;

	/* !dt add <+lvl> trigger reply */
	if (!stricmp(cmd, "ADD")) {
		int new = 1;
		short int pos = -1;
		int16 lvl = -1;
		char *level, *trigger, *reply, *position = NULL;
		TriggerData *td = NULL;
		TriggerEntry *te = NULL;

		if (readonly)
			myModuleNoticeLang(botnick, u, LANG_CMD_DISABLED);
		else if (!u->na)
			notice_lang(botnick, u, NICK_NOT_REGISTERED);
		else if (!nick_identified(u))
			notice_lang(botnick, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
		else if (!params)
			myModuleNoticeLang(botnick, u, global ? LANG_GDT_ADD_SYNTAX : LANG_DT_ADD_SYNTAX);
		else if ((global && !is_services_oper(u)) || !u->na->nc)
			notice_lang(botnick, u, PERMISSION_DENIED);
		else {
			if (params[0] == '+') {
				level = myStrGetToken(++params,' ', 0);
				trigger = myStrGetToken(params,' ', 1);
				reply = myStrGetTokenRemainder(params,' ', 2);
				params--;
			} else {
				level = NULL;
				trigger = myStrGetToken(params,' ', 0);
				reply = myStrGetTokenRemainder(params,' ', 1);
			}

			if (trigger && reply) {
				if (!global) {
					td = getTriggerData(ci->name);
					if (!td)
						td = createTriggerData(ci->name);
					te = getTriggerEntry(td->entries, trigger);
					if (te)
						new = 0;
					else 
						te = addTriggerEntry(td, trigger);
				} else {
					td = NULL;
					te = getGlobalTriggerEntry(trigger);
					if (te)
						new = 0;
					else
						te = addGlobalTriggerEntry(trigger);
				}
				if ((!global && !td) || !te) {
					myModuleNoticeLang(botnick, u, LANG_CANNOT_CREATE_TR);
					if (level) free(level);
					if (trigger) free(trigger);
					if (reply) free(reply);
					return;
				}

				/* We can't edit linked triggers, they have to be deleted first. */
				if (!new && te->link) {
					myModuleNoticeLang(botnick, u, LANG_DT_CANNOT_EDIT_LINKED);
					if (level) free(level);
					if (trigger) free(trigger);
					if (reply) free(reply);
					return;
				}

				if (te->count >= MaxLines) {
					myModuleNoticeLang(botnick, u, LANG_DT_MAX_REPLIES, MaxLines);
					if (level) free(level);
					if (trigger) free(trigger);
					if (reply) free(reply);
					return;
				}

				/* Make sure the user has permission to create it.. */
				if ((!global && (get_access(u, ci) < td->level_a || get_access(u, ci) < td->level_e || (get_access(u, ci) < te->level))) 
						|| ((global || (te->flags & TE_OPERONLY)) && !is_services_oper(u))) {
					notice_lang(botnick, u, PERMISSION_DENIED);
					/* Clean up allocated memory and incomplete trigger entries.. */
					if (te && new) {
						if (global)
							delGlobalTrigger(trigger);
						else
							delTrigger(td, trigger);
					}
					if (level) free(level);
					if (trigger) free(trigger);
					if (reply) free(reply);
					return;
				}

				if (level) {
					if (strspn(level, "1234567890-") == strlen(level))
						lvl = atoi(level);
					else if (!stricmp(level, "VOP"))  
						lvl = ACCESS_VOP;
					else if (!stricmp(level, "HOP"))  
						lvl = ACCESS_HOP;
					else if (!stricmp(level, "AOP"))  
						lvl = ACCESS_AOP;
					else if (!stricmp(level, "SOP"))  
						lvl = ACCESS_SOP;
					else {
						myModuleNoticeLang(botnick, u, LANG_LEVEL_INVALID);
						/* Clean up allocated memory and incomplete trigger entries.. */
						if (te && new) {
							if (global)
								delGlobalTrigger(trigger);
							else
								delTrigger(td, trigger);
						}
						if (level) free(level);
						if (trigger) free(trigger);
						if (reply) free(reply);
						return;
					}

					/* Now make sure the user is allowed to add a trigger with given level.. */
					if (!global && get_access(u, ci) < lvl) {
						notice_lang(botnick, u, PERMISSION_DENIED);
						/* Clean up allocated memory and incomplete trigger entries.. */
						if (te && new) {
							if (global)
								delGlobalTrigger(trigger);
							else
								delTrigger(td, trigger);
						}
						if (level) free(level);
						if (trigger) free(trigger);
						if (reply) free(reply);
						return;
					}
				}

				/* If it s being added to an existing trigger, check whether we have
				 * to insert at a specific position.. */
				if (!new && reply[0] == '+') {
					char *tmp = reply;
					tmp++;
					position = myStrGetToken(tmp , ' ', 0);
					reply = myStrGetTokenRemainder(tmp , ' ', 1);
					tmp--;
					free(tmp);
					if (position && strspn(position, "1234567890") == strlen(position)) {
						pos = atoi(position);
						pos--;
					} else {
						/* Clean up allocated memory and incomplete trigger entries.. */
						myModuleNoticeLang(botnick, u, LANG_DT_INV_POS, position);
						if (te && new) {
							if (global)
								delGlobalTrigger(trigger);
							else
								delTrigger(td, trigger);
						}
						if (level) free(level);
						if (trigger) free(trigger);
						if (position) free(position);
						if (reply) free(reply);
						return;
					}
				}

				/* We got the entry, now add the data to it.. */
				addToTriggerEntryAt(te, reply, pos);
				if (new) {
					/* If it s a new entry, set the creator etc.. */
					te->creator = sstrdup(u->na->nc->display);
					te->added = time(NULL);
				}
				if (te->last_upd) free(te->last_upd);
				te->last_upd = sstrdup(u->na->nc->display);
				te->last_updt = time(NULL);
				if (level) te->level = lvl;

				myModuleNoticeLang(botnick, u, global ? LANG_GDT_ADDED : LANG_DT_ADDED, trigger);
			} else
				myModuleNoticeLang(botnick, u, global ? LANG_GDT_ADD_SYNTAX : LANG_DT_ADD_SYNTAX);

			if (level) free(level);
			if (trigger) free(trigger);
			if (reply) free(reply);
		}

	/* !dt app trigger <+nr> reply */
	} else if (!stricmp(cmd, "APP") || !stricmp(cmd, "APPEND")) {
		TriggerData *td = NULL;
		TriggerEntry *te = NULL;
		char *trigger, *nr = NULL;

		if (readonly)
			myModuleNoticeLang(botnick, u, LANG_CMD_DISABLED);
		else if (!u->na)
			notice_lang(botnick, u, NICK_NOT_REGISTERED);
		else if (!nick_identified(u))
			notice_lang(botnick, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
		else if (!params || !(trigger = myStrGetToken(params,' ', 0)))
			myModuleNoticeLang(botnick, u, global ? LANG_GDT_APP_SYNTAX : LANG_DT_APP_SYNTAX);
		else if ((global && !is_services_oper(u)) || !u->na->nc)
			notice_lang(botnick, u, PERMISSION_DENIED);
		else if (!global && !(td = getTriggerData(ci->name)))
			myModuleNoticeLang(botnick, u, LANG_DT_NO_ENTRIES);
		else if ((global && !(te = getGlobalTriggerEntry(trigger))) || (td && !(te = getTriggerEntry(td->entries, trigger))))
			myModuleNoticeLang(botnick, u, LANG_DT_NO_SUCH_ENTRY);
		else if (te->link)
			myModuleNoticeLang(botnick, u, LANG_DT_CANNOT_EDIT_LINKED);
		else if (te->count > 1 && (nr = myStrGetToken(params,' ', 1)) && nr[0] != '+')
			myModuleNoticeLang(botnick, u, LANG_DT_APP_SPEC_ENTRY_NR);
		else if ((!global && ((get_access(u, ci) < td->level_a) || (get_access(u, ci) < td->level_e) || (get_access(u, ci) < te->level)))
				|| ((global || (te->flags & TE_OPERONLY)) && !is_services_oper(u)))
			notice_lang(botnick, u, PERMISSION_DENIED);
		else {
			int id = -1;
			char *data;

			if (nr && nr[0] == '+') {
				data = myStrGetTokenRemainder(params,' ', 2);
			} else {
				if (nr) free(nr);
				nr = NULL;
				data = myStrGetTokenRemainder(params,' ', 1);
			}

			if (nr) {
				nr++;
				if (strspn(nr, "1234567890") == strlen(nr))
					id = atoi(nr) - 1;
				nr--;
			}

			if ((!nr && te->count == 1) || (id >= 0 && id < te->count)) {
				appToTriggerEntry(te, id >= 0 ? id : 0, data);
				myModuleNoticeLang(botnick, u, LANG_DT_APPENDED, trigger);
			} else 
				myModuleNoticeLang(botnick, u, LANG_DT_INVALID_NR);

			if (data) free(data);
		}
		if (nr) free(nr);

	/* !dt del trigger <nr> */
	} else if (!stricmp(cmd, "DEL")) {
		TriggerData *td = NULL;
		TriggerEntry *te = NULL;
		char *trigger, *nr, *nrp;

		if (readonly)
			myModuleNoticeLang(botnick, u, LANG_CMD_DISABLED);
		else if (!u->na)
			notice_lang(botnick, u, NICK_NOT_REGISTERED);
		else if (!nick_identified(u))
			notice_lang(botnick, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
		else if (!params || !(trigger = myStrGetToken(params,' ', 0)))
			myModuleNoticeLang(botnick, u, global ? LANG_GDT_DEL_SYNTAX : LANG_DT_DEL_SYNTAX);
		else if ((global && !is_services_oper(u)) || !u->na->nc)
			notice_lang(botnick, u, PERMISSION_DENIED);
		else if (!global && !(td = getTriggerData(ci->name)))
			myModuleNoticeLang(botnick, u, LANG_DT_NO_ENTRIES);
		else if ((global && !(te = getGlobalTriggerEntry(trigger))) || (td && !(te = getTriggerEntry(td->entries, trigger))))
			myModuleNoticeLang(botnick, u, LANG_DT_NO_SUCH_ENTRY);
		else if ((!global && ((get_access(u, ci) < td->level_a) || (get_access(u, ci) < td->level_e) || (get_access(u, ci) < te->level)))
				|| ((global || (te->flags & TE_OPERONLY)) && !is_services_oper(u)))
			notice_lang(botnick, u, PERMISSION_DENIED);
		else {
			int id = -1;
			nr = myStrGetToken(params,' ', 1);
			nrp = nr + 1;

			if (nr && ((nr[0] == '+' && strspn(nrp, "1234567890") == strlen(nrp)) || (nr[0] != '+' && strspn(nr, "1234567890") == strlen(nr))))
				id = atoi(nr[0] == '+' ? nrp : nr ) - 1;

			if (!nr) {
				if (global)
					delGlobalTrigger(trigger);
				else
					delTrigger(td, trigger);
				myModuleNoticeLang(botnick, u, LANG_DT_DELETED, trigger);
			} else if (id >= 0 && id < te->count) {
				if (global)
					delGlobalTriggerDataNr(trigger, id);
				else
					delTriggerDataNr(td, trigger, id);
				myModuleNoticeLang(botnick, u, LANG_DT_DELETED_NR, id + 1, trigger);
			} else 
				myModuleNoticeLang(botnick, u, LANG_DT_INVALID_NR);

			if (nr) free(nr);
			if (trigger) free(trigger);
		}

	/* !dt link trigger target */
	} else if (!stricmp(cmd, "LINK")) {
		TriggerData *td = NULL;
		TriggerEntry *te = NULL, *tte = NULL;
		char *trigger = NULL, *target = NULL;

		if (readonly)
			myModuleNoticeLang(botnick, u, LANG_CMD_DISABLED);
		else if (!u->na)
			notice_lang(botnick, u, NICK_NOT_REGISTERED);
		else if (!nick_identified(u))
			notice_lang(botnick, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
		else if (!params || !(trigger = myStrGetToken(params,' ', 0)) || !(target = myStrGetToken(params,' ', 1)))
			myModuleNoticeLang(botnick, u, global ? LANG_GDT_LINK_SYNTAX : LANG_DT_LINK_SYNTAX);
		else if ((global && !is_services_oper(u)) || !u->na->nc)
			notice_lang(botnick, u, PERMISSION_DENIED);
		else if (!global && !(td = getTriggerData(ci->name)))
			myModuleNoticeLang(botnick, u, LANG_DT_NO_ENTRIES);
		else if ((global && !(tte = getRealGlobalTriggerEntry(target))) || (td && !(tte = getRealTriggerEntry(td->entries, target))))
			myModuleNoticeLang(botnick, u, LANG_DT_NO_SUCH_ENTRY);
		else if ((global && (te = getGlobalTriggerEntry(trigger))) || (td && (te = getTriggerEntry(td->entries, trigger))))
			myModuleNoticeLang(botnick, u, LANG_DT_EXISTS);
		else if ((!global && ((get_access(u, ci) < td->level_a) || (get_access(u, ci) < td->level_e) || (get_access(u, ci) < tte->level)))
				|| ((global || (tte->flags & TE_OPERONLY)) && !is_services_oper(u)))
			notice_lang(botnick, u, PERMISSION_DENIED);
		else {
			if (global)
				te = addGlobalTriggerEntry(trigger);
			else
				te = addTriggerEntry(td, trigger);

			if (!te) {
				myModuleNoticeLang(botnick, u, LANG_CANNOT_CREATE_TR);
				if (trigger) free(trigger);
				if (target) free(target);
				return;
			}

			linkTriggerEntries(te, tte);
			myModuleNoticeLang(botnick, u, global ? LANG_GDT_LINKED : LANG_DT_LINKED, trigger, tte->trigger);
		}
		if (trigger) free(trigger);
		if (target) free(target);

	/* !dt dup origin target */
	} else if (!stricmp(cmd, "DUP") || !stricmp(cmd, "CLONE")) {
		TriggerData *td = NULL;
		TriggerEntry *te = NULL, *tte = NULL;
		char *origin = NULL, *target = NULL;

		if (readonly)
			myModuleNoticeLang(botnick, u, LANG_CMD_DISABLED);
		else if (!u->na)
			notice_lang(botnick, u, NICK_NOT_REGISTERED);
		else if (!nick_identified(u))
			notice_lang(botnick, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
		else if (!params || !(origin = myStrGetToken(params,' ', 0)) || !(target = myStrGetToken(params,' ', 1)))
			myModuleNoticeLang(botnick, u, global ? LANG_GDT_DUP_SYNTAX : LANG_DT_DUP_SYNTAX);
		else if ((global && !is_services_oper(u)) || !u->na->nc)
			notice_lang(botnick, u, PERMISSION_DENIED);
		else if (!global && !(td = getTriggerData(ci->name)))
			myModuleNoticeLang(botnick, u, LANG_DT_NO_ENTRIES);
		else if ((global && !(te = getRealGlobalTriggerEntry(origin))) || (td && !(te = getRealTriggerEntry(td->entries, origin))))
			myModuleNoticeLang(botnick, u, LANG_DT_NO_SUCH_ENTRY);
		else if ((global && (tte = getGlobalTriggerEntry(target))) || (td && (tte = getTriggerEntry(td->entries, target))))
			myModuleNoticeLang(botnick, u, LANG_DT_EXISTS);
		else if ((!global && ((get_access(u, ci) < td->level_a) || (get_access(u, ci) < td->level_e) || (get_access(u, ci) < te->level)))
				|| ((global || (te->flags & TE_OPERONLY)) && !is_services_oper(u)))
			notice_lang(botnick, u, PERMISSION_DENIED);
		else {
			char buf[1024];

			if (global)
				tte = addGlobalTriggerEntry(target);
			else
				tte = addTriggerEntry(td, target);

			if (!tte) {
				myModuleNoticeLang(botnick, u, LANG_CANNOT_CREATE_TR);
				if (origin) free(origin);
				if (target) free(target);
				return;
			}

			duplicateTriggerEntry(te, tte);

			/* Store the original creator in brackets.. */
			snprintf(buf, 1024, "%s (Copied from %s [%s])", u->na->nc->display, origin, te->creator);
			tte->creator = sstrdup(buf);
			tte->added = time(NULL);
			tte->last_upd = sstrdup(u->na->nc->display);
			tte->last_updt = time(NULL);

			myModuleNoticeLang(botnick, u, global ? LANG_GDT_DUP : LANG_DT_DUP, origin, tte->trigger);
		}
		if (origin) free(origin);
		if (target) free(target);

	/* !dt list */ 
	} else if (!stricmp(cmd, "LIST")) {
		TriggerData *td = NULL;
		TriggerEntry *te = NULL, **list;

		if (!u->na)
			notice_lang(botnick, u, NICK_NOT_REGISTERED);
		else if (!nick_identified(u))
			notice_lang(botnick, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
		else if ((global && !is_services_oper(u)) || !u->na->nc)
			notice_lang(botnick, u, PERMISSION_DENIED);
		else if (!global && !(td = getTriggerData(ci->name)))
			myModuleNoticeLang(botnick, u, LANG_DT_NO_ENTRIES);
		else if (!global && (get_access(u, ci) < td->level_a))
			notice_lang(botnick, u, PERMISSION_DENIED);
		else {
			int i, count = -1, max = 12;
			char buf[4000], *end = NULL;

			if (global)
				list = gtriggers;
			else
				list = td->entries;

			if (global)
				myModuleNoticeLang(botnick, u, LANG_GDT_LIST_HEADER);
			else
				myModuleNoticeLang(botnick, u, LANG_DT_LIST_HEADER, ci->name);

			for (i = 0; i < 1024; i++) {
				for (te = list[i]; te; te = te->next) {
					/* First make sure user has access to the trigger.. */
					if (get_access(u, ci) < te->level || ((te->flags & TE_OPERONLY) && !is_services_oper(u)))
						continue;

					/* Build the list of triggers.. */
					if (count <= 0) {
						*buf = 0;
						end = buf;
						end += snprintf(end, sizeof(buf) - (end - buf), "%s", te->trigger);
						count = 1;
					} else {
						end += snprintf(end, sizeof(buf) - (end - buf), ", %s", te->trigger);
						count++;
					}

					/* Send out the current list and reset.. */
					if (count == max) {
						myModuleNoticeLang(botnick, u, global ? LANG_GDT_LIST : LANG_DT_LIST, buf);
						count = 0;
					}
				}
			}

			if (count > 0) {
				myModuleNoticeLang(botnick, u, global ? LANG_GDT_LIST : LANG_DT_LIST, buf);
			} else if (count < 0) {
				myModuleNoticeLang(botnick, u, global ? LANG_GDT_LIST_EMPTY : LANG_DT_LIST_EMPTY);
			}
		}

	/* !dt show trigger */
	} else if (!stricmp(cmd, "SHOW")) {
		TriggerData *td = NULL;
		TriggerEntry *te = NULL;

		if (!u->na)
			notice_lang(botnick, u, NICK_NOT_REGISTERED);
		else if (!nick_identified(u))
			notice_lang(botnick, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
		else if (!params)
			myModuleNoticeLang(botnick, u, (global ? LANG_GDT_SHOW_SYNTAX : LANG_DT_SHOW_SYNTAX));
		else if ((global && !is_services_oper(u)) || !u->na->nc)
			notice_lang(botnick, u, PERMISSION_DENIED);
		else if (!global && !(td = getTriggerData(ci->name)))
			myModuleNoticeLang(botnick, u, LANG_DT_NO_ENTRIES);
		else if ((global && !(te = getGlobalTriggerEntry(params))) || (td && !(te = getTriggerEntry(td->entries, params))))
			myModuleNoticeLang(botnick, u, LANG_DT_NO_SUCH_ENTRY);
		else if ((!global && ((get_access(u, ci) < td->level_a) || (get_access(u, ci) < te->level)))
				|| ((global || (te->flags & TE_OPERONLY)) && !is_services_oper(u)))
			notice_lang(botnick, u, PERMISSION_DENIED);
		else {
			int i;
			char buf[BUFSIZE], *end;
			struct tm tm;

			myModuleNoticeLang(botnick, u, global ? LANG_GDT_SHOW_TRIGGER : LANG_DT_SHOW_TRIGGER, params);
			tm = *localtime(&te->added);
			strftime_lang(buf, sizeof(buf), u, STRFTIME_SHORT_DATE_FORMAT, &tm);
			myModuleNoticeLang(botnick, u, LANG_DT_SHOW_CREATED, te->creator, buf);
			tm = *localtime(&te->last_updt);
			strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, &tm);
			myModuleNoticeLang(botnick, u, LANG_DT_SHOW_UPDATED, te->last_upd, buf);
			myModuleNoticeLang(botnick, u, LANG_DT_SHOW_LEVEL, te->level);

			*buf = 0;
			end = buf;
			if (te->flags & TE_LIST)
				end += snprintf(end, sizeof(buf) - (end - buf), "%s", myModuleGetLangString(u, LANG_DT_INFO_OPT_LIST));
			else
				end += snprintf(end, sizeof(buf) - (end - buf), "%s", myModuleGetLangString(u, LANG_DT_INFO_OPT_RAND));
			if (te->flags & TE_OPERONLY) {
				end += snprintf(end, sizeof(buf) - (end - buf), ", %s", myModuleGetLangString(u, LANG_DT_OPT_OPERONLY));
			}
			myModuleNoticeLang(botnick, u, LANG_DT_SHOW_FLAGS, buf);

			myModuleNoticeLang(botnick, u, LANG_DT_SHOW_ENTRIES);
			if (te->flags & TE_LIST) {
				for (i = 0; i < te->count; i++) {
					notice(botnick, u->nick, "[%d] %s", i+1, te->replies[i]->data);
					if (te->replies[i]->condition) 
						myModuleNoticeLang(botnick, u, LANG_DT_SHOW_ENTRY_LIST_COND, i+1, te->replies[i]->condition);
				}
			} else {
				for (i = 0; i < te->count; i++) {
					notice(botnick, u->nick, "[*] %s", te->replies[i]->data);
					if (te->replies[i]->condition) 
						myModuleNoticeLang(botnick, u, LANG_DT_SHOW_ENTRY_COND, te->replies[i]->condition);
				}
			}
		}

	/* !dt clear */
	} else if (!stricmp(cmd, "CLEAR")) {
		TriggerData *td = NULL;

		if (readonly)
			myModuleNoticeLang(botnick, u, LANG_CMD_DISABLED);
		else if (!u->na)
			notice_lang(botnick, u, NICK_NOT_REGISTERED);
		else if (!nick_identified(u))
			notice_lang(botnick, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
		else if ((global && !is_services_admin(u)) || !u->na->nc)
			notice_lang(botnick, u, PERMISSION_DENIED);
		else if (!global && !(td = getTriggerData(ci->name)))
			myModuleNoticeLang(botnick, u, LANG_DT_NO_ENTRIES);
		else if (!global && !is_founder(u, ci))
			notice_lang(botnick, u, PERMISSION_DENIED);
		else {
			if (!global) {
				delTriggerData(ci->name);
				myModuleNoticeLang(botnick, u, LANG_DT_CLEARED, ci->name);
			} else {
				clearGlobalTriggerEntries();
				myModuleNoticeLang(botnick, u, LANG_GDT_CLEARED);
			}
		}

	/* !dt set dtchar <trigger>
	 * !dt set privmsg on/off
	 * !dt set level access lvl
	 * !dt set level edit lvl
	 *
	 * !dt set trigger operonly on/off
	 * !dt set trigger list on/off
	 * !dt set trigger level <level>
	 * !dt set trigger cond <+nr> <condition>
	 */
	} else if (!stricmp(cmd, "SET")) {
		int type = -1, lvl, id = -1;
		char *trigger = NULL, *option = NULL, *nr = NULL, *value = NULL;
		TriggerData *td = NULL;
		TriggerEntry *te = NULL;

		if (readonly)
			myModuleNoticeLang(botnick, u, LANG_CMD_DISABLED);
		else if (!u->na)
			notice_lang(botnick, u, NICK_NOT_REGISTERED);
		else if (!nick_identified(u))
			notice_lang(botnick, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
		else if (!params || !(trigger = myStrGetToken(params,' ', 0)))
			myModuleNoticeLang(botnick, u, LANG_DT_SET_SYNTAX);
		else if ((global && !is_services_oper(u)) || !u->na->nc)
			notice_lang(botnick, u, PERMISSION_DENIED);
		else {
			/* Determine whether it s a set on a trigger or a whole chan.. */
			if (global) {
				if (!(te = getGlobalTriggerEntry(trigger)))
					myModuleNoticeLang(botnick, u, LANG_GDT_SET_UNKWN_TRIGGER);
				else if (!(option = myStrGetToken(params,' ', 1)) || (stricmp(option, "OPERONLY") && stricmp(option, "LIST") && stricmp(option, "LEVEL") && stricmp(option, "COND"))) {
					type = 0;
					myModuleNoticeLang(botnick, u, LANG_GDT_SET_UNKWN_OPTION);
				} else if (te->link) {
					type = 0;
					myModuleNoticeLang(botnick, u, LANG_DT_CANNOT_EDIT_LINKED);
				} else {
					type = 1;
					value = myStrGetToken(params,' ', 2);
				}
			} else {
				td = getTriggerData(ci->name);
				/* It s possible we are changing settings for a channel that doesn't yet exist in the DB.. */
				if (!td)
					td = createTriggerData(ci->name);
				if (trigger[0] != '+' && (!stricmp(trigger, "DTCHAR") || !stricmp(trigger, "PRIVMSG") || !stricmp(trigger, "LEVEL"))) {
					type = 2;
					if (!stricmp(trigger, "LEVEL")) {
						option = myStrGetToken(params,' ', 1);
						value = myStrGetToken(params,' ', 2);
					} else {
						option = NULL;
						value = myStrGetToken(params,' ', 1);
					}
				} else if (td && ((trigger[0] == '+' && (te = getTriggerEntry(td->entries, ++trigger))) || (trigger[0] != '+' && (te = getTriggerEntry(td->entries, trigger)))) && !te->link && 
						(option = myStrGetToken(params,' ', 1)) && (!stricmp(option, "OPERONLY") || !stricmp(option, "LIST") || !stricmp(option, "LEVEL") || !stricmp(option, "COND"))) {
					type = 1;
					if (!stricmp(option, "COND")) {
						nr = myStrGetToken(params,' ', 2);
						if (nr && nr[0] == '+') {
							nr++;
							if (strspn(nr, "1234567890") == strlen(nr))
								id = atoi(nr) - 1;
							nr--;
							value = myStrGetTokenRemainder(params,' ', 3);
						} else {
							if (nr) free(nr);
							nr = NULL;
							value = myStrGetTokenRemainder(params,' ', 2);
						}
					} else
						value = myStrGetToken(params,' ', 2);
				} else if (te && te->link) {
					type = 0;
					myModuleNoticeLang(botnick, u, LANG_DT_CANNOT_EDIT_LINKED);
				} else {
					type = 0;
					myModuleNoticeLang(botnick, u, LANG_DT_SET_UNKWN_OPTION_TRIGGER);
				}
			}

			/* Now we know what it is, check whether the user has access and do it.. */
			if (type == 1) {
				if ((!global && ((get_access(u, ci) < td->level_a) || (get_access(u, ci) < td->level_e) || (get_access(u, ci) < te->level)))
						|| ((global || (te->flags & TE_OPERONLY)) && !is_services_oper(u)))
					notice_lang(botnick, u, ACCESS_DENIED);
				else {
					if (!stricmp(option, "OPERONLY")) {
						if (value && !stricmp(value, "ON")) {
							te->flags |= TE_OPERONLY;
							myModuleNoticeLang(botnick, u, LANG_DT_SET_TR_OPERONLY_ON, te->trigger);
						} else if (value && !stricmp(value, "OFF")) {
							te->flags &= ~TE_OPERONLY;
							myModuleNoticeLang(botnick, u, LANG_DT_SET_TR_OPERONLY_OFF, te->trigger);
						} else
							myModuleNoticeLang(botnick, u, global ? LANG_GDT_SET_TR_OPERONLY_SYNTAX : LANG_DT_SET_TR_OPERONLY_SYNTAX);
					} else if (!stricmp(option, "LIST")) {
						if (value && !stricmp(value, "ON")) {
							te->flags |= TE_LIST;
							myModuleNoticeLang(botnick, u, LANG_DT_SET_TR_LIST, te->trigger);
						} else if (value && !stricmp(value, "OFF")) {
							te->flags &= ~TE_LIST;
							myModuleNoticeLang(botnick, u, LANG_DT_SET_TR_RAND, te->trigger);
						} else
							myModuleNoticeLang(botnick, u, global ? LANG_GDT_SET_TR_LIST_SYNTAX : LANG_DT_SET_TR_LIST_SYNTAX);
					} else if (!stricmp(option, "LEVEL")) {
						if (value) {
							int16 lvl = -1, err = 0;
							if (strspn(value, "1234567890-") == strlen(value))
								lvl = atoi(value);
							else if (!stricmp(value, "VOP"))  
								lvl = ACCESS_VOP;
							else if (!stricmp(value, "HOP"))  
								lvl = ACCESS_HOP;
							else if (!stricmp(value, "AOP"))  
								lvl = ACCESS_AOP;
							else if (!stricmp(value, "SOP"))  
								lvl = ACCESS_SOP;
							else {
								err = 1;
								myModuleNoticeLang(botnick, u, global ? LANG_GDT_SET_TR_LVL_SYNTAX : LANG_DT_SET_TR_LVL_SYNTAX);
							}

							/* Now make sure the user is allowed to set given level.. */
							if (!global && !err && get_access(u, ci) < lvl)
								notice_lang(botnick, u, PERMISSION_DENIED);
							else if (!err) {
								te->level = lvl;
								myModuleNoticeLang(botnick, u, LANG_DT_SET_TR_LEVEL, te->trigger, lvl);
							}
						} else
							myModuleNoticeLang(botnick, u, global ? LANG_GDT_SET_TR_LVL_SYNTAX : LANG_DT_SET_TR_LVL_SYNTAX);
					} else if (!stricmp(option, "COND")) {
						if (te->count > 1 && (!nr || id < 0))
							myModuleNoticeLang(botnick, u, LANG_DT_SET_COND_ENTRY_NR);
						else if (nr && (id < 0 || id >= te->count))
							myModuleNoticeLang(botnick, u, LANG_DT_INVALID_NR);
						else {
							if (!nr) id = 0;
							if (te->replies[id]->condition)
								free(te->replies[id]->condition);
							te->replies[id]->condition = NULL;
							if (value) {
								if (validate_condition_syntax(value)) {
									te->replies[id]->condition = sstrdup(value);
									myModuleNoticeLang(botnick, u, LANG_DT_SET_COND_SET, id + 1);
								} else
									myModuleNoticeLang(botnick, u, LANG_DT_COND_SYNTAX_ERR);
							} else
								myModuleNoticeLang(botnick, u, LANG_DT_SET_COND_CLEARED, id + 1);
						}
					} else
						myModuleNoticeLang(botnick, u, LANG_ERROR);
				}
			} else if (type == 2) {
				/* To change these settings, founder access is required. */
				if (is_founder(u, ci)) {
					if (!stricmp(trigger, "PRIVMSG")) {
						if (value && !stricmp(value, "ON")) {
							td->flags |= TD_USE_PRIVMSG;
							myModuleNoticeLang(botnick, u, LANG_DT_SET_PRIVMSG_ON, ci->name);
						} else if (value && !stricmp(value, "OFF")) {
							td->flags &= ~TD_USE_PRIVMSG;
							myModuleNoticeLang(botnick, u, LANG_DT_SET_PRIVMSG_OFF, ci->name);
						} else
							myModuleNoticeLang(botnick, u, LANG_DT_SET_PRIVMSG_SYNTAX);
					} else if (!stricmp(trigger, "DTCHAR")) {
						if (td->tchar) free(td->tchar);
						td->tchar = sstrdup(value);
						myModuleNoticeLang(botnick, u, LANG_DT_SET_DTCHAR, ci->name, td->tchar);
					} else if (!stricmp(trigger, "LEVEL")) {
						if (!stricmp(option, "ACCESS") || !stricmp(option, "EDIT")) {
							if (strspn(value, "1234567890+-") == strlen(value)) {
								lvl = atoi(value);
								if (!stricmp(option, "ACCESS")) {
									td->level_a = lvl;
									myModuleNoticeLang(botnick, u, LANG_DT_SET_LEVEL_A, ci->name, td->level_a);
								} else if (!stricmp(option, "EDIT")) {
									td->level_e = lvl;
									myModuleNoticeLang(botnick, u, LANG_DT_SET_LEVEL_E, ci->name, td->level_e);
								} else
									myModuleNoticeLang(botnick, u, LANG_ERROR);
							} else
								myModuleNoticeLang(botnick, u, LANG_DT_SET_ACCESS_INV_LVL);
						} else 
							myModuleNoticeLang(botnick, u, LANG_DT_SET_LEVEL_SYNTAX);
					}
				} else 
					notice_lang(botnick, u, ACCESS_DENIED);
			} else if (type == -1)
				myModuleNoticeLang(botnick, u, LANG_ERROR);
		}
		if (trigger) free(trigger);
		if (option) free(option);
		if (nr) free(nr);
		if (value) free(value);

	/* !dt info */
	} else if (stricmp(cmd, "INFO") == 0) {
		TriggerData *td = NULL;

		if (!u->na)
			notice_lang(botnick, u, NICK_NOT_REGISTERED);
		else if (!nick_identified(u))
			notice_lang(botnick, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
		else if ((global && !is_services_oper(u)) || !u->na->nc)
			notice_lang(botnick, u, PERMISSION_DENIED);
		else if (!global && !(td = getTriggerData(ci->name)))
			myModuleNoticeLang(botnick, u, LANG_DT_NO_ENTRIES);
		else if (!global && (get_access(u, ci) < td->level_a))
			notice_lang(botnick, u, PERMISSION_DENIED);
		else {
			if (!global) {
				myModuleNoticeLang(botnick, u, LANG_DT_INFO_COUNT, td->count, ci->name);
				if (td->tchar)
					myModuleNoticeLang(botnick, u, LANG_DT_INFO_TCHAR, ci->name, td->tchar);
				else
					myModuleNoticeLang(botnick, u, LANG_DT_INFO_TCHAR_DEF, TriggerCharacter);
				myModuleNoticeLang(botnick, u, LANG_DT_INFO_LVL_ACCESS, td->level_a);
				myModuleNoticeLang(botnick, u, LANG_DT_INFO_LVL_EDIT, td->level_e);
				if (!(td->flags & TD_USE_PRIVMSG))
					myModuleNoticeLang(botnick, u, LANG_DT_INFO_PRIVMSG_OFF);
				else
					myModuleNoticeLang(botnick, u, LANG_DT_INFO_PRIVMSG_ON);
			} else {
				myModuleNoticeLang(botnick, u, LANG_GDT_INFO_COUNT, gcount);
				myModuleNoticeLang(botnick, u, LANG_GDT_INFO_DEF_TCHAR, TriggerCharacter);
			}
		}
	}
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

	notice(botnick, u->nick, "Fantasy triggers provided by \002bs_fantasy_dt\002. [Author: \002%s\002] [Version: \002%s\002] [Flags: \002%s\002]",
			AUTHOR, VERSION, flags);

	free(flags);
}

/**
 * If a channel s been dropped, delete all associated trigger data.
 **/
int do_cs_drop(int ac, char **av) {
	if (ac != 1)
		return MOD_CONT;

	delTriggerData(av[0]);
	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

/**
 * Control functions for the Triggerdata stored about all channels..
 **/
TriggerData *createTriggerData(char *chan) {
	TriggerData *prev, *ptr, *td;

	if (!chan)
		return NULL;

	td = getTriggerData(chan);
	if (td)
		return NULL;

	td = scalloc(1, sizeof(TriggerData));
	strscpy(td->name, chan, CHANMAX);
	td->count = 0;
	td->flags = DefTdFlags;
	td->level_a = -1;
	td->level_e = DefEditLevel;
	td->tchar = NULL;

	/* Go to the right position to insert.. */
	for (prev = NULL, ptr = channels[(unsigned char) tolower(chan[1])];
			ptr != NULL && stricmp(ptr->name, chan) < 0;
			prev = ptr, ptr = ptr->next);

	/* Insert.. */
	td->prev = prev;
	td->next = ptr;
	if (!prev)
		channels[(unsigned char) tolower(chan[1])] = td;
	else
		prev->next = td;
	if (ptr)
		ptr->prev = td;

	return td;
}

TriggerData *getTriggerData(char *chan) {
	TriggerData *td;

	for(td = channels[(unsigned char) tolower(chan[1])]; 
			td != NULL && (stricmp(td->name, chan));
			td = td->next);

	return td;
}

void delTriggerData(char *chan) {
	TriggerData *td;

	if (!chan)
		return;

	td = getTriggerData(chan);
	if (!td)
		return;

	if (td->next)
		td->next->prev = td->prev;
	if (td->prev)
		td->prev->next = td->next;
	else
		channels[(unsigned char) tolower(chan[1])] = td->next;

	freeTriggerData(td);
}

void freeTriggerData(TriggerData *td) {
	clearTriggerEntries(td);

	if (td->tchar) free(td->tchar);
	free(td);
}

void clearTriggerEntries(TriggerData *td) {
	int i;
	TriggerEntry *te, *next;

	for(i = 0; i < 1024; i++) {
		for (te = td->entries[i]; te; te = next) {
			next = te->next;
			freeTriggerEntry(te);
		}
		td->entries[i] = NULL;
	}
	td->count = 0;
}

void clearGlobalTriggerEntries() {
	int i;
	TriggerEntry *te, *next;

	for(i = 0; i < 1024; i++) {
		for (te = gtriggers[i]; te; te = next) {
			next = te->next;
			freeTriggerEntry(te);
		}
		gtriggers[i] = NULL;
	}
	gcount = 0;
}

TriggerEntry *addTriggerEntry(TriggerData *td, char *trigger) {
	TriggerEntry *te;

	if (!td || !trigger)
		return NULL;

	te = createTriggerEntry(td->entries, trigger);
	if (te) td->count++;
	return te;
}

TriggerEntry *addGlobalTriggerEntry(char *trigger) {
	TriggerEntry *te;

	if (!trigger)
		return NULL;

	te = createTriggerEntry(gtriggers, trigger);
	if (te) gcount++;
	return te;
}

/**
 * Creates and inserts a TriggerEntry for given trigger in the list.
 **/
TriggerEntry *createTriggerEntry(TriggerEntry *list[], char *trigger) {
	TriggerEntry *te;
	int index = HASH(trigger);

	if (!list || !trigger)
		return NULL;

	/* Make sure it doesn't already exist.. */
	te = getTriggerEntry(list, trigger);
	if (te)
		return NULL;

	te = scalloc(1, sizeof(TriggerEntry));
	te->trigger = sstrdup(trigger);
	te->level = -1;
	te->flags = DefTeFlags;
	te->count = 0;
	te->link = NULL;
	te->links = NULL;
	te->linkcount = 0;

	te->prev = NULL;
	te->next = list[index];
	if (te->next)
		te->next->prev = te;
	list[index] = te;

	return te;
}

void addToTriggerEntry(TriggerEntry *te, char *data) {
	if (!te || !data)
		return;

	te->replies = realloc(te->replies, sizeof(TriggerReply *) * (te->count + 1));
	te->replies[te->count] = scalloc(1, sizeof(TriggerReply));
	te->replies[te->count]->data = sstrdup(data);
	te->replies[te->count]->condition = NULL;
	te->count++;
}

void addToTriggerEntryAt(TriggerEntry *te, char *data, short int position) {
	short int i;

	if (!te || !data)
		return;

	if (position < 0 || position > te->count) {
		addToTriggerEntry(te, data);
		return;
	}

	te->replies = realloc(te->replies, sizeof(TriggerReply *) * (te->count + 1));
	for (i = te->count; i > position; i--)
		te->replies[i] = te->replies[i-1];

	te->replies[position] = scalloc(1, sizeof(TriggerReply));
	te->replies[position]->data = sstrdup(data);
	te->replies[position]->condition = NULL;
	te->count++;
}

void appToTriggerEntry(TriggerEntry *te, int nr, char *data) {
	char buf[MAXVALLEN];

	if (!te || nr >= te->count || !data)
		return;

	snprintf(buf, MAXVALLEN, "%s %s", te->replies[nr]->data, data);
	free(te->replies[nr]->data);
	te->replies[nr]->data = sstrdup(buf);
}

TriggerEntry *getTriggerEntry(TriggerEntry *list[], char *trigger) {
	TriggerEntry *te;

	if (!list ||!trigger)
		return NULL;

	for (te = list[HASH(trigger)]; te; te = te->next) {
		if (stricmp(te->trigger, trigger) == 0)
			return te;
	}

	return NULL;
}

TriggerEntry *getRealTriggerEntry(TriggerEntry *list[], char *trigger) {
	TriggerEntry *te;

	if (!list ||!trigger)
		return NULL;

	for (te = list[HASH(trigger)]; te; te = te->next) {
		if (stricmp(te->trigger, trigger) == 0) {
			if (!te->link) 
				return te;
			else
				return getRealTriggerEntry(list, te->link);
		}
	}

	return NULL;
}

TriggerEntry *getGlobalTriggerEntry(char *trigger) {
	return getTriggerEntry(gtriggers, trigger);
}

TriggerEntry *getRealGlobalTriggerEntry(char *trigger) {
	return getRealTriggerEntry(gtriggers, trigger);
}

void delTrigger(TriggerData *td, char *trigger) {
	TriggerEntry *te;

	if (!td || !trigger)
		return;

	te = getTriggerEntry(td->entries, trigger);
	if (!te)
		return;

	td->count--;
	delTriggerEntry(td->entries, te);
}

void delGlobalTrigger(char *trigger) {
	TriggerEntry *te;

	if (!trigger)
		return;

	te = getGlobalTriggerEntry(trigger);
	if (!te)
		return;

	gcount--;
	delTriggerEntry(gtriggers, te);
}

void delTriggerDataNr(TriggerData *td, char *trigger, int nr) {
	int i;
	TriggerEntry *te;

	if (!td || !trigger)
		return;

	te = getTriggerEntry(td->entries, trigger);
	if (!te || nr >= te->count)
		return;

	if (te->count == 1) {
		td->count--;
		delTriggerEntry(td->entries, te);
	} else {
		free(te->replies[nr]->data);
		if (te->replies[nr]->condition) free(te->replies[nr]->condition);
		free(te->replies[nr]);
		te->replies[nr] = NULL;
		for (i = nr; i < te->count; i++)
			te->replies[i] = te->replies[i + 1];
		te->count--;
		te->replies = realloc(te->replies, sizeof(TriggerReply *) * te->count);
	}
}

void delGlobalTriggerDataNr(char *trigger, int nr) {
	int i;
	TriggerEntry *te;

	if (!trigger)
		return;

	te = getGlobalTriggerEntry(trigger);
	if (!te || nr >= te->count)
		return;

	if (te->count == 1) {
		gcount--;
		delTriggerEntry(gtriggers, te);
	} else {
		free(te->replies[nr]->data);
		if (te->replies[nr]->condition) free(te->replies[nr]->condition);
		free(te->replies[nr]);
		for (i = nr; i < te->count; i++)
			te->replies[i] = te->replies[i + 1];
		te->count--;
		te->replies = realloc(te->replies, sizeof(TriggerReply *) * te->count);
	}
}

void delTriggerEntry(TriggerEntry *list[], TriggerEntry *te) {
	TriggerEntry *link;
	int i;

	if (!list || !te)
		return;

	/* Remove us from the list.. */
	if (te->next)
		te->next->prev = te->prev;
	if (te->prev)
		te->prev->next = te->next;
	else
		list[HASH(te->trigger)] = te->next;

	/* First delete all linked entries.. */
	for (i = 0; i < te->linkcount; i++) {
		link = getTriggerEntry(list, te->links[i]);
		delTriggerEntry(list, link);
	}

	freeTriggerEntry(te);
}

void freeTriggerEntry(TriggerEntry *te) {
	int i;

	free(te->trigger);
	for (i = 0; i < te->count; i++) {
		free(te->replies[i]->data);
		if (te->replies[i]->condition) free(te->replies[i]->condition);
		free(te->replies[i]);
	}
	free(te->replies);
	free(te->link);
	for (i = 0; i < te->linkcount; i++)
		free(te->links[i]);
	free(te->links);
	if (te->creator) free(te->creator);
	if (te->last_upd) free(te->last_upd);
	free(te);
}

void linkTriggerEntries(TriggerEntry *te, TriggerEntry *target) {
	if (!te || !target)
		return;

	te->link = sstrdup(target->trigger);
	target->links = realloc(target->links, sizeof(char *) * (target->linkcount + 1));
	target->links[target->linkcount] = sstrdup(te->trigger);
	target->linkcount++;
}

void duplicateTriggerEntry(TriggerEntry *te, TriggerEntry *target) {
	int i;

	if (!te || !target)
		return;

	/* Only copy the actual data and rights, not the links.. */
	for (i = 0; i < te->count; i++) {
		addToTriggerEntry(target, te->replies[i]->data);
		if (te->replies[i]->condition) target->replies[i]->condition = sstrdup(te->replies[i]->condition);
	}

	target->level = te->level;
	target->flags = te->flags;
}

void clear_db() {
	TriggerData *td = NULL, *next = NULL;
	int i;

	/* Clear the memory.... */
	clearGlobalTriggerEntries();
	for (i = 0; i < 256; i++) {
		for (td = channels[i]; td; td = next) {
			next = td->next;
			freeTriggerData(td);
		}
		channels[i] = NULL;
	}
}

/* ------------------------------------------------------------------------------- */

void load_trigger_db(void) {
	DBFile *dbptr = scalloc(1, sizeof(DBFile));
	char *key, *value;
	int version = 0, error = 1, retval = 0, adding_globals = 0, last_data = -1;
	TriggerData *td = NULL;
	TriggerEntry *te = NULL;

	fill_db_ptr(dbptr, 0, TRIGGERDBVERSION, s_BotServ, TriggerDB);

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
			/* The first version of the DB end unexpectedly.. */
			if (version == 1) {
				error = 0;
				break;
			} else {
				free(dbptr);
				return;
			}
		} else if (retval == DB_READ_BLOCKEND) {        /* DB_READ_BLOCKEND */
			if (te) {
				te = NULL;
				last_data = -1;
			} else if (td)
				td = NULL;
			else if (adding_globals)
				adding_globals = 0;
		} else {		 /* DB_READ_SUCCESS */
			if (!*key || !*value)
				continue;

			if (!stricmp(key, "TE")) {
				if ((adding_globals && td)) {
					alog("[\002bs_fantasy_dt\002] ERROR: TE: Conflicting settings...");
					alog("[\002bs_fantasy_dt\002] Aborting database loading...");
					break;
				}
				if (!td) {
					if (debug) alog("[\002bs_fantasy_dt\002] Warning: TE: No open triggerdata found! Skipping triggerentry..");
				}
				if (te) {
					alog("[\002bs_fantasy_dt\002] ERROR: TE: Triggerentry still open!");
					alog("[\002bs_fantasy_dt\002] Aborting database loading...");
					break;
				}
				if (adding_globals)
					te = addGlobalTriggerEntry(value);
				else
					te = addTriggerEntry(td, value);
				last_data = -1;
			} else if (!stricmp(key, "c")) {
				if (!te) {
					if (debug) alog("[\002bs_fantasy_dt\002] Warning: c: No open triggerentry found! Skipping triggerentry..");
				}
				te->creator = sstrdup(value);
			} else if (!stricmp(key, "u")) {
				if (!te) {
					if (debug) alog("[\002bs_fantasy_dt\002] Warning: u: No open triggerentry found! Skipping triggerentry..");
				}
				te->last_upd = sstrdup(value);
			} else if (!stricmp(key, "a")) {
				if (!te) {
					if (debug) alog("[\002bs_fantasy_dt\002] Warning: a: No open triggerentry found! Skipping triggerentry..");
				}
				te->added = atoi(value);
			} else if (!stricmp(key, "ut")) {
				if (!te) {
					if (debug) alog("[\002bs_fantasy_dt\002] Warning: ut: No open triggerentry found! Skipping triggerentry..");
				}
				te->last_updt = atoi(value);
			} else if (!stricmp(key, "l")) {
				if (!te) {
					if (debug) alog("[\002bs_fantasy_dt\002] Warning: l: No open triggerentry found! Skipping triggerentry..");
				}
				te->level = atoi(value);
			} else if (!stricmp(key, "f")) {
				if (!te) {
					if (debug) alog("[\002bs_fantasy_dt\002] Warning: f: No open triggerentry found! Skipping triggerentry..");
				}
				te->flags = atoi(value);
			} else if (!stricmp(key, "d")) {
				if (!te) {
					if (debug) alog("[\002bs_fantasy_dt\002] Warning: d: No open triggerentry found! Skipping triggerentry..");
				}
				addToTriggerEntry(te, value);
				last_data = te->count;
			} else if (!stricmp(key, "dc")) {
				if (!te) {
					if (debug) alog("[\002bs_fantasy_dt\002] Warning: dc: No open triggerentry found! Skipping triggerentry..");
				}
				if (last_data < 0) {
					if (debug) alog("[\002bs_fantasy_dt\002] Warning: dc: No open data entry found! Skipping trigger condition..");
				}
				te->replies[last_data-1]->condition = sstrdup(value);
			/* Link entries must be loaded last.. (They should be saved at the end too..) */
			} else if (!stricmp(key, "ln")) {
				TriggerEntry *te2;
				if (!te) {
					if (debug) alog("[\002bs_fantasy_dt\002] Warning: ln: No open triggerentry found! Skipping triggerentry..");
				}
				/* If a link is not found, delete entry! */
				if (adding_globals) {
					te2 = getGlobalTriggerEntry(value);
					if (!te2) {
						if (debug) alog("[\002bs_fantasy_dt\002] Warning: ln: Target triggerentry not found! Skipping triggerentry..");
						delGlobalTrigger(te->trigger);
						te = NULL;
					}
				} else {
					te2 = getTriggerEntry(td->entries, value);
					if (!te2) {
						if (debug) alog("[\002bs_fantasy_dt\002] Warning: ln: Target triggerentry not found! Skipping triggerentry..");
						delTrigger(td, te->trigger);
						te = NULL;
					}
				}
				linkTriggerEntries(te, te2);
			} else if (!stricmp(key, "TD")) {
				if (te) {
					alog("[\002bs_fantasy_dt\002] ERROR: TD: Triggerentry still open!");
					alog("[\002bs_fantasy_dt\002] Aborting database loading...");
					break;
				}
				if (strspn(value, "1234567890") == strlen(value)) {
					if (!td) {
						alog("[\002bs_fantasy_dt\002] ERROR: Invalid value for key TD!");
						alog("[\002bs_fantasy_dt\002] Aborting database loading...");
						break;
					}
					if ((int)atoi(value) != td->count)
						alog("[\002bs_fantasy_dt\002] Warning: number of trigger found in database for channel %s differs from expected. (%d - %d)", td->name, td->count, (int)atoi(value));
				} else if (!td && cs_findchan(value)) {
					td = createTriggerData(value);
				} else if (td) {
					alog("[\002bs_fantasy_dt\002] ERROR: TD: Previous channel not yet closed!");
					alog("[\002bs_fantasy_dt\002] Aborting database loading...");
					break;
				} else {
					alog("[\002bs_fantasy_dt\002] ERROR: Nonexistent channel (%s) in DB!", value);
				}
			} else if (!stricmp(key, "TDf")) {
				if (!td) {
					alog("[\002bs_fantasy_dt\002] ERROR: TDf: No Triggerdata open!");
					alog("[\002bs_fantasy_dt\002] Aborting database loading...");
					break;
				}
				if (te) {
					alog("[\002bs_fantasy_dt\002] ERROR: TDf: Triggerentry still open!");
					alog("[\002bs_fantasy_dt\002] Aborting database loading...");
					break;
				}
				td->flags = atoi(value);
			} else if (!stricmp(key, "TDla")) {
				if (!td) {
					alog("[\002bs_fantasy_dt\002] ERROR: TDla: No Triggerdata open!");
					alog("[\002bs_fantasy_dt\002] Aborting database loading...");
					break;
				}
				if (te) {
					alog("[\002bs_fantasy_dt\002] ERROR: TDla: Triggerentry still open!");
					alog("[\002bs_fantasy_dt\002] Aborting database loading...");
					break;
				}
				td->level_a = atoi(value);
			} else if (!stricmp(key, "TDle")) {
				if (!td) {
					alog("[\002bs_fantasy_dt\002] ERROR: TDle: No Triggerdata open!");
					alog("[\002bs_fantasy_dt\002] Aborting database loading...");
					break;
				}
				if (te) {
					alog("[\002bs_fantasy_dt\002] ERROR: TDle: Triggerentry still open!");
					alog("[\002bs_fantasy_dt\002] Aborting database loading...");
					break;
				}
				td->level_e = atoi(value);
			} else if (!stricmp(key, "TDc")) {
				if (!td) {
					alog("[\002bs_fantasy_dt\002] ERROR: TDc: No Triggerdata open!");
					alog("[\002bs_fantasy_dt\002] Aborting database loading...");
					break;
				}
				if (te) {
					alog("[\002bs_fantasy_dt\002] ERROR: TDc: Triggerentry still open!");
					alog("[\002bs_fantasy_dt\002] Aborting database loading...");
					break;
				}
				if (value) td->tchar = sstrdup(value);
			} else if (!stricmp(key, "GlBls")) {
				if (strspn(value, "1234567890") == strlen(value)) {
					if (!adding_globals) {
						alog("[\002bs_fantasy_dt\002] ERROR: Received GlBls count while not adding globals!");
						alog("[\002bs_fantasy_dt\002] Aborting database loading...");
						break;
					}
					if ((int)atoi(value) != gcount)
						alog("[\002bs_fantasy_dt\002] Warning: number of global triggers found in database differs from expected. (%d - %d)", gcount, (int)atoi(value));
				} else if (!stricmp(value, "B")) {
					adding_globals = 1;
				} else {
					alog("[\002bs_fantasy_dt\002] ERROR: Invalid value for key GlBls!");
					alog("[\002bs_fantasy_dt\002] Aborting database loading...");
					break;
				}
			} else if (!stricmp(key, "TRIGGER DB VERSION")) {
				version = atoi(value);
				if (version == 1) {
					alog("[\002bs_fantasy_dt\002] Loading database of previous version. (DB version = %d; Current version = %d)", 
							version, TRIGGERDBVERSION);
				} else if (version != TRIGGERDBVERSION) {
					alog("[\002bs_fantasy_dt\002] Database version does not match any database versions supported by this module.");
					alog("[\002bs_fantasy_dt\002] Continuing with clean database...");
					break;
				}
			} else if (!stricmp(key, "EOF")) {
				if (debug && stricmp(value, VERSION))
					alog("[\002bs_fantasy_dt\002] Database saved by version '%s'.", value);
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
		alog("[\002bs_fantasy_dt\002] Starting with fresh database...");
	} else if (version == 1) {
		convert_trigger_db(version);
	}

	free(dbptr);
}


void convert_trigger_db(int version) {
	char *tmp;
	int i, j, k;
	TriggerData *td;
	TriggerEntry *te;

	/* Upgrades to go from version 1 to 2.. */
	if (version <= 1) {
		/* Go over the entire DB and replace all occurrences of $me with $n provided they are not escaped.. */
		for(j = 0; j < 1024; j++) {
			for (te = gtriggers[j]; te; te = te->next) {
				for (k = 0; k < te->count; k++) {
					/* This is not the most efficient way, but it reuses what already exists..
					 * This only ever needs to be done once in any case.. */
					tmp = te->replies[k]->data;
					te->replies[k]->data = str_replace("$me", "$n", tmp);
					free(tmp);
					if (te->replies[k]->condition) {
						tmp = te->replies[k]->condition;
						te->replies[k]->condition = str_replace("$me", "$n", tmp);
						free(tmp);
					}
				}
			}
		}

		for (i = 0; i < 256; i++) {
			for (td = channels[i]; td; td = td->next) {
				for (j = 0; j < 1024; j++) {
					for (te = td->entries[j]; te; te = te->next) {
						for (k = 0; k < te->count; k++) {
							/* This is not the most efficient way, but it reuses what already exists..
							 * This only ever needs to be done once in any case.. */
							tmp = te->replies[k]->data;
							te->replies[k]->data = str_replace("$me", "$n", tmp);
							free(tmp);
							if (te->replies[k]->condition) {
								tmp = te->replies[k]->condition;
								te->replies[k]->condition = str_replace("$me", "$n", tmp);
								free(tmp);
							}
						}
					}
				}
			}
		}
	}
}


void save_trigger_db(void) {
	DBFile *dbptr = scalloc(1, sizeof(DBFile));
	TriggerData *td;
	int i, j;

	if (!TriggerDB)
		return;

	fill_db_ptr(dbptr, 0, TRIGGERDBVERSION, s_BotServ, TriggerDB);

	/* time to backup the old db */
	rename(TriggerDB, dbptr->temp_name);

	if (new_open_db_write(dbptr)) {
		rename(dbptr->temp_name, TriggerDB);
		free(dbptr);
		return;                /* Bang, an error occurred */
	}

	/* Store the version of the DB in the DB as well...
	 * This will make stuff a lot easier if the database scheme needs to modified. */
	new_write_db_entry("TRIGGER DB VERSION", dbptr, "%d", TRIGGERDBVERSION);
	new_write_db_endofblock(dbptr);

	/* Go over the list of all global triggers.. */
	if (gcount > 0) {
		new_write_db_entry("GlBls", dbptr, "B");
		i = save_triggerlist(dbptr, gtriggers);
		new_write_db_entry("GlBls", dbptr, "%d", i);
		new_write_db_endofblock(dbptr);
		if (i != gcount)
			alog("[\002bs_fantasy_dt\002] Warning: Saved global triggers (%d) differs from triggercount(%d) !", i, gcount);
	}

	/* Save channel triggers to DB.. */
	for (i = 0; i < 256; i++) {
		for (td = channels[i]; td; td = td->next) {
			new_write_db_entry("TD", dbptr, "%s", td->name);
			new_write_db_entry("TDf", dbptr, "%d", td->flags);
			new_write_db_entry("TDla", dbptr, "%d", td->level_a);
			new_write_db_entry("TDle", dbptr, "%d", td->level_e);
			if (td->tchar)
				new_write_db_entry("TDc", dbptr, "%s", td->tchar);
			j = save_triggerlist(dbptr, td->entries);
			new_write_db_entry("TD", dbptr, "%d", j);
			new_write_db_endofblock(dbptr);
			if (j != td->count)
				alog("[\002bs_fantasy_dt\002] Warning: Saved triggers for channel %s (%d) differs from triggercount(%d) !", td->name, j, td->count);
		}
	}

	/* Write DB end tag.. */
	new_write_db_entry("EOF", dbptr, "%s", VERSION);

	if (dbptr) {
		new_close_db(dbptr->fptr, NULL, NULL);  /* close file */
		remove(dbptr->temp_name);       /* saved successfully, no need to keep the old one */
		free(dbptr);           /* free the db struct */
	}
}


int save_triggerlist(DBFile *dbptr, TriggerEntry *list[]) {
	int i, j;
	TriggerEntry *te;

	for (i = 0, j = 0; i < 1024; i++) {
		for (te = list[i]; te; te = te->next) {
			/* Do NOT save linked entries untill all normal entries have been saved... */
			if (!te->link) {
				save_triggerentry(dbptr, te);
				j++;
			}
		}
	}

	/* Go over the list again to save linked entries... */
	for (i = 0; i < 1024; i++) {
		for (te = list[i]; te; te = te->next) {
			if (te->link) {
				save_triggerentry(dbptr, te);
				j++;
			}
		}
	}

	return j;
}


void save_triggerentry(DBFile *dbptr, TriggerEntry *te) {
	int i;
	new_write_db_entry("TE", dbptr, "%s", te->trigger);
	new_write_db_entry("c", dbptr, "%s", te->creator);
	new_write_db_entry("u", dbptr, "%s", te->last_upd);
	new_write_db_entry("a", dbptr, "%d", te->added);
	new_write_db_entry("ut", dbptr, "%d", te->last_updt);
	new_write_db_entry("l", dbptr, "%d", te->level);
	new_write_db_entry("f", dbptr, "%d", te->flags);
	if (te->link)
		new_write_db_entry("ln", dbptr, "%s", te->link);
	for (i = 0; i < te->count; i++) {
		new_write_db_entry("d", dbptr, "%s", te->replies[i]->data);
		if (te->replies[i]->condition)
			new_write_db_entry("dc", dbptr, "%s", te->replies[i]->condition);
	}
	new_write_db_endofblock(dbptr);
}

/* ------------------------------------------------------------------------------- */

/**
 * When anope saves her databases, we do the same.
 **/
int do_save(int argc, char **argv) {
	int old_supported = supported;
	
	if ((argc >= 1) && (!stricmp(argv[0], EVENT_STOP)))
		save_trigger_db();

	check_modules();
	if (supported != old_supported) {
		if (supported == 0) {
			alog("[\002bs_fantasy_dt\002] Warning: Module continueing in unsupported mode!");
		} else if (supported == -1) {
			alog("[\002bs_fantasy_dt\002] Disabling module due to incompatibilities!");
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
		alog("[bs_fantasy_dt] Backing up TRIGGER database...");
		ModuleDatabaseBackup(TriggerDB);
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
			alog("[\002bs_fantasy_dt\002] Unsupported module found: os_raw.. (This is fatal!)");
			supported = -1;
		}
		if (findCommand(OPERSERV, "RAW")) {
			alog("[\002bs_fantasy_dt\002] Unsupported module found: os_raw.. (This is fatal!)");
			supported = -1;
		}
		if (!DisableRaw) {
			alog("[\002bs_fantasy_dt\002] RAW has NOT been disabled! (This is fatal!)");
			supported = -1;
		}
	}

	if (supported >= 0) {
		if (findModule("ircd_init")) {
			alog("[\002bs_fantasy_dt\002] This module is unsupported in combination with ircd_init.");
			supported = 0;
		}

		if (findModule("cs_join")) {
			alog("[\002bs_fantasy_dt\002] This module is unsupported in combination with cs_join.");
			supported = 0;
		}

		if (findModule("bs_logchanmon")) {
			alog("[\002bs_fantasy_dt\002] This module is unsupported in combination with bs_logchanmon.");
			supported = 0;
		}

		if (findModule("ircd_gameserv")) {
			alog("[\002bs_fantasy_dt\002] This module is unsupported in combination with ircd_gameserv.");
			supported = 0;
		}

		if (findModule("os_psuedo_cont")) {
			alog("[\002bs_fantasy_dt\002] This module is unsupported in combination with os_psuedo_cont.");
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
			alog("[\002bs_fantasy_dt\002] Unsupported module found: %s.. (This is fatal!)", argv[0]);
			supported = -1;
		}
		if (!stricmp(argv[0], "ircd_init") || !stricmp(argv[0], "cs_join") || !stricmp(argv[0], "bs_logchanmon")
				|| !stricmp(argv[0], "ircd_gameserv") || !stricmp(argv[0], "os_psuedo_cont")) {
			alog("[\002bs_fantasy_dt\002] This module is unsupported in combination with %s.", argv[0]);
			supported = 0;
		}

		if (supported != old_supported) {
			if (supported == 0) {
				alog("[\002bs_fantasy_dt\002] Warning: Module continuing in unsupported mode!");
			} else if (supported == -1) {
				alog("[\002bs_fantasy_dt\002] Disabling module due to incompatibilities!");
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
			alog("[\002bs_fantasy_dt\002] Unsupported command found: %s.. (This is fatal!)", argv[0]);
			supported = -1;
		}

		if (supported != old_supported) {
			if (supported == 0) {
				alog("[\002bs_fantasy_dt\002] Warning: Module continuing in unsupported mode!");
			} else if (supported == -1) {
				alog("[\002bs_fantasy_dt\002] Disabling module due to incompatibilities!");
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
char* get_flags() {
	char tmp[BUFSIZE];
	const char version_flags[] = " " VER_DEBUG VER_OS VER_MYSQL VER_MODULE;
	char *flags;
	
#ifdef SUPPORTED
	snprintf(tmp, BUFSIZE, "%s-%s-T%d-L%d-RMS%d-RMR%d", version_flags, ((supported == -1) ? "U" : 
			(supported == 0) ? "u" : "S"), MaxTriggers, MaxLines, BSTriggerRedirMinSendInterval,
			BSTriggerRedirMinRecvInterval);
#else
	snprintf(tmp, BUFSIZE, "%s-HU-T%d-L%d-RMS%d-RMR%d", version_flags, MaxTriggers, MaxLines,
			BSTriggerRedirMinSendInterval, BSTriggerRedirMinRecvInterval);
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
		m = findModule("bs_fantasy_dt");

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
		{{"BSTriggerDB", {{PARAM_STRING, PARAM_RELOAD, &TriggerDB}}}},
		{{"BSTriggerCharacter", {{PARAM_STRING, PARAM_RELOAD, &TriggerCharacter}}}},
		{{"BSTriggerRedirMinSendInterval", {{PARAM_TIME, PARAM_RELOAD, &BSTriggerRedirMinSendInterval}}}},
		{{"BSTriggerRedirMinRecvInterval", {{PARAM_TIME, PARAM_RELOAD, &BSTriggerRedirMinRecvInterval}}}},
	};

	if (TriggerDB)
		free(TriggerDB);
	TriggerDB = NULL;
	if (TriggerCharacter)
		free(TriggerCharacter);
	TriggerCharacter = NULL;
	BSTriggerRedirMinSendInterval = 0;
	BSTriggerRedirMinRecvInterval = 0;

	for (i = 0; i < 4; i++)
		moduleGetConfigDirective(confvalues[i]);

	if (!TriggerDB)
		TriggerDB = sstrdup(DefTriggerDB);

	if (!TriggerCharacter)
		 TriggerCharacter = sstrdup(BSFantasyCharacter);

	if (debug)
		alog ("[bs_fantasy_dt] debug: TriggerDB set to %s. TriggerCharacter is %s. BSTriggerRedirMinSendInterval=%d. BSTriggerRedirMinRecvInterval=%d.",
				TriggerDB, TriggerCharacter, BSTriggerRedirMinSendInterval, BSTriggerRedirMinRecvInterval);
}


/**
 * Upon /os reload call the routines for reloading the configuration directives
 **/
int reload_config(int argc, char **argv) {
	int old_supported = supported;

	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			if (debug)
				alog("[bs_fantasy_dt] debug: Reloading configuration directives...");
			load_config();
		}
	}

	check_modules();
	if (supported != old_supported) {
		if (supported == 0) {
			alog("[\002bs_fantasy_dt\002] Warning: Module continueing in unsupported mode!");
		} else if (supported == -1) {
			alog("[\002bs_fantasy_dt\002] Disabling module due to incompatibilities!");
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
		/* LANG_TRIGGER_HELP */
		" To query the trigger database use \037%strigger [arguments] [> redirect nicknames]\037.",
		/* LANG_DT_DESC */
		" Manage channel specific triggers and settings.",
		/* LANG_GDT_DESC */
		" Manage global triggers.",
		/* LANG_DT_SYNTAX */
		" Syntax: %BSFdt ADD [+lvl] trigger [+nr] reply\n"
		" Syntax: %BSFdt { APP | DEL } trigger [+nr] [reply]\n"
		" Syntax: %BSFdt { LINK | DUP } trigger target\n"
		" Syntax: %BSFdt SET trigger { LIST | OPERONLY | LEVEL | COND } { { ON | OFF } | level | +nr } [ condition ]\n"
		" Syntax: %BSFdt SET { DTCHAR | PRIVMSG | LEVEL } { trigger char | { ON | OFF } | { ACCESS | EDIT } } [level]\n"
		" Syntax: %BSFdt { SHOW | LIST | CLEAR | INFO } [trigger]\n",
		/* LANG_GDT_SYNTAX */
		" Syntax: %BSFgdt ADD [+lvl] trigger [+nr] reply\n"
		" Syntax: %BSFgdt { APP | DEL } trigger [+nr] [reply]\n"
		" Syntax: %BSFgdt { LINK | DUP } trigger target\n"
		" Syntax: %BSFgdt SET trigger { LIST | OPERONLY | LEVEL | COND } { { ON | OFF } | level | +nr } [ condition ]\n"
		" Syntax: %BSFgdt { SHOW | LIST | CLEAR | INFO } [trigger]\n",
		/* LANG_DT_SYNTAX_EXT */
		" Syntax: %BSFdt ADD [+lvl] trigger [+nr] reply\n"
		" Syntax: %BSFdt APP trigger [+nr] reply\n"
		" Syntax: %BSFdt LINK trigger target\n"
		" Syntax: %BSFdt DUP trigger target\n"
		" Syntax: %BSFdt DEL trigger [nr]\n"
		" Syntax: %BSFdt SET trigger { LIST | OPERONLY | LEVEL | COND } { { ON | OFF } | level | +nr } [ condition ]\n"
		" Syntax: %BSFdt SET { DTCHAR | PRIVMSG | LEVEL } { trigger char | { ON | OFF } | { ACCESS | EDIT } } [level]\n"
		" Syntax: %BSFdt SHOW trigger\n"
		" Syntax: %BSFdt { LIST | CLEAR | INFO }\n"
		" \n"
		" This command controls the list of dynamic triggers specific to a channel.\n"
		" Channel settings can be changed and triggers and replies can be addded, edited and deleted through\n"
		" this command. When a trigger preceded by the trigger character is said in channel, services will reply to it\n"
		" with the stored reply, or with a random one if multiple answers are available and random selection is turned on.\n"
		" \n"
		" DT ADD adds a new trigger and trigger reply to the channel's database or adds a reply to an existing\n"
		" trigger. Optionally, a minimum level can be specified for users to be able to activate the trigger.\n"
		" The level is given as +value or +SOP/AOP/HOP/VOP. If no level is specified no additional\n"
		" restrictions will be applied to this trigger. Note that the DT SET LEVEL ACCESS option always applies.\n"
		" A position can be specified with +number to insert a reply at a specific position in a list.\n"
		" \n"
		" DT APP appends an existing trigger reply. If a trigger has multiple replies a number specifying which reply\n"
		" to append is mandatory.\n"
		" \n"
		" DT LINK links a trigger to another target trigger creating an alias.\n"
		" \n"
		" DT DUP duplicates the contents of trigger into a newly created target trigger.\n"
		" \n"
		" DT DEL deletes a trigger from the database. If a reply number is specified only that reply will be deleted\n"
		" leaving the other replies associated with that trigger intact. If no number is specified the trigger and\n"
		" and all data associated with it will be deleted.\n"
		" \n"
		" DT SET trigger configures trigger specific options. Following options are available: LIST, OPERONLY, LEVEL\n"
		" and COND. The first 2 can be set to either ON or OFF while LEVEL can be given as a numerical value or as SOP/AOP/HOP/VOP.\n"
		" LIST determines whether all trigger replies will be seen as a single reply (ON) or as a collection of possible replies\n"
		" from which one is selected randomly (OFF). OPERONLY determines whether a trigger should be accessible to opers only.\n"
		" LEVEL allows updating a triggers' level, the same level that can be specified when a trigger is added.\n"
		" COND enables a condition to be set which a trigger reply must satisfy if it is to be send.\n"
		" \n"
		" DT SET also configures a number of non-trigger-specific channel settings. Following options are available:\n"
		" DTCHAR, PRIVMSG and LEVEL. The DTCHAR option allows you to set the character that will initiate\n"
		" a trigger lookup on your channel. PRIVMSG determines whether the botserv bot will answer a query in a NOTICE to \n"
		" the channel (OFF) or in a regular channel message (ON). The LEVEL option allows for a minimum level to be specified\n"
		" for accessing and editing entries in this channel. The LEVEL option takes an additional parameter ACCESS or EDIT and a\n"
		" numeric value indicating the level or SOP/AOP/HOP/VOP which will be automatically converted into a  numerical value.\n"
		" \n"
		" DT SHOW lists all data regarding a trigger.\n"
		" \n"
		" DT LIST lists all local triggers available in this channel.\n"
		" \n"
		" DT CLEAR clears all triggers associated with this channel from the database.\n"
		" \n"
		" DT INFO displays general statistics regarding the trigger database of this channel.\n"
		" \n"
		" Conditions and variables may be used in and defined on replies. See %BSFhelp DT CONDITIONS for more info.",
		/* LANG_GDT_SYNTAX_EXT */
		" Syntax: %BSFgdt ADD [+lvl] trigger [+nr] reply\n"
		" Syntax: %BSFgdt APP trigger [+nr] reply\n"
		" Syntax: %BSFgdt LINK trigger target\n"
		" Syntax: %BSFgdt DUP trigger target\n"
		" Syntax: %BSFgdt DEL trigger [nr]\n"
		" Syntax: %BSFgdt SET trigger { LIST | OPERONLY | LEVEL | COND } { { ON | OFF } | level | +nr } [ condition ]\n"
		" Syntax: %BSFgdt SET { DTCHAR | PRIVMSG | LEVEL } { trigger char | { ON | OFF } | { ACCESS | EDIT } } [level]\n"
		" Syntax: %BSFgdt SHOW trigger\n"
		" Syntax: %BSFgdt { LIST | CLEAR | INFO }\n"
		" \n"
		" This command controls the global list of dynamic triggers. These triggers will be available in\n"
		" all channels on the network where triggers can be accessed."
		" Triggers and replies can be addded, edited and deleted through this command.\n"
		" When a trigger preceded by the trigger character is said in channel, services will reply to it\n"
		" with the stored reply, or with a random one if multiple answers are available and random selection is turned on.\n"
		" \n"
		" GDT ADD adds a new trigger and trigger reply to the database or adds a reply to an existing\n"
		" trigger. Optionally, a minimum level can be specified for users to be able to activate the trigger.\n"
		" The level is given as +value or +SOP/AOP/HOP/VOP. If no level is specified no additional\n"
		" restrictions will be applied to this trigger. Note that the DT SET LEVEL ACCESS option always applies.\n"
		" A position can be specified with +number to insert a reply at a specific position in a list.\n"
		" \n"
		" GDT APP appends an existing trigger reply. If a trigger has multiple replies a number specifying which reply\n"
		" to append is mandatory.\n"
		" \n"
		" GDT LINK links a trigger to another target trigger creating an alias.\n"
		" \n"
		" GDT DUP duplicates the contents of trigger into a newly created target trigger.\n"
		" \n"
		" GDT DEL deletes a trigger from the database. If a reply number is specified only that reply will be deleted\n"
		" leaving the other replies associated with that trigger intact. If no number is specified the trigger and\n"
		" and all data associated with it will be deleted.\n"
		" \n"
		" GDT SET trigger configures trigger specific options. Following options are available: LIST, OPERONLY, LEVEL\n"
		" and COND. The first 2 can be set to either ON or OFF while LEVEL can be given as a numerical value or as SOP/AOP/HOP/VOP.\n"
		" LIST determines whether all trigger replies will be seen as a single reply (ON) or as a collection of possible replies\n"
		" from which one is selected randomly (OFF). OPERONLY determines whether a trigger should be accessible to opers only.\n"
		" LEVEL allows updating a triggers' level, the same level that can be specified when a trigger is added.\n"
		" COND enables a condition to be set which a trigger reply must satisfy if it is to be send.\n"
		" \n"
		" GDT SHOW lists all data regarding a trigger.\n"
		" \n"
		" GDT LIST lists all global triggers available on this network.\n"
		" \n"
		" GDT CLEAR clears all global triggers from the database.\n"
		" \n"
		" GDT INFO displays general statistics regarding the global trigger database.\n"
		" \n"
		" Conditions and variables may be used in and defined on replies. See %BSFhelp GDT CONDITIONS for more info.\n"
		" \n"
		" Some command options are limited to IRC Operators.",
		/* LANG_DT_ADD_SYNTAX */
		" Syntax: %BSFdt ADD [+lvl] trigger [+nr] reply",
		/* LANG_DT_ADD_SYNTAX_EXT */
		" Syntax: %BSFdt ADD [+lvl] trigger [+nr] reply\n"
		" \n"
		" DT ADD adds a new trigger and trigger reply to the channel's database or adds a reply to an existing\n"
		" trigger. Optionally, a minimum level can be specified for users to be able to activate the trigger.\n"
		" The level is given as +value or +SOP/AOP/HOP/VOP. If no level is specified no additional\n"
		" restrictions will be applied to this trigger. \n"
		" A position can be specified with +number to insert a reply at a specific position in a list.\n"
		" \n"
		" Conditions and variables may be used in replies. See %BSFhelp DT CONDITIONS for more info.\n"
		" \n"
		" Note that the DT SET LEVEL ACCESS option always applies.",
		/* LANG_DT_APP_SYNTAX */
		" Syntax: %BSFdt APP trigger [+nr] reply",
		/* LANG_DT_APP_SYNTAX_EXT */
		" Syntax: %BSFdt APP trigger [+nr] reply\n"
		" \n"
		" DT APP appends an existing trigger reply. If a trigger has multiple replies a number specifying which reply\n"
		" to append is mandatory.",
		/* LANG_DT_DEL_SYNTAX */
		" Syntax: %BSFdt DEL trigger [nr]",
		/* LANG_DT_DEL_SYNTAX_EXT */
		" Syntax: %BSFdt DEL trigger [nr]\n"
		" \n"
		" DT DEL deletes a trigger from the database. If a reply number is specified only that reply will be deleted\n"
		" leaving the other replies associated with that trigger intact. If no number is specified the trigger and\n"
		" and all data associated with it will be deleted.",
		/* LANG_DT_SET_SYNTAX */
		" Syntax: %BSFdt SET trigger { LIST | OPERONLY | LEVEL | COND } { { ON | OFF } | level | +nr } [ condition ]\n"
		" Syntax: %BSFdt SET { DTCHAR | PRIVMSG | LEVEL } { trigger char | { ON | OFF } | { ACCESS | EDIT } } [level]",
		/* LANG_DT_SET_SYNTAX_EXT */
		" Syntax: %BSFdt SET trigger LIST { ON | OFF }\n"
		" Syntax: %BSFdt SET trigger OPERONLY { ON | OFF }\n"
		" Syntax: %BSFdt SET trigger LEVEL [level]\n"
		" Syntax: %BSFdt SET trigger COND [+nr] [condition]\n"
		" \n"
		" Syntax: %BSFdt SET DTCHAR trigger char\n"
		" Syntax: %BSFdt SET PRIVMSG { ON | OFF }\n"
		" Syntax: %BSFdt SET LEVEL { ACCESS | EDIT } [level]\n"
		" \n"
		" DT SET trigger configures trigger specific options. Following options are available: LIST, OPERONLY and \n"
		" LEVEL. The first 2 can be set to either ON or OFF while LEVEL can be given as a numerical value or as SOP/AOP/HOP/VOP.\n"
		" LIST determines whether all trigger replies will be seen as a single reply (ON) or s a collection of possible replies from which\n"
		" one is selected randomly (OFF). OPERONLY determines whether a trigger should be accessible to opers only.\n"
		" LEVEL allows updating a triggers' level, the same level that can be specified when a trigger is added.\n"
		" COND enables a condition to be set which a trigger reply must satisfy if it is to be send. See %BSFhelp DT CONDITIONS for more info.\n"
		" \n"
		" DT SET also configures a number of non-trigger-specific channel settings. \n"
		" Following options are available: DTCHAR, PRIVMSG and LEVEL.\n"
		" The DTCHAR option allows you to set the character that will initiate a trigger lookup on your channel.\n"
		" PRIVMSG determines whether the botserv bot will answer a query in a NOTICE to the channel (OFF) or in a regular channel message (ON).\n"
		" The LEVEL option allows for a minimum level to be specified for accessing and editing entries in this channel.\n"
		" The LEVEL option takes an additional parameter ACCESS or EDIT and a numeric value indicating the level or \n"
		" SOP/AOP/HOP/VOP which will be automatically converted into a  numerical value.",
		/* LANG_DT_SET_TR_OPERONLY_SYNTAX */
		" Syntax: %BSFdt SET trigger OPERONLY { ON | OFF }",
		/* LANG_DT_SET_TR_LIST_SYNTAX */
		" Syntax: %BSFdt SET trigger LIST { ON | OFF }",
		/* LANG_DT_SET_DTCHAR_SYNTAX */
		" Syntax: %BSFdt SET DTCHAR trigger char",
		/* LANG_DT_SET_PRIVMSG_SYNTAX */
		" Syntax: %BSFdt SET PRIVMSG { ON | OFF }",
		/* LANG_DT_SET_LEVEL_SYNTAX */
		" Syntax: %BSFdt SET LEVEL { ACCESS | EDIT } [level]",
		/* LANG_DT_LIST_SYNTAX */
		" Syntax: %BSFdt LIST",
		/* LANG_DT_LIST_SYNTAX_EXT */
		" Syntax: %BSFdt LIST\n"
		" \n"
		" DT LIST lists all local triggers available in this channel.",
		/* LANG_DT_SHOW_SYNTAX */
		" Syntax: %BSFdt SHOW trigger",
		/* LANG_DT_SHOW_SYNTAX_EXT */
		" Syntax: %BSFdt SHOW trigger\n"
		" \n"
		" DT SHOW lists all data regarding a trigger.",
		/* LANG_DT_CLEAR_SYNTAX */
		" Syntax: %BSFdt CLEAR",
		/* LANG_DT_CLEAR_SYNTAX_EXT */
		" Syntax: %BSFdt CLEAR\n"
		" \n"
		" DT CLEAR clears all triggers associated with this channel from the database.",
		/* LANG_DT_INFO_SYNTAX */
		" Syntax: %BSFdt INFO",
		/* LANG_DT_INFO_SYNTAX_EXT */
		" Syntax: %BSFdt INFO\n"
		" \n"
		" DT INFO displays general statistics regarding the trigger database of this channel.",
		/* LANG_GDT_ADD_SYNTAX */
		" Syntax: %BSFgdt ADD [+lvl] trigger [+nr] reply",
		/* LANG_GDT_ADD_SYNTAX_EXT */
		" Syntax: %BSFgdt ADD [+lvl] trigger [+nr] reply\n"
		" \n"
		" GDT ADD adds a new trigger and trigger reply to the database or adds a reply to an existing\n"
		" trigger. Optionally, a minimum level can be specified for users to be able to activate the trigger.\n"
		" The level is given as +value or +SOP/AOP/HOP/VOP. If no level is specified no additional\n"
		" restrictions will be applied to this trigger. \n"
		" A position can be specified with +number to insert a reply at a specific position in a list.\n"
		" \n"
		" Conditions and variables may be used in replies. See %BSFhelp GDT CONDITIONS for more info.\n"
		" \n"
		" Note that the DT SET LEVEL ACCESS option always applies.",
		/* LANG_GDT_APP_SYNTAX */
		" Syntax: %BSFgdt APP trigger [+nr] reply",
		/* LANG_GDT_APP_SYNTAX_EXT */
		" Syntax: %BSFgdt APP trigger [+nr] reply\n"
		" \n"
		" GDT APP appends an existing trigger reply. If a trigger has multiple replies a number specifying which reply\n"
		" to append is mandatory.",
		/* LANG_GDT_DEL_SYNTAX */
		" Syntax: %BSFgdt DEL trigger [nr]",
		/* LANG_GDT_DEL_SYNTAX_EXT */
		" Syntax: %BSFgdt DEL trigger [nr]\n"
		" \n"
		" GDT DEL deletes a trigger from the database. If a reply number is specified only that reply will be deleted\n"
		" leaving the other replies associated with that trigger intact. If no number is specified the trigger and\n"
		" and all data associated with it will be deleted.",
		/* LANG_GDT_SET_SYNTAX */
		" Syntax: %BSFgdt SET trigger { LIST | OPERONLY | LEVEL | COND } { { ON | OFF } | level | +nr } [ condition ]",
		/* LANG_GDT_SET_SYNTAX_EXT */
		" Syntax: %BSFgdt SET trigger LIST { ON | OFF }\n"
		" Syntax: %BSFgdt SET trigger OPERONLY { ON | OFF }\n"
		" Syntax: %BSFgdt SET trigger LEVEL [level]\n"
		" Syntax: %BSFgdt SET trigger COND [+nr] [condition]\n"
		" \n"
		" GDT SET trigger configures trigger specific options. Following options are available: LIST,  OPERONLY, LEVEL \n"
		" and COND. The first 2 can be set to either ON or OFF while LEVEL can be given as a numerical value or as SOP/AOP/HOP/VOP.\n"
		" LIST determines whether all trigger replies will be seen as a single reply (ON) or s a collection of possible replies from which\n"
		" one is selected randomly (OFF). OPERONLY determines whether a trigger should be accessible to opers only.\n"
		" LEVEL allows updating a triggers' level, the same level that can be specified when a trigger is added.\n"
		" COND enables a condition to be set which a trigger reply must satisfy if it is to be send. See %BSFhelp GDT CONDITIONS for more info.",
		/* LANG_GDT_SET_TR_OPERONLY_SYNTAX */
		" Syntax: %BSFgdt SET trigger OPERONLY { ON | OFF }",
		/* LANG_GDT_SET_TR_LIST_SYNTAX */
		" Syntax: %BSFgdt SET trigger LIST { ON | OFF }",
		/* LANG_GDT_LIST_SYNTAX */
		" Syntax: %BSFgdt LIST",
		/* LANG_GDT_LIST_SYNTAX_EXT */
		" Syntax: %BSFgdt LIST\n"
		" \n"
		" GDT LIST lists all global triggers available on this network.",
		/* LANG_GDT_SHOW_SYNTAX */
		" Syntax: %BSFgdt SHOW trigger",
		/* LANG_GDT_SHOW_SYNTAX_EXT */
		" Syntax: %BSFgdt SHOW trigger\n"
		" \n"
		" GDT SHOW lists all data regarding a trigger.",
		/* LANG_GDT_CLEAR_SYNTAX */
		" Syntax: %BSFgdt CLEAR",
		/* LANG_GDT_CLEAR_SYNTAX_EXT */
		" Syntax: %BSFgdt CLEAR\n"
		" \n"
		" GDT CLEAR clears all global triggers from the database.",
		/* LANG_GDT_INFO_SYNTAX */
		" Syntax: %BSFgdt INFO",
		/* LANG_GDT_INFO_SYNTAX_EXT */
		" Syntax: %BSFgdt INFO\n"
		" \n"
		" GDT INFO displays general statistics regarding the global trigger database.",
		/* LANG_ERROR */
		" An unforeseen error has occured. Please report this incident.",
		/* LANG_CMD_DISABLED */
		" This command is currently disabled.",
		/* LANG_CMD_NOT_AVAILABLE */
		" This command is currently not available.",
		/* LANG_CANNOT_CREATE_TR */
		" Could not create trigger! Please report this problem.",
		/* LANG_LEVEL_INVALID */
		" Invalid level specified..",
		/* LANG_DT_ADDED */
		" Added data for trigger %s.",
		/* LANG_GDT_ADDED */
		" Added data for trigger %s.",
		/* LANG_DT_NO_ENTRIES */
		" Trigger list for this channel is empty.",
		/* LANG_DT_NO_SUCH_ENTRY */
		" Trigger not found.",
		/* LANG_DT_APP_SPEC_ENTRY_NR */
		" This trigger has multiple entries associated, an entry number must therefore be specified.",
		/* LANG_DT_APPENDED */
		" Data appended to %s.",
		/* LANG_DT_INVALID_NR */
		" Triggerentry or specified reply does not exist.",
		/* LANG_DT_DELETED */
		" Trigger %s has been removed from the list.",
		/* LANG_DT_DELETED_NR */
		" Reply %d of trigger %s has been removed from the list.",
		/* LANG_DT_LIST_HEADER */
		" Triggers for %s:",
		/* LANG_GDT_LIST_HEADER */
		" Available global triggers:",
		/* LANG_DT_LIST */
		" %s",
		/* LANG_GDT_LIST */
		" %s",
		/* LANG_DT_LIST_EMPTY */
		" The trigger list is empty.",
		/* LANG_GDT_LIST_EMPTY */
		" No global triggers are available.",
		/* LANG_DT_SHOW_TRIGGER */
		" Summary for trigger %s:",
		/* LANG_GDT_SHOW_TRIGGER */
		" Summary for trigger %s:",
		/* LANG_DT_SHOW_CREATED */
		" Created by %s on %s.",
		/* LANG_DT_SHOW_UPDATED */
		" Last updated by %s on %s.",
		/* LANG_DT_SHOW_LEVEL */
		" Required access level: %d",
		/* LANG_DT_INFO_OPT_LIST */
		"List (Show all replies)",
		/* LANG_DT_INFO_OPT_RAND */
		"Random (Show random reply)",
		/* LANG_DT_OPT_OPERONLY */
		"Opers Only",
		/* LANG_DT_SHOW_FLAGS */
		" Options: %s",
		/* LANG_DT_SHOW_ENTRIES */
		" Trigger replies:",
		/* LANG_DT_CLEARED */
		" All triggers for channel %s have been deleted.",
		/* LANG_GDT_CLEARED */
		" All global triggers have been deleted.",
		/* LANG_GDT_SET_UNKWN_TRIGGER */
		" Trigger not found.",
		/* LANG_GDT_SET_UNKWN_OPTION */
		" Invalid option.",
		/* LANG_DT_SET_UNKWN_OPTION_TRIGGER */
		" Unknown trigger or invalid option.",
		/* LANG_DT_SET_TR_OPERONLY_ON */
		" Trigger %s is now restricted to IRC Operators.",
		/* LANG_DT_SET_TR_OPERONLY_OFF */
		" Trigger %s is no longer restricted to IRC Operators.",
		/* LANG_DT_SET_TR_LIST */
		" All replies for trigger %s will now be shown.",
		/* LANG_DT_SET_TR_RAND */
		" A random reply for trigger %s will now be selected.",
		/* LANG_DT_SET_PRIVMSG_ON */
		" Triggers in %s will now be answered in a channel PRIVMSG.",
		/* LANG_DT_SET_PRIVMSG_OFF */
		" Triggers in %s will now be answered in a channel NOTICE.",
		/* LANG_DT_SET_DTCHAR */
		" Trigger character for channel %s has now been set to %s.",
		/* LANG_DT_SET_LEVEL_A */
		" Access level required to perform trigger queries in %s is now set to %d.",
		/* LANG_DT_SET_LEVEL_E */
		" Access level required to perform trigger updates in %s is now set to %d.",
		/* LANG_DT_SET_ACCESS_INV_LVL */
		" Invalid level specified.",
		/* LANG_DT_INFO_COUNT */
		" %d local triggers are available in channel %s.",
		/* LANG_GDT_INFO_COUNT */
		" %d global triggers are available.",
		/* LANG_DT_INFO_TCHAR */
		" Trigger character for %s set to: %s",
		/* LANG_DT_INFO_LVL_ACCESS */
		" Channel access level required to access triggers: %d",
		/* LANG_DT_INFO_LVL_EDIT */
		" Channel access level required to edit triggers: %d",
		/* LANG_DT_INFO_PRIVMSG_OFF */
		" Queries will be answered through a channel NOTICE.",
		/* LANG_DT_INFO_PRIVMSG_ON */
		" Queries will be answered through a channel PRIVMSG.",
		/* LANG_GDT_INFO_DEF_TCHAR */
		" Default trigger character set to: %s",
		/* LANG_DT_LINK_SYNTAX */
		" Syntax: %BSFdt LINK trigger target",
		/* LANG_DT_LINK_SYNTAX_EXT */
		" Syntax: %BSFdt LINK trigger target\n"
		" \n"
		" DT LINK links a trigger to another target trigger creating an alias.",
		/* LANG_GDT_LINK_SYNTAX */
		" Syntax: %BSFgdt LINK trigger target",
		/* LANG_GDT_LINK_SYNTAX_EXT */
		" Syntax: %BSFgdt LINK trigger target\n"
		" \n"
		" GDT LINK links a trigger to another target trigger creating an alias.",
		/* LANG_DT_EXISTS */
		" Trigger already exists.",
		/* LANG_DT_LINKED */
		" Trigger %s has been linked to %s.",
		/* LANG_GDT_LINKED */
		" Global trigger %s has been linked to %s.",
		/* LANG_DT_CANNOT_EDIT_LINKED */
		" Linked triggers cannot be edited. \n"
		" Either edit the main trigger, or delete the link and create a new trigger.",
		/* LANG_DT_SET_TR_LVL_SYNTAX */
		" Syntax: %BSFdt SET trigger LEVEL [level]",
		/* LANG_GDT_SET_TR_LVL_SYNTAX */
		" Syntax: %BSFgdt SET trigger LEVEL [level]",
		/* LANG_DT_SET_TR_LEVEL */
		" Level for trigger %s is now set to: %d.",
		/* LANG_DT_INV_POS */
		" Invalid position: %s.",
		/* LANG_DT_MAX_REPLIES */
		" Maximum number of replies for this trigger has been reached (%d).",
		/* LANG_DT_DUP_SYNTAX */
		" Syntax: %BSFdt DUP trigger target",
		/* LANG_DT_DUP_SYNTAX_EXT */
		" Syntax: %BSFdt DUP trigger target\n"
		" \n"
		" DT DUP duplicates the contents of trigger into a newly created target trigger.",
		/* LANG_GDT_DUP_SYNTAX */
		" Syntax: %BSFgdt DUP trigger target",
		/* LANG_GDT_DUP_SYNTAX_EXT */
		" Syntax: %BSFgdt DUP trigger target\n"
		" \n"
		" GDT DUP duplicates the contents of trigger into a newly created target trigger.",
		/* LANG_DT_DUP */
		" Trigger %s has been copied to %s.",
		/* LANG_GDT_DUP */
		" Global trigger %s has been copied to %s.",
		/* LANG_DT_INFO_TCHAR_DEF */
		" Channel is using the default trigger character: %s",
		/* LANG_DT_SHOW_ENTRY_COND */
		" Condition for showing entry: %s",
		/* LANG_DT_SHOW_ENTRY_LIST_COND */
		" Condition for showing entry %d: %s",
		/* LANG_DT_SET_COND_SYNTAX */
		" Syntax: %BSFdt SET trigger COND [+nr] [condition]\n",
		/* LANG_GDT_SET_COND_SYNTAX */
		" Syntax: %BSFgdt SET trigger COND [+nr] [condition]\n",
		/* LANG_DT_SET_COND_SET */
		" Condition on trigger reply %d has been set.",
		/* LANG_DT_SET_COND_CLEARED */
		" Condition has been cleared for trigger reply %d.",
		/* LANG_DT_SET_COND_ENTRY_NR */
		" This trigger has multiple entries associated, an entry number must therefore be specified.",
		/* LANG_DT_COND_SYNTAX_ERR */
		" An error occured while attempting to parse the condition. Check the condition syntax.",
		/* LANG_DT_REDIRECT_DENIED */
		" You do not have permissions to redirect output on this channel.",
		/* LANG_DT_CONDITIONS */
		" Conditions can be defined to restrict the sending of replies or modify trigger replies dynamically.\n"
		" Conditions can be defined INLINE - inside a trigger reply. These are typically used for example to append\n"
		" an extra bit of text to the reply if an argument was passed along.\n"
		" Additionally, conditions can also be defined at REPLY level. These are typically used when an entirely\n"
		" different reply has to be send based on who performs the command or what arguments were passed along.\n"
		" \n"
		" For more info on INLINE conditions, see %BSFhelp DT CONDITIONS INLINE.\n"
		" For more info on REPLY conditions, see %BSFhelp DT CONDITIONS REPLY.\n"
		" For more info on what variables can be used in replies and conditions, see %BSFhelp DT VARIABLES.",
		/* LANG_DT_CONDITIONS_INLINE */
		" Triggers support the use of inline conditionals for checking existance of and\n"
		" comparing with user supplied arguments. The following formats are supported:\n"
		"   ** If argument exists          -  $1?\n"
		"   ** If argument does not exist  -  $1!\n"
		"   ** End an If statement         -  $1$\n"
		"   ** If arg 1 is 'test'          -  $1==test?\n"
		"   ** else (if arg1 not 'test')   -  $1==test!\n"
		"   ** End an If statement         -  $1==test$\n"
		"   ** If arg 1 not 'test' (Alt.)  -  $1!=test?\n"
		" \n"
		" Supported comparison operations are: ==, !=, >, >=, < and <=.\n"
		" Note that end or else statements are optional, however if missing, everything\n"
		" following the statement will be considered part of the condition.\n"
		" Also note that the parsing engine will interpret $1==test! as true when no\n"
		" argument is supplied, yet $1!=test? will be false when no argument is given.\n"
		" Likewise $1==test? will be false when no argument is given, yet $1!=test! \n"
		" will be true when no argument is given.\n"
		" \n"
		" A few examples:\n"
		"  - Check if trigger is called with 3 arguments:\n"
		"  *** Trigger $3? called with 3 args $3! not called with 3 args $3$.\n"
		"  - Check if trigger is called with 2 arguments and if first is greater than 5:\n"
		"  *** Trigger $2? called with 2 args and arg1 $1>5? greater $1>5! smaller $1>5$than 5.$2! not called with 2 args.\n"
		" \n"
		" Note that the use of the '$' sign as the character for variables and conditions means that every occurence that\n"
		" isn't a variable or condition must be escaped with '\\'!"
		" \n"
		" For more info on what variables can be used in conditions, see %BSFhelp DT VARIABLES.",
		/* LANG_DT_CONDITIONS_REPLY */
		" It is possible to define a condition on a reply level. Only if that condition\n"
		" translates as 'true' will the reply line be considered for output to the user.\n"
		" The following operators are accepted:\n"
		"    ()   -    Parse content between ( and ) as a whole.\n"
		"    !    -    Invert result of following statement or subsequent ().\n"
		"    ||   -    OR\n"
		"    &&   -    AND\n"
		" \n"
		" The statements making up the conditions support the same comparison operations\n"
		" as inline conditions (==, !=, >, >=, < and <=). However, the == and != operation differ slightly in\n"
		" that they expect subsequent strings to be enclosed by \"s - does not apply to numbers.\n"
		" \n"
		" A few examples:\n"
		"  - Check whether there are 3 arguments and arg1 must be greater than 0 or arg2 must be 'test'\n"
		"  *** $3 && ( $1 > 0 || $2 == \"test\")\n"
		"  - Several ways to check if arguments given are equal to 'anope rulezzz' or none are given and trigger is called by 'Viper'\n"
		"  *** $2 && !$3 && $1- == \"anope rulezzz\" || !$1 && $n == \"viper\"\n"
		"  *** !$3 && $1- == \"anope rulezzz\" || !$1 && $n == \"viper\"\n"
		"  *** (!$3 && $1- == \"anope rulezzz\") || (!$1 && $n == \"viper\")\n"
		" \n"
		" For more info on what variables can be used in conditions, see %BSFhelp DT VARIABLES.",
		/* LANG_DT_VARIABLES */
 		" Trigger replies support a number of build-in variables (indicated by '$') that may be used in (both reply and inline) conditionals or\n"
 		" simply to add dynamic data to the reply itself. These are substitued by their appropriate values when the reply is send to the user.\n"
 		" The following variables are supported:\n"
 		"  Variable  -  Value\n"
 		"  *  $b     -  BotServ bot nickname. \n"
 		"  *  $c     -  Channel name.\n"
 		"  *  $n     -  Nick of the user who called the trigger.\n"
 		"  *  $h     -  Visible user@host of the user who called the trigger.\n"
 		"  *  $d     -  Destination of the message. (Normally channel name, but in case of a redirect it can be a nick.)\n"
  		"  *  $t     -  Topic of the channel the trigger was called in.\n"
 		"  *  $x     -  The name of the trigger that was called..\n"
 		"  *  $me    -  At the beginning of the reply: reply is send as a PRIVMSG ACTION. Inside a trigger, it is substituted the botnick (cf. 'b').\n"
		" \n"
		" Note that the use of the '$' sign as the character for variables means that every occurence that isn't a variable must be escaped with '\\'!",
		/* LANG_REDIRECT_MIN_SEND_INTERVAL */
		" You can only redirect trigger output once every %d seconds. You must wait for another %d seconds.",
		/* LANG_REDIRECT_MIN_RECV_INTERVAL */
		" There must be a minimum of %d seconds between 2 subsequent redirects received by a single user.\n"
		" User %s cannot receive another redirect for another %d seconds.",
		/* LANG_REDIRECT_COMPLETE */
		" Redirect completed."
	};

	char *langtable_nl[] = {
		/* LANG_TRIGGER_HELP */
		" Gebruik \037%strigger [parameters] [> doorsturen naar nicks]\037 om items uit de database op te vragen.",
		/* LANG_DT_DESC */
		" Beheer kanaal specifieke items en instellingen.",
		/* LANG_GDT_DESC */
		" Beheer globale items.",
		/* LANG_DT_SYNTAX */
		" Syntax: %BSFdt ADD [+niveau] item [+nr] antwoord\n"
		" Syntax: %BSFdt { APP | DEL } item [+nr] [antwoord]\n"
		" Syntax: %BSFdt { LINK | DUP } item doel\n"
		" Syntax: %BSFdt SET item { LIST | OPERONLY | LEVEL | COND } { { ON | OFF } | toegangsniveau | [+nr] } [ voorwaarde ]\n"
		" Syntax: %BSFdt SET { DTCHAR | PRIVMSG | LEVEL } { opzoek letter | { ON | OFF } | { ACCESS | EDIT } } [toegangsniveau]\n"
		" Syntax: %BSFdt { SHOW | LIST | CLEAR | INFO } [item]\n",
		/* LANG_GDT_SYNTAX */
		" Syntax: %BSFgdt ADD [+niveau] item [+nr] reply\n"
		" Syntax: %BSFgdt { APP | DEL } item [+nr] [reply]\n"
		" Syntax: %BSFgdt { LINK | DUP } item doel\n"
		" Syntax: %BSFgdt SET item { LIST | OPERONLY | LEVEL | COND } { { ON | OFF } | toegangsniveau | [+nr] } [ voorwaarde ]\n"
		" Syntax: %BSFgdt { SHOW | LIST | CLEAR | INFO } [item]\n",
		/* LANG_DT_SYNTAX_EXT */
		" Syntax: %BSFdt ADD [+niveau] item [+nr] antwoord\n"
		" Syntax: %BSFdt APP item [+nr] antwoord\n"
		" Syntax: %BSFdt LINK item doel\n"
		" Syntax: %BSFdt DUP item doel\n"
		" Syntax: %BSFdt DEL item [nr]\n"
		" Syntax: %BSFdt SET item { LIST | OPERONLY | LEVEL | COND } { { ON | OFF } | toegangsniveau | +nr } [ voorwaarde ]\n"
		" Syntax: %BSFdt SET { DTCHAR | PRIVMSG | LEVEL } { opzoek letter | { ON | OFF } | { ACCESS | EDIT } } [toegangsniveau]\n"
		" Syntax: %BSFdt SHOW item\n"
		" Syntax: %BSFdt { LIST | CLEAR | INFO }\n"
		" \n"
		" Dit commando beheert de lijst met kanaal specifieke items.\n"
		" Instellingen geldig voor dit kanaal kunnen worden gewijzigd en items & antwoorden toegevoegd, bijgewerkt of verwijderd"
		" met behulp van dit commando. Wanneer een item in een kanaal wordt vermeld voorafgegaan door de 'opzoek letter'\n"
		" zal dit beantwoord worden met het overeenkomstige antwoord uit de database, of indien meerdere antwoorden beschikbaar zijn\n"
		" en de optie willekeurige selectie staat aan, een willekeurig geselecteerd antwoord.\n"
		" \n"
		" DT ADD voegt een nieuw item en antwoord toe voor dit kanaal, of indien het item reeds bestaad, voegt een extra antwoord toe.\n"
		" Optioneel kan een minimum toegangsniveau opgegeven worden. Gebruikers moeten dan minstens over dit niveau beschikken\n"
		" om dit item op te kunnen vragen. Dit niveau dient te worden opgegeven als +waarde of +SOP/AOP/HOP/VOP.\n"
		" Als geen niveau is opgegeven worden geen extra beperkingen opgelegd om toegang te krijgen tot dit item.\n"
		" Let wel dat de DT SET LEVEL ACCESS instelling steeds geldt.\n"
		" Een positie can gespecifieerd worden dmv. +nummer om een antwoord op een bepaalde plaats in de lijst in te voegen.\n"
		" \n"
		" DT APP voegt tekst aan een reeds bestaand antwoord toe. Als een item meerdere antwoorden heeft moet een nummer\n"
		" specifieren waaraan de tekst toegevoegd moet worden.\n"
		" \n"
		" DT LINK linkt een item aan een reeds bestaand item als een alias.\n"
		" \n"
		" DT DUP kopieert de inhoud van een bestaande trigger naar een nieuwe onbestaande trigger.\n"
		" \n"
		" DT DEL verwijdert een item uit de database. Als een antwoord nummer is gegeven zal enkeldat antwoord verwijderd worden\n"
		" waarbij de overige antwoorden die bij het item horen in de database blijven. Als geen nummer is opgegeven zal het volledig item\n"
		" met alle geassocieerde antwoorden en instellingen verwijderd worden.\n"
		" \n"
		" DT SET item configureert item specifieke instellingen. De volgende opties zijn beschikbaar: \n"
		" LIST, OPERONLY, LEVEL en COND. De eerste 2 kunnen ofwel aan (ON) of uit (OFF) geschakeld worden,\n"
		" terwijl voor LEVEL een niveau dient te worden opgegeven als numerieke waarde of als SOP/AOP/HOP/VOP.\n"
		" LIST bepaald of alle item antwoorden gezien moeten worden als één groot antwoord (ON), of als een collectie\n"
		" antwoorden waaruit één mogelijk antwoord willekeurig geselecteerd dient te worden (OFF).\n"
		" OPERONLY bepaald of toegang tot een item beperkt is tot enkel IRC operators. LEVEL maakt het mogelijk een items \n"
		" toegangsniveau te wijzigen. Dit is identiek aan deze die kan opgegeven worden tijdens de creatie.\n"
		" COND laat toe een voorwaarde in te stellen op een antwoord. Deze voorwaarden moeten voldaan zijn om het antwoord te kunnen sturen.\n"
		" \n"
		" DT SET configureert eveneens een aantal niet-item-specifieke instellingen. Volgende opties zijn beschikbaar:\n"
		" DTCHAR, PRIVMSG en LEVEL. De DTCHAR optie stelt het karakter in dat een item zoek opdracht aankodigt op dit kanaal\n"
		" PRIVMSG bepaald of de bot een zoekopdracht zal beantwoorden in een NOTICE naar het kanaal (OFF) of in een gewoon bericht (ON).\n"
		" De LEVEL optie stelt een minimum toegangsniveau in voor het opvragen en bijwerken van items voor dit kanaal.\n"
		" De optie LEVEL heeft hiertoe ofwel de ACCESS (toegang) ofwel de EDIT (bijwerken) parameter aan. Het niveau kan worden gegeven\n"
		" als een numerieke waarde of als SOP/AOP/HOP/VOP dewelke dan automatisch worden omgezet in numerieke waardes.\n"
		" \n"
		" DT SHOW geeft alle gegevens in verband met een item weer.\n"
		" \n"
		" DT LIST geeft een lijst weer met alle lokale items voor dit kanaal.\n"
		" \n"
		" DT CLEAR verwijderd alle items van dit kanaal uit de database.\n"
		" \n"
		" DT INFO toont algemene statistische informatie over de item database van dit kanaal."
		" \n"
		" Voorwaarden en variabelen kunnen gebruikt worden in en gedefinieerd voor antwoorden. Zie %BSFhelp DT CONDITIONS voor meer info.",
		/* LANG_GDT_SYNTAX_EXT */
		" Syntax: %BSFgdt ADD [+niveau] item [+nr] antwoord\n"
		" Syntax: %BSFgdt APP item [+nr] antwoord\n"
		" Syntax: %BSFgdt LINK item doel\n"
		" Syntax: %BSFgdt DUP item doel\n"
		" Syntax: %BSFgdt DEL item [nr]\n"
		" Syntax: %BSFgdt SET item { LIST | OPERONLY | LEVEL | COND } { { ON | OFF } | toegangsniveau | +nr } [ voorwaarde ]\n"
		" Syntax: %BSFgdt SET { DTCHAR | PRIVMSG | LEVEL } { opzoek letter | { ON | OFF } | { ACCESS | EDIT } } [toegangsniveau]\n"
		" Syntax: %BSFgdt SHOW item\n"
		" Syntax: %BSFgdt { LIST | CLEAR | INFO }\n"
		" \n"
		" Dit commando beheert de lijst met globale items. Deze items zullen beschikbaar zijn in alle kanalen op\n"
		" het netwerk waar items kunnen worden opgezocht.\n"
		" Items en antwoorden kunnen worden toegevoegd, bijgewerkt of verwijderd met behulp van dit commando. "
		" Wanneer een item in een kanaal wordt vermeld voorafgegaan door de 'opzoek letter'\n"
		" zal dit beantwoord worden met het overeenkomstige antwoord uit de database, of indien meerdere antwoorden beschikbaar zijn\n"
		" en de optie willekeurige selectie staat aan, een willekeurig geselecteerd antwoord.\n"
		" \n"
		" GDT ADD voegt een nieuw item en antwoord toe, of indien het item reeds bestaad, voegt een extra antwoord toe.\n"
		" Optioneel kan een minimum toegangsniveau opgegeven worden. Gebruikers moeten dan minstens over dit niveau beschikken\n"
		" om dit item op te kunnen vragen. Dit niveau dient te worden opgegeven als +waarde of +SOP/AOP/HOP/VOP.\n"
		" Als geen niveau is opgegeven worden geen extra beperkingen opgelegd om toegang te krijgen tot dit item.\n"
		" Let wel dat de DT SET LEVEL ACCESS instelling steeds geldt.\n"
		" Een positie can gespecifieerd worden dmv. +nummer om een antwoord op een bepaalde plaats in de lijst in te voegen.\n"
		" \n"
		" GDT APP voegt tekst aan een reeds bestaand antwoord toe. Als een item meerdere antwoorden heeft moet een nummer\n"
		" specifieren waaraan de tekst toegevoegd moet worden.\n"
		" \n"
		" GDT LINK linkt een item aan een reeds bestaand item als een alias.\n"
		" \n"
		" GDT DUP kopieert de inhoud van een bestaande trigger naar een nieuwe onbestaande trigger.\n"
		" \n"
		" GDT DEL verwijdert een item uit de database. Als een antwoord nummer is gegeven zal enkeldat antwoord verwijderd worden\n"
		" waarbij de overige antwoorden die bij het item horen in de database blijven. Als geen nummer is opgegeven zal het volledig item\n"
		" met alle geassocieerde antwoorden en instellingen verwijderd worden.\n"
		" \n"
		" GDT SET item configureert item specifieke instellingen. De volgende opties zijn beschikbaar: \n"
		" LIST, OPERONLY, LEVEL en COND. De eerste 2 kunnen ofwel aan (ON) of uit (OFF) geschakeld worden,\n"
		" terwijl voor LEVEL een niveau dient te worden opgegeven als numerieke waarde of als SOP/AOP/HOP/VOP.\n"
		" LIST bepaald of alle item antwoorden gezien moeten worden als één groot antwoord (ON), of als een collectie\n"
		" antwoorden waaruit één mogelijk antwoord willekeurig geselecteerd dient te worden (OFF).\n"
		" OPERONLY bepaald of toegang tot een item beperkt is tot enkel IRC operators. LEVEL maakt het mogelijk een items \n"
		" toegangsniveau te wijzigen. Dit is identiek aan deze die kan opgegeven worden tijdens de creatie.\n"
		" COND laat toe een voorwaarde in te stellen op een antwoord. Deze voorwaarden moeten voldaan zijn om het antwoord te kunnen sturen.\n"
		" \n"
		" GDT SHOW geeft alle gegevens in verband met een item weer.\n"
		" \n"
		" GDT LIST geeft een lijst weer met alle globale items.\n"
		" \n"
		" GDT CLEAR verwijderd alle globale items uit de database.\n"
		" \n"
		" GDT INFO toont algemene statieken over de globale item database.\n"
		" \n"
		" Voorwaarden en variabelen kunnen gebruikt worden in en gedefinieerd voor antwoorden. Zie %BSFhelp GDT CONDITIONS voor meer info.",
		" \n"
		" Toegang tot sommige commandos is beperkt tot IRC Operators.",
		/* LANG_DT_ADD_SYNTAX */
		" Syntax: %BSFdt ADD [+niveau] item [+nr] antwoord",
		/* LANG_DT_ADD_SYNTAX_EXT */
		" Syntax: %BSFdt ADD [+niveau] item [+nr] antwoord\n"
		" \n"
		" DT ADD voegt een nieuw item en antwoord toe voor dit kanaal, of indien het item reeds bestaad, voegt een extra antwoord toe.\n"
		" Optioneel kan een minimum toegangsniveau opgegeven worden. Gebruikers moeten dan minstens over dit niveau beschikken\n"
		" om dit item op te kunnen vragen. Dit niveau dient te worden opgegeven als +waarde of +SOP/AOP/HOP/VOP.\n"
		" Als geen niveau is opgegeven worden geen extra beperkingen opgelegd om toegang te krijgen tot dit item.\n"
		" Een positie can gespecifieerd worden dmv. +nummer om een antwoord op een bepaalde plaats in de lijst in te voegen.\n"
		" \n"
		" Voorwaarden en variabelen kunnen gebruikt worden in antwoorden. Zie %BSFhelp DT CONDITIONS voor meer info.\n"
		" \n"
		" Let op dat de DT SET LEVEL ACCESS instelling steeds geldt.\n",
		/* LANG_DT_APP_SYNTAX */
		" Syntax: %BSFdt APP item [+nr] antwoord",
		/* LANG_DT_APP_SYNTAX_EXT */
		" Syntax: %BSFdt APP item [+nr] antwoord\n"
		" \n"
		" DT APP voegt tekst aan een reeds bestaand antwoord toe. Als een item meerdere antwoorden heeft moet een nummer\n"
		" specifieren waaraan de tekst toegevoegd moet worden.",
		/* LANG_DT_DEL_SYNTAX */
		" Syntax: %BSFdt DEL item [nr]",
		/* LANG_DT_DEL_SYNTAX_EXT */
		" Syntax: %BSFdt DEL item [nr]\n"
		" \n"
		" DT DEL verwijdert een item uit de database. Als een antwoord nummer is gegeven zal enkeldat antwoord verwijderd worden\n"
		" waarbij de overige antwoorden die bij het item horen in de database blijven. Als geen nummer is opgegeven zal het volledig item\n"
		" met alle geassocieerde antwoorden en instellingen verwijderd worden.",
		/* LANG_DT_SET_SYNTAX */
		" Syntax: %BSFdt SET item { LIST | OPERONLY | LEVEL } { { ON | OFF } | toegangsniveau }\n"
		" Syntax: %BSFdt SET { DTCHAR | PRIVMSG | LEVEL } { opzoek letter | { ON | OFF } | { ACCESS | EDIT } } [toegangsniveau]",
		/* LANG_DT_SET_SYNTAX_EXT */
		" Syntax: %BSFdt SET item LIST { ON | OFF }\n"
		" Syntax: %BSFdt SET item OPERONLY { ON | OFF }\n"
		" Syntax: %BSFdt SET item LEVEL [toegangsniveau]\n"
		" Syntax: %BSFdt SET item COND [+nr] [voorwaarde]\n"
		" \n"
		" Syntax: %BSFdt SET DTCHAR opzoek letter\n"
		" Syntax: %BSFdt SET PRIVMSG { ON | OFF }\n"
		" Syntax: %BSFdt SET LEVEL { ACCESS | EDIT } [toegangsniveau]\n"
		" \n"
		" DT SET item configureert item specifieke instellingen. De volgende opties zijn beschikbaar: \n"
		" LIST, OPERONLY en LEVEL. De eerste 2 kunnen ofwel aan (ON) of uit (OFF) geschakeld worden,\n"
		" terwijl voor LEVEL een niveau dient te worden opgegeven als numerieke waarde of als SOP/AOP/HOP/VOP.\n"
		" LIST bepaald of alle item antwoorden gezien moeten worden als één groot antwoord (ON), of als een collectie antwoorden\n"
		" waaruit één mogelijk antwoord willekeurig geselecteerd dient te worden (OFF).\n"
		" OPERONLY bepaald of toegang tot een item beperkt is tot enkel IRC operators.\n"
		" LEVEL maakt het mogelijk een items toegangsniveau te wijzigen. Dit is identiek aan deze die kan opgegeven worden tijdens de creatie.\n"
		" COND laat toe een voorwaarde in te stellen op een antwoord. Deze voorwaarden moeten voldaan zijn om het antwoord te kunnen sturen.\n"
		" Zie %BSFhelp DT CONDITIONS voor meer info.\n"
		" \n"
		" DT SET configureert eveneens een aantal niet-item-specifieke instellingen.\n"
		" Volgende opties zijn beschikbaar: LIST, OPERONLY, LEVEL en COND.\n"
		" De DTCHAR optie stelt het karakter in dat een item zoek opdracht aankodigt op dit kanaal PRIVMSG bepaald of de bot\n"
		" een zoekopdracht zal beantwoorden in een NOTICE naar het kanaal (OFF) of in een gewoon bericht (ON).\n"
		" De LEVEL optie stelt een minimum toegangsniveau in voor het opvragen en bijwerken van items voor dit kanaal.\n"
		" De optie LEVEL heeft hiertoe ofwel de ACCESS (toegang) ofwel de EDIT (bijwerken) parameter aan. Het niveau kan worden gegeven\n"
		" als een numerieke waarde of als SOP/AOP/HOP/VOP dewelke dan automatisch worden omgezet in numerieke waardes.",
		/* LANG_DT_SET_TR_OPERONLY_SYNTAX */
		" Syntax: %BSFdt SET item OPERONLY { ON | OFF }",
		/* LANG_DT_SET_TR_LIST_SYNTAX */
		" Syntax: %BSFdt SET item LIST { ON | OFF }",
		/* LANG_DT_SET_DTCHAR_SYNTAX */
		" Syntax: %BSFdt SET DTCHAR opzoek letter",
		/* LANG_DT_SET_PRIVMSG_SYNTAX */
		" Syntax: %BSFdt SET PRIVMSG { ON | OFF }",
		/* LANG_DT_SET_LEVEL_SYNTAX */
		" Syntax: %BSFdt SET LEVEL { ACCESS | EDIT } [toegangsniveau]",
		/* LANG_DT_LIST_SYNTAX */
		" Syntax: %BSFdt LIST",
		/* LANG_DT_LIST_SYNTAX_EXT */
		" Syntax: %BSFdt LIST\n"
		" \n"
		" DT LIST geeft een lijst weer met alle lokale items voor dit kanaal.",
		/* LANG_DT_SHOW_SYNTAX */
		" Syntax: %BSFdt SHOW item",
		/* LANG_DT_SHOW_SYNTAX_EXT */
		" Syntax: %BSFdt SHOW item\n"
		" \n"
		" DT SHOW geeft alle gegevens in verband met een item weer.",
		/* LANG_DT_CLEAR_SYNTAX */
		" Syntax: %BSFdt CLEAR",
		/* LANG_DT_CLEAR_SYNTAX_EXT */
		" Syntax: %BSFdt CLEAR\n"
		" \n"
		" DT CLEAR verwijderd alle items van dit kanaal uit de database.",
		/* LANG_DT_INFO_SYNTAX */
		" Syntax: %BSFdt INFO",
		/* LANG_DT_INFO_SYNTAX_EXT */
		" Syntax: %BSFdt INFO\n"
		" \n"
		" DT INFO toont algemene statistische informatie over de item database van dit kanaal.",
		/* LANG_GDT_ADD_SYNTAX */
		" Syntax: %BSFgdt ADD [+niveau] item [+nr] antwoord",
		/* LANG_GDT_ADD_SYNTAX_EXT */
		" Syntax: %BSFgdt ADD [+niveau] item [+nr] antwoord\n"
		" \n"
		" GDT ADD voegt een nieuw item en antwoord toe, of indien het item reeds bestaad, voegt een extra antwoord toe.\n"
		" Optioneel kan een minimum toegangsniveau opgegeven worden. Gebruikers moeten dan minstens over dit niveau beschikken\n"
		" om dit item op te kunnen vragen. Dit niveau dient te worden opgegeven als +waarde of +SOP/AOP/HOP/VOP.\n"
		" Als geen niveau is opgegeven worden geen extra beperkingen opgelegd om toegang te krijgen tot dit item.\n"
		" Een positie can gespecifieerd worden dmv. +nummer om een antwoord op een bepaalde plaats in de lijst in te voegen.\n"
		" \n"
		" Voorwaarden en variabelen kunnen gebruikt worden in antwoorden. Zie %BSFhelp GDT CONDITIONS voor meer info.\n"
		" \n"
		" Let wel dat de DT SET LEVEL ACCESS instelling steeds geldt.",
		/* LANG_GDT_APP_SYNTAX */
		" Syntax: %BSFgdt APP item [+nr] antwoord",
		/* LANG_GDT_APP_SYNTAX_EXT */
		" Syntax: %sgdt APP item [+nr] antwoord\n"
		" \n"
		" GDT APP voegt tekst aan een reeds bestaand antwoord toe. Als een item meerdere antwoorden heeft moet een nummer\n"
		" specifieren waaraan de tekst toegevoegd moet worden.",
		/* LANG_GDT_DEL_SYNTAX */
		" Syntax: %BSFgdt DEL item [nr]",
		/* LANG_GDT_DEL_SYNTAX_EXT */
		" Syntax: %BSFgdt DEL item [nr]\n"
		" \n"
		" GDT DEL verwijdert een item uit de database. Als een antwoord nummer is gegeven zal enkeldat antwoord verwijderd worden\n"
		" waarbij de overige antwoorden die bij het item horen in de database blijven. Als geen nummer is opgegeven zal het volledig item\n"
		" met alle geassocieerde antwoorden en instellingen verwijderd worden.",
		/* LANG_GDT_SET_SYNTAX */
		" Syntax: %BSFgdt SET item { LIST | OPERONLY | LEVEL } { { ON | OFF } | toegangsniveau }\n",
		/* LANG_GDT_SET_SYNTAX_EXT */
		" Syntax: %BSFgdt SET item LIST { ON | OFF }\n"
		" Syntax: %BSFgdt SET item OPERONLY { ON | OFF }\n"
		" Syntax: %BSFgdt SET item LEVEL [toegangsniveau]\n"
		" Syntax: %BSFgdt SET item COND [+nr] [voorwaarde]\n"
		" \n"
		" GDT SET item configureert item specifieke instellingen. De volgende opties zijn beschikbaar:\n"
		" LIST, OPERONLY, LEVEL en COND. De eerste 2 kunnen ofwel aan (ON) of uit (OFF) geschakeld worden,\n"
		" terwijl voor LEVEL een niveau dient te worden opgegeven als numerieke waarde of als SOP/AOP/HOP/VOP.\n"
		" LIST bepaald of alle item antwoorden gezien moeten worden als één groot antwoord (ON),\n"
		" of als een collectie antwoorden waaruit één mogelijk antwoord willekeurig geselecteerd dient te worden (OFF).\n"
		" OPERONLY bepaald of toegang tot een item beperkt is tot enkel IRC operators.\n"
		" LEVEL maakt het mogelijk een items toegangsniveau te wijzigen. Dit is identiek aan deze die kan opgegeven worden tijdens de creatie.\n"
		" COND laat toe een voorwaarde in te stellen op een antwoord. Deze voorwaarden moeten voldaan zijn om het antwoord te kunnen sturen.\n"
		" Zie %BSFhelp GDT CONDITIONS voor meer info.",
		/* LANG_GDT_SET_TR_OPERONLY_SYNTAX */
		" Syntax: %BSFgdt SET item OPERONLY { ON | OFF }",
		/* LANG_GDT_SET_TR_LIST_SYNTAX */
		" Syntax: %BSFgdt SET item LIST { ON | OFF }",
		/* LANG_GDT_LIST_SYNTAX */
		" Syntax: %BSFgdt LIST",
		/* LANG_GDT_LIST_SYNTAX_EXT */
		" Syntax: %BSFgdt LIST\n"
		" \n"
		" GDT LIST geeft een lijst weer met alle globale items.",
		/* LANG_GDT_SHOW_SYNTAX */
		" Syntax: %BSFgdt SHOW item",
		/* LANG_GDT_SHOW_SYNTAX_EXT */
		" Syntax: %BSFgdt SHOW item\n"
		" \n"
		" GDT SHOW geeft alle gegevens in verband met een item weer.",
		/* LANG_GDT_CLEAR_SYNTAX */
		" Syntax: %BSFgdt CLEAR",
		/* LANG_GDT_CLEAR_SYNTAX_EXT */
		" Syntax: %BSFgdt CLEAR\n"
		" \n"
		" GDT CLEAR verwijderd alle globale items uit de database.",
		/* LANG_GDT_INFO_SYNTAX */
		" Syntax: %BSFgdt INFO",
		/* LANG_GDT_INFO_SYNTAX_EXT */
		" Syntax: %BSFgdt INFO\n"
		" \n"
		" GDT INFO toont algemene statieken over de globale item database.",
		/* LANG_ERROR */
		" Er is een onvoorziene error opgetreden. Gelieve dit incident te rapporteren.",
		/* LANG_CMD_DISABLED */
		" Dit commando is momenteel uitgeschakeld.",
		/* LANG_CMD_NOT_AVAILABLE */
		" Dit commando is momenteel niet beschikbaar.",
		/* LANG_CANNOT_CREATE_TR */
		" Kon item niet aanmaken! Gelieve dit probleem te rapporteren.",
		/* LANG_LEVEL_INVALID */
		" Ongeldig niveau..",
		/* LANG_DT_ADDED */
		" Data toegevoegd voor item %s.",
		/* LANG_GDT_ADDED */
		" Data toegevoegd voor item %s.",
		/* LANG_DT_NO_ENTRIES */
		" De item lijst voor dit kanaal is leeg.",
		/* LANG_DT_NO_SUCH_ENTRY */
		" Item niet gevonden.",
		/* LANG_DT_APP_SPEC_ENTRY_NR */
		" Dit item heeft meerdere antwoorden; een antwoord nummer moet worden gespecifieerd.",
		/* LANG_DT_APPENDED */
		" Data toegevoegd aan %s.",
		/* LANG_DT_INVALID_NR */
		" Item of opgegeven antwoord bestaat niet.",
		/* LANG_DT_DELETED */
		" Item %s is verwijdered.",
		/* LANG_DT_DELETED_NR */
		" Antwoord %d van item %s is verwijdered uit de lijst.",
		/* LANG_DT_LIST_HEADER */
		" Items voor %s:",
		/* LANG_GDT_LIST_HEADER */
		" Beschikbare globale items:",
		/* LANG_DT_LIST */
		" %s",
		/* LANG_GDT_LIST */
		" %s",
		/* LANG_DT_LIST_EMPTY */
		" De item lijst is leeg.",
		/* LANG_GDT_LIST_EMPTY */
		" Er zijn geen globale items beschikbaar.",
		/* LANG_DT_SHOW_TRIGGER */
		" Overzicht van trigger %s:",
		/* LANG_GDT_SHOW_TRIGGER */
		" Overzicht van trigger %s:",
		/* LANG_DT_SHOW_CREATED */
		" Aangemaakt door %s op %s.",
		/* LANG_DT_SHOW_UPDATED */
		" Laatst bijgewerkt door %s op %s.",
		/* LANG_DT_SHOW_LEVEL */
		" Benodigd toegangsniveau: %d",
		/* LANG_DT_INFO_OPT_LIST */
		"Lijst (Toon alle antwoorden)",
		/* LANG_DT_INFO_OPT_RAND */
		"Willekeurig (Toon willekeurig antwoord)",
		/* LANG_DT_OPT_OPERONLY */
		"Operators Only",
		/* LANG_DT_SHOW_FLAGS */
		" Opties: %s",
		/* LANG_DT_SHOW_ENTRIES */
		" Item antwoorden:",
		/* LANG_DT_CLEARED */
		" Alle items van kanaal %s zijn verwijderd.",
		/* LANG_GDT_CLEARED */
		" Alle globale items zijn verwijderd.",
		/* LANG_GDT_SET_UNKWN_TRIGGER */
		" Item niet gevonden.",
		/* LANG_GDT_SET_UNKWN_OPTION */
		" Ongeldige optie.",
		/* LANG_DT_SET_UNKWN_OPTION_TRIGGER */
		" Onbekend item of ongeldige optie.",
		/* LANG_DT_SET_TR_OPERONLY_ON */
		" Item %s is nu beperkt tot IRC Operators.",
		/* LANG_DT_SET_TR_OPERONLY_OFF */
		" Item %s is niet langer beperkt tot IRC Operators.",
		/* LANG_DT_SET_TR_LIST */
		" Alle antwoorden voor item %s zullen nu worden weergegeven.",
		/* LANG_DT_SET_TR_RAND */
		" Een willekeurig antwoord zal geselecteerd worden voor item %s.",
		/* LANG_DT_SET_PRIVMSG_ON */
		" Item opzoekingen in %s zullen nu beantwoord worden in een kanaal PRIVMSG.",
		/* LANG_DT_SET_PRIVMSG_OFF */
		" Item opzoekingen in %s zullen nu beantwoord worden in een kanaal NOTICE.",
		/* LANG_DT_SET_DTCHAR */
		" Opzoek karakter voor kanaal %s is nu %s.",
		/* LANG_DT_SET_LEVEL_A */
		" Toegangsniveau nodig om item opzoekingen te doen in %s nu is %d.",
		/* LANG_DT_SET_LEVEL_E */
		" Toegangsniveau nodig om items bij te werken in %s nu is %d.",
		/* LANG_DT_SET_ACCESS_INV_LVL */
		" Ongeldig niveau opgegeven.",
		/* LANG_DT_INFO_COUNT */
		" Er zijn %d lokale items beschikbaar in kanaal %s.",
		/* LANG_GDT_INFO_COUNT */
		" Er zijn %d globale items zijn beschikbaar.",
		/* LANG_DT_INFO_TCHAR */
		" Opzoek karakter voor %s is nu: %s",
		/* LANG_DT_INFO_LVL_ACCESS */
		" Kanaal toegangsniveau benodigd voor opzoekingen: %d",
		/* LANG_DT_INFO_LVL_EDIT */
		" Kanaal toegangsniveau nbeodigd voor bijwerkingen: %d",
		/* LANG_DT_INFO_PRIVMSG_OFF */
		" Opzoekingen zullen beantwoord worden in een kanaal NOTICE.",
		/* LANG_DT_INFO_PRIVMSG_ON */
		" Opzoekingen zullen beantwoord worden in een kanaal PRIVMSG.",
		/* LANG_GDT_INFO_DEF_TCHAR */
		" Standaard opzoek karakter is: %s",
		/* LANG_DT_LINK_SYNTAX */
		" Syntax: %BSFdt LINK item doel",
		/* LANG_DT_LINK_SYNTAX_EXT */
		" Syntax: %BSFdt LINK item doel\n"
		" \n"
		" DT LINK linkt een item aan een reeds bestaand doel item als een alias.",
		/* LANG_GDT_LINK_SYNTAX */
		" Syntax: %BSFgdt LINK item doel",
		/* LANG_GDT_LINK_SYNTAX_EXT */
		" Syntax: %BSFgdt LINK item doel\n"
		" \n"
		" GDT LINK linkt een item aan een reeds bestaand doel item als een alias.",
		/* LANG_DT_EXISTS */
		" Item bestaat reeds.",
		/* LANG_DT_LINKED */
		" Item %s refereert nu naar %s.",
		/* LANG_GDT_LINKED */
		" Globaal item %s refereert nu naar %s.",
		/* LANG_DT_CANNOT_EDIT_LINKED */
		" Gelinkte items kunnen niet bijgewerkt worden.\n"
		" Bewerk het gelinkte item, of verwijder de link en maak een nieuw item aan.",
		/* LANG_DT_SET_TR_LVL_SYNTAX */
		" Syntax: %BSFdt SET item LEVEL [toegangsniveau]",
		/* LANG_GDT_SET_TR_LVL_SYNTAX */
		" Syntax: %BSFgdt SET item LEVEL [toegangsniveau]",
		/* LANG_DT_SET_TR_LEVEL */
		" Toegangsniveau voor item %s is nu: %d.",
		/* LANG_DT_INV_POS */
		" Ongeldige positie: %s.",
		/* LANG_DT_MAX_REPLIES */
		" Maximum aantal antwoorden voor dit item bereikt (%d).",
		/* LANG_DT_DUP_SYNTAX */
		" Syntax: %BSFdt DUP item doel",
		/* LANG_DT_DUP_SYNTAX_EXT */
		" Syntax: %BSFdt DUP item doel\n"
		" \n"
		" DT DUP kopieert de inhoud van een bestaande trigger naar een nieuwe onbestaande trigger.",
		/* LANG_GDT_DUP_SYNTAX */
		" Syntax: %BSFgdt DUP item doel",
		/* LANG_GDT_DUP_SYNTAX_EXT */
		" Syntax: %BSFgdt DUP item doel\n"
		" \n"
		" GDT DUP kopieert de inhoud van een bestaande trigger naar een nieuwe onbestaande trigger.",
		/* LANG_DT_DUP */
		" Item %s is gekopieerd naar item %s.",
		/* LANG_GDT_DUP */
		" Globaal item %s is gekopieerd naar item %s.",
		/* LANG_DT_INFO_TCHAR_DEF */
		" Kanaal gebruikt het standaard opzoek karakter: %s",
		/* LANG_DT_SHOW_ENTRY_COND */
		" Voorwaarde voor het weergeven van dit item: %s",
		/* LANG_DT_SHOW_ENTRY_LIST_COND */
		" Voorwaarde voor het weergevan van item %d: %s",
		/* LANG_DT_SET_COND_SYNTAX */
		" Syntax: %BSFdt SET item COND [+nr] [voorwaarde]\n",
		/* LANG_GDT_SET_COND_SYNTAX */
		" Syntax: %BSFgdt SET item COND [+nr] [voorwaarde]\n",
		/* LANG_DT_SET_COND_SET */
		" Voorwaarde voor item antwoord %d is toegepast.",
		/* LANG_DT_SET_COND_CLEARED */
		" Voorwaarde voor item antwoord %d is verwijded.",
		/* LANG_DT_SET_COND_ENTRY_NR */
		" Dit item heeft meerdere antwoorden; een antwoord nummer moet worden gespecifieerd.",
		/* LANG_DT_COND_SYNTAX_ERR */
		" Er is een fout opgetreden bij het verwerken van de voorwaarde. Controleer de syntax van de voorwaarde.",
		/* LANG_DT_REDIRECT_DENIED */
		" Je beschikt niet over de nodige rechten om de uitvoer om te leiden op dit kanaal.",
		/* LANG_DT_CONDITIONS */
		" Voorwaarden kunnen verbonden worde aan het versturen van antwoorden of om antwoorden dynamisch aan te passen.\n"
		" Voorwaarden kunnen INLINE - binnenin een antwoord - gedefinieerd worden. Deze worden typisch gebruikt om bijvoorbeeld iets\n"
		" extra toe te voegen aan het antwoord gebaseerd op argumenten die aan de trigger werden meegegeven.\n"
		" Daarbovenop kunnen voorwaarden ook opgelegd worden op het REPLY - antwoord - niveau. Deze worden typisch gebruikt om bijvoorbeeld\n"
		" een volledig ander antwoord te versturen gebaseerd op wie de trigger opvroed of welke argumenten werden meegegeven.\n"
		" \n"
		" Voor meer info over INLINE voorwaarden, zie %BSFhelp DT CONDITIONS INLINE.\n"
		" Voor meer info over REPLY voorwaarden, zie %BSFhelp DT CONDITIONS REPLY.\n"
		" Voor meer info over welke variabelen gebruikt kunnen worden in antwoorden en voorwaarden, zie %BSFhelp DT VARIABLES.",
		/* LANG_DT_CONDITIONS_INLINE */
		" Het is mogelijk om in de antwoorden van items inline voorwaarden - voorwaarden die gedefinieerd worden in het antwoord zelf - op te nemen.\n"
		" Deze kunnen gebruikt worden om te controleren op het bestaan van en het vergelijken van meegegeven argumenten.\n"
		" De volgende formaten worden ondersteund:\n"
		"   ** Als een argument bestaat                    -  $1?\n"
		"   ** Als een argument niet bestaat               -  $1!\n"
		"   ** Einde van de voorwaarde                     -  $1$\n"
		"   ** Als argument 1 gelijk is aan 'test'         -  $1==test?\n"
		"   ** Anders (als arg1 niet gelijk is aan 'test') -  $1==test!\n"
		"   ** einde van de voorwaarde                     -  $1==test$\n"
		"   ** Als arg1 niet gelijk is aan 'test' (Alt.)   -  $1!=test?\n"
		" \n"
		" De ondersteunde vergelijkingsoperatoren zijn:  ==, !=, >, >=, < and <=.\n"
		" Let op dat 'einde' en 'anders' instructies optioneel zijn, maar dat als deze ontbreken, alles\n"
		" na de voorwaarde instructie deel uitmaakt van de voorwaarde.\n"
		" Let ook op dat de vertaler de instructie $1==test! as voldaan beschouwd wanneer er geen argument\n"
		" is opgegeven, maar dat $1!=test? als niet voldaan wordt beschouwd als er geen argument is opgegeven.\n"
		" Gelijkaardig is $1==test? will be onjuist bij het ontbreken van een argument, terwijl  $1!=test!\n"
		" als voldaan wordt beschouwd.\n"
		" \n"
		" Enkele voorbeelden:\n"
		"  - Controleer of er 3 argumenten zijn opgegeven:\n"
		"  *** Item is $3? opgeroepen met 3 argumenten $3! niet opgeroepen met 3 argumenten $3$.\n"
		"  - Controleer of er 2 argumenten zijn opgegeven en de eerste groter is dan 5:\n"
		"  *** Item is $2? opgeroepen met 2 args en arg1 is $1>5? groter $1>5! kleiner $1>5$dan 5.$2! niet opgeroepen met 2 args.\n"
		" \n"
		" Let op dat het gebruik van het '$' teken om variablen en voorwaarden aan te duiden inhoud dat om '$' gewoon weer te geven\n"
		" in het antwoord het voorafgegaan moet worden door een '\\'!"
		" \n"
		" Voor meer info over welke variabelen gebruikt kunnen worden in voorwaarden, zie %BSFhelp DT VARIABLES.",
		/* LANG_DT_CONDITIONS_REPLY */
		" Het is mogelijk om voorwaarden te verbinden aan antwoorden van een item. Enkel indien deze\n"
		" als voldaan wordne beschouwd zal het antwoord verstuurd kunnen worden naar de gebruiker.\n"
		" De volgende operatoren worden aanvaard:\n"
		"    ()   -    Verwerk alles binnen ( en ) alséén geheel.\n"
		"    !    -    Keer de waarde van de volgende instructie of (...) om.\n"
		"    ||   -    OF operator\n"
		"    &&   -    EN operator\n"
		" \n"
		" De instructies waaruit de voorwaarde bestaat ondersteund dezelfde vergelijkings operatoren als\n"
		" inline voorwaarden (==, !=, >, >=, < and <=). Echter, de == en != operaties verschillen lichtjes aangezien\n"
		" in REPLY voorwaarden de daaropvolgende string met \"s omsloten moet worden - niet van toepassing op getallen.\n"
		" \n"
		" Enkele voorbeelden:\n"
		"  - Controleer of er 3 argumenten zijn opgegeven en arg 1 moet groter zijn dan 0 of arg2 gelijk zijn aan 'test'\n"
		"  *** $3 && ( $1 > 0 || $2 == \"test\")\n"
		"  - Enkele manieren om te controleren of de meegegeven argumenten gelijk zijn aan 'anope rulezzz' of dat\n"
		"    er geen zijn gegeven maar we zijn opgeroepen door 'Viper'\n"
		"  *** $2 && !$3 && $1- == \"anope rulezzz\" || !$1 && $n == \"viper\"\n"
		"  *** !$3 && $1- == \"anope rulezzz\" || !$1 && $n == \"viper\"\n"
		"  *** (!$3 && $1- == \"anope rulezzz\") || (!$1 && $n == \"viper\")\n"
		" \n"
		" Voor meer info over welke variabelen gebruikt kunnen worden in voorwaarden, zie %BSFhelp DT VARIABLES.",
		/* LANG_DT_VARIABLES */
 		" Een aantal variabelen (aangegeven met '$') kan gebruikt worden in (zowel antwoord als inline) voorwaarden of simpelweg om dynamische data aan\n"
 		" het antwoord toe te voegen. Deze variabelen worden ingevuld op het moment het antwoord naar de gebruiker wordt verstuurd.\n"
 		" Den volgende variabelen worden ondersteund:\n"
 		"  Variabele  -  Waarde\n"
 		"  *  $b      -  BotServ bot nick. \n"
 		"  *  $c      -  Channel naam.\n"
 		"  *  $n      -  Nick van de gebruiker die het item heeft opgeroepen.\n"
 		"  *  $h      -  Zichtbare naam@host van de gebruiker die het item heeft opgeroepen.\n"
 		"  *  $d      -  Bestemmeling van het antwoord. (Normaal de kanaal naam, maar in geval van een redirect mogelijks een nick.)\n"
  		"  *  $t      -  Onderwerp van het kanaal waarin het item is opgeroepen.\n"
 		"  *  $x      -  De naam waarmee het item was opgeroepen.\n"
 		"  *  $me     -  Als aan het begin van een antwoord: antwoord wordt verstuurd als een actie. Binnenin een antwoord is dit de bot nick (zie ook. 'b').\n"
		" \n"
		" Let op dat het gebruik van het '$' teken om variablen aan te duiden inhoud dat om '$' gewoon weer te geven in het\n"
		" antwoord het voorafgegaan moet worden door een '\\'!",
		/* LANG_REDIRECT_MIN_SEND_INTERVAL */
		" Je kan een antwoord slechts eenmaal elke %d seconden doorsturen.\n"
		" U moet nog %d seconden wachten vooraleer u opnieuw een antwoord kan doorsturen.",
		/* LANG_REDIRECT_MIN_RECV_INTERVAL */
		" Er moet een minimum interval van %d seconden zijn tussen 2 opeenvolgende antwoorden die doorgestuurd worden naar een enkele gebruiker.\n"
		" Gebruiker %s kan geen doorgestuurd antwoord ontvangen voor de volgende %d seconden.",
		/* LANG_REDIRECT_COMPLETE */
		" Doorsturen voltooid."
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
	moduleInsertLanguage(LANG_NL, LANG_NUM_STRINGS, langtable_nl);
}

/* ------------------------------------------------------------------------------- */

/**
 * Send a notice to the user in the correct language, or english.
 * This custom version of moduleNoticeLang will replace certain pre-defined strings
 * in the lang files.
 * %BSF is replaced by the BSFantasyCharacter.
 *
 * @param source Who sends the notice
 * @param u The user to send the message to
 * @param number The message number
 * @param ... The argument list
 **/
void myModuleNoticeLang(char *source, User * u, int number, ...) {
	va_list va;
	char buffer[4096], outbuf[4096];
	int lang = NSDefLanguage;
	char *s, *t, *buf, *fmt = NULL;

	if ((mod_current_module_name) && (!mod_current_module || strcmp(mod_current_module_name, mod_current_module->name)))
		mod_current_module = findModule(mod_current_module_name);

	/* Find the users lang, and use it if we can */
	if (u && u->na && u->na->nc)
		lang = u->na->nc->language;

	/* If the users lang isnt supported, drop back to English */
	if (mod_current_module->lang[lang].argc == 0)
		lang = LANG_EN_US;

	/* If the requested lang string exists for the language */
	if (mod_current_module->lang[lang].argc > number) {
		fmt = mod_current_module->lang[lang].argv[number];

		buf = str_replace("%BSF", BSFantasyCharacter, fmt);
		va_start(va, number);
		vsnprintf(buffer, 4095, buf, va);
		va_end(va);
		s = buffer;
		while (*s) {
			t = s;
			s += strcspn(s, "\n");
			if (*s)
				*s++ = '\0';
			strscpy(outbuf, t, sizeof(outbuf));
			notice_user(source, u, "%s", outbuf);
		}
		free(buf);
	} else
		alog("%s: INVALID language string call, language: [%d], String [%d]", mod_current_module->name, lang, number);
}

/**
 * Get the text of the given lanugage string in the corrent language, or in english.
 * This custom version of moduleGetLangString will replace certain pre-defined strings
 * in the lang files.
 * Note that unlike with the original moduleGetLangString, the return string needs
 * to be free'd.
 *
 * @param u The user to send the message to
 * @param number The message number
 **/
char *myModuleGetLangString(User * u, int number) {
	int lang = NSDefLanguage;

	if ((mod_current_module_name) && (!mod_current_module || strcmp(mod_current_module_name, mod_current_module->name)))
		mod_current_module = findModule(mod_current_module_name);

	/* Find the users lang, and use it if we can */
	if (u && u->na && u->na->nc)
		lang = u->na->nc->language;

	/* If the users lang isnt supported, drop back to English */
	if (mod_current_module->lang[lang].argc == 0)
		lang = LANG_EN_US;

	/* If the requested lang string exists for the language */
	if (mod_current_module->lang[lang].argc > number)
		return str_replace("%BSF", BSFantasyCharacter, mod_current_module->lang[lang].argv[number]);

	/* Return an empty string otherwise, because we might be used without
	 * the return value being checked. If we would return NULL, bad things
	 * would happen!
	 */
	alog("%s: INVALID language string call, language: [%d], String [%d]", mod_current_module->name, lang, number);
	return "";
}

/* ------------------------------------------------------------------------------- */

/**
 * This will replace all occurances of replace_target by replace_with in the source string.
 * Returns pointer to the new string.
 *
 * Note that all passed strings should be terminated.
 * Also note we return a NEW string that will need to be free'd.
 **/
char *str_replace(char *replace_target, char *replace_with, char *source) {
	char *result, *tmp, *end_r, *end_s;
	int count = 0, length, diff;

	if (!replace_target || !replace_with || !source)
		return NULL;

	/* See how many there are.. */
	end_s = source;
	while((tmp = strstr(end_s, replace_target))) {
		end_s = tmp + strlen(replace_target);
		count++;
	}

	/* Alocate the space we ll need.. */
	diff = strlen(replace_with) - strlen(replace_target);
	length = strlen(source) + count * diff;
	result = malloc(sizeof(char*) * length + 1);

	end_s = source;
	end_r = result;

	/* Copy the data to the new string.. */
	while((tmp = strstr(end_s, replace_target))) {
		memcpy(end_r, end_s, tmp - end_s);
		end_r += tmp - end_s;

		memcpy(end_r, replace_with, strlen(replace_with));
		end_r += strlen(replace_with);

		end_s = tmp + strlen(replace_target);
	}

	memcpy(end_r, end_s, strlen(end_s));
	memset(result + length, 0, 1);

	return result;
}

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
