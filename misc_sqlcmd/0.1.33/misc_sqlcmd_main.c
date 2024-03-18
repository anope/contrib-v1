#include "misc_sqlcmd.h"

 /*************************************************************
 *  misc_sqlcmd - Perform commands in Anope using a MySQL DB  *
 *************************************************************/

 /*
  * $Id: misc_sqlcmd_main.c 32 2007-08-23 20:47:21Z heinz $
  */
 
 /**************************************************************
 * Module Info & Changelog      
 * -----------------------                                     
 * Allows you to perform commands in Anope's core which could
 * usually only be performed by a connected user.
 *
 * This module will allow you to perform actions from a 
 * website, for example, allow web-based nickname registrations.
 *
 * For Instructions, Updates and Contact Info please visit:
 * http://modules.anope.org/profile.php?id=7
 *
 * Please configure this module before use.
 *
 * v2.00 - Complete Module Re-write
 *         Use a command hook system to allow for easy
 *           expansion.
 *         Use ModuleDirectives for easy customisation
 *         Implement a checksum validator - makes it harder
 *           for unauthorized users to inject commands unless
 *           they have access to the checksum salt.
 * v1.00 - Unreleased Initial Version
 **************************************************************/
 
 /**************************************************************
 * Configuration Information
 * -------------------------
 * This module requires the following configuration values to
 * be added to services.conf before the module will load.
 *
 * Please see README.txt, which is included with this module
 * source to see how to achieve this.
 *
 **************************************************************/
 
 /**************************************************************
 *                                                             *
 * PLEASE DO NOT EDIT ANYTHING BELOW HERE - MODULE CODE BEGINS *
 *                                                             *
 **************************************************************/
 
/* Global Variables */
char sql[MAX_SQL_BUF];
MYSQL *sqlcmd_mysql;
MYSQL_RES *sqlcmd_res;
MYSQL_RES *chksum_res;
MYSQL_ROW sqlcmd_row;
MYSQL_ROW chksum_row;
int sqlcmd_timeout = 10;
int sqlcmd_mysql_connected = 0;
char *sqlcmd_chksum_salt = NULL;
SQLCmd *SQLCmdHead = NULL;


