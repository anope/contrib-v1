#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: ns_setnoop.c v1.0.0 05-09-2007 n00bie $"

/*
*******************************************************************************************
** Module	: ns_setnoop.c
** Author	: n00bie (n00bie@rediffmail.com)
** Version	: 1.0.0
** Release	: 5th September, 2007
*******************************************************************************************
** Description:
**
** Stops users from adding you to channel access lists. When this is ON, ChanServ will
** not allow your nick to be added to the channel access lists in any channels. Whoever 
** tries to add you will get a notice from ChanServ back saying that you have NOOP set ON.
** The STATUS param will return your current NOOP status.
*******************************************************************************************
** Providing Command:
**
** /msg NickServ HELP SET NOOP
** /msg NickServ SET NOOP {ON | OFF | STATUS}
**
*******************************************************************************************
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
*******************************************************************************************
** This module have 1 configurable option.
*******************************************************************************************
** Copy/paste below on services.conf

# NSNoopDBName [OPTIONAL]
# Module: ns_setnoop
#
# Use the given filename as database to store the NOOP.
# If not given, the default "ns_setnoop.db" will be used.
#
NSNoopDBName "ns_noop.db"

*/

#define DEFAULT_DB_NAME "ns_noop.db"
char *NSNoopDBName;
int mySet(User *u);
int myVOP(User *u);
int myHOP(User *u);
int myAOP(User *u);
int mySOP(User *u);
int myAccess(User *u);
int myNickServHelpSet(User *u);
int myNickServHelpSetNoop(User *u);
int mLoadData(void);
int mSaveData(int argc, char **argv);
int mLoadConfig(int argc, char **argv);
void myNickServHelp(User *u);
int AnopeInit(int argc, char **argv)
{
	Command *c;
	int status = 0;
	EvtHook *hook = NULL;
	NSNoopDBName = NULL;
	c = createCommand("VOP", myVOP, NULL, -1, -1, -1, -1, -1);
    status = moduleAddCommand(CHANSERV, c, MOD_HEAD);
    if (ircd->halfop) {
        c = createCommand("HOP", myHOP, NULL, -1, -1, -1, -1, -1);
        status = moduleAddCommand(CHANSERV, c, MOD_HEAD);
    }
	c = createCommand("AOP", myAOP, NULL, -1, -1, -1, -1, -1);
    status = moduleAddCommand(CHANSERV, c, MOD_HEAD);
	c = createCommand("SOP", mySOP, NULL, -1, -1, -1, -1, -1);
    status = moduleAddCommand(CHANSERV, c, MOD_HEAD);
	c = createCommand("ACCESS", myAccess, NULL, -1, -1, -1, -1, -1);
    status = moduleAddCommand(CHANSERV, c, MOD_HEAD);
    c = createCommand("SET", mySet, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, myNickServHelpSet);
	status = moduleAddCommand(NICKSERV, c, MOD_HEAD);
	c = createCommand("SET NOOP", mySet, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, myNickServHelpSetNoop);
	status = moduleAddCommand(NICKSERV, c, MOD_HEAD);
	hook = createEventHook(EVENT_DB_SAVING, mSaveData);
    status = moduleAddEventHook(hook);
    hook = createEventHook(EVENT_RELOAD, mLoadConfig);
    status = moduleAddEventHook(hook);
	if (mLoadConfig(0, NULL)) {
		return MOD_STOP;
	}
	mLoadData();
	if (status == MOD_ERR_OK) {
		alog("ns_setnoop: \2/msg %s HELP SET NOOP\2", s_NickServ);
	} else {
		return MOD_STOP;
	}
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void)
{
    char *av[1];
    av[0] = sstrdup(EVENT_START);
    mSaveData(1, av);
    free(av[0]);
	if (NSNoopDBName) {
        free(NSNoopDBName);
	}
}

int mLoadData(void)
{
    int ret = 0;
    int len = 0;
    char *nick = NULL;
    NickAlias *na = NULL;
    FILE *in;
    char buffer[2000];
    if ((in = fopen(NSNoopDBName, "r")) == NULL) {
        alog("ns_setnoop: WARNING: Can not open database file! (it might not exist, this is not fatal)");
        ret = 1;
    } else {
        while (fgets(buffer, 1500, in)) {
            nick = myStrGetToken(buffer, ' ', 0);
            if (nick) {
                len = strlen(nick);
                nick[len - 1] = '\0';
                if ((na = findnick(nick))) {
                    moduleAddData(&na->nc->moduleData, "noop", "off");
                }
                free(nick);
            }
        }
    }
    return ret;
}

