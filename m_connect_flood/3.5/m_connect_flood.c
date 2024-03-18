/**
 * ----------------------------------------------------------------
 * Name    : m_connect_flood
 * Author  : Viper <Viper@Anope.org>
 * Date    : 29/05/2006  (Last updated: 12/11/2012)
 * Version : 3.5
 * ----------------------------------------------------------------
 * Tested  : Anope-1.8.7+ UnrealIRCd 3.2.8
 * ----------------------------------------------------------------
 * This module watches any new client connections. If the number of
 * connects reaches the maximum value in the given time, it will
 * activate the preconfigured defcon mode.
 *
 * Made a few changes to Certus's module to suit my needs...
 * Made it public since more ppl seem to be asking for this.
 *
 * Note: Configurations are now best made through config file.
 * I left the old /os connflood in place, but each time the
 * services.conf is reloaded and directives are present, changes
 * that have been made through operserv will be undone.
 *
 * ----------------------------------------------------------------
 * Changelog:
 *
 *   3.5  - Fixed defcon being triggered even if already active. (Reported by GTAXL.)
 *
 *   3.4  - Restructured module & cleanup code dating back to the stone age.
 *        - Fixed major bug causing settings loaded from services.conf failing to
 *              activate the flood protection. (Reported by GTAXL.)
 *        - Fixed DEFCON event and notices not being send.
 *
 *   3.3  - Added win32 support.
 *
 *   3.2  - Fixed bug causing AutoDefconLevel to be set incorrectly
 *              if not defined in services.conf
 *        - added sanity checks..
 *
 *   3.1  - Fixed crashbug if ConnectionLimit is not present in services.conf
 *
 *   3.0  - Defcon level is set in services.conf now
 *        - Connection limits are now set in services.conf
 *        - The module now defaults to a limit of 100 connections in 10 secs
 *
 * ----------------------------------------------------------------
 * Add this to services.conf !!
 *

# AutoDefconLevel <level> [OPTIONAL]
# Module m_connect_flood
#
# Set the defcon level u want services to go to when the connection limit is breached.
# If not defined, Services will go to defconn 3.
#
#AutoDefconLevel 3

# ConnectionLimit "<connecions> <period>" [OPTIONAL]
# Module m_connect_flood
#
# Defines the number of connection of the given period which will trigger automatic defcon.
# If not defined, services will go into defcon after 100 connects in 15 seconds.
#
# The connection limit can also be set with "/os CONNFLOOD <connections> <seconds>", but this
# will be overwritten next time the config is reloaded.
#
#ConnectionLimit "100 15"

 *
 **/


/*------------------------- Source - Don't change below --------------------------*/

#include "module.h"

#define AUTHOR "Viper"
#define VERSION "3.5"


/* Language defines */
#define LANG_NUM_STRINGS 					5

#define LANG_CONNFLOOD_DESC					0
#define LANG_CONNFLOOD_SYNTAX				1
#define LANG_CONNFLOOD_SYNTAX_EXT			2
#define LANG_VALUES_OUT_OF_RANGE			3
#define LANG_CONNFLOOD_SET					4


/* Constants */
static int DEF_DEFCON_LEVEL = 3;
static int DEF_CONNECTIONS = 100;
static int DEF_TIME = 15;


/* Variables */
int cf_count = 0, cf_lasttime = 0, cf_lastserver = 0;
int AutoDefconLevel, ConnectionLimitC, ConnectionLimitT;


/* Functions */
int help_connflood(User *u);
void list_cmds(User *u);

int cf_nick(int argc, char **arg);
int cf_server(int argc, char **arg);

int do_connflood(User *u);

void load_config(void);
int reload_config(int argc, char **argv);

void add_languages(void);


#ifdef _WIN32
extern MDE time_t DefContimer;
extern MDE void runDefCon(void);
#endif


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

	alog("[\002m_connect_flood\002] Loading module...");

	if (!moduleMinVersion(1,8,7,3089)) {
		alog("[\002m_connect_flood\002] Your version of Anope isn't supported. Please update to a newer release.");
		return MOD_STOP;
	}

	c = createCommand("CONNFLOOD", do_connflood, is_services_admin, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV, c, MOD_HEAD);
	moduleAddHelp(c, help_connflood);
	moduleSetOperHelp(list_cmds);

	hook = createEventHook(EVENT_NEWNICK, cf_nick);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002m_connect_flood\002] Can't hook to EVENT_NEWNICK event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_SERVER_CONNECT, cf_server);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002m_connect_flood\002] Can't hook to EVENT_SERVER_CONNECT event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_RELOAD, reload_config);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002m_connect_flood\002] Can't hook to EVENT_RELOAD event");
		return MOD_STOP;
	}

	load_config();
	add_languages();

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	alog("[\002m_connect_flood\002] Module loaded successfully...");
	return MOD_CONT;
}


