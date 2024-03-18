/***************************************************************************************/
/* Anope Module : cs_setmodesunreg.c : v1.x                                            */
/* Scott Seufert                                                                       */
/* katsklaw@ircmojo.org                                                                */
/*                                                                                     */
/* This program is free software; you can redistribute it and/or modify it under the   */
/* terms of the GNU General Public License as published by the Free Software           */
/* Foundation; either version 1, or (at your option) any later version.                */
/*                                                                                     */
/*  This program is distributed in the hope that it will be useful, but WITHOUT ANY    */
/*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A    */
/*  PARTICULAR PURPOSE.  See the GNU General Public License for more details.          */
/*                                                                                     */
/***************************************************************************************/

#include "module.h"

#define AUTHOR "katsklaw"
#define VERSION "1.0.0"

// Functions
int do_mysets(int ac, char **av);
int do_unmysets(int ac, char **av);
int do_mydrop(int ac, char **av);
int do_reload(int argc, char **argv);
void my_load_config(void);

char *CSModesUnreg = NULL;
char *CSModesReg = NULL;

// Load the module
int AnopeInit(int argc, char **argv)
{
    EvtHook *hook1;
    EvtHook *hook2;
    EvtHook *hook3;
    EvtHook *hook4;
    EvtHook *hook5;
    EvtHook *hook6;
    EvtHook *hook7;

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

    hook7 = createEventHook(EVENT_RELOAD, do_reload);
    moduleAddEventHook(hook7);

    my_load_config();
    alog("Init CSModesUnreg=%s CSModesReg=%s", CSModesUnreg, CSModesReg);
    return MOD_CONT;
}

// Unload the module
void AnopeFini(void)
{
	// Nothing to do here.
}

int do_reload(int argc, char **argv)
{
  my_load_config();
  alog("Reload CSModesUnreg=%s CSModesReg=%s", CSModesUnreg, CSModesReg);
  return MOD_CONT;
}

int do_mysets(int ac, char **av)
{
  ChannelInfo *ci;

  ci = cs_findchan(av[2]);

  if ((!ci) && (CSModesUnreg)) {
    if (stricmp(av[0], EVENT_STOP) == 0) {
      send_cmd(ServerName, "MODE %s %s", av[2], CSModesUnreg);
    }
  }
  return MOD_CONT;
}

int do_unmysets(int ac, char **av)
{
  if (CSModesReg) {
    send_cmd(ServerName, "MODE %s %s", av[0], CSModesReg);
  }
  return MOD_CONT;
}

int do_mydrop(int ac, char **av)
{
  if (CSModesUnreg) {
    send_cmd(ServerName, "MODE %s %s", av[0], CSModesUnreg);
  }
  return MOD_CONT;
}

void my_load_config(void)
{
  Directive confvalue[][1] = {
    {{"CSModesUnreg", {{PARAM_STRING, PARAM_RELOAD, &CSModesUnreg}}}},
    {{"CSModesReg", {{PARAM_STRING, PARAM_RELOAD, &CSModesReg }}}},
  };

  if (CSModesUnreg)
    free(CSModesUnreg);
  if (CSModesReg)
    free(CSModesReg);

  moduleGetConfigDirective(confvalue[0]);
  moduleGetConfigDirective(confvalue[1]);
}

/* EOF */
