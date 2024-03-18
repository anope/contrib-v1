/**
 * Main routines for performing nickserv operations.
 *
 ***********
 * Module Name    : misc_sqlcmd
 * Author         : Viper <Viper@Anope.org>
 *
 * More info on http://modules.anope.org and http://forum.anope.org
 ***********
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***********
 *
 * Last Updated   : 13/01/2011
 *
 **/

#include "misc_sqlcmd.h"

/**
 * Registers a nickname.
 *
 * @param ac int Argument count.
 * @param av Array Argument list.
 *    av[0] - Nickname
 *    av[1] - Password
 *    av[2] - E-Mail Address [Optional]
 **/
int sqlcmd_handle_nickreg(int ac, char **av) {
	User *u = NULL;
	NickCore *nc = NULL;
	NickAlias *na = NULL;
	NickRequest *nr = NULL, *anr = NULL;
	char *nick, *pass, *email = NULL, passcode[11];
	int prefixlen, nicklen, idx, min = 1, max = 62, i = 0;
	int chars[] = {
		' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
		'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
		'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
		'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
		'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
	};

	if (ac < 2)
		return SQLCMD_ERROR_SYNTAX_ERROR;

	nick = av[0];
	pass = av[1];
	if (ac == 3)
		email = av[2];
	nicklen = strlen(nick);
	prefixlen = strlen(NSGuestNickPrefix);

	if (readonly)
		return SQLCMD_ERROR_READ_ONLY;
	else if (checkDefCon(DEFCON_NO_NEW_NICKS))
		return SQLCMD_ERROR_DEFCON;
	else if (NSForceEmail && !email)
		return SQLCMD_ERROR_SYNTAX_ERROR;  
	else if ((na = findnick(nick))) {
		if (na->status & NS_VERBOTEN) {
			alog("[misc_sqlcmd] Attempt to register FORBIDden nick %s.", nick);
			return SQLCMD_ERROR_NICK_FORBIDDEN;
		} else
			return SQLCMD_ERROR_NICK_ALREADY_REGISTERED;
	} else if ((anr = findrequestnick(nick)))
		return SQLCMD_ERROR_NICK_ALREADY_REQUESTED;
	else if ((nicklen <= prefixlen + 7 && nicklen >= prefixlen + 1 && stristr(nick, NSGuestNickPrefix) == nick && 
			strspn(nick + prefixlen, "1234567890") == nicklen - prefixlen) || !anope_valid_nick(nick))
		return SQLCMD_ERROR_NICK_CANNOT_BE_REGISTERED;

	if (RestrictOperNicks) {
		for (i = 0; i < RootNumber; i++)
			if (stristr(nick, ServicesRoots[i]))
				return SQLCMD_ERROR_NICK_CANNOT_BE_REGISTERED;

		for (i = 0; i < servadmins.count && (nc = servadmins.list[i]); i++)
			if (stristr(nick, nc->display))
				return SQLCMD_ERROR_NICK_CANNOT_BE_REGISTERED;

		for (i = 0; i < servopers.count && (nc = servopers.list[i]); i++)
			if (stristr(nick, nc->display))
				return SQLCMD_ERROR_NICK_CANNOT_BE_REGISTERED;
	}

	if ((u = finduser(nick)))
		return SQLCMD_ERROR_NICK_IN_USE;
	else if (!stricmp(nick, pass) || (StrictPasswords && strlen(pass) < 5))
		return SQLCMD_ERROR_MORE_OBSCURE_PASS;
	else if (enc_encrypt_check_len(strlen(pass), PASSMAX - 1))
		return SQLCMD_ERROR_PASS_TOO_LONG;
	else if (email && !MailValidate(email))
		return SQLCMD_ERROR_EMAIL_INVALID;
	else {
		for (idx = 0; idx < 9; idx++) {
			passcode[idx] = chars[(1 + (int) (((float) (max - min)) * getrandom16() / (65535 + 1.0)) + min)];
		} passcode[idx] = '\0';
		nr = makerequest(nick);
		nr->passcode = sstrdup(passcode);
		if (enc_encrypt(pass, strlen(pass), nr->password, PASSMAX - 1) < 0)
			alog("[misc_sqlcmd] Error: Failed to encrypt password for %s.", nick);
		if (email)
			nr->email = sstrdup(email);
		else
			nr->email = NULL;
		nr->requested = time(NULL);

		if (debug)
			alog("[misc_sqlcmd] debug: Nickname request for '%s' generated.. Auto-confirming..", nick);
		return sqlcmd_handle_nickconf(2, av);
	}
}

