/***************************************************************************************/
/* Anope Module : os_regflood.c : v1.0                                                 */
/* Phil Lavin - phil@anope.org                                                         */
/*                                                                                     */
/* (c) 2010 Phil Lavin                                                                 */
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

/*
Note:	This module requires Anope stable branch svn rev 2935 or above
		Alternatively, you can just patch revs 2929, 2931 and 2935 onto the 1.8.4 release
*/

/**
* Configuration directives that should be copy-pasted to services.conf

# MaxReg [OPTIONAL]
# Module: os_regflood
#
# Maximum number of combined nickname and channel registrations that
# can occur in MaxRegTime before they are disabled. They can only be
# manually re-enabled by a services admin.
#
#MaxReg 15

# MaxRegTime [OPTIONAL]
# Module: os_regflood
#
# Time in seconds in which, if MaxReg combined nickname and channel
# registrations are reached, nickname and channel registrations will
# be disabled. They can only be manually re-enabled by a services admin.
#
#MaxRegTime 60

# RegDisabled [OPTIONAL]
# Module: os_regflood
#
# Automatically disable nickname on channel registration when then
# module loads. Registration can only be manually re-enabled by a services
# admin.
#
#RegDisabled

*
**/

#include <module.h>

#define AUTHOR "Phil"
#define VERSION "1.0"

/* Defaults */
#define DEF_MAX_REG			15
#define DEF_MAX_REG_TIME	60
#define DEF_REG_DISABLED	0

/* Language Stuff */
#define LANG_NUM_STRINGS		7

#define USER_REG_DISABLED		0
#define ADMIN_REG_DISABLED		1
#define OS_HELP					2
#define OS_REGISTRATION_SYNTAX	3
#define OS_HELP_REGISTRATION	4
#define OS_REG_ENABLED			5
#define OS_REG_DISABLED			6

/*************************************************************************/

int myDoNSRegisterHead(User *u);
int myDoCSRegisterHead(User *u);
int handleReg(User *u, int isChan);
int myDoRegisterEvent(int argc, char **argv);
int cleanRegs(time_t t);
int numRegs();
int myDoRegistration(User *u);

int osHelpRegistration(User *u);
void osHelp(User *u);
void m_AddLanguages(void);

int mLoadConfig(void);
int loadConfParam(char *name, int paramType, int defaultVal, int *ptr);
int mEventReload(int argc, char **argv);

/*************************************************************************/

int maxReg;
int maxRegTime;
int regDisabled;
time_t *regs = NULL;
int offset = 0;
int myNSEmailReg = -1;

/*************************************************************************/

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

	c = createCommand("REGISTER", myDoNSRegisterHead, NULL, -1, -1, -1, -1, -1);
	status = moduleAddCommand(NICKSERV, c, MOD_HEAD);

	c = createCommand("REGISTER", myDoCSRegisterHead, NULL, -1, -1, -1, -1, -1);
	status = moduleAddCommand(CHANSERV, c, MOD_HEAD);

	c = createCommand("REGISTRATION", myDoRegistration, is_services_admin, -1, -1, -1, -1, -1);
	status = moduleAddCommand(OPERSERV, c, MOD_HEAD);
	moduleAddAdminHelp(c, osHelpRegistration);
	moduleSetOperHelp(osHelp);

	if (!NSEmailReg) {
		hook = createEventHook(EVENT_NICK_REGISTERED, myDoRegisterEvent);
		status = moduleAddEventHook(hook);
	}
	else {
		hook = createEventHook(EVENT_NICK_REQUESTED, myDoRegisterEvent);
		status = moduleAddEventHook(hook);
	}

	hook = createEventHook(EVENT_CHAN_REGISTERED, myDoRegisterEvent);
	status = moduleAddEventHook(hook);

	hook = createEventHook(EVENT_RELOAD, mEventReload);
	status = moduleAddEventHook(hook);

	mLoadConfig();
	m_AddLanguages();

	return MOD_CONT;
}

