#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: os_netstaff.c v1.0.1 06-02-2008 n00bie $"

/*
************************************************************************************************
** Module	: os_netstaff.c
** Version	: 1.0.1
** Author	: n00bie (n00bie@rediffmail.com)
** Release	: 17th October, 2007
** Update	: 6th February, 2008
************************************************************************************************
** Description:
**
** Add or delete network staff for the given nick. The network staff will be given special
** privileges without the usual IRCop privileges upon identifying to services. The NETSTAFF
** command without giving any params will display the normal Network Staff nicks. 
** The CLEAR command will remove all network staffs from the database.
**
** The ADD/DEL & CLEAR commands can be used only by Services Admins and above whereas
** Services Operators can only view the Network Staff lists.
**
** Note: SWHOIS currently works on UnrealIRCd only(?) and CHGHOST command used on this module
** are only supported by Unreal, Inspircd, Ultimate, Rageircd and Solidircd atm :s
************************************************************************************************
** Tested: Anope-1.7.21, Unreal3.2.7
**
** Providing Command:
**
** /msg OperServ HELP NETSTAFF
** /msg OperServ NETSTAFF [ADD|DEL|CLEAR] [nick]
** /msg OperServ NETSTAFF
**
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
** Module made possible using the database routines (os_info.c) by Rob
** This module have 4 configurable option.

** Copy/paste below on services.conf

# NetStaffHost [REQUIRED]
# Module: os_netstaff
#
# Define the staff virtual host which will be set upon
# identifying to services. This is not a permanent vhost.
#
NetStaffHost "Staff.LocalHost-IRC.Network"

# NetStaffDBName [OPTIONAL]
#
# Use the given filename as database to store the Staff db's.
# If not given, the default "os_netstaff.db" will be used.
#
NetStaffDBName "os_netstaff.db" 

# NetStaffModes [OPTIONAL]
#
# Define what user modes the staff user will get
# upon identifying to services. Comment this if
# you do not want to give them user modes.
#
NetStaffModes "+gswhGW"
					
# NetStaffSnomask [OPTIONAL]
#
# Define here what SNOMASK the staff will get
# upon identifying to services. If your'e not
# using UnrealIRCd, comment this.
#
NetStaffSnomask "+ks"

*/

#define DEFAULT_DB_NAME "os_netstaff.db"
#define LANG_NUM_STRINGS			15
#define OS_NETSTAFF_OPER_HELP		0
#define OS_NETSTAFF_ADMIN_SYNTAX	1
#define OS_NETSTAFF_ADMIN_HELP		2
#define OS_NETSTAFF_ADD_SUCCESS		3
#define OS_NETSTAFF_DEL_SUCCESS		4
#define OS_NETSTAFF_EXIST			5
#define OS_NETSTAFF_NOT_EXIST		6
#define OS_NETSTAFF_HELP_OPER		7
#define OS_NETSTAFF_HELP_ADMIN		8
#define OS_NETSTAFF_EMPTY			9
#define OS_NETSTAFF_LIST_HEADER		10
#define OS_NETSTAFF_LIST_FOOTER		11
#define OS_NETSTAFF_LIST_CLEARED	12
#define OS_NETSTAFF_SWHOIS			13
#define OS_NETSTAFF_NOTICE			14

