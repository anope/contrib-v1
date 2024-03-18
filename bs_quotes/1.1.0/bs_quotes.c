/**
 * Modules Main Functions for loading and matching commands. - Source
 *
 ***********
 * Module Name: bs_quotes
 * Author: mikea - mikea@mjpa.co.uk
 * Creation Date: 12/06/2007
 * More info on http://forum.anope.org/index.php
 *
 ***********
 * 
 * This module adds a !quote fantasy command to BotServ. This enables
 * anyone on the CS access list (halfop+) to add/delete quotes, everyone
 * else can view a quote, view a random quote, search a quote or check how
 * many quotes exist.
 *
 * Commands:
 *
 * - !quote add <quote> - Adds a new quote
 * - !quote del [#]id - Deletes the quote with id 'id'. The # is optional
 * - !quote view [#]id - Views the quote with id 'id'. The # is optional
 * - !quote get [#]id - Same as !quote view
 * - !quote random - Views a random quote
 * - !quote search <text> - Returns a list of ids containing 'text'
 * - !quote count - Returns how many quotes are in the db.
 *
 ***********
 * 
 * Config directives:
 *
 * QuoteMySQLHost "sql.hostname" - The hostname for the MySQL server (for TCP connections)
 * QuoteMySQLPort 3306 - The port for the MySQL server (for TCP connections)
 * QuoteMySQLSocket "/tmp/mysql.sock" - The UNIX socket file for the MySQL server
 * QuoteMySQLUser "sql_user" - The username to use
 * QuoteMySQLPassword "pass" - The password for the specified username
 * QuoteMySQLDatabase "database" - The database to use for the quotes
 * QuoteEnableMultiLine - Enable multi-line support, disabled by default
 *
 * NOTE:
 *
 * To use TCP connections, the QuoteMySQLHost and QuoteMySQLPort are required.
 * To use a UNIX socket file, comment out the QuoteMySQLHost and use QuoteMySQLSocket
 * 
 ***********
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***********
 *
 * Last Updated: 17/06/2007
 *
 * v1.0 - 14/06/2007 - Initial release
 *
 * v1.1 - 18/06/2007 - Added multiline support, when adding quotes use !n! to insert a new line
 *                   - Extra config option "QuoteEnableMultiLine" to enable multi-line support
 *                   - Made the MySQL connection permanent to allow a single query method
 *
 **/

#include "module.h"
#include <time.h>
#include <mysql.h>

#define AUTHOR               "mikea - mikea@mjpa.co.uk"
#define VERSION              "$Id: bs_quotes.c v1.1.0 11-06-2007 mikea $"

/* Various messages sent to channel, careful when changing not to remove any arguments! */
#define QUOTE_SYNTAX         "[QUOTE] Syntax: add <msg>, del <id>, view/get <id>, random, search <text>, count."
#define QUOTE_NO_QUOTE       "[QUOTE] No such quote."
#define QUOTE_SUCCESS_ADD    "[QUOTE] Successfully added quote #%llu."
#define QUOTE_SUCCESS_DEL    "[QUOTE] Successfully removed quote #%llu."
#define QUOTE_TEMPLATE       "[QUOTE #%s] %s"
#define QUOTE_TEMPLATE_BY    "[QUOTE INFO] Added by %s on the %s."
#define QUOTE_SEARCH_REPLY   "[QUOTE] Results: "
#define QUOTE_COUNT_REPLY    "[QUOTE] %.*s quotes"
#define QUOTE_UNKNOWN_ERROR  "[QUOTE] Unknown error occurred!"

#define QUOTE_MYSQL_ERR      "[QUOTE] MySQL Error: %s.", GET_MYSQL_ERROR
#define GET_MYSQL_ERROR      mysql_error(mysql_obj)

/* Config options */
char *Q_MYSQL_HOST = NULL;
unsigned int Q_MYSQL_PORT = 0;
char *Q_MYSQL_SOCK = NULL;
char *Q_MYSQL_USER = NULL;
char *Q_MYSQL_PASS = NULL;
char *Q_MYSQL_DB = NULL;
int Q_ENABLE_MULTILINE = 0;