/**
 * Unload the module
 **/
void AnopeFini(void) {
	alog("[\002m_connect_flood\002] Unloading module...");
}

/* ------------------------------------------------------------------------------- */

/**
 * Help functions
 **/
int help_connflood(User *u) {
	if (is_services_admin(u))
		moduleNoticeLang(s_OperServ, u, LANG_CONNFLOOD_SYNTAX_EXT, AutoDefconLevel);
	else
		notice_lang(s_OperServ, u, PERMISSION_DENIED);

	return MOD_STOP;
}


void list_cmds(User *u) {
	if (is_services_admin(u)) {
		moduleNoticeLang(s_OperServ, u, LANG_CONNFLOOD_DESC);
	}
}

/* ------------------------------------------------------------------------------- */

int cf_nick(int argc, char **arg) {
	int currenttime = time(NULL);

	/* let's pass conections for 5 seconds after a server joined,
	 * so we avoid malfunctions after a netsplit
	 */
	if (cf_lastserver) {
		if ((currenttime - cf_lastserver) < 5)
			return MOD_CONT;
		else
			/* Reset merge indicator if 5 seconds have passed.. */
			cf_lastserver = 0;
	}

	/* Increment connection cound; don't count connections during net merges.. */
	cf_count++;

	/* if (now - lastchecktime >= timevalue for connects) reset
	 * connect count and set lastchecktime to now [because we check
	 * for connections:SECONDS ]
	 */
	if ((currenttime - cf_lasttime) >= ConnectionLimitT) {
		cf_count = 1;
		cf_lasttime = currenttime;
	} else if (cf_count >= ConnectionLimitC && DefConLevel > AutoDefconLevel) {
		char buf[BUFSIZE];
		char *langglobal = getstring(NULL, DEFCON_GLOBAL);
		snprintf(buf, BUFSIZE, "%d", AutoDefconLevel);

		alog("CAUTION! Connection flood limit reached, possible bot-flood detected. Going into DefCon %d", AutoDefconLevel);
		wallops(s_OperServ, "CAUTION! Connection flood limit reached, possible bot-flood detected. Going into DefCon %d", AutoDefconLevel);
		DefConLevel = AutoDefconLevel;
		send_event(EVENT_DEFCON_LEVEL, 1, buf);
		DefContimer = currenttime;

		/* Global notice the users what is happening.
		 * The message set in services.conf is also send here - if set */
		if (GlobalOnDefcon) {
			if ((DefConLevel == 5) && (DefConOffMessage)) {
				oper_global(NULL, "%s", DefConOffMessage);
			} else {
				oper_global(NULL, langglobal, DefConLevel);
			}
		}
		if (GlobalOnDefconMore) {
			if ((DefConOffMessage) && DefConLevel != 5) {
				oper_global(NULL, "%s", DefconMessage);
			}
		}

		runDefCon();
	}
	return MOD_CONT;
}


