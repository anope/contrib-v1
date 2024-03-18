/**
 * Module's main headers file. Includes all required components.
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
 * Last Updated   : 14/01/2011
 *
 **/

#include "module.h"
#include "encrypt.h"
#include "misc_sqlcmd_errors.h"

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
#define VERSION "3.0.0"

/* Struct Declarations and Typedefs */
typedef struct _sqlcmd_status SQLCmdStatus;
typedef struct _sqlcmd_command SQLCmd;

struct _sqlcmd_status {
	int status;
	char *msg;
};

struct _sqlcmd_command {
	char *name;
	int params;
	int (*handle)(int ac, char **av);
	SQLCmd *prev;
	SQLCmd *next;
};


/* Imported Variables */
#ifdef _WIN32
  extern __declspec(dllimport) do_mysql;
#else
  extern int do_mysql;
#endif


/* Global Variables */
int supported;
E char sql[MAX_SQL_BUF];
E MYSQL *sqlcmd_mysql;
E MYSQL_RES *sqlcmd_res;
E MYSQL_RES *chksum_res;
E MYSQL_ROW sqlcmd_row;
E MYSQL_ROW chksum_row;
E int sqlcmd_timeout;
E int sqlcmd_mysql_connected;
E char *sqlcmd_chksum_salt;
E SQLCmd *SQLCmdHead;


/* Functions - Main */
int reload_config(int argc, char **argv);
int sqlcmd_conf_load(void);
int check_modules(void);
int sqlcmd_handle(int ac, char **av);
E int sqlcmd_handle_ping(int ac, char **av);

/* Functions - Util */
E int na_is_services_root(NickAlias *na);
E int na_is_services_admin(NickAlias *na);
E int na_is_services_oper(NickAlias *na);
E void bot_unassign(ChannelInfo *ci, char *user);
E char *sqlcmd_checksum(char *cmd, char *param_str, char *ts);
E int str_to_params(char *params, int ac, char **param_array);
E NickRequest *makerequest(const char *nick);
E NickAlias *makenick(const char *nick);
E NickAlias *makealias(const char *nick, NickCore *nc);
E void new_memo_mail(NickCore * nc, Memo * m);

/* Functions - MySQL */
E int sqlcmd_mysql_connect();
E int sqlcmd_mysql_query(char *sql, int store_result); 
E char *sqlcmd_mysql_quote(char *unquoted);
E void sql_del_cs_info(char *chan);

/* Functions - Obj */
E int sqlcmd_create(char *name, int params, int (*handle)(int ac, char **av));
E SQLCmd *sqlcmd_find(char *name);
E int sqlcmd_status_update(int id, int status);

/* Functions - NickServ */
E int sqlcmd_handle_nickreg(int ac, char **av);
E int sqlcmd_handle_nickconf(int ac, char **av);
E int sqlcmd_handle_nickgroup(int ac, char **av);
E int sqlcmd_handle_nickdrop(int ac, char **av);

/* Functions - ChanServ */
E int sqlcmd_handle_chanreg(int ac, char **av);
E int sqlcmd_handle_chanaddsop(int ac, char **av);
E int sqlcmd_handle_chanaddaop(int ac, char **av);
E int sqlcmd_handle_chanaddhop(int ac, char **av);
E int sqlcmd_handle_chanaddvop(int ac, char **av);
E int sqlcmd_handle_addxop(int ac, char **av, int lvl);
E int sqlcmd_handle_chanaddaccess(int ac, char **av);
E int sqlcmd_handle_chandelaccess(int ac, char **av);
E int sqlcmd_do_access_add(ChannelInfo *ci, char *target, int lvl, char *by);
E int sqlcmd_do_access_del(ChannelInfo *ci, char *target,  char *by);
E int sqlcmd_handle_chantopic(int ac, char **av);
E int sqlcmd_handle_chandrop(int ac, char **av);

/* Functions - BotServ */
E int sqlcmd_handle_botassign(int ac, char **av);
E int sqlcmd_handle_botsay(int ac, char **av);
E int sqlcmd_handle_botact(int ac, char **av);
E int sqlcmd_handle_botunassign(int ac, char **av);

/* Functions - MemoServ */
E int sqlcmd_handle_memosend(int ac, char **av);
E int sqlcmd_handle_memodel(int ac, char **av);
int del_memo_callback(User * u, int num, va_list args);
E int sqlcmd_handle_memoclear(int ac, char **av);

/* EOF */
