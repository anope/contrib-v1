/**
 * Misc channel commands the module adds - Source
 *
 ***********
 * Module Name    : bs_fantasy_ext
 * Author         : Viper <Viper@Absurd-IRC.net>
 * Creation Date  : 21/07/2006
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
 * Last Updated   : 27/02/2007
 *
 **/

#include "misc_cmds.h"

#ifdef ENABLE_TOPIC
int set_topic(User * u, Channel *c, char *topic) {
	ChannelInfo *ci = c->ci;

	if ((ci->flags & CI_TOPICLOCK) && (!is_services_admin(u) && !check_access(u, ci, CA_TOPIC))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
	} else if (!is_services_admin(u)  && !check_access(u, ci, CA_OPDEOPME)) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
    } else {
		if (ci->last_topic)
			free(ci->last_topic);
		ci->last_topic = topic ? sstrdup(topic) : NULL;
		strscpy(ci->last_topic_setter, u->nick, NICKMAX);
		ci->last_topic_time = time(NULL);

		if (c->topic)
			free(c->topic);
		c->topic = topic ? sstrdup(topic) : NULL;
		strscpy(c->topic_setter, u->nick, NICKMAX);
		if (ircd->topictsbackward) {
			c->topic_time = c->topic_time - 1;
		} else {
			c->topic_time = ci->last_topic_time;
		}

		if (is_services_admin(u) && !check_access(u, ci, CA_TOPIC))
			alog("%s: %s!%s@%s changed topic of %s as services admin.",
				ci->bi->nick, u->nick, u->username, u->host, c->name);

		anope_cmd_topic(ci->bi->nick, c->name, u->nick, topic ? topic : "", c->topic_time);
	}
	return MOD_CONT;
}
#endif


#ifdef ENABLE_APPENDTOPIC
int append_to_topic(User * u, Channel *c, char *newtopic) {
	char topic[1024];
	ChannelInfo *ci = c->ci;


	if (!newtopic) {
		moduleNoticeLang(ci->bi->nick, u, LANG_APPENDTOPIC_SYNTAX, BSFantasyCharacter, BSFantasyCharacter);
		return MOD_STOP;
	}


	if (ci->flags & CI_VERBOTEN) {
		notice_lang(ci->bi->nick, u, CHAN_X_FORBIDDEN, ci->name);
		return MOD_STOP;
	}

	if (!check_access(u, ci, CA_TOPIC)) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
		return MOD_STOP;
	} else {
		if (!ci->last_topic) {
			strcpy(topic, newtopic);
		}

		if (ci->last_topic) {
			snprintf(topic, sizeof(topic), "%s || %s", ci->last_topic, newtopic);
			free(ci->last_topic);
		}

		ci->last_topic = topic ? sstrdup(topic) : NULL;
		strscpy(ci->last_topic_setter, u->nick, NICKMAX);
		ci->last_topic_time = time(NULL);
        if (c->topic)  {
            free(c->topic);
        }
		c->topic = topic ? sstrdup(topic) : NULL;
		strscpy(c->topic_setter, u->nick, NICKMAX);

		if (ircd->topictsbackward) {
			c->topic_time = c->topic_time - 1;
		} else {
			c->topic_time = ci->last_topic_time;
		}

		anope_cmd_topic(ci->bi->nick, c->name, u->nick, topic ? topic : "",	c->topic_time);
    }
	return MOD_CONT;
}
#endif


#ifdef ENABLE_INVITE
int do_invite(User * u, Channel *c, char *nick) {
    ChannelInfo *ci = c->ci;

	if (!nick)
		moduleNoticeLang(ci->bi->nick, u, LANG_INVITE_SYNTAX, BSFantasyCharacter);

	else {
		if (check_access(u, ci, CA_OPDEOP)	&& check_access(u, ci, CA_OPDEOPME)) {
			User *u2;

			if ((u2 = finduser(nick))) {
				if (stricmp(u2->nick, u->nick) != 0) {
					if (!is_on_chan(c, u2)) {
						anope_cmd_invite(ci->bi->nick, ci->name, u2->nick);
						notice(ci->bi->nick, ci->name, "%s was invited to the channel.", u2->nick);
					} else
						moduleNoticeLang(ci->bi->nick, u, LANG_INVITE_IS_ON);
				} else
					moduleNoticeLang(ci->bi->nick, u, LANG_INVITE_YOURSELF);
			} else
				moduleNoticeLang(ci->bi->nick, u, LANG_INVITE_NO_USER);
		} else
			notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
	}

	return MOD_CONT;
}
#endif