char *NetStaffDBName = NULL;
char *NetStaffHostmask;
char *NetStaffUserModes;
char *NetStaffSnomasks;
void doAddLanguages(void);
void doOperServHelp(User *u);
int doLoadConfig(void);
int doLoadData(void);
int doNetStaff(User *u);
int doNetStaffHelp(User *u);
int doNetStaffList(User *u);
int doNetStaffClear(User *u);
int doNickIdentify(User *u);
int doSaveData(int argc, char **argv);
int doBackupData(int argc, char **argv);
int doEventReload(int argc, char **argv);
int AnopeInit(int argc, char** argv)
{
	Command *c;
	EvtHook *hook = NULL;
	int status = 0;
	if (doLoadConfig()) {
        return MOD_STOP;
    }
	hook = createEventHook(EVENT_DB_SAVING, doSaveData);
    status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("os_netstaff: Something isn't init right.");
		return MOD_STOP;
	} else {
		alog("os_netstaff: Successfully hooked to EVENT_DB_SAVING.");
	}
    hook = createEventHook(EVENT_DB_BACKUP, doBackupData);
    status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("os_netstaff: Something isn't init right.");
		return MOD_STOP;
	} else {
		alog("os_netstaff: Successfully hooked to EVENT_DB_BACKUP.");
	}
    hook = createEventHook(EVENT_RELOAD, doEventReload);
    status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("os_netstaff: Something isn't init right.");
		return MOD_STOP;
	} else {
		alog("os_netstaff: Successfully hooked to EVENT_RELOAD.");
	}
	c = createCommand("ID", doNickIdentify, NULL, -1, -1, -1, -1, -1);
	status = moduleAddCommand(NICKSERV, c, MOD_TAIL);
	if (status != MOD_ERR_OK) {
		alog("os_netstaff: Something isn't init right.");
		return MOD_STOP;
	} 
	c = createCommand("IDENTIFY", doNickIdentify, NULL, -1, -1, -1, -1, -1);
	status = moduleAddCommand(NICKSERV, c, MOD_TAIL);
	if (status != MOD_ERR_OK) {
		alog("os_netstaff: Something isn't init right.");
		return MOD_STOP;
	} 
	c = createCommand("NETSTAFF", doNetStaff, is_oper, -1, -1, -1, -1, -1);
	moduleAddHelp(c, doNetStaffHelp);
	moduleSetOperHelp(doOperServHelp);
	status = moduleAddCommand(OPERSERV, c, MOD_HEAD);
	if (status != MOD_ERR_OK) {
		alog("os_netstaff: Something isn't init right.");
		return MOD_STOP;
	} else {
		alog("os_netstaff: New command: \2/msg %s HELP NETSTAFF\2", s_OperServ);
		alog("os_netstaff: Successfully loaded module.");
	}
	doLoadData();
    doAddLanguages();
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void)
{
    char *av[1];
    av[0] = sstrdup(EVENT_START);
    doSaveData(1, av);
    free(av[0]);
	if (NetStaffDBName) {
		free(NetStaffDBName);
	}
	if (NetStaffHostmask) {
		free(NetStaffHostmask);
	}
	if (NetStaffUserModes) {
		free(NetStaffUserModes);
	}
	if (NetStaffSnomasks) {
		free(NetStaffSnomasks);
	}
	alog("os_netstaff: Module unloaded.");
}

void doOperServHelp(User *u)
{
	if (is_services_admin(u)) {
		moduleNoticeLang(s_OperServ, u, OS_NETSTAFF_HELP_ADMIN);
	} else {
		moduleNoticeLang(s_OperServ, u, OS_NETSTAFF_HELP_OPER);
	}
}

int doBackupData(int argc, char **argv)
{
	ModuleDatabaseBackup(NetStaffDBName);
	return MOD_CONT;
}

int doEventReload(int argc, char **argv)
{
    int ret = 0;
    if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			ret = doLoadConfig();
        }
	}
	if (ret) {
		alog("os_netstaff: \2ERROR\2: An error has occured while reloading the configuration file!");
	}
    return MOD_CONT;
}

int doLoadConfig(void)
{
	int i;
	int retval = 0;
    char *tmp = NULL;
	Directive confvalues[][1] = {
		{{"NetStaffDBName", {{PARAM_STRING, 0, &tmp}}}},
		{{"NetStaffHost", {{PARAM_STRING, 0, &NetStaffHostmask}}}},
		{{"NetStaffModes", {{PARAM_STRING, 0, &NetStaffUserModes}}}},
		{{"NetStaffSnomask", {{PARAM_STRING, 0, &NetStaffSnomasks}}}},
	};
    for (i = 0; i < 4; i++)
        moduleGetConfigDirective(confvalues[i]);
	
	if (NetStaffDBName)
		free(NetStaffDBName);

	if (!NetStaffHostmask) {
		alog("os_netstaff: Error: \2NetStaffHost\2 is not defined in Services configuration file");
		retval = 1;
	}
    
	if (tmp) {
        NetStaffDBName = tmp;
    } else {
        NetStaffDBName = sstrdup(DEFAULT_DB_NAME);
	}
    
	return retval;
}

int doNetStaffHelp(User *u)
{
	if (is_services_admin(u)) {
		moduleNoticeLang(s_OperServ, u, OS_NETSTAFF_ADMIN_SYNTAX);
		notice(s_OperServ, u->nick, " ");
		moduleNoticeLang(s_OperServ, u, OS_NETSTAFF_ADMIN_HELP);
	} else {
		moduleNoticeLang(s_OperServ, u, OS_NETSTAFF_OPER_HELP);
	} 
    return MOD_STOP;
}

int doNetStaffList(User *u)
{
	FILE *in;
	int i = 0, count = 0, match_count = 0, ret = 0;
	NickCore *nc = NULL;
	if ((in = fopen(NetStaffDBName, "r")) == NULL) {
		alog("os_netstaff: \2ERROR\2: cannot open the database file!");
		ret = 1;
	} else {
		moduleNoticeLang(s_OperServ, u, OS_NETSTAFF_LIST_HEADER, NetworkName);
		for (i = 0; i < 1024; i++) {
			for (nc = nclists[i]; nc; nc = nc->next) {
				count++;
				if (moduleGetData(&nc->moduleData, "staff")) {
					match_count++;
					notice_user(s_OperServ, u, "%3d  %s", match_count, nc->display);
				}
			}
		}
		if (!match_count) {
			moduleNoticeLang(s_OperServ, u, OS_NETSTAFF_EMPTY);
		}
		moduleNoticeLang(s_OperServ, u, OS_NETSTAFF_LIST_FOOTER);
	}
	return ret;
}