/* MySQL Variables */
MYSQL *mysql_obj = NULL;
MYSQL_RES *mysql_result_set = NULL;
my_ulonglong q_mysql_affected_rows;

/* Main module methods */
int load_config();
int reload_config(int ac, char **av);
int do_quote(int ac, char **av);
int do_quote_normal(int ac, char **av);

/* Methods for responding to !quote * */
void do_add_quote(ChannelInfo *ci, User *u, char *arg);
void do_del_quote(ChannelInfo *ci, User *u, char *arg);
void do_view_quote(ChannelInfo *ci, User *u, char *arg);
void do_search_quote(ChannelInfo *ci, User *u, char *arg);
void do_count_quote(ChannelInfo *ci, User *u);

/* Helper methods */
int do_mysql_connect();
unsigned int do_mysql_query(char *format, ...);
void do_mysql_close();
char *str(char *format, ...);
char *vstr(char *format, va_list args);
char *str_escape(MYSQL *sql, char *in_format, ...);
char *nth(int num);
char *str_month(int mon);
char *ampm(int hr);

/**
 * Called when module is loaded.
 **/
int AnopeInit(void)
{
  EvtHook *hook;

  moduleAddAuthor(AUTHOR);
  moduleAddVersion(VERSION);

  hook = createEventHook(EVENT_BOT_FANTASY, do_quote);
  if (moduleAddEventHook(hook) != MOD_ERR_OK)
  {
    alog("[\002bs_quotes\002] Can't hook to EVENT_BOT_FANTASY event");
    return MOD_STOP;
  }

  hook = createEventHook(EVENT_BOT_FANTASY_NO_ACCESS, do_quote_normal);
  if (moduleAddEventHook(hook) != MOD_ERR_OK)
  {
    alog("[\002bs_quotes\002] Can't hook to EVENT_BOT_FANTASY_NO_ACCESS event");
    return MOD_STOP;
  }

  hook = createEventHook(EVENT_RELOAD, reload_config);
  if (moduleAddEventHook(hook) != MOD_ERR_OK)
  {
    alog("[\002bs_quotes\002] Can't hook to EVENT_RELOAD event");
    return MOD_STOP;
  }

  /* Load config before creating table, any errors in config are dealt with in the load_config */
  if (load_config() == MOD_STOP) return MOD_STOP;

  /* Connect to MySQL server */
  if (do_mysql_connect())
  {
    alog("[\002bs_quotes\002] Can't connect to MySQL server");
    return MOD_STOP;
  }

  /* Create our tables */
  unsigned int ret = do_mysql_query("CREATE TABLE IF NOT EXISTS `quotes` ("
                                    " `id` int(10) unsigned NOT NULL auto_increment,"
                                    "  `user` text NOT NULL,"
                                    " `quote` text NOT NULL,"
                                    " `time` timestamp NOT NULL default CURRENT_TIMESTAMP,"
                                    " `views` int(11) NOT NULL default '0',"
                                    " PRIMARY KEY  (`id`)"
                                    " ) ENGINE=MyISAM  DEFAULT CHARSET=utf8;");

  /* Query error check */
  if (ret != 0)
  {
    alog("[\002bs_quotes\002] Module Init SQL Error: %s", GET_MYSQL_ERROR);
    return MOD_STOP;
  }

  alog("\002bs_quotes\002 loaded... [Author: \002%s\002] [Version: \002%s\002]", AUTHOR, VERSION);

  return MOD_CONT;
}

/**
 * Called when module is unloaded
 **/
void AnopeFini(void)
{
  if (Q_MYSQL_HOST) { free(Q_MYSQL_HOST); Q_MYSQL_HOST = NULL; }
  if (Q_MYSQL_SOCK) { free(Q_MYSQL_SOCK); Q_MYSQL_SOCK = NULL; }
  if (Q_MYSQL_USER) { free(Q_MYSQL_USER); Q_MYSQL_USER = NULL; }
  if (Q_MYSQL_PASS) { free(Q_MYSQL_PASS); Q_MYSQL_PASS = NULL; }
  if (Q_MYSQL_DB)   { free(Q_MYSQL_DB);   Q_MYSQL_DB = NULL;   }

  do_mysql_close();

  alog("[\002bs_quotes\002]: Module Unloaded");
}

