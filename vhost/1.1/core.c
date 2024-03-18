#include "core.h"
#include "commands.h"

/* vhost - Add request and activate functionality to HostServ,
 *         along with adding +req as optional param to HostServ list
 *         and a channel bot.
 *
 * (C) 2008-2010 hmtX^mokkori
 * Contact me at hmtx@immortal-anime.net
 *
 * Based on the modified module by the Anope Team <info@anope.org>
 *
 * Based on the original module by Rob <rob@anope.org>
 * Included in the Anope module pack since Anope 1.7.11
 * Anope Coder: GeniusDex <geniusdex@anope.org>
 *
 * Please read COPYING and README for further details.
 *
 * Send bug reports to me instead of the Anope Coder or module
 * author, because any changes since my modifications
 * are not supported by the Anope Team or the original author.
 */

//  configuration variables
//  these may be modified with care

vHost_t vh[] = {
  {"vhost",                  /* nick */
  "vHost Server",            /* real */
  "#opers",                  /* chan */
  "+h %s",                   /* chanmodes */
  0,                         /* flags */
  0,                         /* displaymax */
  0,                         /* timer */
  0,                         /* requestdelay */
  "hs_request.db",           /* dbname */
  "hs_request.regex.db",     /* regexdbname */
  "hs_request.exempt.db"}    /* exceptiondbname */
};

//  do NOT modify anything below here
//  unless you know what you are doing!

unsigned char vHostInternal = 1;

static const Hooks_t hook_ptr[] = {
    {EVENT_DB_SAVING, my__hook_db_saving},
    {EVENT_RELOAD, my__hook_config_reload},
    {EVENT_DB_BACKUP, my__hook_db_backup},
    {EVENT_NICK_DROPPED, ns_do_drop},
    {EVENT_NICK_EXPIRE, ns_do_drop},
    {EVENT_NICK_FORBIDDEN, ns_do_drop},
    {EVENT_GROUP, ns_do_group}
};

vHost_t def_settings[] = {
    {'\0','\0','\0','\0',
    0,0,0,0,
    '\0','\0','\0'}
};

/*****************************************************************************/
 
int AnopeInit(int argc, char **argv)
{
EvtHook *hook;
Message *msg;
unsigned char *s = &vHostInternal,
    ctr;
    request_head = NULL;
    regex_head = NULL;
    
    if( findModule("hs_request") )
    {
        alog("%s: Please unload hs_request first and make sure it stays unloaded.", NAME);
        return MOD_STOP;
    }
    
    if( !ircd->vhost )
    {
        alog("%s: IRCd does not support ip cloaking.", NAME);
        return MOD_STOP;
    }
    
#ifdef IRC_UNREAL32
    if( UseTokens )
        msg = createMessage("!", my__privmsg);
    else
        msg = createMessage("PRIVMSG", my__privmsg);
#else
    msg = createMessage("PRIVMSG", my__privmsg);
#endif
    if( moduleAddMessage(msg, MOD_HEAD) != MOD_ERR_OK )
    {
        alog("%s: Can't add own messages.", NAME);
        return MOD_STOP;
    }
    else if( debug )
        alog("%s: Added IRCD handler for PRIVMSG event.", NAME);
    
    msg = createMessage("KICK", my__kick);
    if( moduleAddMessage(msg, MOD_TAIL) != MOD_ERR_OK )
    {
        alog("%s: Can't add own messages.", NAME);
        return MOD_STOP;
    }
    else if( debug )
        alog("%s: Added IRCD handler for KICK event.", NAME);
    
    my__add_message_list();
    
    for( ctr = 0; ctr < (sizeof(hook_ptr)/sizeof(Hooks_t)); ctr++ )
    {
        hook = createEventHook(hook_ptr[ctr].src, hook_ptr[ctr].func);
        if( moduleAddEventHook(hook) != MOD_ERR_OK )
        {
            alog("%s: Can't hook to %s event.", NAME, hook_ptr[ctr].src);
            return MOD_STOP;
        }
        else if( debug )
            alog("%s: Added hook for %s event.", NAME, hook_ptr[ctr].src);
    }
    
    //  backup default config
    
    def_settings->flags        = vh->flags;
    def_settings->displaymax   = vh->displaymax;
    def_settings->timer        = vh->timer;
    def_settings->requestdelay = vh->requestdelay;
    
    my__load_config();
    my__add_languages();
    
    if( debug )
        alog("%s: Loading databases.", NAME);
    my__load_db();
    
    if( my__add_client() != MOD_CONT )
        return MOD_STOP;
    else if( my__join_channel(vh->chan) != MOD_CONT )
        return MOD_STOP;
    
    my__vhost_waiting_timer(0, NULL);
    my__vhost_cleanup_timer(0, NULL);
    
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    
#ifdef SUPPORT_WILDCARD
    alog("%s: Support for wildcard nick matching is enabled. To disable this feature follow instructions in core.h and recompile.", NAME);
#else
    alog("%s: Support for wildcard nick matching is disabled. To enable this feature follow instructions in core.h and recompile.", NAME);
#endif
    
#ifdef SUPPORT_REGEX
    alog("%s: Support for regex nick matching is enabled. To disable this feature follow instructions in core.h and recompile.", NAME);
#else
    alog("%s: Support for regex nick matching is disabled. To enable this feature follow instructions in core.h and recompile.", NAME);
#endif
    
    alog("%s: loaded.", NAME);
    *s = 0;
    return MOD_CONT;
}
 
