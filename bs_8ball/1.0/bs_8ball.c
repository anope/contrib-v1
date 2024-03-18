/***************************************************************************************/
/* Anope Module: bs_8ball                                                              */
/* Author: Perihelion (amanda@anope.org)                                               */
/*                                                                                     */
/* For help with this module, please contact the module author. This is a 3rd party    */
/* module and Anope staff will not help with problems you may encounter.               */
/*                                                                                     */
/* This module has no configuration directives. Simply type !8ball <question here>     */
/* (replace ! with your fantasy character if you specified a different one in          */
/* your services.conf)                                                                 */
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

/* Include Anope's module header */
#include "module.h"
/* define the module author and version */
#define AUTHOR "Perihelion"
#define VERSION "Fantasy 8ball v 1.0"

int handle_fantasy(int argc, char **argv);

int AnopeInit(int argc, char **argv)
{
	EvtHook *hook;       /* Event Hook Variable */
	int status;          /* Status for when we add the event */

	/* Add an event handler for EVENT_BOT_FANTASY this event is
	sent when anyone with FANTASY access triggers a ! command */
	hook = createEventHook(EVENT_BOT_FANTASY, handle_fantasy);
	/* Add the event and store the status */
	status = moduleAddEventHook(hook);
	/* Check status, if it is not MOD_ERR_OK return an error message */
	if (status != MOD_ERR_OK) 
	{
        /* Error Message */
        alog("Error binding to event EVENT_BOT_FANTASY [%d]", status);
        /* Stop now as there was an error */
        return MOD_STOP;
	}

	/* Add an event handler for EVENT_BOT_FANTASY this event is
	sent when anyone WITHOUT FANTASY access triggers a ! command */
	hook = createEventHook(EVENT_BOT_FANTASY_NO_ACCESS, handle_fantasy);
	/* Add the event and store the status */
	status = moduleAddEventHook(hook);
	/* Check status, if it is not MOD_ERR_OK then throw an error message */
	if (status != MOD_ERR_OK) 
	{
        /* Error Message */
        alog("Error binding to event EVENT_BOT_FANTASY [%d]", status);
        /* Stop now as there was an error */
        return MOD_STOP;
	}

	/* If all has gone good so far, it is time to add our information
	to the module struct */
	moduleAddAuthor(AUTHOR);       /* Add the author information  */
	moduleAddVersion(VERSION);     /* Add the version information */
	moduleSetType(THIRD);          /* Flag as Third party module  */

	/* A message that says that we loaded successfully */
	alog("Shenanigans: Oh lawd iz dat sum 8ball modload?");
	/* Return that everything is okay */
	return MOD_CONT;
}

/*************************************************************************/

/* Called on unload 
   - if you have allocated any global variables, here is a good place to
     free them.
*/
void AnopeFini(void)
{
 /* A message about unload */
 alog("Shenanigans: Fantasy 8ball unloaded! :(");
}

/*************************************************************************/

int handle_fantasy(int argc, char **argv)
{
	User *u;
	ChannelInfo *ci;

	/* Check that we got at least 3 parameters */
	if (argc < 3)
	{
	/* return now */
	return MOD_CONT;
	}

	/* Get the user struct */
	u = finduser(argv[1]);
	/* Get the channel info struct */
	ci = cs_findchan(argv[2]);

	/* Make sure they aren't null */
	if (!u || !ci)
	{
	return MOD_CONT;
	}

	/* Set up our random responses! */
	int i = 0;
	int randMax = 16;

	/* And now the randomness */
	char *ballResponses[17] = 
	{
		"", /* response 0, will never be picked */
		"Ask again later",
		"Better not tell you now",
		"Concentrate and ask again",
		"Don't count on it",
		"It is certain",
		"Most likely",
		"My reply is no",
		"My sources say no",
		"No",
		"Outlook good",
		"Outlook not so good",
		"Reply hazy, try again",
		"Signs point to yes",
		"Yes",
		"Yes, definately",
		"You may rely on it"
	};

	/* Did we get 8ball after the fantasy char? */
	if(stricmp("8ball",argv[0])==0) 
	{
		/* Nothing came after !8ball, so no question was asked! */
		if(argc < 4)
		{
		anope_cmd_privmsg(ci->bi->nick, ci->name, "You didn't ask a question!");
		}
		/* We got a question...let's answer! */
		else
		{
        	i = 1 + (int) (randMax * (rand() / (RAND_MAX + 1.0)));
        	anope_cmd_privmsg(ci->bi->nick, ci->name,ballResponses[i]);
		}
	
	}	
	/* When all done return MOD_CONT */
	return MOD_CONT;
}

/* EOF */