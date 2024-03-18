/* Module to query a user using nickserv under a SRA's request.
*  Requested by midnight_tiger.
*  Written by Sorcerer (TheKing) on the 4th of April 2009.
*  This file is licensed under the GPLv3.
*/

#include "module.h"

#define AUTHOR "TheKing"
#define VERSION "1.0.0"

int my_query(User *u);
int myQueryHelp(User *u);
void myNickServHelp(User * u);

int AnopeInit(int argc, char **argv) 
{
	Command *c;
	
	c = createCommand("QUERY", my_query, is_services_root, -1, -1, -1, -1, -1);
	moduleAddCommand(NICKSERV, c, MOD_HEAD);
	
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	
	moduleSetNickHelp(myNickServHelp);
	moduleAddHelp(c, myQueryHelp);
	
	alog("[ns_query]: Module loaded.");
	
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("[ns_query]: Module unloaded.");
}

void myNickServHelp(User * u)
{
	if (is_services_root(u)) {
	notice(s_NickServ, u->nick, "QUERY	Sends a user a PRIVMSG	");
      }
}

int myQueryHelp(User *u)
{
	if (is_services_root(u)) {
	notice(s_NickServ, u->nick, "This will allow you to query (PRIVMSG) a user using NickServ.");
	notice(s_NickServ, u->nick, "Syntax: \002query user message\002");
      }
       return MOD_CONT;
}

int my_query(User *u)
{
	char *buf = moduleGetLastBuffer();
	char *user = myStrGetToken(buf, ' ', 0);
	char *msg = myStrGetTokenRemainder(buf, ' ', 1);
	
	if (!user) {
	notice(s_NickServ, u->nick, "No user specified.");
	notice(s_NickServ, u->nick, "Syntax: \002query user message\002");
	
	} else {
	
	if (!msg) {
	notice(s_NickServ, u->nick, "No message specified.");
	notice(s_NickServ, u->nick, "Syntax: \002query user message\002");
	
	} else if (msg && user) {
	
	anope_cmd_privmsg(s_NickServ, user, "%s", msg);
	alog("[ns_query]: %s used the QUERY command. (User queried: %s), (Message: %s)", u->nick, user, msg);
	free(user);
	free(msg);
	return MOD_CONT;
    }
	return MOD_CONT;
  }	
	return MOD_CONT;
}
