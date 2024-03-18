#include "activate.h"

int hs_call_activate(User *u)
{
char *buf   = moduleGetLastBuffer(),
    *nick   = myStrGetToken(buf, ' ', 0),
    *reason = myStrGetTokenRemainder(buf, ' ', 1);
    
    hs_do_activate(u, nick, reason, false);
    
    //  TODO: activate grouped nicks with matching open vhost requests
    
    Anope_Free(nick);
    Anope_Free(reason);
    return MOD_CONT;
}

int hs_call_activate_all(User *u)
{
char *reason = moduleGetLastBuffer();

    hs_do_activate_match(u, reason, false, "*");
    
    Anope_Free(reason);
    return MOD_CONT;
}

int hs_do_activate(User *u, char *nick, char *reason, boolean ischan)
{
HostCore *prev,
    *core;
User *u2 = (ischan ? NULL : u);
boolean found = false;
int32 _time = time(NULL);
char *chan  = vh->chan,
    *target = (ischan ? chan : u->nick);
    if( !nick )
    {
        my__announce(target, moduleGetLangString(u2, LNG_ACTIVATE_SYNTAX));
        return MOD_CONT;
    }
    else if( *nick == '!' )
    {
#ifdef SUPPORT_REGEX
        hs_do_activate_match(u, reason, ischan, nick);
#else
        my__announce(target, moduleGetLangString(u2, LNG_REGEX_DISABLED));
#endif
        return MOD_CONT;
    }
    else if( strchr(nick, '*') || strchr(nick, '?') )
    {
#ifdef SUPPORT_WILDCARD
        hs_do_activate_match(u, reason, ischan, nick);
#else
        my__announce(target, moduleGetLangString(u2, LNG_WILDCARD_DISABLED));
#endif
        
        return MOD_CONT;
    }

    prev = findHostCore(request_head, nick, &found);

    if( found )
    {
        if( !prev )
            core = request_head;
        else
            core = prev->next;
        
        my__activate_host_request(u, core->nick, core->vIdent, core->vHost, u->nick, _time, reason);
        
        if( !ischan )
            my__announce(u->nick, moduleGetLangString(u, LNG_ACTIVATED), core->nick);
            
        if( !HasFlag(&vh->flags, DISPLAYMODE) )
            my__announce(chan, coloredText[my__def_language()][GetFlag(&vh->flags, DISPLAYCOLOR)]._activated, core->nick, u->nick, reason ? reason : "");
        else
            my__show_extended(u, chan, nick, core->vIdent, core->vHost, NULL);
        
        alog("Host Request for %s activated by %s (%s)", core->nick, u->nick, reason ? reason : "");
        
        request_head = deleteHostCore(request_head, prev);
    }
    else
        my__announce(target, moduleGetLangString(u2, LNG_NO_REQUEST), nick);
    return MOD_CONT;
}

int hs_do_activate_match(User *u, char *reason, boolean ischan, char *match)
{
HostCore *ptr = request_head;
User *u2 = (ischan ? NULL : u);
char *nick,
    *target = (ischan ? vh->chan : u->nick);
int ctr  = 0,
    mctr = 0;
#ifdef SUPPORT_REGEX
boolean regex = false;
regex_t expr;
char *p;
    if( *match == '!' )
    {
        if( (p = my__checkregex(++match)) )
        {
            my__announce(target, moduleGetLangString(u2, LNG_REGEX_ERROR), match, p);
            return MOD_STOP;
        }

        regex = true;
        regcomp(&expr, match--, (REG_ICASE|REG_EXTENDED|REG_NOSUB));
    }
#endif
    while( ptr )
    {
        
        // don't allow "*"/"!"/ACTALL if there's less
        // than 2 requests (requested by Kaugustino).
        // this check can be circumvented quite
        // easily with wildcard/regex nick matching
        // enabled and thus is useless in those cases.
        // in the end it comes down to choosing responsible
        // people for the job.
        
        if( !ctr && !ptr->next && !strcasecmp(match, "*") )
        {
            my__announce(target, moduleGetLangString(u2, LNG_NOT_ENOUGH_REQUESTS));
            return MOD_CONT;
        }
        
#ifdef SUPPORT_REGEX
        if( (regex && !regexec(&expr, ptr->nick, 0, NULL, 0)) || match_wild_nocase(match, ptr->nick) )
#else
        if( match_wild_nocase(match, ptr->nick) )
#endif
        {
            nick = sstrdup(ptr->nick);
            ptr  = ptr->next;
            
            hs_do_activate(u, nick, (reason ? reason : NULL), ischan);
            
            //  TODO: activate grouped nicks with matching open vhost requests
            
            free(nick);
            mctr++;
        }
        else
            ptr = ptr->next;
        
        ctr++;
    }
    if( !ctr )
        my__announce(target, moduleGetLangString(u2, LNG_NO_REQUESTS));
    else if( !mctr )
        my__announce(target, moduleGetLangString(u2, LNG_NO_MATCHES), match);
    
#ifdef SUPPORT_REGEX
    if( regex )
        regfree(&expr);
#endif
    
    return MOD_CONT;
}

/* EOF */
