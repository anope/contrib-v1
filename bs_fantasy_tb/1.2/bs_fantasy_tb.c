#include "module.h" 
#define AUTHOR "Roy"
#define VERSION "1.2"

/* ------------------------------------------------------------------------------------
 * Name		: bs_fantasy_tb
 * Author	: royteeuwen
 * Version	: 1.1
 * Date		: 16th Nov, 2006
 * ------------------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 1, or (at your option) any later version.
 * ------------------------------------------------------------------------------------
 * Tested	: Anope-1.7.17 on Unreal3.2.5
 * ------------------------------------------------------------------------------------
 * Description:
 * This is a botserv fantasy command that allows you to type 
 * !tb nick time(:in minutes) [reason:optional]
 * It will auto unban the person after the time, but you can also unban manually
 * ------------------------------------------------------------------------------------
 * Changes v1.2:
 * -There was a bug when the levels for getting voice would be changed so that there were more voice lvls
 * Changes v1.1:
 * - There was a bug where voices could use !tb on an unvoiced, and the nick would be kicked
 */


#define LANG_TB_SYNTAX "SYNTAX: tb nick time [reason]"

int handle_fantasy(int argc, char **argv);

int do_tban(User * u, Channel * c, char * nick, char * time);
void addBan(Channel * c, time_t timeout, char *banmask);
int delBan(int argc, char **argv);
int canBanUser(Channel * c, User * u, User * u2);

int AnopeInit(int argc, char **argv)
{
    EvtHook *hook;       
    int status;          

    hook = createEventHook(EVENT_BOT_FANTASY, handle_fantasy);
    status = moduleAddEventHook(hook);
    if (status != MOD_ERR_OK) {
        alog("Error binding to event EVENT_BOT_FANTASY [%d]", status);
        return MOD_STOP;
    }
	hook = createEventHook(EVENT_BOT_FANTASY_NO_ACCESS, handle_fantasy);
	status = moduleAddEventHook(hook);
    if (status != MOD_ERR_OK) {
        alog("Error binding to event EVENT_BOT_FANTASY [%d]", status);
        return MOD_STOP;
    }

    moduleAddAuthor(AUTHOR);       
    moduleAddVersion(VERSION);     

    alog("bs_fantas_tb: Loaded, usage: !tb nick time [reason]");
    return MOD_CONT;
}

/*************************************************************************/

void AnopeFini(void)
{
 alog("bs_fantasy_tb: unloaded");
}

/*************************************************************************/

int handle_fantasy(int argc, char **argv)
{
    User *u, *u2;
    ChannelInfo *ci;
    Channel *c;  

    char * nick = NULL;
    char * arg2 = NULL;
    char * reason = NULL;

    if (argc < 4) {
	      return MOD_CONT;
    }

    if (!(ci = cs_findchan(argv[2])))
		return MOD_CONT;
    if (!(c = findchan(ci->name)))
		return MOD_CONT;
    if (!(u = finduser(argv[1]))) 
		return MOD_CONT;
    if (!argv[3])
		return MOD_CONT;


    if (stricmp(argv[0], "tb") == 0) {
         if (argc == 4)  {
            nick = myStrGetToken(argv[3],' ',0);
		arg2 = myStrGetToken(argv[3],' ',1);
            reason = myStrGetTokenRemainder(argv[3],' ',2);

		if(!arg2)
			notice(whosends(ci), u->nick, LANG_TB_SYNTAX);
            else {
			u2 = finduser(nick);
    			int ulev = get_access(u, ci);
    			int u2lev = get_access(u2, ci);
			int level = ci->levels[CA_AUTOHALFOP];

			if (u2lev >= ulev) {
	 			notice_lang(s_ChanServ, u, PERMISSION_DENIED);
				if(arg2) free(arg2);
				if(nick) free(nick);
				if (reason) free(reason);
				return MOD_CONT;
			} else if (ulev < level) {
	 			notice_lang(s_ChanServ, u, PERMISSION_DENIED);
				if(arg2) free(arg2);
				if(nick) free(nick);
				if (reason) free(reason);
				return MOD_CONT;
    			} else {
				if (u2 && ci->c && is_on_chan(ci->c, u2)) { 					
                    		if (!reason && !is_protected(u2))
                        		bot_raw_kick(u, ci, nick, "Requested");
                    		else if (!is_protected(u2))
                        		bot_raw_kick(u, ci, nick, reason);
					do_tban(u, c,nick, arg2);					
				}
			}
     		}
          } else {
			notice(whosends(ci), u->nick, LANG_TB_SYNTAX);
			return MOD_CONT;
		}		
	}
	if (reason) free(reason);
    return MOD_CONT;
}
int do_tban(User * u, Channel * c, char * nick, char * time )
{
    char mask[BUFSIZE];	
    User *u2 = NULL;
    if (time && nick) {
		if (!(u2 = finduser(nick))) {
           		 notice_lang(s_ChanServ, u, NICK_X_NOT_IN_USE, nick);
	} else {
            if (canBanUser(c, u, u2)) {
                get_idealban(c->ci, u2, mask, sizeof(mask));
                addBan(c, dotime(time), mask);
            }
        }
    } 
    if (time)
        free(time);
    if (nick)
        free(nick);
    return MOD_CONT;
}

void addBan(Channel * c, time_t timeout, char *banmask)
{
    char *av[3];
    char *cb[2];
    cb[0] = c->name;
    cb[1] = banmask;

    av[0] = sstrdup("+b");
    av[1] = banmask;

    anope_cmd_mode(whosends(c->ci), c->name, "+b %s", av[1]);
    chan_set_modes(s_ChanServ, c, 2, av, 1);
    free(av[0]);
    moduleAddCallback("tban", time(NULL) + (timeout * 60), delBan, 2, cb);
}

int delBan(int argc, char **argv)
{
    char *av[3];
    Channel *c;

    av[0] = sstrdup("-b");
    av[1] = argv[1];

    if ((c = findchan(argv[0])) && c->ci) {
        anope_cmd_mode(whosends(c->ci), c->name, "-b %s", av[1]);
        chan_set_modes(s_ChanServ, c, 2, av, 1);
    }

    free(av[0]);

    return MOD_CONT;
}

int canBanUser(Channel * c, User * u, User * u2)
{
    ChannelInfo *ci;
    int ok = 0;

    if (!(ci = c->ci)) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, c->name);
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, c->name);
    } else if (!check_access(u, ci, CA_BAN)) {
        notice_lang(s_ChanServ, u, ACCESS_DENIED);
    } else if (ircd->except && is_excepted(ci, u2)) {
        notice_lang(s_ChanServ, u, CHAN_EXCEPTED, u2->nick, ci->name);
    } else if (ircd->protectedumode && is_protected(u2)) {
        notice_lang(s_ChanServ, u, PERMISSION_DENIED);
    } else {
        ok = 1;
    }

    return ok;
}

/* EOF */