/**
 * Load the configuration directives
 **/
int load_config(void)
{
  Directive confvalues[][1] = {
    {{ "QuoteMySQLHost",       { { PARAM_STRING, PARAM_RELOAD, &Q_MYSQL_HOST       } } }},
    {{ "QuoteMySQLPort",       { { PARAM_PORT,   PARAM_RELOAD, &Q_MYSQL_PORT       } } }},
    {{ "QuoteMySQLSocket",     { { PARAM_STRING, PARAM_RELOAD, &Q_MYSQL_SOCK       } } }},
    {{ "QuoteMySQLUser",       { { PARAM_STRING, PARAM_RELOAD, &Q_MYSQL_USER       } } }},
    {{ "QuoteMySQLPassword",   { { PARAM_STRING, PARAM_RELOAD, &Q_MYSQL_PASS       } } }},
    {{ "QuoteMySQLDatabase",   { { PARAM_STRING, PARAM_RELOAD, &Q_MYSQL_DB         } } }},
    {{ "QuoteEnableMultiLine", { { PARAM_SET,    PARAM_RELOAD, &Q_ENABLE_MULTILINE } } }}
  };

  /* free() any already set options */
  if (Q_MYSQL_HOST) { free(Q_MYSQL_HOST); Q_MYSQL_HOST = NULL; }
  Q_MYSQL_PORT = 3306;
  if (Q_MYSQL_SOCK) { free(Q_MYSQL_SOCK); Q_MYSQL_SOCK = NULL; }
  if (Q_MYSQL_USER) { free(Q_MYSQL_USER); Q_MYSQL_USER = NULL; }
  if (Q_MYSQL_PASS) { free(Q_MYSQL_PASS); Q_MYSQL_PASS = NULL; }
  if (Q_MYSQL_DB)   { free(Q_MYSQL_DB);   Q_MYSQL_DB = NULL;   }
  Q_ENABLE_MULTILINE = 0;

  /* Get each config option */
  unsigned int i = 0;
  for (i = 0; i < 7; i++)
  {
    moduleGetConfigDirective(confvalues[i]);
  }

  /* Check for valid options */

  /* We *need* user/pass/db */
  if (!Q_MYSQL_USER || !Q_MYSQL_PASS || !Q_MYSQL_DB)
  {
    alog("[\002bs_quotes\002]: Missing config options!");
    return MOD_STOP;
  }
  /* We need one of host or sock */
  if (!Q_MYSQL_HOST && !Q_MYSQL_SOCK)
  {
    alog("[\002bs_quotes\002]: Missing config options!");
    return MOD_STOP;
  }

  return MOD_CONT;
}

/**
 * Upon /os reload call the routines for reloading the configuration directives
 **/
int reload_config(int ac, char **av)
{
  if (ac >= 1)
  {
    if (!stricmp(av[0], EVENT_START))
    {
      alog("[\002bs_quotes\002]: Reloading configuration directives...");
      return load_config();
    }
  }
  return MOD_CONT;
}

/**
 * Handles all fantasy commands.
 * Here we ll identify the command and call the right routines.
 **/
