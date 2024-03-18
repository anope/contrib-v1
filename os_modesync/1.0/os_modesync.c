/* Module to sync modes on all registered channels with mlock.
*  Original os_massmode code written by Sorcerer (TheKing)
*  Modified by Freelancer
*/

#include "module.h"

#define AUTHOR "Freelancer"
#define VERSION "1.0"

int my_modesync(User *u);
int myModeSyncHelp(User * u);
void myOperServHelp(User * u);

int AnopeInit(int argc, char **argv) 
{
	Command *c;
	
	c = createCommand("MODESYNC", my_modesync, is_services_admin, -1, -1, -1, -1, -1 -1);
	moduleAddCommand(OPERSERV, c, MOD_HEAD);
	
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	
	moduleSetOperHelp(myOperServHelp);
	moduleAddHelp(c, myModeSyncHelp);
	
	alog("[os_modesync]: Module loaded.");
	
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("[os_modesync]: Module unloaded.");
}

void myOperServHelp(User * u)
{
	if (is_services_admin(u)) {
		notice(s_OperServ, u->nick, "	MODESYNC	Resets all modes on all registered channels with mlock.");	
	}
}

int myModeSyncHelp(User * u)
{
	if (is_services_admin(u)) {
		notice(s_OperServ, u->nick, "Syntax: /002modesync [Third party modes]/002");
		notice(s_OperServ, u->nick, "This will allow you to sync mode(s) on all registered channels with mlock.");
	}
	return MOD_CONT;
}

int my_modesync(User *u)
{
	char buf2[BUFSIZE];
	char *buf = moduleGetLastBuffer();
	char *modes = myStrGetToken(buf, ' ', 0);

	snprintf(buf2, BUFSIZE - 1, "-cfijklmnprstzACGMKLNOQRSTVu%s", modes);
	do_mass_mode(buf2);
	if (modes) {
		alog("[os_modesync]: %s used the MODESYNC command. Third party modes: %s", u->nick, modes);
		anope_cmd_global(s_OperServ, "\2%s\2 used MODESYNC with Third party modes: %s", u->nick, modes);
		free(modes);
	} else {
		alog("[os_modesync]: %s used the MODESYNC command.", u->nick);
		anope_cmd_global(s_OperServ, "\2%s\2 used MODESYNC", u->nick);
	}
	return MOD_CONT;
}