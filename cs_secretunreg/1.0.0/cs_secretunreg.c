// Mode made by request: http://forum.anope.org/index.php?topic=3429.0
//
// katsklaw@msn.com
//
//

#include "module.h"
#define AUTHOR "katsklaw"
#define VERSION "1.0.0"

// Functions
int do_mysets(int ac, char **av);

// Load the module
int AnopeInit(int argc, char **argv)
{
    EvtHook *hook;

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);

    hook = createEventHook(EVENT_JOIN_CHANNEL, do_mysets);
    moduleAddEventHook(hook);

    return MOD_CONT;
}

// Unload the module
void AnopeFini(void)
{
	// Nothing to do here.
}

int do_mysets(int ac, char **av)
{
  ChannelInfo *ci;
  char *nick = NULL;

  ci = cs_findchan(av[2]);
	// Channel not registered so we now set +s -katsklaw
  if (!ci) {
  	if (stricmp(av[0], EVENT_STOP) == 0) {
		send_cmd(ServerName, "MODE %s +s", av[2]);
	}
  }
  return MOD_CONT;
}

/* EOF */

