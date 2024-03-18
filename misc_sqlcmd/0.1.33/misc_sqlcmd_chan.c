#include "misc_sqlcmd.h"

 /*************************************************************
 *  misc_sqlcmd - Perform commands in Anope using a MySQL DB  *
 *************************************************************/

 /*
  * $Id: misc_sqlcmd_chan.c 20 2006-11-04 23:12:21Z heinz $
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
 * CHAN_REG - Register a channel
 *
 * av[0] - Nickname
 * av[1] - Password
 * av[2] - Channel name
 * av[3] - Channel password
 * av[4] - Channel description
 */
int sqlcmd_handle_chanreg(int ac, char **av)
{
        char *chan = av[2];
        char *pass = av[3];
        char *desc = av[4];
        NickAlias *na;
        NickCore *nc;
        Channel *c;
        ChannelInfo *ci;
        char founderpass[PASSMAX + 1];
        
        na = findnick(av[0]);

        if (readonly) 
                return SQLCMD_ERROR_READ_ONLY;
        else if (checkDefCon(DEFCON_NO_NEW_CHANNELS))
                return SQLCMD_ERROR_DEFCON;
        else if (*chan != '#')
                return SQLCMD_ERROR_CHAN_SYM_REQ;
        else if (!anope_valid_chan(chan))
                return SQLCMD_ERROR_CHAN_INVALID;
        else if (!na || !(nc = na->nc))
                return SQLCMD_ERROR_NICK_NOT_REGISTERED;
        else if (!enc_check_password(av[1], nc->pass))
                return SQLCMD_ERROR_ACCESS_DENIED;
        else if ((c = findchan(chan)))
                return SQLCMD_ERROR_CHAN_MUST_BE_EMPTY;
        else if ((ci = cs_findchan(chan)) != NULL) {
                if (ci->flags & CI_VERBOTEN) {
                        alog("%s: Attempt to register FORBIDden channel %s by %s", s_ChanServ, ci->name, av[0]);
                        return SQLCMD_ERROR_CHAN_FORBIDDEN;
                } else {
                        return SQLCMD_ERROR_CHAN_ALREADY_REG;
                }
        } else if (!stricmp(chan, "#"))
                return SQLCMD_ERROR_CHAN_MAY_NOT_BE_REG;
        else if (nc->channelmax > 0 && nc->channelcount >= nc->channelmax)
                return SQLCMD_ERROR_REACHED_CHAN_LIMIT;
        else if (stricmp(av[0], pass) == 0 || (StrictPasswords && strlen(pass) < 5))
                return SQLCMD_ERROR_MORE_OBSCURE_PASS;
        else if (!(ci = makechan(chan))) {
                alog("%s: makechan() failed for REGISTER %s", s_ChanServ, chan);
                return SQLCMD_ERROR_CHAN_REG_FAILED;
        } else if (strscpy(founderpass, pass, PASSMAX + 1), enc_encrypt_in_place(founderpass, PASSMAX) < 0) {
                alog("%s: Couldn't encrypt password for %s (REGISTER)", s_ChanServ, chan);
                delchan(ci);
                return SQLCMD_ERROR_CHAN_REG_FAILED;
        } else {
                ci->bantype = CSDefBantype;
                ci->flags = CSDefFlags;
                ci->mlock_on = ircd->defmlock;
                ci->memos.memomax = MSMaxMemos;
                ci->last_used = ci->time_registered;
                ci->founder = nc;
                memcpy(ci->founderpass, founderpass, PASSMAX);
                ci->desc = sstrdup(desc);
                /* Set this to something, otherwise it will maliform the topic */
                strscpy(ci->last_topic_setter, s_ChanServ, NICKMAX);        
                ci->bi = NULL;
                ci->botflags = BSDefFlags;
                ci->founder->channelcount++;
                alog("%s: Channel '%s' registered by %s", s_ChanServ, chan, av[0]);
                send_event(EVENT_CHAN_REGISTERED, 1, chan);
                return SQLCMD_ERROR_NONE;
        }
}

/*
 * CHAN_ADD_SOP - Add a SOP to channel
 *
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password
 * av[4] - User to add
 */
int sqlcmd_handle_chanaddsop(int ac, char **av)
{
        ChannelInfo *ci = NULL;
        
        if (readonly)
                return SQLCMD_ERROR_READ_ONLY;
        else if (!(ci = cs_findchan(av[2])))
                return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
        else if (ci->flags & CI_VERBOTEN)
                return SQLCMD_ERROR_CHAN_FORBIDDEN;
        else if (!(ci->flags & CI_XOP))
                return SQLCMD_ERROR_CHAN_NOT_XOP;        
        else if (sqlcmd_checkchanstatus(av[0], av[1], av[2], av[3], -1) != 1)
                return SQLCMD_ERROR_ACCESS_DENIED;
        
        return sqlcmd_do_xop_add(av[0], ACCESS_SOP, av[4], ci);
}