int do_quote(int ac, char **av)
{
  User *u;
  ChannelInfo *ci;
  Channel *c;

  /* Some basic error checking... should never match */
  if (ac < 3) return MOD_CONT;

  if (!(ci = cs_findchan(av[2]))) return MOD_CONT;
  if (!(u = finduser(av[1])))     return MOD_CONT;
  if (!(c = findchan(ci->name)))  return MOD_CONT;

  /* Only check !quote commands */
  if (stricmp(av[0], "quote") != 0) return MOD_CONT;

  /* Only our messages from now on */

  /* Just !quote on it's own? */
  if (ac == 3)
  {
    anope_cmd_privmsg(ci->bi->nick, ci->name, QUOTE_SYNTAX);
    return MOD_CONT;
  }

  if ((strnicmp(av[3], "add ", 4)==0) && (strlen(av[3]) > 4))
  {
    if (check_access(u, ci, CA_AUTOHALFOP))
    {
      do_add_quote(ci, u, av[3] + 4);
    }
    else
    {
      notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
    }
  }
  else if ((strnicmp(av[3], "del ", 4)==0) && (strlen(av[3]) > 4))
  {
    if (check_access(u, ci, CA_AUTOHALFOP))
    {
      do_del_quote(ci, u, av[3] + 4);
    }
    else
    {
      notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
    }
  }
  else if (((strnicmp(av[3], "view ", 5)==0) && (strlen(av[3]) > 5)) || ((strnicmp(av[3], "get ", 4)==0) && (strlen(av[3]) > 4)))
  {
    int arg_skip = 4;
    if (strnicmp(av[3], "view", 4)==0) arg_skip++;
    do_view_quote(ci, u, av[3] + arg_skip);
  }
  else if (stricmp(av[3], "random")==0)
  {
    do_view_quote(ci, u, NULL);
  }
  else if ((strnicmp(av[3], "search ", 7)==0) && (strlen(av[3]) > 7))
  {
    do_search_quote(ci, u, av[3] + 7);
  }
  else if (stricmp(av[3], "count")==0)
  {
    do_count_quote(ci, u);
  }
  else
  {
    anope_cmd_privmsg(ci->bi->nick, ci->name, QUOTE_SYNTAX);
  }

  return MOD_CONT;
}

/**
 * Handles the !quote command for people not on the fantasy list
 **/
int do_quote_normal(int ac, char **av)
{
  User *u;
  ChannelInfo *ci;
  Channel *c;

  /* Some basic error checking... should never match */
  if (ac < 3) return MOD_CONT;

  if (!(ci = cs_findchan(av[2]))) return MOD_CONT;
  if (!(u = finduser(av[1])))     return MOD_CONT;
  if (!(c = findchan(ci->name)))  return MOD_CONT;

  /* Only check !quote commands */
  if (stricmp(av[0], "quote") != 0) return MOD_CONT;

  /* Only our messages from now on */

  /* Just !quote on it's own? */
  if (ac == 3)
  {
    anope_cmd_privmsg(ci->bi->nick, ci->name, QUOTE_SYNTAX);
    return MOD_CONT;
  }

  if ((strnicmp(av[3], "add ", 4)==0) && (strlen(av[3]) > 4))
  {
    notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
  }
  else if ((strnicmp(av[3], "del ", 4)==0) && (strlen(av[3]) > 4))
  {
    notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
  }
  else if (((strnicmp(av[3], "view ", 5)==0) && (strlen(av[3]) > 5)) || ((strnicmp(av[3], "get ", 4)==0) && (strlen(av[3]) > 4)))
  {
    int arg_skip = 4;
    if (strnicmp(av[3], "view", 4)==0) arg_skip++;
    do_view_quote(ci, u, av[3] + arg_skip);
  }
  else if (stricmp(av[3], "random")==0)
  {
    do_view_quote(ci, u, NULL);
  }
  else if ((strnicmp(av[3], "search ", 7)==0) && (strlen(av[3]) > 7))
  {
    do_search_quote(ci, u, av[3] + 7);
  }
  else if (stricmp(av[3], "count")==0)
  {
    do_count_quote(ci, u);
  }
  else
  {
    anope_cmd_privmsg(ci->bi->nick, ci->name, QUOTE_SYNTAX);
  }

  return MOD_CONT;
}

/**
 * Handles the !quote add command
 **/
