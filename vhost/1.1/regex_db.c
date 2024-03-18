#include "regex_db.h"

char *my__checkregex(char *s)
{
regex_t expr;
char *errtmp;
int errorcode,
    errorbufsize = 1;
static char errorbuf[512];
    errorcode = regcomp(&expr, s, REG_ICASE|REG_EXTENDED);
    if( errorcode > 0 )
    {
        errorbufsize += regerror(errorcode, &expr, NULL, 0);
        if( !(errtmp = malloc(errorbufsize)) )
            goto oom;
        regerror(errorcode, &expr, errtmp, errorbufsize);
        strncpy(errorbuf, errtmp, sizeof(errorbuf));
        
        free(errtmp);
        regfree(&expr);
        return errorbuf;
    }
    else
        regfree(&expr);

        done:
    return NULL;
    
        oom:
    anope_cmd_global(vh->nick, "Unable to allocate memory to create the regex LL, problems i sense..");
    goto done;
}

int my__add_regex(char *nick, char *reason, char *regex)
{
Regex *tmp;
boolean found = false;
char *p;
    if( !nick || !regex )
        return MOD_STOP;
    else if( (p = my__checkregex(regex)) )
    {
        my__announce(nick, "Error in '%s': %s", regex, p);
        return MOD_STOP;
    }

    if( !regex_head )
    {
        if( (regex_head = createRegex(regex_head, nick, reason, regex)) )
            return MOD_CONT;
    }
    else
    {
        tmp = findRegex(regex_head, regex, &found);

        if( !found )
            regex_head = insertRegex(regex_head, tmp, nick, reason, regex);

        return MOD_CONT;
    }
    return MOD_STOP;
}

int my__del_regex_by_num(int num)
{
Regex *tmp;
boolean found = false;
    if( !num || !regex_head )
        return MOD_STOP;
    
    tmp = findRegexNum(regex_head, num, &found);

    if( found )
        regex_head = deleteRegex(regex_head, tmp);

     return MOD_CONT;
}

int my__del_regex_by_nick(char *nick)
{
Regex *tmp;
boolean found = false;
    if( !nick || !regex_head )
        return MOD_STOP;
    
        start:
    tmp = findRegexNick(regex_head, nick, &found);

    if( found )
        regex_head = deleteRegex(regex_head, tmp);
    else
        return MOD_CONT;
    
    goto start;
}

Regex *createRegex(Regex *next, char *nick, char *reason, char *regex)
{
    if( !nick || !regex )
        return NULL;
    
    if( (next = malloc(sizeof(Regex))) )
    {
        next->next = NULL;
        
        if( (next->str = malloc(sizeof(char) * strlen(regex) + 1)) )
#ifdef HAVE_MEMPCPY
            *((char *) mempcpy (next->str, regex, sizeof(char) * strlen(regex))) = '\0';
#else
        {
            memcpy (next->str, regex, sizeof(char) * strlen(regex));
            next->str[sizeof(char) * strlen(regex)] = '\0';
        }
#endif
        else
            goto oom;
            
        if( (next->nick = malloc(sizeof(char) * strlen(nick) + 1)) )
#ifdef HAVE_MEMPCPY
            *((char *) mempcpy (next->nick, nick, sizeof(char) * strlen(nick))) = '\0';
#else
        {
            memcpy (next->nick, nick, sizeof(char) * strlen(nick));
            next->nick[sizeof(char) * strlen(nick)] = '\0';
        }
#endif
        else
            goto oom;
        if( reason )
        {
            if( (next->reason = malloc(sizeof(char) * strlen(reason) + 1)) )
#ifdef HAVE_MEMPCPY
                *((char *) mempcpy (next->reason, reason, sizeof(char) * strlen(reason))) = '\0';
#else
            {
                memcpy (next->reason, reason, sizeof(char) * strlen(reason));
                next->reason[sizeof(char) * strlen(reason)] = '\0';
            }
#endif
            else
                goto oom;
        }
        else
            next->reason = NULL;

        regcomp(&next->expr, regex, (REG_ICASE|REG_EXTENDED|REG_NOSUB));

        if( !vHostInternal )
            alog("Regex '%s' added by %s with reason '%s'.", regex, nick, (reason ? reason : "none"));    
        return next;
    }
    
        oom:
    anope_cmd_global(vh->nick, "Unable to allocate memory to create the regex LL, problems i sense..");
    return NULL;
}

