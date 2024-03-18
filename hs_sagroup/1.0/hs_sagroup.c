/***************************************************************************************/
/* Anope Module : hs_sagroup.c : v1.x                                                  */
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
/*****************************************HEADER****************************************/

#include <module.h>
#include <math.h>

#define AUTHOR "Phil"
#define VERSION "1.0"

/* Multi-language stuff */
#define LANG_NUM_STRINGS			4

#define HSSAGROUP_SUCCESS			0
#define HSSAGROUP_SYNTAX			1
#define HSSAGROUP_HELP_SAGROUP		2
#define HSSAGROUP_HELP				3

/****************************************TYPEDEFS***************************************/

// None here

/***************************************VARIABLES***************************************/

// None here

/****************************************STRUCTS****************************************/

// None here

/**********************************FORWARD DECLARATIONS*********************************/

int AnopeInit(int argc, char **argv);
void AnopeFini(void);

int do_group(char * nick, User * u);
int do_sagroup(User *u);
int do_help_sagroup(User *u);
void do_help(User *u);

void mAddLanguages(void);

extern int do_hs_sync(NickCore * nc, char *vIdent, char *hostmask, char *creator, time_t time);

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
	int status;

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	c = createCommand("SAGROUP", do_sagroup, is_services_admin, -1, -1, -1, -1, -1);
	status = moduleAddCommand(HOSTSERV, c, MOD_HEAD);
	moduleAddHelp(c, do_help_sagroup);
	moduleSetHostHelp(do_help);

	mAddLanguages();

	return MOD_CONT;
}

/**
* Unload the module
**/
void AnopeFini(void)
{

}

/************************************COMMAND HANDLERS***********************************/

/**
* Called on /hs sagroup
* @param u The user who executed this command
* @return MOD_CONT if we want to process other commands in this command
* stack, MOD_STOP if we dont
**/
int do_sagroup(User *u)
{
	char *text = NULL;
	char *nick = NULL;
	int rtn;
	ChannelInfo *ci = NULL;

	if (!(text = moduleGetLastBuffer())) {
		moduleNoticeLang(s_ChanServ, u, HSSAGROUP_SYNTAX, ci->name);
		rtn = MOD_STOP;
	}
	else if (!(nick = myStrGetToken(text, ' ', 0))) {
		moduleNoticeLang(s_ChanServ, u, HSSAGROUP_SYNTAX, ci->name);
		rtn = MOD_STOP;
	}
	else if (!findnick(nick)) {
		notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, nick);
		rtn = MOD_STOP;
	}
	else {
		rtn = do_group(nick, u);
	}
	
	if (nick)
		free(nick);
	
	return rtn;
}

/*************************************EVENT HANDLERS************************************/

// None here

/************************************CUSTOM FUNCTIONS***********************************/

/**
 * Borrowed from hs_group in the core of git tree 8246116bd11f549c30cb2a7bcfd55cd8b59eec03
 * Modified for purpose
 * @param nick The nickname of the user to group the host for
 * @param u The user who is doing the grouping
 * @param MOD_CONT to continue processing other modules, MOD_STOP to stop processing.
 **/
int do_group(char * nick, User * u)
{
    NickAlias *na;
    HostCore *tmp;
    char *vHost = NULL;
    char *vIdent = NULL;
    char *creator = NULL;
    HostCore *head = NULL;
    time_t time;
    boolean found = false;

    head = hostCoreListHead();

    if ((na = findnick(nick))) {
		tmp = findHostCore(head, na->nick, &found);
		if (found) {
			if (tmp == NULL) {
				tmp = head; /* incase first in list */
			} else if (tmp->next) { /* we dont want the previous entry were not inserting! */
				tmp = tmp->next;    /* jump to the next */
			}

			vHost = sstrdup(tmp->vHost);
			if (tmp->vIdent)
				vIdent = sstrdup(tmp->vIdent);
			creator = sstrdup(tmp->creator);
			time = tmp->time;

			do_hs_sync(na->nc, vIdent, vHost, creator, time);
			if (tmp->vIdent) {
				notice_lang(s_HostServ, u, HOST_IDENT_GROUP,
							na->nc->display, vIdent, vHost);
			} else {
				notice_lang(s_HostServ, u, HOST_GROUP, na->nc->display,
							vHost);
			}
			alog("%s: %s!%s@%s used SAGROUP to group the vhost of %s",
				 s_HostServ, u->nick, u->username, u->host, na->nick);
			free(vHost);
			if (vIdent)
				free(vIdent);
			free(creator);

		} else {
			notice_lang(s_HostServ, u, HOST_NOT_ASSIGNED);
		}
    } else {
        notice_lang(s_HostServ, u, HOST_NOT_REGED);
    }
    return MOD_CONT;
}

/**
* Called after a user does a "/msg hostserv help"
* @param u The user who requested info
* @return MOD_CONT to continue processing commands or MOD_STOP to stop
**/
void do_help(User *u)
{
	if (is_services_admin(u)) {
		moduleNoticeLang(s_ChanServ, u, HSSAGROUP_HELP);
	}
}

/**
* Called after a user does a "/msg hostserv help sagroup"
* @param u The user who requested info
* @return MOD_CONT to continue processing commands or MOD_STOP to stop
**/
int do_help_sagroup(User *u)
{
	moduleNoticeLang(s_ChanServ, u, HSSAGROUP_SYNTAX);
	notice_user(s_ChanServ, u, " \n");
	moduleNoticeLang(s_ChanServ, u, HSSAGROUP_HELP_SAGROUP);

	return MOD_CONT;
}

/**
* Manages the multilanguage stuff
**/
void mAddLanguages(void)
{
	char *langtable_en_us[] = {
		/* HSSAGROUP_SUCCESS */
		"All vhost's in the group %s have been set to %s",
		/* HSSAGROUP_SYNTAX */
		"Syntax: SAGROUP nickname",
		/* HSSAGROUP_HELP_SAGROUP */
		"This command allows a services admin to set the vhost of a user's\n"
		"nickname group to that of the nickname specified in the nickname\n"
		"parameter.",
		/* HSSAGROUP_HELP */
		"    SAGROUP     Syncs the vhost for all nicks in a group",
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}

/* EOF */
