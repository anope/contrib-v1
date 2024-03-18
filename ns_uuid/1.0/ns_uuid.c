/***************************************************************************************/
/* Anope Module : ns_uuid.c : v1.x                                                     */
/* Phil Lavin - phil@anope.org                                                         */
/*                                                                                     */
/* (c) 2010 Phil Lavin                                                                 */
/*                                                                                     */
/* Database stuff based on ircd_community_info.c by katsklaw                           */
/*                                                                                     */
/* This program is free software; you can redistribute it and/or modify it under the   */
/* terms of the GNU General Public License as published by the Free Software           */
/* Foundation; either version 1, or (at your option) any later version.                */
/*                                                                                     */
/*  This program is distributed in the hope that it will be useful, but WITHOUT ANY    */
/*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A    */
/*  PARTICULAR PURPOSE.  See the GNU General Public License for more details.          */
/*                                                                                     */
/*****************************************HEADER****************************************/

#include <module.h>
#include <math.h>

#define AUTHOR "Phil"
#define VERSION "1.0"

/* Default database name */
#define DEFAULT_DB_NAME "ns_uuid.db"

/* Multi-language stuff */
#define LANG_NUM_STRINGS	5

#define NSUUID_YOURID		0
#define NSUUID_THEID		1
#define NSUUID_SYNTAX		2
#define NSUUID_HELP_UUID	3
#define NSUUID_HELP			4

/****************************************TYPEDEFS***************************************/

/* None today */

/***************************************VARIABLES***************************************/

char *nsUUIdDBName = NULL;
int autoIncrement = 0;

/****************************************STRUCTS****************************************/

/* None today */

/**********************************FORWARD DECLARATIONS*********************************/

int AnopeInit(int argc, char **argv);
void AnopeFini(void);

int do_uuid(User *u);
void do_help(User *u);
int do_help_uuid(User *u);

int do_event_register(int argc, char **argv);
int do_event_save(int argc, char **argv);
int do_event_backup(int argc, char **argv);
int do_event_reload(int argc, char **argv);

char *mIntToChar(int cint);
int mCharToInt(char *cint);
int mIntSize();
int mLoadConfig(void);
int mloadConfParam(char *name, char *defaultVal, char **ptr);
int mLoadData(void);
void mSetUUIDs(void);
void mAddLanguages(void);

/***************************************ANOPE REQS**************************************/

/**
* AnopeInit is called when the module is loaded
* @param argc Argument count
* @param argv Argument list
* @return MOD_CONT to allow the module, MOD_STOP to stop it
**/
int AnopeInit(int argc, char **argv)
{
	Command *c;
	EvtHook *hook = NULL;
	int status;

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	alog("ns_uuid: Loading configuration directives...");
	if (mLoadConfig()) {
		return MOD_STOP;
	}

	c = createCommand("UUID", do_uuid, NULL, -1, -1, -1, -1, -1);
	status = moduleAddCommand(NICKSERV, c, MOD_HEAD);
	moduleAddHelp(c, do_help_uuid);
	moduleSetNickHelp(do_help);
	
	hook = createEventHook(EVENT_NICK_REGISTERED, do_event_register);
	status = moduleAddEventHook(hook);
	
	hook = createEventHook(EVENT_DB_SAVING, do_event_save);
	status = moduleAddEventHook(hook);

	hook = createEventHook(EVENT_DB_BACKUP, do_event_backup);
	status = moduleAddEventHook(hook);

	hook = createEventHook(EVENT_RELOAD, do_event_reload);
	status = moduleAddEventHook(hook);

	mLoadData();
	mAddLanguages();
	mSetUUIDs();

	return MOD_CONT;
}

/**
* Unload the module
**/
void AnopeFini(void)
{
	char *av[1];

	av[0] = sstrdup(EVENT_START);
	do_event_save(1, av);
	free(av[0]);

	if (nsUUIdDBName)
		free(nsUUIdDBName);
}

/************************************COMMAND HANDLERS***********************************/