void AnopeFini(void)
{
unsigned char *s = &vHostInternal;
    *s = 1;
    
    if( debug )
        alog("%s: Saving databases.", NAME);
    my__save_db();

    //  clean up all open host requests
    
    while( request_head )
        request_head = deleteHostCore(request_head, NULL);
    while( regex_head )
        regex_head = deleteRegex(regex_head, NULL);
    
    my__del_client();
    
    alog("%s: Unloaded successfully.", NAME);
}

/*****************************************************************************/

void my__core(User *u, int ac, char *buf[])
{
char *target,
    *msg,
    *cmd,
    *ping = NULL;
unsigned char i = 1;
unsigned short int ctr = 0;
    if( ac < 2 )
        return;
    
    target = buf[0];
    msg    = buf[1];
    
    do
    {
        cmd = myStrGetToken(msg, ' ', ctr);
        ctr++;
    } while( !cmd && sizeof(char) * strlen(msg) >= ctr );
    if( !cmd )
        return;
    
    if( !stricmp(cmd, "\1PING") )
    {
        if( !(ping = myStrGetToken(msg, ' ', ctr)) )
            ping = "\1";
        else
            i = 0;
            
        notice(vh->nick, u->nick, "\1PING %s", ping);
    }
    else if( skeleton )
        notice_lang(vh->nick, u, SERVICE_OFFLINE, vh->nick);
    else if( *target != '#' )
        goto clear;
    else
    {
        Command *c = findCommand(vHost_cmdTable, cmd);

        if( c )
            do_run_cmd(vh->nick, u, c, msg);
    }
    
        clear:
    Anope_Free(cmd);
    if (i && ping)
        free(ping);
}

/*************************************************************************/

int my__add_client(void)
{
NickAlias *na;
char *ch;
    if( findbot(vh->nick) )
        alog("%s: Bot \002%s\002 already exists.", NAME, vh->nick);
    else if( strlen(vh->nick) > NickLen )
        alog("%s: Bot Nicks may only contain valid nick characters.", NAME);
    else
    {
        if( isdigit(vh->nick[0]) || vh->nick[0] == '-' )
        {
            alog("%s: Bot Nicks may only contain valid nick characters.", NAME);
            return MOD_STOP;
        }
        for( ch = vh->nick; *ch && (ch - vh->nick) < NICKMAX; ch++ )
            if( !isvalidnick(*ch) )
            {
                alog("%s: Bot Nicks may only contain valid nick characters.", NAME);
                return MOD_STOP;
            }
        
        if( !anope_valid_nick(vh->nick) )
        {
            alog("%s: Bot Nicks may only contain valid nick characters.", NAME);
            return MOD_STOP;
        }
        
        if( nickIsServices(vh->nick, 0) )
        {
            alog("%s: Sorry, bot creation failed.", NAME);
            return MOD_STOP;
        }
        
        if( (na = findnick(vh->nick)) )
        {
            alog("%s: Nickname \002%s\002 is already registered. Drop nick and reload.", NAME, vh->nick);
            return MOD_STOP;
        }
        
        anope_cmd_nick(vh->nick, vh->real, ircd->botserv_bot_mode);
        alog("%s: '%s' connected.", NAME, vh->nick);
        return MOD_CONT;
    }
    return MOD_STOP;
}

int my__join_channel(char *chan)
{
ChannelInfo *ci;
    if( BadPtr(chan) )
        alog("%s: No channel specified.", NAME);
    else if( !(ci = cs_findchan(chan)) )
        alog("%s: Channel \002%s\002 isn't registered.", NAME, chan);
    else if( ci->flags & CI_VERBOTEN )
        alog("%s: Channel \002%s\002 may not be registered or used.", NAME, chan);
    else
    {
        anope_cmd_join(vh->nick, chan, time(NULL));
        anope_cmd_mode(vh->nick, chan, vh->chanmodes, vh->nick);
        alog("%s: Joined '%s'", vh->nick, chan);
        return MOD_CONT;
    }
    return MOD_STOP;
}

void my__del_client(void)
{
    anope_cmd_quit(vh->nick, "Module Unloaded");
    alog("%s: '%s' disconnected.", NAME, vh->nick);
    
    if( ircd->sqline )
    {
        anope_cmd_unsqline(vh->nick);
    }
}

/*************************************************************************/

boolean my__ismine(char *what)
{
    if( !what )
        return false;
    
    if( *what == '#' && !stricmp(what, vh->chan) )
        return true;
    else if( !stricmp(what, vh->nick) )
        return true;
    
    return false;
}

int my__privmsg(char *source, int ac, char **av)
{
User *u;
char *s;
    if( ac != 2 )
        return MOD_CONT;
    else if( !(u = finduser(source)) )
        return MOD_CONT;
    
    if( (s = strchr(av[0], '@')) )
    {
        *s++ = 0;
        if( stricmp(s, ServerName) )
            return MOD_CONT;
    }
    
    if( my__ismine(av[0]) )
        my__core(u, ac, av);
    
    return MOD_CONT;
}

int my__kick(char *source, int ac, char **av)
{
    if( !my__ismine(av[0]) || !my__ismine(av[1]) )
        return MOD_CONT;
    
    if( LogChannel )
        alog("%s got kicked from '%s' by %s (Auto re-joining)", av[1], av[0], source);

    my__join_channel(av[0]);
    
    return MOD_CONT;
}

/*************************************************************************/

