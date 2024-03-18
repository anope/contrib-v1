/***************************************************************************************/
/* Anope Module : os_amode.c : v1.0                                                    */  
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
#define VERSION "1.0.0"

int m_amode(User * u);                        /* OperServ Command function           */
void Module_Help_OPERSERV_AMODE(User *u);
int Module_Help_OPERSERV_AMODE_FULL(User *u);

/***************************************************************************************/

int AnopeInit(int argc, char **argv) 
{
 	Command *c;
	c = createCommand("AMODE",m_amode, is_services_admin,-1,-1,-1,-1,-1);
   moduleAddCommand(OPERSERV,c,MOD_HEAD);
 
    moduleAddHelp(c, Module_Help_OPERSERV_AMODE_FULL);
    moduleSetHelpHelp(Module_Help_OPERSERV_AMODE);

    alog("Loading module this will add AMODE to %s",s_OperServ);

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);

    return MOD_CONT;
}

/***************************************************************************************/

void AnopeFini(void) {
  alog("Unloading os_amode.so");
}

/***************************************************************************************/

void Module_Help_OPERSERV_AMODE(User *u)
{
   notice(s_HelpServ, u->nick, "    AMODE    Set mode on all channels.");
   return;
}

/***************************************************************************************/

int Module_Help_OPERSERV_AMODE_FULL(User *u)
{
        notice(s_HelpServ, u->nick, " Syntax: AMODE <mode>");
        notice(s_HelpServ, u->nick, "   ");
        notice(s_HelpServ, u->nick, " Set a mode on all channels");
        return MOD_CONT;
}

/***************************************************************************************/

int m_amode(User * u)
{
 char *temp = moduleGetLastBuffer();
 char *modes = myStrGetTokenRemainder(temp,' ',0);

 if (!modes) {
  notice(s_OperServ,u->nick,"Syntax error : No Modes");
  return MOD_CONT;
 }
 notice(s_OperServ,u->nick,"Setting all channels to : %s",modes);
 	/* Added global by request - katsklaw */
 anope_cmd_global(s_OperServ,"\2%s\2 set all channels to : \2%s\2", u->nick, modes);
 do_mass_mode(modes);
 if (modes) {
	free(modes);
 }
 return MOD_CONT;
}



