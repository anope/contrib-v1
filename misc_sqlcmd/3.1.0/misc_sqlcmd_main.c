/**
 * Main routines for receiving, parsing and executing commands.
 *
 ***********
 * Module Name    : misc_sqlcmd
 * Author         : Viper <Viper@Anope.org>
 *
 * More info on http://modules.anope.org and http://forum.anope.org
 ***********
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***********
 *
 * Last Updated   : 15/03/2011
 *
 **/

#include "misc_sqlcmd.h"

/* Constants */
int timeout_default = 10;
int sqlcmd_max_age = 300;			/* Timeframe (in seconds) wherein a command must be parsed. (Based on timestamp.) */
int sqlcmd_max_future = 30;			/* Max timeframe (in seconds) in which a command with a timestamp in the future will be parsed. */
 
/* Global Variables */
char sql[MAX_SQL_BUF];
MYSQL *sqlcmd_mysql;
MYSQL_RES *sqlcmd_res;
MYSQL_RES *chksum_res;
MYSQL_ROW sqlcmd_row;
MYSQL_ROW chksum_row;
int sqlcmd_timeout = 0;
int sqlcmd_mysql_connected = 0;
char *sqlcmd_chksum_salt = NULL;
SQLCmd *SQLCmdHead = NULL;

/* ------------------------------------------------------------------------------- */

/**
 * Prepare the module for use and set us up for receiving commands.
 *
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int ac, char **av) {
	int status = 0;
	EvtHook *hook = NULL;
	supported = 1;

	alog("[\002misc_sqlcmd\002] Loading module...");

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	if (!moduleMinVersion(1,8,5,3037)) {
		alog("[\002misc_sqlcmd\002] Your version of Anope isn't supported. Please update to a newer release.");
		return MOD_STOP;
	}

	check_modules();
	if (supported == 0) {
		alog("[\002misc_sqlcmd\002] Warning: Module continuing in unsupported mode!");
	} else if (supported == -1) {
		alog("[\002misc_sqlcmd\002] Unloading module due to incompatibilities!");
		return MOD_STOP;
	}

	if (!do_mysql) {
		alog("[\002misc_sqlcmd\002] ERROR: MySQL is not enabled/configured in services.conf!");
		alog("[\002misc_sqlcmd\002] ERROR: Please enable MySQL in services.conf before loading this module.");
		return MOD_STOP;
	}

	snprintf(sql, MAX_SQL_BUF, "DESCRIBE `anope_sqlcmd`");
	if (sqlcmd_mysql_query(sql, 1) == 0) {
		if (sqlcmd_res)
			mysql_free_result(sqlcmd_res);
		alog("[\002misc_sqlcmd\002] ERROR: Cannot perform MySQL query!");
		alog("[\002misc_sqlcmd\002] ERROR: Please ensure you import anope_sqlcmd.sql into your MySQL Database.");
		return MOD_STOP;
	}
	mysql_free_result(sqlcmd_res);

	/* Hook the SQLcmd's */
	status += sqlcmd_create("NICK_REQ", 2, sqlcmd_handle_nickreq);
	status += sqlcmd_create("NICK_REG", 2, sqlcmd_handle_nickreg);
	status += sqlcmd_create("NICK_CONF", 1, sqlcmd_handle_nickconf);
	status += sqlcmd_create("NICK_GROUP", 2, sqlcmd_handle_nickgroup);
	status += sqlcmd_create("NICK_DROP", 1, sqlcmd_handle_nickdrop);

	status += sqlcmd_create("CHAN_REG", 3, sqlcmd_handle_chanreg);
	status += sqlcmd_create("CHAN_ADD_SOP", 3, sqlcmd_handle_chanaddsop);
	status += sqlcmd_create("CHAN_ADD_AOP", 3, sqlcmd_handle_chanaddaop);
	status += sqlcmd_create("CHAN_ADD_HOP", 3, sqlcmd_handle_chanaddhop);
	status += sqlcmd_create("CHAN_ADD_VOP", 3, sqlcmd_handle_chanaddvop);
	status += sqlcmd_create("CHAN_ADD_ACC", 4, sqlcmd_handle_chanaddaccess);
	status += sqlcmd_create("CHAN_DEL_ACC", 3, sqlcmd_handle_chandelaccess);
	status += sqlcmd_create("CHAN_TOPIC", 3, sqlcmd_handle_chantopic);
	status += sqlcmd_create("CHAN_DROP", 2, sqlcmd_handle_chandrop);

	status += sqlcmd_create("BOT_ASSIGN", 3, sqlcmd_handle_botassign);
	status += sqlcmd_create("BOT_UNASSIGN", 2, sqlcmd_handle_botunassign);
	status += sqlcmd_create("BOT_SAY", 3, sqlcmd_handle_botsay);
	status += sqlcmd_create("BOT_ACT", 3, sqlcmd_handle_botact);

	status += sqlcmd_create("MEMO_SEND", 3, sqlcmd_handle_memosend);
	status += sqlcmd_create("MEMO_DEL", 3, sqlcmd_handle_memodel);
	status += sqlcmd_create("MEMO_CLEAR", 2, sqlcmd_handle_memoclear);

	status += sqlcmd_create("PING", 0, sqlcmd_handle_ping);

	if (status > 0) {
		alog("[\002misc_sqlcmd\002] ERROR: Could not create all SQLcmd Command Hooks");
		return MOD_STOP;
	}

	if (!sqlcmd_conf_load()) {
		alog("[\002misc_sqlcmd\002] ERROR: Cannot load configuration directives!");
		alog("[\002misc_sqlcmd\002] ERROR: Please read README.txt before loading this module.");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_RELOAD, reload_config);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002misc_sqlcmd\002] Can't hook to EVENT_RELOAD event");
		return MOD_STOP;
	}

	moduleAddCallback("sqlcmd", time(NULL) + sqlcmd_timeout, sqlcmd_handle, 0, NULL);
	alog("[\002misc_sqlcmd\002] Module loaded successfully...");
	return MOD_CONT;
}

