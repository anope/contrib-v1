#include "module.h"
#define AUTHOR "heinz (http://modules.anope.org/profile.php?id=7)"
#define VERSION "2.0"
#define MODNAME "ns_restrictreg"

 /**************************************************************
 *  ns_restrictreg - Restrict NickServ registrations to users  *
 *                   in an existing MySQL database.            *
 *                                                             *
 *         Version 2.0 - Released: 27th September 2006         *
 **************************************************************/
 
 /**************************************************************
 * Module Info & Changelog      
 * -----------------------                                     
 * Module checks for valid nickname/password in a MySQL        
 * database. If the requested nick/pass isn't found in the     
 * database, registration will not be permitted.
 *
 * For Instructions, Updates and Contact Info please visit:
 * http://modules.anope.org/viewmod.php?id=1
 *
 * Please configure this module before use.
 *
 * v2.00 - Re-wrote module to use Module Directives.
 *         Module now compiles under Microsoft Windows.
 *         Can also restrict nickname grouping if enabled.
 *         Now supports SHA1 hashed passwords in database.
 *         Cleaned up the code to make more readable.
 * v1.21 - Changed URL for support, some minor code cleanup.
 * v1.20 - Fixed a bug whereby if an email wasn't specified,
 *          the check could be bypassed (Thanks to Vincent)
 * v1.10 - Added support for MD5 Passwords.
 *          Please read the comment for details.
 * v1.02 - More fixes. Completely (?) stable.
 * v1.01 - Fix SQL Injection Bug. (Cheers Ribosome :))
 * v1.00 - Stable Release.
 *          Added custom table and user/pass fields.
 * v0.11 - Bug fixes. Code Cleanup. Working State.
 * v0.10 - Initial Version
 **************************************************************/
 
 /**************************************************************
 * Configuration Information
 * -------------------------
 * This module requires the following configuration values to
 * be added to services.conf before the module will load.
 *
 * The values are listed below along with an explaination of
 * what each one does.
 *
 * - RestrictRegMySQLHost "localhost"
 *   Sets the hostname of the remote MySQL database
 *
 * - RestrictRegMySQLUser "mysqluser"
 *   Sets the MySQL Username to use when connecting
 *
 * - RestrictRegMySQLPass "mysqlpass"
 *   Sets the MySQL Password to use when connecting
 *
 * - RestrictRegMySQLDBName "my_website_db"
 *   Sets the MySQL Database to use when checking registrations
 *
 * - RestrictRegMySQLTable "my_user_table"
 *   Sets the name of the table which contains the user entries
 *
 * - RestrictRegMySQLUserCol "username"
 *   Sets the column name in the table which contains the 
 *   username
 *
 * - RestrictRegMySQLPassCol "password"
 *   Sets the column name in the table which contains the
 *   password
 *
 * - RestrictRegMySQLEnc "[option]"
 *   Sets the type of hash/encryption used on the password
 *   field in the database.
 *
 *   ns_restrictreg supports the following [option]'s:
 *      - "md5"
 *      - "sha1"
 *      - "none"
 *
 *   If the password column has no hash/encryption, set
 *   option to "".
 *
 * - RestrictRegMessage1 "Message goes here"
 *   This will be the first line of the message sent to users
 *   when their nickname registration is denied.
 *
 * - RestrictRegMessage2 "Message goes here"
 *   This will be the second line of the message sent to users
 *   when their nickname registration is denied. Set to "" to
 *   have no second line.
 *
 * - RestrictRegMessage3 "Message goes here"
 *   This will be the third line of the message sent to users
 *   when their nickname registration is denied. Set to "" to
 *   have no third line.
 *
 * - RestrictRegAllowGroup [option]
 *   Sets restrictions on the use of the /nickserv group cmd.
 *
 *   Possible options are:
 *      - 1 - Disallow all /nickserv group requests
 *      - 2 - Validate new nickname against the database
 *      - 3 - Allow all group requests - no restrictions
 *
 * - RestrictRegGroupDisabledMsg "Message goes here"
 *   Sets the message to be displayed when /nickserv group
 *   requests are disabled. If you have set the above option
 *   to 1 or 2, you'll still need to define this option in
 *   services.conf to make the module work.
 *
 **************************************************************/
 
 /**************************************************************
 *                                                             *
 * PLEASE DO NOT EDIT ANYTHING BELOW HERE - MODULE CODE BEGINS *
 *                                                             *
 **************************************************************/
 
