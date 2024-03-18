/* ----------------------------------------------------------------------------
 * Name    : bs_perchannelfantasy.c
 * Author  : Naram Qashat (CyberBotX)
 * Version : 1.2
 * Date    : (Created) August 7th, 2010 (Last modified) December 30th, 2010
 * ----------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 1, or (at your option) any later
 * version.
 * ----------------------------------------------------------------------------
 * Requires: Anope-1.8.x (May work with later versions of 1.7.x, untested)
 * ----------------------------------------------------------------------------
 * Description:
 *
 * Allows channel founder (and Services Administrators) to set custom fantasy
 * triggers on a per-channel basis.
 * ----------------------------------------------------------------------------
 * Usage:
 *
 * This module has 1 configurable option.
 *
 * ----- Put these lines in your services.conf --------------------------------

# BSPerChannelFantasyDBName [OPTIONAL]
# Specifies the filename of bs_perchannelfantasy's database.  If not given,
#   defaults to "bs_perchannelfantasy.db".

#BSPerChannelFantasyDBName "bs_perchannelfantasy.db"

 * ----- End of Configuration -------------------------------------------------
 * Changelog:
 *
 *   1.2 - Fix segfault if ChanServ's SET is called without a parameter after
 *         the channel name.
 *   1.1 - Fix potential bug caused by my custom PRIVMSG handler changing the
 *           value of the current module name.
 *   1.0 - Initial version.
 *
 * ----------------------------------------------------------------------------
 */

#include "module.h"

static const char *const AUTHOR = "Naram Qashat (CyberBotX)";
static const char *const VERSION = "v1.1";

enum LNG /* Language strings */
{
	CHAN_SET_FANTASY_SYNTAX,
	CHAN_SET_FANTASY_RESET,
	CHAN_SET_FANTASY_TRIGGER,
	CHAN_INFO_FANTASY,
	MY_CHAN_HELP_SET,
	CHAN_HELP_SET_FANTASY,
	MY_NUM_STRINGS
};

static char *BSPerChannelFantasyDBName = NULL; /* Database file name */
static char *module_name = NULL; /* The module's name, needed in our custom PRIVMSG message */

int (*EventPrivmsg)(char *, int, char **);

/** Custom PRIVMSG message.
 * @param source The source of the message (We don't really care in this case, as we pass it off to the protocol module anyways)
 * @param ac Number of parameters (We only care that we actually got some parameters)
 * @param av The parameters (In this case, we only care about the 1st one which might contain a channel name)
 * @return MOD_CONT is the only thing returned to allow things to work as expected.
 *
 * This custom PRIVMSG message is designed to change the fantasy trigger for that channel if there is a custom one.
 */
int MyPrivmsg(char *source, int ac, char **av)
{
	char oldTrigger = *BSFantasyCharacter, *old_mod_current_module_name = mod_current_module_name;
	Module *old_mod_current_module = mod_current_module;

	/* This is to set the current module as the module name is the previous module when this is called, it will be set back to it's original value later */
	mod_current_module_name = module_name;
	if (mod_current_module_name)
		mod_current_module = findModule(module_name);

	/* We only check for a custom fantasy trigger if the recipient is a channel and we have BotServ enabled */
	if (ac > 0 && *av[0] == '#' && s_BotServ)
	{
		/* Find the chanenel */
		ChannelInfo *ci = cs_findchan(av[0]);
		if (ci)
		{
			/* Find if a custom trigger exists and replace the fantasy character with that trigger */
			char *trigger = moduleGetData(&ci->moduleData, "bs_perchannelfantasy");
			if (trigger)
			{
				*BSFantasyCharacter = *trigger;
				free(trigger);
			}
		}
	}

	mod_current_module_name = old_mod_current_module_name;
	mod_current_module = old_mod_current_module;

	/* Pass the message off to the protocol module and then reset the fantasy character back to it's original value */
	EventPrivmsg(source, ac, av);
	*BSFantasyCharacter = oldTrigger;

	return MOD_CONT;
}

/** SET command
 * @param u User who requested the command
 *
 * Adds a FANTASY subcommand to ChanServ's SET command.
 */
