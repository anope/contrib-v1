#include "cmd_meow.h"

int do_meow(User *u)
{
char *chan = vh->chan;
    
    my__announce(chan, "MEOW!");
    
    return MOD_STOP;
}

/* EOF */
