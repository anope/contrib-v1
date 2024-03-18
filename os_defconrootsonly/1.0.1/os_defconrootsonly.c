
#include "module.h"

#define AUTHOR "UnDeRGoD"
#define VERSION "1.0.1"


  /* -----------------------------------------------------------
   * Name  : os_defconrootsonly
   * Author: UnDeRGoD - undergod@netcabo.pt
   * Date  : 15/09/2007
   * -----------------------------------------------------------
   * Incompatibilities: None Known
   * Tested on: SolidIRCd 3.4.x
   * -----------------------------------------------------------
   * Although this version has only been tested on SolidIRCd, it seems has no problem
   * if you run it on other IRCd's.
   *
   * This module will restrict the OperServ DEFCON command to Services Roots.
   *
   * -----------------------------------------------------------
   */


int a_os_defcon(User *u);

/* load the module */

int AnopeInit(int argc, char **argv)
{
   Command *c;
   c = createCommand("DEFCON", a_os_defcon, is_services_root,-1,-1,-1,-1,-1);
   moduleAddCommand(OPERSERV, c, MOD_HEAD);
   alog("os_defconrootsonly (LOADED): OperServ DEFCON is now restricted to Services Roots.");
   moduleAddAuthor(AUTHOR);
   moduleAddVersion(VERSION);
   return MOD_CONT;
}

/* Unload the module */

void AnopeFini(void)
{
alog("os_defconrootsonly (UNLOADED): OperServ DEFCON is now restricted to Services Admins.");
}

int a_os_defcon(User *u)
{
   return MOD_CONT;
}
