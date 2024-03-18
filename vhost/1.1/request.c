#include "request.h"

int hs_call_request(User *u)
{
char *buf = moduleGetLastBuffer(),
    *nick = u->nick,
    *request;
    if( !(request = myStrGetToken(buf, ' ', 0)) )
    {
        my__announce(nick, moduleGetLangString(u, LNG_REQUEST_SYNTAX));
        return MOD_CONT;
    }

    hs_do_request(u, nick, request, false);

    free(request);
    return MOD_CONT;
}

int hs_match_request(char *nick, char *vhost, char *vident)
{
char *host = getvHost(nick),
    *ident = NULL;
    if( host && !stricmp(host, vhost) )
    {
        ident = getvIdent(nick);
        if( vident )
        {
            if( ident && !stricmp(ident, vident) )
                return 1;
        }
        else
        {
            if( !ident )
                return 1;
            return 2;
        }
    }
    return 0;
}

int hs_match_request_case(char *nick, char *vhost, char *vident)
{
char *host = getvHost(nick),
    *ident = NULL;
    if( host && !strcmp(host, vhost) )
    {
        ident = getvIdent(nick);
        if( vident )
        {
            if( ident && !strcmp(ident, vident) )
                return 1;
        }
        else
        {
            if( !ident )
                return 1;
            return 2;
        }
    }
    return 0;
}

