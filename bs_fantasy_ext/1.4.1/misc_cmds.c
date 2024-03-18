/**
 * Misc channel commands the module adds - Source
 *
 ***********
 * Module Name    : bs_fantasy_ext
 * Author         : Viper <Viper@Anope.org>
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
 * Last Updated   : 03/09/2012
 *
 **/

#include "misc_cmds.h"

#ifdef ENABLE_TOPIC
int set_topic(User * u, Channel *c, char *topic) {
	ChannelInfo *ci = c->ci;

	if (!my_check_access(u, ci, CA_TOPIC) && ((ci->flags & CI_TOPICLOCK) || !my_check_access(u, ci, CA_OPDEOPME)))
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
	else {
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

		if (is_services_admin(u) && !check_access(u, ci, CA_TOPIC) &&
				((ci->flags & CI_TOPICLOCK) || !check_access(u, ci, CA_OPDEOPME)))
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
		noticeLang(ci->bi->nick, u, LANG_APPENDT_SYNTAX);
		return MOD_STOP;
	}

	if (ci->flags & CI_VERBOTEN) {
		notice_lang(ci->bi->nick, u, CHAN_X_FORBIDDEN, ci->name);
		return MOD_STOP;
	}

	if (!my_check_access(u, ci, CA_TOPIC) && ((ci->flags & CI_TOPICLOCK) || !my_check_access(u, ci, CA_OPDEOPME))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
		return MOD_STOP;
	}

	if (ci->last_topic) {
		snprintf(topic, sizeof(topic), "%s %s %s", ci->last_topic,
			(AppendToTopicDel ? AppendToTopicDel : "||"), newtopic);
		free(ci->last_topic);
	} else
		strcpy(topic, newtopic);

	ci->last_topic = *topic ? sstrdup(topic) : NULL;
	strscpy(ci->last_topic_setter, u->nick, NICKMAX);
	ci->last_topic_time = time(NULL);

	if (c->topic) free(c->topic);
	c->topic = *topic ? sstrdup(topic) : NULL;
	strscpy(c->topic_setter, u->nick, NICKMAX);

	if (ircd->topictsbackward) {
		c->topic_time = c->topic_time - 1;
	} else {
		c->topic_time = ci->last_topic_time;
	}

	if (is_services_admin(u) && !check_access(u, ci, CA_TOPIC) &&
			((ci->flags & CI_TOPICLOCK) || !check_access(u, ci, CA_OPDEOPME)))
		alog("%s: %s!%s@%s changed topic of %s as services admin.",
			ci->bi->nick, u->nick, u->username, u->host, c->name);

	anope_cmd_topic(ci->bi->nick, c->name, u->nick, topic, c->topic_time);

	return MOD_CONT;
}

int topic_replace_append(User * u, Channel *c, char *newtopic) {
	char topic[1024], buf[1024], *mtopic;
	ChannelInfo *ci = c->ci;

	if (ci->flags & CI_VERBOTEN) {
		notice_lang(ci->bi->nick, u, CHAN_X_FORBIDDEN, ci->name);
		return MOD_STOP;
	}

	if (!my_check_access(u, ci, CA_TOPIC) && ((ci->flags & CI_TOPICLOCK) || !my_check_access(u, ci, CA_OPDEOPME))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
		return MOD_STOP;
	}

	memset(topic, 0, 1024);
	if (ci->last_topic) {
		/* Copy topic up to the last append.. */
		mtopic = strrstr(ci->last_topic, (AppendToTopicDel ? AppendToTopicDel : "||"));
		if (mtopic) {
			memset(buf, 0, 1024);
			strncat(buf, ci->last_topic, mtopic - ci->last_topic);

			if (newtopic)
				snprintf(topic, sizeof(topic), "%s%s %s", buf,
						(AppendToTopicDel ? AppendToTopicDel : "||"), newtopic);
			else
				snprintf(topic, sizeof(topic), "%s", buf);
		} else
			snprintf(topic, sizeof(topic), "%s %s %s", ci->last_topic,
					(AppendToTopicDel ? AppendToTopicDel : "||"), newtopic);
	} else if (newtopic)
		strcpy(topic, newtopic);

	ci->last_topic = *topic ? sstrdup(topic) : NULL;
	strscpy(ci->last_topic_setter, u->nick, NICKMAX);
	ci->last_topic_time = time(NULL);

	if (c->topic) free(c->topic);
	c->topic = *topic ? sstrdup(topic) : NULL;
	strscpy(c->topic_setter, u->nick, NICKMAX);

	if (ircd->topictsbackward) {
		c->topic_time = c->topic_time - 1;
	} else {
		c->topic_time = ci->last_topic_time;
	}

	if (is_services_admin(u) && !check_access(u, ci, CA_TOPIC) &&
			((ci->flags & CI_TOPICLOCK) || !check_access(u, ci, CA_OPDEOPME)))
		alog("%s: %s!%s@%s changed topic of %s as services admin.",
			ci->bi->nick, u->nick, u->username, u->host, c->name);

	anope_cmd_topic(ci->bi->nick, c->name, u->nick, topic, c->topic_time);

	return MOD_CONT;
}
#endif

