#include "module.h"

/* 
* User Config 
* Change this BEFORE you compile!
* set operchan to the channel you would like the help notice to appear in.
* 
* You can also choose to use a privmsg in place of a channel notice by uncommenting use_privmsg.
*
* BEGIN USER CONFIG
*/

#define operchan "#opers"
//#define use_privmsg

/*
* END USER CONFIG
*/

#define AUTHOR "Jackster35"
#define VERSION "0.6"

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
		if(stricmp(argv[0],"999")==0) { 
            		if((ci = cs_findchan(argv[2]))) {

				alog("!999 activated and opers notified.");
                		anope_cmd_privmsg(ci->bi->nick, ci->name, "%s, all online opers have been alerted. Help should arrive soon.",argv[1]);
				anope_cmd_privmsg(ci->bi->nick, ci->name, "Please do not use !999 again!",argv[1]);
				#ifndef use_privmsg
				anope_cmd_privmsg(s_OperServ, operchan, "ATTENTION! Help is needed in %s! User: %s", ci->name, argv[1]);
				#endif
				#ifdef use_privmsg
				anope_cmd_privmsg(s_OperServ, operchan, "ATTENTION! Help is needed in %s! User: %s", ci->name, argv[1]);
				#endif
				return MOD_STOP; 

            		}
		} 
    }
    return MOD_CONT; 
}

