#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: ns_regcode.c v1.0.0 15-08-2007 n00bie $"

/****************************************************************************************
** Module	: ns_regcode.c
** Version	: 1.0.0
** Author	: n00bie [n00bie@rediffmail.com]
** Release	: 15th August, 2007
**
** Description: This module splits the nick registration process into 2 steps. The first
** after giving the registration command will give out a passcode. The passcode must be
** supplied using the PROCEED or CONFIRM command in order to complete the registration.
**
** Example:
** -NickServ- To complete your registration you need to enter a passcode.
** -NickServ- Type: /msg NickServ PROCEED passcode
** -NickServ- Your passcode is: JrYH8rCpz
**
** NOTE: Do NOT use this module if you have 'NSEmailReg' enabled on your network.
** 
** Providing command:
** /msg NickServ PROCEED passcode
** /msg NickServ CONFIRM passcode
**
** Tested: Anope-1.7.18, 1.7.19
*****************************************************************************************
** This program is free software; you can redistribute it and/or modify it under the 
** terms of the GNU General Public License as published by the Free Software 
** Foundation; either version 1, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful, but WITHOUT ANY 
** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** for a PARTICULAR PURPOSE. See the GNU General Public License for more details.
*****************************************************************************************
** This module have no configurable option.
****************************************************************************************/

int m_do_register(User *u);
int do_proceed(User *u);
int do_confirm(User *u);
int do_check(int argc, char **argv);
NickRequest *makerequest(const char *nick);
NickAlias *makenick(const char *nick);
int AnopeInit(int argc, char **argv)
{
	EvtHook *hook = NULL;
	Command *c;
	int status = 0;
	if (NSEmailReg) {
		alog("%s: ns_regcode: Sorry, this module is not designed to work on services which have \2NSEmailReg\2 enabled.", s_NickServ);
		return MOD_STOP;
	}
	if (!moduleMinVersion(1,7,18,1214)) {
		alog("%s: ns_regcode: Sorry, your version of Anope is not supported. Please upgrade to a new version.", s_NickServ);
		return MOD_STOP;
	}
	c = createCommand("REGISTER", m_do_register, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		alog("%s: ns_regcode: Something isn't init right.", s_NickServ);
		return MOD_STOP;
	}
	c = createCommand("PROCEED", do_proceed, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		alog("%s: ns_regcode: Something isn't init right.", s_NickServ);
		return MOD_STOP;
	}
	c = createCommand("CONFIRM", do_confirm, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		alog("%s: ns_regcode: Something isn't init right.", s_NickServ);
		return MOD_STOP;
	}
	hook = createEventHook(EVENT_USER_LOGOFF, do_check);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog("%s: ns_regcode: Cannot hook to event EVENT_USER_LOGOFF", s_NickServ);
		return MOD_STOP;
	} else {
		alog("%s: ns_regcode: Successfully hook to EVENT_USER_LOGOFF", s_NickServ);
		alog("%s: ns_regcode: Module loaded and active.", s_NickServ);
	}
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("%s: ns_regcode: Module Unloaded.", s_NickServ);
}


