/**
 * Main routines for performing channel operations.
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
 * Register a channel.
 *
 * @param ac int Argument count.
 * @param av Array Argument list.
 *    av[0] - Nickname
 *    av[1] - Channel name
 *    av[2] - Channel password
 *    av[3] - Channel description
 **/
int sqlcmd_handle_chanreg(int ac, char **av) {
	NickAlias *na;
	NickCore *nc;
	Channel *c;
	ChannelInfo *ci;
	char *nick, *chan, *pass, *desc;

	if (ac != 4)
		return SQLCMD_ERROR_SYNTAX_ERROR;

	nick = av[0];
	chan = av[1];
	pass = av[2];
	desc = av[3];
	na = findnick(nick);

	if (readonly) 
		return SQLCMD_ERROR_READ_ONLY;
	else if (checkDefCon(DEFCON_NO_NEW_CHANNELS))
		return SQLCMD_ERROR_DEFCON;
	else if (*chan != '#')
		return SQLCMD_ERROR_CHAN_SYM_REQ;
	else if (!anope_valid_chan(chan))
		return SQLCMD_ERROR_CHAN_INVALID;
	else if (!na || !(nc = na->nc))
		return SQLCMD_ERROR_NICK_NOT_REGISTERED;
	else if (na->status & NS_VERBOTEN)
		return SQLCMD_ERROR_NICK_FORBIDDEN;
	else if (na->nc->flags & NI_SUSPENDED)
		return SQLCMD_ERROR_NICK_SUSPENDED;
	else if ((c = findchan(chan)))
		return SQLCMD_ERROR_CHAN_MUST_BE_EMPTY;
	else if ((ci = cs_findchan(chan)) != NULL) {
		if (ci->flags & CI_VERBOTEN) {
			alog("[misc_sqlcmd] Attempt to register FORBIDden channel %s by %s.", ci->name, nick);
			return SQLCMD_ERROR_CHAN_FORBIDDEN;
		} else
			return SQLCMD_ERROR_CHAN_ALREADY_REG;
	} else if (!stricmp(chan, "#"))
		return SQLCMD_ERROR_CHAN_MAY_NOT_BE_REG;
	else if (nc->channelmax > 0 && nc->channelcount >= nc->channelmax)
		return SQLCMD_ERROR_REACHED_CHAN_LIMIT;
	else if (stricmp(nick, pass) == 0 || (StrictPasswords && strlen(pass) < 5))
		return SQLCMD_ERROR_MORE_OBSCURE_PASS;
	else if (enc_encrypt_check_len(strlen(pass), PASSMAX - 1))
		return SQLCMD_ERROR_PASS_TOO_LONG;
	else if (!(ci = makechan(chan))) {
		alog("[misc_sqlcmd] makechan() failed for REGISTER %s", chan);
		return SQLCMD_ERROR_CHAN_REG_FAILED;
	} else if (enc_encrypt(pass, strlen(pass), ci->founderpass, PASSMAX - 1) < 0) {
		alog("[misc_sqlcmd] Couldn't encrypt password for %s (REGISTER)", chan);
		delchan(ci);
		return SQLCMD_ERROR_CHAN_REG_FAILED;
	} else {
		ci->bantype = CSDefBantype;
		ci->flags = CSDefFlags;
		ci->mlock_on = ircd->defmlock;
		ci->memos.memomax = MSMaxMemos;
		ci->last_used = ci->time_registered;
		ci->founder = nc;

		ci->desc = sstrdup(desc);
		/* Set this to something, otherwise it will maliform the topic */
		strscpy(ci->last_topic_setter, s_ChanServ, NICKMAX);        
		ci->bi = NULL;
		ci->botflags = BSDefFlags;
		ci->founder->channelcount++;
		alog("[misc_sqlcmd] Channel '%s' registered by %s", chan, av[0]);

#ifdef USE_RDB
		if (rdb_open()) {
			rdb_save_cs_info(ci);
			rdb_close();
		}
#endif

		send_event(EVENT_CHAN_REGISTERED, 1, chan);
		return SQLCMD_ERROR_NONE;
	}
}

/* ------------------------------------------------------------------------------- */

/**
 * Add an SOP to a channel.
 *
 * @param ac int Argument count.
 * @param av Array Argument list.
 *    av[0] - Channel name.
 *    av[1] - User to add.
 *    av[2] - Nick of the user adding the entry. [Recommended]
 **/
int sqlcmd_handle_chanaddsop(int ac, char **av) {
	return sqlcmd_handle_addxop(ac, av, ACCESS_SOP);
}

