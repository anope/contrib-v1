/**
 * -----------------------------------------------------------------------------
 * Name    : ns_vhost
 * Author  : Aragon Gouveia <aragon@phat.za.net>
 * Date    : 2009/08/23
 * Version : 0.2
 * -----------------------------------------------------------------------------
 * Limitations: Any IRCd which supports CHGHOST
 * Tested     : Anope-1.8.2 + UnrealIRCd 3.2.8
 * Requires   : Anope-1.8.2
 * -----------------------------------------------------------------------------
 **/

/**
 * Configuration directives that should be copy-pasted to services.conf

# VhostDB [OPTIONAL]
# Module: ns_vhost
#
# Use the given filename as database to store the AJOINs.
# If not given, the default of "vhost.db" will be used.
#
#VhostDB "vhost.db"

HELP NEEDED
It would be good if compat_modules() did something useful.  Currently it does
nothing.  I need your feedback regarding ns_vhost compatibility with other modules.
If you encounter another module that clashes with ns_vhost, please let me know.

 *
 **/

#include <sys/param.h>
#include "module.h"
#define AUTHOR "Aragon Gouveia"
#define VERSION "0.2"
#define MODNAME "ns_vhost"

#if (NICKMAX == 32 && HOSTMAX == 64)
#define VHOSTDBVERSION 1
#else
#error "This module won't work with your version of Anope."
#endif

#define VHOSTDBFILE "vhost.db"
#define VHOSTLENMIN 5
#define VHOSTMAX 10000

// Response message defines
#define MSG_NUM_STRINGS 		22
#define MSG_SYSERROR			0
#define MSG_NS_SET_HELP			1
#define MSG_NS_SET_VHOST_HELP		2
#define MSG_NS_SET_VHOST_GOOD		3
#define MSG_NS_SET_VHOST_INVALID	4
#define MSG_NS_SET_VHOST_FORBIDDEN	5
#define MSG_NS_SET_VHOST_DELAY		6
#define MSG_NS_SET_VHOST_UNSET		7
#define MSG_NS_INFO_VHOST		8
#define MSG_OS_FVHOST_HELP		9
#define MSG_OS_FVHOST_SYNTAX		10
#define MSG_OS_FVHOST_ADD_HELP		11
#define MSG_OS_FVHOST_ADD_SYNTAX	12
#define MSG_OS_FVHOST_LIST_HELP		13
#define MSG_OS_FVHOST_DEL_HELP		14
#define MSG_OS_FVHOST_DEL_SYNTAX	15
#define MSG_OS_FVHOST_ADDED		16
#define MSG_OS_FVHOST_ADDED_ALREADY	17
#define MSG_OS_FVHOST_DELED		18
#define MSG_OS_FVHOST_NOTFOUND		19
#define MSG_OS_FVHOST_LIST		20
#define MSG_OS_FVHOST_LIST_EMPTY	21

// vhost set return values
#define VHOSTSET_SUCCESS 	0
#define VHOSTSET_INVALID	1
#define VHOSTSET_FORBIDDEN	2
#define VHOSTSET_NOTFOUND	3
#define VHOSTSET_ERROR		4

// fvhost list return values
#define FVHOSTLIST_ZERO		0
#define FVHOSTLIST_END		1
#define FVHOSTLIST_MORE		2

// Database return values
#define DB_READ_SUCCESS   0
#define DB_READ_ERROR     1
#define DB_EOF_ERROR      2
#define DB_VERSION_ERROR  3

// Throttle times
#define TH_TIME_BASE 10
#define TH_TIME_BKOFF 604800

struct VhostDB {
	char filename[MAXPATHLEN];
	int vhost_count;
	int fvhost_count;
	struct Vhost {
		char nick[NICKMAX];
		char vhost[HOSTMAX];
		struct Vhost *next;
	} *vhosts;
	struct Vhost *fvhosts;
	int changes;
} VhostDB = {"", 0, 0, NULL, NULL, 0};

struct VhostThrottle {
	char nick[NICKMAX];
	int changes;
	time_t lastchange;
	struct VhostThrottle *next;
} *VhostThrottle = NULL;

size_t gensize;

// NickServ functions
int ns_identify(User *u);
int ns_info(User *u);
int ns_set(User *u);
int ns_saset(User *u);
static void ns_set_vhost(User *u, NickAlias *na, char *vhost, const int fcheck);
int help_ns_set_vhost(User *u);
int help_ns_set(User *u);
static void help_ns_set_vhost_delay(User *u, int delay);

// OperServ functions
int os_fvhost(User *u);
int help_os_fvhost(User *u);
int help_os_fvhost_add(User *u);
int help_os_fvhost_list(User *u);
int help_os_fvhost_del(User *u);

// Event functions
int ev_db_save(int argc, char **argv);
int ev_reload(int argc, char **argv);
int ev_db_backup(int argc, char **argv);

// Backend functions
static void config_load(void);
static int vhostdb_init(void);
static int vhostdb_read_end(FILE *fp, const int ret);
static int vhostdb_read_vhosts(FILE *fp, int vhost_count, struct Vhost *wvhost);
static int vhostdb_read(void);
static void vhostdb_write(void);
static int vhost_canset(const char *nick);
static int vhost_set(const char *nick, const char *vhost, const int fcheck);
static int fvhost_set(char *nick, const char *vhost);
static char *vhost_get(const char *nick);
static int fvhost_list(char *nick, char *vhost);