#ifdef ENABLE_PREPENDTOPIC
int prepend_to_topic(User * u, Channel *c, char *newtopic) {
	char topic[1024];
	ChannelInfo *ci = c->ci;

	if (!newtopic) {
		noticeLang(ci->bi->nick, u, LANG_PREPENDT_SYNTAX);
		return MOD_STOP;
	}

	if (ci->flags & CI_VERBOTEN) {
		notice_lang(ci->bi->nick, u, CHAN_X_FORBIDDEN, ci->name);
		return MOD_STOP;
	}

	if (!my_check_access(u, ci, CA_TOPIC) && ((ci->flags & CI_TOPICLOCK) || !my_check_access(u, ci, CA_OPDEOPME))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
		return MOD_STOP;
	}

	if (ci->last_topic) {
		snprintf(topic, sizeof(topic), "%s %s %s", newtopic,
			(AppendToTopicDel ? AppendToTopicDel : "||"), ci->last_topic);
		free(ci->last_topic);
	} else
		strcpy(topic, newtopic);

	ci->last_topic = *topic ? sstrdup(topic) : NULL;
	strscpy(ci->last_topic_setter, u->nick, NICKMAX);
	ci->last_topic_time = time(NULL);

	if (c->topic) free(c->topic);
	c->topic = *topic ? sstrdup(topic) : NULL;
	strscpy(c->topic_setter, u->nick, NICKMAX);

	if (ircd->topictsbackward) {
		c->topic_time = c->topic_time - 1;
	} else {
		c->topic_time = ci->last_topic_time;
	}

	if (is_services_admin(u) && !check_access(u, ci, CA_TOPIC) &&
			((ci->flags & CI_TOPICLOCK) || !check_access(u, ci, CA_OPDEOPME)))
		alog("%s: %s!%s@%s changed topic of %s as services admin.",
			ci->bi->nick, u->nick, u->username, u->host, c->name);

	anope_cmd_topic(ci->bi->nick, c->name, u->nick, topic, c->topic_time);

	return MOD_CONT;
}

int topic_replace_prepend(User * u, Channel *c, char *newtopic) {
	char topic[1024], buf[1024], *mtopic;
	ChannelInfo *ci = c->ci;

	if (ci->flags & CI_VERBOTEN) {
		notice_lang(ci->bi->nick, u, CHAN_X_FORBIDDEN, ci->name);
		return MOD_STOP;
	}

	if (!my_check_access(u, ci, CA_TOPIC) && ((ci->flags & CI_TOPICLOCK) || !my_check_access(u, ci, CA_OPDEOPME))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
		return MOD_STOP;
	}

	memset(topic, 0, 1024);
	if (ci->last_topic) {
		/* Copy topic starting after the first delimiter.. */
		mtopic = strstr(ci->last_topic, (AppendToTopicDel ? AppendToTopicDel : "||"));
		if (mtopic) {
			memset(buf, 0, 1024);
			strcat(buf, mtopic + strlen((AppendToTopicDel ? AppendToTopicDel : "||")));

			if (newtopic)
				snprintf(topic, sizeof(topic), "%s %s%s", newtopic,
						(AppendToTopicDel ? AppendToTopicDel : "||"), buf);
			else
				snprintf(topic, sizeof(topic), "%s", buf);
		} else
			snprintf(topic, sizeof(topic), "%s %s %s", newtopic,
					(AppendToTopicDel ? AppendToTopicDel : "||"), ci->last_topic);
	} else if (newtopic)
		strcpy(topic, newtopic);

	ci->last_topic = *topic ? sstrdup(topic) : NULL;
	strscpy(ci->last_topic_setter, u->nick, NICKMAX);
	ci->last_topic_time = time(NULL);

	if (c->topic) free(c->topic);
	c->topic = *topic ? sstrdup(topic) : NULL;
	strscpy(c->topic_setter, u->nick, NICKMAX);

	if (ircd->topictsbackward) {
		c->topic_time = c->topic_time - 1;
	} else {
		c->topic_time = ci->last_topic_time;
	}

	if (is_services_admin(u) && !check_access(u, ci, CA_TOPIC) &&
			((ci->flags & CI_TOPICLOCK) || !check_access(u, ci, CA_OPDEOPME)))
		alog("%s: %s!%s@%s changed topic of %s as services admin.",
			ci->bi->nick, u->nick, u->username, u->host, c->name);

	anope_cmd_topic(ci->bi->nick, c->name, u->nick, topic, c->topic_time);

	return MOD_CONT;
}
#endif

