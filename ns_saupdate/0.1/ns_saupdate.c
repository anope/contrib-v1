#include "module.h"

/*
 * ns_saupdate.c
 * Author: darkex (jzdce1@gmail.com)
 * Version 0.1, initial release.
 *
 * Update's someone else's status in one hit,
 * just like if you were issuing /ns update.
 *
 */

#define AUTHOR "darkex"
#define VERSION "0.1"

int my_ns_saupdate(User *u);
int ns_saupdate_help(User *u);
void saupdate_help(User *u);

int AnopeInit(int argc, char **argv)
{
    Command *c;
    c = createCommand("SAUPDATE", my_ns_saupdate, is_services_admin, -1, -1, -1, -1, -1);
    moduleAddCommand(NICKSERV, c, MOD_TAIL);
    
    moduleAddAdminHelp(c, ns_saupdate_help);
    moduleAddRootHelp(c, ns_saupdate_help);
    moduleSetNickHelp(saupdate_help);
    
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    
    return MOD_CONT;
}

void AnopeFini(void)
{

}

int my_ns_saupdate(User *u)
{
    NickAlias *na;
    char *buf = moduleGetLastBuffer();
     char *unick = myStrGetToken(buf, ' ', 0);
    User *u2 = NULL;
    
    if (!unick) {
        notice(s_NickServ, u->nick, "Syntax: \002SAUPDATE \037nick\037\002");
        notice(s_NickServ, u->nick, "\002/msg %s HELP SAUPDATE\002 for more information.", s_NickServ);
        return MOD_CONT;
    }
    u2 = finduser(unick);
    if (!(u2 = finduser(unick))) {
        notice_lang(s_NickServ, u, NICK_X_NOT_IN_USE, unick);
    } else if (!nick_identified(u2)) {
        notice(s_NickServ, u->nick, "Nick \002%s\002 isn't identified.", unick);
    } else {
        na = u2->na;
        do_setmodes(u2);
        check_memos(u2);
        if (na->last_realname)
            free(na->last_realname);
        na->last_realname = sstrdup(u2->realname);
        na->status |= NS_IDENTIFIED;
        na->last_seen = time(NULL);
        if (ircd->vhost) {
            do_on_id(u2);
        }
        notice(s_NickServ, u->nick, "Status for \002%s\002 updated (memos, vhost, chmodes, flags).", unick);
    }
    
    if (unick) free(unick);
    
    return MOD_CONT;
}

int ns_saupdate_help(User *u)
{
    notice(s_NickServ, u->nick, "Syntax: \002SAUPDATE \037nick\037\002");
    notice(s_NickServ, u->nick, " ");
    notice(s_NickServ, u->nick, "Updates another person's status.");
    return MOD_CONT;
}

void saupdate_help(User *u)
{
    if (is_services_admin(u)) {
        notice(s_NickServ, u->nick, "    SAUPDATE   Updates someone else's status");
    }
}