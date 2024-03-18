#include "module.h"

#define AUTHOR "Thomas Edwards (TMFKSOFT)"
#define VERSION "1.0.0"
#define TOS "<TOS Link>"

/* Show TOS Link */
int tos(User *u);

int AnopeInit(int argc, char **argv) {
    /* We create the command we're going to use */
    Command *c;
    c = createCommand("tos", tos, NULL, -1, -1, -1, -1, -1);

    /* We add the command to HelpServ and log it */
    alog("tos: Add command 'tos' status: %d", moduleAddCommand(HELPSERV, c, MOD_HEAD));

    /* We tell Anope who we are and what version this module is */
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);

    /* Loading succeeded */
    return MOD_CONT;
}

void AnopeFini(void) {
    /* Nothing to clean up */
}

int tos(User *u) {
    /* Return TOS Link */
    notice(s_HelpServ, u->nick, "\002%s\002 you can find the Terms Of Service at \002%s\002", u->nick, TOS);

    /* Halt processing */
    return MOD_STOP;
}

/* EOF */