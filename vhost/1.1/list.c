#include "list.h"

int hs_call_list_out(User *u)
{
    if( hs_do_list_out(u, false) )
        return MOD_STOP;
    else
        return MOD_CONT;
}

int hs_do_list_out(User *u, boolean ischan)
{
HostCore *current;
struct tm *tm;
unsigned short int ctr = 1,
    display_ctr        = 0;
char *key = moduleGetLastBuffer(),
    *nick = u->nick;
char buf[BUFSIZE];
    if( !key || stricmp(key, "+req") )
        return 0;
    
    for( current = request_head; current; current = current->next )
    {
        if( display_ctr < NSListMax )
        {
            tm = localtime(&current->time);
            strftime(buf, sizeof(buf), getstring(NULL, STRFTIME_DATE_TIME_FORMAT), tm);
            
            if( current->vIdent )
                my__announce_lang(u, nick, HOST_IDENT_ENTRY, ctr, current->nick, current->vIdent, current->vHost, current->creator, buf);
            else
                my__announce_lang(u, nick, HOST_ENTRY, ctr, current->nick, current->vHost, current->creator, buf);
                
            display_ctr++;
        }
        ctr++;
    }
    my__announce(nick, moduleGetLangString(u, LNG_VHOST_REQUEST_DISPLAYED), display_ctr, (display_ctr == 1 ? " " : "s "), --ctr);
    
    return 1;
}

/* EOF */
