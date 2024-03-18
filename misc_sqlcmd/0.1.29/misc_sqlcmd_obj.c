#include "misc_sqlcmd.h"

 /*************************************************************
 *  misc_sqlcmd - Perform commands in Anope using a MySQL DB  *
 *************************************************************/

 /*
  * $Id: misc_sqlcmd_obj.c 18 2006-10-30 18:09:53Z heinz $
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

 
/* sqlcmd_create - Create an SQLCmd entry and hook it in */
int sqlcmd_create(char *name, int params, int (*handle)(int ac, char **av))
{
        SQLCmd *cmd = NULL;
        SQLCmd *tmp = NULL;
        
        cmd = smalloc(sizeof(SQLCmd));
        if (!cmd) {
                alog("debug: [%s] Cannot allocate memory!", MODNAME);
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
        }
        else {
                for (tmp = SQLCmdHead; tmp; tmp = tmp->next) {
                        if (tmp->next == NULL)
                                break;
                }
                if (!tmp) {
                        free(cmd->name);
                        free(cmd);
                        alog("debug: [%s] Cannot find where to put new hook!", MODNAME);
                        return 1;
                }
                tmp->next = cmd;
                cmd->prev = tmp;
                cmd->next = NULL;
        }
        return 0;
}

/* sqlcmd_find - Find an SQLCmd entry based on name */
SQLCmd *sqlcmd_find(char *name)
{
        SQLCmd *tmp = NULL;
        
        for (tmp = SQLCmdHead; tmp; tmp = tmp->next) { 
                if (stricmp(tmp->name, name) == 0)
                        return tmp;
        }
        
        return NULL;       
}

/* sqlcmd_status_update - Update the database with the status info */
int sqlcmd_status_update(int id, int status)
{
        if (debug)
               alog("debug: [%s] Command processed with status %d", MODNAME, status);
        snprintf(sql, MAX_SQL_BUF, "UPDATE `anope_sqlcmd` SET `status` = %d WHERE `id` = %d", status, id);
        sqlcmd_mysql_query(sql, 0);
        return 0;
}
