#include "module.h"

#define DATABASEFILE "swhois.db"
#define AUTHOR "Adam-"
#define VERSION "2.3.0"

#define LLIST_FOREACH(list, var) for (var = list->head; var; var = var->next)

typedef struct lentry_ LEntry;
struct lentry_
{
	LEntry *next, *prev;
	void *data;
};

typedef struct llist_ LList;
struct llist_
{
	LEntry *head, *tail;
	int entries;
};

static void LListAddEntry(LList *list, void *data)
{
	LEntry *entry = smalloc(sizeof(LEntry));
	entry->data = data;
	entry->next = NULL;

	if (list->tail)
	{
		list->tail->next = entry;
		entry->prev = list->tail;
		list->tail = entry;
	}
	else
	{
		list->head = list->tail = entry;
		entry->prev = NULL;
	}

	list->entries++;
}

static void LListDelEntry(LList *list, LEntry *entry)
{
	if (entry->prev)
		entry->prev->next = entry->next;
	if (entry->next)
	 	entry->next->prev = entry->prev;
	if (list->head == entry)
		list->head = entry->next;
	if (list->tail == entry)
		list->tail = entry->prev;
	list->entries--;
}

static void LListDelData(LList *list, void *data)
{
	LEntry *entry;

	LLIST_FOREACH(list, entry)
	{
		if (entry->data == data)
		{
			LListDelEntry(list, entry);
			break;
		}
	}
}

static LList SWhois_List;

typedef struct swhois_ SWhois;
struct swhois_
{
	char *nick;
	char *swhois;
	char *creator;
	time_t created;
};

static SWhois *GetSWhois(const char *nick)
{
	LEntry *e;

	LLIST_FOREACH((&SWhois_List), e)
	{
		SWhois *s = (SWhois *)e->data;

		if (nick && s->nick && !stricmp(nick, s->nick))
		{
			return s;
		}
	}

	return NULL;
}

static int swhois_syntax(User *u);
static void os_help(User *u);
static int os_help_swhois(User *u);
static void ns_help(User *u);
static int ns_help_groupswhois(User *u);

static int do_swhois(User *u);

static int do_groupswhois(User *u);

static void swhois_add(User *u, const char *nick, const char *swhois);
static void swhois_del(User *u, const char *nick);
static void swhois_list(User *u);

static int do_update(User *u);
static int do_on_identify(int ac, char **av);
static int nick_drop(int ac, char **av);
static int nick_expire(int ac, char **av);
static int do_group(int ac, char **av);
static int do_db_save(int ac, char **av);
static int do_db_backup(int ac, char **av);

static void swhois_on(User *u);
static void DelNick(const char *nick);

static void LoadDatabase();

