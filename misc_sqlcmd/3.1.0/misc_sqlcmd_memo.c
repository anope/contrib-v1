/**
 * Main routines for performing memoserv operations.
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
 * Send a memo to a user or channel.
 *
 * @param ac int Argument count.
 * @param av Array Argument list.
 *    av[0] - Sender Nickname
 *    av[1] - Receiver name (nick or channel)
 *    av[2] - Memo Text
 * @param pass Password hash. [NOT USED]
 **/
int sqlcmd_handle_memosend(int ac, char **av, char *pass) {
	char *from, *to, *text;
	NickAlias *sender = NULL;
	Memo *m = NULL;
	MemoInfo *mi = NULL;
	int ischan, isforbid, i;

	if (ac != 3)
		return SQLCMD_ERROR_SYNTAX_ERROR;

	from = av[0];
	to = av[1];
	text = av[2];

	if (readonly)
		return SQLCMD_ERROR_READ_ONLY;
	else if (checkDefCon(DEFCON_NO_NEW_MEMOS))
		return SQLCMD_ERROR_DEFCON;
	else if (!(sender = findnick(from)))
		return SQLCMD_ERROR_NICK_NOT_REGISTERED;
	else if (!(mi = getmemoinfo(to, &ischan, &isforbid))) {
		if (isforbid)
			return ischan ? SQLCMD_ERROR_CHAN_FORBIDDEN : SQLCMD_ERROR_NICK_FORBIDDEN;
		else
			return ischan ? SQLCMD_ERROR_CHAN_NOT_REGISTERED : SQLCMD_ERROR_NICK_NOT_REGISTERED;
	} else if (mi->memomax == 0 && !na_is_services_oper(sender))
		return SQLCMD_ERROR_MEMO_X_GETS_NO_MEMOS;
	else if (mi->memocount >= 32767 || (mi->memomax > 0 && mi->memocount >= mi->memomax && !na_is_services_oper(sender)))
		return SQLCMD_ERROR_MEMO_X_HAS_TOO_MANY_MEMOS;
	else {
		mi->memocount++;
		mi->memos = srealloc(mi->memos, sizeof(Memo) * mi->memocount);
		m = &mi->memos[mi->memocount - 1];
		strscpy(m->sender, sender->nick, NICKMAX);
		m->moduleData = NULL;
		if (mi->memocount > 1) {
			m->number = m[-1].number + 1;
			if (m->number < 1) {
				for (i = 0; i < mi->memocount; i++)
						mi->memos[i].number = i + 1;
			}
		} else
			m->number = 1;

		m->time = time(NULL);
		m->text = sstrdup(text);
		m->flags = MF_UNREAD;
#ifdef USE_MYSQL
		m->id = 0;
#endif

		if (!ischan) {
			User *u = NULL;
			NickAlias *na;
			NickCore *nc = (findnick(to))->nc;

			if (MSNotifyAll) {
				if ((nc->flags & NI_MEMO_RECEIVE) && !get_ignore(to)) {
					for (i = 0; i < nc->aliases.count; i++) {
						na = nc->aliases.list[i];
						if (na->u && nick_identified(na->u))
							notice_lang(s_MemoServ, na->u, MEMO_NEW_MEMO_ARRIVED, from,
									s_MemoServ, m->number);
					}
				} else if ((u = finduser(to)) && nick_identified(u) && (nc->flags & NI_MEMO_RECEIVE))
					notice_lang(s_MemoServ, u, MEMO_NEW_MEMO_ARRIVED,
							from, s_MemoServ, m->number);
			}

			if (nc->flags & NI_MEMO_MAIL)
				new_memo_mail(nc, m);
#ifdef USE_RDB
			if (rdb_open()) {
				rdb_save_ns_core(nc);
				rdb_close();
			}
#endif
		} else {
			struct c_userlist *cu, *next;
			ChannelInfo *ci = cs_findchan(to);

			if (MSNotifyAll && ci->c) {
				for (cu = ci->c->users; cu; cu = next) {
					next = cu->next;
					if (check_access(cu->user, ci, CA_MEMO)) {
						if (cu->user->na && (cu->user->na->nc->flags & NI_MEMO_RECEIVE)
								&& !get_ignore(cu->user->nick)) {
							notice_lang(s_MemoServ, cu->user, MEMO_NEW_X_MEMO_ARRIVED,
									ci->name, s_MemoServ, ci->name, m->number);
						}
					}
				}
			}
#ifdef USE_RDB
			if (rdb_open()) {
				rdb_save_cs_info(ci);
				rdb_close();
			}
#endif
		}
		return SQLCMD_ERROR_NONE;
	}
}

/* ------------------------------------------------------------------------------- */