/*
 * CHAN_ADD_AOP - Add a AOP to channel
 *
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password
 * av[4] - User to add
 */
int sqlcmd_handle_chanaddaop(int ac, char **av)
{
        ChannelInfo *ci = NULL;
        
        if (readonly)
                return SQLCMD_ERROR_READ_ONLY;
        else if (!(ci = cs_findchan(av[2])))
                return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
        else if (ci->flags & CI_VERBOTEN)
                return SQLCMD_ERROR_CHAN_FORBIDDEN;
        else if (!(ci->flags & CI_XOP))
                return SQLCMD_ERROR_CHAN_NOT_XOP;
        else if (sqlcmd_checkchanstatus(av[0], av[1], av[2], av[3], ACCESS_SOP) < 0)
                return SQLCMD_ERROR_ACCESS_DENIED;
                
        return sqlcmd_do_xop_add(av[0], ACCESS_AOP, av[4], ci);
}

/*
 * CHAN_ADD_HOP - Add a HOP to channel
 *
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password
 * av[4] - User to add
 */
int sqlcmd_handle_chanaddhop(int ac, char **av)
{
        ChannelInfo *ci = NULL;
        
        if (!ircd->halfop)
                return SQLCMD_ERROR_UNSUPPORTED;
        else if (readonly)
                return SQLCMD_ERROR_READ_ONLY;
        else if (!(ci = cs_findchan(av[2])))
                return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
        else if (ci->flags & CI_VERBOTEN)
                return SQLCMD_ERROR_CHAN_FORBIDDEN;
        else if (!(ci->flags & CI_XOP))
                return SQLCMD_ERROR_CHAN_NOT_XOP;        
        else if (sqlcmd_checkchanstatus(av[0], av[1], av[2], av[3], ACCESS_AOP) < 0)
                return SQLCMD_ERROR_ACCESS_DENIED;
                
        return sqlcmd_do_xop_add(av[0], ACCESS_HOP, av[4], ci);
}

/*
 * CHAN_ADD_VOP - Add a VOP to channel
 *
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password
 * av[4] - User to add
 */
int sqlcmd_handle_chanaddvop(int ac, char **av)
{
        ChannelInfo *ci = NULL;
        
        if (readonly)
                return SQLCMD_ERROR_READ_ONLY;
        else if (!(ci = cs_findchan(av[2])))
                return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
        else if (ci->flags & CI_VERBOTEN)
                return SQLCMD_ERROR_CHAN_FORBIDDEN;
        else if (!(ci->flags & CI_XOP))
                return SQLCMD_ERROR_CHAN_NOT_XOP;
        else if (sqlcmd_checkchanstatus(av[0], av[1], av[2], av[3], ACCESS_AOP) < 0)
                return SQLCMD_ERROR_ACCESS_DENIED;
                
        return sqlcmd_do_xop_add(av[0], ACCESS_VOP, av[4], ci);
}

/*
 * CHAN_DEL_SOP - Delete a SOP from channel
 *
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password
 * av[4] - User to delete
 */
int sqlcmd_handle_chandelsop(int ac, char **av)
{
        ChannelInfo *ci = NULL;
        
        if (readonly)
                return SQLCMD_ERROR_READ_ONLY;
        else if (!(ci = cs_findchan(av[2])))
                return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
        else if (ci->flags & CI_VERBOTEN)
                return SQLCMD_ERROR_CHAN_FORBIDDEN;
        else if (!(ci->flags & CI_XOP))
                return SQLCMD_ERROR_CHAN_NOT_XOP;
        else if (sqlcmd_checkchanstatus(av[0], av[1], av[2], av[3], -1) != 1)
                return SQLCMD_ERROR_ACCESS_DENIED;
                
        return sqlcmd_do_xop_del(av[0], ACCESS_SOP, av[4], ci);
}

/*
 * CHAN_DEL_AOP - Delete a AOP from channel
 *
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password
 * av[4] - User to delete
 */
int sqlcmd_handle_chandelaop(int ac, char **av)
{
        ChannelInfo *ci = NULL;
        
        if (readonly)
                return SQLCMD_ERROR_READ_ONLY;
        else if (!(ci = cs_findchan(av[2])))
                return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
        else if (ci->flags & CI_VERBOTEN)
                return SQLCMD_ERROR_CHAN_FORBIDDEN;
        else if (!(ci->flags & CI_XOP))
                return SQLCMD_ERROR_CHAN_NOT_XOP;
        else if (sqlcmd_checkchanstatus(av[0], av[1], av[2], av[3], ACCESS_SOP) < 0)
                return SQLCMD_ERROR_ACCESS_DENIED;
                
        return sqlcmd_do_xop_del(av[0], ACCESS_AOP, av[4], ci);
}