/* AnopeInit - Prepare the module for use */
int AnopeInit(int ac, char **av)
{
        int status = 0;
        EvtHook *hook = NULL;
        
	if (!do_mysql) {
		alog("[%s] ERROR: MySQL is not enabled/configured in services.conf!", MODNAME);
		alog("[%s] ERROR: Please enable MySQL in services.conf before loading this module.", MODNAME);
		return MOD_STOP;
	}
	snprintf(sql, MAX_SQL_BUF, "DESCRIBE `anope_sqlcmd`");	
	if (sqlcmd_mysql_query(sql, 1) == 0) {
	        if (sqlcmd_res)
	                mysql_free_result(sqlcmd_res);	        
		alog("[%s] ERROR: Cannot perform MySQL query!", MODNAME);
		alog("[%s] ERROR: Please ensure you import anope_sqlcmd.sql into your MySQL Database.", MODNAME);
		return MOD_STOP;
	}	
	mysql_free_result(sqlcmd_res);
	
	/* Hook the SQLCmd's here */
	status += sqlcmd_create("NICK_REG", 3, sqlcmd_handle_nickreg);
	status += sqlcmd_create("NICK_CONF", 3, sqlcmd_handle_nickconf);	
	status += sqlcmd_create("NICK_GROUP", 3, sqlcmd_handle_nickgroup);
	status += sqlcmd_create("NICK_DROP", 2, sqlcmd_handle_nickdrop);

	status += sqlcmd_create("CHAN_REG", 4, sqlcmd_handle_chanreg);	
        status += sqlcmd_create("CHAN_ADD_SOP", 5, sqlcmd_handle_chanaddsop);
        status += sqlcmd_create("CHAN_DEL_SOP", 5, sqlcmd_handle_chandelsop);
        status += sqlcmd_create("CHAN_ADD_AOP", 5, sqlcmd_handle_chanaddaop);
        status += sqlcmd_create("CHAN_DEL_AOP", 5, sqlcmd_handle_chandelaop);
        status += sqlcmd_create("CHAN_ADD_HOP", 5, sqlcmd_handle_chanaddhop);
        status += sqlcmd_create("CHAN_DEL_HOP", 5, sqlcmd_handle_chandelhop);             
        status += sqlcmd_create("CHAN_ADD_VOP", 5, sqlcmd_handle_chanaddvop);
        status += sqlcmd_create("CHAN_DEL_VOP", 5, sqlcmd_handle_chandelvop);
        status += sqlcmd_create("CHAN_ACCESS", 6, sqlcmd_handle_chanaccess);
        status += sqlcmd_create("CHAN_TOPIC", 4, sqlcmd_handle_chantopic);
        status += sqlcmd_create("CHAN_DROP", 4, sqlcmd_handle_chandrop);
	
	status += sqlcmd_create("MEMO_SEND", 3, sqlcmd_handle_memosend);
	status += sqlcmd_create("MEMO_DEL", 3, sqlcmd_handle_memodel);
	status += sqlcmd_create("MEMO_CLEAR", 2, sqlcmd_handle_memoclear);
	
	status += sqlcmd_create("BOT_ASSIGN", 5, sqlcmd_handle_botassign);
	status += sqlcmd_create("BOT_SAY", 4, sqlcmd_handle_botsay);
	status += sqlcmd_create("BOT_ACT", 4, sqlcmd_handle_botact);
	status += sqlcmd_create("BOT_UNASSIGN", 4, sqlcmd_handle_botunassign);

	status += sqlcmd_create("PING", 1, sqlcmd_handle_ping);
	
	if (status > 0) {
	        alog("[%s] ERROR: Cannot create SQLCmd Command Hooks", MODNAME);
	        return MOD_STOP;
	}
	
	if (!sqlcmd_conf_load()) {
	        alog("[%s] ERROR: Cannot load configuration directives!", MODNAME);
	        alog("[%s] ERROR: Please read README.txt before loading this module.", MODNAME);
	        return MOD_STOP;
        }

        hook = createEventHook(EVENT_RELOAD, sqlcmd_conf_reload);
        moduleAddEventHook(hook);	
        
        moduleAddCallback("sqlcmd", time(NULL) + sqlcmd_timeout, sqlcmd_handle, 0, NULL);
	if (debug)
		alog("debug: [%s/%s] Module Loaded Successfully!", MODNAME, VERSION);
	return MOD_CONT;
}

/* AnopeFini - Unload the module */
void AnopeFini()
{
        SQLCmd *tmp = NULL;
        SQLCmd *tmp2 = NULL;
          
        for (tmp = SQLCmdHead; tmp != NULL; tmp = tmp2) {
                if (tmp->next)
                        tmp2 = tmp->next;                
                else
                        tmp2 = NULL;
                if (tmp->name)
                        free(tmp->name);
                free(tmp);
        }                             
	moduleDelCallback("sqlcmd");
	if (debug)
		alog("debug: [%s] Module Unloaded Successfully!", MODNAME);
}

/* sqlcmd_conf_reload - Handles the RELOAD event */
int sqlcmd_conf_reload(int argc, char **argv)
{
        sqlcmd_conf_load();
        return MOD_CONT;
}

