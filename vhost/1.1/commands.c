#include "commands.h"

#include "activate.h"
#include "help.h"
#include "list.h"
#include "reject.h"
#include "request.h"
#include "waiting.h"
#include "cmd_meow.h"
#include "cmd_vhost.h"
#include "conf.h"

void my__add_message_list(void)
{
Command *c;
    c = createCommand("request", hs_call_request, nick_identified, -1, -1, -1, -1, -1);
    moduleAddHelp(c, hs_help_request);
    moduleAddCommand(HOSTSERV, c, MOD_HEAD);

    c = createCommand("a", hs_call_activate, is_host_setter, -1, -1, -1, -1, -1);
    moduleAddHelp(c, hs_help_activate);
    moduleAddCommand(HOSTSERV, c, MOD_HEAD);
    c = createCommand("activate", hs_call_activate, is_host_setter, -1, -1, -1, -1, -1);
    moduleAddHelp(c, hs_help_activate);
    moduleAddCommand(HOSTSERV, c, MOD_HEAD);

    c = createCommand("r", hs_call_reject, is_host_setter, -1, -1, -1, -1, -1);
    moduleAddHelp(c, hs_help_reject);
    moduleAddCommand(HOSTSERV, c, MOD_HEAD);
    c = createCommand("reject", hs_call_reject, is_host_setter, -1, -1, -1, -1, -1);
    moduleAddHelp(c, hs_help_reject);
    moduleAddCommand(HOSTSERV, c, MOD_HEAD);
    
    c = createCommand("actall", hs_call_activate_all, is_host_setter, -1, -1, -1, -1, -1);
    moduleAddHelp(c, hs_help_actall);
    moduleAddCommand(HOSTSERV, c, MOD_HEAD);

    c = createCommand("rejall", hs_call_reject_all, is_host_setter, -1, -1, -1, -1, -1);
    moduleAddHelp(c, hs_help_rejall);
    moduleAddCommand(HOSTSERV, c, MOD_HEAD);

    c = createCommand("w", hs_call_waiting, is_host_setter, -1, -1, -1, -1, -1);
    moduleAddHelp(c, hs_help_waiting);
    moduleAddCommand(HOSTSERV, c, MOD_HEAD);
    c = createCommand("waiting", hs_call_waiting, is_host_setter, -1, -1, -1, -1, -1);
    moduleAddHelp(c, hs_help_waiting);
    moduleAddCommand(HOSTSERV, c, MOD_HEAD);

    c = createCommand("list", hs_call_list_out, is_services_oper, -1, -1, -1, -1, -1);
    moduleAddCommand(HOSTSERV, c, MOD_HEAD);
    
    c = createCommand("config", hs_call_config, is_services_admin, -1, -1, -1, -1, -1);
    moduleAddHelp(c, hs_help_config);
    moduleAddCommand(HOSTSERV, c, MOD_HEAD);
    
    moduleSetHostHelp(hs_help);
    
    c = createCommand("info", ns_call_info, nick_identified, -1, -1, -1, -1, -1);
    moduleAddCommand(NICKSERV, c, MOD_TAIL);
    
    c = createCommand("meow", do_meow, NULL, -1, -1, -1, -1, -1);
    moduleAddCommand(vHost_cmdTable, c, MOD_UNIQUE);
    
    c = createCommand("!vhost", do_vhost, is_host_setter, -1, -1, -1, -1, -1);
    moduleAddCommand(vHost_cmdTable, c, MOD_UNIQUE);
}

int ns_call_info(User *u)
{
char *buf = moduleGetLastBuffer(),
    *nick = myStrGetToken(buf, ' ', 0),
    *s    = NULL;
NickAlias *na;
    
    if( (na = findnick(nick)) && (s = moduleGetData(&na->nc->moduleData, "exempt")) )
        my__announce(u->nick, "Nick is exempt by %s", s);
    
    Anope_Free(nick);
    return MOD_CONT;
}

/* EOF */
