#include "help.h"

void hs_help(User *u)
{
char *nick = u->nick;
    my__announce(nick, moduleGetLangString(u, LNG_HELP));
    if( is_host_setter(u) )
        my__announce(nick, moduleGetLangString(u, LNG_HELP_SETTER));
}

int hs_help_request(User *u)
{
char *nick = u->nick;
    my__announce(nick, moduleGetLangString(u, LNG_REQUEST_SYNTAX));
    my__announce(nick, " ");
    my__announce(nick, moduleGetLangString(u, LNG_HELP_REQUEST));

    return MOD_CONT;
}

int hs_help_activate(User *u)
{
char *nick = u->nick;
    if( is_host_setter(u) )
    {
        my__announce(nick, moduleGetLangString(u, LNG_ACTIVATE_SYNTAX));
        my__announce(nick, " ");
        my__announce(nick, moduleGetLangString(u, LNG_HELP_ACTIVATE));
        if( HasFlag(&vh->flags, MEMOUSER) )
            my__announce(nick, moduleGetLangString(u, LNG_HELP_ACTIVATE_MEMO));
    }
    else
        my__announce_lang(u, nick, NO_HELP_AVAILABLE, "ACTIVATE");

    return MOD_CONT;
}

int hs_help_actall(User *u)
{
char *nick = u->nick;
    if( is_host_setter(u) )
    {
        my__announce(nick, moduleGetLangString(u, LNG_ACTALL_SYNTAX));
        my__announce(nick, " ");
        my__announce(nick, moduleGetLangString(u, LNG_HELP_ACTALL));
        /*
        if (vh->memouser)
            my__announce(nick, moduleGetLangString(u, LNG_HELP_ACTALL_MEMO));
        */
    }
    else
        my__announce_lang(u, nick, NO_HELP_AVAILABLE, "REJALL");

    return MOD_CONT;
}

int hs_help_reject(User *u)
{
char *nick = u->nick;
    if( is_host_setter(u) )
    {
        my__announce(nick, moduleGetLangString(u, LNG_REJECT_SYNTAX));
        my__announce(nick, " ");
        my__announce(nick, moduleGetLangString(u, LNG_HELP_REJECT));
        if( HasFlag(&vh->flags, MEMOUSER) )
            my__announce(nick, moduleGetLangString(u, LNG_HELP_REJECT_MEMO));
    }
    else
        my__announce_lang(u, nick, NO_HELP_AVAILABLE, "REJECT");

    return MOD_CONT;
}

int hs_help_rejall(User *u)
{
char *nick = u->nick;
    if( is_host_setter(u) )
    {
        my__announce(nick, moduleGetLangString(u, LNG_REJALL_SYNTAX));
        my__announce(nick, " ");
        my__announce(nick, moduleGetLangString(u, LNG_HELP_REJALL));
        if( HasFlag(&vh->flags, MEMOUSER) )
            my__announce(nick, moduleGetLangString(u, LNG_HELP_REJALL_MEMO));
    }
    else
        my__announce_lang(u, nick, NO_HELP_AVAILABLE, "REJALL");

    return MOD_CONT;
}

int hs_help_waiting(User *u)
{
char *nick = u->nick;
    if( is_host_setter(u) )
    {
        my__announce(nick, moduleGetLangString(u, LNG_WAITING_SYNTAX));
        my__announce(nick, " ");
        my__announce(nick, moduleGetLangString(u, LNG_HELP_WAITING));
    }
    else
        my__announce_lang(u, nick, NO_HELP_AVAILABLE, "WAITING");

    return MOD_CONT;
}

int hs_help_config(User *u)
{
char *nick = u->nick;
    if( is_services_admin(u) )
    {
        my__announce(nick, moduleGetLangString(u, LNG_CONFIG_SYNTAX));
        my__announce(nick, " ");
        my__announce(nick, moduleGetLangString(u, LNG_HELP_CONFIG));
    }
    else
        my__announce_lang(u, nick, NO_HELP_AVAILABLE, "CONFIG");

    return MOD_CONT;
}

int hs_help_config_regex(User *u)
{
char *nick = u->nick;
    if( is_services_admin(u) )
        my__announce(nick, moduleGetLangString(u, LNG_CONFIG_REGEX_SYNTAX));
    else
        my__announce_lang(u, nick, NO_HELP_AVAILABLE, "CONFIG");

    return MOD_CONT;
}

int hs_help_config_exception(User *u)
{
char *nick = u->nick;
    if( is_services_admin(u) )
        my__announce(nick, moduleGetLangString(u, LNG_CONFIG_EXCEPTION_SYNTAX));
    else
        my__announce_lang(u, nick, NO_HELP_AVAILABLE, "CONFIG");

    return MOD_CONT;
}

/* EOF */
