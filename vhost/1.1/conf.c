#include "conf.h"

int hs_call_config(User *u)
{
char *buf    = moduleGetLastBuffer(),
    *setting = myStrGetToken(buf, ' ', 0),
    *param   = myStrGetTokenRemainder(buf, ' ', 1);
    
    hs_do_config(u, setting, param, false);
    
    Anope_Free(setting);
    Anope_Free(param);
   
    return MOD_STOP;
}

int hs_do_config(User *u, char *setting, char *param, boolean ischan)
{
    if( !setting )
        hs_help_config(u);
    else if( !strcasecmp(setting, "regex") )
        hs_do_config_regex(u, param, ischan);
    else if( !strcasecmp(setting, "exception") )
        hs_do_config_exception(u, param, ischan);
    else if( !strcasecmp(setting, "save_db") )
    {
        my__save_db();
        my__announce(u->nick, moduleGetLangString(u, LNG_CONF_SAVE_DB));
    }
    else if( !strcasecmp(setting, "list") )
        hs_do_config_show(u, ischan);
    else if( !strcasecmp(setting, "set") )
        hs_do_config_set(u, param, ischan);
    else
        my__announce_lang(u, u->nick, UNKNOWN_COMMAND_HELP, setting, s_HostServ);

    return MOD_CONT;
}

int hs_do_config_exception(User *u, char *param, boolean ischan)
{
Exception_t *ptr;
char *nick  = u->nick,
    *cmd,
    *cparam;
int status,
    i = 0;
    if( !param )
    {
        hs_help_config_exception(u);
        return MOD_CONT;
    }
    
    cmd    = myStrGetToken(param, ' ', 0);
    cparam = myStrGetTokenRemainder(param, ' ', 1);
    
    if( !strcasecmp(cmd, "add") && cparam )
    {
        if( (status = my__add_exception(cparam, nick)) )
            my__announce(nick, moduleGetLangString(u, LNG_CONF_EXCEPTION_ADD_FAIL), cparam);
        else
            my__announce(nick, moduleGetLangString(u, LNG_CONF_EXCEPTION_ADD), cparam);
    }
    else if( !strcasecmp(cmd, "list") )
    {
        for( ptr = exception_head; ptr; ptr = ptr->next )
            my__announce(nick, moduleGetLangString(u, LNG_CONF_EXCEPTION_LIST), ++i, ptr->who, ptr->by);
        my__announce(nick, moduleGetLangString(u, LNG_CONF_EXCEPTION_LIST_END));
    }
    else if( !strcasecmp(cmd, "del") && cparam )
    {
        if( !strncmp(cparam++, "#", 1) )
            status = my__del_exception_by_num(atoi(cparam--));
        else
            status = my__del_exception_by_nick(--cparam);
        
        if( status )
            my__announce(nick, moduleGetLangString(u, LNG_CONF_EXCEPTION_NOT_FOUND));
        else
            my__announce(nick, moduleGetLangString(u, LNG_CONF_EXCEPTION_DELETED));
    }
    else
        hs_help_config_exception(u);
    
    Anope_Free(cmd);
    Anope_Free(cparam);
    
    return MOD_CONT;
}

int hs_do_config_regex(User *u, char *param, boolean ischan)
{
Regex *regmask;
char *nick  = u->nick,
    *reason = NULL,
    *cmd,
    *cparam;
int status,
    i = 0;
    if( !param )
    {
        hs_help_config_regex(u);
        return MOD_CONT;
    }
    
    cmd    = myStrGetToken(param, ' ', 0);
    cparam = myStrGetTokenRemainder(param, ' ', 1);
    
    if( !strcasecmp(cmd, "add") && cparam )
    {
        reason = myStrGetToken(cparam, ' ', 0);
        cparam = myStrGetTokenRemainder(cparam, ' ', 1);
        
        if( (status = my__add_regex(nick, (!strcmp(reason, "none") ? NULL : my__decodespace(reason)), cparam)) )
        {
            my__announce(nick, moduleGetLangString(u, LNG_CONF_REGEX_ADD_FAIL), cparam);
            alog("%s: Could not add regex '%s'", NAME, cparam);
        }
        else
            my__announce(nick, moduleGetLangString(u, LNG_CONF_REGEX_ADD), cparam);
    }
    else if( !strcasecmp(cmd, "list") )
    {
        for( regmask = regex_head; regmask; regmask = regmask->next )
        {
            my__announce(nick, moduleGetLangString(u, LNG_CONF_REGEX_LIST), ++i, regmask->str, regmask->nick);
            if( regmask->reason )
                my__announce(nick, "- %s: '%s'", moduleGetLangString(u, LNG_KEY_REASON), regmask->reason);
        }
        my__announce(nick, moduleGetLangString(u, LNG_CONF_REGEX_LIST_END));
    }
    else if( !strcasecmp(cmd, "del") && cparam )
    {
        if( !strncmp(cparam++, "#", 1) )
            status = my__del_regex_by_num(atoi(cparam--));
        else
            status = my__del_regex_by_nick(--cparam);
            
        if( status ) /* should never happen */
        {
            my__announce(nick, "Could not delete anything. Debug: '%s'", cparam);
            alog("%s: Could not delete anything. Debug: '%s'", NAME, cparam);
        }
        else
            my__announce(nick, moduleGetLangString(u, LNG_CONF_REGEX_DELETED));
    }
    else
        hs_help_config_regex(u);

    Anope_Free(cmd);
    Anope_Free(cparam);
    Anope_Free(reason);
    
    return MOD_CONT;
}

