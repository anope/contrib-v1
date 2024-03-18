#include "misc_sqlcmd.h"

 /*************************************************************
 *  misc_sqlcmd - Perform commands in Anope using a MySQL DB  *
 *************************************************************/

 /*
  * $Id: misc_sqlcmd_mysql.c 25 2006-12-12 15:12:33Z heinz $
  */
 
 /**************************************************************
 * Module Info & Changelog      
 * -----------------------                                     
 * Please see misc_sqlcmd_main.c for the information, changelog
 * and the configuration information.
 *
 **************************************************************/
 
 /**************************************************************
 *                                                             *
 * PLEASE DO NOT EDIT ANYTHING BELOW HERE - MODULE CODE BEGINS *
 *                                                             *
 **************************************************************/


/* sqlcmd_mysql_connect - Check connection to mysql and connect if needed */
int sqlcmd_mysql_connect()
{
        if (sqlcmd_mysql_connected == 1)
                return 1;
        if (!do_mysql) {
                alog("[%s] ERROR: MySQL is disabled in Anope - Please configure Anope to use MySQL", MODNAME);
                return 0;
        }
                
        sqlcmd_mysql = mysql_init(NULL);
        if (sqlcmd_mysql == NULL) {
                alog("[%s] ERROR: Cannot create MySQL Object!", MODNAME);
                return 0;
        }
       
        if (MysqlSock) {
                if ((!mysql_real_connect(sqlcmd_mysql, MysqlHost, MysqlUser, MysqlPass, MysqlName, MysqlPort, MysqlSock, 0))) {
                        log_perror("[%s] MySQL Error: Cant connect to MySQL: %s\n", MODNAME, mysql_error(sqlcmd_mysql));                           
                        return 0;
                }
        } else {
                if ((!mysql_real_connect(sqlcmd_mysql, MysqlHost, MysqlUser, MysqlPass, MysqlName, MysqlPort, NULL, 0))) {
                        log_perror("[%s] MySQL Error: Cant connect to MySQL: %s\n", MODNAME, mysql_error(sqlcmd_mysql));
                        return 0;
                }
        }
         
        sqlcmd_mysql_connected = 1;      
        return 1;
}

/* sqlcmd_mysql_query - Perform a MySQL query */
int sqlcmd_mysql_query(char *sql, int store_result)
{
        int lcv = 0;
        int ret = 0;
        for (lcv = 0; lcv < MysqlRetries; lcv++) {        
                if (!sqlcmd_mysql_connect())
                        continue;
                if (debug)
                        alog("debug: [%s] Query: %s", MODNAME, sql);
                if (!mysql_query(sqlcmd_mysql, sql)) {
                        if (store_result == 1)
                                sqlcmd_res = mysql_store_result(sqlcmd_mysql);
                        else if (store_result == 2)
                                chksum_res = mysql_store_result(sqlcmd_mysql);
                        ret = 1;
                        break;
                }
                log_perror("[%s] Unable to run query: %s\n", MODNAME, mysql_error(sqlcmd_mysql));
        }       
        return ret;              
}