void my__show_list(User *u, char *nick, char *title, boolean ischan, boolean call_from_timer)
{
HostCore *ptr;
User *u2 = (ischan ? NULL : u);
boolean found = false,
    wild = false;
#ifdef SUPPORT_REGEX
boolean regex = false;
regex_t expr;
char *p;
#endif
int max = ((nick && isdigit(nick[0])) ? (atoi(nick) <= 0 ? -1 : atoi(nick)) : 0);
unsigned short int ctr = 0,
    total = 0;
char *target = (ischan ? vh->chan : u->nick);
    if( max )
    {
        if( max > NSListMax )
            max = NSListMax;
        nick = NULL;
    }
    else
        max = NSListMax;
    
    if( (ptr = request_head) )
    {
        if( title && !nick && !call_from_timer && max != -1 )
            my__announce(target, moduleGetLangString(u2, LNG_VHOST_LISTING), title);
        
        if( nick )
        {
            if( *nick == '!' )
            {
#ifdef SUPPORT_REGEX
                if( (p = my__checkregex(++nick)) )
                {
                    my__announce(target, moduleGetLangString(u2, LNG_REGEX_ERROR), nick, p);
                    return;
                }
    
                regex = true;
                regcomp(&expr, nick, (REG_ICASE|REG_EXTENDED|REG_NOSUB));
#else
                my__announce(target, moduleGetLangString(u2, LNG_REGEX_DISABLED));
                return;
#endif
            }
            else if( strchr(nick, '*') || strchr(nick, '?') )
#ifdef SUPPORT_WILDCARD
                wild = true;
#else
            {
                my__announce(target, moduleGetLangString(u2, LNG_WILDCARD_DISABLED));
                return;
            }
#endif
        }
        
        while( ptr )
        {
            if( nick )
            {
                if( wild )
                    if( match_wild_nocase(nick, ptr->nick) )
                        found = true;
                    else
                        goto skip;
#ifdef SUPPORT_REGEX
                else if( regex )
                    if( !regexec(&expr, ptr->nick, 0, NULL, 0) )
                        found = true;
                    else
                        goto skip;
#endif
                else if( strcasecmp(nick, ptr->nick) )
                    goto skip;
                else
                    found = true;
            }
            
            if( !call_from_timer && ctr < max )
                my__show_entry(u2, ptr, true, ischan);
            
            ctr++;
                skip:
            ptr = ptr->next;
        }
        total = (max && max < ctr ? max : ctr);
        
#ifdef SUPPORT_REGEX
        if( regex )
            regfree(&expr);
#endif
        
        if( max == -1 && !call_from_timer )
            my__announce(target, moduleGetLangString(u2, LNG_VHOST_REQUEST_COUNT), ctr);
        else if( nick && !found )
            my__announce(target, moduleGetLangString(u2, LNG_NO_REQUEST), nick);
        else if( !call_from_timer )
            my__announce(target, moduleGetLangString(u2, LNG_VHOST_REQUEST_DISPLAYED), total, (total == 1 ? " " : "s "), ctr);
    }
    else if( !call_from_timer )
    {
        if( max == -1 )
            my__announce(target, moduleGetLangString(u2, LNG_VHOST_REQUEST_COUNT), ctr);
        else
            my__announce(target, moduleGetLangString(u2, LNG_NO_REQUESTS));
    }
    else if( nick )
        my__announce(target, moduleGetLangString(u2, LNG_NO_REQUEST), nick);

    
    if( call_from_timer && ctr )
        my__announce(target, moduleGetLangString(u2, LNG_VHOST_REMINDER), ctr, (ctr == 1 ? " ": "s "), vh->nick);
}

