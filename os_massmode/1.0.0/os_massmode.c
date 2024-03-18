/* Module to set modes on all registered channels.
*  Written by Sorcerer (TheKing) on the 1st of April 2009. (YAY APRIL FOOLS)!
*  This file is licensed under the GPLv3.
*/

#include "module.h"

#define AUTHOR "Sorcerer"
#define VERSION "1.0.0"

int my_massmode(User *u);
void myOperServHelp(User * u);

int AnopeInit(int argc, char **argv) 
{
	Command *c;
	
	c = createCommand("MASSMODE", my_massmode, is_services_admin, -1, -1, -1, -1, -1 -1);
	moduleAddCommand(OPERSERV, c, MOD_HEAD);
	
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	
	moduleSetOperHelp(myOperServHelp);
	
	alog("[os_massmode]: Module loaded.");
	
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("[os_massmode]: Module unloaded.");
}

void myOperServHelp(User * u)
{
	if (is_services_admin(u)) {
	notice(s_OperServ, u->nick, "	MASSMODE	Set's a mode on all registered channels	");	
   }
}

int my_massmode(User *u)
{
	char *buf = moduleGetLastBuffer();
	char *modes = myStrGetToken(buf, ' ', 0);
	
	if (!modes) {
	notice(s_OperServ, u->nick, "You did not specify the modes.");
	notice(s_OperServ, u->nick, "Syntax: \002massmode +(-)modes");
	
	} else {
	
	if ((modes[0] != '+') && (modes[0] != '-')) {
	notice(s_OperServ, u->nick, "Invalid mode prefix, only + and - is allowed.");
	notice(s_OperServ, u->nick, "Syntax: \002massmode +(-)modes");

	} else {
	
	do_mass_mode(modes);
	alog("[os_massmode]: %s used the MASSMODE command. Modes: /002%s/002", u->nick, modes);
	return MOD_CONT;
	
    }
  }
  return MOD_CONT;
}	