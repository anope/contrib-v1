#include "module.h"
#define AUTHOR "Switch"
#define VERSION "1.0.2"

  /* -----------------------------------------------------------
   * Name  : ns_clearaccessonreg 
   * Author: Switch <switch__AT__chat-libre.net>
   * Date  : 15/04/2004
   * Update: 07/01/2006
   * -----------------------------------------------------------
   * Functions: cb_ns_clearaccessonreg
   * Limitations: None Known
   * Tested: Unreal(3.2)
   * -----------------------------------------------------------
   * This version has been tested on Unreal,
   * but it should work with others ircds.
   *
   * This module will clear the access list when someone register
   * his nick, to prevent nick stoll because someone have same host. 
   *
   * This module has no configurable options.
   *
   * Versions info :
   * 1.0.0 : Initial release.
   * 1.0.1 : Fixed a bug that could crash Anope.
   * 1.0.2 : Fixed a bug that prevent the module to work if email confirmation is used.
   * 
   * -----------------------------------------------------------
   */
  
/* -------------------------------------------------------------- */
/* DO NOT EDIT BELOW THIS LINE UNLESS YOU KNOW WHAT YOU ARE DOING */
/* -------------------------------------------------------------- */


int cb_ns_clearaccessonreg(User *u);

int AnopeInit(int argc, char **argv)
{
   Command *c;
   int err;
   c = createCommand("REGISTER", cb_ns_clearaccessonreg, NULL,-1,-1,-1,-1,-1);
   err = moduleAddCommand(NICKSERV, c, MOD_TAIL);
   c = createCommand("CONFIRM", cb_ns_clearaccessonreg, NULL,-1,-1,-1,-1,-1);
   err = err || moduleAddCommand(NICKSERV, c, MOD_TAIL);

   if (err == MOD_ERR_OK)
   {
      alog("[ns_clearaccessonreg] %s REGISTER and CONFIRM commands modified to clear access list after registration.", s_NickServ);
      moduleAddAuthor(AUTHOR);
      moduleAddVersion(VERSION);
      return MOD_CONT;
   }
   else
   {
      alog("[ns_clearaccessonreg] %s REGISTER or CONFIRM command can not be modified, module unloading.", s_NickServ);
      return MOD_STOP;
   }
}

int cb_ns_clearaccessonreg(User *u)
{
   NickAlias *na;
   na = findnick(u->nick);
   if (na!=NULL)
   {
      free(na->nc->access);
      na->nc->access = NULL;
      na->nc->accesscount = 0;
   }
   return MOD_CONT;
}