void my__show_entry(User *u, HostCore *ptr, boolean dur, boolean ischan)
{
User *u2 = NULL;
char *ident,
    *cvident,
    *cvhost,
    *target = vh->chan,
    *nick = ptr->nick,
    *s;
char durastr[192];
    if( !ischan )
        target = u->nick;
    
    if( (u2 = finduser(nick)) )
        my__announce(target, "- %s: \002%s\002 [%s@%s]", moduleGetLangString(u, LNG_KEY_NICK), u2->nick, u2->username, u2->host);
    else
        my__announce(target, "- %s: \002%s\002 [%s]", moduleGetLangString(u, LNG_KEY_NICK), nick, moduleGetLangString(u, LNG_KEY_OFFLINE));
        
    if( (cvhost = getvHost(nick)) )
    {
        if( (cvident = getvIdent(nick)) )
        {
            if( (ident = malloc(sizeof(char) * strlen(cvident) + sizeof(char) * 2 + 1)) )
            {
#ifdef HAVE_MEMPCPY
                ident = mempcpy(ident, "\002", sizeof(char));
                ident = mempcpy(ident, cvident, sizeof(char) * strlen(cvident));
                *((char *) mempcpy(ident, "\002", sizeof(char))) = '\0';
                ident -= sizeof(char) + sizeof(char) * strlen(cvident);
#else
                memcpy(ident, "\002", sizeof(char));
                memcpy(&ident[sizeof(char)], cvident, sizeof(char) * strlen(cvident));
                memcpy(&ident[sizeof(char) * (strlen(cvident) + 1)], "\002", sizeof(char));
                ident[sizeof(char) * (strlen(cvident) + 2)] = '\0';
#endif
            }
            else
                goto oom;
        }
        else
            ident = NULL;
    
        my__announce(target, "| %s: %s@\002%s\002", moduleGetLangString(u, LNG_KEY_CURRENT), (ident ? ident : "*"), cvhost);
        
        Anope_Free(ident);
    }
    else
        my__announce(target, "| %s: %s", moduleGetLangString(u, LNG_KEY_CURRENT), moduleGetLangString(u, LNG_KEY_NONE));
    
    if( ptr->vIdent )
    {
        if( (ident = malloc(sizeof(char) * strlen(ptr->vIdent) + sizeof(char) * 2 + 1)) )
        {
#ifdef HAVE_MEMPCPY
            ident = mempcpy(ident, "\002", sizeof(char));
            ident = mempcpy(ident, ptr->vIdent, sizeof(char) * strlen(ptr->vIdent));
            *((char *) mempcpy (ident, "\002", sizeof(char))) = '\0';
            ident -= sizeof(char) + sizeof(char) * strlen(ptr->vIdent);
#else
            memcpy(ident, "\002", sizeof(char));
            memcpy(&ident[sizeof(char)], ptr->vIdent, sizeof(char) * strlen(ptr->vIdent));
            memcpy(&ident[sizeof(char) * (strlen(ptr->vIdent) + 1)], "\002", sizeof(char));
            ident[sizeof(char) * (strlen(ptr->vIdent) + 2)] = '\0';
#endif
        }
        else
            goto oom;
    }
    else
        ident = NULL;
    
    if( dur )
    {
        duration(NULL, durastr, sizeof(durastr), time(NULL) - ptr->time);
        
        my__announce(target, "| %s: %s@\002%s\002 %s %s", moduleGetLangString(u, LNG_KEY_REQUEST), (ident ? ident : "*"), ptr->vHost, durastr, moduleGetLangString(u, LNG_KEY_AGO));
    }
    else
        my__announce(target, "| %s: %s@\002%s\002", moduleGetLangString(u, LNG_KEY_REQUEST), (ident ? ident : "*"), ptr->vHost);
    
    if( u2 && (s = moduleGetData(&u2->na->nc->moduleData, "exempt")) )
        my__announce(target, "| %s %s", moduleGetLangString(u, LNG_USER_IS_EXEMPT), s);
    
    Anope_Free(ident);
    return;
    
        oom:
    anope_cmd_global(vh->nick, "Unable to allocate memory to create the vIdent LL, problems i sense..");
}

void my__show_extended(User *u, char *chan, char *nick, char *vident, char *vhost, char *reason)
{
User *u2;
char *ident = NULL;
    if( !BadPtr(vident) )
    {
        if( (ident = malloc(sizeof(char) * strlen(vident) + sizeof(char) * 2 + 1)) )
        {
#ifdef HAVE_MEMPCPY
            ident = mempcpy(ident, "\002", sizeof(char));
            ident = mempcpy(ident, vident, sizeof(char) * strlen(vident));
            *((char *) mempcpy(ident, "\002", sizeof(char))) = '\0';
            ident -= sizeof(char) + sizeof(char) * strlen(vident);
#else
            memcpy(ident, "\002", sizeof(char));
            memcpy(&ident[sizeof(char)], vident, sizeof(char) * strlen(vident));
            memcpy(&ident[sizeof(char) * (strlen(vident) + 1)], "\002", sizeof(char));
            ident[sizeof(char) * (strlen(vident) + 2)] = '\0';
#endif
        }
        else
            goto oom;
    }

    my__announce(chan, "%s", reason ? coloredText[my__def_language()][GetFlag(&vh->flags, DISPLAYCOLOR)]._rejected_ext : coloredText[my__def_language()][GetFlag(&vh->flags, DISPLAYCOLOR)]._activated_ext);
    if( (u2 = finduser(nick)) )
        my__announce(chan, "- %s: \002%s\002 [%s@%s]", moduleGetLangString(NULL, LNG_KEY_NICK), nick, u2->username, u2->host);
    else
        my__announce(chan, "- %s: \002%s\002 [offline]", moduleGetLangString(NULL, LNG_KEY_NICK), nick);
    my__announce(chan, "| %s: \002%s\002", moduleGetLangString(NULL, LNG_KEY_OPER), u->nick);
    my__announce(chan, "%s %s: %s@\002%s\002", reason ? "|" : "-", moduleGetLangString(NULL, LNG_KEY_HOST), ident ? ident : "*", vhost);
    if( reason )
        my__announce(chan, "- %s: %s", moduleGetLangString(NULL, LNG_KEY_REASON), reason);
    
    Anope_Free(ident);
    return;
    
        oom:
    anope_cmd_global(vh->nick, "Unable to allocate memory to create the vIdent LL, problems i sense..");
}

/*************************************************************************/

int my__vhost_waiting_timer(int argc, char **argv)
{
    if( vh->timer )
    {
        if( debug )
            alog("%s: Reminding %s about open vhost requests.", NAME, vh->chan);
        my__show_list(NULL, NULL, NULL, true, true);
        
        if (moduleAddCallback("my__vhost_waiting_timer", time(NULL)+vh->timer, my__vhost_waiting_timer, 0, NULL) != MOD_ERR_OK)
            alog("%s: Failed to addCallback for open vhost requests reminder", NAME);
    }
    
    return MOD_CONT;
}

