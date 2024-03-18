#include "exempt_db.h"

int my__add_exception(char *who, char *by)
{
Exception_t *tmp;
NickAlias *na;
boolean found = false;
    if( !who || !by)
        return MOD_STOP;
    else if( !(na = findnick(who)) )
    {
        my__announce(by, getstring(na, NICK_X_NOT_REGISTERED), who);
        return MOD_STOP;
    }

    if( !exception_head )
    {
        exception_head = createException(exception_head, na, by);
        
        if( exception_head )
            return MOD_CONT;
    }
    else
    {
        tmp = findException(exception_head, who, &found);

        if( !found )
            exception_head = insertException(exception_head, tmp, na, by);

        return MOD_CONT;
    }
    
    return MOD_STOP;
}

int my__del_exception_by_num(int num)
{
Exception_t *tmp;
boolean found = false;
    if( !num || !exception_head )
        return MOD_STOP;
    
    tmp = findExceptionNum(exception_head, num, &found);
    
    if( !found )
        return MOD_STOP;
    
    exception_head = deleteException(exception_head, tmp);
    
    return MOD_CONT;
}

int my__del_exception_by_nick(char *who)
{
Exception_t *tmp;
boolean found = false;
    if( !who || !exception_head )
        return MOD_STOP;
    
    tmp = findExceptionNick(exception_head, who, &found);

    if( !found )
        return MOD_STOP;
    
    exception_head = deleteException(exception_head, tmp);
    
    return MOD_CONT;
}

Exception_t *createException(Exception_t *next, NickAlias *na, char *by)
{
NickAlias *na_group;
int ctr;
char *who = na->nick;
    if( !who || !by )
        return NULL;
    
    if( (next = malloc(sizeof(Exception_t))) )
    {
        next->next = NULL;
        next->na   = NULL;
        
        if( (next->who = malloc(sizeof(char) * strlen(who) + 1)) )
#ifdef HAVE_MEMPCPY
            *((char *) mempcpy (next->who, who, sizeof(char) * strlen(who))) = '\0';
#else
        {
            memcpy (next->who, who, sizeof(char) * strlen(who));
            next->who[sizeof(char) * strlen(who)] = '\0';
        }
#endif
        else
            goto oom;
            
        if( (next->by = malloc(sizeof(char) * strlen(by) + 1)) )
#ifdef HAVE_MEMPCPY
            *((char *) mempcpy (next->by, by, sizeof(char) * strlen(by))) = '\0';
#else
        {
            memcpy (next->by, by, sizeof(char) * strlen(by));
            next->by[sizeof(char) * strlen(by)] = '\0';
        }
#endif
        else
            goto oom;
        
        for( ctr = 0; ctr < na->nc->aliases.count; ctr++ )
        {
            na_group = na->nc->aliases.list[ctr];
            
            if( !next->na && strcmp(who, na_group->nick) )
                next->na = na_group;
                
            moduleAddData(&na_group->nc->moduleData, "exempt", by);
        }
        
        if( !vHostInternal )
            alog("Exception for '%s' added by %s.", who, by);    
        return next;
    }
    
        oom:
    anope_cmd_global(vh->nick, "Unable to allocate memory to create the exception LL, problems i sense..");
    return NULL;
}

Exception_t *findException(Exception_t *head, char *who, boolean *found)
{
Exception_t *prev,
    *current;
    *found  = false;
    current = head;
    prev    = current;

    if( !who )
        return NULL;

    while( current )
    {
        if( !stricmp(who, current->who) )
        {
            *found = true;
            break;
        }
        else
        {
            prev    = current;
            current = current->next;
        }
    }
    if( current == head )
        return NULL;
    else
        return prev;
}

Exception_t *findExceptionNick(Exception_t *head, char *who, boolean *found)
{
Exception_t *current,
    *prev;
    *found  = false;
    current = head;
    prev    = current;
    
    if( !who )
        return NULL;

    while( current )
    {
        if( !stricmp(who, current->who) )
        {
            *found = true;
            break;
        }
        else
        {
            prev    = current;
            current = current->next;
        }
    }
    if( current == head)
        return NULL;
    else
        return prev;
}

Exception_t *insertException(Exception_t *head, Exception_t *prev, NickAlias *na, char *by)
{
NickAlias *na_group;
Exception_t *next,
    *tmp;
int ctr;
char *who = na->nick;
    if( !who || !by )
        return NULL;

    if( (next = malloc(sizeof(Exception_t))) )
    {
        next->na = NULL;
        
        if( (next->who = malloc(sizeof(char) * strlen(who) + 1)) )
#ifdef HAVE_MEMPCPY
            *((char *) mempcpy (next->who, who, sizeof(char) * strlen(who))) = '\0';
#else
        {
            memcpy (next->who, who, sizeof(char) * strlen(who));
            next->who[sizeof(char) * strlen(who)] = '\0';
        }
#endif
        else
            goto oom;
            
        if( (next->by = malloc(sizeof(char) * strlen(by) + 1)) )
#ifdef HAVE_MEMPCPY
            *((char *) mempcpy (next->by, by, sizeof(char) * strlen(by))) = '\0';
#else
        {
            memcpy (next->by, by, sizeof(char) * strlen(by));
            next->by[sizeof(char) * strlen(by)] = '\0';
        }
#endif
        else
            goto oom;
        
        if( !prev )
        {
            tmp        = head;
            head       = next;
            next->next = tmp;
        }
        else
        {
            tmp        = prev->next;
            prev->next = next;
            next->next = tmp;
        }
        
        for( ctr = 0; ctr < na->nc->aliases.count; ctr++ )
        {
            na_group = na->nc->aliases.list[ctr];
            
            if( !next->na && strcmp(who, na_group->nick) )
                next->na = na_group;
            
            moduleAddData(&na_group->nc->moduleData, "exempt", by);
        }
        
        if( !vHostInternal )
            alog("Exception for '%s' added by %s.", who, by);
        return head;
    }
    
        oom:
    anope_cmd_global(vh->nick, "Unable to allocate memory to create the exception LL, problems i sense..");
    return NULL;
}

Exception_t *deleteException(Exception_t *head, Exception_t *prev)
{
NickAlias *na;
Exception_t *tmp;
int ctr;
    if( !prev )
    {
        tmp  = head;
        head = head->next;
    }
    else
    {
        tmp        = prev->next;
        prev->next = tmp->next;
    }
    
    if( tmp->na )
    {
        for( ctr = 0 ; ctr < tmp->na->nc->aliases.count; ctr++ )
        {
            na = tmp->na->nc->aliases.list[ctr];
            moduleDelData(&na->nc->moduleData, "exempt");
        }
        tmp->na = NULL;
    }
    
    if( !vHostInternal )
        alog("Exception for '%s' deleted.", tmp->who);
    
    free(tmp->who);
    free(tmp->by);
    free(tmp);
    
    return head;
}

Exception_t *findExceptionNum(Exception_t *head, int num, boolean *found)
{
Exception_t *current,
    *prev;
int ctr;
    current = head;
    prev    = current;
    *found  = false;
    
    if( !num )
        return NULL;
    
    for( ctr = 1; current; ctr++ )
    {
        if( ctr == num )
        {
            *found = true;
            break;
        }
        else
        {
            prev    = current;
            current = current->next;
        }
    }
    
    if( current == head )
        return NULL;
    else
        return prev;
}

/* EOF */
