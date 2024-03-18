#include "module.h"

#define LIST_FOREACH(list, var) for (var = (list)->head; var; var = var->next)
#define LIST_FOREACH_SAFE(list, var1, var2) for (var1 = (list)->head, var2 = var1 ? var1->next : NULL; var1; var1 = var2, var2 = var1 ? var1->next : NULL)

typedef boolean bool;

static void *my_malloc(size_t sz)
{
	return scalloc(1, sz);
}

struct r_node
{
	struct r_node *next, *prev;
	void *data;
};

struct r_list
{
	struct r_node *head, *tail;
	size_t count;
	void (*free)(void *data);
};

static void r_list_add_r_node(struct r_list *list, struct r_node *r_node, void *data)
{
	r_node->data = data;

	if (list->tail)
	{
		list->tail->next = r_node;
		r_node->prev = list->tail;
		r_node->next = NULL;
		list->tail = r_node;
	}
	else
	{
		list->head = list->tail = r_node;
		r_node->prev = r_node->next = NULL;
	}

	list->count++;
}

static struct r_node *r_list_add(struct r_list *list, void *data)
{
	struct r_node *r_node;

	r_node = my_malloc(sizeof(struct r_node));
	r_list_add_r_node(list, r_node, data);
	return r_node;
}

static void *r_list_del_r_node(struct r_list *list, struct r_node *r_node)
{
	void *data = NULL;

	if (r_node->prev)
		r_node->prev->next = r_node->next;
	if (r_node->next)
		r_node->next->prev = r_node->prev;
	if (list->head == r_node)
		list->head = r_node->next;
	if (list->tail == r_node)
		list->tail = r_node->prev;
	
	list->count--;

	if (list->free)
		list->free(r_node->data);
	else
		data = r_node->data;
	
	r_node->next = r_node->prev = NULL;
	r_node->data = NULL;

	return data;
}

bool r_list_has_data(struct r_list *list, const void *data)
{
	struct r_node *n;

	LIST_FOREACH(list, n)
	{
		if (data == n->data)
		{
			return true;
		}
	}

	return false;
}

void r_list_clear(struct r_list *list)
{
	struct r_node *n, *tn;

	LIST_FOREACH_SAFE(list, n, tn)
	{
		r_list_del_r_node(list, n);
		free(n);
	}
}

/**************************************************/

static int action;
enum
{
	ACTION_WARN,
	ACTION_AKILL
};

static struct r_list restrict_list;

struct email_restrict
{
	char email[64];
	char creator[NICKMAX];
	time_t created;
	time_t expires;
};

static struct email_restrict *find_restrict_for(const char *mail)
{
	struct r_node *entry, *entry2;
	time_t t = time(NULL);

	LIST_FOREACH_SAFE(&restrict_list, entry, entry2)
	{
		struct email_restrict *r = (struct email_restrict *) entry->data;

		if (r->expires && t > r->expires)
		{
			alog("%s: Expiring RESTRICTEMAIL for %s", s_OperServ, r->email);
			r_list_del_r_node(&restrict_list, entry);
			free(entry);
			continue;
		}

		if (match_wild_nocase(r->email, mail))
			return r;
	}

	return NULL;
}

static void take_action(User *u, bool new)
{
	int a = new == false ? ACTION_WARN : action;

	if (!u)
		return;

	switch (a)
	{
		case ACTION_WARN:
			notice_user(s_NickServ, u, "Your email has been detected as a throwaway account and may lead to your nick being dropped. Please change to a different one.");
			break;
		case ACTION_AKILL:
		{
			char mask[512];

			if (!nick_identified(u))
				return;

			snprintf(mask, sizeof(mask), "*@%s", u->host);

			add_akill(NULL, mask, s_OperServ, time(NULL) + 86400, "Disallowed email address");

			anope_cmd_global(s_OperServ, "%s added an AKILL for %s (%s)", s_OperServ, mask, "Disallowed email address");

			break;
		}
	}
}

static void do_syntax_error(User *u)
{
	notice_user(s_OperServ, u, "Syntax: \2RESTRICTEMAIL {ADD|DEL|LIST|CLEAR} [\037+expiry\037] [\037email@address\037]");
	notice_user(s_OperServ, u, "\2/msg %s HELP RESTRICTEMAIL\2 for more information.", s_OperServ);
}

