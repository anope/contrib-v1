// Mode made by request: http://forum.anope.org/index.php?topic=3429.0
//
// katsklaw@msn.com
//
//

#include "module.h"
#define AUTHOR "katsklaw"
#define VERSION "1.0.1"

// Functions
int do_mysets(int ac, char **av);
int do_unmysets(int ac, char **av);
int do_mydrop(int ac, char **av);

// Load the module
int AnopeInit(int argc, char **argv)
{
    EvtHook *hook1;
    EvtHook *hook2;
    EvtHook *hook3;
    EvtHook *hook4;
    EvtHook *hook5;
    EvtHook *hook6;

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);

    hook1 = createEventHook(EVENT_JOIN_CHANNEL, do_mysets);
    moduleAddEventHook(hook1);

    hook2 = createEventHook(EVENT_CHAN_REGISTERED, do_unmysets);
    moduleAddEventHook(hook2);

    hook3 = createEventHook(EVENT_CHAN_SUSPENDED, do_mydrop);
    moduleAddEventHook(hook3);

    hook4 = createEventHook(EVENT_CHAN_UNSUSPEND, do_unmysets);
    moduleAddEventHook(hook4);

    hook5 = createEventHook(EVENT_CHAN_EXPIRE, do_mydrop);
    moduleAddEventHook(hook5);

    hook6 = createEventHook(EVENT_CHAN_DROP, do_mydrop);
    moduleAddEventHook(hook6);

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

  ci = cs_findchan(av[2]);
	// Channel not registered so we now set +s -katsklaw

  if (!ci) {
  	if (stricmp(av[0], EVENT_STOP) == 0) {
		send_cmd(ServerName, "MODE %s +s", av[2]);
	}
  }
  return MOD_CONT;
}

int do_unmysets(int ac, char **av)
{
	// No need to check if the channel is in the db,
	// this function isn't triggered unless it's being
	// unsuspended or registered. -katsklaw
  send_cmd(ServerName, "MODE %s -s", av[0]);
  return MOD_CONT;
}

int do_mydrop(int ac, char **av)
{
  send_cmd(ServerName, "MODE %s +s", av[0]);
  return MOD_CONT;
}

/* EOF */

