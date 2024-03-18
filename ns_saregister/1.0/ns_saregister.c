#include "module.h"

int do_saregister(User *u);
void myNickServHelp(User *u);
NickAlias *makenick(const char *nick);
int cs_help_saregister(User *u);

int AnopeInit(int argc, char **argv)
{
	Command *c;

	c = createCommand("SAREGISTER", do_saregister, is_services_admin, -1, -1, -1, -1, -1);
	moduleAddCommand(NICKSERV, c, MOD_HEAD);
	moduleAddHelp(c, cs_help_saregister);
	moduleSetNickHelp(myNickServHelp);

	return MOD_CONT;
}

void AnopeFini()
{

}

void myNickServHelp(User *u)
{
	if (is_services_admin(u))
		notice_user(s_NickServ, u, "    SAREGISTER Register a nickname");
}

int cs_help_saregister(User *u)
{
	notice_user(s_NickServ, u, "Syntax: \2SAREGISTER \37nick\37 \37password\37 \37email\37");
	notice_user(s_NickServ, u, " ");
	notice_user(s_NickServ, u, "Allows services admins to register other nicks");
	
	return MOD_STOP;
}

int do_saregister(User *u)
{
	char *buf, *nick, *pass, *email;
	NickRequest *nr;
	NickAlias *na;

	buf = moduleGetLastBuffer();
	nick = myStrGetToken(buf, ' ', 0);
	pass = myStrGetToken(buf, ' ', 1);
	email = myStrGetToken(buf, ' ', 2);

	if (!email)
	{
		notice_user(s_NickServ, u, "Syntax: \2SAREGISTER \37nick\37 \37password\37 \37email\37");
		notice_lang(s_NickServ, u, MORE_INFO, s_NickServ, "SAREGISTER");
	}
	else if (readonly)
	{
		notice_lang(s_NickServ, u, NICK_REGISTRATION_DISABLED);
	}
	else if ((nr = findrequestnick(nick)))
	{
		notice_lang(s_NickServ, u, NICK_REQUESTED);
	}
	else if (!anope_valid_nick(nick))
	{
		notice_lang(s_NickServ, u, NICK_X_FORBIDDEN, nick);
	}
	else if ((na = findnick(nick)))
	{
		if (na->status & NS_VERBOTEN)
			notice_lang(s_NickServ, u, NICK_CANNOT_BE_REGISTERED, nick);
		else
			notice_lang(s_NickServ, u, NICK_ALREADY_REGISTERED, nick);
	}
	else if (!MailValidate(email))
	{
		notice_lang(s_NickServ, u, MAIL_X_INVALID, email);
	}
	else
	{
		na = makenick(nick);

		if (!na)
		{
			alog("%s: makenick(%s) failed", s_NickServ, u->nick);
			notice_lang(s_NickServ, u, NICK_REGISTRATION_FAILED);
		}
		else
		{
			enc_encrypt(pass, strlen(pass), na->nc->pass, PASSMAX - 1);
			na->nc->flags |= NSDefFlags;
			na->nc->memos.memomax = MSMaxMemos;
			na->last_usermask = sstrdup("*@*");
			na->last_realname = sstrdup("unknown");
			na->time_registered = na->last_seen = time(NULL);
			na->nc->language = NSDefLanguage;
			na->nc->email = sstrdup(email);

			alog("%s: %s (%s@%s) used saregister to register %s", s_NickServ, u->nick, u->username, u->host, nick);

			notice_user(s_NickServ, u, "Nick \2%s\2 has been registered", nick);
		}
	}

	if (email)
		free(email);
	if (pass)
		free(pass);
	if (nick)
		free(nick);

	return MOD_CONT;
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
    alog("%s: group %s has been created", s_NickServ, nc->display);

    /* Then make the alias */
    na = scalloc(1, sizeof(NickAlias));
    na->nick = sstrdup(nick);
    na->nc = nc;
    slist_add(&nc->aliases, na);
    alpha_insert_alias(na);
    return na;
}
