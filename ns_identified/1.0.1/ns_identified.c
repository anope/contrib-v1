#include "module.h" 
#define AUTHOR "Roy"
#define VERSION "1.0.0"

/* ------------------------------------------------------------------------------------
 * Name		: ns_identified
 * Author	: royteeuwen
 * Version	: 1.0
 * Date		: 27th Aug, 2006
 * ------------------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 1, or (at your option) any later version.
 * ------------------------------------------------------------------------------------
 * Tested	: Anope-1.7.17 on Unreal3.2.5
 * ------------------------------------------------------------------------------------
 * Description:
 * With this module you can see if a person is identified or not
 * this can be very handy when having alot of mirc bots on the irc, so that they dont
 * to use raw, because this is not really reliable
 * ------------------------------------------------------------------------------------
 * Changes:
 * -Fixed compiling bug
 * -Fixed bug when typing nothing else then /ns ided
 */


 
/* START CODING */
 
 
int do_id(User *u);
 
int AnopeInit(int argc, char **argv) {
   Command *c;
 
   c = createCommand("ided", do_id, NULL, -1, -1, -1, -1, -1);
   moduleAddCommand(NICKSERV, c, MOD_HEAD);
   alog("ns_identified: Loaded module. '/msg %s IDED \037nick\037'", s_NickServ);

   moduleAddAuthor(AUTHOR);
   moduleAddVersion(VERSION);
   return MOD_CONT;
}
 
int do_id(User *u) {
	char *buffer = moduleGetLastBuffer();
	char *option = myStrGetToken(buffer, ' ', 0);
	User *u2 = finduser(option);

	if (!option) {
		notice(s_NickServ, u->nick, "SYNTAX: \2/msg %s IDED \037nick\037\2", s_NickServ);
		return MOD_CONT;
	}

	if (!(u2 = finduser(option))) 
	{ 
		notice(s_NickServ, u->nick, "This nick: %s does not excist", option); return MOD_CONT; 
	}

	if (nick_identified(u2))
	{
		notice(s_NickServ, u->nick, "This nick is identified");
	} else {
		notice(s_NickServ, u->nick, "This nick is not identified");
	}

	if(option) {
		free(option);
	}
  return MOD_CONT;
}

void AnopeFini(void)
{
	alog("ns_identified: Module unloaded.");
}

 
/* EOF */
