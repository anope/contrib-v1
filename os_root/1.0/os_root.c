#include "module.h"

static int is_superadmin(User *u)
{
	if (u && u->isSuperAdmin)
		return 1;
	return 0;
}

int do_root(User *u);
void os_help(User *u);
int os_help_root(User *u);

int AnopeInit(int argc, char **argv)
{
	moduleAddAuthor("Adam");
	moduleAddVersion("1.0");

	Command *c = createCommand("ROOT", do_root, is_superadmin, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV, c, MOD_HEAD);
	moduleAddRootHelp(c, os_help_root);
	moduleSetOperHelp(os_help);

	return MOD_CONT;
}

void AnopeFini()
{
}

void os_help(User *u)
{
	if (is_superadmin(u))
	{
		notice_user(s_OperServ, u, "    ROOT        Control the services root list");
	}
}

int os_help_root(User *u)
{
	if (is_superadmin(u))
	{
		notice_user(s_OperServ, u, "Syntax: \2ROOT (ADD|DEL|LIST) [\037nick\037]\2");
		notice_user(s_OperServ, u, " ");
		notice_user(s_OperServ, u, "This command allows superadmins to modify who has services root access to Services.");
		notice_user(s_OperServ, u, "The access granted or denied with this command is only temporary, you need to modify");
		notice_user(s_OperServ, u, "the ServicesRoot directive in the configuration for this to be permanemnt.");
	}

	return MOD_CONT;
}

static void do_syntax_err(User *u)
{
	notice_user(s_OperServ, u, "Syntax: \2ROOT (ADD|DEL|LIST) [\037nick\037]\2");
	notice_user(s_OperServ, u, "\2/msg %s HELP ROOT\2 for more information.", s_OperServ);
}

int do_root(User *u)
{
	char *buf = moduleGetLastBuffer();
	char *cmd = myStrGetToken(buf, ' ', 0);
	char *nick = myStrGetToken(buf, ' ', 1);

	if (!cmd)
	{
		do_syntax_err(u);
	}
	else
	{
		if (!stricmp(cmd, "LIST"))
		{
			notice_user(s_OperServ, u, "Current services roots:");

			int i;
			for (i = 0; i < RootNumber; ++i)
			{
				notice_user(s_OperServ, u, "%d. %s", i + 1, ServicesRoots[i]);
			}

			notice_user(s_OperServ, u, "End of services root list.");
		}
		else if (!nick)
			do_syntax_err(u);
		else
		{
			NickAlias *na = findnick(nick);
			if (!na)
				notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, nick);
			else if (!stricmp(cmd, "ADD"))
			{
				if (na->nc->flags & NI_SERVICES_ROOT)
				{
					notice_user(s_OperServ, u, "\2%s\2 is already a services root", na->nick);
				}
				else
				{
					++RootNumber;
					ServicesRoots = srealloc(ServicesRoots, sizeof(char *) * RootNumber);
					ServicesRoots[RootNumber - 1] = sstrdup(na->nick);
					na->nc->flags |= NI_SERVICES_ROOT;

					anope_cmd_global(s_OperServ, "\2%s\2 made \2%s\2 (\2%s\2) a services root administrator", u->nick, na->nick, na->nc->display);
					alog("%s: %s!%s@%s made %s (%s) a services root administrator", s_OperServ, u->nick, u->username, u->host, na->nick, na->nc->display);

					notice_user(s_OperServ, u, "\2%s\2 added to the services root administrator list.", na->nick);
				}
			}
			else if (!stricmp(cmd, "DEL"))
			{
				if (!(na->nc->flags & NI_SERVICES_ROOT))
				{
					notice_user(s_OperServ, u, "\2%s\2 isn't already a services root", na->nick);
				}
				else
				{
					na->nc->flags &= ~NI_SERVICES_ROOT;
					int i, j, removed = 0;
					NickAlias *na2;
					for (i = 0; i < RootNumber; ++i)
					{
						for (j = 0; j < na->nc->aliases.count; ++j)
						{
							na2 = na->nc->aliases.list[j];
							if (ServicesRoots[i] && !stricmp(ServicesRoots[i], na2->nick))
							{
								free(ServicesRoots[i]);
								ServicesRoots[i] = NULL;
								++removed;
								break;
							}
						}
					}
					int RealRootNum = RootNumber - removed;;
					char **RealRoots = scalloc(RealRootNum, sizeof(char *));
					j = 0;
					for (i = 0; i < RootNumber; ++i)
					{
						if (ServicesRoots[i])
						{
							RealRoots[j] = sstrdup(ServicesRoots[i]);
							++j;
						}
					}
				
					free(ServicesRoots);
					ServicesRoots = RealRoots;
					RootNumber = RealRootNum;

					anope_cmd_global(s_OperServ, "\2%s\2 removed \2%s\2 (\2%s\2) from being a services root administrator", u->nick, na->nick, na->nc->display);
					alog("%s: %s!%s@%s made %s (%s) a services root administrator", s_OperServ, u->nick, u->username, u->host, na->nick, na->nc->display);

					notice_user(s_OperServ, u, "\2%s\2 removed from the services root administrator list.", na->nick);
				}
			}
			else
				do_syntax_err(u);
		}
	}

	if (cmd)
		free(cmd);
	if (nick)
		free(nick);
	
	return MOD_CONT;
}