#ifdef ENABLE_BAN
int do_ban(User * u, Channel *c, char *params) {
	ChannelInfo *ci = c->ci;
	User *u2;
	int is_same, exists;
	char *av[2];

	if (!params) {
		params = u->nick;
		is_same = 1;
	} else
		is_same = (params == u->nick) ? 1 : (stricmp(params, u->nick) == 0);

	if (is_same) {
		u2 = u;
		exists = 1;
	} else
		exists = ((u2 = finduser(params)) ? 1 : 0);

	if (!is_same ? !check_access(u, ci, CA_BAN) : !check_access(u, ci, CA_BANME)) {
		notice_lang(ci->bi->nick, u, ACCESS_DENIED);
	} else if (!is_same && exists && (ci->flags & CI_PEACE) && (get_access(u2, ci) >= get_access(u, ci))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
	/**
	 * Dont ban the user on channels where he is excepted
	 * to prevent services <-> server wars.
	 **/
	} else if (exists && (ircd->except && is_excepted(ci, u2))) {
		notice_lang(ci->bi->nick, u, CHAN_EXCEPTED, u2->nick, ci->name);
	} else if (!exists && (ircd->except && is_excepted_mask(ci, params))) {
		notice_lang(ci->bi->nick, u, CHAN_EXCEPTED, params, ci->name);
	} else if (exists && ((ircd->protectedumode && is_protected(u2)) && !is_founder(u, ci))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
	} else if (exists) {
		char mask[BUFSIZE];

		if (!stricmp(u2->nick, ci->bi->nick))
			bot_raw_kick(u, ci, u->nick, "Oops!");

		else {
			do_up_down(u2, c, "down", 0);

			av[0] = sstrdup("+b");
			get_idealban(ci, u2, mask, sizeof(mask));
			av[1] = sstrdup(mask);
			anope_cmd_mode(ci->bi->nick, c->name, "+b %s", av[1]);
			chan_set_modes(ci->bi->nick, c, 2, av, 1);

			if (av[0]) free(av[0]);
			if (av[1]) free(av[1]);
		}
	} else {
	/* If could be a hostmask.. set whatever we get */

		av[0] = sstrdup("+b");
		av[1] = sstrdup(params);

		anope_cmd_mode(ci->bi->nick, c->name, "+b %s", av[1]);
		chan_set_modes(ci->bi->nick, c, 2, av, 1);

		if (av[0]) free(av[0]);
		if (av[1]) free(av[1]);
	}

	return MOD_CONT;
}
#endif


#ifdef ENABLE_MUTEUNMUTE
int do_mute(User * u, Channel *c, char *params) {
	ChannelInfo *ci = c->ci;
	User *u2;
	int is_same, exists;
	char *av[2];
	char buf[256];

	if (!params) {
		params = u->nick;
	}

	is_same = (params == u->nick) ? 1 : (stricmp(params, u->nick) == 0);

	if (is_same) {
		u2 = u;
		exists = 1;
	} else
		exists = ((u2 = finduser(params)) ? 1 : 0);

	if (!is_same ? !check_access(u, ci, CA_BAN) : !check_access(u, ci, CA_BANME)) {
		notice_lang(ci->bi->nick, u, ACCESS_DENIED);
	} else if (!is_same && exists && (ci->flags & CI_PEACE) && (get_access(u2, ci) >= get_access(u, ci))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
	} else if (exists && ((ircd->protectedumode && is_protected(u2)) && !is_founder(u, ci))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
	/**
	 * Dont ban the user on channels where he is excepted
	 * to prevent services <-> server wars.
	 **/
	} else if (exists && (ircd->except && is_excepted(ci, u2))) {
		notice_lang(ci->bi->nick, u, CHAN_EXCEPTED, u2->nick, ci->name);
	} else if (!exists && (ircd->except && is_excepted_mask(ci, params))) {
		notice_lang(ci->bi->nick, u, CHAN_EXCEPTED, params, ci->name);
	} else {
		if (!stricmp(params, ci->bi->nick))
			bot_raw_kick(u, ci, u->nick, "Oops!");

		else {
			char mask[BUFSIZE];

			if (exists) {
				do_up_down(u2, c, "down", 0);
				get_idealban(ci, u2, mask, sizeof(mask));
			}

			av[0] = sstrdup("+b");

			if (!stricmp(IRCDModule, "unreal32")) {
				if (exists)
					snprintf(buf, sizeof(buf), "~q:%s", mask);
				else
					snprintf(buf, sizeof(buf), "~q:%s", params);

				av[1] = sstrdup(buf);

			} else {
				if (exists)
					av[1] = sstrdup(mask);
				else
					av[1] = sstrdup(params);

			}

			anope_cmd_mode(ci->bi->nick,c ->name, "+b %s", av[1]);
			chan_set_modes(ci->bi->nick, c, 2, av, 1);


			if (av[0]) free(av[0]);
			if (av[1]) free(av[1]);
		}
	}

	return MOD_CONT;
}