int hs_do_config_show(User *u, boolean ischan)
{
char *nick = u->nick;
    my__announce(nick, "NickName(*)       = %s", vh->nick);
    my__announce(nick, "RealName(*)       = %s", vh->real);
    my__announce(nick, "Channel(*)        = %s", vh->chan);
    my__announce(nick, "MemoUser          = %d", GetFlag(&vh->flags, MEMOUSER));
    my__announce(nick, "MemoOper          = %d", GetFlag(&vh->flags, MEMOOPER));
    my__announce(nick, "MemoSetters       = %d", GetFlag(&vh->flags, MEMOSETTERS));
    my__announce(nick, "DisplayMode       = %d", GetFlag(&vh->flags, DISPLAYMODE));
    my__announce(nick, "DisplayColor      = %d", GetFlag(&vh->flags, DISPLAYCOLOR));
    my__announce(nick, "DisplayMax        = %d", vh->displaymax);
    my__announce(nick, "Timer             = %d", vh->timer);
    my__announce(nick, "RequestDelay      = %d", vh->requestdelay);
    my__announce(nick, "DenyIdentUpdate   = %d", GetFlag(&vh->flags, DENYIDENTUPDATE));
    my__announce(nick, "Database          = %s", vh->dbname);
    my__announce(nick, "RegexDatabase     = %s", vh->regexdbname);
    my__announce(nick, "ExceptionDatabase = %s", vh->exceptiondbname);
    my__announce(nick, " ");
    my__announce(nick, "* = Changes need module reload.");
    my__announce(nick, "End of config.");
    
    return MOD_CONT;
}

int hs_do_config_set(User *u, char *param, boolean ischan)
{
char *setting = myStrGetToken(param, ' ', 0),
    *value    = myStrGetTokenRemainder(param, ' ', 1);
int ret = hs_do_config_set_internal(u, setting, atoi(value), ischan);
    Anope_Free(value);
    Anope_Free(setting);
    return ret;
}

int hs_do_config_set_internal(User *u, char *setting, int value, boolean ischan)
{
char *on = "ON",
    *off = "OFF";
    if( !setting )
        return MOD_STOP;
    
    if( !strcasecmp(setting, "Timer") )
    {
        if( value && value <= 9 )
            value = 10;

        vh->timer = value;
        
        moduleDelCallback("my__vhost_waiting_timer");
        my__vhost_waiting_timer(0, NULL);
    }
    else if( !strcasecmp(setting, "RequestDelay") )
    {
        if( value && value < 0 )
            value = 0;

        vh->requestdelay = value;
    }
    else if( !strcasecmp(setting, "DenyIdentUpdate") )
    {
        if( value >= 2 || value < 0 )
            value = 0;
        SetFlag_int(&vh->flags, DENYIDENTUPDATE, value);
    }
    else if( !strcasecmp(setting, "DisplayMax") )
    {
        if( value && value < 0 )
            value = 0;
        vh->displaymax = value;
        
        on  = "LIMITED";
        off = "NO LIMIT";
    }
    else if( !strcasecmp(setting, "DisplayColor") )
    {
        if( value >= 2 || value < 0 )
            value = 0;
        SetFlag_int(&vh->flags, DISPLAYCOLOR, value);
    }
    else
    {
        value = (value ? 1 : 0);
        
        if( !strcasecmp(setting, "MemoUser") )
            SetFlag_int(&vh->flags, MEMOUSER, value);
        else if( !strcasecmp(setting, "MemoOper") )
            SetFlag_int(&vh->flags, MEMOOPER, value);
        else if( !strcasecmp(setting, "MemoSetters") )
            SetFlag_int(&vh->flags, MEMOSETTERS, value);
        else if( !strcasecmp(setting, "DisplayMode") )
        {
            SetFlag_int(&vh->flags, DISPLAYMODE, value);
            
            on  = "EXTENDED";
            off = "DEFAULT";
        }
        else
        {
            if( u )
                my__announce_lang(u, u->nick, UNKNOWN_COMMAND_HELP, setting, s_HostServ);
            return MOD_STOP;
        }
    }
    
    if( u )
    {
        my__announce(u->nick, moduleGetLangString(u, LNG_CONF_TEMPORARY), setting, value, (value ? on : off));
        alog("%s: %s setting %s temporarily to: %d (%s)", NAME, u->nick, setting, value, (value ? on : off));
    }
    if( debug )
        alog("%s: Set %s to: %d (%s)", NAME, setting, value, (value ? on : off));
    
    return MOD_CONT;
}

/* EOF */
