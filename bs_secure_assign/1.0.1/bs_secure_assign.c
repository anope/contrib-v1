/*------------------------------------------------------------
* Name:    bs_secure_assign
* Author:  LEthaLity 'Lee' <lee@lethality.me.uk>
* Version: 1.0.1
* Date:    26th August 2009
* ------------------------------------------------------------
* Function: This module adds a password for BotServ Assign
* AND Unassign. Idea extends on Katsklaws bs_assign_pass.
* ------------------------------------------------------------
* This Module is supported on Anope-1.8.2, and should work
* problem free down to about 1.7.21. Contact me if it doesn't.
* ------------------------------------------------------------
* This module has Required config directives.
*
* Please add the following to services.conf, and edit as 
* necessary:
BSSecurePass "oursecretpassword"
#BSSecurePassRoot 
*
* ------------------------------------------------------------
* Changes:
* 1.0.1 - Re-Added optional Configuration Directive to require
          Roots to use password too.
* 1.0.0 - First Release
*/

#include "module.h"

#define AUTHOR "LEthaLity"
#define VERSION "1.0.1"

int bs_assign(User * u);
int bs_unassign(User * u);
int do_reload(int argc, char **argv);
void my_load_config(void); //load config directive BSSecurePass

char *BSSecurePass = NULL;
int BSSecurePassRoot = 0;

int AnopeInit(int argc, char **argv)
{
	Command *c;
	EvtHook *hook = NULL;
	int status;

	c = createCommand("assign", bs_assign, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(BOTSERV, c, MOD_HEAD);

	c = createCommand("unassign", bs_unassign, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(BOTSERV, c, MOD_HEAD);
	
	hook = createEventHook(EVENT_RELOAD, do_reload);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("Unable to hook to EVENT_RELOAD. Unloading module [%d]", status);
		return MOD_STOP;
	}
	my_load_config();

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("bs_secure_assign: Unloading Module");
}

int do_reload(int argc, char **argv)
{
    my_load_config();
    return MOD_CONT;
}

int bs_assign(User *u)
{
	char *text;
	char *setting;
	char *value;
	char *pass;
	int ret = MOD_CONT;
	
	if (!BSSecurePass)
		return MOD_CONT;
	
	text = moduleGetLastBuffer();
	setting = myStrGetToken(text, ' ', 0);
	value = myStrGetToken(text, ' ', 1);
	pass = myStrGetToken(text, ' ', 2);
	
	if (!is_services_root(u) || (BSSecurePassRoot && is_services_root(u))) {
		if (!pass) {
			notice_user(s_OperServ, u, "No Password given.");
			ret = MOD_STOP;
		} else if (strcmp(pass, BSSecurePass) != 0) {
			notice_user(s_OperServ, u, "invalid password!");
			ret = MOD_STOP;
			free(setting);
			free(value);
			free(pass);
		}
	}
	return ret;
}

int bs_unassign(User *u)
{
	char *text;
	char *setting;
	char *value;
	char *pass;
	int ret = MOD_CONT;
	
	if (!BSSecurePass)
		return MOD_CONT;
	
	text = moduleGetLastBuffer();
	setting = myStrGetToken(text, ' ', 0);
	value = myStrGetToken(text, ' ', 1);
	pass = myStrGetToken(text, ' ', 2);
	
	if (!is_services_root(u) || (BSSecurePassRoot && is_services_root(u))) {
		if (!pass) {
			notice_user(s_OperServ, u, "No Password given.");
			ret = MOD_STOP;
	} else if (strcmp(pass, BSSecurePass) != 0) {
			notice_user(s_OperServ, u, "Invalid Password");
			ret = MOD_STOP;
	free(setting);
	free(value);
	free(pass);
		}
	}
	return ret;
}

void my_load_config(void) //Get the BSSecurePass from services conf
{
	alog("[bs_secure_assign] Config (re)loaded.");
    Directive confvalue_one[] = {
		{"BSSecurePass", {{PARAM_STRING, PARAM_RELOAD, &BSSecurePass}}}
    };
	Directive confvalue_two[] = { //Re-added optional directive to require roots to use password
        {"BSSecurePassRoot", {{PARAM_SET, PARAM_RELOAD, &BSSecurePassRoot}}}
    };
 
	if (BSSecurePass)
		free(BSSecurePass);
	
    moduleGetConfigDirective(confvalue_one);
	moduleGetConfigDirective(confvalue_two);
}

/* End of File */