/* sqlcmd_conf_load - Loads the modules configuration directives */
int sqlcmd_conf_load()
{
        Directive sqlcmd_confdirs[] = {
                {"SQLCmdChecksumSalt", {{PARAM_STRING, PARAM_RELOAD, &sqlcmd_chksum_salt}}},
                {"SQLCmdUpdateTimeout", {{PARAM_POSINT, PARAM_RELOAD, &sqlcmd_timeout}}}
        };
        int timeout_default = 10;

        moduleGetConfigDirective(&sqlcmd_confdirs[0]);
        if (!(*(char **) (&sqlcmd_confdirs[0])->params[0].ptr)) {
                alog("[%s] ERROR: Missing configuration option '%s' - Please read README.txt for details", MODNAME, (&sqlcmd_confdirs[0])->name);
                return false;
        }
        moduleGetConfigDirective(&sqlcmd_confdirs[1]);
        
        if (((int) (&sqlcmd_confdirs[1])->params[0].ptr) <= 0) {
                alog("debug: [%s] Setting timeout to default of 10 seconds..", MODNAME);
                (&sqlcmd_confdirs[1])->params[0].ptr = &timeout_default;
        }
                
        if (debug) {
                alog("debug: [%s] Successfully loaded configuration options!", MODNAME);
        }

        return true;
}

/*
 * sqlcmd_handle - Callback from core every sqlcmd_timeout seconds
 */
int sqlcmd_handle(int ac, char **av)
{
        char *param_array[12] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
        char *chksum = NULL;
        SQLCmd *cmd = NULL;
        int i;
        int cmd_status;

	snprintf(sql, MAX_SQL_BUF, "SELECT * FROM `anope_sqlcmd` WHERE `status` = 0 ORDER BY `timestamp` ASC");        
	if (sqlcmd_mysql_query(sql, 1) == 0) {
		alog("[%s] ERROR: Unable to perform MySQL Query!", MODNAME);
		alog("[%s] ERROR: Please check the error above for more details..", MODNAME);
		return MOD_STOP;
	}

        if (debug)
        	alog("debug: [%s] There are %d functions awaiting processing...", MODNAME, (int) mysql_num_rows(sqlcmd_res));
        	
	while ((sqlcmd_row = mysql_fetch_row(sqlcmd_res))) {
	        cmd_status = 0;
	        if (debug)
	                alog("debug: [%s]  Command %d: %s - Params: %s - Timestamp: %s - Checksum: %s", MODNAME, atoi(sqlcmd_row[0]), sqlcmd_row[1], sqlcmd_row[2], sqlcmd_row[3], sqlcmd_row[4]);
        	chksum = sqlcmd_checksum(sqlcmd_row[1], sqlcmd_row[2], sqlcmd_row[3]);
        	if (stricmp(chksum, sqlcmd_row[4]) != 0) {
        	        alog("[%s] ERROR: Invalid checksum received for ID %d", MODNAME, atoi(sqlcmd_row[0]));
        	        cmd_status = SQLCMD_ERROR_CHECKSUM;
        	} else if (!(cmd = sqlcmd_find(sqlcmd_row[1]))) {              
        	        alog("[%s] ERROR: Received unknown command '%s'", MODNAME, sqlcmd_row[1]);
        	        cmd_status = SQLCMD_ERROR_UNKNOWN_CMD;
        	} else {
        	        if (debug)
        	                alog("debug: [%s] Command %d has a valid checksum - Processing...", MODNAME, atoi(sqlcmd_row[0]));
        	        sqlcmd_str_to_params(sqlcmd_row[2], cmd->params, param_array);
        	        cmd_status = cmd->handle(cmd->params, param_array);
                        for (i = 0; param_array[i]; i++) {
                                free(param_array[i]);
                        }                
                }
                if (cmd_status)
                        sqlcmd_status_update(atoi(sqlcmd_row[0]), cmd_status);                 
                if (chksum)
                        free(chksum);
	}	
	mysql_free_result(sqlcmd_res);

	moduleAddCallback("sqlcmd", time(NULL) + sqlcmd_timeout, sqlcmd_handle, 0, NULL);
	return MOD_CONT;
}

int sqlcmd_handle_notsupported(int ac, char **av)
{
        return SQLCMD_ERROR_UNSUPPORTED;
}