static int do_set(User *u)
{
	char *text = moduleGetLastBuffer();

	if (text)
	{
		char *chan = myStrGetToken(text, ' ', 0);

		if (chan)
		{
			ChannelInfo *ci = cs_findchan(chan);

			if (ci && !(ci->flags & CI_VERBOTEN))
			{
				char *cmd = myStrGetToken(text, ' ', 1);

				if (cmd && !stricmp(cmd, "FANTASY"))
				{
					/* We only allow Services Administrators or channel founders to change a channel's custom fantasy trigger */
					if (is_services_admin(u) || (ci->flags & CI_SECUREFOUNDER ? is_real_founder(u, ci) : is_founder(u, ci)))
					{
						char *trigger = myStrGetToken(text, ' ', 2);

						if (!trigger)
						{
							moduleNoticeLang(s_ChanServ, u, CHAN_SET_FANTASY_SYNTAX);

							free(cmd);
							free(chan);

							return MOD_STOP;
						}

						/* RESET is a special "trigger" to remove the custom fantasy trigger */
						if (!stricmp(trigger, "RESET"))
						{
							moduleDelData(&ci->moduleData, "bs_perchannelfantasy");
							moduleNoticeLang(s_ChanServ, u, CHAN_SET_FANTASY_RESET, ci->name);
						}
						/* We only accept 1-character triggers as that is all the core accepts anyways */
						else if (strlen(trigger) == 1)
						{
							moduleAddData(&ci->moduleData, "bs_perchannelfantasy", trigger);
							moduleNoticeLang(s_ChanServ, u, CHAN_SET_FANTASY_TRIGGER, ci->name, *trigger);
						}
						else
							moduleNoticeLang(s_ChanServ, u, CHAN_SET_FANTASY_SYNTAX);

						free(trigger);
					}
					else
						notice_lang(s_ChanServ, u, ACCESS_DENIED);

					free(cmd);
					free(chan);

					return MOD_STOP;
				}

				if (cmd)
					free(cmd);
			}
		}

		free(chan);
	}

	return MOD_CONT;
}

/** INFO command
 * @param u User who requested the command.
 *
 * Adds a line at the end of CS INFO to show if there is a custom fantasy trigger.
 */
static int do_info(User *u)
{
	char *text = moduleGetLastBuffer(), *chan = myStrGetToken(text, ' ', 0);
	if (chan)
	{
		ChannelInfo *ci = cs_findchan(chan);
		if (ci && !(ci->flags & CI_VERBOTEN) && ((ci->flags & CI_SECUREFOUNDER ? is_real_founder(u, ci) : is_founder(u, ci)) || is_services_oper(u)))
		{
			char *trigger = moduleGetData(&ci->moduleData, "bs_perchannelfantasy");
			if (trigger)
			{
				moduleNoticeLang(s_ChanServ, u, CHAN_INFO_FANTASY, *trigger);
				free(trigger);
			}
		}
		free(chan);
	}
	return MOD_CONT;
}

/* This is only here to get around a bug in Anope 1.8.4 and below. */
static int do_nothing(User *u)
{
	return MOD_CONT;
}

/** HELP SET command
 * @param u User who requested the help
 */
static int do_help_set(User *u)
{
	moduleNoticeLang(s_ChanServ, u, MY_CHAN_HELP_SET);
	return MOD_CONT;
}

/** HELP SET FANTASY command
 * @param u User who requested the help
 */
static int do_help_set_fantasy(User *u)
{
	moduleNoticeLang(s_ChanServ, u, CHAN_HELP_SET_FANTASY);
	return MOD_CONT;
}

/** Load bs_perchannelfantasy's configuration.
 *
 * Only directive is BSPerChannelFantasyDBName, defaults to bs_perchannelfantasy.db if not given.
 */
void do_load_config(void)
{
	int i;
	char *tmpDBName = NULL;
	Directive confvalues[][1] = {
		{{"BSPerChannelFantasyDBName", {{PARAM_STRING, PARAM_RELOAD, &tmpDBName}}}},
	};

	for (i = 0; i < 1; ++i)
		moduleGetConfigDirective(confvalues[i]);

	if (BSPerChannelFantasyDBName)
		free(BSPerChannelFantasyDBName);

	BSPerChannelFantasyDBName = sstrdup(tmpDBName ? tmpDBName : "bs_perchannelfantasy.db");
}

/** Load bs_prechannelfantasy's database.
 *
 * bs_perchannelfantasy's database consists of all the custom fantasy triggers for a per-channel change.
 */