// Misc utility functions
static void add_languages(void);
static int supported_ircd(void);
static int compat_modules(void);
static int wildmatch(const char *string, const char *wild);
static int wildcmp(const char *s1, const char *s2);
static int vhost_valid(const char *vhost, const int wilds);
static void vhost_log(const char *fmt, ...);

int
AnopeInit(int argc, char **argv) {
	Command *c;
	EvtHook *hook;

	vhost_log("%s %s initialising...", MODNAME, VERSION);

	if (!supported_ircd()) {
		vhost_log("ERROR: IRCd not supported by this module");
		return MOD_STOP;
	}

	if (!moduleMinVersion(1,8,2,0)) {
		vhost_log("Your version of Anope isn't supported");
		return MOD_STOP;
	}

	if (!compat_modules())
		return MOD_STOP;

	// Hook onto the IDENTIFY and UPDATE NickServ commands
	c = createCommand("IDENTIFY", ns_identify, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV,c,MOD_TAIL) != MOD_ERR_OK) {
		vhost_log("Failed hooking to IDENTIFY command");
		return MOD_STOP;
	}
	c = createCommand("ID", ns_identify, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV,c,MOD_TAIL) != MOD_ERR_OK) {
		vhost_log("Failed hooking to ID command");
		return MOD_STOP;
	}
	c = createCommand("UPDATE", ns_identify, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV,c,MOD_TAIL) != MOD_ERR_OK) {
		vhost_log("Failed hooking to UPDATE command");
		return MOD_STOP;
	}

	// Create the SET VHOST command
	c = createCommand("SET VHOST", NULL, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		vhost_log("Failed creating SET VHOST help");
		return MOD_STOP;
	}
	moduleAddHelp(c, help_ns_set_vhost);
	c = createCommand("SET", ns_set, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		vhost_log("Failed hooking to SET command");
		return MOD_STOP;
	}
	c = createCommand("SET", ns_set, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV, c, MOD_TAIL) != MOD_ERR_OK) {
		vhost_log("Failed adding to SET listing");
		return MOD_STOP;
	}
	moduleAddHelp(c, help_ns_set);
	c = createCommand("INFO", ns_info, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV, c, MOD_TAIL) != MOD_ERR_OK) {
		vhost_log("Failed hooking to INFO command");
		return MOD_STOP;
	}
	
	// create SASET VHOST command
	c = createCommand("SASET", ns_saset, is_services_oper, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		vhost_log("Failed hooking to SASET command");
		return MOD_STOP;
	}
	
	// create FVHOST command
	c = createCommand("FVHOST", os_fvhost, is_services_oper, -1, -1, -1, -1, -1);
	if (moduleAddCommand(OPERSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		vhost_log("Failed creating OperServ FVHOST command");
		return MOD_STOP;
	}
	moduleAddHelp(c, help_os_fvhost);
	c = createCommand("FVHOST ADD", NULL, is_services_oper, -1, -1, -1, -1, -1);
	if (moduleAddCommand(OPERSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		vhost_log("Failed creating OperServ FVHOST ADD command");
		return MOD_STOP;
	}
	moduleAddHelp(c, help_os_fvhost_add);
	c = createCommand("FVHOST LIST", NULL, is_services_oper, -1, -1, -1, -1, -1);
	if (moduleAddCommand(OPERSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		vhost_log("Failed creating OperServ FVHOST LIST command");
		return MOD_STOP;
	}
	moduleAddHelp(c, help_os_fvhost_list);
	c = createCommand("FVHOST DEL", NULL, is_services_oper, -1, -1, -1, -1, -1);
	if (moduleAddCommand(OPERSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		vhost_log("Failed creating OperServ FVHOST DEL command");
		return MOD_STOP;
	}
	moduleAddHelp(c, help_os_fvhost_del);

	hook = createEventHook(EVENT_RELOAD, ev_reload);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		vhost_log("Can't hook to EVENT_RELOAD event");
		return MOD_STOP;
	}
	hook = createEventHook(EVENT_DB_SAVING, ev_db_save);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		vhost_log("Can't hook to EVENT_DB_SAVING event");
		return MOD_STOP;
	}
	hook = createEventHook(EVENT_DB_BACKUP, ev_db_backup);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		vhost_log("Can't hook to EVENT_DB_BACKUP event");
		return MOD_STOP;
	}

	config_load();
	add_languages();
	if (!vhostdb_init()) {
		vhost_log("Can't init VhostDB");
		return MOD_STOP;
	}

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	return MOD_CONT;
}

void
AnopeFini(void) {
	struct Vhost *pvhost = NULL;
	struct VhostThrottle *pvthrottle = NULL;
	
	while (VhostDB.vhosts) {
		pvhost = VhostDB.vhosts;
		VhostDB.vhosts = VhostDB.vhosts->next;
		free(pvhost);
	}
	while (VhostDB.fvhosts) {
		pvhost = VhostDB.fvhosts;
		VhostDB.fvhosts = VhostDB.fvhosts->next;
		free(pvhost);
	}
	while (VhostThrottle) {
		pvthrottle = VhostThrottle;
		VhostThrottle = VhostThrottle->next;
		free(pvthrottle);
	}
}


// ########################
//    NICKSERV FUNCTIONS
// ########################

