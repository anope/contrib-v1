/***************************************************************************************/
/* Anope Module : ircd_chanurl.c : v1.0                                                */
/* Scott Seufert                                                                       */
/* katsklaw@ircmojo.org                                                                */
/*                                                                                     */
/* Anope (c) 2000-2010 Anope.org                                                       */
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
#define VERSION "1.0.3"

int do_chanurl(int ac, char **av);

int AnopeInit(int argc, char **argv)
{
        EvtHook *hook;

        hook = createEventHook(EVENT_JOIN_CHANNEL, do_chanurl);
        if (moduleAddEventHook(hook) != MOD_ERR_OK) {
        alog("Can't hook to EVENT_JOIN_CHANNEL event");
        return MOD_STOP;
     }

        alog("Loading ircd_chanurl");

        if (findModule("os_raw")) {
                alog("I found os_raw loaded!, Sorry it didn't work out.");
                return MOD_STOP;
        }

        moduleAddAuthor(AUTHOR);
        moduleAddVersion(VERSION);

    return MOD_CONT;

}

void AnopeFini(void)
{
  alog("Unloading");
}

int do_chanurl(int ac, char **av)
{
  ChannelInfo *ci;
  char *nick = NULL;

  nick = av[1];
  ci = cs_findchan(av[2]);

  if (ci) {
  	if (stricmp(av[0], EVENT_STOP) == 0) {
  		if (ci->url) {
  			send_cmd(ServerName, "328 %s %s :%s", nick, ci->name, ci->url);
		}
	}
  }
  return MOD_CONT;
}



/* EOF */