/**
* Unload the module
**/
void AnopeFini(void) {
	if (regs != NULL)
		free(regs);
}

/*************************************************************************/

/* Called when someone does /ns register before core processing */
int myDoNSRegisterHead(User *u)
{
	return handleReg(u, 0);
}

/* Called when someone does /cs register before core processing */
int myDoCSRegisterHead(User *u)
{
	return handleReg(u, 1);
}

/* Joint handler function for chan and nick registrations */
int handleReg(User *u, int isChan)
{
	char *src;

	if (isChan == 0) {
		src = s_NickServ;
	}
	else {
		src = s_ChanServ;
	}

	if (regDisabled == 1) {
		moduleNoticeLang(src, u, USER_REG_DISABLED);

		return MOD_STOP;
	}

	time_t now = time(NULL);

	// Clean array and minus from offset
	offset -= cleanRegs(now - maxRegTime);

	if (offset >= maxReg) {
		char *alert = moduleGetLangString(u, ADMIN_REG_DISABLED);

		alog("%s", alert);
		anope_cmd_global(s_OperServ, alert);

		regDisabled = 1;

		moduleNoticeLang(src, u, USER_REG_DISABLED);

		return MOD_STOP;
	}

	return MOD_CONT;
}

/* Called when a nickname or channel is registered */
int myDoRegisterEvent(int argc, char **argv)
{
	time_t now = time(NULL);

	// Add to array
	regs[offset] = now;
	offset++;
}

/* Function to remove registrations older than t from the array */
/* Returns the number of items removed */
int cleanRegs(time_t t)
{
	int cleanedCount = 0;
	int i = 0;

	// Remove the expired entries
	while (i < offset) {
		if (regs[i] <= t) {
			regs[i] = 0;
			cleanedCount++;
		}

		i++;
	}

	// Move all unexpired entries up to fill space left at start of array
	if (cleanedCount > 0) {
		i = cleanedCount;
		int j = 0;

		while (i < offset) {
			time_t tmp = regs[i];
			regs[i] = 0;
			regs[j] = tmp;

			i++;
			j++;
		}
	}

	return cleanedCount;
}

/* Called when someone does /os registration */
int myDoRegistration(User *u)
{
	char *text;
	char *param;

	text = moduleGetLastBuffer();

	if (text) {
		param = myStrGetToken(text, ' ', 0);

		if (param) {
			if (stricmp(param, "ENABLE") == 0) {
				regDisabled = 0;
				moduleNoticeLang(s_OperServ, u, OS_REG_ENABLED);
			}
			else if (stricmp(param, "DISABLE") == 0) {
				regDisabled = 1;
				moduleNoticeLang(s_OperServ, u, OS_REG_DISABLED);
			}
			else if (stricmp(param, "STATUS") == 0) {
				time_t now = time(NULL);

				offset -= cleanRegs(now - maxRegTime);

				notice_user(s_OperServ, u, "There has been %d registration(s) in the past %d seconds.", offset, maxRegTime);
				notice_user(s_OperServ, u, "Registrations are disabled after %d registrations in %d seconds.", maxReg, maxRegTime);
				notice_user(s_OperServ, u, " ");

				if (regDisabled == 0) {
					notice_user(s_OperServ, u, "Registrations are currently: ENABLED");
				}
				else {
					notice_user(s_OperServ, u, "Registrations are currently: DISABLED");
				}
			}
			else {
				moduleNoticeLang(s_OperServ, u, OS_REGISTRATION_SYNTAX);
			}

			free(param);
		}
	}
	else {
		moduleNoticeLang(s_OperServ, u, OS_REGISTRATION_SYNTAX);
	}

	return MOD_CONT;
}

/*************************************************************************/

