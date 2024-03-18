/**
 * Utility functions specific to this module.
 * Many core functions for example want a User*, alternatives may be needed that accept a NickAlias*.
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

/*************************************************************************/

/**
 * Check if the user has SRA privileges.
 *
 * @param na NickAlias Nick to check.
 * @return True/false.
 **/
int na_is_services_root(NickAlias *na) {
	if (na && na->nc->flags & NI_SERVICES_ROOT)
		return 1;
	return 0;
}

/**
 * Check if the user has SA privileges.
 *
 * @param na NickAlias Nick to check.
 * @return True/false.
 **/
int na_is_services_admin(NickAlias *na) {
	if (na && na->nc->flags & (NI_SERVICES_ADMIN | NI_SERVICES_ROOT))
		return 1;
	return 0;
}

/**
 * Check if the user has SO privileges.
 *
 * @param na NickAlias Nick to check.
 * @return True/false.
 **/
int na_is_services_oper(NickAlias *na) {
	if (na && na->nc->flags & (NI_SERVICES_OPER | NI_SERVICES_ADMIN | NI_SERVICES_ROOT))
		return 1;
	return 0;
}

/*************************************************************************/

/**
 * Unassign a bot from a channel.
 **/
void bot_unassign(ChannelInfo *ci, char *user) {
	send_event(EVENT_BOT_UNASSIGN, 2, ci->name, ci->bi->nick);
	if (ci->c && ci->c->usercount >= BSMinUsers) {
		if (user)
			anope_cmd_part(ci->bi->nick, ci->name, "UNASSIGN received from %s via SQL Command.", user);
		else
			anope_cmd_part(ci->bi->nick, ci->name, "UNASSIGN received via SQL Command.");
	}
	ci->bi->chancount--;
	ci->bi = NULL;
}

/*************************************************************************/

/**
 * Produce a checksum from the needed values.
 * This function uses MySQL's MD5() so requires an open DB connection.
 **/
char *sqlcmd_checksum(char *cmd, char *param_str, char *ts) {
	char *chksum_sql = NULL, *quote_param = NULL, *chksum = NULL;
	int checksum_len = 0;

	checksum_len = strlen(cmd) + strlen(param_str) + strlen(ts) + strlen(sqlcmd_chksum_salt) + 20;
	chksum_sql = smalloc(sizeof(char) * checksum_len);

	quote_param = sqlcmd_mysql_quote(param_str);
	snprintf(chksum_sql, checksum_len - 1, "SELECT MD5('%s:%s:%s:%s')", cmd, quote_param, ts, sqlcmd_chksum_salt);
	free(quote_param);

	if (sqlcmd_mysql_query(chksum_sql, 2) != 1)
		return NULL;
	if (mysql_num_rows(chksum_res) != 1)
		return NULL;

	chksum_row = mysql_fetch_row(chksum_res);
	chksum = sstrdup(chksum_row[0]);
	mysql_free_result(chksum_res);
	return chksum;
}

/*************************************************************************/

/**
 * Turn a space-deliminated string into a char array.
 **/
int str_to_params(char *params, int ac, char **param_array) {
	char *tmp_tok = NULL;
	char delim = ' ';
	int i ;

	if (!ac)
		return 0;

	for (i = 0; ((i < ac - 1) && (tmp_tok = myStrGetToken(params, delim, i))); i++)
		param_array[i] = tmp_tok;
	if ((param_array[i] = myStrGetTokenRemainder(params, delim, i)))
		i++;
	return i;
}

/*************************************************************************/

NickRequest *makerequest(const char *nick) {
	NickRequest *nr;
	nr = scalloc(1, sizeof(NickRequest));
	nr->nick = sstrdup(nick);
	insert_requestnick(nr);
	alog("[misc_sqlcmd] Nick %s has been requested.", nr->nick);
	return nr;
}

NickAlias *makenick(const char *nick) {
	NickCore *nc;

	nc = scalloc(1, sizeof(NickCore));
	nc->display = sstrdup(nick);
	slist_init(&nc->aliases);
	insert_core(nc);
	alog("[misc_sqlcmd] Group %s has been created.", nc->display);
	return makealias(nick, nc);
}

NickAlias *makealias(const char *nick, NickCore *nc) {
	NickAlias *na;

	na = scalloc(1, sizeof(NickAlias));
	na->nick = sstrdup(nick);
	na->nc = nc;
	slist_add(&nc->aliases, na);
	alpha_insert_alias(na);
	alog("[misc_sqlcmd] Nick %s has been added to nickname group %s.", nick, nc->display);
	return na;
}

/*************************************************************************/

void new_memo_mail(NickCore * nc, Memo * m) {
	MailInfo *mail = NULL;

	if (!nc || !m)
		return;

	mail = MailMemoBegin(nc);
	if (!mail)
		return;

	fprintf(mail->pipe, getstring2(NULL, MEMO_MAIL_TEXT1), nc->display);
	fprintf(mail->pipe, "\n");
	fprintf(mail->pipe, getstring2(NULL, MEMO_MAIL_TEXT2), m->sender,
			m->number);
	fprintf(mail->pipe, "\n\n");
	fprintf(mail->pipe, "%s", getstring2(NULL, MEMO_MAIL_TEXT3));
	fprintf(mail->pipe, "\n\n");
	fprintf(mail->pipe, "%s", m->text);
	fprintf(mail->pipe, "\n");
	MailEnd(mail);
	return;
}

/*************************************************************************/

/* EOF */