void do_load_db(void)
{
	char readbuf[1024];
	FILE *in = fopen(BSPerChannelFantasyDBName, "rb");

	/* Fail if we were unable to open the database */
	if (!in)
	{
		alog("[bs_perchannelfantasy] Unable to open database ('%s') for reading", BSPerChannelFantasyDBName);
		return;
	}

	/* Read in the database line by line, format is: {channel} {fantasytrigger} */
	while (fgets(readbuf, 1024, in))
	{
		char *name = myStrGetToken(readbuf, ' ', 0), *trigger = myStrGetToken(readbuf, ' ', 1);
		if (name)
		{
			if (trigger)
			{
				unsigned len = strlen(trigger);
				/* Make sure to get rid of the trailing \n */
				trigger[len - 1] = 0;

				ChannelInfo *ci = cs_findchan(name);
				if (ci)
					moduleAddData(&ci->moduleData, "bs_perchannelfantasy", trigger);

				free(trigger);
			}
			free(name);
		}
	}

	fclose(in);

	if (debug)
		alog("[bs_perchannelfantasy] Successfully loaded database");
}

/** Save bs_prechannelfantasy's database.
 */
void do_save_db(void)
{
	FILE *out = fopen(BSPerChannelFantasyDBName, "wb");
	int i;

	/* Fail if we were unable to open the database */
	if (!out)
	{
		alog("[bs_perchannelfantasy] Unable to open database ('%s') for writing", BSPerChannelFantasyDBName);
		return;
	}

	/* Go through all the registered channels and add them to the database file if they have a custom fantasy trigger */
	for (i = 0; i < 256; ++i)
	{
		ChannelInfo *ci = chanlists[i];
		for (; ci; ci = ci->next)
		{
			char *info = moduleGetData(&ci->moduleData, "bs_perchannelfantasy");
			if (info)
			{
				fprintf(out, "%s %s\n", ci->name, info);
				free(info);
			}
		}
	}

	fclose(out);

	if (debug)
		alog("[bs_perchannelfantasy] Successfully saved database");
}

/** Save the database when Anope saves it's own databases.
 * @param argc The number of arguments (should be 1, technically)
 * @param argv The arguments, should be either EVENT_START or EVENT_STOP
 *
 * We only care if the argument is EVENT_START as it doesn't matter either way.
 */
int do_save_db_evt(int argc, char **argv)
{
	if (!stricmp(argv[0], EVENT_START))
		do_save_db();

	return MOD_CONT;
}

/** Backup the database when Anope backs up it's own databases.
 * @param argc The number of arguments (should be 1, technically)
 * @param argv The arguments, should be either EVENT_START or EVENT_STOP
 *
 * We only care if the argument is EVENT_START as it doesn't matter either way.
 */
int do_backup_db_evt(int argc, char **argv)
{
	if (!stricmp(argv[0], EVENT_START))
		ModuleDatabaseBackup(BSPerChannelFantasyDBName);

	return MOD_CONT;
}

/** Add the languages for the module.
 */
void do_add_languages(void)
{
	char *langtable_en_us[] = {
		/* CHAN_SET_FANTASY_SYNTAX */
		"Syntax: SET channel FANTASY {trigger|RESET}",
		/* CHAN_SET_FANTASY_RESET */
		"Fantasy trigger for channel %s has been reset to default",
		/* CHAN_SET_FANTASY_TRIGGER */
		"Fantasy trigger for channel %s set to '%c'",
		/* CHAN_INFO_FANTASY */
		"Custom Fantasy Trigger: %c",
		/* MY_CHAN_HELP_SET */
		" \n"
		"Additional commands to SET:\n"
		" \n"
		"    FANTASY        Change channel's fantasy trigger",
		/* CHAN_HELP_SET_FANTASY */
		"Syntax: SET channel FANTASY {trigger|RESET}\n"
		" \n"
		"Sets a custom fantasy trigger for the given channel, the\n"
		"trigger must be only 1 character long.  If you wish to\n"
		"go back to using the original trigger, use RESET in place\n"
		"of a trigger."
	};
	char *langtable_de[] = {
		/* CHAN_SET_FANTASY_SYNTAX */
		"Syntax: SET channel FANTASY {trigger|RESET}",
		/* CHAN_SET_FANTASY_RESET */
		"Fantasy Trigger für den Channel %s wurde auf zurückgestellt\n"
		"  auf Standard",
		/* CHAN_SET_FANTASY_TRIGGER */
		"Fantasy Trigger für den Channel %s wurde gesetzt auf '%c'",
		/* CHAN_INFO_FANTASY */
		"Benutzerdefinierter Fantasy Trigger: %c",
		/* MY_CHAN_HELP_SET */
		" \n"
		"Zusätzliche Befehle für SET:\n"
		" \n"
		"    FANTASY        Verändert den Fantasy Trigger von einen Channel",
		/* CHAN_HELP_SET_FANTASY */
		"Syntax: SET channel FANTASY {trigger|RESET}\n"
		" \n"
		"Bestimmt den Fantasy Trigger für den Channel, der Trigger\n"
		"kann nur ein einzelnes Zeichen sein.  Wenn Du den originalen\n"
		"Trigger wieder nutzen möchtest, benutze RESET um den\n"
		"orignalen Trigger wiederherzustellen."
	};
	moduleInsertLanguage(LANG_EN_US, MY_NUM_STRINGS, langtable_en_us);
	moduleInsertLanguage(LANG_DE, MY_NUM_STRINGS, langtable_de);
}

