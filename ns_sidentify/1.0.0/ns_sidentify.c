#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: ns_sidentify.c v1.0.0 25-08-2007 n00bie $"

/*
**************************************************************************************
** Module	: ns_sidentify.c
** Author	: n00bie (n00bie@rediffmail.com)
** Version	: 1.0.0
** Release	: 25th August, 2007
**************************************************************************************
** Description:
**
** Kill another user who has taken your nick and regain custody of your nick.
** This module allows you to recover your nickname if someone else has taken it.
** When you give this command, services will disconnect the other user and will
** automatically change your nick to the 'sidentified' nick. After giving the 
** command, identifying to NickServ is not necessary.
**************************************************************************************
** Providing Command:
**
** /msg NickServ HELP SIDENTIFY
** /msg NickServ SIDENTIFY nickname [password]
** /msg NickServ SID nickname [password]
**
** Tested: 1.7.19, Unreal3.2.7
**************************************************************************************
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
**************************************************************************************
** Module suggested by Steven_elvisda
** This module have no configurable option.
*/

#define SIDENTIFY_SYNTAX "Syntax: \2SIDENTIFY \037nickname\037 [\037password\037]\2"
#define NO_SIDENTIFY_SELF "You can't sidentify yourself!"

int my_id(int argc, char **argv);
int do_sidentify(User *u);
int myNickServSIDHelp(User *u); 
void myNickServHelp(User *u);
int AnopeInit(int argc, char **argv)
{
	Command *c;
	int status = 0;
	c = createCommand("SID", do_sidentify, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, myNickServSIDHelp);
	moduleSetNickHelp(myNickServHelp);
    status = moduleAddCommand(NICKSERV, c, MOD_HEAD);
	c = createCommand("SIDENTIFY", do_sidentify, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, myNickServSIDHelp);
	moduleSetNickHelp(myNickServHelp);
	status = moduleAddCommand(NICKSERV, c, MOD_HEAD);
	if (status == MOD_ERR_OK) {
		alog("%s: ns_sidentify: New command: \2/msg %s HELP SIDENTIFY\2", s_NickServ, s_NickServ);
		alog("%s: ns_sidentify: Successfully loaded module.", s_NickServ);
	} else {
		return MOD_STOP;
	}
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("%s: ns_sidentify: Module unloaded.", s_NickServ);
}

void myNickServHelp(User *u)
{
	anope_cmd_notice(s_NickServ, u->nick, "    SIDENTIFY  Kill another user who has taken your nick and regain");
	anope_cmd_notice(s_NickServ, u->nick, "               custody of your nick");
}

int myNickServSIDHelp(User *u)
{
	anope_cmd_notice(s_NickServ, u->nick, SIDENTIFY_SYNTAX);
	anope_cmd_notice(s_NickServ, u->nick, " ");
	anope_cmd_notice(s_NickServ, u->nick, "Allows you to recover your nickname if someone else has taken it.");
	anope_cmd_notice(s_NickServ, u->nick, "When you give this command, services will disconnect the other user");
	anope_cmd_notice(s_NickServ, u->nick, "and will automatically change your nick to the \2sidentified\2 nick.");
	anope_cmd_notice(s_NickServ, u->nick, "After giving the command, identifying to %s is not necessary.", s_NickServ);
	return MOD_CONT;
}

int do_sidentify(User *u)
{
	char *buf = moduleGetLastBuffer();
	char *nick = myStrGetToken(buf, ' ', 0);
	char *pass = myStrGetToken(buf, ' ', 1);
	NickAlias *na;
	User *u2;
	char *argv[1];
    int res;

	if (!nick) {
		anope_cmd_notice(s_NickServ, u->nick, SIDENTIFY_SYNTAX);
	} else if (nickIsServices(nick, 1)) {
		notice_lang(s_NickServ, u, NICK_X_IS_SERVICES, nick);
	} else if (!(u2 = finduser(nick))) {
        notice_lang(s_NickServ, u, NICK_X_NOT_IN_USE, nick);
    } else if (!(na = u2->na)) {
        notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, nick);
    } else if (na->status & NS_VERBOTEN) {
        notice_lang(s_NickServ, u, NICK_X_FORBIDDEN, na->nick);
    } else if (na->nc->flags & NI_SUSPENDED) {
        notice_lang(s_NickServ, u, NICK_X_SUSPENDED, na->nick);
    } else if (stricmp(nick, u->nick) == 0) {
        anope_cmd_notice(s_NickServ, u->nick, NO_SIDENTIFY_SELF);
	} else if (pass) {
		if (!(res = enc_check_password(pass, na->nc->pass))) {
			anope_cmd_notice(s_NickServ, u->nick, "Access denied. Wrong password.");
                alog("%s: SIDENTIFY: invalid password for %s by %s!%s@%s", 
					s_NickServ, nick, u->nick, u->username, u->host);
                bad_password(u);
		} else {
			char buf[NICKMAX + 32];
			snprintf(buf, sizeof(buf), "SIDENTIFY command used by %s", u->nick);
            if (LimitSessions) {
                del_session(u2->host);
            }
            kill_user(s_NickServ, nick, buf);
			anope_cmd_svsnick(u->nick, nick, time(NULL));
			argv[0] = nick;
			moduleAddCallback("ns_sidentify: my_id", (time(NULL) + 2), my_id, 1, argv);
		}
	} else {
		anope_cmd_notice(s_NickServ, u->nick, SIDENTIFY_SYNTAX);
	}
	if (nick)
		free(nick);
	if (pass)
		free(pass);
	return MOD_STOP;
}

int my_id(int argc, char **argv)
{
	User *u = finduser(argv[0]);
	NickAlias *na = u->na;
	char tsbuf[16];
    char modes[512];
    int len;
	na->status |= NS_IDENTIFIED;
	na->last_seen = time(NULL);
	snprintf(tsbuf, sizeof(tsbuf), "%lu",
		(unsigned long int) u->timestamp);
	if (ircd->modeonreg) {
		len = strlen(ircd->modeonreg);
		strncpy(modes, ircd->modeonreg, 512);
		if (ircd->rootmodeonid && is_services_root(u)) {
			strncat(modes, ircd->rootmodeonid, 512-len);
		} else if (ircd->adminmodeonid && is_services_admin(u)) {
			strncat(modes, ircd->adminmodeonid, 512-len);
		} else if (ircd->opermodeonid && is_services_oper(u)) {
			strncat(modes, ircd->opermodeonid, 512-len);
		}
		if (ircd->tsonmode) {
			common_svsmode(u, modes, tsbuf);
		} else {
			common_svsmode(u, modes, "");
		}
	}
	if (ircd->vhost) {
		do_on_id(u);
	}
	if (NSModeOnID) {
		do_setmodes(u);
	}
	if (!(na->status & NS_RECOGNIZED)) {
		check_memos(u);
	}
	if (NSNickTracking) {
		nsStartNickTracking(u);
	}
	alog("%s: %s!%s@%s automatically identified for nick %s", s_NickServ, u->nick, u->username, u->host, u->nick);
	return MOD_STOP;
}
/* EOF */