/**
 * Delete a memo/set of memos.
 *
 * @param ac int Argument count.
 * @param av Array Argument list.
 *    av[0] - Nickname or Channel.
 *    av[1] - Memo/list of memos.
 * @param pass Password hash. [NOT USED]
 **/
int sqlcmd_handle_memodel(int ac, char **av, char *pass) { 
	char *numstr, buf[BUFSIZE], *end;
	NickAlias *na = NULL;
	ChannelInfo *ci = NULL;
	MemoInfo *mi;
	int count = 0, left = 0, last = -1, last0 = -1;

	if (ac < 2)
		return SQLCMD_ERROR_SYNTAX_ERROR;

	if (av[0][0] == '#') {
		if (!(ci = cs_findchan(av[0])))
			return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
		else if (ci->flags & CI_VERBOTEN)
			return SQLCMD_ERROR_CHAN_FORBIDDEN;
		mi = &ci->memos;
	} else {
		if (!(na = findnick(av[0])))
			return SQLCMD_ERROR_NICK_NOT_REGISTERED;
		else if (na->status & NS_VERBOTEN)
			return SQLCMD_ERROR_NICK_FORBIDDEN;
		mi = &na->nc->memos;
	}
	numstr = av[1];

	if (!isdigit(*numstr))
		return SQLCMD_ERROR_SYNTAX_ERROR;
	else if (!mi || mi->memocount == 0)
		return SQLCMD_ERROR_NO_MEMOS;

	end = buf;
	left = sizeof(buf);
	process_numlist(numstr, &count, del_memo_callback, NULL, mi,
			&last, &last0, &end, &left);

	if (last == -1 && count == 1)
		return SQLCMD_ERROR_MEMO_DOES_NOT_EXIST;
	else if (last == -1)
		return SQLCMD_ERROR_MEMO_DELETED_NONE;

#ifdef USE_RDB
	if (rdb_open()) {
		if (ci)
			rdb_save_cs_info(ci);
		if (na)
			rdb_save_ns_core(na->nc);
		rdb_close();
	}
#endif

	return SQLCMD_ERROR_NONE;
}

/**
 * Delete a single memo from a MemoInfo. Callback function.
 *
 * @param u User Struct
 * @param int Number
 * @param va_list Variable Arguemtns
 * @return 1 if successful, 0 if it fails
 */
int del_memo_callback(User *u, int num, va_list args) {
	MemoInfo *mi = va_arg(args, MemoInfo *);
	int *last = va_arg(args, int *);
	int *last0 = va_arg(args, int *);
	char **end = va_arg(args, char **);
	int *left = va_arg(args, int *);

	if (delmemo(mi, num)) {
		if (num != (*last) + 1) {
			if (*last != -1) {
				int len;
				if (*last0 != *last)
					len = snprintf(*end, *left, ",%d-%d", *last0, *last);
				else
					len = snprintf(*end, *left, ",%d", *last);
				*end += len;
				*left -= len;
			}
			*last0 = num;
		}
		*last = num;
		return 1;
	} else {
		return 0;
	}
}

/* ------------------------------------------------------------------------------- */

/**
 * Delete all memos.
 *
 * @param ac int Argument count.
 * @param av Array Argument list.
 *    av[0] - Nickname or Channel.
 * @param pass Password hash. [NOT USED]
 **/
int sqlcmd_handle_memoclear(int ac, char **av, char *pass) {
	NickAlias *na = NULL;
	ChannelInfo *ci = NULL;
	MemoInfo *mi;
	int i;

	if (ac != 1)
		return SQLCMD_ERROR_SYNTAX_ERROR;

	if (av[0][0] == '#') {
		if (!(ci = cs_findchan(av[0])))
			return SQLCMD_ERROR_CHAN_NOT_REGISTERED;
		else if (ci->flags & CI_VERBOTEN)
			return SQLCMD_ERROR_CHAN_FORBIDDEN;
		mi = &ci->memos;
	} else {
		if (!(na = findnick(av[0])))
			return SQLCMD_ERROR_NICK_NOT_REGISTERED;
		else if (na->status & NS_VERBOTEN)
			return SQLCMD_ERROR_NICK_FORBIDDEN;
		mi = &na->nc->memos;
	}

	if (!mi || mi->memocount == 0)
		return SQLCMD_ERROR_NO_MEMOS;

	for (i = 0; i < mi->memocount; i++) {
		free(mi->memos[i].text);
		moduleCleanStruct(&mi->memos[i].moduleData);
	}
	free(mi->memos);
	mi->memos = NULL;
	mi->memocount = 0;

#ifdef USE_RDB
	if (rdb_open()) {
		if (ci)
			rdb_save_cs_info(ci);
		if (na)
			rdb_save_ns_core(na->nc);
		rdb_close();
	}
#endif

	return SQLCMD_ERROR_NONE;
}

/* EOF */
