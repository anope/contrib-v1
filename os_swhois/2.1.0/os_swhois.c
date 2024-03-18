#include "module.h"

#define DATABASEFILE "swhois.db"
#define AUTHOR "Adam-"
#define VERSION "2.1.0"

int swhois_syntax(User *u);
void os_help(User *u);
int os_help_swhois(User *u);

int do_swhois(User *u);

int swhois_set(User *u);
int swhois_add(User *u, char *nick, char *swhois);
int swhois_del(User *u, char *nick);
int swhois_list(User *u);

int do_update(User *u);
int do_on_identify(int ac, char **av);
int nick_drop(int ac, char **av);
int nick_expire(int ac, char **av);
int do_group(int ac, char **av);

int nick_exists(NickAlias *na, char *nick);
int add_user(NickAlias *na, char *swhois);
int remove_user(NickAlias *na, char *nick);
char *get_swhois(User *u);

int AnopeInit(int argc, char **argv) {
	Command *c;
	EvtHook *hook;

	c = createCommand("SWHOIS", do_swhois, is_services_oper, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV, c, MOD_HEAD);
	moduleAddHelp(c, os_help_swhois);
	moduleSetOperHelp(os_help);

	c = createCommand("UPDATE", do_update, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(NICKSERV, c, MOD_HEAD);

	hook = createEventHook(EVENT_NICK_IDENTIFY, do_on_identify);
	moduleAddEventHook(hook);

	hook = createEventHook(EVENT_NICK_DROPPED, nick_drop);
	moduleAddEventHook(hook);

	hook = createEventHook(EVENT_NICK_EXPIRE, nick_expire);
	moduleAddEventHook(hook);

	hook = createEventHook(EVENT_GROUP, do_group);
	moduleAddEventHook(hook);

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	return MOD_CONT;
}

void AnopeFini(void) {

}

int swhois_syntax(User *u) {
	notice_user(s_OperServ, u, "Syntax: \x2SWHOIS {ADD | DEL | LIST} [\x1Fnick\x1F] [\x1Fswhois\x1F]\x2");
	notice_user(s_OperServ, u, "\x2/msg %s HELP SWHOIS\x2 for more information.", s_OperServ);
	return MOD_CONT;
}
void os_help(User *u) {
	notice_user(s_OperServ, u, "    SWHOIS    Manage swhois list");
}
int os_help_swhois(User *u) {
	if (is_services_oper(u)) {
		notice_user(s_OperServ, u, "Syntax: \x2SWHOIS {ADD | DEL | LIST} [\x1Fnick\x1F] [\x1Fswhois\x1F]\x2");
		notice_user(s_OperServ, u, " ");
		notice_user(s_OperServ, u, "Allows you to assign or remove swhoises");
		notice_user(s_OperServ, u, "for a given user.");
	}
	return 1;
}

int do_swhois(User *u) {
	char *buf = moduleGetLastBuffer();
	char *option, *param1, *param2;

	option = myStrGetToken(buf, ' ', 0);
	param1 = myStrGetToken(buf, ' ', 1);
	param2 = myStrGetTokenRemainder(buf, ' ', 2);

	if (!option)
		swhois_syntax(u);
	else if ((stricmp(option, "ADD") == 0) && (param2))
		swhois_add(u, param1, param2);
	else if ((stricmp(option, "DEL") == 0) && (param1))
			swhois_del(u, param1);
	else if (stricmp(option, "LIST") == 0)
		swhois_list(u);
	else
		swhois_syntax(u);

	if (option)
		free(option);
	if (param1)
		free(param1);
	if (param2)
		free(param2);

	return 1;
}

int swhois_set(User *u) {
	char *swhois;
	if (!nick_identified(u))
		return 1;
	else if (nick_exists(u->na, NULL)) {
		swhois = get_swhois(u);
		anope_cmd_swhois(s_NickServ, u->nick, swhois);
		notice_user(s_NickServ, u, "Your swhois of \x2%s %s\x2 is now activated.", u->nick, swhois);
		free(swhois);
	}
	return 1;
}
int swhois_add(User *u, char *nick, char *swhois) {
    NickAlias *na;
	
	if (!(na = findnick(nick))) {
		notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, nick);
	}
	else if (strlen(swhois) > 280)
		notice_user(s_OperServ, u, "The swhois is too long, the max length is 280 characters.");
	else {
		if (nick_exists(na, NULL) == 1)
			remove_user(na, NULL);
		if (add_user(na, swhois) == 1) {
			alog("%s: %s (%s!%s@%s) added %s to the swhois list",
				s_OperServ, u->nick, u->nick, u->username, u->host, nick);
			notice_user(s_OperServ, u, "\x2%s\x2 has been added to the swhois list.", na->nc->display);
			return 1;
		}
		else
			notice_user(s_OperServ, u, "An error has occured adding user %s (%s) to the swhois database",
			nick, na->nc->display);
	}
	return 0;
}
int swhois_del(User *u, char *nick) {
	NickAlias *na;

	if (!(na = findnick(nick)))
		notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, nick);
	else if (nick_exists(na, NULL) == 1) {
		remove_user(na, NULL);
		alog("%s: %s (%s!%s@%s) removed %s from the swhois list.",
			s_OperServ, u->nick, u->nick, u->username, u->host, na->nc->display);
		notice_user(s_OperServ, u, "\x2%s\x2 has been removed from the swhois list.", na->nc->display);
	}
	else
		notice_user(s_OperServ, u, "\x2%s\x2 was not found on the swhois list.", na->nc->display);
	if (nick_exists(NULL, nick))
		remove_user(NULL, nick);
	return 1;
}
int swhois_list(User *u) {
	FILE *databasefile;
	char line[312];
	char *nickfromfile;
	char *swhoisfromfile;
	int x = 1;
	
	databasefile = fopen(DATABASEFILE, "r");

	if (databasefile) {
		while (!feof(databasefile)) {
			fgets(line, sizeof(line), databasefile);
			nickfromfile = strtok(line, ":");
			swhoisfromfile = strtok(NULL, "\n");
			if ((nickfromfile) && (swhoisfromfile)) {
				if (x == 1) {
					notice_user(s_OperServ, u, "Swhois list:");
					notice_user(s_OperServ, u, "Num Nick Swhois");
				}
				notice_user(s_OperServ, u, "%d %s: %s", x, nickfromfile, swhoisfromfile);
				x++;
			}
			if (x >= 100) {
				notice_user(s_OperServ, u, "List has been stopped because it has reached 100 users.");
				break;
			}
		}
		fclose(databasefile);
	}

	if (x == 1)
		notice_user(s_OperServ, u, "The swhois list is empty.");
	else
		notice_user(s_OperServ, u, "End of \x2Swhois\x2 list.");

	return 1;
}

