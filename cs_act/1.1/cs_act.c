/* 
 *  ---------------------------------------------------------------
 *  Name:           cs_act
 *  Description:    Allows users with CA_SAY rights to act via
 *                  the ChanServ service on current channel.
 *  Author:         gurumadmat <gurumadmat@gmail.com>
 *  Date:           04/Oct/2011
 *  Version:        1.0
 *  Commands Added: /msg ChanServ Act [#Channel] [Message]
 *  ---------------------------------------------------------------
 *  NOTE: This is intended for use on networks without BotServ,
 *  but will work alongside BotServ bot's with no known issues.
 *  ---------------------------------------------------------------
 *  Releases:
 *  1.0   -  First public release to Anope Modules Site.
 * 
 */
#include "module.h"

#define AUTHOR "GuruMadMat"
#define VERSION "cs_act.c v1.0 by GuruMadMat"

int my_query(User *u);
int myQueryHelp(User *u);
void myChanServHelp(User * u);

int AnopeInit(int argc, char **argv) 
{
	Command *c;
	
	c = createCommand("ACT", my_query, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(CHANSERV, c, MOD_HEAD);
	
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	
	moduleSetChanHelp(myChanServHelp);
	moduleAddHelp(c, myQueryHelp);
	
	alog("[CS_ACT]: Module loaded.");
	
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("[CS_ACT]: Module unloaded.");
}

void myChanServHelp(User * u)
{
	notice(s_ChanServ, u->nick, "    ACT     Sends a Channel Message");
}

int myQueryHelp(User *u)
{
	notice(s_ChanServ, u->nick, "Syntax: ACT [#Channel] [Message]");
	notice(s_ChanServ, u->nick, "Make's ChanServ Act On Channel Specified.");
	notice(s_ChanServ, u->nick, "Limited to users who have CA_SAY permission.");
    return MOD_CONT;
}

int my_query(User *u)
{
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	char *msg = myStrGetTokenRemainder(buf, ' ', 1);

	ChannelInfo *ci;
	
	if ((!chan) || (!msg)) {
		notice(s_ChanServ, u->nick, "No Channel or Message Specified!");
	} else if (msg && chan) {
	if (!(ci = cs_findchan(chan))) {
		notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
	} else if (!nick_identified(u)) {
		notice_lang(s_ChanServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
	} else if ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED)) {
		notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
	} else if (!is_services_admin(u) && !check_access(u, ci, CA_SAY)) {
		notice_lang(s_ChanServ, u, PERMISSION_DENIED);
	} else {
		anope_cmd_action(s_ChanServ, chan, "%s", msg);
		alog("%s: Act on %s \"%s\" (%s!%s@%s) ", s_ChanServ, chan, msg, u->nick, u->vident, u->host);
		
	}
	}
	
	if (chan)
		free(chan);
	
	if (msg)
		free(msg);
	
	return MOD_CONT;
}
