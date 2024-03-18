/* Module to unban hostmasks. 
*  Written by Sorcerer (TheKing) on the 31st of March 2009.
*  This file is licensed under the GPLv3.
*  Syntax: /msg chanserv maskunban #channel mask.
*/

#include "module.h"

#define AUTHOR "Sorcerer"
#define VERSION "v1.0.0"

int my_maskunban(User *u);
void myChanServHelp(User * u);

int AnopeInit(int argc, char **argv)
{
	Command *c;

	c = createCommand("MASKUNBAN", my_maskunban, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(CHANSERV, c, MOD_HEAD);

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	moduleSetChanHelp(myChanServHelp);

	alog("[cs_maskunban]: Module loaded.");

	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("[cs_maskunban]: Module unloaded.");
}

void myChanServHelp(User * u)
{
	notice(s_ChanServ, u->nick, "	MASKUNBAN	Unbans a specific hostmask on a channel	");
}

int my_maskunban(User *u)
{
	ChannelInfo *ci;
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	char *mask = myStrGetToken(buf, ' ', 1);

	if (!chan && !mask) {

	notice(s_ChanServ, u->nick, "No parameters specified.");
	notice(s_ChanServ, u->nick, "Syntax: \002maskunban chan mask\002");

	} else {

	if (!(ci = cs_findchan(chan))) {
	notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
	
	} else if (ci->flags & CI_VERBOTEN) {

	notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);

	} else {

	if (!check_access(u, ci, CA_UNBAN) && !is_services_admin(u)) {
	notice_lang(s_ChanServ, u, ACCESS_DENIED);

	} else {

	anope_cmd_mode(whosends(ci), ci->name, "-b %s", mask);

	 }
	 return MOD_CONT;
	}
       }
       return MOD_CONT; 
     }
     
