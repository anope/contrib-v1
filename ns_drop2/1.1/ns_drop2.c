#include "module.h"

#define AUTHOR "Adam-"
#define VERSION "1.1"

void do_syntax(User *u);
int ns_help_drop(User *u);
int do_drop(User *u);

int AnopeInit(int argc, char **argv) {
	Command *c;

	c = createCommand("DROP", do_drop, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(NICKSERV, c, MOD_HEAD);
	moduleAddHelp(c, ns_help_drop);

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	return MOD_CONT;
}

void AnopeFini(void) {

}

void do_syntax(User *u) {
	notice_user(s_NickServ, u, "Syntax: \2DROP \037nickname\037 \037password\037\2");
	notice_user(s_NickServ, u, "\2/msg %s HELP DROP\2 for more information.", s_NickServ);
}

int ns_help_drop(User *u) {
	notice_user(s_NickServ, u, "Syntax: \2DROP \037nickname\037 \037password\037\2");
	notice_user(s_NickServ, u, " ");
	notice_user(s_NickServ, u, "Drops the named nick from the database.");
	notice_user(s_NickServ, u, "You may drop any nick within your group without any");
	notice_user(s_NickServ, u, "special privileges.");
	if (is_services_admin(u)) {
		notice_user(s_NickServ, u, " ");
		notice_user(s_NickServ, u, "\2Services admins\2 can drop any nick without");
		notice_user(s_NickServ, u, "knowing the password.");
	}
	return 1;
}

int do_drop(User *u) {
	char *buf = moduleGetLastBuffer();
	char *nick, *pass;
	NickAlias *na;
	int is_mine;
	int correct_pass;

	if (is_services_admin(u))
		return MOD_CONT;

	nick = myStrGetToken(buf, ' ', 0);
	pass = myStrGetToken(buf, ' ', 1);

	if (!pass) {
		do_syntax(u);
		if (nick)
			free(nick);
		return MOD_STOP;
	}

	na = findnick(nick);
	if (na) {
		is_mine = (u->na && u->na->nc == na->nc);
		correct_pass = enc_check_password(pass, na->nc->pass);
		free(nick);
		free(pass);
		if (is_mine) {
			if (correct_pass)
				return MOD_CONT;
			else {
				notice_lang(s_NickServ, u, PASSWORD_INCORRECT);
				bad_password(u);
				return MOD_STOP;
			}
		}
	}
	else {
		free(nick);
		free(pass);
	}
	return MOD_CONT;
}
