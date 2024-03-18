#include "misc_sqlcmd.h"

 /*************************************************************
 *  misc_sqlcmd - Perform commands in Anope using a MySQL DB  *
 *************************************************************/

 /*
  * $Id: misc_sqlcmd_nick.c 18 2006-10-30 18:09:53Z heinz $
  */
 
 /**************************************************************
 * Module Info & Changelog      
 * -----------------------                                     
 * Please see misc_sqlcmd_main.c for the information, changelog
 * and the configuration information.
 *
 **************************************************************/
 
 /**************************************************************
 *                                                             *
 * PLEASE DO NOT EDIT ANYTHING BELOW HERE - MODULE CODE BEGINS *
 *                                                             *
 **************************************************************/


/*
 * NICK_REG - Register a nickname
 *
 * av[0] - Nickname
 * av[1] - Password
 * av[2] - E-Mail Address
 */
int sqlcmd_handle_nickreg(int ac, char **av)
{
        User *u = NULL;
        NickRequest *nr = NULL, *anr = NULL;
        NickCore *nc = NULL;
        NickAlias *na = NULL;
        int prefixlen = strlen(NSGuestNickPrefix);
        int nicklen = strlen(av[0]);
        char *pass = av[1];
        char *email = av[2];
        char passcode[11];
        int idx, min = 1, max = 62, i = 0;
        int chars[] =
                { ' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
                'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
                'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
                'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
                'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
        };
        
        if (debug)
                alog("debug: [%s] Processing nickname registration for '%s'", MODNAME, av[0]);        
        if (!pass || (NSForceEmail && !email))
                return SQLCMD_ERROR_SYNTAX_ERROR;  
        if ((na = findnick(av[0])))
                return SQLCMD_ERROR_NICK_ALREADY_REGISTERED;
        if (readonly)
                return SQLCMD_ERROR_READ_ONLY;
        if (checkDefCon(DEFCON_NO_NEW_NICKS))
                return SQLCMD_ERROR_DEFCON;
        if ((anr = findrequestnick(av[0])))
                return SQLCMD_ERROR_NICK_ALREADY_REGISTERED;
        if (nicklen <= prefixlen + 7 && nicklen >= prefixlen + 1 && stristr(av[0], NSGuestNickPrefix) == av[0] && strspn(av[0] + prefixlen, "1234567890") == nicklen - prefixlen)
                return SQLCMD_ERROR_NICK_FORBIDDEN;
        if (!anope_valid_nick(av[0]))
                return SQLCMD_ERROR_NICK_FORBIDDEN;
        if (RestrictOperNicks) {
                for (i = 0; i < RootNumber; i++) {
                        if (stristr(av[0], ServicesRoots[i]))
                                return SQLCMD_ERROR_NICK_CANNOT_BE_REGISTERED;
                }
        }
        
        for (i = 0; i < servadmins.count && (nc = servadmins.list[i]); i++) {
                if (stristr(av[0], nc->display))
                        return SQLCMD_ERROR_NICK_CANNOT_BE_REGISTERED;
        }
        
        for (i = 0; i < servopers.count && (nc = servopers.list[i]); i++) {
                if (stristr(av[0], nc->display))
                        return SQLCMD_ERROR_NICK_CANNOT_BE_REGISTERED;
        }
    
        u = finduser(av[0]);
        if (u && u->na) {
                if (u->na->status & NS_VERBOTEN) {
                        alog("[%s] User tried to register FORBIDden nick %s", MODNAME, av[0]);
                        return SQLCMD_ERROR_NICK_CANNOT_BE_REGISTERED;
                } else {
                        return SQLCMD_ERROR_NICK_ALREADY_REGISTERED;
                }
        } else if (stricmp(av[0], pass) == 0 || (StrictPasswords && strlen(pass) < 5)) {
                return SQLCMD_ERROR_MORE_OBSCURE_PASS;
        } else if (email && !MailValidate(email)) {
                return SQLCMD_ERROR_EMAIL_INVALID;
        } else {
                if (strlen(pass) > PASSMAX) {
                        pass[PASSMAX] = 0;
                }
                for (idx = 0; idx < 9; idx++) {
                        passcode[idx] = chars[(1 + (int) (((float) (max - min)) * getrandom16() / (65535 + 1.0)) + min)];
                } passcode[idx] = '\0';
                nr = makerequest(av[0]);
                nr->passcode = sstrdup(passcode);
                nr->password = sstrdup(pass);
                if (email) {
                        nr->email = sstrdup(email);
                }
                nr->requested = time(NULL);
                if (av[2])
                        free(av[2]);
                av[2] = sstrdup(passcode);
                if (debug)
                        alog("debug: [%s] Nickname request for '%s' generated.. Auto-confirming..", MODNAME, av[0]);
                return sqlcmd_handle_nickconf(ac, av);
        }
}

