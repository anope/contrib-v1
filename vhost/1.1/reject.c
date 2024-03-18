#include "reject.h"

int hs_call_reject(User *u)
{
char *buf   = moduleGetLastBuffer(),
    *nick   = myStrGetToken(buf, ' ', 0),
    *reason = myStrGetTokenRemainder(buf, ' ', 1);
    hs_do_reject(u, nick, (reason ? reason : NULL), false);

    Anope_Free(nick);
    Anope_Free(reason);
    return MOD_CONT;
}

int hs_call_reject_all(User *u)
{
char *reason = moduleGetLastBuffer();
    hs_do_reject_match(u, reason, false, "*");
    
    return MOD_CONT;
}

int hs_do_reject(User *u, char *nick, char *reason, boolean ischan)
{
User *u2 = (ischan ? NULL : u),
    *u3;
HostCore *prev,
    *core;
boolean found = false;
char *chan   = vh->chan,
    *target  = (ischan ? vh->chan : u->nick);
char rejection_time[33+USERMAX+HOSTMAX+1]; // 4*8 uint32 + 1; HOSTMASK is +1 already, therefore only +1 at the end
    if( !nick )
    {
        my__announce(target, moduleGetLangString(u2, LNG_REJECT_SYNTAX));

        return MOD_CONT;
    }
    else if( *nick == '!' )
    {
#ifdef SUPPORT_REGEX
        hs_do_reject_match(u, reason, ischan, nick);
#else
        my__announce(target, moduleGetLangString(u2, LNG_REGEX_DISABLED));
#endif
        return MOD_CONT;
    }
    else if( strchr(nick, '*') || strchr(nick, '?') )
    {
#ifdef SUPPORT_WILDCARD
        hs_do_reject_match(u, reason, ischan, nick);
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
        
        if( (u3 = finduser(nick)) )
        {
            if( core->vIdent )
            {
                sprintf(rejection_time, "%X|%s:%s", (uint32) time(NULL), core->vHost, core->vIdent);
                my__announce(u3->nick, moduleGetLangString(u3, LNG_HOST_IDENT_REJECTED), core->vIdent, core->vHost, reason ? reason : moduleGetLangString(u3, LNG_KEY_NONE));
            }
            else
            {
                sprintf(rejection_time, "%X|%s", (uint32) time(NULL), core->vHost);
                my__announce(u3->nick, moduleGetLangString(u3, LNG_HOST_REJECTED), core->vHost, reason ? reason : moduleGetLangString(u3, LNG_KEY_NONE));
            }
            
            moduleAddData(&u3->na->nc->moduleData, "time", rejection_time);
        }
        else if( HasFlag(&vh->flags, MEMOUSER) )
        {
            if( reason )
                my__send_memo(u, core->nick, 2, LNG_REJECT_MEMO_REASON, reason);
            else
                my__send_memo(u, core->nick, 2, LNG_REJECT_MEMO);
        }
        
        if( !ischan )
            my__announce(u->nick, moduleGetLangString(u, LNG_REJECTED), core->nick);

        if( !HasFlag(&vh->flags, DISPLAYMODE) )
            my__announce(chan, coloredText[my__def_language()][GetFlag(&vh->flags, DISPLAYCOLOR)]._rejected, core->nick, u->nick, reason ? reason : "");
        else
            my__show_extended(u, chan, nick, core->vIdent, core->vHost, reason ? reason : "");
            
        alog("Host Request for %s rejected by %s (%s)", core->nick, u->nick, reason ? reason : "");
        
        request_head = deleteHostCore(request_head, prev);
    }
    else
        my__announce(target, moduleGetLangString(u2, LNG_NO_REQUEST), nick);

    return MOD_CONT;
}

int hs_do_reject_match(User *u, char *reason, boolean ischan, char *match)
{
User *u2 = (ischan ? NULL : u);
HostCore *ptr = request_head;
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
        
         // don't allow "*"/"!"/REJALL if there's less
         //  than 2 requests (requested by Kaugustino).
         //  this check can be circumvented quite
         //  easily with wildcard/regex nick matching
         //  enabled and thus useless in those cases
        
#if !defined(SUPPORT_WILDCARD) && !defined(SUPPORT_REGEX)
        if( !ctr && !ptr->next && !strcasecmp(match, "*") )
        {
            my__announce(target, moduleGetLangString(u2, LNG_NOT_ENOUGH_REQUESTS));
            
            return MOD_CONT;
        }
#endif
        
#ifdef SUPPORT_REGEX
        if( (regex && !regexec(&expr, ptr->nick, 0, NULL, 0)) || match_wild_nocase(match, ptr->nick) )
#else
        if( match_wild_nocase(match, ptr->nick) )
#endif
        {
            nick = sstrdup(ptr->nick);
            ptr  = ptr->next;
            
            hs_do_reject(u, nick, (reason ? reason : NULL), ischan);
            
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