/* Global Variables */
MYSQL *ns_restrictreg_mysql;
MYSQL_RES *ns_restrictreg_mysql_res;

/* Configuration Values */
char *RestrictRegMySQLHost = NULL;
char *RestrictRegMySQLUser = NULL;
char *RestrictRegMySQLPass = NULL;
char *RestrictRegMySQLDBName = NULL;
char *RestrictRegMySQLTable = NULL;
char *RestrictRegMySQLUserCol = NULL;
char *RestrictRegMySQLPassCol = NULL;
char *RestrictRegMessage1 = NULL;
char *RestrictRegMessage2 = NULL;
char *RestrictRegMessage3 = NULL;
char *RestrictRegGroupDisabledMsg = NULL;
char *RestrictRegMySQLEnc = NULL;
int RestrictRegAllowGroup = 0;

/* Function Definitions */
int AnopeInit(int argc, char **argv);
void AnopeFini();
int ns_restrictreg_register(User *u);
int ns_restrictreg_group(User *u);
int ns_restrictreg_common(User *u, char *pass);
int ns_restrictreg_checkvalues();
int ns_restrictreg_reload(int argc, char **argv);
int ns_restrictreg_loadconf();

/* AnopeInit - Prepares the module for use */
int AnopeInit(int argc, char **argv)
{
    Command *c = NULL;
    EvtHook *hook = NULL;
    User *mod_loader = NULL;
    
    if (argc == 1)
        mod_loader = finduser(argv[0]);     
    
    c = createCommand("REGISTER", ns_restrictreg_register, NULL, -1, -1, -1, -1, -1);
    moduleAddCommand(NICKSERV, c, MOD_HEAD);
    
    c = createCommand("GROUP", ns_restrictreg_group, NULL, -1, -1, -1, -1, -1);
    moduleAddCommand(NICKSERV, c, MOD_HEAD);
    
    hook = createEventHook(EVENT_RELOAD, ns_restrictreg_reload);
    moduleAddEventHook(hook);
    
    if (!ns_restrictreg_loadconf()) {
        if (mod_loader)
            notice(s_OperServ, mod_loader->nick, "ERROR: Missing configuration options in services.conf - Please see the logs for details");
        return MOD_STOP;
    }
                
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    moduleMinVersion(1, 7, 14, 0);
    
    alog("[%s] Module Loaded Successfully!", MODNAME);
    
    return MOD_CONT;
}

/* AnopeFini - Unloads the module */
void AnopeFini()
{
    /* Free our module directives */
    if (RestrictRegMySQLHost)
        free(RestrictRegMySQLHost);
    if (RestrictRegMySQLUser)
        free(RestrictRegMySQLUser);
    if (RestrictRegMySQLPass)
        free(RestrictRegMySQLPass);
    if (RestrictRegMySQLDBName)
        free(RestrictRegMySQLDBName);
    if (RestrictRegMySQLTable)
        free(RestrictRegMySQLTable);
    if (RestrictRegMySQLUserCol)
        free(RestrictRegMySQLUserCol);
    if (RestrictRegMySQLPassCol)
        free(RestrictRegMySQLPassCol);
    if (RestrictRegMySQLEnc)
        free(RestrictRegMySQLEnc);
    if (RestrictRegMessage1)
        free(RestrictRegMessage1);
    if (RestrictRegMessage2)
        free(RestrictRegMessage2);
    if (RestrictRegMessage3)
        free(RestrictRegMessage3);       
    if (RestrictRegGroupDisabledMsg)
        free(RestrictRegGroupDisabledMsg);
        
    alog("[%s] Module Unloaded Successfully!", MODNAME);
}

