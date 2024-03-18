#include "group.h"

int ns_do_group(int argc, char **argv)
{
NickAlias *na,
    *na_group;
char *s;
int ctr;
    if( argc < 1 )
        return MOD_CONT;
    else if( (na = findnick(argv[0])) )
    {
        for( ctr = 0; ctr < na->nc->aliases.count; ctr++ )
        {
            na_group = na->nc->aliases.list[ctr];
            
            if( strcmp(argv[0], na_group->nick) && (s = moduleGetData(&na->nc->moduleData, "exempt")) )
            {
                moduleAddData(&na->nc->moduleData, "exempt", s);
                break;
            }
        }
    }
    return MOD_CONT;
}

/* EOF */