int m_do_register(User *u)
{
	NickRequest *nr = NULL, *anr = NULL;
	NickCore *nc = NULL;
	int prefixlen = strlen(NSGuestNickPrefix);
    int nicklen = strlen(u->nick);
	char *buf = moduleGetLastBuffer();
	char *pass = myStrGetToken(buf, ' ', 0);
	char *email = myStrGetToken(buf, ' ', 1);
	char passcode[11];
    int idx, min = 1, max = 62, i = 0;
    int chars[] =
        { ' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
        'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
        'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
        'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
        'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
    };
	if (readonly) {
        notice_lang(s_NickServ, u, NICK_REGISTRATION_DISABLED);
		if (pass)
			free(pass);
		if (email)
			free(email);
        return MOD_STOP;
    }
    if (checkDefCon(DEFCON_NO_NEW_NICKS)) {
        notice_lang(s_NickServ, u, OPER_DEFCON_DENIED);
		if (pass)
			free(pass);
		if (email)
			free(email);
        return MOD_STOP;
    }
    if (!is_oper(u) && NickRegDelay && ((time(NULL) - u->my_signon) < NickRegDelay)) {
        notice_lang(s_NickServ, u, NICK_REG_DELAY, NickRegDelay);
		if (pass)
			free(pass);
		if (email)
			free(email);
        return MOD_STOP;
    }
	if ((anr = findrequestnick(u->nick))) {
        notice(s_NickServ, u->nick, "This nick has already been requested.");
		if (pass)
			free(pass);
		if (email)
			free(email);
        return MOD_STOP;
    }
	if (nicklen <= prefixlen + 7 && nicklen >= prefixlen + 1 &&
		stristr(u->nick, NSGuestNickPrefix) == u->nick &&
		strspn(u->nick + prefixlen, "1234567890") == nicklen - prefixlen) {
			notice_lang(s_NickServ, u, NICK_CANNOT_BE_REGISTERED, u->nick);
			if (pass)
				free(pass);
			if (email)
				free(email);
			return MOD_STOP;
	}
	if (!anope_valid_nick(u->nick)) {
        notice_lang(s_NickServ, u, NICK_X_FORBIDDEN, u->nick);
		if (pass)
			free(pass);
		if (email)
			free(email);
        return MOD_STOP;
    }
	if (RestrictOperNicks) {
        for (i = 0; i < RootNumber; i++) {
            if (stristr(u->nick, ServicesRoots[i]) && !is_oper(u)) {
                notice_lang(s_NickServ, u, NICK_CANNOT_BE_REGISTERED, u->nick);
				if (pass)
					free(pass);
				if (email)
					free(email);
                return MOD_STOP;
            }
        }
        for (i = 0; i < servadmins.count && (nc = servadmins.list[i]); i++) {
            if (stristr(u->nick, nc->display) && !is_oper(u)) {
                notice_lang(s_NickServ, u, NICK_CANNOT_BE_REGISTERED, u->nick);
                if (pass)
					free(pass);
				if (email)
					free(email);
                return MOD_STOP;
            }
        }
        for (i = 0; i < servopers.count && (nc = servopers.list[i]); i++) {
            if (stristr(u->nick, nc->display) && !is_oper(u)) {
                notice_lang(s_NickServ, u, NICK_CANNOT_BE_REGISTERED, u->nick);
                if (pass)
					free(pass);
				if (email)
					free(email);
                return MOD_STOP;
            }
        }
    }
	if (!pass) {
        if (NSForceEmail) {
            syntax_error(s_NickServ, u, "REGISTER", NICK_REGISTER_SYNTAX_EMAIL);
			return MOD_STOP;
        } else {
            syntax_error(s_NickServ, u, "REGISTER", NICK_REGISTER_SYNTAX);
			return MOD_STOP;
        }
    } else if (NSForceEmail && !email) {
        syntax_error(s_NickServ, u, "REGISTER", NICK_REGISTER_SYNTAX_EMAIL);
		if (pass)
			free(pass);
		return MOD_STOP;
	} else if (time(NULL) < u->lastnickreg + NSRegDelay) {
        notice_lang(s_NickServ, u, NICK_REG_PLEASE_WAIT, NSRegDelay);
		if (pass)
			free(pass);
		if (email)
			free(email);
		return MOD_STOP;
    } else if (u->na) {         /* i.e. there's already such a nick regged */
        if (u->na->status & NS_VERBOTEN) {
            alog("%s: %s@%s tried to register FORBIDden nick %s", s_NickServ, u->username, u->host, u->nick);
            notice_lang(s_NickServ, u, NICK_CANNOT_BE_REGISTERED, u->nick);
			if (pass)
				free(pass);
			if (email)
				free(email);
			return MOD_STOP;
        } else {
            notice_lang(s_NickServ, u, NICK_ALREADY_REGISTERED, u->nick);
			if (pass)
				free(pass);
			if (email)
				free(email);
			return MOD_STOP;
        }
    } else if (stricmp(u->nick, pass) == 0 || (StrictPasswords && strlen(pass) < 5)) {
        notice_lang(s_NickServ, u, MORE_OBSCURE_PASSWORD);
		if (pass)
			free(pass);
		if (email)
			free(email);
		return MOD_STOP;
    } else if (email && !MailValidate(email)) {
        notice_lang(s_NickServ, u, MAIL_X_INVALID, email);
		if (pass)
			free(pass);
		if (email)
			free(email);
		return MOD_STOP;
    } else {
		if (strlen(pass) > PASSMAX) {
			pass[PASSMAX] = 0;
			notice_lang(s_NickServ, u, PASSWORD_TRUNCATED, PASSMAX);
		}
		for (idx = 0; idx < 9; idx++) {
			passcode[idx] = chars[(1 + (int) (((float) (max - min)) * getrandom16() / (65535 + 1.0)) + min)];
		}
		passcode[idx] = '\0';
		nr = makerequest(u->nick);
        nr->passcode = sstrdup(passcode);
        nr->password = sstrdup(pass);
        if (email) {
            nr->email = sstrdup(email);
        }
        nr->requested = time(NULL);
		notice(s_NickServ, u->nick, "To complete your registration you need to enter a passcode.");
		notice(s_NickServ, u->nick, "Type: \2/msg %s PROCEED \037passcode\037\2", s_NickServ);
		notice(s_NickServ, u->nick, "Your passcode is: %s", passcode);
	}
	if (pass)
		free(pass);
	if (email)
		free(email);
	if (passcode)
		free(passcode);
	return MOD_STOP;
}