int my__vhost_cleanup_timer(int argc, char **argv)
{
NickCore *nc;
int32 time_left   = 0,
    time_called   = time(NULL);
int i;
char *time = NULL;
    
    //  periodically scan nickcore and remove
    //  outdated time data

    for( i = 0; i < 1024; i++ )
    {
        for( nc = nclists[i]; nc; nc = nc->next )
        {
            if( (time = moduleGetData(&nc->moduleData, "time"))
            && ((time_left = strtol(time, (char **) NULL, 16)) + 10800 - time_called) < 0 )   //  give auto-reject atleast 3 hours/10800 seconds to work properly
            {
                moduleDelData(&nc->moduleData, "time");
                free(time);
            }
        }
    }
    
    if( moduleAddCallback("my__vhost_cleanup_timer", time_called + 3600, my__vhost_cleanup_timer, 0, NULL) != MOD_ERR_OK )
        alog("%s: Failed to addCallback for cleanup timer", NAME);
    
    return MOD_CONT;
}

/*************************************************************************/

void my__announce(char *target, const char *fmt, ...)
{
va_list args;
char buf[4096];
char *s,
    *t;
    if( !fmt )
        return;

    va_start(args, fmt);
    memset(buf, 0, 4096);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    
    s = buf;
    while( *s )
    {
        t = s;
        s += strcspn(s, "\n");
        if( *s )
            *s++ = 0;

        if( *target == '#' )
            privmsg(vh->nick, target, (*t ? t : " "));
        else
            notice(s_HostServ, target, (*t ? t : " "));
    }
}

void my__announce_lang(User *u, char *target, int message, ...)
{
va_list args;
char buf[4096];
char *s,
    *t;
const char *fmt;
    if( !target || !message )
        return;

    va_start(args, message);
    fmt = getstring(u->na, message);
    if( !fmt )
        return;
        
    memset(buf, 0, 4096);
    vsnprintf(buf, sizeof(buf), fmt, args);
    
    s = buf;
    while( *s )
    {
        t = s;
        s += strcspn(s, "\n");
        if( *s )
            *s++ = 0;

        my__announce(target, (*t ? t : " "));
    }
    va_end(args);
}

void my__send_memo(User *u, char *name, int z, int number, ...)
{
va_list va;
User *u2;
char buffer[4096],
    outbuf[4096];
char *s,
    *t,
    *buf,
    *fmt = NULL;
int lang = LANG_EN_US;
    if( mod_current_module_name && (!mod_current_module || strcmp(mod_current_module_name, mod_current_module->name)) )
        mod_current_module = findModule(mod_current_module_name);

    u2 = finduser(name);
    
    //  find the users lang, and use it if we can
    
    if( u2 && u2->na && u2->na->nc )
        lang = u2->na->nc->language;

    //  if the users lang isnt supported, drop back to english
    
    if( !mod_current_module->lang[lang].argc )
        lang = LANG_EN_US;
    
    //  if the requested lang string exists for the language
    
    if( mod_current_module->lang[lang].argc > number )
    {
        fmt = mod_current_module->lang[lang].argv[number];

        buf = sstrdup(fmt);
        s = buf;
        while( *s )
        {
            t = s;
            s += strcspn(s, "\n");
            if( *s )
                *s++ = '\0';
            strscpy(outbuf, t, sizeof(outbuf));

            va_start(va, number);
            vsnprintf(buffer, 4095, outbuf, va);
            va_end(va);
            memo_send(u, name, buffer, z);
        }
        free(buf);
    }
    else
        alog("%s: Invalid language string call, language: [%d], String [%d]", NAME, lang, number);
}

void my__send_staff_memo(User *u, char *vhost)
{
int i,
    z = 2;
    if( checkDefCon(DEFCON_NO_NEW_MEMOS) )
        return;

    if( HasFlag(&vh->flags, MEMOOPER) )
    {
        for( i = 0; i < servopers.count; i++ )
            my__send_memo(u, (((NickCore *) servopers.list[i])->display), z, LNG_REQUEST_MEMO, vhost);
        for( i = 0; i < servadmins.count; i++ )
            my__send_memo(u, (((NickCore *) servadmins.list[i])->display),z, LNG_REQUEST_MEMO, vhost);
        for( i = 0; i < RootNumber; i++ )
            my__send_memo(u, ServicesRoots[i], z, LNG_REQUEST_MEMO, vhost);
    }
    if( HasFlag(&vh->flags, MEMOSETTERS) )
    {
        for( i = 0; i < HostNumber; i++ )
            my__send_memo(u, HostSetters[i], z, LNG_REQUEST_MEMO, vhost);
    }
}

/*************************************************************************/

void my__add_host_request(char *nick, char *vident, char *vhost, char *creator, int32 request_time)
{
HostCore *prev;
boolean found = false;
    if( !request_head )
        request_head = createHostCorelist(request_head, nick, vident, vhost, creator, request_time);
    else
    {
        prev = findHostCore(request_head, nick, &found);
        
        if( !found )
            request_head = insertHostCore(request_head, prev, nick, vident, vhost, creator, request_time);
        else
        {
            request_head = deleteHostCore(request_head, prev);
            my__add_host_request(nick, vident, vhost, creator, request_time);
        }
    }
}

boolean my__del_host_request(char *nick)
{
HostCore *prev;
boolean found = false;
    if( request_head )
    {
        prev = findHostCore(request_head, nick, &found);
        
        if( found )
            request_head = deleteHostCore(request_head, prev);
    }
    
    return found;
}

