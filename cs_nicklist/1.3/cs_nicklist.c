#include "module.h"
#include <mysql.h>

#define AUTHOR "Tzann"
#define VERSION "1.3"

// SQL
#define GET_MYSQL_ERROR      mysql_error(mysql_obj)
MYSQL *mysql_obj = NULL;
MYSQL_RES *res = NULL;
my_ulonglong q_mysql_affected_rows;

char *NICK_SQL_HOST = NULL;
unsigned int NICK_SQL_PORT = 0;
char *NICK_SQL_USER = NULL;
char *NICK_SQL_PASS = NULL;
char *NICK_SQL_DB = NULL;

// intern struct
struct c_userlist *cu, *next;
unsigned int i;

// Events
int reload_config(int ac, char **av);
int channels_update(int ac, char **av);
int user_quit(int ac, char **av);

// Methods
int load_config();
int channel_update(char *chan);

// SQL
int do_mysql_connect();
int do_mysql_query(char *q);
void do_mysql_close();

//Helper
char *lower(char *str);
char *esc_str(char *str);

int AnopeInit(void)
{
	EvtHook *hook;
	unsigned int ret;

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	hook = createEventHook(EVENT_JOIN_CHANNEL, channels_update);
	if (moduleAddEventHook(hook) != MOD_ERR_OK)
		return MOD_STOP;

	hook = createEventHook(EVENT_PART_CHANNEL, channels_update);
	if (moduleAddEventHook(hook) != MOD_ERR_OK)
		return MOD_STOP;

	hook = createEventHook(EVENT_CHANGE_NICK, channels_update);
	if (moduleAddEventHook(hook) != MOD_ERR_OK)
		return MOD_STOP;

	hook = createEventHook(EVENT_USER_LOGOFF, user_quit);
	if (moduleAddEventHook(hook) != MOD_ERR_OK)
		return MOD_STOP;

	hook = createEventHook(EVENT_CHAN_DROP, channels_update);
	if (moduleAddEventHook(hook) != MOD_ERR_OK)
		return MOD_STOP;

	hook = createEventHook(EVENT_CHAN_EXPIRE, channels_update);
	if (moduleAddEventHook(hook) != MOD_ERR_OK)
		return MOD_STOP;

	hook = createEventHook(EVENT_NICK_LOGOUT, channels_update);
	if (moduleAddEventHook(hook) != MOD_ERR_OK)
		return MOD_STOP;

	hook = createEventHook(EVENT_RELOAD, reload_config);
	if (moduleAddEventHook(hook) != MOD_ERR_OK)
		return MOD_STOP;

	if (load_config() == MOD_STOP) return MOD_STOP;

	if (do_mysql_connect() == MOD_STOP)
	{
		alog("[\002cs_nicklist\002] Can't connect to MySQL server");
		return MOD_STOP;
	}

	ret = do_mysql_query("CREATE TABLE IF NOT EXISTS channels"
		"(chan char(50),"
		"user char(50))");

	if(ret == MOD_STOP)
		return MOD_STOP;

	alog("[\002cs_nicklist\002]: loaded . . . [Author: \002%s\002] [Version: \002%s\002]", AUTHOR, VERSION);

	return MOD_CONT;
}

int load_config(void)
{
	Directive confvalues[][1] = {
		{{ "NickMysqlHost",       { { PARAM_STRING, PARAM_RELOAD, &NICK_SQL_HOST       } } }},
		{{ "NickMysqlPort",       { { PARAM_PORT,   PARAM_RELOAD, &NICK_SQL_PORT       } } }},
		{{ "NickMysqlUser",       { { PARAM_STRING, PARAM_RELOAD, &NICK_SQL_USER       } } }},
		{{ "NickMysqlPass",		  { { PARAM_STRING, PARAM_RELOAD, &NICK_SQL_PASS       } } }},
		{{ "NickMysqlName",		  { { PARAM_STRING, PARAM_RELOAD, &NICK_SQL_DB         } } }}
	};

	NICK_SQL_PORT = 3306;
	if (NICK_SQL_HOST) { free(NICK_SQL_HOST); NICK_SQL_HOST = NULL; }
	if (NICK_SQL_USER) { free(NICK_SQL_USER); NICK_SQL_USER = NULL; }
	if (NICK_SQL_PASS) { free(NICK_SQL_PASS); NICK_SQL_PASS = NULL; }
	if (NICK_SQL_DB)   { free(NICK_SQL_DB);   NICK_SQL_DB = NULL;   }

	i = 0;
	for (i = 0; i < 5; i++)
	{
		moduleGetConfigDirective(confvalues[i]);
	}

	return MOD_CONT;
}

