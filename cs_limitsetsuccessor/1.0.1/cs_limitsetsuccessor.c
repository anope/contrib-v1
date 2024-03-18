#include "module.h"
#define AUTHOR "heinz (http://modules.anope.org/profile.php?id=7)"
#define VERSION "1.0"
#define MODNAME "cs_limitsetsuccessor"

 /****************************
 * cs_limitsetsuccessor v1.0 *
 *****************************/

 /********************************
 *
 * This module limits the user of
 * /msg ChanServ SET #chan SUCCESSOR
 * to one of three user levels.
 *
 * - Services Roots Only
 * - Services Admins and above
 * - Services Operators and above
 *
 * There is one configuration directive
 * that MUST be specified in services.conf
 * before this module is loaded.
 *
 * LimitCSSetSuccessor "level"
 *
 * Where level is one of:
 *  - root (Services Roots only)
 *  - admin (Services Admin and above)
 *  - oper (Services Opers and above)
 *
 * This module can be obtained from the
 * Anope Modules site located at:
 *
 * http://modules.anope.org/viewmod.php?id=77
 *
 ********************************/

 /**********************************
 * DO NOT EDIT ANYTHING BELOW HERE *
 **********************************/

char *set_successor_level = NULL;
int cs_limitsetsuccessor(User *u);
int cs_limitsetsuccessor_reload(int argc, char **argv);
int cs_limitsetsuccessor_conf();

int AnopeInit(int argc, char **argv)
{
	Command *c = NULL;
	EvtHook *h = NULL;
	int status;

	status = cs_limitsetsuccessor_conf();
	if (status == 0) {
		alog("[%s] Unloading due to error...", MODNAME);
		return MOD_STOP;
	}

	c = createCommand("SET", cs_limitsetsuccessor, NULL, -1, -1, -1, -1, -1);
	status = moduleAddCommand(CHANSERV, c, MOD_HEAD);
	if (status != MOD_ERR_OK) {
		alog("[%s] ERROR: Unable to hook command - Unloading...", MODNAME);
		return MOD_STOP;
	}

	h = createEventHook(EVENT_RELOAD, cs_limitsetsuccessor_reload);
	status = moduleAddEventHook(h);
	if (status != MOD_ERR_OK) {
		alog("[%s] ERROR: Unable to hook to event - Unloading...", MODNAME);
		return MOD_STOP;
	}

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	alog("[%s] /msg ChanServ SET #channel SUCCESSOR is now limited to Services %ss", MODNAME, set_successor_level);
	alog("[%s] Module loaded successfully!", MODNAME);
	return MOD_CONT;
}

void AnopeFini()
{
	alog("[%s] Module unloaded successfully!", MODNAME);
}

int cs_limitsetsuccessor(User *u)
{
	char *buffer = moduleGetLastBuffer();
	char *cmd = NULL;
	int user_has_access = MOD_STOP;

	if (!buffer)
		return MOD_CONT;
	cmd = myStrGetToken(buffer, ' ', 1);
	if (!cmd || stricmp(cmd, "successor"))
		return MOD_CONT;	

	if (!stricmp(set_successor_level, "root") && is_services_root(u))
		user_has_access = MOD_CONT;
	else if (!stricmp(set_successor_level, "admin") && (is_services_root(u) || is_services_admin(u)))
		user_has_access = MOD_CONT;
	else if (!stricmp(set_successor_level, "oper") && (is_services_root(u) || is_services_admin(u) || is_services_oper(u)))
		user_has_access = MOD_CONT;
	else
		user_has_access = MOD_STOP;

	if (user_has_access == MOD_STOP)
		notice_lang(s_ChanServ, u, ACCESS_DENIED);

	if (cmd)
		free(cmd);

	return user_has_access;
}

int cs_limitsetsuccessor_reload(int argc, char **argv)
{
	int status;
	if (argc >= 1 && !stricmp(argv[0], EVENT_START)) {
		status = cs_limitsetsuccessor_conf();
	}
	return MOD_CONT;
}

int cs_limitsetsuccessor_conf()
{
	char *tmp = NULL;
	Directive cs_limitsetsuccessor_dir = 
        	{"LimitCSSetSuccessor", {{PARAM_STRING, PARAM_RELOAD, &tmp}}};    
                   
	moduleGetConfigDirective(&cs_limitsetsuccessor_dir);      
        
	if (!tmp || (stricmp(tmp, "root") && stricmp(tmp, "admin") && stricmp(tmp, "oper"))) {
        	alog("[%s] ERROR: Missing configuration option '%s' - Please read %s.c before compiling", MODNAME, (&cs_limitsetsuccessor_dir)->name, MODNAME);
		if (set_successor_level)
			alog("[%s] WARNING: Using previous value of: %s", MODNAME, set_successor_level);
		if (mod_current_user) {
	   		notice(s_OperServ, mod_current_user->nick, "\002%s\002 - Invalid or Missing configuration directive. See services.log for more information.", MODNAME);
			if (set_successor_level)
				notice(s_OperServ, mod_current_user->nick, "\002%s\002 - Using previous value of: %s", MODNAME, set_successor_level);
		}
        	return 0;
        }

	if (set_successor_level)
		free(set_successor_level);
	set_successor_level = sstrdup(tmp);
	free(tmp);	

	return 1;
}