/* ns_restrictreg_register - Handles the Register command */
int ns_restrictreg_register(User *u)
{
    char *modBuffer = NULL;
    char *pass = NULL;
    char delim = ' ';
    int status;
    
    if (!u || !u->nick) {
        return MOD_STOP;
    } else if (readonly) {
        notice_lang(s_NickServ, u, NICK_REGISTRATION_DISABLED);
        return MOD_STOP;
    } else if (checkDefCon(DEFCON_NO_NEW_NICKS)) {
        notice_lang(s_NickServ, u, OPER_DEFCON_DENIED);
        return MOD_STOP;
    } else if (u->na) {
        if (u->na->status & NS_VERBOTEN) {
            alog("%s: %s@%s tried to register FORBIDden nick %s", s_NickServ, u->username, common_get_vhost(u), u->nick);
            notice_lang(s_NickServ, u, NICK_CANNOT_BE_REGISTERED, u->nick);
        }
        else {
              notice_lang(s_NickServ, u, NICK_ALREADY_REGISTERED, u->nick);
        }
        return MOD_STOP;    
    }
    
    modBuffer = moduleGetLastBuffer();
    if (!modBuffer)
        return MOD_CONT;
    
    pass = myStrGetOnlyToken(modBuffer, delim, 0);
    if (!pass)
        return MOD_CONT;
    
    status = ns_restrictreg_common(u, pass);
    
    if (status != MOD_CONT) {
        if (stricmp(RestrictRegMessage1, "") != 0)
            notice(s_NickServ, u->nick, RestrictRegMessage1);
        if (stricmp(RestrictRegMessage2, "") != 0)
            notice(s_NickServ, u->nick, RestrictRegMessage2);
        if (stricmp(RestrictRegMessage3, "") != 0)
            notice(s_NickServ, u->nick, RestrictRegMessage3);
    }
    
    if (pass)
        free(pass);
    
    return status;
}

/* ns_restrictreg_group - Handles the Group command */
int ns_restrictreg_group(User *u)
{
    char *modBuffer = NULL;
    char *pass = NULL;
    char delim = ' ';
    int status;
    
    if (!u || !u->nick)
        return MOD_STOP;
        
    if (readonly) {
        notice_lang(s_NickServ, u, NICK_REGISTRATION_DISABLED);
        return MOD_STOP;
    } else if (checkDefCon(DEFCON_NO_NEW_NICKS)) {
        notice_lang(s_NickServ, u, OPER_DEFCON_DENIED);
        return MOD_STOP;
    } else if (u->na) {
        if (u->na->status & NS_VERBOTEN) {
            alog("%s: %s@%s tried to register FORBIDden nick %s", s_NickServ, u->username, common_get_vhost(u), u->nick);
            notice_lang(s_NickServ, u, NICK_CANNOT_BE_REGISTERED, u->nick);
        }
        else {
              notice_lang(s_NickServ, u, NICK_ALREADY_REGISTERED, u->nick);
        }
        return MOD_STOP;    
    } else if (!RestrictRegAllowGroup || RestrictRegAllowGroup <= 1) {
        if (debug)
            alog("[%s] debug: %s just attempted to group their nickname (grouping disabled)", MODNAME, u->nick);
        notice(s_NickServ, u->nick, "%s", RestrictRegGroupDisabledMsg);
        return MOD_STOP;
    } else if  (RestrictRegAllowGroup >= 3) {
        return MOD_CONT;
    }
    
    modBuffer = moduleGetLastBuffer();
    if (!modBuffer)
        return MOD_CONT;
    
    pass = myStrGetToken(modBuffer, delim, 1);
    
    if (!pass)
        return MOD_CONT;
        
    status = ns_restrictreg_common(u, pass);
     
    if (status != MOD_CONT) {
        if (stricmp(RestrictRegMessage1, "") != 0)
            notice(s_NickServ, u->nick, RestrictRegMessage1);
        if (stricmp(RestrictRegMessage2, "") != 0)
            notice(s_NickServ, u->nick, RestrictRegMessage2);
        if (stricmp(RestrictRegMessage3, "") != 0)
            notice(s_NickServ, u->nick, RestrictRegMessage3);
    }    
    
    if (pass)
        free(pass);
        
    return status;
}

