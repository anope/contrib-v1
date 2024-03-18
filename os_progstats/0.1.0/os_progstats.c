/*           Anope Rejoin Module
*
*            (C) 2006 Taazoo
* 
*    rejoins specified services to
* main channels, then reads from an file
* as to other channels to join.
*
* Start - 03-06-2006 - Creation of main routines
*         toggle on and off created. manual join
*         and part command added for more control.
*/

/***********************************************************************/

#include <stdio.h>
#include "module.h"
#define AUTHOR "Twitch"
#define VERSION "ID$ os_progstats 2006-05-14 twitch $"


//#######################################################################
//#                                                                     #
//#                      Configuration Section                          #
//#                                                                     #
//#######################################################################


//#######################################################################
//#                                                                     #
//#                   End of Configuration Section                      #
//#                                                                     #
//#######################################################################

static int cmd_progstats  (User *);
static int progstats_help (User *);
static int module_help    (User *);

/**********************************************************************/

int AnopeInit(int argc, char **argv) {
	
	//command setup and initialization
	Command *c;

	c = createCommand("progstats", cmd_progstats, is_services_root, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV,c,MOD_HEAD);
	
	moduleSetOperHelp(module_help);
	moduleAddHelp(c,progstats_help);

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);    	

    moduleSetType(THIRD);	
	
	return 0;
}

void AnopeFini(void)
{

}



/*********************************************************************/
static int cmd_progstats(User *u)
{
	FILE *fp;
	
	char buf[400];
	char cmd[100];
	char **params;


	notice(s_OperServ,u->nick,"Program CPU Statistics");

	snprintf(cmd,(sizeof(cmd) - 1), "ps ux -p %lu | grep \'services\'",(unsigned long) getpid());
	fp = popen(cmd,"r");

	fread(buf,1,sizeof(buf),fp);
	
	notice(s_OperServ,u->nick,"%s",buf);

	pclose(fp);

	return MOD_CONT;
}


/*********************************************************************/

static int progstats_help(User *u) 
{
	if (is_services_root(u)) 
	{
		notice(s_OperServ, u->nick, "Syntax: \02PROGSTATS\02");
		notice(s_OperServ, u->nick, "  ");
		notice(s_OperServ, u->nick, "Displays program cpu usage and statistics to a user");
	}
	return MOD_STOP;
}

/**********************************************************************/

static int module_help(User *u) 
{
	if (is_services_root(u)) 
	{ 
		notice(s_OperServ, u->nick, "     PROGSTATS  Displays current cpu usage");
	}
	return MOD_STOP;
}