Regex *findRegex(Regex *head, char *regex, boolean *found)
{
Regex *current = head,
    *prev      = current;
    *found = false;
    if( !regex )
        return NULL;

    while( current )
    {
        if( !stricmp(regex, current->str) )
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

Regex *findRegexNick(Regex *head, char *nick, boolean *found)
{
Regex *current = head,
    *prev      = current;
    *found = false;
    if( !nick )
        return NULL;

    while( current )
    {
        if( !stricmp(nick, current->nick) )
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

Regex *insertRegex(Regex *head, Regex *prev, char *nick, char *reason, char *regex)
{
Regex *next,
    *tmp;
    if( !nick || !regex )
        return NULL;

    if( (next = malloc(sizeof(Regex))) )
    {
        if( (next->str = malloc(sizeof(char) * strlen(regex) + 1)) )
#ifdef HAVE_MEMPCPY
            *((char *) mempcpy (next->str, regex, sizeof(char) * strlen(regex))) = '\0';
#else
        {
            memcpy (next->str, regex, sizeof(char) * strlen(regex));
            next->str[sizeof(char) * strlen(regex)] = '\0';
        }
#endif
        else
            goto oom;
            
        if( (next->nick = malloc(sizeof(char) * strlen(nick) + 1)) )
#ifdef HAVE_MEMPCPY
            *((char *) mempcpy (next->nick, nick, sizeof(char) * strlen(nick))) = '\0';
#else
        {
            memcpy (next->nick, nick, sizeof(char) * strlen(nick));
            next->nick[sizeof(char) * strlen(nick)] = '\0';
        }
#endif
        else
            goto oom;
        if( reason )
        {
            if ((next->reason = malloc(sizeof(char) * strlen(reason) + 1)))
#ifdef HAVE_MEMPCPY
                *((char *) mempcpy (next->reason, reason, sizeof(char) * strlen(reason))) = '\0';
#else
            {
                memcpy (next->reason, reason, sizeof(char) * strlen(reason));
                next->reason[sizeof(char) * strlen(reason)] = '\0';
            }
#endif
            else
                goto oom;
        }
        else
            next->reason = NULL;
    
        regcomp(&next->expr, regex, (REG_ICASE|REG_EXTENDED|REG_NOSUB));

        if( !prev )
        {
            tmp  = head;
            head = next;
            next->next = tmp;
        }
        else
        {
            tmp = prev->next;
            prev->next = next;
            next->next = tmp;
        }
        
        if( !vHostInternal )
            alog("Regex '%s' added by %s with reason '%s'.", regex, nick, (reason ? reason : "none"));
        return head;
    }
    
        oom:
    anope_cmd_global(vh->nick, "Unable to allocate memory to create the regex LL, problems i sense..");
    return NULL;
}

Regex *deleteRegex(Regex *head, Regex *prev)
{
Regex *tmp;
    if( !prev )
    {
        tmp  = head;
        head = head->next;
    }
    else
    {
        tmp = prev->next;
        prev->next = tmp->next;
    }
    
    if( !vHostInternal )
        alog("Regex '%s' deleted.", tmp->str);
    
    free(tmp->str);
    free(tmp->nick);
    Anope_Free(tmp->reason);
    regfree(&tmp->expr);

    free(tmp);
    
    return head;
}

Regex *findRegexNum(Regex *head, int num, boolean *found)
{
Regex *current = head,
    *prev      = current;
int ctr = 0;
    *found = false;
    if( !num )
        return NULL;

    while( current )
    {
        if( ++ctr == num )
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