/**
 * Add an AOP to a channel.
 *
 * @param ac int Argument count.
 * @param av Array Argument list.
 *    av[0] - Channel name.
 *    av[1] - User to add.
 *    av[2] - Nick of the user adding the entry. [Recommended]
 **/
int sqlcmd_handle_chanaddaop(int ac, char **av) {
	return sqlcmd_handle_addxop(ac, av, ACCESS_AOP);
}

/**
 * Add a HOP to a channel.
 *
 * @param ac int Argument count.
 * @param av Array Argument list.
 *    av[0] - Channel name.
 *    av[1] - User to add.
 *    av[2] - Nick of the user adding the entry. [Recommended]
 **/
int sqlcmd_handle_chanaddhop(int ac, char **av) {
	return sqlcmd_handle_addxop(ac, av, ACCESS_HOP);
}

/**
 * Add a VOP to a channel.
 *
 * @param ac int Argument count.
 * @param av Array Argument list.
 *    av[0] - Channel name.
 *    av[1] - User add.
 *    av[2] - Nick of the user adding the entry. [Recommended]
 **/
int sqlcmd_handle_chanaddvop(int ac, char **av) {
	return sqlcmd_handle_addxop(ac, av, ACCESS_VOP);
}

/**
 * Add a user to a channel's XOP list.
 *
 * @param ac int Argument count.
 * @param av Array Argument list.
 *    av[0] - Channel name.
 *    av[1] - User to add.
 *    av[2] - Nick of the user adding the entry. [Recommended]
 * @param lvl int XOP level to assign to the user.
 **/
int sqlcmd_handle_addxop(int ac, char **av, int lvl) {
	ChannelInfo *ci = NULL;

	if (ac < 2 || ac > 3)
		return SQLCMD_ERROR_SYNTAX_ERROR;

	if (readonly)
		return SQLCMD_ERROR_READ_ONLY;
	else if (!(ci = cs_findchan(av[0])))
		return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
	else if (ci->flags & CI_VERBOTEN)
		return SQLCMD_ERROR_CHAN_FORBIDDEN;
	else if (ci->flags & CI_SUSPENDED)
		return SQLCMD_ERROR_CHAN_SUSPENDED;
	else if (!(ci->flags & CI_XOP))
		return SQLCMD_ERROR_CHAN_NOT_XOP;

	return sqlcmd_do_access_add(ci, av[1], lvl, ac == 3 ? av[2] : NULL);
}

/**
 * Add a user to a channel's access list.
 *
 * @param ac int Argument count.
 * @param av Array Argument list.
 *    av[0] - Channel name.
 *    av[1] - User to add.
 *    av[2] - Level to add the user with.
 *    av[3] - Nick of the user adding the entry. [Recommended]
 **/
int sqlcmd_handle_chanaddaccess(int ac, char **av) {
	ChannelInfo *ci = NULL;

	if (ac < 3 || ac > 4)
		return SQLCMD_ERROR_SYNTAX_ERROR;

	if (readonly)
		return SQLCMD_ERROR_READ_ONLY;
	else if (!(ci = cs_findchan(av[0])))
		return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
	else if (ci->flags & CI_VERBOTEN)
		return SQLCMD_ERROR_CHAN_FORBIDDEN;
	else if (ci->flags & CI_SUSPENDED)
		return SQLCMD_ERROR_CHAN_SUSPENDED;
	else if (ci->flags & CI_XOP)
		return SQLCMD_ERROR_CHAN_NOT_ACC;

	return sqlcmd_do_access_add(ci, av[1], atoi(av[2]), ac == 4 ? av[3] : NULL);
}

/**
 * Delete a user from a channel's access list.
 *
 * @param ac int Argument count.
 * @param av Array Argument list.
 *    av[0] - Channel name.
 *    av[1] - User to delete.
 *    av[2] - Nick of the user deleting the entry. [Recommended]
 **/
int sqlcmd_handle_chandelaccess(int ac, char **av) {
	ChannelInfo *ci = NULL;

	if (ac < 2 || ac > 3)
		return SQLCMD_ERROR_SYNTAX_ERROR;

	if (readonly)
		return SQLCMD_ERROR_READ_ONLY;
	else if (!(ci = cs_findchan(av[0])))
		return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
	else if (ci->flags & CI_VERBOTEN)
		return SQLCMD_ERROR_CHAN_FORBIDDEN;
	else if (ci->flags & CI_SUSPENDED)
		return SQLCMD_ERROR_CHAN_SUSPENDED;

	return sqlcmd_do_access_del(ci, av[1], ac == 3 ? av[2] : NULL);
}

/* ------------------------------------------------------------------------------- */