NickRequest *makerequest(const char *nick)
{
    NickRequest *nr;
    nr = scalloc(1, sizeof(NickRequest));
    nr->nick = sstrdup(nick);
    insert_requestnick(nr);
    alog("%s: Nick \2%s\2 has been requested", s_NickServ, nr->nick);
    return nr;
}

int do_proceed(User *u)
{
	NickRequest *nr = NULL;
	char *buf = moduleGetLastBuffer();
	char *passcode = myStrGetToken(buf, ' ', 0);
	nr = findrequestnick(u->nick);
	if (!passcode) {
		notice(s_NickServ, u->nick, "You need to enter a passcode.");
		return MOD_STOP;
	}
	if (nr) {
		if (stricmp(nr->passcode, passcode) != 0) {
			notice(s_NickServ, u->nick, "Sorry, you have entered an invalid passcode and your registration");
			notice(s_NickServ, u->nick, "session have expired. You need to give the REGISTER command again.");
			alog("%s: registration for \2%s\2 failed. Deleting \2%s\2 from the database.", s_NickServ, nr->nick, nr->nick);
			delnickrequest(nr);
			if (passcode)
				free(passcode);
			return MOD_STOP;
		} else {
			do_confirm(u);
		}
	} else {
		notice_lang(s_NickServ, u, NICK_CONFIRM_NOT_FOUND, s_NickServ);
	}
	if (passcode)
		free(passcode);
	return MOD_STOP;
}

