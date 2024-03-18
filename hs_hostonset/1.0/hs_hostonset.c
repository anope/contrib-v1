#include "module.h"
#define AUTHOR "Roy"
#define VERSION "1.0"

/* ------------------------------------------------------------------------------------
 * Name		: hs_hostonset
 * Author	: royteeuwen
 * Version	: 1.0
 * Date		: 15th Nov, 2006
 * ------------------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 1, or (at your option) any later version.
 * ------------------------------------------------------------------------------------
 * Tested	: Anope-1.7.18 on Unreal3.2.5
 * ------------------------------------------------------------------------------------
 * Description:
 * This module is build on hs_set.c, When an irc operator types /hs set nick hostmask
 * this module will automaticly put the vhost on for the given nick
 * ------------------------------------------------------------------------------------
 */

/* ---------------------------------------------------------------------- */
/* DO NOT EDIT BELOW THIS LINE UNLESS YOU KNOW WHAT YOU ARE DOING         */
/* ---------------------------------------------------------------------- */


int do_setonhost(User *u);

int AnopeInit(int argc, char **argv)
{
    Command *c;
    c = createCommand("SET", do_setonhost, NULL, -1, -1, -1, -1,
                      -1);
    moduleAddCommand(HOSTSERV, c, MOD_TAIL);
    moduleAddAuthor(AUTHOR);

    moduleAddVersion(VERSION);
    alog("hs_setonhost.c v%s by %s loaded", VERSION, AUTHOR);
    return MOD_CONT;
}

int do_setonhost(User *u)
{
      NickAlias *na;
	char *buffer = moduleGetLastBuffer();
 	char *option = myStrGetToken(buffer, ' ', 0);
 	User *u2 = NULL;

	if (!option) {
		return MOD_CONT;
      }
	u2 = finduser(option);

	char *vHost;
      char *vIdent = NULL;
	if (!(u2 = finduser(option))) {
		if(option) free(option);
		return MOD_STOP;
	}

      if ((na = findnick(u2->nick))) {
        if (na->status & NS_IDENTIFIED) {
            vHost = getvHost(u2->nick);
            vIdent = getvIdent(u2->nick);
		if (!vHost) {
			if (option) free(option);
			return MOD_STOP;
		}
                if (vIdent) {
                    notice_lang(s_HostServ, u2, HOST_IDENT_ACTIVATED, vIdent, vHost);
                } else {
                    notice_lang(s_HostServ, u2, HOST_ACTIVATED, vHost);
                }
                anope_cmd_vhost_on(u2->nick, vIdent, vHost); 
                if (ircd->vhost) {
                    u2->vhost = sstrdup(vHost);
                }
                if (ircd->vident) {
                    if (vIdent)
                        u2->vident = sstrdup(vIdent);
                }
                set_lastmask(u2);           
          } 
        } 
	if(option) free(option);
    return MOD_CONT;
}

void AnopeFini(void)
{
	alog("hs_hostonset: Module unloaded.");
}

/* EOF */ 