int do_update(User *u) {
	if (nick_identified(u))
		swhois_set(u);
	return MOD_CONT;
}
int do_on_identify(int ac, char **av) {
	User *u = finduser(av[0]);
	if (u)
		swhois_set(u);

	moduleEventDelHook(EVENT_NICK_IDENTIFY);
	return MOD_CONT;
}
int nick_drop(int ac, char **av) {
	if (nick_exists(NULL, av[0]))
		remove_user(NULL, av[0]);

	moduleEventDelHook(EVENT_NICK_DROPPED);
	return 1;
}
int nick_expire(int ac, char **av) {
	if (nick_exists(NULL, av[0]))
		remove_user(NULL, av[0]);

	moduleEventDelHook(EVENT_NICK_EXPIRE);
	return 1;
}
int do_group(int ac, char **av) {
	if (nick_exists(NULL, av[0]))
		remove_user(NULL, av[0]);

	moduleEventDelHook(EVENT_GROUP);
	return 1;
}

int nick_exists(NickAlias *na, char *nick) {
	FILE *databasefile;
	char line[312];
	char *nickfromfile, *checknick;

	databasefile = fopen(DATABASEFILE, "r");

	if (databasefile == NULL)
		return 0;
	if (na)
		checknick = na->nc->display;
	else
		checknick = nick;
	while (!feof(databasefile)) {
		memset(&line, 0, sizeof(line));
		fgets(line, sizeof(line), databasefile);
		nickfromfile = strtok(line, ":");
		if (nickfromfile != NULL) {
			if (stricmp(checknick, nickfromfile) == 0) {
				fclose(databasefile);
				return 1;
			}
		}
	}

	fclose(databasefile);

	return 0;
}
int add_user(NickAlias *na, char *swhois) {
	FILE *databasefile;
	
	databasefile = fopen(DATABASEFILE, "a");

	if (databasefile != NULL) {
		fprintf(databasefile, "%s:%s\n", na->nc->display, swhois);
		fclose(databasefile);
		return 1;
	}

	fclose(databasefile);
	return 0;
}

int remove_user(NickAlias *na, char *nick) {
	FILE *databasefile, *newdatabasefile;
	char *nickfromfile ,*swhoisfromfile, *removenick;
	char line[312];

	databasefile = fopen(DATABASEFILE, "r");
	newdatabasefile = fopen("temp.db","w");

	if ((databasefile) && (newdatabasefile)) {
		if (na)
			removenick = na->nc->display;
		else
			removenick = nick;
		while (!feof(databasefile)) {
			memset(&line, 0, sizeof(line));
			fgets(line, sizeof(line), databasefile);
			nickfromfile = strtok(line, ":");
			swhoisfromfile = strtok(NULL, "");
			if ((nickfromfile != NULL) && (swhoisfromfile != NULL)) {
				if (stricmp(removenick, nickfromfile)) {
					fputs(nickfromfile, newdatabasefile);
					fputs(":", newdatabasefile);
					fputs(swhoisfromfile, newdatabasefile);
				}
			}
		}

		fclose(databasefile);
		fclose(newdatabasefile);

#ifdef _WIN32
		remove(DATABASEFILE);
#endif

		rename("temp.db", DATABASEFILE);

		return 1;
	}

	if (databasefile)
		fclose(databasefile);
	if (newdatabasefile)
		fclose(newdatabasefile);
	return 0;
}
char *get_swhois(User *u) {
	FILE *file;
	char line[312];
	char *nickff, *swhoisff;

	file = fopen(DATABASEFILE, "r");
	if (!file)
		return NULL;
	else while (!feof(file)) {
		fgets(line, sizeof(line), file);
		nickff = strtok(line, ":");
		swhoisff = strtok(NULL, "\n");
		if ((nickff) && (swhoisff)) {
			if (!stricmp(nickff, u->na->nc->display)) {
				fclose(file);
				return sstrdup(swhoisff);
			}
		}
	}
	if (file)
		fclose(file);
	return NULL;
}