/* ns_restrictreg_common - The common handler for checking databases */
int ns_restrictreg_common(User *u, char *pass)
{
    char *sql;
    char *quoted_nick;
    char *quoted_pass;
    int nick_len;
    int pass_len;
    int result = -1;
    int retval = MOD_STOP;
    
    if (!ns_restrictreg_checkvalues()) {
        alog("[%s] ERROR: There are missing values in services.conf - Please configure these before using this module", MODNAME);
        return MOD_CONT;
    }
    
    ns_restrictreg_mysql = mysql_init(NULL);
    if (!ns_restrictreg_mysql) {     
        alog("[%s] ERROR: Cannot create MySQL Object!", MODNAME);
        if (u)
            notice(s_NickServ, u->nick, "Your request could not be processed - A MySQL Error has occurred.");
        return MOD_STOP;
    }        
    
    if (!mysql_real_connect(ns_restrictreg_mysql, RestrictRegMySQLHost, RestrictRegMySQLUser, RestrictRegMySQLPass, RestrictRegMySQLDBName, 3306, NULL, 0)) {
        alog("[%s] ERROR: Unable to connect to MySQL Server - %s", MODNAME, mysql_error(ns_restrictreg_mysql));
        if (u)
            notice(s_NickServ, u->nick, "Your request could not be processed - A MySQL Error has occurred.");
        return MOD_STOP;
    }
      
    nick_len = strlen(u->nick);
    pass_len = strlen(pass);
    
    quoted_nick = smalloc((1 + (nick_len * 2)) * sizeof(char));
    quoted_pass = smalloc((1 + (pass_len * 2)) * sizeof(char));
    
    mysql_real_escape_string(ns_restrictreg_mysql, quoted_nick, u->nick, nick_len);
    mysql_real_escape_string(ns_restrictreg_mysql, quoted_pass, pass, pass_len);
    
    sql = smalloc((1 + (MAX_SQL_BUF * 2)) * sizeof(char));
    
    if (stricmp(RestrictRegMySQLEnc, "sha1") == 0) {
        snprintf(sql, MAX_SQL_BUF, "SELECT `%s` FROM %s WHERE `%s` = '%s' AND `%s` = SHA1('%s') LIMIT 0,1", RestrictRegMySQLUserCol, RestrictRegMySQLTable, RestrictRegMySQLUserCol, quoted_nick, RestrictRegMySQLPassCol, quoted_pass);
    } else if (stricmp(RestrictRegMySQLEnc, "md5") == 0) {
        snprintf(sql, MAX_SQL_BUF, "SELECT `%s` FROM %s WHERE `%s` = '%s' AND `%s` = MD5('%s') LIMIT 0,1", RestrictRegMySQLUserCol, RestrictRegMySQLTable, RestrictRegMySQLUserCol, quoted_nick, RestrictRegMySQLPassCol, quoted_pass);
    } else {
        snprintf(sql, MAX_SQL_BUF, "SELECT `%s` FROM %s WHERE `%s` = '%s' AND `%s` = '%s' LIMIT 0,1", RestrictRegMySQLUserCol, RestrictRegMySQLTable, RestrictRegMySQLUserCol, quoted_nick, RestrictRegMySQLPassCol, quoted_pass);     
    }
    
    if (debug > 2)
       alog("[%s] debug: SQL Query: %s", MODNAME, sql);
        
    result = mysql_query(ns_restrictreg_mysql, sql);
    if (result) {
        alog("[%s] ERROR: Error performing MySQL Query - %s", MODNAME, mysql_error(ns_restrictreg_mysql));
        if (u)
            notice(s_NickServ, u->nick, "Your request could not be processed - A MySQL Error has occurred.");        
        retval = MOD_STOP;
    } else {
        ns_restrictreg_mysql_res = mysql_store_result(ns_restrictreg_mysql);
        if (mysql_num_rows(ns_restrictreg_mysql_res) == 0) {
            retval = MOD_STOP;
        }
        else {
            retval = MOD_CONT;
        }
        
        mysql_free_result(ns_restrictreg_mysql_res);
    }
    
    mysql_close(ns_restrictreg_mysql);
    
    if (sql)
        free(sql);
    if (quoted_nick)
        free(quoted_nick);
    if (quoted_pass)
        free(quoted_pass);
    
    return retval;
}