/** Custom version of findMessage.
 * @param msgTable The message table to search in, will almost always be IRCD
 * @param name The name of the message we are looking for
 * @return A pointer to the message found, or NULL if it's not found
 *
 * This is a custom version of findMessage designed to find a core message only.
 * A core message is designated by it's lack of mod_name set in the Message structure.
 */
Message *myFindMessage(MessageHash *msgTable[], const char *name)
{
	int idx;
	MessageHash *current = NULL;
	if (!msgTable || !name)
		return NULL;

	idx = CMD_HASH(name);

	for (current = msgTable[idx]; current; current = current->next)
	{
		if (UseTokens)
		{
			if (ircd->tokencaseless)
			{
				if (!stricmp(name, current->name))
					break;
			}
			else
			{
				if (!strcmp(name, current->name))
					break;
			}
		}
		else
		{
			if (!stricmp(name, current->name))
				break;
		}
	}

	if (current)
	{
		Message *msg = NULL;

		for (msg = current->m; msg; msg = msg->next)
			if (!msg->mod_name)
				return msg;
	}

	return NULL;
}

/** Module initialization.
 * @param argc The number of arguments passed to the module
 * @param argv The arguments themselves
 *
 * Initializes the module by replacing the PRIVMSG messages with our own versions, creating a CHANSERV SET command,
 * hooking to events, adding the language strings, and loading the database.
 */
int AnopeInit(int argc, char **argv)
{
	Command *c = NULL;
	Message *m = NULL;
	EvtHook *hook = NULL;

	module_name = mod_current_module_name;

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	do_load_config();

	if (!s_BotServ)
	{
		alog("Error: bs_perchannelfantasy does not work with BotServ disabled!");
		return MOD_STOP;
	}

	m = myFindMessage(IRCD, "PRIVMSG");
	if (m)
	{
		EventPrivmsg = m->func;
		m->func = MyPrivmsg;
	}

	if (UseTokens)
	{
		m = myFindMessage(IRCD, "!");
		if (m)
			m->func = MyPrivmsg;
	}

	c = createCommand("SET", do_set, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(CHANSERV, c, MOD_HEAD);

	c = createCommand("SET FANTASY", NULL, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(CHANSERV, c, MOD_UNIQUE);
	moduleAddHelp(c, do_help_set_fantasy);

	c = createCommand("SET", do_nothing, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(CHANSERV, c, MOD_TAIL);
	moduleAddHelp(c, do_help_set);

	c = createCommand("INFO", do_info, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(CHANSERV, c, MOD_TAIL);

	hook = createEventHook(EVENT_DB_SAVING, do_save_db_evt);
	moduleAddEventHook(hook);

	hook = createEventHook(EVENT_DB_BACKUP, do_backup_db_evt);
	moduleAddEventHook(hook);

	do_add_languages();
	do_load_db();
	return MOD_CONT;
}

void AnopeFini()
{
	Message *m = myFindMessage(IRCD, "PRIVMSG");
	if (m)
		m->func = EventPrivmsg;

	if (UseTokens)
	{
		m = myFindMessage(IRCD, "!");
		if (m)
			m->func = EventPrivmsg;
	}

	do_save_db();
}