/* AnopeFini - Unload the module */
void AnopeFini() {
	SQLCmd *tmp = NULL;
	SQLCmd *tmp2 = NULL;

	for (tmp = SQLCmdHead; tmp != NULL; tmp = tmp2) {
		if (tmp->next)
			tmp2 = tmp->next;
		else
			tmp2 = NULL;
		if (tmp->name)
			free(tmp->name);
		free(tmp);
	}
	moduleDelCallback("sqlcmd");
	alog("[\002misc_sqlcmd\002] Module Unloaded Successfully!");
}

/* ------------------------------------------------------------------------------- */

/**
 * Upon /os reload call the routines for reloading the configuration directives
 **/
int reload_config(int argc, char **argv) {
	int old_supported = supported;

	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			alog("[misc_sqlcmd]: Reloading configuration directives...");
			sqlcmd_conf_load();
		}
	}

	check_modules();
	if (supported != old_supported) {
		if (supported == 0) {
			alog("[\002misc_sqlcmd\002] Warning: Module continueing in unsupported mode!");
		} else if (supported == -1) {
			alog("[\002misc_sqlcmd\002] Disabling module due to incompatibilities!");
			return MOD_STOP;
		}
	}

	return MOD_CONT;
}

/**
 * Loads the modules configuration directives.
 **/
int sqlcmd_conf_load(void) {
	int i;

	Directive confvalues[][1] = {
		{{"SQLCmdChecksumSalt", {{PARAM_STRING, PARAM_RELOAD, &sqlcmd_chksum_salt}}}},
		{{"SQLCmdUpdateTimeout", {{PARAM_POSINT, PARAM_RELOAD, &sqlcmd_timeout}}}},
	};

	if (sqlcmd_chksum_salt)
		free(sqlcmd_chksum_salt);
	sqlcmd_chksum_salt = NULL;
	sqlcmd_timeout = timeout_default;

	for (i = 0; i < 2; i++)
		moduleGetConfigDirective(confvalues[i]);

	if (!sqlcmd_chksum_salt) {
		alog("[\002misc_sqlcmd\002] ERROR: SQLCmdChecksumSalt is NOT set in services.conf!");
		return 0;
	}

	if (sqlcmd_timeout <= 0) {
		alog("[misc_sqlcmd] SQLCmdUpdateTimeout is NOT set or has invalid value. Setting to default of %d.", timeout_default);
		sqlcmd_timeout = timeout_default;
	} else if (sqlcmd_timeout > 20)
		alog("[misc_sqlcmd] SQLCmdUpdateTimeout is greater than 20. It is recommended to keep this below 20!");

	if (debug)
		alog("[misc_sqlcmd] debug: Configuration loaded: SQLCmdChecksumSalt set to '%s', SQLCmdUpdateTimeout set to '%d.'", sqlcmd_chksum_salt, sqlcmd_timeout);

	return 1;
}

/* ------------------------------------------------------------------------------- */

/**
 * Checks whether the right conditions to continue loading are met.
 **/
int check_modules(void) {
#ifdef SUPPORTED
	if (supported >= 0) {
		if (findModule("os_raw")) {
			alog("[\002misc_sqlcmd\002] Unsupported module found: os_raw.. (This is fatal!)");
			supported = -1;
		}
#ifndef _WIN32
		if (findCommand(OPERSERV, "RAW")) {
			alog("[\002misc_sqlcmd\002] Unsupported module found: os_raw.. (This is fatal!)");
			supported = -1;
		}
#endif
		if (!DisableRaw) {
			alog("[\002misc_sqlcmd\002] RAW has NOT been disabled! (This is fatal!)");
			supported = -1;
		}
	}

	if (supported >= 0) {
		if (findModule("ircd_init")) {
			alog("[\002misc_sqlcmd\002] This module is unsupported in combination with ircd_init.");
			supported = 0;
		}

		if (findModule("cs_join")) {
			alog("[\002misc_sqlcmd\002] This module is unsupported in combination with cs_join.");
			supported = 0;
		}

		if (findModule("bs_logchanmon")) {
			alog("[\002misc_sqlcmd\002] This module is unsupported in combination with bs_logchanmon.");
			supported = 0;
		}

		if (findModule("ircd_gameserv")) {
			alog("[\002misc_sqlcmd\002] This module is unsupported in combination with ircd_gameserv.");
			supported = 0;
		}

		if (findModule("os_psuedo_cont")) {
			alog("[\002misc_sqlcmd\002] This module is unsupported in combination with os_psuedo_cont.");
			supported = 0;
		}
	}
#endif
	return supported;
}

