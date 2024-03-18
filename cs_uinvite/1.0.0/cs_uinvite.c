/* Module to allow ChanServ to invite other users instead of yourself.
*  Syntax: /msg chanserv uinvite user channel.
*  Written by Sorcerer (TheKing) on the 11th of April, 2009.
*  This file is licensed under the GPLv2.
*/

#include "module.h"

#define AUTHOR "Sorcerer"
#define VERSION "1.0.0"

int myUInviteHelp(User * u);
int my_uinvite(User *u);
void myChanServHelp(User * u);

int AnopeInit(int argc, char **argv)
{
	Command *c;
	
	c = createCommand("UINVITE", my_uinvite, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV, c, MOD_HEAD);
	
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	
	moduleAddHelp(c, myUInviteHelp);
	moduleSetChanHelp(myChanServHelp);
	
	alog("[cs_uinvite]: Module loaded.");
	
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("[cs_uinvite]: Module unloaded.");
}

int myUInviteHelp(User *u)
{
	notice(s_ChanServ, u->nick, "Syntax: \002uinvite user channel\002");
	notice(s_ChanServ, u->nick, "This module allows you to invite a user");
	notice(s_ChanServ, u->nick, "to a channel as long as you have INVITE");
	notice(s_ChanServ, u->nick, "access to the channel. Unlike the /cs invite");
	notice(s_ChanServ, u->nick, "command.");
	return MOD_CONT;
}

void myChanServHelp(User * u)
{
	notice(s_ChanServ, u->nick, "	UINVITE	Invites a user to a channel	");
	return;
}

int my_uinvite(User *u)
{
	Channel *c;
	ChannelInfo *ci;

	char *buf = moduleGetLastBuffer();
	char *user = myStrGetToken(buf, ' ', 0);
	char *chan = myStrGetToken(buf, ' ', 1);
	
	if (!user) {
	
		notice(s_ChanServ, u->nick, "Missing user variable.");
		notice(s_ChanServ, u->nick, "Syntax: \002uinvite user channel\002");
		
	} else if (!chan) {
	
		notice(s_ChanServ, u->nick, "Missing channel variable.");
		notice(s_ChanServ, u->nick, "Syntax: \002uinvite user channel\002");

	} else if (!(c = findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_IN_USE, chan);
    	} else if (!(ci = c->ci)) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    	} else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    	} else if (ci->flags & CI_SUSPENDED) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    	} else if (!u || !check_access(u, ci, CA_INVITE)) {
	notice_lang(s_ChanServ, u, PERMISSION_DENIED);
    } else {
        
	anope_cmd_invite(whosends(ci), chan, user);
    	free(user);
    	free(chan);
    	return MOD_CONT;
    
    }
    return MOD_CONT;
 }