void my__activate_host_request(User *u, char *nick, char *vident, char *vhost, char *creator, int32 tmp_time, char *reason)
{
User *u2;
char activation_time[33]; // 4*8 uint32 + 1;
    addHostCore(nick, vident, vhost, creator, tmp_time);
    
    if( (u2 = finduser(nick)) )
    {
        anope_cmd_vhost_on(u2->nick, vident, vhost);
        
        sprintf(activation_time, "%X", (uint32) tmp_time);
        moduleAddData(&u2->na->nc->moduleData, "time", activation_time);
        
        if( vident )
            my__announce(u2->nick, moduleGetLangString(u2, LNG_HOST_IDENT_ACTIVATED), vident, vhost, reason ? reason : moduleGetLangString(u2, LNG_KEY_NONE));
        else
            my__announce(u2->nick, moduleGetLangString(u2, LNG_HOST_ACTIVATED), vhost, reason ? reason : moduleGetLangString(u2, LNG_KEY_NONE));
        
        if( ircd->vhost )
            u2->vhost = sstrdup(vhost);

        if( ircd->vident && vident )
            u2->vident = sstrdup(vident);
            
        set_lastmask(u2);
    }
    else if( HasFlag(&vh->flags, MEMOUSER) )
    {
        if( reason )
            my__send_memo(u, nick, 2, LNG_ACTIVATE_MEMO_REASON, reason);
        else
            my__send_memo(u, nick, 2, LNG_ACTIVATE_MEMO);
    }
}

/*************************************************************************/

char *my__decodespace(char *s)
{
static char *i,
    *o;
static char buf[512];
    for( i = s, o = buf; (*i) && (o < buf+510); i++ )
        if( *i == '_' )
        {
            if( i[1] != '_' )
                *o++ = ' ';
            else
            {
                *o++ = '_';
                i++;
            }
        }
        else
            *o++ = *i;
    *o = '\0';
    return buf;
}

/*************************************************************************/

void my__load_db(void)
{
FILE *fp;
int32 tmp_time;
char *filename,
    *buf,
    *nick,
    *vident,
    *vhost,
    *creator,
    *tmp,
    *reason,
    *by;
char readbuf[1024];
int dbnum,
    status;
    
    //  change this when there's more dbs to load, don't forget to add their code aswell
    //  and don't forget about my__hook_db_backup and my__save_db
    
    for( dbnum = HSREQ_DBNUM; dbnum; dbnum-- )
    {
        filename = NULL;
        
        switch( dbnum )
        {
            case 1: if( !(filename = vh->dbname) )
                            filename = HSREQ_DEFAULT_DBNAME;
                        break;
            case 2: if( !(filename = vh->regexdbname) )
                            filename = HSREQ_REGEX_DBNAME;
                        break;
            case 3: if( !(filename = vh->exceptiondbname) )
                            filename = HSREQ_EXCEPTION_DBNAME;
                        break;
        }
        
        if( !(fp = fopen(filename, "r")) )
        {
            alog("%s: Unable to open database ('%s') for reading", NAME, filename);
            continue;
        }

        while( fgets(readbuf, 1024, fp) )
        {
            buf = normalizeBuffer(readbuf);
            if( buf || *buf )
            {
                switch( dbnum )
                {
                    case 1: nick = myStrGetToken(buf, ':', 0);
                                vident = myStrGetToken(buf, ':', 1);
                                vhost  = myStrGetToken(buf, ':', 2);
                                tmp    = myStrGetToken(buf, ':', 3);

                                if( tmp )
                                {
                                    tmp_time = strtol(tmp, (char **) NULL, 16);
                                    free(tmp);
                                }
                                else
                                    tmp_time = 0;

                                creator = myStrGetToken(buf, ':', 4);
                                if( !nick || !vident || !vhost || !creator )
                                {
                                    alog("%s: Error while reading database ('%s'), skipping record", NAME, filename);
                                    continue;
                                }
                                
                                if( !stricmp(vident, "(null)") )
                                {
                                    free(vident);
                                    vident = NULL;
                                }
                                
                                my__add_host_request(nick, vident, vhost, creator, tmp_time);
                                
                                free(nick);
                                free(vhost);
                                free(creator);
                                Anope_Free(vident);
                                break;
                    case 2: nick = myStrGetToken(buf, ':', 0);
                                reason = myStrGetToken(buf, ':', 1);
                                tmp    = myStrGetTokenRemainder(buf, ':', 2);
                                
                                if( !nick || !reason || !tmp )
                                {
                                    alog("%s: Error while reading database ('%s'), skipping record", NAME, filename);
                                    
                                    Anope_Free(nick);
                                    Anope_Free(reason);
                                    Anope_Free(tmp);
                                    continue;
                                }
                                
                                if( !stricmp(reason, "(null)") )
                                {
                                    free(reason);
                                    reason = NULL;
                                }
                                
                                if( (status = my__add_regex(nick, reason, tmp)) )
                                    alog("%s: Could not add regex '%s'", NAME, tmp);
                                
                                free(nick);
                                Anope_Free(reason);
                                free(tmp);
                                break;
                    case 3: nick = myStrGetToken(buf, ':', 0);
                                by = myStrGetToken(buf, ':', 1);
                                
                                if( !nick || !by )
                                {
                                    alog("%s: Error while reading database ('%s'), skipping record", NAME, filename);
                                    Anope_Free(nick);
                                    Anope_Free(by);
                                    continue;
                                }
                                
                                my__add_exception(nick, by);
                                
                                free(nick);
                                free(by);
                                break;
                }
            }
            free(buf);
        }

        fclose(fp);

        if( debug )
            alog("%s: Succesfully loaded database ('%s')", NAME, filename);
    }
}