/* ------------------------------------------------------------------------------- */

/**
 * Handle the callback from core every sqlcmd_timeout seconds.
 * This checks for commands in the SQL buffer and executes them.
 **/
int sqlcmd_handle(int ac, char **av) {
	char *param_array[12] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
	char *chksum = NULL;
	char pass[PASSMAX];
	unsigned long *lengths;
	SQLCmd *cmd = NULL;

	if (supported < 0)
		return MOD_CONT;

	snprintf(sql, MAX_SQL_BUF, "SELECT * FROM anope_sqlcmd WHERE status = 0 ORDER BY tstamp ASC");
	if (sqlcmd_mysql_query(sql, 1) == 0) {
		alog("[\002misc_sqlcmd\002] ERROR: Unable to query for commands!");
		return MOD_STOP;
	}

	if (debug)
		alog("[misc_sqlcmd] debug: There are %d functions awaiting processing...", (int) mysql_num_rows(sqlcmd_res));

	while ((sqlcmd_row = mysql_fetch_row(sqlcmd_res))) {
		int i = 0, cmd_status = 0, new_ac = 0;
		/* We need to get the length of the password.. */
		lengths = mysql_fetch_lengths(sqlcmd_res);
		memset(pass, 0, PASSMAX);

		if (debug)
			alog("[misc_sqlcmd] debug: Command %d: %s - Params: %s - Pass: %s - Timestamp: %s - Checksum: %s", 
					atoi(sqlcmd_row[0]), sqlcmd_row[1], sqlcmd_row[2], sqlcmd_row[3], sqlcmd_row[4], sqlcmd_row[5]);
		chksum = sqlcmd_checksum(sqlcmd_row[1], sqlcmd_row[2], sqlcmd_row[3], lengths[3], sqlcmd_row[4]);
		if (!chksum || stricmp(chksum, sqlcmd_row[5]) != 0) {
			alog("[\002misc_sqlcmd\002] ERROR: Invalid checksum received for ID %d.", atoi(sqlcmd_row[0]));
			cmd_status = SQLCMD_ERROR_CHECKSUM;
		} else if (!(cmd = sqlcmd_find(sqlcmd_row[1]))) {              
			alog("[\002misc_sqlcmd\002] ERROR: Received unknown command '%s' for ID %d.", sqlcmd_row[1], atoi(sqlcmd_row[0]));
			cmd_status = SQLCMD_ERROR_UNKNOWN_CMD;
		} else if (atoi(sqlcmd_row[4]) < time(NULL) - sqlcmd_max_age) {
			alog("[\002misc_sqlcmd\002] ERROR: Skipped command '%s' [ID: %d] for being older than %d seconds.", sqlcmd_row[1], atoi(sqlcmd_row[0]), sqlcmd_max_age);
			cmd_status = SQLCMD_ERROR_TIMESTAMP_PAST;
		} else if (atoi(sqlcmd_row[4]) > time(NULL) + sqlcmd_max_future) {
			alog("[\002misc_sqlcmd\002] ERROR: Command '%s' [ID: %d] has a timestamp too far (over %d seconds) into the future.", sqlcmd_row[1], atoi(sqlcmd_row[0]), sqlcmd_max_future);
			cmd_status = SQLCMD_ERROR_TIMESTAMP_FUTURE;
		} else {
			if (debug)
				alog("[misc_sqlcmd] debug: Command %d has a valid checksum - Processing...", atoi(sqlcmd_row[0]));

			new_ac = str_to_params(sqlcmd_row[2], cmd->params, param_array);
			if (lengths[3] >= PASSMAX)
				memcpy(pass, sqlcmd_row[3], PASSMAX - 1);
			else
				memcpy(pass, sqlcmd_row[3], lengths[3]);

			cmd_status = cmd->handle(new_ac, param_array, pass);

			for (i = 0; param_array[i]; i++) {
				free(param_array[i]);
				param_array[i] = NULL;
			}
		}
		if (cmd_status)
			sqlcmd_status_update(atoi(sqlcmd_row[0]), cmd_status);
		if (chksum)
			free(chksum);
	}
	mysql_free_result(sqlcmd_res);

	moduleAddCallback("sqlcmd", time(NULL) + sqlcmd_timeout, sqlcmd_handle, 0, NULL);
	return MOD_CONT;
}

/*************************************************************************/

/**
 * Handles a ping request to check whether anope is still alive.
 *
 * @param ac int Argument count. [Unused]
 * @param av Array Argument list. [Unused]
 * @param pass Password hash. [Unused]
 **/
int sqlcmd_handle_ping(int ac, char **av, char *pass) {
	return SQLCMD_ERROR_NONE;
}

/* EOF */
