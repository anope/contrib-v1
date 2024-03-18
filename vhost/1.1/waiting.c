#include "waiting.h"

int hs_call_waiting(User *u)
{
char *buf = moduleGetLastBuffer(),
    *nick = myStrGetToken(buf, ' ', 0); 
    hs_do_waiting(u, nick, NULL, false);
    
    Anope_Free(nick);
    
    return MOD_CONT;
}

int hs_do_waiting(User *u, char *nick, char *title, boolean ischan)
{
    my__show_list(u, nick, title, ischan, false);

    return MOD_CONT;
}

/* EOF */
