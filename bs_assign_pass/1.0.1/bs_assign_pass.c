/***************************************************************************************/
/* Anope Module : bs_assign_pass.c : v1.x                                              */
/* Scott Seufert                                                                       */
/* katsklaw@ircmojo.net                                                                */
/*                                                                                     */
/* Anope (c) 2000-2002 Anope.org                                                       */
/*                                                                                     */
/* This program is free software; you can redistribute it and/or modify it under the   */
/* terms of the GNU General Public License as published by the Free Software           */
/* Foundation; either version 1, or (at your option) any later version.                */
/*                                                                                     */
/*  This program is distributed in the hope that it will be useful, but WITHOUT ANY    */
/*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A    */
/*  PARTICULAR PURPOSE.  See the GNU General Public License for more details.          */
/*                                                                                     */
/***************************************************************************************/

#include "module.h"

#define AUTHOR "katsklaw"
#define VERSION "1.0.1"

int bs_assign(User * u);
int do_reload(int argc, char **argv);
void my_load_config(void);

char *BSAssignPass = NULL;
int BSAssignPassRoot = 0;

int AnopeInit(int argc, char **argv)
{
	EvtHook *hook = NULL;
	int status;

	Command *c;
	
	c = createCommand("assign", bs_assign, is_services_admin, -1, -1, -1, -1, -1);
	moduleAddCommand(BOTSERV, c, MOD_HEAD);
	
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	
	hook = createEventHook(EVENT_RELOAD, do_reload);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("Unable to hook to EVENT_RELOAD. Unloading module [%d]", status);
		return MOD_STOP;
	}

	my_load_config();
	return MOD_CONT;
}

void AnopeFini(void)
{
	/* Nothing to do here */
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
	
	if (!BSAssignPass)
		return MOD_CONT;
	
	text = moduleGetLastBuffer();
	setting = myStrGetToken(text, ' ', 0);
	value = myStrGetToken(text, ' ', 1);
	pass = myStrGetToken(text, ' ', 2);
	
	if (!is_services_root(u) || (BSAssignPassRoot && is_services_root(u))) {
		if (!pass) {
			notice_user(s_OperServ, u, "No Password given.");
			ret = MOD_STOP;
		} else if (strcmp(pass, BSAssignPass) != 0) {
			notice_user(s_OperServ, u, "invalid password!");
			ret = MOD_STOP;
		}
	}
	
	if (setting)
		free(setting);
	if (value)
		free(value);
	if (pass)
		free(pass);
	
	return ret;
}

void my_load_config(void)
{
    Directive confvalue[][1] = {
		{{"BSAssignPass", {{PARAM_STRING, PARAM_RELOAD, &BSAssignPass}}}},
		{{"BSAssignPassRoot", {{PARAM_SET, PARAM_RELOAD, &BSAssignPassRoot}}}}
    };

	if (BSAssignPass)
		free(BSAssignPass);
	
    moduleGetConfigDirective(confvalue[0]);
	moduleGetConfigDirective(confvalue[1]);
}

/* EOF */
