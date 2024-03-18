/***************************************************************************************/
/* Anope Module : new_regmsg.c : v1.x                                                  */
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
#define VERSION "1.0.1"

int cs_myreg(int ac, char **av);
int ns_myreg(int ac, char **av);
int do_reload(int argc, char **argv);
void my_load_config(void);

char *CSRegMsg = NULL;
char *NSRegMsg = NULL;

// Load the module
int AnopeInit(int argc, char **argv)
{
    EvtHook *hook1;
    EvtHook *hook2;
    EvtHook *hook3;

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);

    hook1 = createEventHook(EVENT_NICK_REGISTERED, ns_myreg);
    moduleAddEventHook(hook1);

    hook2 = createEventHook(EVENT_CHAN_REGISTERED, cs_myreg);
    moduleAddEventHook(hook2);
    
    hook3 = createEventHook(EVENT_RELOAD, do_reload);
    moduleAddEventHook(hook3);

    my_load_config();
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
    return MOD_CONT;
}

int cs_myreg(int ac, char **av)
{
  ChannelInfo *ci;

  ci = cs_findchan(av[0]);

  if (ci) {
      if (CSRegMsg) {
	 send_cmd(s_ChanServ, "NOTICE %s :%s", ci->founder->display, CSRegMsg);
      }
  }
  return MOD_CONT;
}

int ns_myreg(int ac, char **av)
{
  if (NSRegMsg) {
	send_cmd(s_NickServ, "NOTICE %s :%s", av[0], NSRegMsg);
  }
  return MOD_CONT;
}

void my_load_config(void)
{
    Directive confvalue[][1] = {
	{{"CSRegMsg", {{PARAM_STRING, PARAM_RELOAD, &CSRegMsg}}}},
	{{"NSRegMsg", {{PARAM_STRING, PARAM_RELOAD, &NSRegMsg}}}}
    };

  if (CSRegMsg)
	free(CSRegMsg);
  if (NSRegMsg)
	free(NSRegMsg);

  moduleGetConfigDirective(confvalue[0]);
  moduleGetConfigDirective(confvalue[1]);
}

/* EOF */
