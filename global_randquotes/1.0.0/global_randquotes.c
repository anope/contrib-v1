#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: global_randquotes.c v1.0.1 27-08-2007 n00bie $"

/*
************************************************************************************************
** Module	: global_randquotes.c
** Version	: 1.0.0
** Author	: n00bie (n00bie@rediffmail.com)
** Release	: 27th August, 2007
************************************************************************************************
** Description:
**
** Display a random quotes when users connect. The quotes will be displayed by 'Global'
** using notices. Random quotes are read from the file 'quotes.db'. New quotes can be
** added or remove on-the-fly from the quotes database.
************************************************************************************************
** Tested: 1.7.18, 1.7.19
**
** Usage:
** Put the file 'quotes.db' inside services folder (nix) or inside the 'data' folder(windows). 
** I've included 325 lines of quotes inside the database, feel free to add more or remove them ^^
**
** Note: Every IRCd(?) have a message limit per line, so remember to keep your quotes line by 
** line and try to keep them per the limit. For more info please consult your IRCd documentation.
************************************************************************************************
** This program is free software; you can redistribute it and/or modify it under the
** terms of the GNU General Public License as published by the Free Software
** Foundation; either version 1, or (at your option) any later version. 
**
** This program is distributed in the hope that it will be useful, but WITHOUT ANY
** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS for
** a PARTICULAR PURPOSE. THIS MODULE IS DISTRIBUTED 'AS IS'. NO WARRANTY OF ANY
** KIND IS EXPRESSED OR IMPLIED. YOU USE AT YOUR OWN RISK. THE MODULE AUTHOR WILL 
** NOT BE RESPONSIBLE FOR DATA LOSS, DAMAGES, LOSS OF PROFITS OR ANY OTHER KIND OF 
** LOSS WHILE USING OR MISUSING THIS MODULE. 
**
** See the GNU General Public License for more details.
************************************************************************************************
** This module have no configurable option.
*/

#define QuotesDB "quotes.db"

int doGlobal(int argc, char **argv);
int AnopeInit(int argc, char **argv)
{
	EvtHook *hook;
	int status = 0;
	hook = createEventHook(EVENT_NEWNICK, doGlobal);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("global_randquotes: Something isn't init right.");
		return MOD_STOP;
	} else {
		alog("global_randquotes: Successfully loaded module.");
	}
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("global_randquotes: Module unloaded.");
}

int doGlobal(int argc, char **argv)
{
	User *u = finduser(argv[0]);
	int flines;
	int rline;
	int i;
	char buf[2000];
	FILE *fp = fopen(QuotesDB, "r");
	srand(time(NULL));
	if (!(fp)) {
		alog("global_randquotes: error opening quotes database file %s. It might not exists!", QuotesDB);
		return MOD_CONT;
	}
	for (flines = 0; fgets(buf, sizeof(buf), fp) ; ++flines);
	if (flines == 0) {
		alog("global_randquotes: the quotes database file %s is empty!", QuotesDB);
	} else {
		rline = rand() % (flines + 1);
		rewind(fp);
		for (i = 0; i < rline; ++i)
			fgets(buf, sizeof(buf), fp);
		notice_user(s_GlobalNoticer, u, "\2Random Quote:\2 %s", buf);
	}
	fclose(fp);
	return MOD_CONT;
}
/* EOF */