int doNetStaffClear(User *u)
{
	FILE *out;
	int i = 0, ret = 0;
	NickCore *nc = NULL;
	if ((out = fopen(NetStaffDBName, "w")) == NULL) {
		alog("os_netstaff: \2ERROR\2: cannot open the database file!");
		ret = 1;
	} else {
		for (i = 0; i < 1024; i++) {
			for (nc = nclists[i]; nc; nc = nc->next) {
				if (moduleGetData(&nc->moduleData, "staff")) {
					moduleDelData(&nc->moduleData, "staff");
				}
			}
		}
		moduleNoticeLang(s_OperServ, u, OS_NETSTAFF_LIST_CLEARED);
		fclose(out);
	}
	return ret;
}

int doNetStaff(User *u)
{
	NickAlias *na = NULL;
	if (is_services_admin(u)) {
		char *text = NULL;
		char *cmd = NULL;
		char *nick = NULL;
		char *info = NULL;
		text = moduleGetLastBuffer();
		if (text) {
			cmd = myStrGetToken(text, ' ', 0);
			nick = myStrGetToken(text, ' ', 1);
			if (cmd && nick) {
				if (strcasecmp(cmd, "ADD") == 0) {
					if ((na = findnick(nick))) {
						if ((info = moduleGetData(&na->nc->moduleData, "staff"))) {
							moduleNoticeLang(s_OperServ, u, OS_NETSTAFF_EXIST, info);
						} else {
							moduleAddData(&na->nc->moduleData, "staff", nick);
							moduleNoticeLang(s_OperServ, u, OS_NETSTAFF_ADD_SUCCESS, nick, NetworkName);
						}
					} else {
						notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, nick);
					}
				} else if (strcasecmp(cmd, "DEL") == 0) {
					if ((na = findnick(nick))) {
						if ((info = moduleGetData(&na->nc->moduleData, "staff"))) {
							moduleDelData(&na->nc->moduleData, "staff");
							moduleNoticeLang(s_OperServ, u, OS_NETSTAFF_DEL_SUCCESS, nick, NetworkName);
						} else {
							moduleNoticeLang(s_OperServ, u, OS_NETSTAFF_NOT_EXIST, nick);
						}
					} else {
						notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, nick);
					}
				} else {
					moduleNoticeLang(s_OperServ, u, OS_NETSTAFF_ADMIN_SYNTAX);
				}
			} else if (cmd) {
				if (strcasecmp(cmd, "CLEAR") == 0) {
					doNetStaffClear(u);
				} else {
					moduleNoticeLang(s_OperServ, u, OS_NETSTAFF_ADMIN_SYNTAX);
				}
			} else {
				moduleNoticeLang(s_OperServ, u, OS_NETSTAFF_ADMIN_SYNTAX);
			}
			free(info);
			free(cmd);
			free(nick);
		} else {
			doNetStaffList(u);
		}
	} else {
		doNetStaffList(u);
	}
	return MOD_CONT;
}

int doLoadData(void)
{
    int ret = 0;
    FILE *in;
    char *cmd = NULL;
    char *nick = NULL;
    int len = 0;
    NickAlias *na = NULL;
    char buffer[2000];
	if ((in = fopen(NetStaffDBName, "r")) == NULL) {
		alog("os_netstaff: \2WARNING\2: cannot open the database file! (it might not exist, this is not fatal!)");
        ret = 1;
    } else {
		while (fgets(buffer, 1500, in)) {
            cmd = myStrGetToken(buffer, ' ', 0);
            nick = myStrGetToken(buffer, ' ', 1);
			if (cmd) {
				if (nick) {
					len = strlen(nick);
					nick[len - 1] = '\0';
					if (stricmp(cmd, "S") == 0) {
						if ((na = findnick(nick))) {
							moduleAddData(&na->nc->moduleData, "staff", nick);
						}
					}
					free(nick);
				}
				free(cmd);
			}
		}
	}
	return ret;
}

