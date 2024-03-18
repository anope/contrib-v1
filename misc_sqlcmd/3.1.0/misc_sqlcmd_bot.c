/**
 * Main routines for performing botserv operations.
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
 * Last Updated   : 16/02/2011
 *
 **/

#include "misc_sqlcmd.h"

/**
 * Assign a BotServ bot to a channel.
 *
 * @param ac int Argument count.
 * @param av Array Argument list.
 *    av[0] - Channel name.
 *    av[1] - Bot to assign.
 *    av[2] - Nick of the user assigning the bot. [Optional]
 * @param pass Password hash. [NOT USED]
 **/
int sqlcmd_handle_botassign(int ac, char **av, char *pass) {
	BotInfo *bi;
	ChannelInfo *ci;
	NickAlias *na = NULL;

	if (ac == 3)
		na = findnick(av[2]);

	if (readonly)
		return SQLCMD_ERROR_READ_ONLY;
	else if (!(ci = cs_findchan(av[0])))
		return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
	else if (ci->flags & CI_VERBOTEN)
		return SQLCMD_ERROR_CHAN_FORBIDDEN;
	else if (!(bi = findbot(av[1])))
		return SQLCMD_ERROR_BOT_NO_EXIST;
	else if ((ci->botflags & BS_NOBOT || bi->flags & BI_PRIVATE) && (!na || !na_is_services_oper(na)))
		return SQLCMD_ERROR_PERMISSION_DENIED;
	else if ((ci->bi) && (stricmp(ci->bi->nick, av[1]) == 0))
		return SQLCMD_ERROR_BOT_ALREADY_ASSIGNED;
	else {
		if (ci->bi)
			bot_unassign(ci, na ? na->nick : NULL);
		ci->bi = bi;
		bi->chancount++;
		if (ci->c && ci->c->usercount >= BSMinUsers)
			bot_join(ci);
		alog("[misc_sqlcmd] Assigned bot %s to %s (%s).", bi->nick, ci->name, na ? av[2] : "");
		send_event(EVENT_BOT_ASSIGN, 2, ci->name, bi->nick);
	}
#ifdef USE_RDB
	if (rdb_open()) {
		rdb_save_cs_info(ci);
		rdb_close();
	}
#endif
	return SQLCMD_ERROR_NONE;
}

/**
 * Unassign a BotServ bot from a channel.
 *
 * @param ac int Argument count.
 * @param av Array Argument list.
 *    av[0] - Channel name.
 *    av[1] - Nick of the user unassigning the bot. [Optional]
 * @param pass Password hash. [NOT USED]
 **/
int sqlcmd_handle_botunassign(int ac, char **av, char *pass) {
	ChannelInfo *ci;
	NickAlias *na = NULL;

	if (ac == 2)
		na = findnick(av[1]);

	if (readonly)
		return SQLCMD_ERROR_READ_ONLY;
	else if (!(ci = cs_findchan(av[0])))
		return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
	else if (ci->flags & CI_VERBOTEN)
		return SQLCMD_ERROR_CHAN_FORBIDDEN;
	else if (!ci->bi)
		return SQLCMD_ERROR_BOT_NOT_ASSIGNED;
	else
		bot_unassign(ci, na ? na->nick : NULL);
	alog("[misc_sqlcmd] Unassigned bot from %s (%s).", ci->name, na ? av[1] : "");
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
 * Make a botserv bot say something to a channel.
 *
 * @param ac int Argument count.
 * @param av Array Argument list.
 *    av[0] - Nick of the user unassigning the bot. [Optional]
 *    av[1] - Channel name.
 *    av[2] - Message to send.
 * @param pass Password hash. [NOT USED]
 **/
int sqlcmd_handle_botsay(int ac, char **av, char *pass) {
	ChannelInfo *ci;
	char *nick = NULL, *chan = NULL, *msg = NULL;

	if (ac == 3 && av[0][0] != '#') {
		nick = av[0];
		chan = av[1];
		msg = av[2];
	} else {
		chan = av[0];
		/* When given no nick, if av[2] exists, it has to be appended to av[1].. */
		if (ac == 3) {
			av[1] = srealloc(av[1], sizeof(char*) * (strlen(av[1]) + strlen(av[2]) + 2));
			strcat(av[1], " ");
			strcat(av[1], av[2]);
		}
		msg = av[1];
	}

	if (!(ci = cs_findchan(chan)))
		return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
	else if (ci->flags & CI_VERBOTEN)
		return SQLCMD_ERROR_CHAN_FORBIDDEN;
	else if (!ci->bi)
		return SQLCMD_ERROR_BOT_NOT_ASSIGNED;
	else if (!ci->c || ci->c->usercount < BSMinUsers)
		return SQLCMD_ERROR_BOT_NOT_ON_CHAN;
	else {
		strnrepl(msg, sizeof(msg), "\001", "");
		anope_cmd_privmsg(ci->bi->nick, ci->name, "%s", msg);
		ci->bi->lastmsg = time(NULL);
		if (debug)
			alog("[misc_sqlcmd] %s SAY %s %s", nick ? nick : "", ci->name, msg);
		else if (logchan && LogBot && findchan(LogChannel))
			anope_cmd_privmsg(ci->bi->nick, LogChannel, "[misc_sqlcmd] %s SAY %s %s", nick ? nick : "", ci->name, msg);
	}
	return SQLCMD_ERROR_NONE;
}


/**
 * Make a botserv bot act something in a channel.
 *
 * @param ac int Argument count.
 * @param av Array Argument list.
 *    av[0] - Nick of the user unassigning the bot. [Optional]
 *    av[1] - Channel name.
 *    av[2] - Message to send.
 * @param pass Password hash. [NOT USED]
 **/
int sqlcmd_handle_botact(int ac, char **av, char *pass) {
	ChannelInfo *ci;
	char *nick = NULL, *chan = NULL, *msg = NULL;

	if (ac == 3 && av[0][0] != '#') {
		nick = av[0];
		chan = av[1];
		msg = av[2];
	} else {
		chan = av[0];
		/* When given no nick, if av[2] exists, it has to be appended to av[1].. */
		if (ac == 3) {
			av[1] = srealloc(av[1], sizeof(char*) * (strlen(av[1]) + strlen(av[2]) + 2));
			strcat(av[1], " ");
			strcat(av[1], av[2]);
		}
		msg = av[1];
	}

	if (!(ci = cs_findchan(chan)))
		return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
	else if (ci->flags & CI_VERBOTEN)
		return SQLCMD_ERROR_CHAN_FORBIDDEN;
	else if (!ci->bi)
		return SQLCMD_ERROR_BOT_NOT_ASSIGNED;
	else if (!ci->c || ci->c->usercount < BSMinUsers)
		return SQLCMD_ERROR_BOT_NOT_ON_CHAN;
	else {
		strnrepl(msg, sizeof(msg), "\001", "");
		anope_cmd_action(ci->bi->nick, ci->name, "%s", msg);
		ci->bi->lastmsg = time(NULL);
		if (debug)
			alog("[misc_sqlcmd] %s ACT %s %s", nick ? nick : "", ci->name, msg);
		else if (logchan && LogBot && findchan(LogChannel))
			anope_cmd_privmsg(ci->bi->nick, LogChannel, "[misc_sqlcmd] %s ACT %s %s", nick ? nick : "", ci->name, msg);
	}
	return SQLCMD_ERROR_NONE;
}

/* EOF */
