/*----------------------------------------------------------------------------------------
 * Name:        cs_accessfounder
 * Author:      Jens 'DukePyrolator' Voss
 *              <DukePyrolator@FantasyIRC.net>
 * Version:     3.3
 *
 * ---------------------------------------------------------------------------------------
 * Supported IRCd:      Unreal3.2
 * Requires:            Anope1.7.15 and MySQL
 *----------------------------------------------------------------------------------------
 *
 * If you find bugs, please send me a Mail or contact me on irc.anope.org #anope
 * here you can find the latest version: http://dev.anope.de/cs_accessfounder.c
 *
 *---------------------------------------------------------------------------------------
 * Credits
 *  - thanks to all the people in #anope :-)
 *  - special thanks to Trystan who maintained my module when I was inactive 
 *
 *----------------------------------------------------------------------------------------
 * Changelog:
 *
 * v3.3    03/10/2006   - module should now work with any ircd that supports owner
 *                      - removed the function "is_identified" from the module because
 *                        its no longer declared as "static" in the core.
 *                      - removed the function "is_real_founder" from the module because
 *                        its no longer declared as "static" in the core.
 *                      - added a small workaround for an issue, where founder status is
 *                        not deleted on logout.
 * v3.2    09/17/2006   - dont give any modes if autoop is off
 * v3.1    09/16/2006   - first update since 2004 
 *                        works now with anope 1.7.15
 *
 *----------------------------------------------------------------------------------------*/


#define AUTHOR "Jens 'DukePyrolator' Voss <DukePyrolator@FantasyIRC.net>"
#define VERSION "3.3"

#include "module.h"
#include <version.h>

/* access level for auto +q */
#define CA_AUTOQ 9999

int do_on_join(int ac, char **av);
int do_on_identify(int ac, char **av);
int do_on_update(User *u);
int do_on_logout(int ac, char **av);
int is_real_founder(User * user, ChannelInfo * ci);
int do_set_founder(User * u);

int AnopeInit(int argc, char **argv) {
    Command *c = NULL;    
    EvtHook *hook;    
    int status = 0;  


    if (!ircd->owner) {
	alog("[cs_accessfounder] your ircd is not supported. sorry");
        return MOD_STOP;
    }
 
    hook = createEventHook(EVENT_JOIN_CHANNEL, do_on_join);
    status = moduleAddEventHook(hook);
    if (status != MOD_ERR_OK) {
        alog("[cs_accessfounder] unable to bind to EVENT_JOIN_CHANNEL error [%d]", status);
        return MOD_STOP;
    }

    hook = createEventHook(EVENT_NICK_IDENTIFY, do_on_identify);
    status = moduleAddEventHook(hook);
    if (status != MOD_ERR_OK) {
        alog("[cs_accessfounder] unable to bind to EVENT_NICK_IDENTIFY error [%d]", status);
        return MOD_STOP;
    }

    hook = createEventHook(EVENT_NICK_LOGOUT, do_on_logout);
    status = moduleAddEventHook(hook);
    if (status != MOD_ERR_OK) {
        alog("[cs_accessfounder] unable to bind to EVENT_NICK_LOGOUT error [%d]", status);
        return MOD_STOP;
    }


    c = createCommand("UPDATE", do_on_update, NULL, -1, -1, -1, -1, -1);
    moduleAddCommand(NICKSERV, c, MOD_HEAD);

    

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    moduleSetType(THIRD);

    return MOD_CONT;
}


int do_on_join(int ac, char **av) {
    User *u;
    Channel *c = NULL;

    struct u_chaninfolist *uc;

    if (!stricmp(av[0], EVENT_START)) {
	c = findchan(av[2]);
	if (!c || !c->ci) return MOD_CONT; /* chan not registered */
	if (!(u = finduser(av[1]))) return MOD_CONT;
        if (!nick_identified(u)) return MOD_CONT;

	if ((get_access_level(c->ci, u->na)>=CA_AUTOQ) && (!is_real_founder(u,c->ci))) {
    	    if (!is_identified(u, c->ci)) {
        	uc = scalloc(sizeof(*uc), 1);
        	uc->next = u->founder_chans;
        	if (u->founder_chans) u->founder_chans->prev = uc;
        	u->founder_chans = uc;
        	uc->chan = c->ci;
	    }
	}
	do_setmodes(u);
    }
    return MOD_CONT;
}



int do_on_identify(int ac, char **av) {
    User *u;

    u = finduser(av[0]);
    if (!nick_identified(u)) 
	return MOD_CONT; 

    if (NSModeOnID) {
	do_set_founder(u);
    }
    return MOD_CONT;
}

int do_on_update(User *u) {

    if (!nick_identified(u)) 
	return MOD_CONT; 
    if (NSModeOnID) {
	do_set_founder(u);
    }
    return MOD_CONT;
}

int do_on_logout(int ac, char **av) {
    User *u;
    struct u_chaninfolist *ci, *ci2;

    u = finduser(av[0]);
    /* deletes founder-status from all channels */
    ci = u->founder_chans;
    while (ci) {
        ci2 = ci->next;
        free(ci);
        ci = ci2;
    }
    u->founder_chans = NULL;  /* never point into unassigned memory */
    return MOD_CONT;
}


int do_set_founder(User * u) {
    struct u_chanlist *uc;
    struct u_chaninfolist *uci;
    Channel *c;

    
    /* Walk users current channels */
    for (uc = u->chans; uc; uc = uc->next) {
        if ((c = findchan(uc->chan->name)) && (c->ci)) {
	    if (should_mode_change(uc->status, CUS_OWNER) 
		&& ((get_access_level(c->ci,u->na)>=CA_AUTOQ) && (!is_real_founder(u,c->ci)))) {

	        if (!is_identified(u, c->ci)) {
        	    uci = scalloc(sizeof(*uci), 1);
        	    uci->next = u->founder_chans;
    	    	    if (u->founder_chans) u->founder_chans->prev = uci;
    	    	    u->founder_chans = uci;
		    uci->chan = c->ci;
    		}
	    } 
	}
    }
    return MOD_CONT;
}