#ifdef ENABLE_INVITE
int do_invite(User * u, Channel *c, char *nick) {
	ChannelInfo *ci = c->ci;

	if (!nick)
		noticeLang(ci->bi->nick, u, LANG_INVITE_SYNTAX);

	else {
		if (check_access(u, ci, CA_OPDEOP)	&& check_access(u, ci, CA_OPDEOPME)) {
			User *u2;

			if ((u2 = finduser(nick))) {
				if (stricmp(u2->nick, u->nick) != 0) {
					if (!is_on_chan(c, u2)) {
						anope_cmd_invite(ci->bi->nick, ci->name, u2->nick);
						notice(ci->bi->nick, ci->name, "%s was invited to the channel.", u2->nick);
					} else
						noticeLang(ci->bi->nick, u, LANG_INVITE_IS_ON);
				} else
					noticeLang(ci->bi->nick, u, LANG_INVITE_YOURSELF);
			} else
				noticeLang(ci->bi->nick, u, LANG_INVITE_NO_USER);
		} else
			notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
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

	if (!target)
		target = u->nick;

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
	} else if (exists && RestrictKB && ((!is_founder(u, ci) && is_services_oper(u2)) ||
			(is_founder(u, ci) && is_services_admin(u2)))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
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

		} else if (my_match_wild_nocase("*@*", target)) {
			char mask[BUFSIZE];

			/* If we get a *@* target we need to add the *!... */
			if (!my_match_wild_nocase("*!*@*", target))
				snprintf(mask, BUFSIZE, "*!%s", target);
			else
				snprintf(mask, BUFSIZE, "%s", target);

			/* Only continue if the mask doesn't match an exception or is otherwise prohibited.. */
			if (check_banmask(u, c, mask)) {
				struct c_userlist *cu = NULL, *next = NULL;

				av[0] = "+b";
				av[1] = mask;

				anope_cmd_mode(ci->bi->nick, c->name, "+b %s", av[1]);
				chan_set_modes(ci->bi->nick, c, 2, av, 1);

				cu = c->users;
				while (cu) {
					next = cu->next;
					/* This only checks against the cloacked host & vhost for normal users.
					 * IPs are only checked when triggered by an oper.. */
					if (is_oper(u) ? match_usermask_full(mask, cu->user, true) : match_usermask(mask, cu->user)) {
						if (!reason)
							bot_raw_kick(u, ci, cu->user->nick, "Requested");
						else
							bot_raw_kick(u, ci, cu->user->nick, reason);
					}
					cu = next;
				}
			}
		} else
			noticeLang(ci->bi->nick, u, LANG_REQ_NICK_OR_MASK);
	}
	return MOD_CONT;
}
#endif