static int do_restrictemail(User *u)
{
	char *buf = moduleGetLastBuffer();
	char *cmd = myStrGetToken(buf, ' ', 0);
	char *expiry = myStrGetToken(buf, ' ', 1);
	char *mask = myStrGetToken(buf, ' ', 2);
	time_t expires = 0, t = time(NULL);

	if (!cmd)
		do_syntax_error(u);
	else if (!stricmp(cmd, "ADD") && expiry)
	{
		if (*expiry != '+')
		{
			if (mask)
				free(mask);
			mask = expiry;
			expiry = NULL;
		}
		else
		{
			expires = (t + dotime(expiry));
			if (expires == t)
				expires = 0;
		}

		if (!mask)
			do_syntax_error(u);
		else
		{
			if (strspn(mask, "~@.*?") == strlen(mask))
				notice_lang(s_OperServ, u, MAIL_X_INVALID, mask);
			else
			{
				struct r_node *entry;
				struct email_restrict *r;
				int cont = 1;

				LIST_FOREACH(&restrict_list, entry)
				{
					r = (struct email_restrict *) entry->data;

					if (match_wild_nocase(r->email, mask))
					{
						if (!stricmp(r->email, mask))
						{
							r->expires = expires;
							notice_user(s_OperServ, u, "%s is already on the restrict email list, changing expiry time to %s", r->email, expiry ? expiry : "never");
						}
						else
						{
							notice_user(s_OperServ, u, "%s is already covered by %s", mask, r->email);
						}

						cont = 0;
						break;
					}
				}

				if (cont)
				{
					r = my_malloc(sizeof(struct email_restrict));
					strncpy(r->creator, u->nick, sizeof(r->creator));
					strncpy(r->email, mask, sizeof(r->email));
					r->created = t;
					r->expires = expires;
					r_list_add(&restrict_list, r);
					alog("%s: %s!%s@%s added RESTRICTEMAIL for %s", s_OperServ, u->nick, u->username, u->host, mask);
					notice_user(s_OperServ, u, "Added email restriction for \2%s\2", mask);
				}
			}
		}
	}
	else if (!stricmp(cmd, "DEL") && expiry)
	{
		if (mask)
			free(mask);
		mask = expiry;
		expiry = NULL;
		int deleted = 0;

		struct r_node *entry;
		LIST_FOREACH(&restrict_list, entry)
		{
			struct email_restrict *r = (struct email_restrict *)entry->data;

			if (!stricmp(r->email, mask))
			{
				r_list_del_r_node(&restrict_list, entry);
				free(entry);
				deleted = 1;
				break;
			}
		}

		if (deleted)
		{
			alog("%s: %s!%s@%s removed RESTRICTEMAIL on %s", s_OperServ, u->nick, u->username, u->host, mask);
			notice_user(s_OperServ, u, "Removed email restriction from \2%s\2", mask);
		}
		else
		{
			notice_user(s_OperServ, u, "Unable to find \2%s\2 on the restrict list.", mask);
		}
	}
	else if (!stricmp(cmd, "LIST"))
	{
		if (restrict_list.count == 0)
		{
			notice_user(s_OperServ, u, "The restrict list is empty");
		}
		else
		{
			notice_user(s_OperServ, u, "Email restrict list:");
			notice_user(s_OperServ, u, "%-3s %-20s %-15s %-15s %-15s", "Num", "Mask", "Creator", "Created", "Expires");
			struct r_node *entry, *entry2;
			int i = 0;
			char created[64], expires[64];
			struct tm tm;
			time_t t = time(NULL);

			LIST_FOREACH_SAFE(&restrict_list, entry, entry2)
			{
				struct email_restrict *r = (struct email_restrict *) entry->data;

				if (r->expires && t > r->expires)
				{
					alog("%s: Expiring RESTRICTEMAIL for %s", s_OperServ, r->email);
					r_list_del_r_node(&restrict_list, entry);
					free(entry);
					continue;
				}

				tm = *localtime(&r->created);
				strftime_lang(created, sizeof(created), u, STRFTIME_SHORT_DATE_FORMAT, &tm);

				if (r->expires)
				{
					tm = *localtime(&r->expires);
					strftime_lang(expires, sizeof(expires), u, STRFTIME_SHORT_DATE_FORMAT, &tm);
				}
				else
				{
					snprintf(expires, sizeof(expires), "Never");
				}

				notice_user(s_OperServ, u, "%-3d %-20s %-15s %-15s %-15s", ++i, r->email, r->creator, created, expires);
			}
			notice_user(s_OperServ, u, "End of restrict list.");
		}
	}
	else if (!stricmp(cmd, "CLEAR"))
	{
		r_list_clear(&restrict_list);
		alog("%s: %s!%s@%s cleared the RESTRICTEMAIL list", s_OperServ, u->nick, u->username, u->host);
		notice_user(s_OperServ, u, "Email restrict list cleared.");
	}
	else
		do_syntax_error(u);

	if (cmd)
		free(cmd);
	if (expiry)
		free(expiry);
	if (mask)
		free(mask);
	
	return MOD_CONT;
}

static int do_help(User *u)
{
	notice_user(s_OperServ, u, "Syntax: \2RESTRICTEMAIL {ADD|DEL|LIST|CLEAR} [\037+expiry\037] [\037email@address\037]");
	notice_user(s_OperServ, u, " ");
	notice_user(s_OperServ, u, "This command allows you to prevent people from registering with an email matching");
	notice_user(s_OperServ, u, "email@address (wildcards allowed).");

	return MOD_CONT;
}

static void do_oper_listhelp(User *u)
{
	if (is_services_admin(u))
		notice_user(s_OperServ, u, "    RESTRICTEMAIL   Control the restricted email addresses");
}