/* set the time of the last SERVER event */
int cf_server(int argc, char **arg) {
	cf_lastserver = time(NULL);
	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

/**
 * This is legacy functionality alowing the limit to be set by SAs at runtime.
 **/
int do_connflood(User *u) {
	char *buffer, *conn, *time;
	int t_conn = 0, t_time = 0;
	
	buffer = moduleGetLastBuffer();
	conn = myStrGetToken(buffer, ' ', 0);
	time = myStrGetToken(buffer, ' ', 1);

	if (!conn || !time)  {
		moduleNoticeLang(s_OperServ, u, LANG_CONNFLOOD_SYNTAX);
	} else {
		t_conn = atoi(conn);
		t_time = atoi(time);

		if ((t_conn <= 0) || (t_time <= 0) || (t_conn > 300) || (t_time > 300)) {
			moduleNoticeLang(s_OperServ, u, LANG_VALUES_OUT_OF_RANGE);
		} else {
			ConnectionLimitC = t_conn;
			ConnectionLimitT = t_time;

			moduleNoticeLang(s_OperServ, u, LANG_CONNFLOOD_SET, AutoDefconLevel, ConnectionLimitC, ConnectionLimitT);
			alog("[m_connect_flood] New Connflood set by %s: services will now go into defcon %d after %d connections in %d secs",
					u->nick, AutoDefconLevel, ConnectionLimitC, ConnectionLimitT);
		}
	}

	if (conn) free(conn);
	if (time) free(time);
	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

void load_config(void) {
	int i;
	char *limits = NULL;

	Directive confvalues[][1] = {
		{{"AutoDefconLevel", {{PARAM_POSINT, PARAM_RELOAD, &AutoDefconLevel}}}},
		{{"ConnectionLimit", {{PARAM_STRING, PARAM_RELOAD, &limits}}}}
	};

	for (i = 0; i < 2; i++)
		moduleGetConfigDirective(confvalues[i]);

	if (AutoDefconLevel != 0) {
		if (AutoDefconLevel < 1 && AutoDefconLevel > 5) {
			AutoDefconLevel = DEF_DEFCON_LEVEL;
			alog("[m_connect_flood] AutoDefconLevel not configured correctly. Using default value %d. ", AutoDefconLevel);
		}
	} else {
		AutoDefconLevel = DEF_DEFCON_LEVEL;
		alog("[m_connect_flood] AutoDefconLevel not set in services.conf. Using default value %d. ", AutoDefconLevel);
	}

	if (limits) {
		char *arg1 = myStrGetToken(limits,' ',0);
		char *arg2 = myStrGetToken(limits,' ',1);

		if (!arg1 || !arg2) {
			ConnectionLimitC = DEF_CONNECTIONS;
			ConnectionLimitT = DEF_TIME;

			alog("[m_connect_flood] ConnectionLimit not configured correctly. Using default values: %d connections in %d seconds.", ConnectionLimitC, ConnectionLimitT);
		} else {
			ConnectionLimitC = atoi(arg1);
			ConnectionLimitT = atoi(arg2);

			if ((ConnectionLimitC <= 0) || (ConnectionLimitT <= 0) || (ConnectionLimitC > 300) || (ConnectionLimitT > 300)) {
				ConnectionLimitC = DEF_CONNECTIONS;
				ConnectionLimitT = DEF_TIME;

				alog("[m_connect_flood] The ConnectionLimit values must be between 0 and 300. Using default values: %d connections in %d seconds.", ConnectionLimitC, ConnectionLimitT);
			}
		}
		if (arg1) free(arg1);
		if (arg2) free(arg2);
	} else {
		ConnectionLimitC = DEF_CONNECTIONS;
		ConnectionLimitT = DEF_TIME;
		alog("[m_connect_flood] ConnectionLimit not set in services.conf. Using default values: %d connections in %d seconds.", ConnectionLimitC, ConnectionLimitT);
	}

	if (debug)
		alog("[m_connect_flood] Services will go into defcon %d after %d connections in %d secs", AutoDefconLevel, ConnectionLimitC, ConnectionLimitT);

	if (limits)
		free(limits);
}


int reload_config(int argc, char **argv) {
	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			alog("[m_connect_flood] Reloading configuration directives...");
			load_config();
		}
	}

	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

/**
 * Add language strings to the module's language db.
 **/
void add_languages(void) {
	char *langtable_en_us[] = {
		/* LANG_CONNFLOOD_DESC */
		"    CONNFLOOD   Sets the threshold for triggering auto-defcon.",
		/* LANG_CONNFLOOD_SYNTAX */
		"Syntax: \002CONNFLOOD \037connections\037 \037seconds>\037\002",
		/* LANG_CONNFLOOD_SYNTAX_EXT */
		"Syntax: \002CONNFLOOD \037connections\037 \037seconds>\037\002\n"
		"\n"
		"Sets the threshold rate at which services will automatically enable defcon.\n"
		"The defcon level is configured in the service config and cannot be changed dynamically. (Currently: %d)",
		/* LANG_VALUES_OUT_OF_RANGE*/
		"The values must be between 0 and 300.",
		/* LANG_CONNFLOOD_SET */
		"New connflood values set: services will now go into DefCon %d after %d connects in %d seconds).",
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}

/* EOF */
