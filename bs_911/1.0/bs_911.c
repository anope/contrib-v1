#include "module.h"

/* 
* User Config 
* Change this BEFORE you compile!
* set operchan to the channel you would like the help notice to appear in.
* 
* BEGIN USER CONFIG
*/

#define operchan "#opers"

/*
* END USER CONFIG
*/

#define AUTHOR "Andrew"
#define VERSION "1.0"

int do_calloper(int argc, char **argv);


int AnopeInit(int argc, char **argv)
{
    EvtMessage *msg = NULL;
    EvtHook    *hook = NULL;
    int status;
    hook = createEventHook(EVENT_BOT_FANTASY, do_calloper);
    status = moduleAddEventHook(hook);
    alog("bs_911: Module loaded.");
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    return MOD_CONT;
}

void AnopeFini(void)
{
	alog("bs_911: Module unloaded.");
}



int do_calloper(int argc, char **argv) {
    ChannelInfo *ci;
    if(argc>=3) { 
		if(stricmp(argv[0],"911")==0) { 
            if((ci = cs_findchan(argv[2]))) {
				alog("!911 activated and opers notified.");
                anope_cmd_privmsg(ci->bi->nick, ci->name, "%s, all online opers have been alerted. Help should arrive soon.",argv[1]);
				notice(s_HelpServ, operchan, "ATTENTION! Help is needed in %s! User: %s", ci->name, argv[1]);
				return MOD_STOP; 
            }
		} 
    }
    return MOD_CONT; 
}