void do_add_quote(ChannelInfo *ci, User *u, char *arg)
{
  /* Prepare arguments */
  char *user = str_escape(mysql_obj, "%s", u->nick);
  char *quote = str_escape(mysql_obj, "%s", arg);

  /* Do query */
  unsigned int ret = do_mysql_query("INSERT INTO `%s`.`quotes` ( `id` , `user` , `quote`, `time` , `views` ) VALUES ( NULL , '%s', '%s', CURRENT_TIMESTAMP, 0 );", Q_MYSQL_DB, user, quote);
  
  /* Free the arguments */
  free(user);
  free(quote);

  if (ret != 0)
  {
    /* Failed, dump error to irc */
    anope_cmd_privmsg(ci->bi->nick, ci->name, QUOTE_MYSQL_ERR);
  }
  else
  {
    /* Success, show new id */
    my_ulonglong new_id = mysql_insert_id(mysql_obj);
    anope_cmd_privmsg(ci->bi->nick, ci->name, QUOTE_SUCCESS_ADD, new_id);
  }
}

/**
 * Handles the !quote del command
 **/
void do_del_quote(ChannelInfo *ci, User *u, char *arg)
{
  if ((strlen(arg) > 1) && (*arg == '#')) arg++;

  my_ulonglong id_num;
  if (1 != sscanf(arg, "%llu", &id_num))
  {
    /* Bad ID */
    anope_cmd_privmsg(ci->bi->nick, ci->name, QUOTE_NO_QUOTE);
    return;
  }

  unsigned int ret = do_mysql_query("DELETE FROM `%s`.`quotes` WHERE id = %llu;", Q_MYSQL_DB, id_num);
  if (ret != 0)
  {
    anope_cmd_privmsg(ci->bi->nick, ci->name, QUOTE_MYSQL_ERR);
    return;
  }

  if (q_mysql_affected_rows > 0)
    anope_cmd_privmsg(ci->bi->nick, ci->name, QUOTE_SUCCESS_DEL, id_num);
  else
    anope_cmd_privmsg(ci->bi->nick, ci->name, QUOTE_NO_QUOTE);
}

/**
 * Handles the !quote view command
 **/
void do_view_quote(ChannelInfo *ci, User *u, char *arg)
{
  my_ulonglong id_num;
  /* if arg, then id was specified - find it */
  if (arg)
  {
    if ((strlen(arg) > 1) && (*arg == '#')) arg++;

    if (1 != sscanf(arg, "%llu", &id_num))
    {
      /* Bad ID */
      anope_cmd_privmsg(ci->bi->nick, ci->name, QUOTE_NO_QUOTE);
      return;
    }
  }

  unsigned int ret;
  /* arg != NULL, use id; else randomise */
  if (arg)
    ret = do_mysql_query("SELECT quote, user, time FROM `%s`.`quotes` WHERE id = %llu;", Q_MYSQL_DB, id_num);
  else
    ret = do_mysql_query("SELECT quote, user, time, id FROM `%s`.`quotes` ORDER BY RAND() LIMIT 1;", Q_MYSQL_DB);

  if (ret != 0)
  {
    anope_cmd_privmsg(ci->bi->nick, ci->name, QUOTE_MYSQL_ERR);
    return;
  }

  MYSQL_ROW row = mysql_fetch_row(mysql_result_set);
  if (row)
  {
    char *quote = NULL, *user = NULL, *date = NULL, *id_str = NULL;

    /* Get results */
    unsigned long *lengths = mysql_fetch_lengths(mysql_result_set);
    quote = str("%.*s", (int) lengths[0], row[0] ? row[0] : "");
    user  = str("%.*s", (int) lengths[1], row[1] ? row[1] : "");
    id_str = arg ? str("%llu", id_num) : str("%.*s", (int) lengths[3], row[3] ? row[3] : "");

    /* Format the date */          
    date  = str("%.*s", (int) lengths[2], row[2] ? row[2] : "");
    int date_parts[6];
    sscanf(date, "%d-%d-%d %d:%d:%d", &date_parts[0], &date_parts[1], &date_parts[2], &date_parts[3], &date_parts[4], &date_parts[5]); 
    free(date);
    date = str("%d%s of %s %d at %d:%s%d%s", date_parts[2], nth(date_parts[2]), str_month(date_parts[1]), date_parts[0], date_parts[3], 
               date_parts[4] < 10 ? "0" : "", date_parts[4], ampm(date_parts[3]));

    if (Q_ENABLE_MULTILINE == 1)
    {
      /* Find each !n! and send the parts as separate lines */
      char *quote_mk1 = quote;
      while (*quote_mk1 != 0)
      {
        char *quote_mk2 = quote_mk1;
  
        /* Find the next !n! */
        while ((*quote_mk2 != 0) && (strncmp(quote_mk2, "!n!", 3) != 0)) quote_mk2++;
    
        /* Create the line */
        char *line = (char*)malloc(quote_mk2 - quote_mk1 + 1);
        memset(line, 0, quote_mk2 - quote_mk1 + 1);
        strncpy(line, quote_mk1, quote_mk2 - quote_mk1);

        /* Send the line */
        anope_cmd_privmsg(ci->bi->nick, ci->name, QUOTE_TEMPLATE, id_str, line);

        /* Free the malloc'd stuff */
        free(line);

        /* Move pointer on */
        quote_mk1 = quote_mk2 + (*quote_mk2 != 0 ? 3 : 0);
      }
    }
    else
    {
      /* Send the line without multi-line support*/
      anope_cmd_privmsg(ci->bi->nick, ci->name, QUOTE_TEMPLATE, id_str, quote);
    }

    /* Send quote by */
    anope_cmd_privmsg(ci->bi->nick, ci->name, QUOTE_TEMPLATE_BY, user, date);
  
    /* Free the quote parts */
    free(id_str);
    free(quote);
    free(user);
    free(date);

    do_mysql_query("UPDATE `%s`.`quotes` SET views = views + 1 WHERE id = %llu;", Q_MYSQL_DB, id_num);
  }
  else
  {
    anope_cmd_privmsg(ci->bi->nick, ci->name, QUOTE_NO_QUOTE);
  }
}