static int do_register(User *u)
{
	char *buf = moduleGetLastBuffer();
	int ret = MOD_CONT;

	char *email = myStrGetToken(buf, ' ', 1);

	if (email)
	{
		struct email_restrict *r = find_restrict_for(email);

		if (r && !is_oper(u))
		{
			alog("%s: %s!%s@%s tried to register with disallowed email address %s.", s_NickServ, u->nick, u->username, u->host, email);
			take_action(u, true);
			ret = MOD_STOP;
		}

		free(email);
	}
	
	return ret;
}

static int do_set(User *u)
{
	char *buf = moduleGetLastBuffer();
	int ret = MOD_CONT;

	char *what = myStrGetToken(buf, ' ', 0);
	char *email = myStrGetToken(buf, ' ', 1);

	if (what && !stricmp(what, "EMAIL") && email && !is_oper(u))
	{
		struct email_restrict *r = find_restrict_for(email);

		if (r)
		{
			alog("%s: %s!%s@%s tried to change their email to disallowed email address %s.", s_NickServ, u->nick, u->username, u->host, email);
			take_action(u, false);
			ret = MOD_STOP;
		}
	}

	if (what)
		free(what);
	if (email)
		free(email);

	return ret;
}

static void db_load()
{
	FILE *fd = fopen("restrictemail.db", "r");
	if (!fd)
	{
		alog("Unable to open restrictemail.db for reading!");
		return;
	}

	struct email_restrict *r;
	char buffer[512], *p;
	while (!feof(fd) && fgets(buffer, sizeof(buffer), fd))
	{
		while ((p = strchr(buffer, '\n')))
			*p = 0;

		r = my_malloc(sizeof(struct email_restrict));

		p = myStrGetToken(buffer, ' ', 0);
		if (p)
		{
			strncpy(r->email, p, sizeof(r->email));
			free(p);
		}
		else
			strncpy(r->email, "unknown", sizeof(r->email));

		p = myStrGetToken(buffer, ' ', 1);
		if (p)
		{
			strncpy(r->creator, p, sizeof(r->creator));
			free(p);
		}
		else
			strncpy(r->creator, "unknown", sizeof(r->creator));

		p = myStrGetToken(buffer, ' ', 2);
		if (p)
		{
			r->created = atol(p);
			free(p);
		}
		else
			r->created = time(NULL);

		p = myStrGetToken(buffer, ' ', 3);
		if (p)
		{
			r->expires = atol(p);
			free(p);
		}
		else
			r->expires = time(NULL);

		r_list_add(&restrict_list, r);
	}

	fclose(fd);
}

static int db_save(int argc, char **argv)
{
	if (!stricmp(argv[0], EVENT_START))
	{
		struct r_node *entry;
		struct email_restrict *r;

		FILE *fd = fopen("restrictemail.db", "w");
		if (!fd)
		{
			alog("Unable to open restrictemail.db for writing!");
			return MOD_CONT;
		}

		LIST_FOREACH(&restrict_list, entry)
		{
			r = (struct email_restrict *)entry->data;

			fprintf(fd, "%s %s %ld %ld\n", r->email, r->creator, r->created, r->expires);
		}

		fclose(fd);
	}

	return MOD_CONT;
}

static int do_id(int argc, char **argv)
{
	User *u = finduser(argv[0]);
	if (u && !is_oper(u) && nick_identified(u) && u->na && u->na->nc && u->na->nc->email)
	{
		struct email_restrict *r = find_restrict_for(u->na->nc->email);
		if (r)
		{
			alog("%s: %s!%s@%s identified for %s which has a disallowed email address %s.", s_NickServ, u->nick, u->username, u->host, u->na->nc->display, u->na->nc->email);
			take_action(u, false);
		}
	}

	return MOD_CONT;
}

static int do_reload(int argc, char **argv)
{
	Directive confvalue = { "RestrictEmailAction", {{ PARAM_INT, PARAM_RELOAD, &action }} };

	action = ACTION_WARN;
	moduleGetConfigDirective(&confvalue);

	return MOD_CONT;
}

int AnopeInit(int argc, char **argv)
{
	Command *c;
	EvtHook *hook;
	
	moduleAddAuthor("Adam");
	moduleAddVersion("1.0");

	restrict_list.free = free;
	
	c = createCommand("RESTRICTEMAIL", do_restrictemail, is_services_admin, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV, c, MOD_HEAD);
	moduleAddAdminHelp(c, do_help);
	moduleSetOperHelp(do_oper_listhelp);

	c = createCommand("REGISTER", do_register, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(NICKSERV, c, MOD_HEAD);

	c = createCommand("SET", do_set, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(NICKSERV, c, MOD_HEAD);

	hook = createEventHook(EVENT_DB_SAVING, db_save);
	moduleAddEventHook(hook);
	hook = createEventHook(EVENT_NICK_IDENTIFY, do_id);
	moduleAddEventHook(hook);
	hook = createEventHook(EVENT_RELOAD, do_reload);
	moduleAddEventHook(hook);

	do_reload(0, NULL);
	db_load();

	return MOD_CONT;
}

void AnopeFini()
{
	char *event = EVENT_START;
	db_save(1, &event);
	r_list_clear(&restrict_list);
}