/*
 * CHAN_DEL_HOP - Delete a HOP from channel
 *
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password
 * av[4] - User to delete
 */
int sqlcmd_handle_chandelhop(int ac, char **av)
{
        ChannelInfo *ci = NULL;
        
        if (!ircd->halfop)
                return SQLCMD_ERROR_UNSUPPORTED;     
        else if (readonly)
                return SQLCMD_ERROR_READ_ONLY;
        else if (!(ci = cs_findchan(av[2])))
                return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
        else if (ci->flags & CI_VERBOTEN)
                return SQLCMD_ERROR_CHAN_FORBIDDEN;
        else if (!(ci->flags & CI_XOP))
                return SQLCMD_ERROR_CHAN_NOT_XOP;                
        else if (sqlcmd_checkchanstatus(av[0], av[1], av[2], av[3], ACCESS_AOP) < 0)
                return SQLCMD_ERROR_ACCESS_DENIED;
                
        return sqlcmd_do_xop_del(av[0], ACCESS_HOP, av[4], ci);
}

/*
 * CHAN_DEL_VOP - Delete a VOP from channel
 *
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password
 * av[4] - User to delete
 */
int sqlcmd_handle_chandelvop(int ac, char **av)
{
        ChannelInfo *ci = NULL;
        
        if (!ircd->halfop)
                return SQLCMD_ERROR_UNSUPPORTED;     
        else if (readonly)
                return SQLCMD_ERROR_READ_ONLY;
        else if (!(ci = cs_findchan(av[2])))
                return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
        else if (ci->flags & CI_VERBOTEN)
                return SQLCMD_ERROR_CHAN_FORBIDDEN;
        else if (!(ci->flags & CI_XOP))
                return SQLCMD_ERROR_CHAN_NOT_XOP;                
        else if (sqlcmd_checkchanstatus(av[0], av[1], av[2], av[3], ACCESS_AOP) < 0)
                return SQLCMD_ERROR_ACCESS_DENIED;
                
        return sqlcmd_do_xop_del(av[0], ACCESS_VOP, av[4], ci);
}

/*
 * CHAN_ACCESS - Add/Change/Remove access from channel
 *
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password
 * av[4] - User
 * av[5] - Level
 */
int sqlcmd_handle_chanaccess(int ac, char **av)
{
        if (sqlcmd_checkchanstatus(av[0], av[1], av[2], av[3], -1) < 0)
                return SQLCMD_ERROR_ACCESS_DENIED;
        return SQLCMD_ERROR_NOT_IMPLEMENTED;
}

/*
 * CHAN_TOPIC - Change the topic of a channel
 *
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password
 * av[4] - New topic
 */
int sqlcmd_handle_chantopic(int ac, char **av)
{
        Channel *c;
        ChannelInfo *ci;
               
        if (sqlcmd_checkchanstatus(av[0], av[1], av[2], av[3], CA_TOPIC) < 0)
                return SQLCMD_ERROR_ACCESS_DENIED;
        else if (!(c = findchan(av[2])))
                return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
        else if (!(ci = c->ci))
                return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
        else if (ci->flags & CI_VERBOTEN)
                return SQLCMD_ERROR_CHAN_FORBIDDEN;
        else {
                if (ci->last_topic)
                        free(ci->last_topic);
                ci->last_topic = av[4] ? sstrdup(av[4]) : NULL;
                strscpy(ci->last_topic_setter, av[0], NICKMAX);
                ci->last_topic_time = time(NULL);
                if (c->topic)
                        free(c->topic);
                c->topic = av[4] ? sstrdup(av[4]) : NULL;
                strscpy(c->topic_setter, av[0], NICKMAX);
                if (ircd->topictsbackward)
                        c->topic_time = c->topic_time - 1;
                else
                        c->topic_time = ci->last_topic_time;
                if (ircd->join2set) {
                        if (whosends(ci) == s_ChanServ) {
                                anope_cmd_join(s_ChanServ, c->name, time(NULL));
                                anope_cmd_mode(NULL, c->name, "+o %s", s_ChanServ);
                        }
                }
                anope_cmd_topic(whosends(ci), c->name, av[0], av[4], c->topic_time);
                if (ircd->join2set) {
                        if (whosends(ci) == s_ChanServ) {
                                anope_cmd_part(s_ChanServ, c->name, NULL);
                        }
                }
                return SQLCMD_ERROR_NONE;
        }
}

/*
 * CHAN_DROP - Drop a channel
 *
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password
 */