int
ns_identify(User *u) {
	char *vhost;
	
	if (nick_identified(u) && u->na->nc) {
		vhost = vhost_get(u->na->nick);
		if (vhost)
			anope_cmd_vhost_on(u->na->nick, NULL, vhost);
	}

	return MOD_CONT;
}

int
ns_info(User *u) {
	char *args, *vhost, *nick = NULL, *option = NULL;

	args = moduleGetLastBuffer();
	if (!args)
		return MOD_CONT;

	nick = myStrGetToken(args, ' ', 0);
	option = myStrGetToken(args, ' ', 1);
	
	if (nick && option && !strncasecmp(option, "ALL", 3*sizeof(char))) {
		vhost = vhost_get(nick);
		if (vhost)
			moduleNoticeLang(s_NickServ, u, MSG_NS_INFO_VHOST, vhost);
	}
	
	if (nick) free(nick);
	if (option) free(option);
	return MOD_CONT;
}

int
ns_set(User *u) {
	char *args, *cmd = NULL, *option = NULL;

	args = moduleGetLastBuffer();
	if (!args)
		return MOD_CONT;

	cmd = myStrGetToken(args, ' ', 0);
	option = myStrGetToken(args, ' ', 1);
	
	if (cmd && !strncasecmp(cmd, "VHOST", 5*sizeof(char)) && u->na) {
		free(cmd);
		if (!nick_identified(u)) {
			notice_lang(s_NickServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
		} else {
			ns_set_vhost(u, u->na, option, 1);
		}
		
		free(cmd);
		if (option) free(option);
		return MOD_STOP;
	}
	
	if (cmd) free(cmd);
	if (option) free(option);
	return MOD_CONT;
}

int
ns_saset(User *u) {
	char *args, *nick = NULL, *cmd = NULL, *option = NULL;
	NickAlias *na;

	args = moduleGetLastBuffer();
	if (!args)
		return MOD_CONT;

	nick = myStrGetToken(args, ' ', 0);
	cmd = myStrGetToken(args, ' ', 1);
	option = myStrGetToken(args, ' ', 2);
	
	if (nick && cmd && !strncasecmp(cmd, "VHOST", 5*sizeof(char))
			&& (na = findnick(nick))) {
		ns_set_vhost(u, na, option, 0);
		free(cmd);
		free(nick);
		if (option) free(option);
		return MOD_STOP;
	}
	
	if (nick) free(nick);
	if (cmd) free(cmd);
	if (option) free(option);
	return MOD_CONT;
}

// If called with NULL vhost, unset the vhost
static void
ns_set_vhost(User *u, NickAlias *na, char *vhost, const int fcheck) {
	int ret;
	
	if (!vhost) {
		ret = vhost_set(na->nick, NULL, fcheck);
		switch (ret) {
		case VHOSTSET_SUCCESS:
			if (na->u) anope_cmd_vhost_off(na->u);
		case VHOSTSET_NOTFOUND:
			moduleNoticeLang(s_NickServ, u, MSG_NS_SET_VHOST_UNSET);
			break;
		case VHOSTSET_ERROR:
			moduleNoticeLang(s_NickServ, u, MSG_SYSERROR);
			break;
		default:
			help_ns_set_vhost_delay(u, ret);
		}
	} else {
		ret = vhost_set(na->nick, vhost, fcheck);
		switch (ret) {
		case VHOSTSET_SUCCESS:
			moduleNoticeLang(s_NickServ, u, MSG_NS_SET_VHOST_GOOD, vhost);
			if (na->u) anope_cmd_vhost_on(na->u->nick, NULL, vhost);
			break;
		case VHOSTSET_INVALID:
			moduleNoticeLang(s_NickServ, u, MSG_NS_SET_VHOST_INVALID);
			break;
		case VHOSTSET_FORBIDDEN:
			moduleNoticeLang(s_NickServ, u, MSG_NS_SET_VHOST_FORBIDDEN);
			break;
		case VHOSTSET_ERROR:
			moduleNoticeLang(s_NickServ, u, MSG_SYSERROR);
			break;
		default:
			help_ns_set_vhost_delay(u, ret);
		}
	}
}

int
help_ns_set_vhost(User *u) {
	moduleNoticeLang(s_NickServ, u, MSG_NS_SET_VHOST_HELP);
	return MOD_CONT;
}

int
help_ns_set(User *u) {
	moduleNoticeLang(s_NickServ, u, MSG_NS_SET_HELP);
	return MOD_CONT;
}

static void
help_ns_set_vhost_delay(User *u, int delay) {
	if (delay >= 0) return;
	delay *= -1;
	if (delay > 7200) {
		moduleNoticeLang(s_NickServ, u,	MSG_NS_SET_VHOST_DELAY,
			delay/3600, "hours");
	} else if (delay > 120) {
		moduleNoticeLang(s_NickServ, u,	MSG_NS_SET_VHOST_DELAY,
			delay/60, "minutes");
	} else {
		moduleNoticeLang(s_NickServ, u,	MSG_NS_SET_VHOST_DELAY,
			delay, "seconds");
	}
}


// #############################
// ## OPERSERV FUNCTIONS
// #############################

int
os_fvhost(User *u) {
	char nick[NICKMAX], *args, *cmd = NULL, *option = NULL;
	int ret;

	args = moduleGetLastBuffer();
	if (!args) {
		moduleNoticeLang(s_OperServ, u, MSG_OS_FVHOST_SYNTAX);
		return MOD_STOP;
	}

	cmd = myStrGetToken(args, ' ', 0);
	option = myStrGetToken(args, ' ', 1);
	
	if (!cmd) {
		moduleNoticeLang(s_OperServ, u, MSG_OS_FVHOST_SYNTAX);
		if (option) free(option);
		return MOD_STOP;
	}
	
	strcpy(nick, u->na->nick);
	if (!strncasecmp(cmd, "ADD", 4*sizeof(char))) {
		if (option) {
			switch(fvhost_set(nick, option)) {
			case VHOSTSET_SUCCESS:
				moduleNoticeLang(s_OperServ, u, MSG_OS_FVHOST_ADDED, option);
				break;
			case VHOSTSET_INVALID:
				moduleNoticeLang(s_OperServ, u, MSG_NS_SET_VHOST_INVALID);
				break;
			case VHOSTSET_FORBIDDEN:
				moduleNoticeLang(s_OperServ, u, MSG_OS_FVHOST_ADDED_ALREADY, option, nick);
				break;
			case VHOSTSET_ERROR:
				moduleNoticeLang(s_OperServ, u, MSG_SYSERROR);
				break;
			}
		} else {
			moduleNoticeLang(s_OperServ, u, MSG_OS_FVHOST_ADD_SYNTAX);
		}
	} else if (!strncasecmp(cmd, "LIST", 5*sizeof(char))) {
		char vhost[HOSTMAX];
		
		while ((ret = fvhost_list(nick, vhost)) == FVHOSTLIST_MORE) {
			moduleNoticeLang(s_OperServ, u, MSG_OS_FVHOST_LIST, vhost, nick);
		}
		if (ret == FVHOSTLIST_END)
			moduleNoticeLang(s_OperServ, u, MSG_OS_FVHOST_LIST, vhost, nick);
		if (ret == FVHOSTLIST_ZERO)
			moduleNoticeLang(s_OperServ, u, MSG_OS_FVHOST_LIST_EMPTY);
	} else if (!strncasecmp(cmd, "DEL", 4*sizeof(char))) {
		if (option) {
			switch(fvhost_set(NULL, option)) {
			case VHOSTSET_SUCCESS:
				moduleNoticeLang(s_OperServ, u, MSG_OS_FVHOST_DELED, option);
				break;
			case VHOSTSET_NOTFOUND:
				moduleNoticeLang(s_OperServ, u, MSG_OS_FVHOST_NOTFOUND, option);
				break;
			case VHOSTSET_INVALID:
				moduleNoticeLang(s_OperServ, u, MSG_NS_SET_VHOST_INVALID);
				break;
			case VHOSTSET_ERROR:
				moduleNoticeLang(s_OperServ, u, MSG_SYSERROR);
				break;
			}
		} else {
			moduleNoticeLang(s_OperServ, u, MSG_OS_FVHOST_DEL_SYNTAX);
		}
	} else {
		moduleNoticeLang(s_OperServ, u, MSG_OS_FVHOST_SYNTAX);
	}
	
	if (cmd) free(cmd);
	if (option) free(option);
	return MOD_STOP;
}

int
help_os_fvhost(User *u) {
	moduleNoticeLang(s_OperServ, u, MSG_OS_FVHOST_HELP);
	return MOD_STOP;
}

int
help_os_fvhost_add(User *u) {
	moduleNoticeLang(s_OperServ, u, MSG_OS_FVHOST_ADD_HELP);
	return MOD_STOP;
}

int
help_os_fvhost_list(User *u) {
	moduleNoticeLang(s_OperServ, u, MSG_OS_FVHOST_LIST_HELP);
	return MOD_STOP;
}

int
help_os_fvhost_del(User *u) {
	moduleNoticeLang(s_OperServ, u, MSG_OS_FVHOST_DEL_HELP);
	return MOD_STOP;
}


// ###############################
// ##  EVENT FUNCTIONS
// ###############################

int
ev_db_save(int argc, char **argv) {
	if (argc >= 1 && !strcasecmp(argv[0], EVENT_STOP)) vhostdb_write();
	return MOD_CONT;
}

int
ev_reload(int argc, char **argv) {
	if (argc >= 1 && !strcasecmp(argv[0], EVENT_START)) config_load();
	return MOD_CONT;
}

int
ev_db_backup(int argc, char **argv) {
	if ((argc >= 1) && (!strcasecmp(argv[0], EVENT_STOP))) {
		ModuleDatabaseBackup(VhostDB.filename);
	}
	return MOD_CONT;
}


// ############################
// ##  BACKEND FUNCTIONS
// ############################

static void
config_load(void) {
	Directive confvalue = {
		"VhostDB", {
			{PARAM_STRING, PARAM_RELOAD, &(VhostDB.filename)},
			{PARAM_NONE, 0, NULL}
		}
	};
	moduleGetConfigDirective(&confvalue);
	if (!strlen(VhostDB.filename)) strcpy(VhostDB.filename, VHOSTDBFILE);
}

static int
vhostdb_init(void) {
	if (vhostdb_read() == DB_READ_SUCCESS)
		return 1;
	return 0;
}

static int
vhostdb_read_end(FILE *fp, const int ret) {
	fclose(fp);
	return ret;
}

static int
vhostdb_read_vhosts(FILE *fp, int vhost_count, struct Vhost *wvhost) {
	int ret;
	struct Vhost *nvhost;
	
	if (vhost_count == 0) return 1;
	do {
		nvhost = malloc(sizeof(struct Vhost));
		if (!nvhost) break;
		memset(nvhost, 0, sizeof(struct Vhost));
		ret = fread(nvhost, gensize, 1, fp);
		if (!ret) {
			free(nvhost);
			nvhost = NULL;
			break;
		}
		wvhost->next = nvhost;
		wvhost = nvhost;
	} while (--vhost_count > 0);
	if (!nvhost) {
		return 0;
	}
	return 1;
}

static int
vhostdb_read(void) {
	struct Vhost *wvhost;
	int dbver, vhost_count, ret;
	FILE *fp;
	
	if ((fp = fopen(VhostDB.filename, "r")) == NULL) {
		if ((fp = fopen(VhostDB.filename, "w")) == NULL) {
			return DB_READ_ERROR;
		} else {
			return vhostdb_read_end(fp, DB_READ_SUCCESS);
		}
	}
	
	if ((dbver = fgetc(fp)) == EOF)
		return vhostdb_read_end(fp, DB_READ_SUCCESS); // empty file - no problem
	
	if (dbver != VHOSTDBVERSION)
		return vhostdb_read_end(fp, DB_VERSION_ERROR);
	
	if ((vhost_count = fgetc(fp)) == EOF)
		return vhostdb_read_end(fp, DB_EOF_ERROR);
	VhostDB.vhost_count = vhost_count << 8;
	if ((vhost_count = fgetc(fp)) == EOF)
		return vhostdb_read_end(fp, DB_EOF_ERROR);
	VhostDB.vhost_count |= vhost_count;
	
	if (VhostDB.vhost_count != 0) {
		vhost_log("vhost_count: %d", VhostDB.vhost_count);
		gensize = (NICKMAX + HOSTMAX) * sizeof(char);
		VhostDB.vhosts = malloc(sizeof(struct Vhost));
		if (!VhostDB.vhosts) {
			vhost_log("Couldn't malloc for database read");
			return vhostdb_read_end(fp, DB_READ_ERROR);
		}
		memset(VhostDB.vhosts, 0, sizeof(struct Vhost));
		ret = fread(VhostDB.vhosts, gensize, 1, fp);
		if (!ret || !vhostdb_read_vhosts(fp, VhostDB.vhost_count-1,
				VhostDB.vhosts)) {
			return vhostdb_read_end(fp, DB_READ_ERROR);
		}
	}
	
	if ((vhost_count = fgetc(fp)) == EOF) return DB_EOF_ERROR;
	VhostDB.fvhost_count = vhost_count << 8;
	if ((vhost_count = fgetc(fp)) == EOF) return DB_EOF_ERROR;
	VhostDB.fvhost_count |= vhost_count;
	
	if (VhostDB.fvhost_count != 0) {
		vhost_log("fvhost_count: %d", VhostDB.fvhost_count);
		gensize = (NICKMAX + HOSTMAX) * sizeof(char);
		VhostDB.fvhosts = wvhost = malloc(sizeof(struct Vhost));
		if (!VhostDB.fvhosts) {
			vhost_log("Couldn't malloc for database read");
			return vhostdb_read_end(fp, DB_READ_ERROR);
		}
		memset(wvhost, 0, sizeof(struct Vhost));
		ret = fread(wvhost, gensize, 1, fp);
		if (!ret || !vhostdb_read_vhosts(fp, VhostDB.fvhost_count-1,
				VhostDB.fvhosts)) {
			return vhostdb_read_end(fp, DB_READ_ERROR);
		}
	}
	
	fclose(fp);
	return DB_READ_SUCCESS;
}

static void
vhostdb_write(void) {
	struct Vhost *wvhost;
	int vhost_count;
	FILE *fp;
	
	if (!VhostDB.changes) return;
	
	if ((fp = fopen(VhostDB.filename, "w+")) == NULL) {
		vhost_log("Unable to open %s for writing", VhostDB.filename);
		return;
	}
	
	if (fputc(VHOSTDBVERSION, fp) == EOF) goto vhostdb_write_err;
	
	vhost_count = VhostDB.vhost_count >> 8;
	if (fputc((unsigned char)vhost_count, fp) == EOF) goto vhostdb_write_err;
	vhost_count = VhostDB.vhost_count;
	if (fputc((unsigned char)vhost_count, fp) == EOF) goto vhostdb_write_err;
	
	wvhost = VhostDB.vhosts;
	while (wvhost) {
		if (!fwrite(wvhost->nick, NICKMAX, 1, fp)) goto vhostdb_write_err;
		if (!fwrite(wvhost->vhost, HOSTMAX, 1, fp)) goto vhostdb_write_err;
		wvhost = wvhost->next;
	}

	vhost_count = VhostDB.fvhost_count >> 8;
	if (fputc((unsigned char)vhost_count, fp) == EOF) goto vhostdb_write_err;
	vhost_count = VhostDB.fvhost_count;
	if (fputc((unsigned char)vhost_count, fp) == EOF) goto vhostdb_write_err;
	
	wvhost = VhostDB.fvhosts;
	while (wvhost) {
		if (!fwrite(wvhost->nick, NICKMAX, 1, fp)) goto vhostdb_write_err;
		if (!fwrite(wvhost->vhost, HOSTMAX, 1, fp)) goto vhostdb_write_err;
		wvhost = wvhost->next;
	}
	
	goto vhostdb_write_end;
	vhostdb_write_err:
	vhost_log("Unable to write to %s", VhostDB.filename);
	vhostdb_write_end:
	fclose(fp);
}

// returns time left until user is permitted to change his vhost again
// 0 = user may set a new vhost
static int
vhost_canset(const char *nick) {
	struct VhostThrottle *nvthrottle, *pvthrottle = NULL;
	int cmp, tdiff, i;
	time_t now;
	
	now = time(NULL);
	if (now < 0) return 0;
	
	nvthrottle = VhostThrottle;
	while (nvthrottle) {
		cmp = strncasecmp(nick, nvthrottle->nick, NICKMAX);
		if (cmp > 0) {
			// not our nick, but let's do some housekeeping
			if ((now - nvthrottle->lastchange) >= TH_TIME_BKOFF) {
				// Back-off time reached, delete this entry
				if (pvthrottle) {
					pvthrottle->next = nvthrottle->next;
					free(nvthrottle);
					nvthrottle = pvthrottle->next;
				} else {
					VhostThrottle = nvthrottle->next;
					free(nvthrottle);
					nvthrottle = VhostThrottle;
				}
				continue;
			}
		} else if (cmp == 0) {
			// entry found, update it
			if ((now - nvthrottle->lastchange) >= TH_TIME_BKOFF) {
				// Back-off time reached, start fresh
				nvthrottle->changes = 1;
				nvthrottle->lastchange = now;
				return 0;
			}
			
			// our all-critical delay math
			for (i=1, tdiff=TH_TIME_BASE; i<nvthrottle->changes;
					i++, tdiff*=TH_TIME_BASE);
			
			if ((now - nvthrottle->lastchange) >= tdiff) {
				// Yup, you may set your vhost
				nvthrottle->changes++;
				nvthrottle->lastchange = now;
				return 0;
			}
			// oh no, you have to wait longer!
			return (tdiff - (now - nvthrottle->lastchange));
		} else {
			// nick not found, and no chance of finding it now due to sorting
			// insert new entry here
			break;
		}
		pvthrottle = nvthrottle;
		nvthrottle = nvthrottle->next;
	}
	
	// if we've reached here, we need to insert/append a new record
	if (!(nvthrottle = malloc(sizeof(struct VhostThrottle)))) {
		vhost_log("Can't malloc for vhost_canset");
		return 0;
	}
	strlcpy(nvthrottle->nick, nick, NICKMAX);
	nvthrottle->changes = 1;
	nvthrottle->lastchange = now;
	if (pvthrottle) {
		nvthrottle->next = pvthrottle->next;
		pvthrottle->next = nvthrottle;
	} else {
		nvthrottle->next = NULL;
		VhostThrottle = nvthrottle;
	}
	return 0;
}

// if called with NULL vhost parameter, delete the entry for specified nick
// returns VHOSTSET_* or a negative value denoting time remaining from vhost_canset()
static int
vhost_set(const char *nick, const char *vhost, const int fcheck) {
	struct Vhost *wvhost, *nvhost, *pvhost=NULL;
	int cmp;
	
	if (vhost) {
		cmp = strlen(vhost);
		if (cmp < VHOSTLENMIN || cmp >= HOSTMAX) return VHOSTSET_INVALID;
		if (!vhost_valid(vhost, 0)) return VHOSTSET_INVALID;
	}
	
	if (fcheck) {
		if (vhost) {
			// is the vhost forbidden?
			wvhost = VhostDB.fvhosts;
			while (wvhost) {
				if (wildmatch(vhost, wvhost->vhost)) {
					return VHOSTSET_FORBIDDEN;
				}
				wvhost = wvhost->next;
			}
		}
		
		cmp = vhost_canset(nick);
		if (cmp > 0) return (-cmp);
	}
	
	// where are we going to put this vhost?
	wvhost = VhostDB.vhosts;
	while (wvhost) {
		// vhosts are sorted by nick
		cmp = strncasecmp(nick, wvhost->nick, NICKMAX-1);
		if (cmp < 0) {
			if (!vhost) {
				// deleting entry, but entry not found
				return VHOSTSET_NOTFOUND;
			} else {
				// insertion point found
				break;
			}
		} else if (cmp == 0) {
			if (!vhost) {
				// deleting entry
				break;
			} else {
				// same nick, new vhost, easy peasy
				strlcpy(wvhost->vhost, vhost, HOSTMAX);
				VhostDB.changes++;
				return VHOSTSET_SUCCESS;
			}
		}
		pvhost = wvhost;
		wvhost = wvhost->next;
	}
	
	if (!vhost) {
		if (!wvhost) return VHOSTSET_NOTFOUND;
		if (pvhost) {
			pvhost->next = wvhost->next;
			free(wvhost);
		} else {
			VhostDB.vhosts = wvhost->next;
			free(wvhost);
		}
		VhostDB.vhost_count--;
		VhostDB.changes++;
	} else {
		if (VhostDB.vhost_count == VHOSTMAX) return VHOSTSET_ERROR;
		if ((nvhost = malloc(sizeof(struct Vhost))) == NULL) {
			vhost_log("Unable to malloc new vhost entry");
			return VHOSTSET_ERROR;
		}
		memset(nvhost, 0, sizeof(struct Vhost));
		strlcpy(nvhost->nick, nick, NICKMAX);
		strlcpy(nvhost->vhost, vhost, HOSTMAX);
		nvhost->next = wvhost;
		
		if (pvhost) {
			pvhost->next = nvhost;
		} else {
			VhostDB.vhosts = nvhost;
		}
		VhostDB.vhost_count++;
		VhostDB.changes++;
	}
	
	return VHOSTSET_SUCCESS;
}

// if called with NULL nick parameter, delete the vhost
static int
fvhost_set(char *nick, const char *vhost) {
	struct Vhost *wvhost, *nvhost, *pvhost=NULL;
	int cmp;
	
	cmp = strlen(vhost);
	if (cmp < VHOSTLENMIN || cmp >= HOSTMAX)
		return VHOSTSET_INVALID;
	
	if (!vhost_valid(vhost, 1))
		return VHOSTSET_INVALID;
	
	wvhost = VhostDB.fvhosts;
	while (wvhost) {
		// fvhosts are sorted by vhost
		cmp = wildcmp(vhost, wvhost->vhost);
		if (cmp < 0) {
			if (!nick) {
				// we're deleting, but not found
				return VHOSTSET_NOTFOUND;
			} else {
				// insertion point found
				break;
			}
		} else if (cmp == 0) {
			if (!nick) {
				// we're deleting
				break;
			} else {
				// vhost already forbidden
				strcpy(nick, wvhost->nick);
				return VHOSTSET_FORBIDDEN;
			}
		}
		pvhost = wvhost;
		wvhost = wvhost->next;
	}
	
	if (!nick) {
		if (!wvhost) return VHOSTSET_NOTFOUND;
		if (pvhost) {
			pvhost->next = wvhost->next;
			free(wvhost);
		} else {
			VhostDB.fvhosts = wvhost->next;
			free(wvhost);
		}
		VhostDB.fvhost_count--;
		VhostDB.changes++;
	} else {
		if (VhostDB.fvhost_count == VHOSTMAX) return VHOSTSET_ERROR;
		if ((nvhost = malloc(sizeof(struct Vhost))) == NULL) {
			vhost_log("Unable to malloc new fvhost entry");
			return VHOSTSET_ERROR;
		}
		memset(nvhost, 0, sizeof(struct Vhost));
		strlcpy(nvhost->nick, nick, NICKMAX);
		strlcpy(nvhost->vhost, vhost, HOSTMAX);
		nvhost->next = wvhost;
		
		if (pvhost) {
			pvhost->next = nvhost;
		} else {
			VhostDB.fvhosts = nvhost;
		}
		VhostDB.fvhost_count++;
		VhostDB.changes++;
	}
	
	return VHOSTSET_SUCCESS;
}

static char *
vhost_get(const char *nick) {
	struct Vhost *wvhost;
	int cmp;
	
	wvhost = VhostDB.vhosts;
	while (wvhost) {
		cmp = strncasecmp(nick, wvhost->nick, NICKMAX);
		if (cmp > 0) {
			// keep searching
			wvhost = wvhost->next;
		} else if (cmp < 0) {
			// not found
			return NULL;
		} else {
			// found
			return wvhost->vhost;
		}
	}
	
	return NULL;
}

static int
fvhost_list(char *nick, char *vhost) {
	static struct Vhost *wvhost = NULL;
	
	if (wvhost == NULL)
		wvhost = VhostDB.fvhosts;
	
	if (wvhost == NULL)
		return FVHOSTLIST_ZERO;
	
	strcpy(nick, wvhost->nick);
	strcpy(vhost, wvhost->vhost);
	
	wvhost = wvhost->next;
	if (wvhost)
		return FVHOSTLIST_MORE;
		
	return FVHOSTLIST_END;
}


// ###############################
// ##  MISC UTILITY FUNCTIONS
// ###############################

static void
add_languages(void) {
	char *langtable_en_us[] = {
		// MSG_SYSERROR
		"Internal system error.",
		// MSG_NS_SET_HELP
		" \n"
		"Module " MODNAME ":\n"
		"    VHOST      Set your nick's virtual host.",
		// MSG_NS_SET_VHOST_HELP
		"Syntax: \002SET VHOST \037vhost\037\n"
		" \n"
		"Sets your nick's virtual host.\n"
		" \n"
		"Virtual hosts must be longer than 5 characters\n"
		"and shorter than 64 characters.  Only letters,\n"
		"numbers, and hyphens may appear in a virtual host\n"
		"as defined in RFC 3696 for DNS host names.\n"
		" \n"
		"To unset, set an empty vhost.\n"
		" \n"
		"Example:\n"
		"        \002/msg NickServ SET VHOST microsoft.com\002\n",
		// MSG_NS_SET_VHOST_GOOD
		"Virtual host set to \002%s\002",
		// MSG_NS_SET_VHOST_INVALID
		"Invalid virtual host specified.\n",
		// MSG_NS_SET_VHOST_FORBIDDEN
		"That virtual host is forbidden.  Please choose another.\n",
		// MSG_NS_SET_VHOST_DELAY
		"Sorry, you need to wait another %d %s before you can change your virtual host.\n",
		// MSG_NS_SET_VHOST_UNSET
		"Virtual host has been unset",
		// MSG_NS_INFO_VHOST
		" \n"
		"Module " MODNAME ":\n"
		"     Virtual host: %s\n",
		// MSG_OS_FVHOST_HELP
		"Syntax: \002FVHOST ADD \037vhost\037\002\n"
		"        \002FVHOST DEL \037vhost\037\002\n"
		"        \002FVHOST LIST\002\n"
		" \n"
		"Allows operators to manipulate the forbidden Vhost list.\n"
		"If a vhost is forbidden, users will be unable to set it as\n"
		"their vhost with the NickServ \002SET VHOST\002 command.\n"
		" \n"
		"This will not clear virtual hosts that have already been\n"
		"set by users.  Use NickServ's \002ASET\002 command to clear\n"
		"a virtual host already set.\n",
		// MSG_OS_FVHOST_SYNTAX
		"Syntax: \002FVHOST {ADD | DEL | LIST} [\037vhost\037]\002\n",
		// MSG_OS_FVHOST_ADD_HELP
		"Syntax: \002FVHOST ADD \037vhost\037\002\n"
		" \n"
		"Adds a vhost to the forbidden vhost list.\n",
		// MSG_OS_FVHOST_ADD_SYNTAX
		"Syntax: \002FVHOST ADD \037vhost\037\002\n",
		// MSG_OS_FVHOST_LIST_HELP
		"Syntax: \002FVHOST LIST\002\n"
		" \n"
		"Lists all vhosts in the forbidden vhost list.\n",
		// MSG_OS_FVHOST_DEL_HELP
		"Syntax: \002FVHOST DEL \037vhost\037\002\n"
		" \n"
		"Deletes a vhost from the forbidden vhost list.\n",
		// MSG_OS_FVHOST_DEL_SYNTAX
		"Syntax: \002FVHOST DEL \037vhost\037\002\n",
		// MSG_OS_FVHOST_ADDED
		"Virtual host \002%s\002 has been forbidden.\n",
		// MSG_OS_FVHOST_ADDED_ALREADY
		"Virtual host \002%s\002 is already forbidden by \002%s\002.\n",
		// MSG_OS_FVHOST_DELED
		"Virtual host \002%s\002 has been unforbidden.\n",
		// MSG_OS_FVHOST_NOTFOUND
		"Virtual host \002%s\002 not found.\n",
		// MSG_OS_FVHOST_LIST
		"\002%-50s\002 (set by \002%s\002)\n",
		// MSG_OS_FVHOST_LIST_EMPTY
		"Forbidden vhost list is empty\n"
	};

	moduleInsertLanguage(LANG_EN_US, MSG_NUM_STRINGS, langtable_en_us);
}

static int
supported_ircd(void) {
	if (!strcasecmp(IRCDModule, "unreal31"))
		return 1;

	if (!strcasecmp(IRCDModule, "unreal32"))
		return 1;

	if (!strcasecmp(IRCDModule, "inspircd11"))
		return 1;

	if (!strcasecmp(IRCDModule, "viagra"))
		return 1;
	
	if (!strcasecmp(IRCDModule, "ultimate2"))
		return 1;

	if (!strcasecmp(IRCDModule, "solidircd"))
		return 1;
	
	if (!strcasecmp(IRCDModule, "shadowircd"))
		return 1;
	
	if (!strcasecmp(IRCDModule, "rageircd"))
		return 1;

	return 0;
}

static int
compat_modules(void) {
	// needs work - please help me with feedback
	return 1;
}

static int
vhost_valid(const char *vhost, const int wilds) {
	// RFC3696 check
	do {
		if (!*vhost) return 0;
		if (*vhost == '-') return 0;
		if (*vhost == '.') return 0;
		do {
			if (*vhost == '-') continue;
			if (wilds && (*vhost == '*' || *vhost == '?')) continue;
			if (!isalnum(*vhost)) return 0;
		} while (*++vhost != '.' && *vhost);
		if (*(vhost-1) == '-') return 0;
	} while (*vhost++);
	return 1;
}

static int
wildmatch(const char *string, const char *wild) {
	// Written by Jack Handy - jakkhandy@hotmail.com
	// case folding added by Aragon Gouveia

	const char *cp = NULL, *mp = NULL;

	while ((*string) && (*wild != '*')) {
		if ((tolower(*wild) != tolower(*string)) && (*wild != '?')) {
			return 0;
		}
		wild++;
		string++;
	}

	while (*string) {
		if (*wild == '*') {
			if (!*++wild) return 1;
			mp = wild;
			cp = string+1;
		} else if ((tolower(*wild) == tolower(*string)) || (*wild == '?')) {
			wild++;
			string++;
		} else {
			wild = mp;
			string = cp++;
		}
	}

	while (*wild == '*') {
		wild++;
	}
	
	return !*wild;
}

static int
wildcmp(const char *s1, const char *s2) {
	int is1, is2;

	while (tolower(*s1) == tolower(*s2++))
		if (*s1++ == '\0')
			return (0);
	switch (*s1) {
	case '*':
		is1 = 0;
		break;
	case '?':
		is1 = 1;
		break;
	default:
		is1 = tolower(*s1);
	}
	switch (*--s2) {
	case '*':
		is2 = 0;
		break;
	case '?':
		is2 = 1;
		break;
	default:
		is2 = tolower(*s2);
	}
	return (is1 - is2);
}

static void
vhost_log(const char *fmt, ...) {
	char str[BUFSIZE];
	char nvstr[BUFSIZE];
	va_list args;
	
	if (!fmt) return;
	
	va_start(args, fmt);
	vsnprintf(str, sizeof(str), fmt, args);
	va_end(args);
	
	strlcpy(nvstr, "[" MODNAME "] ", sizeof(str));
	strlcat(nvstr, str, sizeof(str));
	
	alog(nvstr);
}

// EOF
