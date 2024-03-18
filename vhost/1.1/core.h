#ifndef CORE_H
#define CORE_H

//  global configuration variables
//  these may be modified with care

#define SUPPORT_WILDCARD    /* comment this if you don't want wildcard support in nick matching. */
#define SUPPORT_REGEX       /* comment this if you don't want regex support in nick matching. */

//  do NOT modify anything below here
//  unless you know what you are doing!

#define AUTHOR      "hmtX^mokkori"
#define VERSION     "$Id: Release 1.1 2010-03-09 $"
#define NAME        "vhost"

#include "module.h"
#include "conf.h"
#include "drop.h"
#include "group.h"
#include "language.h"
#include <regex.h>

#define HSREQ_DBNUM             3
#define HSREQ_DEFAULT_DBNAME    "hs_request.db"
#define HSREQ_REGEX_DBNAME      "hs_request.regex.db"
#define HSREQ_EXCEPTION_DBNAME  "hs_request.exempt.db"

#ifdef HAVE_MEMPCPY
void *mempcpy(void *dest, const void *src, size_t n);
#endif

#define BadPtr(x) (!(x) || (*(x) == '\0'))

typedef struct regex_ Regex;
typedef struct hooks_t_ Hooks_t;
typedef struct ctext_t_ cText_t;
typedef struct vhost_t_ vHost_t;
typedef struct exception_t_ Exception_t;

struct regex_ {
    regex_t expr;
    char *str;
    char *nick;
    char *reason;
    Regex *next;
};

struct hooks_t_
{
    char *src;
    int (*func)(int argc, char **argv);
};

struct ctext_t_
{
    char *_new;
    char *_updated;
    char *_activated_ext;
    char *_rejected_ext;
    char *_activated;
    char *_rejected;
};

struct vhost_t_
{
    char *nick;
    char *real;

    char *chan;
    char *chanmodes;

    uint32 flags;
#       define MEMOUSER        0x00000001
#       define MEMOOPER        0x00000002
#       define MEMOSETTERS     0x00000004
#       define DISPLAYMODE     0x00000008
#       define DISPLAYCOLOR    0x00000010
#       define DENYIDENTUPDATE 0x00000020
  
    int displaymax;
    int timer;
    int requestdelay;
  
    char *dbname;
    char *regexdbname;
    char *exceptiondbname;
};

struct exception_t_ {
    char *who;
    char *by;
    NickAlias *na;
    Exception_t *next;
};

#include "regex_db.h"
#include "exempt_db.h"
#include "extern.inc.h"

//  copy from datafiles.c until anopes fixes 'void ModuleDatabaseBackup(char *dbname)'
void my__rename_database(char *name, char *ext);

char *my__decodespace(char *s);
int my__join_channel(char *chan);
int my__add_client(void);
void my__del_client(void);
void my__show_entry(User *u, HostCore *ptr, boolean dur, boolean ischan);
void my__show_extended(User *u, char *chan, char *nick, char *vident, char *vhost, char *reason);
void my__show_list(User *u, char *nick, char *title, boolean ischan, boolean call_from_timer);
void SetFlag(uint32 *src, uint32 flag);
void SetFlag_bool(uint32 *src, uint32 flag, boolean val);
void SetFlag_int(uint32 *src, uint32 flag, int val);
void UnsetFlag(uint32 *src, uint32 flag);
const int GetFlag(uint32 *src, uint32 flag);
const boolean HasFlag(uint32 *src, uint32 flag);
void my__load_config(void);
void my__announce(char *target, const char *fmt, ...);
void my__announce_lang(User *u, char *target, int message, ...);
void my__add_host_request(char *nick, char *vident, char *vhost, char *creator, int32 request_time);
void my__activate_host_request(User *u, char *nick, char *vident, char *vhost, char *creator, int32 tmp_time, char *reason);
void my__send_memo(User *u, char *name, int z, int number, ...);
void my__send_staff_memo(User *u, char *vhost);
void my__save_db(void);
void my__load_db(void);
void my__core(User *u, int ac, char *buf[]);
int my__privmsg(char *source, int ac, char **av);
int my__kick(char *source, int ac, char **av);
int my__hook_db_saving(int argc, char **argv);
int my__hook_config_reload(int argc, char **argv);
int my__hook_db_backup(int argc, char **argv);
int my__vhost_waiting_timer(int argc, char **argv);
int my__vhost_cleanup_timer(int argc, char **argv);
boolean my__ismine(char *what);
boolean my__del_host_request(char *nick);

HostCore *request_head;
Regex *regex_head;
Exception_t *exception_head;

#endif

/* EOF */