int sqlcmd_handle_chandrop(int ac, char **av)
{
        char *chan = av[2];
        ChannelInfo *ci;

        if (readonly)
                return SQLCMD_ERROR_READ_ONLY;
        else if (!chan)
                return SQLCMD_ERROR_SYNTAX_ERROR;
        else if (!(ci = cs_findchan(chan)))
                return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
        else if (ci->flags & CI_VERBOTEN)
                return SQLCMD_ERROR_CHAN_FORBIDDEN;
        else if (ci->flags & CI_SUSPENDED)
                return SQLCMD_ERROR_CHAN_FORBIDDEN;
        else if (sqlcmd_checkchanstatus(av[0], av[1], av[2], av[3], -1) != 1)
                return SQLCMD_ERROR_ACCESS_DENIED;
        else {
                if (ci->c) {
                        if (ircd->regmode) {
                                ci->c->mode &= ~ircd->regmode;
                                anope_cmd_mode(whosends(ci), ci->name, "-r");
                        }
                }
                if (ircd->chansqline && (ci->flags & CI_VERBOTEN)) {
                        anope_cmd_unsqline(ci->name);
                }

                alog("%s: Channel %s dropped by %s (founder: %s)", s_ChanServ, ci->name, av[0], (ci->founder ? ci->founder->display : "(none)"));
                delchan(ci);                
                send_event(EVENT_CHAN_DROP, 1, chan);
                return SQLCMD_ERROR_NONE;
        }
}

int sqlcmd_do_xop_add(char *nick, int xop_lvl, char *xop_nick, ChannelInfo *ci)
{
        NickAlias *na = NULL;
        NickCore *nc = NULL;
        ChanAccess *access = NULL;
        char event_access[BUFSIZE];
        int change = 0;
        int i = 0;
        
        na = findnick(xop_nick);
        if (!na)
                return SQLCMD_ERROR_NICK_NOT_REGISTERED;
        else if (na->status & NS_VERBOTEN)
                return SQLCMD_ERROR_NICK_FORBIDDEN;      

        nc = na->nc;
        for (access = ci->access, i = 0; i < ci->accesscount; access++, i++) {
                if (access->nc == nc) {
                        change++;
                        break;
                }
        }
        
        if (!change) {
                for (i = 0; i < ci->accesscount; i++)
                        if (!ci->access[i].in_use)
                                break;

                if (i == ci->accesscount) {
                        if (i < CSAccessMax) {
                                ci->accesscount++;
                                ci->access = srealloc(ci->access, sizeof(ChanAccess) * ci->accesscount);
                        } else
                                return SQLCMD_ERROR_CHAN_XOP_REACHED_LIMIT;
                }
                
                access = &ci->access[i];
                access->nc = nc;
        }

        access->in_use = 1;
        access->level = xop_lvl;
        access->last_seen = 0;
        
        alog("%s: %s %s access level %d to %s (group %s) on channel %s", s_ChanServ, nick, change ? "changed" : "set", access->level, na->nick, nc->display, ci->name);
        snprintf(event_access, BUFSIZE, "%d", access->level);

        if (!change)
                send_event(EVENT_ACCESS_CHANGE, 4, ci->name, nick, na->nick, event_access);
        else
                send_event(EVENT_ACCESS_ADD, 4, ci->name, nick, na->nick, event_access);

        return SQLCMD_ERROR_NONE;
}

int sqlcmd_do_xop_del(char *nick, int xop_lvl, char *xop_nick, ChannelInfo *ci)
{
        NickAlias *na = NULL;
        NickCore *nc = NULL;
        ChanAccess *access = NULL;
        int i;
        int a;
        int b;

        na = findnick(xop_nick);
        if (!na)
                return SQLCMD_ERROR_NICK_NOT_REGISTERED;

        nc = na->nc;
        for (i = 0; i < ci->accesscount; i++)
                if (ci->access[i].nc == nc && ci->access[i].level == xop_lvl)
                        break;

        if (i == ci->accesscount)
                return SQLCMD_ERROR_CHAN_XOP_NOT_FOUND;
       
        access = &ci->access[i];
        send_event(EVENT_ACCESS_DEL, 3, ci->name, nick, na->nick);
        access->nc = NULL;
        access->in_use = 0;
        
        for (b = 0; b < ci->accesscount; b++) {
                if (ci->access[b].in_use) {
                        for (a = 0; a < ci->accesscount; a++) {
                                if (a > b)
                                        break;
                                if (!ci->access[a].in_use) {
                                        ci->access[a].in_use = 1;
                                        ci->access[a].level = ci->access[b].level;
                                        ci->access[a].nc = ci->access[b].nc;
                                        ci->access[a].last_seen = ci->access[b].last_seen;
                                        ci->access[b].nc = NULL;
                                        ci->access[b].in_use = 0;
                                        break;
                                }
                        }
                }
        }        
    
        return SQLCMD_ERROR_NONE;
}