/**
 * Handles the !quote search command
 **/
void do_search_quote(ChannelInfo *ci, User *u, char *arg)
{
  /* Build query */
  char *search_string = str_escape(mysql_obj, "%s", arg);
  unsigned int ret = do_mysql_query("SELECT id FROM `%s`.`quotes` WHERE quote LIKE '%%%s%%'", Q_MYSQL_DB, search_string);
  free(search_string);

  if (ret != 0)
  {
    anope_cmd_privmsg(ci->bi->nick, ci->name, QUOTE_MYSQL_ERR);
    return;
  }

  MYSQL_ROW row;
  char *buffer = str(QUOTE_SEARCH_REPLY);
  my_ulonglong found_results = 0;
  my_ulonglong results = mysql_num_rows(mysql_result_set);
  while ((row = mysql_fetch_row(mysql_result_set)) != NULL)
  {
    found_results++;

    unsigned long *length = mysql_fetch_lengths(mysql_result_set);
    buffer = (char*)realloc(buffer, strlen(buffer) + ((int) length[0]) + 3);

    strncat(buffer, row[0], (int) length[0]);
    if (found_results != results) strcat(buffer, ", ");
  }
  
  if (found_results == 0)
    anope_cmd_privmsg(ci->bi->nick, ci->name, QUOTE_NO_QUOTE);
  else
    anope_cmd_privmsg(ci->bi->nick, ci->name, buffer);

  /* Moved out of else{} above, otherwise at found_results == 0 => mem leak */
  free(buffer);
}

/**
 * Handles !quote count
 **/
void do_count_quote(ChannelInfo *ci, User *u)
{
  /* Run query */
  unsigned int ret = do_mysql_query("SELECT COUNT(id) AS ids FROM `%s`.`quotes`;", Q_MYSQL_DB);

  if (ret != 0)
  {
    anope_cmd_privmsg(ci->bi->nick, ci->name, QUOTE_MYSQL_ERR);
    return;
  }

  MYSQL_ROW row = mysql_fetch_row(mysql_result_set);
  if (row)
  {
    unsigned long *lengths = mysql_fetch_lengths(mysql_result_set);
    anope_cmd_privmsg(ci->bi->nick, ci->name, QUOTE_COUNT_REPLY, (int)lengths[0], row[0]);
  }
  else
  {
    anope_cmd_privmsg(ci->bi->nick, ci->name, QUOTE_UNKNOWN_ERROR);
  }
}