int AnopeInit(int argc, char **argv)
{
	Command *c;
	EvtHook *hook;

	c = createCommand("SWHOIS", do_swhois, is_services_oper, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV, c, MOD_HEAD);
	moduleAddHelp(c, os_help_swhois);
	moduleSetOperHelp(os_help);

	c = createCommand("UPDATE", do_update, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(NICKSERV, c, MOD_HEAD);

	c = createCommand("GROUPSWHOIS", do_groupswhois, nick_identified, -1, -1, -1, -1, -1);
	moduleAddCommand(NICKSERV, c, MOD_HEAD);
	moduleAddHelp(c, ns_help_groupswhois);
	moduleSetNickHelp(ns_help);

	hook = createEventHook(EVENT_NICK_IDENTIFY, do_on_identify);
	moduleAddEventHook(hook);

	hook = createEventHook(EVENT_NICK_DROPPED, nick_drop);
	moduleAddEventHook(hook);

	hook = createEventHook(EVENT_NICK_EXPIRE, nick_expire);
	moduleAddEventHook(hook);

	hook = createEventHook(EVENT_GROUP, do_group);
	moduleAddEventHook(hook);

	hook = createEventHook(EVENT_DB_SAVING, do_db_save);
	moduleAddEventHook(hook);

	hook = createEventHook(EVENT_DB_BACKUP, do_db_backup);
	moduleAddEventHook(hook);

	LoadDatabase();

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	return MOD_CONT;
}

void AnopeFini(void)
{

}

static int swhois_syntax(User *u)
{
	notice_user(s_OperServ, u, "Syntax: \x2SWHOIS {ADD | DEL | LIST} [\x1Fnick\x1F] [\x1Fswhois\x1F]\x2");
	notice_user(s_OperServ, u, "\x2/msg %s HELP SWHOIS\x2 for more information.", s_OperServ);

	return MOD_CONT;
}

static void os_help(User *u)
{
	notice_user(s_OperServ, u, "    SWHOIS    Manage swhois list");
}

static int os_help_swhois(User *u)
{
	if (is_services_oper(u))
	{
		notice_user(s_OperServ, u, "Syntax: \x2SWHOIS {ADD | DEL | LIST} [\x1Fnick\x1F] [\x1Fswhois\x1F]\x2");
		notice_user(s_OperServ, u, " ");
		notice_user(s_OperServ, u, "Allows you to assign or remove swhoises");
		notice_user(s_OperServ, u, "for a given user.");
	}

	return MOD_CONT;
}

static void ns_help(User *u)
{
	notice_user(s_NickServ, u, "    GROUPSWHOIS  Syncs all the swhoises for all nicks in a group");
}

static int ns_help_groupswhois(User *u)
{
	notice_user(s_NickServ, u, "Syntax: \2GROUPSWHOIS\2");
	notice_user(s_NickServ, u, " ");
	notice_user(s_NickServ, u, "This command allows users to set the swhois of their");
	notice_user(s_NickServ, u, "CURRENT nick to be the swhois for all nicks in the");
	notice_user(s_NickServ, u, "same group.");

	return MOD_CONT;
}

static int do_swhois(User *u)
{
	char *buf = moduleGetLastBuffer();
	char *option, *param1, *param2;

	option = myStrGetToken(buf, ' ', 0);
	param1 = myStrGetToken(buf, ' ', 1);
	param2 = myStrGetTokenRemainder(buf, ' ', 2);

	if (!option)
		swhois_syntax(u);
	else if (!stricmp(option, "ADD") && param2)
		swhois_add(u, param1, param2);
	else if (!stricmp(option, "DEL") && param1)
		swhois_del(u, param1);
	else if (!stricmp(option, "LIST"))
		swhois_list(u);
	else
		swhois_syntax(u);

	if (option)
		free(option);
	if (param1)
		free(param1);
	if (param2)
		free(param2);

	return MOD_CONT;
}

static int do_groupswhois(User *u)
{
	SWhois *s;

	if (!nick_identified(u))
		return MOD_CONT;
	
	s = GetSWhois(u->nick);
	if (s)
	{
		int i;

		for (i = 0; i < u->na->nc->aliases.count; ++i)
		{
			NickAlias *na = (NickAlias *)u->na->nc->aliases.list[i];
			SWhois *news;

			if (na == u->na)
				continue;

			if (GetSWhois(na->nick))
				DelNick(na->nick);

			news = scalloc(sizeof(SWhois), 1);
			news->nick = sstrdup(na->nick);
			news->swhois = sstrdup(s->swhois);
			news->creator = sstrdup(s->creator);
			news->created = s->created;

			LListAddEntry(&SWhois_List, news);
		}

		notice_user(s_NickServ, u, "ALL SWhoises in your group set to \2%s\2", s->swhois);
	}
	else
	{
		notice_user(s_NickServ, u, "You do not have a swhois set.");
	}

	return MOD_CONT;
}

static void swhois_add(User *u, const char *nick, const char *swhois)
{
	NickAlias *na = findnick(nick);
	
	if (!na)
	{
		notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, nick);
	}
	else if (strlen(swhois) > 280)
		notice_user(s_OperServ, u, "The swhois is too long, the max length is 280 characters.");
	else
	{
		SWhois *s;

		if (GetSWhois(na->nick))
			DelNick(na->nick);

		s = scalloc(sizeof(SWhois), 1);
		s->nick = sstrdup(na->nick);
		s->swhois = sstrdup(swhois);
		s->creator = sstrdup(u->nick);
		s->created = time(NULL);

		LListAddEntry(&SWhois_List, s);

		alog("%s: %s (%s!%s@%s) added %s to the swhois list", s_OperServ, u->nick, u->nick, u->username, u->host, nick);
		notice_user(s_OperServ, u, "\x2%s\x2 has been added to the swhois list.", na->nc->display);
	}
}