int hs_do_request(User *u, char *nick, char *request, boolean ischan)
{
HostCore *prev,
    *ptr,
    *ptr_found = NULL;
NickAlias *na,
    *na_group;
Regex *expr;
int32 request_time = time(NULL),
    time_left      = 0;
boolean found = false,
    ffound    = false,
    update    = false,
    dofree    = false,
    matches_requested_vident = false,
    matches_requested_vhost  = false;
char hostmask[HOSTMAX];
char *p,
    *s        = NULL,
    *vident   = NULL,
    *vhost    = NULL,
    *creator  = NULL,
    *dupHost  = NULL,
    *dupIdent = NULL,
    *chan     = vh->chan;
unsigned char max = vh->displaymax;
int ctr;
    
    //  make sure the user exists
    
    if( !(na = findnick(nick)) )
    {
        my__announce_lang(u, nick, HOST_NOREG, nick);
        goto clear;
    }
    
    //  get the vident (if possible), @ as delimiter
    
    if( (vident = myStrGetOnlyToken(request, '@', 0)) )
    {
        
        //  ircd does not support vident
        
        if( !ircd->vident )
        {
            my__announce_lang(u, nick, HOST_NO_VIDENT);
            goto clear;
        }
        
        //  mark vhost for free() and
        //  get the remaining string
        
        dofree = true;
        vhost  = myStrGetTokenRemainder(request, '@', 1);
        
        //  reset vident if vident matches string '*'
        //  thus allowing '*@host' requests.
        //  deny requests containing a vident if:
        //  1. this is disabled in the config,
        //  2. its vident is too long,
        //  3. its vident contains invalid characters.
        
        if( !strcasecmp(vident, "*") )
            vident = NULL;
        else if( HasFlag(&vh->flags, DENYIDENTUPDATE) )
        {
            my__announce(nick, moduleGetLangString(u, LNG_REQUEST_UPDATE_DENIED), vhost);
            goto clear;
        }
        else if( strlen(vident) > USERMAX )
        {
            my__announce_lang(u, nick, HOST_SET_IDENTTOOLONG, USERMAX);
            goto clear;
        }
        else
        {
            for( s = vident; *s; s++ )
            {
                if( !isvalidchar(*s) )
                {
                    my__announce_lang(u, nick, HOST_SET_IDENT_ERROR);
                    goto clear;
                }
            }
        }
    }
    else
        vhost = request;
    
    //  no (valid) vhost was requested.
    
    if( !vhost )
    {
        my__announce(nick, moduleGetLangString(u, LNG_REQUEST_SYNTAX));
        goto clear;
    }
    
    //  check if the host isn't too long.
    
    if( strlen(vhost) >= HOSTMAX )
    {
        my__announce_lang(u, nick, HOST_SET_TOOLONG, HOSTMAX);
        goto clear;
    }
    else
        snprintf(hostmask, HOSTMAX, "%s", vhost);
    
    
    //  check if the host is valid
    
    if( !isValidHost(hostmask, 3) )
    {
        my__announce_lang(u, nick, HOST_SET_ERROR);
        goto clear;
    }
    
    if( !is_oper(u) && !(s = moduleGetData(&na->nc->moduleData, "exempt")) )
    {
        //  prevent request spam
        //  this is done at the beginning to
        //  deliver a consistent user-experience.
        //  one could argue, the auto-accept
        //  code afterwards should not be
        //  penalized by the artificial
        //  delay value, but i wont concede.
        //  
        //  warning: very ugly
        //  
        //  this method is very suboptimal.
        //  the cleanup_timer will randomly
        //  interfere with the time value,
        //  leading to unreliable auto-rejects.
        //  
        //  TODO: "un-uglify"
        
        if( (s = moduleGetData(&na->nc->moduleData, "time")) && !is_oper(u) )
        {
            if( (p = strchr(s, '|')) )
            {
                *p++    = '\0';
                dupHost = p;
            }

            if( dupHost && (p = strchr(dupHost, ':')) )
            {
                *p++     = '\0';
                dupIdent = p;
            }

            if( dupHost )
            {
                if( !strcmp(dupHost, vhost) && ((!vident && !dupIdent) || (vident && dupIdent && !strcmp(dupIdent, vident))) )
                {
                        dupHost  = NULL;
                        dupIdent = NULL;
                        
                        if (vident)
                            my__announce(nick, moduleGetLangString(u, LNG_HOST_IDENT_REJECTED), vident, vhost, moduleGetLangString(u, LNG_REQUEST_AUTO_REJECTED));
                        else
                            my__announce(nick, moduleGetLangString(u, LNG_HOST_REJECTED), vhost, moduleGetLangString(u, LNG_REQUEST_AUTO_REJECTED));
                        goto clear;
                }
            }
            else if( (time_left = strtol(s, (char **) NULL, 16) + vh->requestdelay - request_time) > 0 )
            {
                dupHost  = NULL;
                dupIdent = NULL;
                
                my__announce(nick, moduleGetLangString(u, LNG_REQUEST_WAIT), time_left);
                goto clear;
            }
            
            dupHost  = NULL;
            dupIdent = NULL;
        }
        
#ifdef SUPPORT_REGEX
        //  deny forbidden hosts
        for( expr = regex_head; expr; expr = expr->next )
        {
            if( !regexec(&expr->expr, request, 0, NULL, 0) )
            {
                if( expr->reason )
                    my__announce(nick, moduleGetLangString(u, LNG_REQUEST_FORBIDDEN_REASON), expr->reason);
                else
                    my__announce(nick, moduleGetLangString(u, LNG_REQUEST_FORBIDDEN));
                
                goto clear;
            }
        }
#endif
    }
    
    //  search for previous
    //  STILL OPEN request.
    
    prev = findHostCore(request_head, nick, &found);
    if( found )
    {
        if( !prev )
            ptr = request_head;
        else
            ptr = prev->next;
        
        //  same comment as above about
        //  request spam prevention.
        
        if( ((time_left = ptr->time + vh->requestdelay - request_time) > 0) && !is_oper(u) && !(s = moduleGetData(&na->nc->moduleData, "exempt")) )
        {
            my__announce(nick, moduleGetLangString(u, LNG_REQUEST_WAIT), time_left);
            goto clear;
        }
        
        //  if new request doesn't match
        //  the old one, mark it as an update.
        //  otherwise fake a success notice
        //  and don't bother announcing again.
        //  ... users like to spam their request.
        
        matches_requested_vident = ((vident && !ptr->vIdent) || (!vident && ptr->vIdent) || (!BadPtr(ptr->vIdent) && strcmp(ptr->vIdent, vident)) ? false : true);
        matches_requested_vhost  = (strcmp(ptr->vHost, vhost) ? false : true);
        
        if( !matches_requested_vident || !matches_requested_vhost )
            update = true;
        else
        {
            ptr->time = request_time;
            
            my__announce(nick, moduleGetLangString(u, LNG_REQUESTED));
            goto clear;
        }
    }
    
    //  automagically activate requests
    //  that either only reset the ident value
    //  or change the case (or both) of a
    //  previously ACCEPTED request.
    
    if( hs_match_request(na->nick, vhost, vident) )
    {
        if( hs_match_request_case(nick, vhost, vident) == 1 )
            my__announce(nick, moduleGetLangString(u, LNG_REQUEST_ALREADY_APPROVED), request);
        else
        {
            
            //  search for previously accepted
            //  request entry.
            
            ptr_found = findHostCore(hostCoreListHead(), na->nick, &ffound);
            if( ffound )
            {
                if( !ptr_found )
                    ptr_found = hostCoreListHead();
                else
                    ptr_found = ptr_found->next;
                
                //  copy original creator name and
                //  activate.
                    
                creator = sstrdup(ptr_found->creator);
                dupHost = sstrdup(ptr_found->vHost);
                
                my__activate_host_request(u, na->nick, vident, vhost, creator, time(NULL), moduleGetLangString(u, LNG_REQUEST_AUTO_APPROVED));
                
                if( !vident )
                {
                    if( !strcmp(dupHost, vhost) )
                        alog("New vhost request \"%s\" for %s automatically activated. Request only reset vIdent.", request, u->nick);
                    else
                        alog("New vhost request \"%s\" for %s automatically activated. Request reset vIdent and changed case.", request, u->nick);
                }
                else
                    alog("New vhost request \"%s\" for %s automatically activated. Request only changed case.", request, u->nick);
            }
            else
                goto clear;  // will this ever occur? if so, should we do something?
        }
        
        //  remove old request,
        //  if one exists.
        
        if( found )
        {
            alog("Removing vhost request for %s", u->nick);
           
            if( my__del_host_request(nick) )
            {
                my__announce(chan, moduleGetLangString(NULL, LNG_REQUEST_REMOVED_HOST), u->nick);
                my__show_list(u, "0", NULL, true, false);
            }
        }
        
        goto clear;
    }
    
    //  automagically activate requests that
    //  match a grouped nick. 
    
    for( ctr = 0; ctr < na->nc->aliases.count; ctr++ )
    {
        na_group = na->nc->aliases.list[ctr];
    
        if( strcmp(nick, na_group->nick) && hs_match_request(na_group->nick, vhost, vident) == 1 )
        {
            ptr_found = findHostCore(hostCoreListHead(), na_group->nick, &ffound);
            if( ffound )
            {
                if( !ptr_found )
                    ptr_found = hostCoreListHead();
                else
                    ptr_found = ptr_found->next;
                
                creator  = sstrdup(ptr_found->creator);
                dupHost  = sstrdup(ptr_found->vHost);
                if( ptr_found->vIdent )
                    dupIdent = sstrdup(ptr_found->vIdent);
                
                my__activate_host_request(u, nick, dupIdent, dupHost, creator, time(NULL), moduleGetLangString(u, LNG_REQUEST_AUTO_APPROVED));
                
                alog("New vhost request \"%s\" for %s automatically activated. Request matched approved host of grouped nick %s.", request, u->nick, na_group->nick);
                
                //  remove old request,
                //  if one exists.
                
                if( found )
                {
                    alog("Removing previous vhost request for %s", u->nick);
                    
                    if( my__del_host_request(nick) )
                    {
                        my__announce(chan, moduleGetLangString(NULL, LNG_REQUEST_REMOVED_GROUP), u->nick, na_group->nick);
                        my__show_list(u, "0", NULL, true, false);
                    }
                }
                
                goto clear;
            }
        }
    }
    
    //  all checks passed, woohoo!
    //  finally add request to our open
    //  requests list.
    
    my__add_host_request(nick, vident, vhost, u->nick, request_time);
    my__announce(nick, moduleGetLangString(u, LNG_REQUESTED));
    my__send_staff_memo(u, hostmask);
    
    moduleDelData(&na->nc->moduleData, "time");
    
    //  traverse open request list
    //  until nick entry is found.
    //  need pointer for announce.
    //  should do this differently...
    
    for( ctr = 0, ptr = request_head; ptr; ptr = ptr->next )
    {
        ctr++;
        
        if( !stricmp(ptr->nick, nick) )
            ptr_found = ptr;
    }
    
    if( ptr_found )
    {
        
        //  limit number of displayed entries.
        //  deliberate request spam will not
        //  spam the announce channel.
        
        if( max && ctr >= ++max && !update )
        {
            
            //  only display notice for first entry
            //  to exceed the limit.
            
            if( ctr == max )
                my__announce(chan, moduleGetLangString(NULL, LNG_REQUEST_NOT_SHOWN));
            goto skip;
        }
        else if( update )
            my__announce(chan, coloredText[my__def_language()][GetFlag(&vh->flags, DISPLAYCOLOR)]._updated);
        else
            my__announce(chan, coloredText[my__def_language()][GetFlag(&vh->flags, DISPLAYCOLOR)]._new);
        my__show_entry(NULL, ptr_found, false, true);
        my__announce(chan, moduleGetLangString(NULL, LNG_VHOST_REQUEST_DISPLAYED), 1, " ", ctr);
    }

        skip:
    alog("New vhost requested by %s", nick);

        clear:
    Anope_Free(s);
    Anope_Free(dupHost);
    Anope_Free(dupIdent);
    Anope_Free(creator);
    Anope_Free(vident);
    if( dofree && vhost )
        free(vhost);

    return MOD_CONT;
}

/* EOF */