/**
 * Updates a users level on a channels access list.
 *
 * @param ci ChannelInfo Channel to perform the update in.
 * @param target String User to modify access level of.
 * @param lvl int New access level for target user.
 * @param by String Nick of the user performing the update. [Optional - Only used for logging]
 **/
int sqlcmd_do_access_add(ChannelInfo *ci, char *target, int lvl, char *by) {
	char event_access[BUFSIZE];
	int change = 0, i = 0;
	NickAlias *na = NULL;
	NickCore *nc = NULL;
	ChanAccess *access = NULL;

	na = findnick(target);
	if (!na)
		return SQLCMD_ERROR_NICK_NOT_REGISTERED;
	else if (na->status & NS_VERBOTEN)
		return SQLCMD_ERROR_NICK_FORBIDDEN;
	else if (na->nc->flags & NI_SUSPENDED)
		return SQLCMD_ERROR_NICK_SUSPENDED;

	nc = na->nc;
	for (access = ci->access, i = 0; i < ci->accesscount; access++, i++) {
		if (access->nc == nc) {
			change++;
			break;
		}
	}

	if (!change) {
		if (i < CSAccessMax) {
			ci->accesscount++;
			ci->access = srealloc(ci->access, sizeof(ChanAccess) * ci->accesscount);
		} else
			return SQLCMD_ERROR_CHAN_ACCESS_REACHED_LIMIT;

		access = &ci->access[i];
		access->nc = nc;
	}

	access->in_use = 1;
	access->level = lvl;
	access->last_seen = 0;

	if (by)
		alog("[misc_sqlcmd] %s %s access level %d to %s (group %s) on channel %s", by, change ? "changed" : "set", access->level, na->nick, nc->display, ci->name);
	else
		alog("[misc_sqlcmd] Access level for %s (group %s) was %s to %d on channel %s", na->nick, nc->display, change ? "changed" : "set", access->level, ci->name);
	snprintf(event_access, BUFSIZE, "%d", access->level);

#ifdef USE_RDB
	if (rdb_open()) {
		rdb_save_cs_info(ci);
		rdb_close();
	}
#endif

	if (!change)
		send_event(EVENT_ACCESS_CHANGE, 4, ci->name, by ? by : "", na->nick, event_access);
	else
		send_event(EVENT_ACCESS_ADD, 4, ci->name, by ? by : "", na->nick, event_access);

	return SQLCMD_ERROR_NONE;
}

/**
 * Deletes a users from a channels access list.
 *
 * @param ci ChannelInfo Channel to edit the access list for.
 * @param target String User to remove from the access list.
 * @param by String Nick of the user performing the update. [Optional - Only used for logging]
 **/
int sqlcmd_do_access_del(ChannelInfo *ci, char *target,  char *by) {
	NickAlias *na = NULL;
	NickCore *nc = NULL;
	ChanAccess *access = NULL;
	int i;

	na = findnick(target);
	if (!na)
		return SQLCMD_ERROR_NICK_NOT_REGISTERED;

	nc = na->nc;
	for (i = 0; i < ci->accesscount; i++)
		if (ci->access[i].nc == nc)
			break;

	if (i == ci->accesscount)
		return SQLCMD_ERROR_CHAN_ACC_NOT_FOUND;
   
	access = &ci->access[i];
	send_event(EVENT_ACCESS_DEL, 3, ci->name, by ? by : "", na->nick);
	access->nc = NULL;
	access->in_use = 0;
	CleanAccess(ci);
	if (by)
		alog("[misc_sqlcmd] %s deleted access of user %s on %s", by, na->nick, ci->name);
	else
		alog("[misc_sqlcmd] Access of user %s was deleted on %s", na->nick, ci->name);

#ifdef USE_RDB
	if (rdb_open()) {
		rdb_save_cs_info(ci);
		rdb_close();
	}
#endif

	return SQLCMD_ERROR_NONE;
}

/* ------------------------------------------------------------------------------- */

/**
 * Change the topic of a channel.
 *
 * @param ac int Argument count.
 * @param av Array Argument list.
 *    av[0] - Nick of the user setting the password. [Optional]
 *    av[1] - Channel name.
 *    av[2] - New topic. Leave empty to clear topic.
 **/