/**
 * Method for connecting to mysql
 **/
int do_mysql_connect()
{
  if (mysql_obj) return 0;

  mysql_obj = mysql_init(NULL);
  if (!mysql_real_connect(mysql_obj, Q_MYSQL_HOST, Q_MYSQL_USER, Q_MYSQL_PASS, Q_MYSQL_DB, Q_MYSQL_PORT, Q_MYSQL_SOCK, CLIENT_IGNORE_SPACE))
  {
    mysql_close(mysql_obj);
    mysql_obj = NULL;
    return 1;
  }
  else
  {
    my_bool my_true = true;
    mysql_options(mysql_obj, MYSQL_OPT_RECONNECT, &my_true);
    return 0;
  }
}

/**
 * Method for running mysql queries 
 **/
unsigned int do_mysql_query(char *format, ...)
{
  va_list args;
  va_start(args,format);
  char *query = vstr(format, args);
  va_end(args);

  if (mysql_result_set)
  {
    mysql_free_result(mysql_result_set);
    mysql_result_set = NULL;
  }

  /* Run query */
  if (mysql_query(mysql_obj, query))
  {
    free(query);
    return 1;
  }
  
  /* free() query */
  free(query);

  /* Fill in info */
  mysql_result_set = mysql_store_result(mysql_obj);
  q_mysql_affected_rows = mysql_affected_rows(mysql_obj);

  return 0;
}

/**
 * Method to close connection
 **/
void do_mysql_close()
{
  if (mysql_result_set)
  {
    mysql_free_result(mysql_result_set);
    mysql_result_set = NULL;
  }
  mysql_close(mysql_obj);
  mysql_obj = NULL;
}

/**
 * Method to produce a malloc()'d char*, similar to printf()
 **/
char *str(char *format, ...)
{
  va_list args;
  va_start(args,format);
  char *ret = vstr(format, args);
  va_end(args);
  return ret;
}

/**
 * The va_list method for str() above
 **/
char *vstr(char *format, va_list args)
{
  char *buffer = NULL;
  int va_size = 256;
  /* Parse the format and generate a proper string output */
  do
  {
    va_size *= 2;
    buffer = (char*)realloc(buffer, va_size);
    memset(buffer, 0, va_size);
  } while (vsnprintf(buffer, va_size, format, args) < 0);

  /* Trim any white space at the end */
  char *output = (char*)malloc(strlen(buffer) + 1);
  strcpy(output, buffer);
  free(buffer);
  return output;
}

/**
 * Similar to str(), but returns a result safe for use in SQL queries
 **/
char *str_escape(MYSQL *sql, char *in_format, ...)
{
  va_list args;
  va_start(args,in_format);
  char *in = vstr(in_format, args);
  va_end(args);

  char *safe_in = (char*)malloc(strlen(in)*2 + 1);
  mysql_real_escape_string(sql, safe_in, in, strlen(in));

  free(in);

  return safe_in;  
}

/**
 * Simple method to add the 'nth' suffix for the given number
 **/
char *nth(int num)
{
  if (num == 11 || num == 12 || num == 13) return "th";

  switch (num % 10)
  {
    case 1: return "st";
    case 2: return "nd";
    case 3: return "rd";
    default: return "th";
  }
}

/**
 * Method to return the short month name for the month number (1-12)
 **/
char *str_month(int mon)
{
  switch(mon)
  {
    case 1: return "Jan";
    case 2: return "Feb";
    case 3: return "Mar";
    case 4: return "Apr";
    case 5: return "May";
    case 6: return "June";
    case 7: return "July";
    case 8: return "Aug";
    case 9: return "Sept";
    case 10: return "Oct";
    case 11: return "Nov";
    case 12: return "Dec";
    default: return "???";
  }
}

/**
 * Method to return whether am or pm for the given hour
 **/
char *ampm(int hr)
{
  if (hr < 12) return "am";
  else return "pm";
}