int reload_config(int ac, char **av)
{
  if (ac >= 1)
  {
    if (!stricmp(av[0], EVENT_START))
    {
      alog("[\002cs_nicklist\002]: Reloading configuration directives...");
      return load_config();
    }
  }
  return MOD_CONT;
}

void AnopeFini(void)
{
	if (NICK_SQL_HOST) { free(NICK_SQL_HOST); NICK_SQL_HOST = NULL; }
	if (NICK_SQL_USER) { free(NICK_SQL_USER); NICK_SQL_USER = NULL; }
	if (NICK_SQL_PASS) { free(NICK_SQL_PASS); NICK_SQL_PASS = NULL; }
	if (NICK_SQL_DB)   { free(NICK_SQL_DB);   NICK_SQL_DB = NULL;   }

	do_mysql_close();

	alog("[\002cs_nicklist\002]: Modul unloaded");
}

int channels_update(int ac, char **av)
{
	FILE *file;

	Channel *c;
	ChannelInfo *ci;

    int carryon = 0;
    int i;

	carryon = 0;
	do_mysql_query("DELETE FROM channels;");
	for(i = 0; i < 1024; i++)
	{
		for(c = chanlist[i]; c; c = c->next)
		{
			channel_update(c->name);
		}
	}

	return MOD_CONT;
}

int user_quit(int ac, char **av)
{
	char query[200];
	snprintf(query, 200, "DELETE FROM channels WHERE (user=\"%s\");",esc_str(av[0]));
	do_mysql_query(query);

	return 1;
}

int channel_update(char *chan)
{
	Channel *c;
	char query[200];

	c = findchan(chan);
	cu = c->users;
	while (cu) 
	{
		next = cu->next;

		snprintf(query, 200, "INSERT INTO channels(chan,user) VALUES (\"%s\",\"%s\");",esc_str(lower(c->name)),esc_str(cu->user->nick));
		do_mysql_query(query);

		cu = next;
	}

	return 1;
}

int do_mysql_connect()
{
	mysql_obj = mysql_init(NULL);

	if(mysql_real_connect(mysql_obj, NICK_SQL_HOST, NICK_SQL_USER, NICK_SQL_PASS, NICK_SQL_DB, 3306, NULL, 0) == NULL)
	{
		alog("[\002cs_nicklist\002]: mysql_real_connect FAILED!");
		return MOD_STOP;
	}
	
	return MOD_CONT;
}

int do_mysql_query(char *q)
{
	char log[300];
	unsigned querysize = 0;

	querysize = (unsigned) strlen(q);

	//snprintf(log, 300, "Query: \"%s\" (%i)",q,querysize);
	//alog(log);

	if (mysql_real_query(mysql_obj, q, querysize)) {
		alog("[\002cs_nicklist\002]: mysql_real_query FAILED!");
		return MOD_STOP;
	}
	if(res != NULL) 
	{
		mysql_free_result(res);
		res = NULL;
	}
	
	res = mysql_store_result(mysql_obj);

	return MOD_CONT;
}

void do_mysql_close()
{
	if (res)
	{
		mysql_free_result(res);
		res = NULL;
	}
	mysql_close(mysql_obj);
}

char *lower(char *str)
{
	char *toRet = str;
	for(i = 0; i < strlen(toRet); i++) 
		toRet[i] = tolower(toRet[i]);
	return toRet;
}

char *esc_str(char *str)
{
	char *toRet = (char *) malloc(strlen(str) * 2 + 1);
	mysql_real_escape_string(mysql_obj, toRet, str, strlen(str));
	return toRet;
}