int sqlcmd_handle_chantopic(int ac, char **av) {
	Channel *c;
	ChannelInfo *ci;
	char *setter = NULL, *chan = NULL, *topic = NULL;

	if (!ac)
		return SQLCMD_ERROR_SYNTAX_ERROR;
	else if (av[0][0] != '#') {
		setter = av[0];
		chan = av[1];
		if (ac == 3)
			topic = av[2];
	} else {
		setter = s_ChanServ;
		chan = av[0];
		/* We may have to merge the last 2 arguments.. */
		if (ac == 3) {
			av[1] = srealloc(av[1], sizeof(char*) * (strlen(av[1]) + strlen(av[2]) + 2));
			strcat(av[1], " ");
			strcat(av[1], av[2]);
		}
		topic = av[1];
	}

	if (!(c = findchan(chan)))
		return SQLCMD_ERROR_CHAN_NOT_IN_USE;
	else if (!(ci = c->ci))
		return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
	else if (ci->flags & CI_VERBOTEN)
		return SQLCMD_ERROR_CHAN_FORBIDDEN;
	else if (ci->flags & CI_SUSPENDED)
		return SQLCMD_ERROR_CHAN_SUSPENDED;
	else {
		if (ci->last_topic)
			free(ci->last_topic);
		ci->last_topic = topic ? sstrdup(topic) : NULL;
		strscpy(ci->last_topic_setter, setter, NICKMAX);
		ci->last_topic_time = time(NULL);

		if (c->topic)
			free(c->topic);
		c->topic = topic ? sstrdup(topic) : NULL;
		strscpy(c->topic_setter, setter, NICKMAX);
		if (ircd->topictsbackward)
			c->topic_time = c->topic_time - 1;
		else
			c->topic_time = ci->last_topic_time;

		if (ircd->join2set) {
			if (whosends(ci) == s_ChanServ) {
				anope_cmd_join(s_ChanServ, c->name, c->creation_time);
				anope_cmd_mode(NULL, c->name, "+o %s", GET_BOT(s_ChanServ));
			}
		}
		anope_cmd_topic(whosends(ci), c->name, setter, topic, c->topic_time);
		if (ircd->join2set) {
			if (whosends(ci) == s_ChanServ)
				anope_cmd_part(s_ChanServ, c->name, NULL);
		}
		if (debug)
			alog("[misc_sqlcmd] debug: Changed topic of %s to '%s' (%s).", c->name, topic ? topic : "", 
					(setter && setter != s_ChanServ ? setter : ""));

#ifdef USE_RDB
		if (rdb_open()) {
			rdb_save_cs_info(ci);
			rdb_close();
		}
#endif

	}
	return SQLCMD_ERROR_NONE;
}

/* ------------------------------------------------------------------------------- */

/**
 * Drop a channel.
 *
 * @param ac int Argument count.
 * @param av Array Argument list.
 *    av[0] - Nick of the user dropping the channel.
 *    av[1] - Channel name.
 *    av[2] - Channel password. [Optional]
 **/
int sqlcmd_handle_chandrop(int ac, char **av) {
	char *nick = NULL, *chan = NULL, *pass = NULL;
	ChannelInfo *ci;
	NickAlias *na = findnick(av[0]);

	if (ac < 2)
		return SQLCMD_ERROR_SYNTAX_ERROR;

	nick = av[0];
	chan = av[1];
	if (ac == 3 && av[2])
		pass = av[2];

	if (readonly)
		return SQLCMD_ERROR_READ_ONLY;
	else if (!(ci = cs_findchan(chan)))
		return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
	else if (ci->flags & CI_VERBOTEN)
		return SQLCMD_ERROR_CHAN_FORBIDDEN;
	else if (ci->flags & CI_SUSPENDED)
		return SQLCMD_ERROR_CHAN_SUSPENDED;
	else if (!na)
		return SQLCMD_ERROR_NICK_NOT_REGISTERED;
	else if (na->status & NS_VERBOTEN)
		return SQLCMD_ERROR_NICK_FORBIDDEN;
	else if (na->nc->flags & NI_SUSPENDED)
		return SQLCMD_ERROR_NICK_SUSPENDED;
	else if (!(na_is_services_admin(na) || na->nc == ci->founder || 
			(!(ci->flags & CI_SECUREFOUNDER) && pass && enc_check_password(pass, ci->founderpass))))
		return SQLCMD_ERROR_ACCESS_DENIED;
	else {
		if (ci->c) {
			if (ircd->regmode) {
				ci->c->mode &= ~ircd->regmode;
				anope_cmd_mode(whosends(ci), ci->name, "-r");
			}
		}
		if (ircd->chansqline && (ci->flags & CI_VERBOTEN)) {
			anope_cmd_unsqline(ci->name);
		}

		alog("[misc_sqlcmd] Channel %s dropped by %s (founder: %s)", ci->name, nick, (ci->founder ? ci->founder->display : "(none)"));
		sql_del_cs_info(ci->name);
		delchan(ci);
		send_event(EVENT_CHAN_DROP, 1, chan);
		return SQLCMD_ERROR_NONE;
	}
}

/* EOF */
