/***************************************************************************************/
/* Anope Module : getpass_pass.c : v1.x                                                */
/* Scott Seufert                                                                       */
/* katsklaw@ircmojo.org                                                                */
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
#define VERSION "1.0.0"

int cs_mygetpass(User * u);
int ns_mygetpass(User * u);
int do_reload(int argc, char **argv);
void my_load_config(void);

char *CSGetPassPass = NULL;
char *NSGetPassPass = NULL;

int AnopeInit(int argc, char **argv)
{
	EvtHook *hook = NULL;
	int status;

	Command *c;
	
	c = createCommand("GETPASS", cs_mygetpass, is_services_root, -1, -1, -1, -1, -1);
	moduleAddCommand(CHANSERV, c, MOD_HEAD);

	c = createCommand("GETPASS", ns_mygetpass, is_services_root, -1, -1, -1, -1, -1);
	moduleAddCommand(NICKSERV, c, MOD_HEAD);
	
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


int cs_mygetpass(User *u)
{
	char *text;
	char *channel;
	char *pass;
	int ret = MOD_CONT;
	
	if (!CSGetPassPass)
		return MOD_CONT;
	
	text = moduleGetLastBuffer();
	channel = myStrGetToken(text, ' ', 0);
	pass = myStrGetToken(text, ' ', 1);
	
	if (!pass) {
		notice_user(s_ChanServ, u, "No Password given.");
		ret = MOD_STOP;
	} else if (strcmp(pass, CSGetPassPass) != 0) {
		notice_user(s_ChanServ, u, "invalid password!");
		ret = MOD_STOP;
	}
	
	if (channel)
		free(channel);
	if (pass)
		free(pass);
	
	return ret;
}

int ns_mygetpass(User *u)
{
	char *text;
	char *nick;
	char *pass;
	int ret = MOD_CONT;
	
	if (!NSGetPassPass)
		return MOD_CONT;
	
	text = moduleGetLastBuffer();
	nick = myStrGetToken(text, ' ', 0);
	pass = myStrGetToken(text, ' ', 1);
	
	if (!pass) {
		notice_user(s_NickServ, u, "No Password given.");
		ret = MOD_STOP;
	} else if (strcmp(pass, NSGetPassPass) != 0) {
		notice_user(s_NickServ, u, "invalid password!");
		ret = MOD_STOP;
	}
	
	if (nick)
		free(nick);
	if (pass)
		free(pass);
	
	return ret;
}

void my_load_config(void)
{
    Directive confvalue[][1] = {
		{{"CSGetPassPass", {{PARAM_STRING, PARAM_RELOAD, &CSGetPassPass}}}},
		{{"NSGetPassPass", {{PARAM_STRING, PARAM_RELOAD, &NSGetPassPass}}}}
    };

	if (CSGetPassPass)
		free(CSGetPassPass);
	if (NSGetPassPass)
		free(NSGetPassPass);
	
    moduleGetConfigDirective(confvalue[0]);
	moduleGetConfigDirective(confvalue[1]);
}

/* EOF */