int do_unmute(User * u, Channel *c, char *params) {
	ChannelInfo *ci = c->ci;
	User *u2;
	int is_same, exists;
	char *av[2];
	char buf[256];

	if (!params) {
		params = u->nick;
	}

	is_same = (params == u->nick) ? 1 : (stricmp(params, u->nick) == 0);

	if (is_same) {
		u2 = u;
		exists = 1;
	} else
		exists = ((u2 = finduser(params)) ? 1 : 0);

	if (!check_access(u, ci, CA_UNBAN)) {
		notice_lang(ci->bi->nick, u, ACCESS_DENIED);
	} else {
		char mask[BUFSIZE];

		if (exists) {
			get_idealban(ci, u2, mask, sizeof(mask));
		}

		av[0] = sstrdup("-b");

		if (!stricmp(IRCDModule, "unreal32")) {
			if (exists)
				snprintf(buf, sizeof(buf), "~q:%s", mask);
			else
				snprintf(buf, sizeof(buf), "~q:%s", params);

			av[1] = sstrdup(buf);

		} else {
			if (exists)
				av[1] = sstrdup(mask);
			else
				av[1] = sstrdup(params);

		}
		anope_cmd_mode(ci->bi->nick,c ->name, "-b %s", av[1]);
		chan_set_modes(ci->bi->nick, c, 2, av, 1);

		if (av[0]) free(av[0]);
		if (av[1]) free(av[1]);
	}

	return MOD_CONT;
}
#endif


#ifdef ENABLE_KICKBAN
int do_core_kickban(User * u, Channel *c, char *target, char *reason) {
	ChannelInfo *ci = c->ci;
	User *u2;
	int is_same, exists;
	char *av[2];

	if (!target) {
		target = u->nick;
	}

	is_same = (target == u->nick) ? 1 : (stricmp(target, u->nick) == 0);

	if (is_same) {
		u2 = u;
		exists = 1;
	} else
		exists = ((u2 = finduser(target)) ? 1 : 0);

	if (!is_same ? !check_access(u, ci, CA_BAN) : !check_access(u, ci, CA_BANME)) {
		notice_lang(ci->bi->nick, u, ACCESS_DENIED);
	} else if (!is_same && exists && (ci->flags & CI_PEACE) && (get_access(u2, ci) >= get_access(u, ci))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
	} else if (exists && ((ircd->protectedumode && is_protected(u2)) && !is_founder(u, ci))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
	/**
	 * Dont ban the user on channels where he is excepted
	 * to prevent services <-> server wars.
	 **/
	} else if (exists && (ircd->except && is_excepted(ci, u2))) {
		notice_lang(ci->bi->nick, u, CHAN_EXCEPTED, u2->nick, ci->name);
	} else if (!exists && (ircd->except && is_excepted_mask(ci, target))) {
		notice_lang(ci->bi->nick, u, CHAN_EXCEPTED, target, ci->name);
	} else if (stricmp(target, ci->bi->nick) == 0) {
		bot_raw_ban(u, ci, u->nick, "Oops!");
	} else {
		if (exists) {
			if (is_on_chan(ci->c, u2)) {
				if (!reason)
					bot_raw_ban(u, ci, target, "Requested");
				else
					bot_raw_ban(u, ci, target, reason);
			}

		} else if (match_wild_nocase("*@*", target)) {
			struct c_userlist *cu = NULL;
			struct c_userlist *next;
			int ok = 1;
			cu = c->users;

			/* Check to make sure were are not going to ban an admin here... */
			while ((cu) && ok) {
				next = cu->next;
				if (match_usermask(target, cu->user)) {
					if (ircd->protectedumode && is_protected(cu->user) && !is_founder(u, ci)) {
						notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
						ok = 0;
					} else if ((ci->flags & CI_PEACE) && (get_access(cu->user, ci) >= get_access(u, ci))) {
						notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
						ok = 0;
					} else if (ircd->except && is_excepted(ci, cu->user)) {
						notice_lang(ci->bi->nick, u, CHAN_EXCEPTED, cu->user->nick, ci->name);
						ok = 0;
					}
				}
				cu = next;
			}

			if (ok) {
				av[0] = sstrdup("+b");
				av[1] = sstrdup(target);

				anope_cmd_mode(ci->bi->nick, c->name, "+b %s", av[1]);
				chan_set_modes(ci->bi->nick, c, 2, av, 1);

				if (av[0]) free(av[0]);
				if (av[1]) free(av[1]);

				cu = c->users;

				while (cu) {
					next = cu->next;
					if (match_usermask(target, cu->user)) {
						if (!reason)
							bot_raw_kick(u, ci, cu->user->nick, "Requested");
						else
							bot_raw_kick(u, ci, cu->user->nick, reason);

					}
					cu = next;
				}
			}
		} else
			moduleNoticeLang(ci->bi->nick, u, LANG_REQ_NICK_OR_MASK);
	}
	return MOD_CONT;
}
#endif

/* EOF */