/**
* Called when user does /ns uuid
* @param u The user who executed this command
* @return MOD_CONT if we want to process other commands in this command
* stack, MOD_STOP if we dont
**/
int do_uuid(User *u)
{
	char *text;
	char *nick;
	char *uuid;
	NickCore *nc = NULL;
	NickAlias *na = NULL;
	int is_servadmin = is_services_admin(u);

	text = moduleGetLastBuffer();

	/* Nick param was used */
	if (text && (nick = myStrGetToken(text, ' ', 0))) {
		if (!is_servadmin) {
			notice_lang(s_NickServ, u, ACCESS_DENIED);
		}
		else {
			if (!(na = findnick(nick))) {
				notice_lang(s_NickServ, u, NICK_SASET_BAD_NICK, nick);
			}
			else {
				nc = na->nc;
				
				if ((uuid = moduleGetData(&nc->moduleData, "uuid"))) {
					moduleNoticeLang(s_NickServ, u, NSUUID_THEID, nc->display, uuid);
				}
				else {
					notice_user(s_NickServ, u, "%s does not have a UUID! This shouldn't happen.", nc->display);
				}
			}
		}

		free(nick);
	}
	/* No nick param */
	else {
		if (!(na = findnick(u->nick))) {
			notice_lang(s_NickServ, u, NICK_NOT_REGISTERED);
		}
		else if (!nick_identified(u)) {
			notice_lang(s_NickServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
		}
		else {
			nc = na->nc;
			
			if ((uuid = moduleGetData(&nc->moduleData, "uuid"))) {
				moduleNoticeLang(s_NickServ, u, NSUUID_YOURID, uuid);
			}
			else {
				notice_user(s_NickServ, u, "You do not have a UUID! This shouldn't happen.");
			}
		}
	}

	return MOD_CONT;
}

/**
* Called after a user does a "/msg nickserv help"
* @param u The user who requested info
* @return MOD_CONT to continue processing commands or MOD_STOP to stop
**/
void do_help(User *u)
{
	moduleNoticeLang(s_ChanServ, u, NSUUID_HELP);
}

/**
* Called after a user does a "/msg nickserv help uuid"
* @param u The user who requested info
* @return MOD_CONT to continue processing commands or MOD_STOP to stop
**/
int do_help_uuid(User *u)
{
	moduleNoticeLang(s_ChanServ, u, NSUUID_SYNTAX);
	notice_user(s_ChanServ, u, " \n");
	moduleNoticeLang(s_ChanServ, u, NSUUID_HELP_UUID);
	
	return MOD_CONT;
}

/*************************************EVENT HANDLERS************************************/

/**
* Handle nick registrations
* @return 0 for success
**/
int do_event_register(int argc, char **argv)
{
	NickCore *nc = NULL;
	char *uuid;

	if ((nc = findcore(argv[0]))) {
		uuid = mIntToChar(autoIncrement);
		moduleAddData(&nc->moduleData, "uuid", uuid);
		free(uuid);

		autoIncrement++;
	}
	
	return 0;
}

/**
* Save all our data to our db file
* @return 0 for success
**/
int do_event_save(int argc, char **argv)
{
	NickCore *nc = NULL;
	int i = 0;
	int ret = 0;
	FILE *out;
	char *info = NULL;

	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			if ((out = fopen(nsUUIdDBName, "w")) == NULL) {
				alog("ns_uuid: ERROR: can not open the database file!");
				anope_cmd_global(s_OperServ, "ns_uuid: ERROR: can not open the database file!");
				ret = 1;
			} else {
				for (i = 0; i < 1024; i++) {
					for (nc = nclists[i]; nc; nc = nc->next) {
						/* If we have any info on this core */
						if ((info = moduleGetData(&nc->moduleData, "uuid"))) {
							fprintf(out, "%s %s\n", nc->display, info);
							free(info);
						}
					}
				}
				
				// Store the auto increment at the end of the database as e.g. "* 7"
				fprintf(out, "* %d\n", autoIncrement);

				fclose(out);
			}
		} else {
			ret = 0;
		}
	}

	return ret;
}

/**
* Backup our databases using the commands provided by Anope
* @return MOD_CONT
**/
int do_event_backup(int argc, char **argv)
{
	if (!stricmp(argv[0], EVENT_START))
	ModuleDatabaseBackup(nsUUIdDBName);

	return MOD_CONT;
}

/**
* Manage the RELOAD EVENT
* @return MOD_CONT
**/
int do_event_reload(int argc, char **argv)
{
	int ret = 0;
	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			alog("ns_uuid: Reloading configuration directives...");
			ret = mLoadConfig();
		}
	}

	if (ret)
	alog("ns_uuid: ERROR: An error has occured while reloading the configuration file");

	return MOD_CONT;
}

/************************************CUSTOM FUNCTIONS***********************************/

