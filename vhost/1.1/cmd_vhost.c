#include "cmd_vhost.h"

int do_vhost(User *u)
{
char *buf = moduleGetLastBuffer(),
    *cmd,
    *param = NULL,
    *chan  = vh->chan,
    *nick  = u->nick;
unsigned short int ctr = 0;
    do
    {
        cmd = myStrGetToken(buf, ' ', ctr);
        ctr++;
        
        if( cmd && *cmd == '!' )
        {
            free(cmd);
            cmd = NULL;
        }
    } while( !cmd && ctr < 2 );
    if( !cmd || !strcasecmp(cmd, "help") )
    {
        if( (cmd = myStrGetToken(buf, ' ', ctr)) )
        {
            if( !strcasecmp(cmd, "w") || !strcasecmp(cmd, "waiting") )
                my__announce(nick, moduleGetLangString(u, LNG_WAITING_SYNTAX));
            else if( !strcasecmp(cmd, "a") || !strcasecmp(cmd, "activate") )
                my__announce(nick, moduleGetLangString(u, LNG_ACTIVATE_SYNTAX));
            else if( !strcasecmp(cmd, "r") || !strcasecmp(cmd, "reject") )
                my__announce(nick, moduleGetLangString(u, LNG_REJECT_SYNTAX));
            else if( !strcasecmp(cmd, "rejall") )
                my__announce(nick, moduleGetLangString(u, LNG_REJALL_SYNTAX));
            else if( !strcasecmp(cmd, "actall") )
                my__announce(nick, moduleGetLangString(u, LNG_ACTALL_SYNTAX));
            else
                my__announce(nick, moduleGetLangString(u, LNG_VHOST_SYNTAX));
        }
        else
            my__announce(nick, moduleGetLangString(u, LNG_VHOST_SYNTAX));
        
        return MOD_CONT;
    }

    if( !is_host_setter(u) )
        my__announce_lang(u, chan, HOST_DENIED);
    else if( !strcasecmp(cmd, "waiting") || !strcasecmp(cmd, "w") )
    {
        cmd = myStrGetToken(buf, ' ', ctr);
        
        hs_do_waiting(u, cmd, "Requested", true);
    }
    else if( !strcasecmp(cmd, "activate") || !strcasecmp(cmd, "a") )
    {
        cmd   = myStrGetToken(buf, ' ', ctr++);
        param = myStrGetTokenRemainder(buf, ' ', ctr);

        hs_do_activate(u, cmd, param, true);
    }
    else if( !strcasecmp(cmd, "reject") || !strcasecmp(cmd, "r") )
    {
        cmd   = myStrGetToken(buf, ' ', ctr++);
        param = myStrGetTokenRemainder(buf, ' ', ctr);
            
        hs_do_reject(u, cmd, param, true);
    }
    else if( !strcasecmp(cmd, "rejall") ) 
    {
        param = myStrGetTokenRemainder(buf, ' ', ctr);
            
        hs_do_reject_match(u, param, true, "*");
    }
    else if (!strcasecmp(cmd, "actall") )
    {
        param = myStrGetTokenRemainder(buf, ' ', ctr);
        
        hs_do_activate_match(u, param, true, "*");
    }
    else
        my__announce_lang(u, chan, UNKNOWN_COMMAND, cmd);
    
    Anope_Free(cmd);
    Anope_Free(param);
    
    return MOD_STOP;
}

/* EOF */
