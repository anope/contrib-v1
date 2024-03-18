#include "drop.h"

int ns_do_drop(int argc, char **argv)
{
char *chan = vh->chan;
    if( my__del_host_request(argv[0]) )
    {
        my__announce(chan, moduleGetLangString(NULL, LNG_REQUEST_REMOVED_GONE), argv[0]);
        my__show_list(NULL, "0", NULL, true, false);
    }
    
    my__del_exception_by_nick(argv[0]);
    
    return MOD_CONT;
}

/* EOF */