int mLoadConfig(void)
{
	EvtHook *hook = NULL;
	int status;

	loadConfParam("MaxReg", PARAM_INT, DEF_MAX_REG, &maxReg);
	loadConfParam("MaxRegTime", PARAM_INT, DEF_MAX_REG_TIME, &maxRegTime);
	loadConfParam("RegDisabled", PARAM_SET, DEF_REG_DISABLED, &regDisabled);

	// Allocate memory to regs based on MaxReg directive
	if (regs == NULL) {
		regs = malloc(maxReg * sizeof(time_t));
	}
	// Reallocate if already allocated (i.e. we reloaded the config)
	else {
		regs = realloc(regs, maxReg * sizeof(time_t));
	}

	// Recreate event hooks
	if (myNSEmailReg != -1 && myNSEmailReg != NSEmailReg) {
		if (!NSEmailReg) {
			if ((status = moduleEventDelHook(EVENT_NICK_REQUESTED)) != MOD_ERR_OK) {
				alog("os_regflood: Error deleting EVENT_NICK_REQUESTED hook (%d)", status);
			}

			hook = createEventHook(EVENT_NICK_REGISTERED, myDoRegisterEvent);
			status = moduleAddEventHook(hook);
		}
		else {
			if ((status = moduleEventDelHook(EVENT_NICK_REGISTERED)) != MOD_ERR_OK) {
				alog("os_regflood: Error deleting EVENT_NICK_REGISTERED hook (%d)", status);
			}

			hook = createEventHook(EVENT_NICK_REQUESTED, myDoRegisterEvent);
			status = moduleAddEventHook(hook);
		}
	}

	myNSEmailReg = NSEmailReg;

	return MOD_CONT;
}

int loadConfParam(char *name, int paramType, int defaultVal, int *ptr)
{
	int tmp = 0;

	Directive directivas[] = {
		{name, {{paramType, PARAM_RELOAD, &tmp}}},
	};

	Directive *d = &directivas[0];
	moduleGetConfigDirective(d);

	if (tmp) {
		*ptr = tmp;
	} else {
		*ptr = defaultVal;
	}

	alog("os_regflood: Directive %s loaded (%d)...", name, *ptr);

	return 0;
}

int mEventReload(int argc, char **argv)
{
	int ret = 0;
	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			alog("os_regflood: Reloading configuration directives...");
			ret = mLoadConfig();
		}
	}

	if (ret)
		alog("os_regflood: An error has occured while reloading the configuration file");

	return MOD_CONT;
}

/*************************************************************************/

int osHelpRegistration(User *u)
{
	moduleNoticeLang(s_OperServ, u, OS_REGISTRATION_SYNTAX);
	notice_user(s_OperServ, u, " ");
	moduleNoticeLang(s_OperServ, u, OS_HELP_REGISTRATION, maxReg, maxRegTime);

	return MOD_CONT;
}

void osHelp(User *u) {
	if (is_services_admin(u)) {
		moduleNoticeLang(s_OperServ, u, OS_HELP);
	}
}

/*************************************************************************/

/* Language stuffs */
void m_AddLanguages(void)
{
	char *langtable_en_us[] = {
		/* USER_REG_DISABLED */
		"Services have detected a flood in registrations and have disabled the\n"
		"registration of nicknames and channels. Please join #help for further information.",
		/* ADMIN_REG_DISABLED */
		"Oper Alert: Services have detected a flood and have disabled registrations! "
		"Type /os REGISTRATION ENABLE to re-enable.",
		/* OS_HELP */
		"    REGISTRATION  Enable/disable nickname and channel registrations",
		/* OS_REGISTRATION_SYNTAX */
		"Syntax: REGISTRATION {DISABLE | ENABLE | STATUS}\n",
		/* OS_HELP_REGISTRATION */
		"Enables or disables nickname and channel registration. Registrations\n"
		"are automatically disabled after %d registrations in %d seconds. The\n"
		"STATUS parameter shows the status of the nickname registration list.",
		/* OS_REG_ENABLED */
		"Nickname and channel registrations have been ENABLED.",
		/* OS_REG_DISABLED */
		"Nickname and channel registrations have been DISABLED."
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}

/* EOF */