void my__save_db(void)
{
FILE *fp;
HostCore *current;
Regex *tmp;
Exception_t *tmp_e;
char *filename,
    *vident,
    *reason;
int dbnum;
    
    //  change this when there's more dbs to save, don't forget to add their code aswell
    //  and don't forget about my__hook_db_backup and my__load_db
    
    for( dbnum = HSREQ_DBNUM; dbnum; dbnum-- )
    {
        filename = NULL;
        
        switch( dbnum )
        {
            case 1: if( !(filename = vh->dbname) )
                            filename = HSREQ_DEFAULT_DBNAME;
                        break;
            case 2: if( !(filename = vh->regexdbname) )
                            filename = HSREQ_REGEX_DBNAME;
                        break;
            case 3: if( !(filename = vh->exceptiondbname) )
                            filename = HSREQ_EXCEPTION_DBNAME;
                        break;
        }
        
        if( !(fp = fopen(filename, "w")) )
        {
            alog("%s: Unable to open database ('%s') for writing", NAME, filename);
            return;
        }

        switch( dbnum )
        {
            case 1: current = request_head;
                        while( current )
                        {
                            vident = (current->vIdent ? current->vIdent : "(null)");
                            fprintf(fp, "%s:%s:%s:%X:%s\n", current->nick, vident, current->vHost, (uint32) current->time, current->creator);
                            current = current->next;
                        }
                        break;
            case 2: tmp = regex_head;
                        while( tmp )
                        {
                            reason = (tmp->reason ? tmp->reason : "(null)");
                            fprintf(fp, "%s:%s:%s\n", tmp->nick, reason, tmp->str);
                            tmp = tmp->next;
                        }
                        break;
            case 3: tmp_e = exception_head;
                        while( tmp_e )
                        {
                            fprintf(fp, "%s:%s\n", tmp_e->who, tmp_e->by);
                            tmp_e = tmp_e->next;
                        }
                        break;
        }

        fclose(fp);

        if( debug )
            alog("%s: Successfully saved database ('%s')", NAME, filename);
    }
}

int my__hook_db_saving(int argc, char **argv)
{
    if( argc >= 1 && !stricmp(argv[0], EVENT_START) )
        my__save_db();

    return MOD_CONT;
}

int my__hook_config_reload(int argc, char **argv)
{
    if( argc >= 1 && !stricmp(argv[0], EVENT_START) )
    {
        alog("%s: Reloading configuration directives...", NAME);
        my__load_config();
    }
    
    return MOD_CONT;
}

int my__hook_db_backup(int argc, char **argv)
{
time_t t;
struct tm tm;
char *db = (vh->dbname ? vh->dbname : HSREQ_DEFAULT_DBNAME),
    *rdb = (vh->regexdbname ? vh->regexdbname : HSREQ_REGEX_DBNAME),
    *edb = (vh->exceptiondbname ? vh->exceptiondbname : HSREQ_EXCEPTION_DBNAME);
char ext[9];
    if( argc >= 1 && !stricmp(argv[0], EVENT_START) )
    {
        time(&t);
        tm = *localtime(&t);
        strftime(ext, sizeof(ext), "%Y%m%d", &tm);
        
        ModuleRemoveBackups(db);
        ModuleRemoveBackups(rdb);
        ModuleRemoveBackups(edb);
        
        if (debug)
            alog("%s Database Backing up %s, %s, %s", NAME, db, rdb, edb);
        my__rename_database(db, ext);
        my__rename_database(rdb, ext);
        my__rename_database(edb, ext);
    }

    return MOD_CONT;
}

void my__rename_database(char *name, char *ext)
{
char destpath[PATH_MAX];
    snprintf(destpath, sizeof(destpath), "backups/%s.%s", name, ext);
    
    if( rename(name, destpath) )
    {
        alog("Backup of %s failed.", name);
        anope_cmd_global(s_OperServ, "WARNING! Backup of %s failed.", name);
    }
}

/*************************************************************************/

void SetFlag(uint32 *src, uint32 flag)
{
    *src |= flag;
}

void SetFlag_bool(uint32 *src, uint32 flag, boolean val)
{
    if( val )
        SetFlag(src, flag);
    else
        UnsetFlag(src, flag);
}

void SetFlag_int(uint32 *src, uint32 flag, int val)
{
    if( val == 1 )
        SetFlag(src, flag);
    else
        UnsetFlag(src, flag);
}

void UnsetFlag(uint32 *src, uint32 flag)
{
    *src &= ~flag;
}

const int GetFlag(uint32 *src, uint32 flag)
{
    return ((*src & flag) == flag ? 1 : 0);
}

const boolean HasFlag(uint32 *src, uint32 flag)
{
    return ((*src & flag) == flag ? true : false);
}