int mSaveData(int argc, char **argv)
{
    NickCore *nc = NULL;
    int i = 0;
    int ret = 0;
    FILE *out;
    if (argc >= 1) {
        if (stricmp(argv[0], EVENT_START) == 0) {
            if ((out = fopen(NSNoopDBName, "w")) == NULL) {
                alog("ns_setnoop: ERROR: can not open the database file!");
                ret = 1;
            } else {
                for (i = 0; i < 1024; i++) {
                    for (nc = nclists[i]; nc; nc = nc->next) {
                        if (moduleGetData(&nc->moduleData, "noop")) {
                            fprintf(out, "%s\n", nc->display);
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

int mLoadConfig(int argc, char **argv)
{
    char *tmp = NULL;
    Directive d[] = {
        {"NSNoopDBName", {{PARAM_STRING, PARAM_RELOAD, &tmp}}},
    };
    moduleGetConfigDirective(d);
    if (NSNoopDBName)
        free(NSNoopDBName);
    if (tmp) {
        NSNoopDBName = tmp;
    } else {
        NSNoopDBName = sstrdup(DEFAULT_DB_NAME);
        alog("ns_setnoop: \2NSNoopDBName\2 is not defined in Services configuration file, using default \2%s\2", NSNoopDBName);
    }
    if (!NSNoopDBName) {
        alog("ns_setnoop: FATAL: Can't read required configuration directives!");
        return MOD_STOP;
    }
    return MOD_CONT;
}
int myNickServHelpSet(User *u)
{
	notice(s_NickServ, u->nick, "    NOOP       Stops users from adding you to channel access lists");
	return MOD_CONT;
}

int myNickServHelpSetNoop(User *u)
{
	notice(s_NickServ, u->nick, "Syntax: \2SET NOOP {ON | OFF | STATUS}\2");
	notice(s_NickServ, u->nick, " ");
	notice(s_NickServ, u->nick, "When this is \2ON\2, %s will not allow your nick to be added to", s_ChanServ);
	notice(s_NickServ, u->nick, "the channel access lists in any channels. Whoever tries to add");
	notice(s_NickServ, u->nick, "you will get a notice from %s back saying that you have NOOP set \2ON\2.", s_ChanServ);
	notice(s_NickServ, u->nick, "The \2STATUS\2 param will return your current NOOP status.");
	return MOD_CONT;
}

int mySet(User *u)
{
    NickAlias *na = NULL;
	char *buf = moduleGetLastBuffer();
    char *cmd = myStrGetToken(buf, ' ', 0);
	char *param = myStrGetToken(buf, ' ', 1);
	if (readonly) {
		notice_lang(s_NickServ, u, NICK_SET_DISABLED);
		if (cmd)
			free(cmd);
		if (param)
			free(param);
		return MOD_STOP;
	} else if (!(na = u->na)) {
		notice_lang(s_NickServ, u, NICK_NOT_REGISTERED);
		if (cmd)
			free(cmd);
		if (param)
			free(param);
		return MOD_STOP;
	} else if (!nick_identified(u)) {
        notice_lang(s_NickServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
		if (cmd)
			free(cmd);
		if (param)
			free(param);
		return MOD_STOP;
    } else if (!cmd || !param) {
		syntax_error(s_NickServ, u, "SET", NICK_SET_SYNTAX);
		return MOD_STOP;
	} else if (stricmp(cmd, "NOOP") == 0) {
		if (stricmp(param, "ON") == 0) {
			if ((na = findnick(u->nick))) {
				moduleAddData(&na->nc->moduleData, "noop", "on");
				notice(s_NickServ, u->nick, "NOOP setting for \2%s\2 is now \2ON\2.", na->nick);
			} else {
				alog("ns_setnoop: WARNING: Can not find NickAlias for %s", u->nick);
			}
		} else if (stricmp(param, "OFF") == 0) {
			if ((na = findnick(u->nick))) {
				moduleDelData(&na->nc->moduleData, "noop");
				notice(s_NickServ, u->nick, "NOOP setting for \2%s\2 is now \2OFF\2.", na->nick);
			} else {
				alog("ns_setnoop: WARNING: Can not find NickAlias for %s", u->nick);
			}
		} else if (stricmp(param, "STATUS") == 0) {
			if ((na = findnick(u->nick))) {
				if (moduleGetData(&na->nc->moduleData, "noop")) {
					notice(s_NickServ, u->nick, "Your current NOOP setting is \2ON\2.");
				} else {
					notice(s_NickServ, u->nick, "Your current NOOP setting is \2OFF\2.");
				}
			}
		} else {
			notice(s_NickServ, u->nick, "Syntax: \2SET NOOP {ON | OFF | STATUS}\2");
			notice(s_NickServ, u->nick, "For more info see: /msg %s HELP SET NOOP\2", s_NickServ);
		}
		if (cmd)
			free(cmd);
		if (param)
			free(param);
		return MOD_STOP;
	}
    return MOD_CONT;
}

int myVOP(User *u)
{
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	char *cmd = myStrGetToken(buf, ' ', 1);
	char *nick = myStrGetToken(buf, ' ', 2);
	NickAlias *na;
	ChannelInfo *ci;
	if (!chan || !cmd) {
		notice_user(s_ChanServ, u, "Syntax: \2VOP \037channel\037 {ADD|DEL|LIST|CLEAR} [\037nick\037 | entry-list]\2");
		notice_user(s_ChanServ, u, "\2/msg %s HELP VOP\2 for more information.", s_ChanServ);
	} else if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    } else if (!(ci->flags & CI_XOP)) {
        notice_lang(s_ChanServ, u, CHAN_XOP_ACCESS, s_ChanServ);
    } else if (stricmp(cmd, "ADD") == 0) {
		if (readonly) {
			notice_lang(s_ChanServ, u, CHAN_VOP_DISABLED);
		} else if (check_access(u, ci, CA_AUTOOP)) {
			na = findnick(nick);
			if (!nick) {
				notice_user(s_ChanServ, u, "Syntax: \2VOP \037channel\037 {ADD|DEL|LIST|CLEAR} [\037nick\037 | entry-list]\2");
				notice_user(s_ChanServ, u, "\2/msg %s HELP VOP\2 for more information.", s_ChanServ);
			} else if (!na) {
				notice_lang(s_ChanServ, u, CHAN_VOP_NICKS_ONLY);
			} else if (na->status & NS_VERBOTEN) {
				notice_lang(s_ChanServ, u, NICK_X_FORBIDDEN, na->nick);
			} else if (moduleGetData(&na->nc->moduleData, "noop")) {
				notice_user(s_ChanServ, u, "\2%s\2 currently have NOOP settings turned ON and cannot be added on the channel VOP list.", na->nick);
				notice_user(s_ChanServ, u, "For more info see: \2/msg %s HELP SET NOOP\2", s_NickServ);
			} else {
				if (chan)
					free(chan);
				if (cmd)
					free(cmd);
				if (nick)
					free(nick);
				return MOD_CONT;
			}
		} else {
			notice_lang(s_ChanServ, u, PERMISSION_DENIED);
		}
	} else {
		if (chan)
			free(chan);
		if (cmd)
			free(cmd);
		if (nick)
			free(nick);
		return MOD_CONT;
	}
	if (chan)
		free(chan);
	if (cmd)
		free(cmd);
	if (nick)
		free(nick);
	return MOD_STOP;
}

int myHOP(User *u)
{
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	char *cmd = myStrGetToken(buf, ' ', 1);
	char *nick = myStrGetToken(buf, ' ', 2);
	NickAlias *na;
	ChannelInfo *ci;
	if (!chan || !cmd) {
		notice_user(s_ChanServ, u, "Syntax: \2HOP \037channel\037 {ADD|DEL|LIST|CLEAR} [\037nick\037 | entry-list]\2");
		notice_user(s_ChanServ, u, "\2/msg %s HELP HOP\2 for more information.", s_ChanServ);
	} else if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    } else if (!(ci->flags & CI_XOP)) {
        notice_lang(s_ChanServ, u, CHAN_XOP_ACCESS, s_ChanServ);
    } else if (stricmp(cmd, "ADD") == 0) {
		if (readonly) {
			notice_lang(s_ChanServ, u, CHAN_HOP_DISABLED);
		} else if (check_access(u, ci, CA_AUTOOP)) {
			na = findnick(nick);
			if (!nick) {
				notice_user(s_ChanServ, u, "Syntax: \2HOP \037channel\037 {ADD|DEL|LIST|CLEAR} [\037nick\037 | entry-list]\2");
				notice_user(s_ChanServ, u, "\2/msg %s HELP HOP\2 for more information.", s_ChanServ);
			} else if (!na) {
				notice_lang(s_ChanServ, u, CHAN_HOP_NICKS_ONLY);
			} else if (na->status & NS_VERBOTEN) {
				notice_lang(s_ChanServ, u, NICK_X_FORBIDDEN, na->nick);
			} else if (moduleGetData(&na->nc->moduleData, "noop")) {
				notice_user(s_ChanServ, u, "\2%s\2 currently have NOOP settings turned ON and cannot be added on the channel HOP list.", na->nick);
				notice_user(s_ChanServ, u, "For more info see: \2/msg %s HELP SET NOOP\2", s_NickServ);
			} else {
				if (chan)
					free(chan);
				if (cmd)
					free(cmd);
				if (nick)
					free(nick);
				return MOD_CONT;
			}
		} else {
			notice_lang(s_ChanServ, u, PERMISSION_DENIED);
		}
	} else {
		if (chan)
			free(chan);
		if (cmd)
			free(cmd);
		if (nick)
			free(nick);
		return MOD_CONT;
	}
	if (chan)
		free(chan);
	if (cmd)
		free(cmd);
	if (nick)
		free(nick);
	return MOD_STOP;
}

int myAOP(User *u)
{
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	char *cmd = myStrGetToken(buf, ' ', 1);
	char *nick = myStrGetToken(buf, ' ', 2);
	NickAlias *na;
	ChannelInfo *ci;
	if (!chan || !cmd) {
		notice_user(s_ChanServ, u, "Syntax: \2AOP \037channel\037 {ADD|DEL|LIST|CLEAR} [\037nick\037 | entry-list]\2");
		notice_user(s_ChanServ, u, "\2/msg %s HELP AOP\2 for more information.", s_ChanServ);
	} else if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    } else if (!(ci->flags & CI_XOP)) {
        notice_lang(s_ChanServ, u, CHAN_XOP_ACCESS, s_ChanServ);
    } else if (stricmp(cmd, "ADD") == 0) {
		if (readonly) {
			notice_lang(s_ChanServ, u, CHAN_AOP_DISABLED);
		} else if (check_access(u, ci, CA_AUTOPROTECT)) {
			na = findnick(nick);
			if (!nick) {
				notice_user(s_ChanServ, u, "Syntax: \2AOP \037channel\037 {ADD|DEL|LIST|CLEAR} [\037nick\037 | entry-list]\2");
				notice_user(s_ChanServ, u, "\2/msg %s HELP AOP\2 for more information.", s_ChanServ);
			} else if (!na) {
				notice_lang(s_ChanServ, u, CHAN_AOP_NICKS_ONLY);
			} else if (na->status & NS_VERBOTEN) {
				notice_lang(s_ChanServ, u, NICK_X_FORBIDDEN, na->nick);
			} else if (moduleGetData(&na->nc->moduleData, "noop")) {
				notice_user(s_ChanServ, u, "\2%s\2 currently have NOOP settings turned ON and cannot be added on the channel AOP list.", na->nick);
				notice_user(s_ChanServ, u, "For more info see: \2/msg %s HELP SET NOOP\2", s_NickServ);
			} else {
				if (chan)
					free(chan);
				if (cmd)
					free(cmd);
				if (nick)
					free(nick);
				return MOD_CONT;
			}
		} else {
			notice_lang(s_ChanServ, u, PERMISSION_DENIED);
		}
	} else {
		if (chan)
			free(chan);
		if (cmd)
			free(cmd);
		if (nick)
			free(nick);
		return MOD_CONT;
	}
	if (chan)
		free(chan);
	if (cmd)
		free(cmd);
	if (nick)
		free(nick);
	return MOD_STOP;
}

int mySOP(User *u)
{
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	char *cmd = myStrGetToken(buf, ' ', 1);
	char *nick = myStrGetToken(buf, ' ', 2);
	NickAlias *na;
	ChannelInfo *ci;
	if (!chan || !cmd) {
		notice_user(s_ChanServ, u, "Syntax: \2SOP \037channel\037 {ADD|DEL|LIST|CLEAR} [\037nick\037 | entry-list]\2");
		notice_user(s_ChanServ, u, "\2/msg %s HELP SOP\2 for more information.", s_ChanServ);
	} else if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    } else if (!(ci->flags & CI_XOP)) {
        notice_lang(s_ChanServ, u, CHAN_XOP_ACCESS, s_ChanServ);
    } else if (stricmp(cmd, "ADD") == 0) {
		if (readonly) {
			notice_lang(s_ChanServ, u, CHAN_SOP_DISABLED);
		} else if (is_founder(u, ci)) {
			na = findnick(nick);
			if (!nick) {
				notice_user(s_ChanServ, u, "Syntax: \2SOP \037channel\037 {ADD|DEL|LIST|CLEAR} [\037nick\037 | entry-list]\2");
				notice_user(s_ChanServ, u, "\2/msg %s HELP SOP\2 for more information.", s_ChanServ);
			} else if (!na) {
				notice_lang(s_ChanServ, u, CHAN_SOP_NICKS_ONLY);
			} else if (na->status & NS_VERBOTEN) {
				notice_lang(s_ChanServ, u, NICK_X_FORBIDDEN, na->nick);
			} else if (moduleGetData(&na->nc->moduleData, "noop")) {
				notice_user(s_ChanServ, u, "\2%s\2 currently have NOOP settings turned ON and cannot be added on the channel SOP list.", na->nick);
				notice_user(s_ChanServ, u, "For more info see: \2/msg %s HELP SET NOOP\2", s_NickServ);
			} else {
				if (chan)
					free(chan);
				if (cmd)
					free(cmd);
				if (nick)
					free(nick);
				return MOD_CONT;
			}
		} else {
			notice_lang(s_ChanServ, u, PERMISSION_DENIED);
		}
	} else {
		if (chan)
			free(chan);
		if (cmd)
			free(cmd);
		if (nick)
			free(nick);
		return MOD_CONT;
	}
	if (chan)
		free(chan);
	if (cmd)
		free(cmd);
	if (nick)
		free(nick);
	return MOD_STOP;
}

int myAccess(User *u)
{
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	char *cmd = myStrGetToken(buf, ' ', 1);
	char *nick = myStrGetToken(buf, ' ', 2);
	NickAlias *na;
	ChannelInfo *ci;
	if (!chan || !cmd) {
		notice_user(s_ChanServ, u, "Syntax: \2ACCESS \037channel\037 {ADD|DEL|LIST|CLEAR} [\037nick\037 [\037level\037] | \037entry-list\037]\2");
		notice_user(s_ChanServ, u, "\2/msg %s HELP ACCESS\2 for more information.", s_ChanServ);
	} else if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    } else if (ci->flags & CI_XOP) {
		if (ircd->halfop) {
	        notice_lang(s_ChanServ, u, CHAN_ACCESS_XOP_HOP, s_ChanServ);
		} else {
			notice_lang(s_ChanServ, u, CHAN_ACCESS_XOP, s_ChanServ);
		}
    } else if (!check_access(u, ci, CA_ACCESS_CHANGE) && !is_services_admin(u)) {
        notice_lang(s_ChanServ, u, ACCESS_DENIED);
    } else if (stricmp(cmd, "ADD") == 0) {
		if (readonly) {
			notice_lang(s_ChanServ, u, CHAN_ACCESS_DISABLED);
		} else if (check_access(u, ci, CA_ACCESS_CHANGE)) {
			na = findnick(nick);
			if (!nick) {
				notice_user(s_ChanServ, u, "Syntax: \2ACCESS \037channel\037 {ADD|DEL|LIST|CLEAR} [\037nick\037 [\037level\037] | \037entry-list\037]\2");
				notice_user(s_ChanServ, u, "\2/msg %s HELP ACCESS\2 for more information.", s_ChanServ);
			} else if (!na) {
				notice_lang(s_ChanServ, u, CHAN_ACCESS_NICKS_ONLY);
			} else if (na->status & NS_VERBOTEN) {
				notice_lang(s_ChanServ, u, NICK_X_FORBIDDEN, na->nick);
			} else if (moduleGetData(&na->nc->moduleData, "noop")) {
				notice_user(s_ChanServ, u, "\2%s\2 currently have NOOP settings turned ON and cannot be added on the channel access list.", na->nick);
				notice_user(s_ChanServ, u, "For more info see: \2/msg %s HELP SET NOOP\2", s_NickServ);
			} else {
				if (chan)
					free(chan);
				if (cmd)
					free(cmd);
				if (nick)
					free(nick);
				return MOD_CONT;
			}
		} else {
			notice_lang(s_ChanServ, u, PERMISSION_DENIED);
		}
	} else {
		if (chan)
			free(chan);
		if (cmd)
			free(cmd);
		if (nick)
			free(nick);
		return MOD_CONT;
	}
	if (chan)
		free(chan);
	if (cmd)
		free(cmd);
	if (nick)
		free(nick);
	return MOD_STOP;
}
/* EOF */
