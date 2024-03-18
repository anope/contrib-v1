/**
 * Main routines for performing channel operations.
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
 * Last Updated   : 16/02/2011
 *
 **/

#include "misc_sqlcmd.h"

/**
 * Check connection to mysql and connect if needed.
 **/
int sqlcmd_mysql_connect() {
	if (sqlcmd_mysql_connected == 1)
		return 1;
	if (!do_mysql) {
		alog("[misc_sqlcmd] ERROR: MySQL is disabled in Anope - Please configure Anope to use MySQL");
		return 0;
	}

	sqlcmd_mysql = mysql_init(NULL);
	if (sqlcmd_mysql == NULL) {
		alog("[misc_sqlcmd] ERROR: Cannot create MySQL Object!");
		return 0;
	}

	if (MysqlSock) {
		if ((!mysql_real_connect(sqlcmd_mysql, MysqlHost, MysqlUser, MysqlPass, MysqlName, MysqlPort, MysqlSock, 0))) {
			log_perror("[misc_sqlcmd] MySQL Error: Cant connect to MySQL: %s\n", mysql_error(sqlcmd_mysql));
			return 0;
		}
	} else {
		if ((!mysql_real_connect(sqlcmd_mysql, MysqlHost, MysqlUser, MysqlPass, MysqlName, MysqlPort, NULL, 0))) {
			log_perror("[misc_sqlcmd] MySQL Error: Cant connect to MySQL: %s\n", mysql_error(sqlcmd_mysql));
			return 0;
		}
	}

	sqlcmd_mysql_connected = 1;
	return 1;
}

/**
 * Perform a MySQL query.
 *
 * @param sql String SQL query to execute.
 * @param store_result int Specifies whether, and where to store the query result.
 * @return 1 on success, 0 on failure.
 **/
int sqlcmd_mysql_query(char *sql, int store_result) {
	int lcv = 0, ret = 0;

	for (lcv = 0; lcv < MysqlRetries; lcv++) {
		if (!sqlcmd_mysql_connect())
			continue;
		if (debug)
			alog("[misc_sqlcmd] debug: Executing query: %s", sql);
		if (!mysql_query(sqlcmd_mysql, sql)) {
			if (store_result == 1)
				sqlcmd_res = mysql_store_result(sqlcmd_mysql);
			else if (store_result == 2)
				chksum_res = mysql_store_result(sqlcmd_mysql);
			ret = 1;
			break;
		}
		log_perror("[misc_sqlcmd] Unable to run query: %s\n", mysql_error(sqlcmd_mysql));
	}

	return ret;
}

/* ------------------------------------------------------------------------------- */

/**
 * Quote a string to prevent injections.
 * This function allocates memory and creates a new string so the result must be free'd.
 *
 * @param unquoted String Text to be escaped.
 * @return Pointer to newly created string with all special chars properly escaped.
 **/
char *sqlcmd_mysql_quote(char *unquoted, int size) {
	char *quoted;

	quoted = smalloc((1 + (size * 2)) * sizeof(char));
	mysql_real_escape_string(sqlcmd_mysql, quoted, unquoted, size);

	return quoted;
}

/* ------------------------------------------------------------------------------- */

/**
 * Delete a channel from the database.
 **/
void sql_del_cs_info(char *chan) {
	char *name = sqlcmd_mysql_quote(chan, strlen(chan));
	if (debug)
		alog("[misc_sqlcmd] debug: Deleting ChannelInfo %s from the database.", chan);
	snprintf(sql, MAX_SQL_BUF, 
			"DELETE FROM anope_cs_info WHERE name = '%s'; DELETE FROM anope_ms_info WHERE name = '%s';"
			"DELETE FROM anope_cs_access WHERE name = '%s;' DELETE FROM anope_cs_levels WHERE name = '%s';"
			"DELETE FROM anope_cs_akicks WHERE name = '%s;' DELETE FROM anope_cs_badwords WHERE name = '%s';"
			"DELETE FROM anope_cs_ttb WHERE name = '%s';", name, name, name, name, name, name, name);
	sqlcmd_mysql_query(sql, 0);
	free(name);
}

/* EOF */
