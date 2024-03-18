/* 
 *  ---------------------------------------------------------------
 *  Name:           cs_speak
 *  Description:    Allows users with CA_SAY rights to speak via
 *                  the ChanServ service on current channel.
 *  Author:         Swampy <repton77@googlemail.com>
 *  Date:           03/Jan/2011
 *  Version:        1.0
 *  Commands Added: /msg ChanServ Speak [#Channel] [Message]
 *  ---------------------------------------------------------------
 *  NOTE: This is intended for use on networks without BotServ,
 *  but will work alongside BotServ bot's with no known issues.
 *  ---------------------------------------------------------------
 *  Releases:
 *  1.0   -  First public release to Anope Modules Site.
 * 
 */
#include "module.h"

#define AUTHOR "Swampy"
#define VERSION "cs_speak.c v1.1 by Swampy"

int my_query(User *u);
int myQueryHelp(User *u);
void myChanServHelp(User * u);

int AnopeInit(int argc, char **argv) 
{
	Command *c;
	
	c = createCommand("SPEAK", my_query, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(CHANSERV, c, MOD_HEAD);
	
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	
	moduleSetChanHelp(myChanServHelp);
	moduleAddHelp(c, myQueryHelp);
	
	alog("[CS_SPEAK]: Module loaded.");
	
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("[CS_SPEAK]: Module unloaded.");
}

void myChanServHelp(User * u)
{
	notice(s_ChanServ, u->nick, "    SPEAK     Sends a Channel Message");
}

int myQueryHelp(User *u)
{
	notice(s_ChanServ, u->nick, "Syntax: SPEAK [#Channel] [Message]");
	notice(s_ChanServ, u->nick, "Make's ChanServ Speak On Channel Specified.");
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
		anope_cmd_privmsg(s_ChanServ, chan, "%s", msg);
		alog("%s: Speak on %s \"%s\" (%s!%s@%s) ", s_ChanServ, chan, msg, u->nick, u->vident, u->host);
	}
	}
	
	if (chan)
		free(chan);
	
	if (msg)
		free(msg);
	
	return MOD_CONT;
}