void my__load_config(void)
{
int MemoUser           = GetFlag(&def_settings->flags, MEMOUSER),
    MemoOper           = GetFlag(&def_settings->flags, MEMOOPER),
    MemoSetters        = GetFlag(&def_settings->flags, MEMOSETTERS),
    DisplayMode        = GetFlag(&def_settings->flags, DISPLAYMODE),
    DisplayColor       = GetFlag(&def_settings->flags, DISPLAYCOLOR),
    DisplayMax         = def_settings->displaymax,
    DenyIdentUpdate    = GetFlag(&def_settings->flags, DENYIDENTUPDATE),
    Timer              = def_settings->timer,
    RequestDelay       = def_settings->requestdelay;
char *Nick           = sstrdup(vh->nick),
    *Real            = sstrdup(vh->real),
    *Channel         = sstrdup(vh->chan),
    *DBName          = sstrdup(vh->dbname),
    *RegexDBName     = sstrdup(vh->regexdbname),
    *ExceptionDBName = sstrdup(vh->exceptiondbname);
    
    //  had a lot of trouble getting
    //  config parsing to work so this
    //  is the ridiculous workaround i
    //  came up with.
    
    Directive confvalues1[] = {
        {"vhMemoUser", {{PARAM_SET, PARAM_RELOAD, &MemoUser}}}
    };
    
    Directive confvalues2[] = {
        {"vhMemoOper", {{PARAM_SET, PARAM_RELOAD, &MemoOper}}}
    };
    
    Directive confvalues3[] = {
        {"vhMemoSetters", {{PARAM_SET, PARAM_RELOAD, &MemoSetters}}}
    };
    
    Directive confvalues4[] = {
        {"vhDisplayMode", {{PARAM_SET, PARAM_RELOAD, &DisplayMode}}}
    };
    
    Directive confvalues5[] = {
        {"vhDisplayColor", {{PARAM_SET, PARAM_RELOAD, &DisplayColor}}}
    };
    
    Directive confvalues6[] = {
        {"vhDisplayMax", {{PARAM_PORT, PARAM_RELOAD, &DisplayMax}}}
    };
    
    Directive confvalues7[] = {
        {"vhTimer", {{PARAM_TIME, PARAM_RELOAD, &Timer}}}
    };
    
    Directive confvalues8[] = {
        {"vhRequestDelay", {{PARAM_TIME, PARAM_RELOAD, &RequestDelay}}}
    };
    
    Directive confvalues9[] = {
        {"vhDenyIdentUpdate", {{PARAM_SET, PARAM_RELOAD, &DenyIdentUpdate}}}
    };
    
    Directive confvalues10[] = {
        {"vhDBName", {{PARAM_STRING, 0, &DBName}}}
    };
    
    Directive confvalues11[] = {
        {"vhRegexDBName", {{PARAM_STRING, 0, &RegexDBName}}}
    };
    
    Directive confvalues12[] = {
        {"vhExceptionDBName", {{PARAM_STRING, 0, &ExceptionDBName}}}
    };
    
    Directive confvalues13[] = {
        {"vhServName", {{PARAM_STRING, 0, &Nick},{PARAM_STRING, 0, &Real}}}
    };
    
    Directive confvalues14[] = {
        {"vhServChannel", {{PARAM_STRING, 0, &Channel}}}
    };
    
    moduleGetConfigDirective(confvalues14);
    moduleGetConfigDirective(confvalues13);
    moduleGetConfigDirective(confvalues12);
    moduleGetConfigDirective(confvalues11);
    moduleGetConfigDirective(confvalues10);
    moduleGetConfigDirective(confvalues9);
    moduleGetConfigDirective(confvalues8);
    moduleGetConfigDirective(confvalues7);
    moduleGetConfigDirective(confvalues6);
    moduleGetConfigDirective(confvalues5);
    moduleGetConfigDirective(confvalues4);
    moduleGetConfigDirective(confvalues3);
    moduleGetConfigDirective(confvalues2);
    moduleGetConfigDirective(confvalues1);
    
    hs_do_config_set_internal(NULL, "MemoUser", MemoUser, true);
    hs_do_config_set_internal(NULL, "MemoOper", MemoOper, true);
    hs_do_config_set_internal(NULL, "MemoSetters", MemoSetters, true);
    hs_do_config_set_internal(NULL, "DisplayMode", DisplayMode, true);
    hs_do_config_set_internal(NULL, "DisplayColor", DisplayColor, true);
    hs_do_config_set_internal(NULL, "DisplayMax", DisplayMax, true);
    hs_do_config_set_internal(NULL, "Timer", Timer, true);
    hs_do_config_set_internal(NULL, "RequestDelay", RequestDelay, true);
    hs_do_config_set_internal(NULL, "DenyIdentUpdate", DenyIdentUpdate, true);
    //  reload vhost for the following changes to take effect!
    if( vHostInternal )
    {
        vh->nick = sstrdup(Nick);
        vh->real = sstrdup(Real);
        vh->chan = sstrdup(Channel);
    }
    vh->dbname          = sstrdup(DBName);
    vh->regexdbname     = sstrdup(RegexDBName);
    vh->exceptiondbname = sstrdup(ExceptionDBName);
    
    if( debug )
    {
        alog("%s: MemoUser=%d MemoOper=%d MemoSetters=%d DisplayMode=%d DisplayColor=%d DisplayMax=%d Timer=%d RequestDelay=%d DenyIdentUpdate=%d", NAME, MemoUser, MemoOper, MemoSetters, DisplayMode, DisplayColor, DisplayMax, Timer, RequestDelay, DenyIdentUpdate);
        alog("%s: Nick='%s' Real='%s' Channel='%s' DBName='%s' RegexDBName='%s' ExceptionDBName='%s'", NAME, Nick, Real, Channel, DBName, RegexDBName, ExceptionDBName);
    }
    
    Anope_Free(Nick);
    Anope_Free(Real);
    Anope_Free(Channel);
    Anope_Free(DBName);
    Anope_Free(RegexDBName);
    Anope_Free(ExceptionDBName);
}

/* EOF */