/**
 * Confirm a nickname.
 *
 * @param ac int Argument count.
 * @param av Array Argument list.
 *    av[0] - Nickname
 *    av[1] - Passcode [Optional]
 **/
int sqlcmd_handle_nickconf(int ac, char **av) {
	NickRequest *nr = NULL;
	NickAlias *na = NULL;
	char *nick, *passcode = NULL;
	int i;

	if (!ac)
		return SQLCMD_ERROR_SYNTAX_ERROR;

	nick = av[0];
	if (ac == 2)
		passcode = av[1];

	if (readonly)
		return SQLCMD_ERROR_READ_ONLY;
	else if (!(nr = findrequestnick(nick)))
		return SQLCMD_ERROR_NICK_CONF_NOT_FOUND;
	else if (NSEmailReg) {
		if (!passcode)
			return SQLCMD_ERROR_SYNTAX_ERROR;
		if (stricmp(nr->passcode, passcode) != 0)
			return SQLCMD_ERROR_NICK_CONF_INVALID;
	} 

	na = makenick(nr->nick);
	if (na) {
		memcpy(na->nc->pass, nr->password, PASSMAX);
		na->nc->flags |= NSDefFlags;
		for (i = 0; i < RootNumber; i++) {
			if (!stricmp(ServicesRoots[i], nr->nick)) {
				na->nc->flags |= NI_SERVICES_ROOT;
				break;
			}
		}
		na->nc->memos.memomax = MSMaxMemos;
		na->nc->channelmax = CSMaxReg;
		na->last_usermask = sstrdup("UNKN@web.created.user");
		na->last_realname = sstrdup("unknown");
		na->time_registered = na->last_seen = time(NULL);
		na->nc->accesscount = 0;
		na->nc->access = NULL;
		na->nc->language = NSDefLanguage;
		if (nr->email)
			na->nc->email = sstrdup(nr->email);

#ifdef USE_RDB
		if (rdb_open()) {
			rdb_save_ns_core(na->nc);
			rdb_save_ns_alias(na);
			rdb_close();
		}
#endif

		send_event(EVENT_NICK_REGISTERED, 1, nick);
		delnickrequest(nr);
		if (debug)
			alog("[misc_sqlcmd] Nickname registration for '%s' completed successfully!", nick);
		return SQLCMD_ERROR_NONE;
	} else
		return SQLCMD_ERROR_NICK_REG_FAILED;
}

/* ------------------------------------------------------------------------------- */

/**
 * Group a nickname.
 *
 * @param ac int Argument count.
 * @param av Array Argument list.
 *    av[0] - Nickname in nickgroup to join.
 *    av[1] - Nick to group
 *    av[2] - Target nickgroup password. [Optional]
 **/