/**
* Convert value referenced by char pointer to int
* @return int
**/
int mCharToInt(char *cint)
{
	int out;

	if (sizeof(int) == sizeof(long) || sizeof(int) == sizeof(int)) {
		out = strtol(cint, (char **) NULL, 10);
	}
	else if (sizeof(int) == sizeof(long long)) {
		out = strtoll(cint, (char **) NULL, 10);
	}
	
	return out;
}

/**
* Convert value referenced by char pointer to int
* @return char*
**/
char *mIntToChar(int cint)
{
	char *out = NULL;	
	out = malloc(mIntSize());
	
	sprintf(out, "%d", cint);
	
	return out;
}

/**
* Get number of chars an int would consume as a string (including \0)
* @return unsigned long
**/
int mIntSize()
{
	long double largestVal;
	int i = 0;
	
	largestVal = pow(2, sizeof(int) * 8);
	
	while (largestVal >= 10) {
		largestVal /= 10;
		i++;
	}
	
	return i + 1;
}

/**
* Load the configuration directives from Services configuration file.
* @return 0 for success
**/
int mLoadConfig(void)
{
	mloadConfParam("nsUUIdDBName", DEFAULT_DB_NAME, &nsUUIdDBName);
	
	return 0;
}

/**
* Load a configuration directive from Services configuration file.
* @return 0 for success
**/
int mloadConfParam(char *name, char *defaultVal, char **ptr)
{
	char *tmp = NULL;

	Directive directivas[] = {
		{name, {{PARAM_STRING, PARAM_RELOAD, &tmp}}},
	};

	Directive *d = &directivas[0];
	moduleGetConfigDirective(d);

	if (tmp) {
		*ptr = tmp;
	} else {
		*ptr = sstrdup(defaultVal);
	}

	alog("ns_uuid: Directive %s loaded (%s)...", name, *ptr);

	return 0;
}

/**
* Load data from the db file
* @return 0 for success
**/
int mLoadData(void)
{
	int ret = 0;
	FILE *in;

	char *nick = NULL;
	char *uuid = NULL;
	int len = 0;

	NickCore *nc = NULL;

	/* will _never_ be this big thanks to the limit of nick lengths */
	char buffer[2000];
	
	if ((in = fopen(nsUUIdDBName, "r")) == NULL) {
		alog("ns_uuid: WARNING: can not open the %s database file! (it might not exist, this is not fatal)", nsUUIdDBName);
		ret = 1;
	} else {
		while (fgets(buffer, 1500, in)) {
			nick = myStrGetToken(buffer, ' ', 0);
			uuid = myStrGetToken(buffer, ' ', 1);
			
			if (nick) {
				if (uuid) {
					len = strlen(uuid);
					/* Take the \n from the end of the line */
					uuid[len - 1] = '\0';
					
					// If it's the auto increment value we're loading
					if (strcmp(nick, "*") == 0) {
						alog("ns_uuid: Loaded auto increment value of %s from the database", uuid);
						autoIncrement = mCharToInt(uuid);
					}
					else if ((nc = findcore(nick))) {
						moduleAddData(&nc->moduleData, "uuid", uuid);
					}
					free(uuid);
				}
				free(nick);
			}
		}
	}
	return ret;
}

/**
* Sets a UUID on any nick core which doesn't have one set
**/
void mSetUUIDs(void)
{
	NickCore *nc = NULL;
	int i = 0;
	char *info;
	char *uuid;
	
	for (i = 0; i < 1024; i++) {
		for (nc = nclists[i]; nc; nc = nc->next) {
			if (!(info = moduleGetData(&nc->moduleData, "uuid"))) {
				uuid = mIntToChar(autoIncrement);
				moduleAddData(&nc->moduleData, "uuid", uuid);
				free(uuid);
				
				autoIncrement++;
			}
			else {
				free(info);
			}
		}
	}
}

/**
* Manages the multilanguage stuff
**/
void mAddLanguages(void)
{
	char *langtable_en_us[] = {
		/* NSUUID_YOURID */
		"Your UUID is: %s",
		/* NSUUID_THEID */
		"The UUID for %s is: %s",
		/* NSUUID_SYNTAX */
		"Syntax: UUID [nickname]",
		/* NSUUID_HELP_UUID */
		"Gets the Universally Unique Identifier for your nickname. The UUID is unique to\n"
		"a nickname group on a single services install. The optional nickname parameter\n"
		"can be used by services admins to obtain the UUID of another nickname group",
		/* NSUUID_HELP */
		"    UUID       Gets the Universally Unique Identifier for your nickname"
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}

/* EOF */
