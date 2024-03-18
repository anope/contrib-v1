/*
 * spamfilter.c written for use on chatter.lu network
 * Author retains all original rights to work.
 * You may modify/redistribute this code under
 * the GPL agreement provided you maintain Author's credit.
 * This code comes from ipadd_module.
 * I thank his author David Chafin for this little module which
 * enables me to create my first module inspired by David Chafin.
 *
 */

/* make sure you edit the following lines */

#define PATH_TO_SPAMFILTER_CONF "/home/your_irc_path/unreal/spamfilter.conf"
#define PATH_TO_UNREAL_START_STOP "/home/your_irc_path/unreal/./unreal"

/***** DO NOT EDIT BELOW THIS LINE *****/

#include "module.h"
#include <stdio.h>

#define AUTHOR "irc0p"
#define VERSION "v1.1"

int spamfilter(int argc, char **argv)
{
	User *u;
	ChannelInfo *ci;
	FILE * pFile;

	if (argc && stricmp("spamfilter", argv[0]))
		return MOD_CONT;

	if (argc != 4)
	{
		notice(whosends(ci), u->nick, "Syntax is !spamfilter _regex_");
		return MOD_CONT;
	}

	if (!(u = finduser(argv[1])))
		return MOD_CONT;

	if (!is_services_admin(u))
	{
		notice(whosends(ci), u->nick, "Permission denied, only for IRCops.");
		return MOD_STOP;
	}

	if (!(ci = cs_findchan(argv[2])))
	{
		notice(whosends(ci), u->nick, "You must supply an #channel.");
		return MOD_STOP;
	}

	pFile = fopen (PATH_TO_SPAMFILTER_CONF,"a");

	if ( pFile != NULL )
	{
		fprintf(pFile, "#Entry added by %s\n", u->nick);
		fprintf(pFile, "spamfilter {\n");
		fprintf(pFile, "regex \"%s\";\n", argv[3]);
		fprintf(pFile, "target { private; channel; };\n");
		fprintf(pFile, "action block;\n");
		fprintf(pFile, "reason \"No spam on our network!\";\n");
		fprintf(pFile, "};\n");
		fprintf(pFile, "\n");
		fclose (pFile);

		notice(whosends(ci), ci->name, "!spamfilter used by %s: adding the following URL: %s."
				" Server will be rehashed automatically!", u->nick, argv[3]);

		system(PATH_TO_UNREAL_START_STOP" rehash");

		return MOD_CONT;
	}
	else
	{
		notice(whosends(ci), u->nick, "Could not open spamfilter.conf! Aborting.");
		alog("Unable to open spamfilter.conf");
		return MOD_STOP;
	}
	/* All Done */
	return MOD_CONT;
}


int AnopeInit(int argc, char **argv)
{
	EvtHook *hook;
	int status;

	hook = createEventHook(EVENT_BOT_FANTASY, spamfilter);
	if (moduleAddEventHook(hook) != MOD_ERR_OK)
	{
		alog("[\002spamfilter\002] Error binding to event EVENT_BOT_FANTASY [%d]", status);
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_BOT_FANTASY_NO_ACCESS, spamfilter);
	if (moduleAddEventHook(hook) != MOD_ERR_OK)
	{
		alog("[\002spamfilter\002] Error binding to event EVENT_BOT_FANTASY [%d]", status);
		return MOD_STOP;
	}

	moduleAddAuthor(AUTHOR);       /* Add the author information  */
	moduleAddVersion(VERSION);     /* Add the version information */
	moduleSetType(THIRD);          /* Flag as Third party module  */

	alog("Spamfilter successfully loaded.");
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("Spamfilter successfully unloaded.");
}