/* sqlcmd_str_to_params - Turn a space-deliminated string into a char array */
void sqlcmd_str_to_params(char *params, int ac, char **param_array)
{
        char *tmp_tok = NULL;
        char delim = ' ';
        int i = 0;
        
        if (ac > 11)
                ac = 11;
        
        for (i = 0; ((i < ac) && ((tmp_tok = myStrGetToken(params, delim, i)) != NULL)); i++) {
                param_array[i] = tmp_tok;
        }
        param_array[i] = myStrGetTokenRemainder(params, delim, i);
}

/* sqlcmd_checksum - Produce a checksum from the needed values */
char *sqlcmd_checksum(char *cmd, char *param_str, char *ts)
{
        char *chksum_sql = NULL;
        char *chksum = NULL;
        char *quote_param;
        int checksum_len = 0;
        
        checksum_len = strlen(cmd) + strlen(param_str) + strlen(ts) + strlen(sqlcmd_chksum_salt) + 20;
        chksum_sql = smalloc(sizeof(char) * checksum_len);

        quote_param = sqlcmd_mysql_quote(param_str);       
        snprintf(chksum_sql, checksum_len - 1, "SELECT MD5('%s:%s:%s:%s')", cmd, quote_param, ts, sqlcmd_chksum_salt);
        free(quote_param);
        if (sqlcmd_mysql_query(chksum_sql, 2) != 1)
                return NULL;
        if (mysql_num_rows(chksum_res) != 1)
                return NULL;
        chksum_row = mysql_fetch_row(chksum_res);
        chksum = sstrdup(chksum_row[0]);
        mysql_free_result(chksum_res);
        return chksum;
}

/*
 * sqlcmd_checkchanstatus - Checks to see whether a user has access
 * to a channel, either via access list or via password.
 *
 * Return values:
 *   < 0 = Error occurred / No access
 *   0 = Access granted via access list
 *   1 = Access granted with password/founder access
 *
 */
int sqlcmd_checkchanstatus(char *nick, char *nickpass, char *channel, char *chanpass, int minlvl)
{
        NickAlias *na;
        Channel *c;
        ChanAccess *access;
        int i;
        
        if (!nick || !nickpass || !channel || !chanpass) {
                if (debug)
                        alog("debug: [%s] sqlcmd_checkchanstatus - invalid params!", MODNAME);
                return -2;
        }
        na = findnick(nick);
        if (!na) {
                return -3;          
                if (debug)
                        alog("debug: [%s] sqlcmd_checkchanstatus for %s on %s - No such nickname!", MODNAME, nick, channel);      
        }
        if (enc_check_password(nickpass, na->nc->pass) != 1) {
                if (debug)
                        alog("debug: [%s] sqlcmd_checkchanstatus for %s on %s - Invalid Password!", MODNAME, nick, channel);
                return -4;
        }
        c = findchan(channel);
        if (!c || !c->ci) {
               if (debug)
                        alog("debug: [%s] sqlcmd_checkchanstatus for %s on %s - No such channel!", MODNAME, nick, channel);
                return -5;
        }
        if (na->nc == c->ci->founder) {
               if (debug)
                        alog("debug: [%s] sqlcmd_checkchanstatus for %s on %s - User is the founder!", MODNAME, nick, channel);         
                return 1;
        }
        if (enc_check_password(chanpass, c->ci->founderpass) == 1) {
                if (debug)
                        alog("debug: [%s] sqlcmd_checkchanstatus for %s on %s - User gave the founder password!", MODNAME, nick, channel);
                return 1;
        }        
        if (minlvl == -1)
                return -1;
                
        for (access = c->ci->access, i = 0; i < c->ci->accesscount; access++, i++) {
                if (access->in_use && access->nc == na->nc) {        
                        if (c->ci->levels[minlvl] == access->level) {
                                if (debug)
                                        alog("debug: [%s] sqlcmd_checkchanstatus for %s on %s - User has access via access list!", MODNAME, nick, channel);
                                return 0;
                        }
                }
        }
        
        return -1;
}

/*
 * PING - Handles a ping request
 *
 * av[0] - Timestamp
 */
int sqlcmd_handle_ping(int ac, char **av)
{
	return SQLCMD_ERROR_NONE;
}