/*
 * NICK_CONF - Confirm a nickname
 *
 * av[0] - Nickname
 * av[1] - Password
 * av[2] - Passcode
 */
int sqlcmd_handle_nickconf(int ac, char **av)
{
        NickRequest *nr = NULL;
        NickAlias *na = NULL;
        char *passcode = av[2];
        char *pass = NULL;
        char *email = NULL;
        int len;
        int i;

        nr = findrequestnick(av[0]);

        if (debug)
                alog("debug: [%s] Confirming nickname request for '%s'...", MODNAME, av[0]);

        if (NSEmailReg) {
                if (!passcode)
                        return SQLCMD_ERROR_SYNTAX_ERROR;
                if (!nr)
                        return SQLCMD_ERROR_NICK_CONF_NOT_FOUND; 
                if (stricmp(nr->passcode, passcode) != 0)
                        return SQLCMD_ERROR_NICK_CONF_INVALID; 
        } 
        if (!nr)
                return SQLCMD_ERROR_NICK_CONF_NOT_FOUND; 

        if (stricmp(av[1], nr->password) != 0)
                return SQLCMD_ERROR_NICK_CONF_INVALID; 

        pass = sstrdup(nr->password);
        if (nr->email)
                email = sstrdup(nr->email);
        na = makenick(nr->nick);

        if (na) {
                len = strlen(pass);
                na->nc->pass = smalloc(PASSMAX);
                if (enc_encrypt(pass, len, na->nc->pass, PASSMAX) < 0) {
                        memset(pass, 0, strlen(pass));
                        alog("%s: Failed to encrypt password for %s (register)", s_NickServ, nr->nick);
                        if (email)
                                free(email);
                        if (pass)
                                free(pass);
                        return SQLCMD_ERROR_NICK_REG_FAILED;
                }
                memset(pass, 0, strlen(pass));
                na->status = (int16) (NS_IDENTIFIED | NS_RECOGNIZED);
                na->nc->flags |= NSDefFlags;
                for (i = 0; i < RootNumber; i++) {
                        if (!stricmp(ServicesRoots[i], nr->nick)) {
                                na->nc->flags |= NI_SERVICES_ROOT;
                                break;
                        }
                }
                na->nc->memos.memomax = MSMaxMemos;
                na->nc->channelmax = CSMaxReg;
                na->last_usermask = sstrdup("*@unknown.host");
                na->last_realname = sstrdup("unknown");
                na->time_registered = na->last_seen = time(NULL);
                na->nc->accesscount = 0;
                na->nc->access = NULL;
                na->nc->language = NSDefLanguage;
                if (email)
                        na->nc->email = sstrdup(email);
                send_event(EVENT_NICK_REGISTERED, 1, av[0]);                
                delnickrequest(nr);     /* remove the nick request */	    
                if (email)
                        free(email);
                if (pass)
                        free(pass);
                if (debug)
                        alog("debug: [%s] Nickname registration for '%s' completed successfully!", MODNAME, av[0]);
                return SQLCMD_ERROR_NONE;

        } else {
                if (email)
                        free(email);
                if (pass)
                        free(pass);        
                return SQLCMD_ERROR_NICK_REG_FAILED;
        }
}