int ns_restrictreg_checkvalues()
{
    if (!RestrictRegMySQLHost || !RestrictRegMySQLUser || !RestrictRegMySQLPass || !RestrictRegMySQLDBName || !RestrictRegMySQLTable || !RestrictRegMySQLUserCol || !RestrictRegMySQLPassCol || !RestrictRegMySQLEnc || !RestrictRegAllowGroup || !RestrictRegGroupDisabledMsg || !RestrictRegMessage1 || !RestrictRegMessage2 || !RestrictRegMessage3) {
        return false;
    }
    return true;
}
        

/* ns_restrictreg_reload - Handles the OperServ Reload event */
int ns_restrictreg_reload(int argc, char **argv)
{
    ns_restrictreg_loadconf();
    return MOD_CONT;
}

/* ns_restrictreg_loadconf - Loads the modules configuration directives */
int ns_restrictreg_loadconf()
{                             
    Directive ns_restrictreg_confdirs[] = {
        {"RestrictRegMySQLHost", {{PARAM_STRING, PARAM_RELOAD, &RestrictRegMySQLHost}}},
        {"RestrictRegMySQLUser", {{PARAM_STRING, PARAM_RELOAD, &RestrictRegMySQLUser}}},
        {"RestrictRegMySQLPass", {{PARAM_STRING, PARAM_RELOAD, &RestrictRegMySQLPass}}},
        {"RestrictRegMySQLDBName", {{PARAM_STRING, PARAM_RELOAD, &RestrictRegMySQLDBName}}},
        {"RestrictRegMySQLTable", {{PARAM_STRING, PARAM_RELOAD, &RestrictRegMySQLTable}}},
        {"RestrictRegMySQLUserCol", {{PARAM_STRING, PARAM_RELOAD, &RestrictRegMySQLUserCol}}},
        {"RestrictRegMySQLPassCol", {{PARAM_STRING, PARAM_RELOAD, &RestrictRegMySQLPassCol}}},
        {"RestrictRegMySQLEnc", {{PARAM_STRING, PARAM_RELOAD, &RestrictRegMySQLEnc}}},
        {"RestrictRegMessage1", {{PARAM_STRING, PARAM_RELOAD, &RestrictRegMessage1}}},
        {"RestrictRegMessage2", {{PARAM_STRING, PARAM_RELOAD, &RestrictRegMessage2}}},
        {"RestrictRegMessage3", {{PARAM_STRING, PARAM_RELOAD, &RestrictRegMessage3}}},
        {"RestrictRegGroupDisabledMsg", {{PARAM_STRING, PARAM_RELOAD, &RestrictRegGroupDisabledMsg}}},
        {"RestrictRegAllowGroup", {{PARAM_POSINT, PARAM_RELOAD, &RestrictRegAllowGroup}}}        
    };
    
    int confcount = 12;
    int i = 0;
        
    for (i = 0; i < confcount; i++) {
        if (!&ns_restrictreg_confdirs[i])
            break;
                   
        moduleGetConfigDirective(&ns_restrictreg_confdirs[i]);      
        
        if (!(*(char **) (&ns_restrictreg_confdirs[i])->params[0].ptr)) {
            alog("[%s] ERROR: Missing configuration option '%s' - Please read %s.c before compiling", MODNAME, (&ns_restrictreg_confdirs[i])->name, MODNAME);
            return false;
        }            
    }

    moduleGetConfigDirective(&ns_restrictreg_confdirs[confcount]);
    
    if (debug) {  
        alog("[%s] debug: Successfully loaded %d configuration options!", MODNAME, confcount);
    }

    return true;
}
