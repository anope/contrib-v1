/**
 * Main routines for handling SQL Commands.
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
 * Last Updated   : 15/02/2011
 *
 **/

#include "misc_sqlcmd.h"

/**
 * Create an SQLCmd entry and hook it in.
 **/
int sqlcmd_create(char *name, int params, int (*handle)(int ac, char **av, char *pass)) {
	SQLCmd *cmd = NULL;
	SQLCmd *tmp = NULL;
	
	cmd = smalloc(sizeof(SQLCmd));
	if (!cmd) {
		alog("[misc_sqlcmd] Error: Cannot allocate memory!");
		return 1;
	}

	cmd->name = sstrdup(name);
	cmd->params = params;
	cmd->handle = handle;
	cmd->prev = NULL;
	cmd->next = NULL;
	
	if (SQLCmdHead == NULL) {
		SQLCmdHead = cmd;
		SQLCmdHead->prev = NULL;
		SQLCmdHead->next = NULL;
	} else {
		for (tmp = SQLCmdHead; tmp; tmp = tmp->next) {
			if (tmp->next == NULL)
				break;
		}
		if (!tmp) {
			free(cmd->name);
			free(cmd);
			alog("[misc_sqlcmd] Error: Cannot find where to put new hook!");
			return 1;
		}
		tmp->next = cmd;
		cmd->prev = tmp;
		cmd->next = NULL;
	}
	return 0;
}

/**
 * Find an SQLCmd entry based on name.
 **/
SQLCmd *sqlcmd_find(char *name) {
	SQLCmd *tmp = NULL;

	for (tmp = SQLCmdHead; tmp; tmp = tmp->next) { 
		if (stricmp(tmp->name, name) == 0)
			return tmp;
	}

	return NULL;
}

/**
 * Update the database with the status info.
 **/
int sqlcmd_status_update(int id, int status) {
	if (debug)
		alog("[misc_sqlcmd] debug: Command processed with status %d", status);
	snprintf(sql, MAX_SQL_BUF, "UPDATE anope_sqlcmd SET status = %d WHERE id = %d", status, id);
	sqlcmd_mysql_query(sql, 0);
	return 0;
}

/* EOF */
