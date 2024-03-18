#include "module.h"

/* 
 * cs_unblock.c
 * Author: darkex (jzdce1@gmail.com)
 * 
 * This module allows channel founders to remove modes -ilk
 * so they are not caught out of their own channel if locked out.
 *
 * Best used on other IRCds, considering that Unreal's invite can
 * override all modes except +R and +z. Can be a good substitute when
 * going without noisy /NOTICEs.
 * 
 * This is not a substitute to UNBAN, which should be used first.
 * 
 */

#define AUTHOR "darkex"
#define VERSION "0.1"

int my_cs_unblock(User *u);
int cs_unblock_help(User *u);
void unblock_help(User *u);

int AnopeInit(int argc, char **argv)
{
    Command *c;
    c = createCommand("UNBLOCK", my_cs_unblock, NULL, -1, -1, -1, -1, -1);
    moduleAddCommand(CHANSERV, c, MOD_TAIL);
    
    moduleAddHelp(c, cs_unblock_help);
    moduleSetChanHelp(unblock_help);
    
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    
    return MOD_CONT;
}

void AnopeFini(void) {
}

int my_cs_unblock(User *u)
{
    ChannelInfo *ci;
    char *buf = moduleGetLastBuffer();
    char *chan = myStrGetToken(buf, ' ', 0);
    if (!chan) {
        notice(s_ChanServ, u->nick, "Syntax: \002UNBLOCK \037channel\037\002");
        notice(s_ChanServ, u->nick, "\002/msg %s HELP UNBLOCK\002 for more information.", s_ChanServ);
    } else if (!nick_identified(u)) {
        notice_lang(s_ChanServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
    } else if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else if ((ci->flags & CI_VERBOTEN) || (ci->flags & CI_SUSPENDED)) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    } else if (!is_founder(u, ci) && !is_services_admin(u)) {
        notice_lang(s_ChanServ, u, PERMISSION_DENIED);
        if (LogChannel) alog("Access denied for %s with service %s and command UNBLOCK on %s", u->nick, s_ChanServ, chan);
    } else {
        anope_cmd_mode(s_ChanServ, chan, "-ikl * *");
        notice(s_ChanServ, u->nick, "Modes preventing you from joining \002%s\002 have been removed.", chan);
        if (is_services_admin(u) && !is_founder(u, ci)) alog("%s used %s UNBLOCK on %s with services admin privileges", u->nick, s_ChanServ, chan);
    }
    if (chan) free(chan);
    
    return MOD_CONT;
}

int cs_unblock_help(User *u)
{
    notice(s_ChanServ, u->nick, "Syntax: \002UNBLOCK \037channel\037\002");
    notice(s_ChanServ, u->nick, " ");
    notice(s_ChanServ, u->nick, "This command allows channel founders to remove the modes");
    notice(s_ChanServ, u->nick, "\002ikl\002 that prevent them from joining their channel.");
    notice(s_ChanServ, u->nick, "This command \002will not\002 remove matching bans, use \002UNBAN\002");
    notice(s_ChanServ, u->nick, "instead.");
    if (is_services_admin(u)) notice(s_ChanServ, u->nick, "\002Services administrators\002 can use unblock on any channel.");
    
    return MOD_CONT;
}

void unblock_help(User *u)
{
    notice(s_ChanServ, u->nick, "    UNBLOCK   Removes modes preventing founders from joining");
}