static void swhois_del(User *u, const char *nick)
{
	NickAlias *na = findnick(nick);

	if (!na)
		notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, nick);
	else if (GetSWhois(na->nick))
	{
		DelNick(na->nick);

		alog("%s: %s (%s!%s@%s) removed %s from the swhois list.", s_OperServ, u->nick, u->nick, u->username, u->host, na->nc->display);
		notice_user(s_OperServ, u, "\x2%s\x2 has been removed from the swhois list.", na->nc->display);
	}
	else
		notice_user(s_OperServ, u, "\x2%s\x2 was not found on the swhois list.", na->nc->display);
}

static void swhois_list(User *u)
{
	LEntry *e;

	if (SWhois_List.entries == 0)
	{
		notice_user(s_OperServ, u, "The swhois list is empty.");
	}
	else
	{
		int i = 0;

		notice_user(s_OperServ, u, "SWhois list:");
		notice_user(s_OperServ, u, "Num  nick  creator  created  swhois");

		LLIST_FOREACH((&SWhois_List), e)
		{
			SWhois *s = (SWhois *)e->data;
			char timebuf[64];
			struct tm tm;

			tm = *localtime(&s->created);
			strftime_lang(timebuf, sizeof(timebuf), u, STRFTIME_SHORT_DATE_FORMAT, &tm);
			
			notice_user(s_OperServ, u, "%d. %s %s %s %s", ++i, s->nick, s->creator, timebuf, s->swhois);
		}

		notice_user(s_OperServ, u, "End of SWhois list");
	}
}

static int do_update(User *u) 
{
	if (nick_identified(u))
		swhois_on(u);

	return MOD_CONT;
}

static int do_on_identify(int ac, char **av)
{
	User *u = finduser(av[0]);
	if (u)
		swhois_on(u);

	return MOD_CONT;
}

static int nick_drop(int ac, char **av)
{
	DelNick(av[0]);

	return MOD_CONT;
}

static int nick_expire(int ac, char **av)
{
	DelNick(av[0]);

	return MOD_CONT;
}

static int do_group(int ac, char **av)
{
	DelNick(av[0]);

	return MOD_CONT;
}

static int do_db_save(int ac, char **av)
{
	FILE *fd = fopen(DATABASEFILE, "w");
	LEntry *e;
	char buf[512];

	if (!fd)
	{
		return MOD_CONT;
	}

	LLIST_FOREACH((&SWhois_List), e)
	{
		SWhois *s = (SWhois *)e->data;

		snprintf(buf, sizeof(buf), "%s %s %ld %s\n", s->nick, s->creator, s->created, s->swhois);
		fputs(buf, fd);
	}

	fclose(fd);

	return MOD_CONT;
}

static int do_db_backup(int ac, char **av)
{
	if (ac && !strcmp(av[0], EVENT_START))
	{
		ModuleDatabaseBackup(DATABASEFILE);
	}

	return MOD_CONT;
}

static void swhois_on(User *u)
{
	SWhois *s;

	if (!nick_identified(u))
		return;

	s = GetSWhois(u->na->nick);
	if (s)
	{
		anope_cmd_swhois(s_NickServ, u->nick, s->swhois);
		notice_user(s_NickServ, u, "Your swhois of \x2%s %s\x2 is now activated.", u->nick, s->swhois);
	}
}

static void DelNick(const char *nick)
{
	SWhois *s = GetSWhois(nick);
	
	if (s)
	{
		LListDelData((&SWhois_List), s);

		if (s->nick)
			free(s->nick);
		if (s->swhois)
			free(s->swhois);
		if (s->creator)
			free(s->creator);
		
		free(s);
	}
}

static void LoadDatabase()
{
	FILE *f = fopen(DATABASEFILE, "r");
	char buf[512], *p;

	if (!f)
	{
		return;
	}

	while (fgets(buf, sizeof(buf), f))
	{
		SWhois *s;

		while ((p = strchr(buf, '\n')))
			*p = '\0';
		if (!*buf)
			continue;

		s = scalloc(sizeof(SWhois), 1);
		s->nick = myStrGetToken(buf, ' ', 0);
		s->creator = myStrGetToken(buf, ' ', 1);
		p = myStrGetToken(buf, ' ', 2);
		s->created = atol(p);
		free(p);
		s->swhois = myStrGetTokenRemainder(buf, ' ', 3);

		LListAddEntry((&SWhois_List), s);
	}

	fclose(f);
}