int doSaveData(int argc, char **argv)
{
	NickCore *nc = NULL;
    int i = 0;
    int ret = 0;
    FILE *out;
    char *nick = NULL;
    if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
            if ((out = fopen(NetStaffDBName, "w")) == NULL) {
                alog("os_netstaff: \2ERROR\2: cannot open the database file!");
				if (WallOSGlobal) {
					anope_cmd_global(s_OperServ, "os_netstaff: \2ERROR\2: cannot open the database file!");
				}
				ret = 1;
			} else {
				for (i = 0; i < 1024; i++) {
					for (nc = nclists[i]; nc; nc = nc->next) {
						if ((nick = moduleGetData(&nc->moduleData, "staff"))) {
                            fprintf(out, "S %s\n", nc->display);
							free(nick);
						}
					}
				}
				fclose(out);
			}
		} else {
			ret = 0;
        }
    }
    return ret;
}

int doNickIdentify(User *u)
{
	NickAlias *na = findnick(u->nick);
	char *str;
	if (nick_identified(u)) {
		if (moduleGetData(&na->nc->moduleData, "staff")) {
			if ((str = moduleGetLangString(u, OS_NETSTAFF_SWHOIS))) {
				anope_cmd_swhois(s_OperServ, na->nick, str);
			}
			if (NetStaffUserModes) {
				if (!stricmp(IRCDModule, "unreal32")) {
					if (UseSVS2MODE) {
						send_cmd(s_OperServ, "SVS2MODE %s %s", na->nick, NetStaffUserModes);
					} else {
						send_cmd(s_OperServ, "SVSMODE %s %s", na->nick, NetStaffUserModes);
					}
				} else {
					send_cmd(s_OperServ, "SVSMODE %s %s", na->nick, NetStaffUserModes);
				}
			}
			if (NetStaffSnomasks) {
				if (!stricmp(IRCDModule, "unreal32")) {
					if (UseSVS2MODE) {
						send_cmd(s_OperServ, "SVS2SNO %s %s", na->nick, NetStaffSnomasks);
					} else {
						send_cmd(s_OperServ, "SVSSNO %s %s", na->nick, NetStaffSnomasks);
					}
				}
			}
			if (!stricmp(IRCDModule, "unreal31") || 
				!stricmp(IRCDModule, "unreal32") ||
				!stricmp(IRCDModule, "inspircd10") ||
				!stricmp(IRCDModule, "inspircd11")) {
				send_cmd(s_OperServ, "CHGHOST %s %s", na->nick, NetStaffHostmask);
			} else if (!stricmp(IRCDModule, "rageircd")) {
				send_cmd(s_OperServ, "VHOST %s %s", na->nick, NetStaffHostmask);
			} else if (!stricmp(IRCDModule, "solidircd")) {
				send_cmd(s_OperServ, "SVHOST %s %s", na->nick, NetStaffHostmask);
			}
			moduleNoticeLang(s_NickServ, u, OS_NETSTAFF_NOTICE);
			if (WallOSGlobal) {
				anope_cmd_global(s_OperServ, "\2%s\2 [%s] is now a Network Staff.", na->nick, na->nc->display);
			}
		}
	}
	return MOD_CONT;
} 

void doAddLanguages(void)
{
    char *langtable_en_us[] = {
		/* OS_NETSTAFF_OPER_HELP */
		"Syntax: \2NETSTAFF\2\n"
		"Displays all Network Staff nicks.",
		/* OS_NETSTAFF_ADMIN_SYNTAX */
		"Syntax: \2NETSTAFF [ADD|DEL|CLEAR] \037nick\037\2",
		/* OS_NETSTAFF_ADMIN_HELP */
		"Add or delete network staff for the given nick.\n"
		"The network staff will be given special privilege\n"
		"without the usual IRCop privileges. The NETSTAFF\n"
		"command without any params will display the normal\n"
		"Network Staff nicks. The CLEAR command will remove\n"
		"all network staff from the database.",
		/* OS_NETSTAFF_ADD_SUCCESS */
		"\2%s\2 added to %s network staff list.",
		/* OS_NETSTAFF_DEL_SUCCESS */
		"\2%s\2 deleted from %s network staff list.",
		/* OS_NETSTAFF_EXIST */
		"\2%s\2 already exists on network staff list.",
		/* OS_NETSTAFF_NOT_EXIST */
		"\2%s\2 not found on network staff list.",
		/* OS_NETSTAFF_HELP_OPER */
		"    NETSTAFF    Display network staff list",
		/* OS_NETSTAFF_HELP_ADMIN */
		"    NETSTAFF    Manage network staff",
		/* OS_NETSTAFF_EMPTY */
		"Network Staff list is empty.",
		/* OS_NETSTAFF_LIST_HEADER */
		"%s Network Staff:\n"
		"\037Num\037  \037Nickname\037",
		/* OS_NETSTAFF_LIST_FOOTER */
		"End of list.",
		/* OS_NETSTAFF_LIST_CLEARED */
		"All network staff has been cleared.",
		/* OS_NETSTAFF_SWHOIS */
		"is a Network Staff",
		/* OS_NETSTAFF_NOTICE */
		"You are now a network staff."
    };
	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}
/* EOF */