int sqlcmd_handle_nickgroup(int ac, char **av) { 
	char *tnick, *nick, *pass = NULL;
	NickAlias *na, *target;
	NickCore *nc;
	User *u;
	int i, nicklen, prefixlen;

	if (ac < 2)
		return SQLCMD_ERROR_SYNTAX_ERROR;

	tnick = av[0];
	nick = av[1];
	if (ac == 3)
		pass = av[2];
	nicklen = strlen(nick);
	prefixlen = strlen(NSGuestNickPrefix);

	if (readonly)
		return SQLCMD_ERROR_READ_ONLY;
	else if (checkDefCon(DEFCON_NO_NEW_NICKS))
		return SQLCMD_ERROR_DEFCON;
	else if (NSEmailReg && (findrequestnick(nick)))
		return SQLCMD_ERROR_NICK_ALREADY_REQUESTED;
	else if ((na = findnick(nick))) {
		if (na->status & NS_VERBOTEN) {
			alog("[misc_sqlcmd] Attempt to register FORBIDden nick %s.", nick);
			return SQLCMD_ERROR_NICK_FORBIDDEN;
		} else if (na->nc->flags & NI_SUSPENDED) {
			alog("[misc_sqlcmd] Attempt to register SUSPENDed nick %s.", nick);
			return SQLCMD_ERROR_NICK_SUSPENDED;
		} else if (NSNoGroupChange)
			return SQLCMD_ERROR_NICK_GROUP_CHANGE_DISABLED;
		else 
			return SQLCMD_ERROR_NICK_ALREADY_REGISTERED;
	} else if (!na && ((nicklen <= prefixlen + 7 && nicklen >= prefixlen + 1 && stristr(nick, NSGuestNickPrefix) == nick && 
			strspn(nick + prefixlen, "1234567890") == nicklen - prefixlen) || !anope_valid_nick(nick)))
		return SQLCMD_ERROR_NICK_CANNOT_BE_REGISTERED;

	if (RestrictOperNicks) {
		for (i = 0; i < RootNumber; i++)
			if (stristr(nick, ServicesRoots[i]))
				return SQLCMD_ERROR_NICK_CANNOT_BE_REGISTERED;

		for (i = 0; i < servadmins.count && (nc = servadmins.list[i]); i++)
			if (stristr(nick, nc->display))
				return SQLCMD_ERROR_NICK_CANNOT_BE_REGISTERED;

		for (i = 0; i < servopers.count && (nc = servopers.list[i]); i++)
			if (stristr(nick, nc->display))
				return SQLCMD_ERROR_NICK_CANNOT_BE_REGISTERED;
	}

	if ((u = finduser(tnick)))
		return SQLCMD_ERROR_NICK_IN_USE;
	else if (!(target = findnick(tnick)))
		return SQLCMD_ERROR_NICK_NOT_REGISTERED;
	else if (target->status & NS_VERBOTEN) {
		alog("[misc_sqlcmd] Attempt to GROUP %s to FORBIDen nick %s.", nick, tnick);
		return SQLCMD_ERROR_NICK_FORBIDDEN;
	} else if (target->nc->flags & NI_SUSPENDED) {
		alog("[misc_sqlcmd] Attempt to GROUP %s to SUSPENDed nick %s.", nick, tnick);
		return SQLCMD_ERROR_NICK_SUSPENDED;
	} else if (na && target->nc == na->nc)
		return SQLCMD_ERROR_NICK_GROUP_SAME;
	else if (NSMaxAliases && (target->nc->aliases.count >= NSMaxAliases) && !na_is_services_admin(target))
		return SQLCMD_ERROR_NICK_GROUP_TOO_MANY;
	else if (pass && enc_check_password(pass, target->nc->pass) != 1) {
		alog("[misc_sqlcmd] Failed attempt to add %s to group of %s (invalid password).", nick, tnick);
		return SQLCMD_ERROR_ACCESS_DENIED;
	} else {
		if (na)
			delnick(na);

		na = makealias(nick, target->nc);
		if (na) {
			na->last_usermask = scalloc(15, sizeof(char));
			sprintf(na->last_usermask, "UNKWN@web.host");
			na->last_realname = sstrdup(target->last_realname);
			na->time_registered = na->last_seen = time(NULL);
			na->status = (int16) (NS_IDENTIFIED | NS_RECOGNIZED);
			if (!(na->nc->flags & NI_SERVICES_ROOT)) {
				for (i = 0; i < RootNumber; i++) {
					if (!stricmp(ServicesRoots[i], nick)) {
						na->nc->flags |= NI_SERVICES_ROOT;
						break;
					}
				}
			}

#ifdef USE_RDB
			if (rdb_open()) {
				rdb_save_ns_alias(na);
				rdb_close();
			}
#endif
			send_event(EVENT_GROUP, 1, nick);
			alog("[misc_sqlcmd] Added %s to group of %s (%s) (e-mail: %s).", nick, tnick, target->nc->display, (target->nc->email ? target->nc->email : "none"));
			return SQLCMD_ERROR_NONE;
		} else {
			alog("[misc_sqlcmd] makealias(%s) failed!", nick);
			return SQLCMD_ERROR_NICK_REG_FAILED;
		}
	}
}

/* ------------------------------------------------------------------------------- */

/**
 * Drop a nickname.
 *
 * @param ac int Argument count.
 * @param av Array Argument list.
 *    av[0] - Nickname to drop.
 *    av[1] - Nickname password. [Optional]
 **/
int sqlcmd_handle_nickdrop(int ac, char **av) { 
	NickAlias *na;

	if (!ac)
		return SQLCMD_ERROR_SYNTAX_ERROR;

	if (readonly)
		return SQLCMD_ERROR_READ_ONLY;
	else if (!(na = findnick(av[0])))
		return SQLCMD_ERROR_NICK_NOT_REGISTERED;
	else if (ac == 2 && enc_check_password(av[1], na->nc->pass) != 1)
		return SQLCMD_ERROR_ACCESS_DENIED;
	else {
		if (ircd->sqline && (na->status & NS_VERBOTEN))
			anope_cmd_unsqline(na->nick);

		alog("[misc_sqlcmd] Dropped nickname %s (group %s) (e-mail: %s)", na->nick, na->nc->display, (na->nc->email ? na->nc->email : "none"));
		delnick(na);			/* This should also handle deleting the nick from the DB.*/
		send_event(EVENT_NICK_DROPPED, 1, av[0]);
		return SQLCMD_ERROR_NONE;
	}
}

/* EOF */