/*
 * NICK_GROUP - Group a nickname
 *
 * av[0] - Existing Nickname
 * av[1] - Password
 * av[2] - Nick to group
 */
int sqlcmd_handle_nickgroup(int ac, char **av)
{ 
        NickAlias *na, *target, *newalias;
        NickCore *nc;
        User *u;
        int i;
        
        newalias = findnick(av[2]);
        
        if (NSEmailReg && (findrequestnick(av[2])))
                return SQLCMD_ERROR_NICK_ALREADY_REGISTERED;
        else if (readonly)
                return SQLCMD_ERROR_READ_ONLY;
        else if (checkDefCon(DEFCON_NO_NEW_NICKS))
                return SQLCMD_ERROR_DEFCON;
        else if (RestrictOperNicks) {
                for (i = 0; i < RootNumber; i++) {
                        if (stristr(av[2], ServicesRoots[i]))
                                return SQLCMD_ERROR_NICK_CANNOT_BE_REGISTERED;               
                }   
                for (i = 0; i < servadmins.count && (nc = servadmins.list[i]); i++) {
                        if (stristr(av[2], nc->display))
                                return SQLCMD_ERROR_NICK_CANNOT_BE_REGISTERED;
                }
                for (i = 0; i < servopers.count && (nc = servopers.list[i]); i++) {
                        if (stristr(av[2], nc->display))
                                return SQLCMD_ERROR_NICK_CANNOT_BE_REGISTERED;
                }
        }
        if (!(target = findnick(av[0])))
                return SQLCMD_ERROR_NICK_NOT_REGISTERED;
        else if ((u = finduser(av[0])))
                return SQLCMD_ERROR_NICK_IN_USE;
        else if (newalias && (newalias->status & NS_VERBOTEN)) {
                alog("%s: %s tried to use GROUP from FORBIDden nick %s", s_NickServ, av[0], av[2]);
                return SQLCMD_ERROR_NICK_FORBIDDEN;
        } else if (newalias && (newalias->nc->flags & NI_SUSPENDED)) {
                alog("%s: %s tried to use GROUP from SUSPENDED nick %s", s_NickServ, av[2], av[0]);
                return SQLCMD_ERROR_NICK_SUSPENDED;
        } else if (newalias && NSNoGroupChange)
                return SQLCMD_ERROR_NICK_GROUP_CHANGE_DISABLED;
        else if (target && (target->nc->flags & NI_SUSPENDED)) {
                alog("%s: %s tried to use GROUP from SUSPENDED nick %s", s_NickServ, av[2], av[0]);
                return SQLCMD_ERROR_NICK_SUSPENDED;
        } else if (target->status & NS_VERBOTEN)
                return SQLCMD_ERROR_NICK_FORBIDDEN;
        else if (newalias && target->nc == newalias->nc)
                return SQLCMD_ERROR_NICK_GROUP_SAME;
        else if (NSMaxAliases && (target->nc->aliases.count >= NSMaxAliases) && !nick_is_services_admin(target->nc))
                return SQLCMD_ERROR_NICK_GROUP_TOO_MANY;
        else if (enc_check_password(av[1], target->nc->pass) != 1) {
                alog("%s: Failed GROUP for %s (invalid password)", s_NickServ, av[2]);
                return SQLCMD_ERROR_ACCESS_DENIED;
        } else {
                /* If the nick is already registered, drop it.
                 * If not, check that it is valid.
                 */
                if (newalias) {
                        delnick(newalias);
                } else {
                        int prefixlen = strlen(NSGuestNickPrefix);
                        int nicklen = strlen(av[2]);
                        if (nicklen <= prefixlen + 7 && nicklen >= prefixlen + 1 && stristr(av[2], NSGuestNickPrefix) == av[2] && strspn(av[2] + prefixlen, "1234567890") == nicklen - prefixlen)
                                return SQLCMD_ERROR_NICK_CANNOT_BE_REGISTERED;
                }
                na = makealias(av[2], target->nc);
                if (na) {
                        na->last_usermask = scalloc(13, 1);
                        sprintf(na->last_usermask, "unknown@host");
                        na->last_realname = sstrdup(target->last_realname);
                        na->time_registered = na->last_seen = time(NULL);
                        na->status = (int16) (NS_IDENTIFIED | NS_RECOGNIZED);
                        if (!(na->nc->flags & NI_SERVICES_ROOT)) {
                                for (i = 0; i < RootNumber; i++) {
                                        if (!stricmp(ServicesRoots[i], av[2])) {
                                                na->nc->flags |= NI_SERVICES_ROOT;
                                                break;
                                        }
                                }
                        }
#ifdef USE_RDB
                        /* Is this really needed? Since this is a new alias it will get
                         * its unique id on the next update, since it was previously
                        * deleted by delnick. Must observe...
                        */
                        if (rdb_open()) {
                                rdb_save_ns_alias(na);
                                rdb_close();
                        }
#endif
                        send_event(EVENT_GROUP, 1, av[2]);
                        alog("%s: %s makes %s join group of %s (%s) (e-mail: %s)", s_NickServ, av[2], av[2], target->nick, target->nc->display, (target->nc->email ? target->nc->email : "none"));
                        return SQLCMD_ERROR_NONE;
                } else {
                        alog("%s: makealias(%s) failed", s_NickServ, av[2]);
                        return SQLCMD_ERROR_NICK_REG_FAILED;
                }
        }
}

