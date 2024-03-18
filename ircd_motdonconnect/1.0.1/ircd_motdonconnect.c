/***************************************************************************************/
/* Anope Module : ircd_motdonconnect.c : v1.x                                          */
/* Scott Seufert - katsklaw@ircmojo.net                                                */
/*                                                                                     */
/* Anope (c) 2000-2002 Anope.org                                                       */
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
#include <module.h>

#define AUTHOR "katsklaw"
#define VERSION "1.0.1"

int do_sendmotd(int ac, char **av);
int m_mymotd(char *source);

int AnopeInit(int argc, char **argv)
{
	EvtHook *hook;
    
	hook = createEventHook(EVENT_NEWNICK, do_sendmotd);
    	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
        alog("Can't hook to EVENT_NEWNICK event");
        return MOD_STOP;
     }

   	alog("Loading ircd_motdonconnect");

	if (findModule("os_raw")) {
		alog("I found os_raw loaded!, Sorry it didn't work out.");
		return MOD_STOP;
	}

	moduleAddAuthor(AUTHOR);
   	moduleAddVersion(VERSION);

    return MOD_CONT;

}

void AnopeFini(void) {
	/* Windows likes to complain sometimes */
  alog("Unloading ircd_motdonconnect");
}

int do_sendmotd(int ac, char **av)
{
	if (ac < 1)
	{
		return MOD_CONT;
	} else {
		m_mymotd(av[0]);
	}
	return MOD_CONT;
}

int m_mymotd(char *source)
{
    FILE *f;
    char buf[BUFSIZE];

    if (!source) {
        return MOD_CONT;
    }

    f = fopen(MOTDFilename, "r");
    if (f) {
        anope_cmd_375(source);
        while (fgets(buf, sizeof(buf), f)) {
            buf[strlen(buf) - 1] = 0;
            anope_cmd_372(source, buf);
        }
        fclose(f);
        anope_cmd_376(source);
    } else {
        anope_cmd_372_error(source);
    }
    return MOD_CONT;
}


