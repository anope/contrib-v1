#include "module.h"
#define AUTHOR "heinz"
#define VERSION "1.21"

  /* -----------------------------------------------------------
   * Name  : ns_restrictreg
   * Author: heinz <heinz@anope.org>
   * Date  : 24/09/2006
   * -----------------------------------------------------------
   * Tested: Unreal 3.2
   * -----------------------------------------------------------
   * Module Info & Changelog
   * -----------------------
   * Module checks for valid nickname/password in an sql database
   * If the requested nick/pass isn't found in the database, reg-
   * istration will not be permitted.
   *
   * For Instructions, Updates and Contact Info please visit:
   * http://modules.anope.org/viewmod.php?id=1
   *
   * Please configure this module before use.
   *
   * v1.21 - Changed URL for support, some minor code cleanup.
   * v1.20 - Fixed a bug whereby if an email wasn't specified, the check could
   *             be bypassed (Thanks to Vincent)
   * v1.10 - Added support for MD5 Passwords. Please read the comment for details.
   * v1.02 - More fixes. Completely (?) stable.
   * v1.01 - Fix SQL Injection Bug. (Cheers Ribosome :))
   * v1.00 - Stable Release. Added custom table and user/pass fields.
   * v0.11 - Bug fixes. Code Cleanup. Working State.
   * v0.10 - Initial Version
   * -----------------------------------------------------------
   */

/* ---------------------------------------------------------------------- */
/* START OF CONFIGURATION BLOCK - please read the comments :)             */
/* ---------------------------------------------------------------------- */

/* NS_RESTRICTREG_DB - MySQL Database Name (eg: forums) */
#define NS_RESTRICTREG_DB "forums"

/* NS_RESTRICTREG_USER - MySQL Username (eg: anopeuser) */
#define NS_RESTRICTREG_USER "anopeuser"

/* NS_RESTRICTREG_PASS - MySQL Password (eg: anopepass) */
#define NS_RESTRICTREG_PASS "anopepass"

/* NS_RESTRICTREG_HOST - MySQL Server Address (eg: localhost) */
#define NS_RESTRICTREG_HOST "localhost"

/* NS_RESTRICTREG_USERTABLE - User table (eg: users) */
/* This table contains the data from say forums, or a web portal. Users in this table will be allowed to register. Anyone else will be denied. */
#define NS_RESTRICTREG_USERTABLE "users"

/* NS_RESTRICTREG_USERFIELD - Username Column in NS_RESTRICTREG_USERTABLE (eg: username) */
#define NS_RESTRICTREG_USERFIELD "username"

/* NS_RESTRICTREG_PASSFIELD - Password Column in NS_RESTRICTREG_USERTABLE (eg: password) */
#define NS_RESTRICTREG_PASSFIELD "password"

/* NS_RESTRICTREG_ISMD5 - If the SQL password in NS_RESTRICTREG_USERTABLE is MD5 */
/* If the passwords ARE MD5, please put 1, otherwise, put 0 */
#define NS_RESTRICTREG_ISMD5 1

/* NS_RESTRICTREG_MSG* - Message to send a user when they have been denied registration. */
/* You may delete any lines you do not wish to use here. Maximum lines currently is 3 */
#define NS_RESTRICTREG_MSG1 "Sorry, but you must be a member of [website] to register."
#define NS_RESTRICTREG_MSG2 "You can register for [website] by visiting [url] and signing up to the forums."
#define NS_RESTRICTREG_MSG3 "Your nickname registration has therefore been halted."


/* ------------------------------------------------------------------------ */
/* End of Configuration Section - Please don't edit anything below here     */
/* ------------------------------------------------------------------------ */

MYSQL *rrmysql;                   /* MySQL Handler */
MYSQL_RES *rr_mysql_res;           /* MySQL Result  */
int ns_restrictreg(User *u);

int AnopeInit(int argc, char **argv)
{
 Command *c;
 c = createCommand("REGISTER", ns_restrictreg, NULL, -1, -1, -1, -1, -1);
 moduleAddCommand(NICKSERV, c, MOD_HEAD);
 moduleAddAuthor(AUTHOR);
 moduleAddVersion(VERSION);
 alog("[NS_RESTRICTREG] Module Loaded Successfully.");
 return MOD_CONT;
}

void ANopeFini(void) {
 alog("[NS_RESTRICTREG] Module Unloaded Successfully!");
}

int ns_restrictreg(User *u) {
 int result;
 char sql[MAX_SQL_BUF];
 char *tmp;
 char *pass;
 char dilim = ' ';
 int slen;
 char *quoted;

 tmp = moduleGetLastBuffer();
 if (tmp == NULL) {
  return MOD_STOP;
 }
 rrmysql = mysql_init(NULL);
 if (rrmysql == NULL) {
  alog("[NS_RESTRICTREG] Error - Cannot create MySQL Object.");
  return MOD_STOP;
 }
 if ((!mysql_real_connect(rrmysql, NS_RESTRICTREG_HOST, NS_RESTRICTREG_USER, NS_RESTRICTREG_PASS, NS_RESTRICTREG_DB, 3306, NULL, 0))) {
  alog("[NS_RESTRICTREG] Error - Cannot connect to MySQL Server - %s", mysql_error(rrmysql));
  return MOD_STOP;
 }
 pass = myStrGetOnlyToken(tmp, dilim, 0);
 if (pass == NULL) {
     return MOD_CONT;
 }
 slen = strlen(pass);
 quoted = malloc((1 + (slen * 2)) * sizeof(char));
 mysql_real_escape_string(rrmysql, quoted, pass, slen);
 if (NS_RESTRICTREG_ISMD5 == 1)
  snprintf(sql, MAX_SQL_BUF, "SELECT %s from %s where %s = '%s' and %s = MD5('%s')", NS_RESTRICTREG_USERFIELD, NS_RESTRICTREG_USERTABLE, NS_RESTRICTREG_USERFIELD, u->nick, NS_RESTRICTREG_PASSFIELD, quoted);
 else
  snprintf(sql, MAX_SQL_BUF, "SELECT %s from %s where %s = '%s' and %s = '%s'", NS_RESTRICTREG_USERFIELD, NS_RESTRICTREG_USERTABLE, NS_RESTRICTREG_USERFIELD, u->nick, NS_RESTRICTREG_PASSFIELD, quoted);
 result = mysql_query(rrmysql, sql);
 if (result) {
  alog("[NS_RESTRICTREG] MySQL Error performing query. Error: %s.", mysql_error(rrmysql));
  return MOD_STOP;
 }
 rr_mysql_res = mysql_store_result(rrmysql);
 if (mysql_num_rows(rr_mysql_res) == 0) {
  #ifdef NS_RESTRICTREG_MSG1
  notice(s_NickServ, u->nick, NS_RESTRICTREG_MSG1);
  #endif
  #ifdef NS_RESTRICTREG_MSG2
  notice(s_NickServ, u->nick, NS_RESTRICTREG_MSG2);
  #endif
  #ifdef NS_RESTRICTREG_MSG3
  notice(s_NickServ, u->nick, NS_RESTRICTREG_MSG3);
  #endif
  mysql_free_result(rr_mysql_res);
  mysql_close(rrmysql);
  return MOD_STOP;
 }
 mysql_free_result(rr_mysql_res);
 mysql_close(rrmysql);
 return MOD_CONT;
}
