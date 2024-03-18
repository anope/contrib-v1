#include "module.h"
#include "encrypt.h"
#include "misc_sqlcmd_errors.h"

#define AUTHOR "heinz (http://modules.anope.org/profile.php?id=7)"
#define VERSION "2.0"
#define MODNAME "misc_sqlcmd"

/* Struct Declarations and Typedefs */
typedef struct _sqlcmd_status SQLCmdStatus;
struct _sqlcmd_status {
        int status;
        char *msg;
};

typedef struct _sqlcmd_command SQLCmd;
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

/* Global Variables - misc_sqlcmd_main.c */
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
E int sqlcmd_conf_reload(int argc, char **argv);
E int sqlcmd_conf_load();
E int sqlcmd_handle(int ac, char **av);
E void sqlcmd_str_to_params(char *params, int ac, char **param_array);
E char *sqlcmd_checksum(char *cmd, char *param_str, char *ts);
E int sqlcmd_checkchanstatus(char *nick, char *nickpass, char *channel, char *chanpass, int minlvl);

E int sqlcmd_handle_nickreg(int ac, char **av);
E int sqlcmd_handle_nickconf(int ac, char **av);
E int sqlcmd_handle_nickgroup(int ac, char **av);
E int sqlcmd_handle_nickdrop(int ac, char **av);
E NickRequest *makerequest(const char *nick);
E NickAlias *makenick(const char *nick);
E NickAlias *makealias(const char *nick, NickCore * nc);

E int sqlcmd_handle_botassign(int ac, char **av);
E int sqlcmd_handle_botsay(int ac, char **av);
E int sqlcmd_handle_botact(int ac, char **av);
E int sqlcmd_handle_botunassign(int ac, char **av);
E void sqlcmd_bot_unassign(ChannelInfo *ci, char *user);

E int sqlcmd_handle_chanreg(int ac, char **av);
E int sqlcmd_handle_chanaddsop(int ac, char **av);
E int sqlcmd_handle_chanaddaop(int ac, char **av);
E int sqlcmd_handle_chanaddhop(int ac, char **av);
E int sqlcmd_handle_chanaddvop(int ac, char **av);
E int sqlcmd_handle_chandelsop(int ac, char **av);
E int sqlcmd_handle_chandelaop(int ac, char **av);
E int sqlcmd_handle_chandelhop(int ac, char **av);
E int sqlcmd_handle_chandelvop(int ac, char **av);
E int sqlcmd_handle_chanaccess(int ac, char **av);
E int sqlcmd_handle_chantopic(int ac, char **av);
E int sqlcmd_handle_chandrop(int ac, char **av);
E int sqlcmd_do_xop_add(char *nick, int xop_lvl, char *xop_nick, ChannelInfo *ci);
E int sqlcmd_do_xop_del(char *nick, int xop_lvl, char *xop_nick, ChannelInfo *ci);

E int sqlcmd_handle_memosend(int ac, char **av);
E int sqlcmd_handle_memodel(int ac, char **av);
E int sqlcmd_handle_memoclear(int ac, char **av);

E int sqlcmd_handle_ping(int ac, char **av);

E int sqlcmd_mysql_connect();
E int sqlcmd_mysql_query(char *sql, int store_result); 

E int sqlcmd_create(char *name, int params, int (*handle)(int ac, char **av));
E int sqlcmd_status_update(int id, int status);
E SQLCmd *sqlcmd_find(char *name);
E int sqlcmd_destroy(char *name);