int do_confirm(User * u)
{
    NickRequest *nr = NULL;
    NickAlias *na = NULL;
    char *passcode = moduleGetLastBuffer();
    char *pass = NULL;
    char *email = NULL;
    int forced = 0;
    User *utmp = NULL;
    char modes[512];
    int len;
    nr = findrequestnick(u->nick);
	if (!NSEmailReg) {
        if (!passcode) {
            notice(s_NickServ, u->nick, "You need to enter a passcode.");
            return MOD_STOP;
        }
		if (!nr) {
			if (is_services_admin(u)) {
/* If an admin, thier nick is obviously already regged, so look at the passcode to get the nick
   of the user they are trying to validate, and push that user through regardless of passcode */
                nr = findrequestnick(passcode);
                if (nr) {
                    utmp = finduser(passcode);
                    if (utmp) {
                        //sprintf(passcode, "FORCE_ACTIVATION_DUE_TO_OPER_CONFIRM %s", nr->passcode);
                        //passcode = strtok(passcode, " ");
                        notice_lang(s_NickServ, u, NICK_FORCE_REG, nr->nick);
                        do_confirm(utmp);
						if (passcode) free(passcode);
						return MOD_STOP;
                    } else {
                        passcode = sstrdup(nr->passcode);
                        forced = 1;
                    }
                } else {
					notice_lang(s_NickServ, u, NICK_CONFIRM_NOT_FOUND, s_NickServ);
					if (passcode)
						free(passcode);
					return MOD_STOP;
                }
			} else {
                notice_lang(s_NickServ, u, NICK_CONFIRM_NOT_FOUND, s_NickServ);
				if (passcode)
					free(passcode);
				return MOD_STOP;
            }
        }
		if (stricmp(passcode, nr->passcode) != 0) {
            notice(s_NickServ, u->nick, "Invalid passcode has been entered.");
			if (passcode)
				free(passcode);
            return MOD_STOP;
        }
	}
    if (!nr) {
        notice_lang(s_NickServ, u, NICK_REGISTRATION_FAILED);
        return MOD_STOP;
    }
    pass = sstrdup(nr->password);
    if (nr->email) {
        email = sstrdup(nr->email);
    }
    na = makenick(nr->nick);
    if (na) {
		int i;
        char tsbuf[16];
		char tmp_pass[PASSMAX];
        len = strlen(pass);
        na->nc->pass = smalloc(PASSMAX);
        if (enc_encrypt(pass, len, na->nc->pass, PASSMAX) < 0) {
            memset(pass, 0, strlen(pass));
            alog("%s: Failed to encrypt password for %s (register)", s_NickServ, nr->nick);
            notice_lang(s_NickServ, u, NICK_REGISTRATION_FAILED);
            return MOD_CONT;
        }
        memset(pass, 0, strlen(pass));
        na->status = (int16) (NS_IDENTIFIED | NS_RECOGNIZED);
		/* na->nc->flags |= NI_ENCRYPTEDPW; */
        na->nc->flags |= NSDefFlags;
        for (i = 0; i < RootNumber; i++) {
            if (!stricmp(ServicesRoots[i], nr->nick)) {
                na->nc->flags |= NI_SERVICES_ROOT;
                break;
            }
        }
        na->nc->memos.memomax = MSMaxMemos;
        na->nc->channelmax = CSMaxReg;
        if (forced == 1) {
            na->last_usermask = sstrdup("*@*");
            na->last_realname = sstrdup("unknown");
        } else {
            na->last_usermask =
                scalloc(strlen(common_get_vident(u)) +
                        strlen(common_get_vhost(u)) + 2, 1);
            sprintf(na->last_usermask, "%s@%s", common_get_vident(u),
                    common_get_vhost(u));
            na->last_realname = sstrdup(u->realname);
        }
        na->time_registered = na->last_seen = time(NULL);
        if (NSAddAccessOnReg) {
            na->nc->accesscount = 1;
            na->nc->access = scalloc(sizeof(char *), 1);
            na->nc->access[0] = create_mask(u);
        } else {
            na->nc->accesscount = 0;
            na->nc->access = NULL;
        }
        na->nc->language = NSDefLanguage;
        if (email)
            na->nc->email = sstrdup(email);
        if (forced != 1) {
            u->na = na;
            na->u = u;
            alog("%s: '%s' registered by %s@%s (e-mail: %s)", s_NickServ, u->nick, u->username, u->host, (email ? email : "none"));
            if (NSAddAccessOnReg)
                notice_lang(s_NickServ, u, NICK_REGISTERED, u->nick, na->nc->access[0]);
            else
                notice_lang(s_NickServ, u, NICK_REGISTERED_NO_MASK, u->nick);
            send_event(EVENT_NICK_REGISTERED, 1, u->nick);	    
	    if(enc_decrypt(na->nc->pass,tmp_pass,PASSMAX)==1) 
                notice_lang(s_NickServ, u, NICK_PASSWORD_IS, tmp_pass);
            u->lastnickreg = time(NULL);
            if (ircd->modeonreg) {
                len = strlen(ircd->modeonreg);
                strncpy(modes,ircd->modeonreg,512);
	        if(ircd->rootmodeonid && is_services_root(u)) { 
                    strncat(modes,ircd->rootmodeonid,512-len);
	        } else if(ircd->adminmodeonid && is_services_admin(u)) {
                    strncat(modes,ircd->adminmodeonid,512-len);
	        } else if(ircd->opermodeonid && is_services_oper(u)) {
                    strncat(modes,ircd->opermodeonid,512-len);
                }
                if (ircd->tsonmode) {
                    snprintf(tsbuf, sizeof(tsbuf), "%lu",
                             (unsigned long int) u->timestamp);
                    common_svsmode(u, modes, tsbuf);
                } else {
                    common_svsmode(u, modes, NULL);
                }
            }
		} else {
            notice_lang(s_NickServ, u, NICK_FORCE_REG, nr->nick);
		}
        delnickrequest(nr);     /* remove the nick request */
	} else {
        alog("%s: makenick(%s) failed", s_NickServ, u->nick);
        notice_lang(s_NickServ, u, NICK_REGISTRATION_FAILED);
    }

	/* Enable nick tracking if enabled */
    if (NSNickTracking)
        nsStartNickTracking(u);

	if (passcode)
		free(passcode);
    return MOD_STOP;
}

NickAlias *makenick(const char *nick)
{
    NickAlias *na;
    NickCore *nc;
    /* First make the core */
    nc = scalloc(1, sizeof(NickCore));
    nc->display = sstrdup(nick);
    slist_init(&nc->aliases);
    insert_core(nc);
    alog("%s: group \2%s\2 has been created", s_NickServ, nc->display);
    /* Then make the alias */
    na = scalloc(1, sizeof(NickAlias));
    na->nick = sstrdup(nick);
    na->nc = nc;
    slist_add(&nc->aliases, na);
    alpha_insert_alias(na);
    return na;
}

/* A user gets disconnected from the server
** before entering a passcode. Lets make sure
** that the pre-regged nick is deleted from
** the database so that he can register again
** when he logged back in ;-)
*/
int do_check(int argc, char **argv)
{
	NickRequest *nr = NULL;
	if (argc >= 1) {
		if ((nr = findrequestnick(argv[0]))) {
			alog("%s: nick request for \2%s\2 expired. Deleting \2%s\2 from the database.", s_NickServ, nr->nick, nr->nick);
			delnickrequest(nr);
		}
	}
	return MOD_STOP;
}
/* EOF */