/*
 * NICK_DROP - Drop a nickname
 *
 * av[0] - Nickname
 * av[1] - Password
 */
int sqlcmd_handle_nickdrop(int ac, char **av)
{ 
        NickAlias *na;
        
        if (readonly)
                return SQLCMD_ERROR_READ_ONLY;
        else if (!(na = findnick(av[0])))
                return SQLCMD_ERROR_NICK_NOT_REGISTERED;
        else if (enc_check_password(av[1], na->nc->pass) != 1)
                return SQLCMD_ERROR_ACCESS_DENIED;
        else {
                if (ircd->sqline && (na->status & NS_VERBOTEN)) {
                        anope_cmd_unsqline(na->nick);
                }
                alog("%s: %s dropped nickname %s (group %s) (e-mail: %s)", s_NickServ, av[0], na->nick, na->nc->display, (na->nc->email ? na->nc->email : "none"));
                delnick(na);
                send_event(EVENT_NICK_DROPPED, 1, av[0]);
                return SQLCMD_ERROR_NONE;
        }
}

NickRequest *makerequest(const char *nick)
{
        NickRequest *nr;
        nr = scalloc(1, sizeof(NickRequest));
        nr->nick = sstrdup(nick);
        insert_requestnick(nr);
        alog("%s: Nick %s has been requested", s_NickServ, nr->nick);
        return nr;
}

NickAlias *makenick(const char *nick)
{
        NickAlias *na;
        NickCore *nc;

        /* First make the core */
        nc = scalloc(1, sizeof(NickCore));
        nc->display = sstrdup(nick);
        slist_init(&nc->aliases);
        insert_core(nc);
        alog("%s: group %s has been created", s_NickServ, nc->display);
        /* Then make the alias */
        na = scalloc(1, sizeof(NickAlias));
        na->nick = sstrdup(nick);
        na->nc = nc;
        slist_add(&nc->aliases, na);
        alpha_insert_alias(na);
        return na;
}

NickAlias *makealias(const char *nick, NickCore * nc)
{
        NickAlias *na;

        /* Just need to make the alias */
        na = scalloc(1, sizeof(NickAlias));
        na->nick = sstrdup(nick);
        na->nc = nc;
        slist_add(&nc->aliases, na);
        alpha_insert_alias(na);
        return na;
}
