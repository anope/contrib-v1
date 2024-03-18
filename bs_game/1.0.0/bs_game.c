#include "module.h"

#define AUTHOR "mooncup"
#define VERSION "Fantasy game command v1.0"

int handle_fantasy(int argc, char **argv);


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
    moduleSetType(THIRD); 

    alog("Fantasy: Game module loaded");
    return MOD_CONT;
}

void AnopeFini(void)
{
 alog("Fantasy: Game module unloaded");
}

/*************************************************************************/

int handle_fantasy(int argc, char **argv)
{
    User *u;
    ChannelInfo *ci;

    if (argc < 3) {
      return MOD_CONT;
    }

    u = finduser(argv[1]);
    ci = cs_findchan(argv[2]);

    if (!u || !ci) {
       return MOD_CONT;
    }

    if (!stricmp("game", argv[0])) {
		anope_cmd_notice(ci->bi->nick, ci->name, "%s JUST MADE YOU LOSE THE GAME BITCHES", argv[3]);
    }
    

    return MOD_CONT;
}