#ifdef ENABLE_KICK
int do_core_kick(User * u, Channel *c, char *target, char *reason) {
	ChannelInfo *ci = c->ci;
	User *u2;
	int is_same, exists;

	if (!target)
		target = u->nick;

	is_same = (target == u->nick) ? 1 : (stricmp(target, u->nick) == 0);

	if (is_same) {
		u2 = u;
		exists = 1;
	} else
		exists = ((u2 = finduser(target)) ? 1 : 0);

	if (!is_same ? !check_access(u, ci, CA_KICK) : !check_access(u, ci, CA_KICKME)) {
		notice_lang(ci->bi->nick, u, ACCESS_DENIED);
	} else if (!is_same && exists && (ci->flags & CI_PEACE) && (get_access(u2, ci) >= get_access(u, ci))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
	} else if (exists && ((ircd->protectedumode && is_protected(u2)) && !is_founder(u, ci))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
	} else if (exists && RestrictKB && ((!is_founder(u, ci) && is_services_oper(u2)) ||
			(is_founder(u, ci) && is_services_admin(u2)))) {
		notice_lang(ci->bi->nick, u, PERMISSION_DENIED);
	} else if (stricmp(target, ci->bi->nick) == 0) {
		bot_raw_kick(u, ci, u->nick, "Oops!");
	} else {
		if (exists) {
			if (is_on_chan(ci->c, u2)) {
				if (!reason)
					bot_raw_kick(u, ci, target, "Requested");
				else
					bot_raw_kick(u, ci, target, reason);
			}

		} else {
			char mask[BUFSIZE];
			struct c_userlist *cu = NULL, *next = NULL;

			if (my_match_wild_nocase("*!*@*", target))
				snprintf(mask, BUFSIZE, "%s", target);
			/* If we get a *@* target we need to add the *!... */
			else if (my_match_wild_nocase("*@*", target))
				snprintf(mask, BUFSIZE, "*!%s", target);
			else if (my_match_wild_nocase("*!*", target))
				snprintf(mask, BUFSIZE, "%s@*", target);
			/* If we get a * target we need to add the !*@* (assume nick)... */
			else
				snprintf(mask, BUFSIZE, "%s!*@*", target);

			cu = c->users;
			while (cu) {
				next = cu->next;
				/* This only checks against the cloacked host & vhost for normal users.
				 * IPs are only checked when triggered by an oper.. */
				if (is_oper(u) ? match_usermask_full(mask, cu->user, true) : match_usermask(mask, cu->user)) {
					/* Check whether we are allowed to kick this matching user.. */
					if (!((ircd->protectedumode && is_protected(cu->user) && !is_founder(u, ci))
							|| ((ci->flags & CI_PEACE) && (get_access(cu->user, ci) >= get_access(u, ci)))
							|| (RestrictKB && ((!is_founder(u, ci) && is_services_oper(cu->user)) ||
							(is_founder(u, ci) && is_services_admin(cu->user)))))) {
						if (!reason)
							bot_raw_kick(u, ci, cu->user->nick, "Requested");
						else
							bot_raw_kick(u, ci, cu->user->nick, reason);
					}
				}
				cu = next;
			}
		}
	}
	return MOD_CONT;
}
#endif


#ifdef ENABLE_SYNC
int do_sync(User *u, Channel *c) {
	struct c_userlist *cu, *next;
	/* Variables needed for building the cmd to remove voices.. */
	int count = 0, newac = 0;
	char modes[BUFSIZE], nicks[BUFSIZE], buf[BUFSIZE];
	char *end1, *end2;
	char **newav = scalloc(sizeof(char *) * 13, 1);
	end1 = modes;
	end2 = nicks;
	*end1 = 0;
	*end2 = 0;
	newav[newac] = c->name;
	newac++;

	if (!c->ci) {
		free(newav);
		return MOD_CONT;
	}

	if (ircdcap->tsmode) {
		snprintf(buf, BUFSIZE - 1, "%ld", (long int) time(NULL));
		newav[newac] = buf;
		newac++;
	}

	cu = c->users;
	while (cu) {
		/* We store the next user just in case current user ends up getting kicked off the channel. */
		next = cu->next;
		do_up_down(cu->user, c, "up", 0);

		/* We have to remove voices manually because the cores function doesn't care about that
		 * as it's not covered by secureops.. */
		if (chan_has_user_status(c, cu->user, CUS_VOICE) && !check_access(cu->user, c->ci, CA_AUTOVOICE)
				&& !check_access(cu->user, c->ci, CA_VOICEME) && !check_access(cu->user, c->ci, CA_VOICE)) {
			/* Add the user to the string to remove his voice.. */
			end1 += snprintf(end1, sizeof(modes) - (end1 - modes), "-v");
			end2 += snprintf(end2, sizeof(nicks) - (end2 - nicks), "%s ", GET_USER(cu->user));
			newav[newac+1] = GET_USER(cu->user);
			newac++;
			count++;

			/* Check whether the command hasn't grown too long yet.
			 * We don't allow more then 10 mode changes per command.. this should keep us safely below the 512 max length per cmd. */
			if (count == 10) {
				/* We've reached the maximum.. send the mode changes. */
				if (ircdcap->tsmode)
					newav[2] = modes;
				else
					newav[1] = modes;
				newac++;

				anope_cmd_mode(c->ci->bi->nick, c->name, "%s %s", modes, nicks);
				do_cmode(c->ci->bi->nick, newac, newav);

				/* Reset the buffer for the next set of changes.. */
				if (ircdcap->tsmode)
					newac = 2;
				else
					newac = 1;
				*end1 = 0;
				*end2 = 0;
				count = 0;
			}
		}

		cu = next;
	}

	/* If we still have some mode changes to send, do it now.. */
	if (count > 0) {
		if (ircdcap->tsmode)
			newav[2] = modes;
		else
			newav[1] = modes;
		newac++;

		anope_cmd_mode(c->ci->bi->nick, c->name, "%s %s", modes, nicks);
		do_cmode(c->ci->bi->nick, newac, newav);
	}

	noticeLang(c->ci->bi->nick, u, LANG_SYNC_DONE, c->name);

	free(newav);
	return MOD_CONT;
}
#endif

/* EOF */
