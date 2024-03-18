#include "module.h"

typedef boolean bool;

static int join2set;
static BotInfo ChanServ;

static int event_chan_registered(int argc, char **argv)
{
	ChannelInfo *ci = cs_findchan(argv[0]);

	if (ci)
	{
		ci->bi = &ChanServ;
		++ChanServ.chancount;
		
		if (ci->c)
		{
			anope_cmd_join(s_ChanServ, ci->c->name, ci->c->creation_time);
			anope_cmd_bot_chan_mode(s_ChanServ, ci->c->name);
		}
	}

	return MOD_CONT;
}

static int event_join_channel(int argc, char **argv)
{
	if (!s_BotServ && !strcmp(argv[0], EVENT_STOP))
	{
		Channel *c = findchan(argv[2]);
		if (c && c->ci && c->usercount == 1)
		{
			anope_cmd_join(s_ChanServ, c->name, c->creation_time);
			anope_cmd_bot_chan_mode(s_ChanServ, c->name);
		}
	}

	return MOD_CONT;
}

static int event_part_channel(int argc, char **argv)
{
	if (!s_BotServ && !strcmp(argv[0], EVENT_START))
	{
		Channel *c = findchan(argv[2]);

		if (c && c->ci && c->usercount == 1)
		{
			anope_cmd_part(s_ChanServ, c->name, "");
		}
	}

	return MOD_CONT;
}

static int event_db_saving(int argc, char **argv)
{
	if (s_BotServ)
	{
		if (!strcmp(argv[0], EVENT_START))
		{
			if (ChanServ.next)
				ChanServ.next->prev = ChanServ.prev;
			if (ChanServ.prev)
				ChanServ.prev->next = ChanServ.next;
			else
				botlists[tolower(*ChanServ.nick)] = ChanServ.next;

			--nbots;
		}
		else if (!strcmp(argv[0], EVENT_STOP))
		{
			insert_bot(&ChanServ);
			++nbots;
		}
	}

	return MOD_CONT;
}

static int message_privmsg(char *source, int argc, char **argv)
{
	User *u = NULL;
	Channel *c;
	bool ok = false;
	char *beginning = NULL;

	if (argc < 2)
		return MOD_STOP;

	if (ircd->ts6 && UseTS6)
		u = find_byuid(source);
	if (!u)
		u = finduser(source);
	if (!u)
		return MOD_CONT;
	
	c = findchan(argv[0]);
	if (!c || !c->ci || !c->ci->bi)
		return MOD_CONT;
	
	if ((BSFantasyCharacter ? *BSFantasyCharacter : '!') == *argv[1])
	{
		beginning = argv[1];
		++beginning;
		ok = true;
	}
	else if (!strncmp(argv[1], c->ci->bi->nick, strlen(c->ci->bi->nick)))
	{
		beginning = strchr(argv[1], ' ');
		if (beginning)
			++beginning;
		ok = true;
	}

	if (!ok || !beginning || !*beginning)
		return MOD_CONT;

	{
		char commandbuf[512];
		char *command = myStrGetToken(beginning, ' ', 0);
		char *rest = myStrGetTokenRemainder(beginning, ' ', 1);

		if (findCommand(CHANSERV, command) != NULL)
		{
			if (command && !stricmp(command, "HELP"))
				snprintf(commandbuf, sizeof(commandbuf), "%s %s", command, rest ? rest : "");
			else
				snprintf(commandbuf, sizeof(commandbuf), "%s %s %s", command, c->name, rest ? rest : "");
			if (mod_current_buffer)
				free(mod_current_buffer);
			mod_current_buffer = sstrdup(commandbuf);
			strtok(commandbuf, " ");
			mod_run_cmd(s_ChanServ, u, CHANSERV, command);
		}

		if (command)
			free(command);
		if (rest)
			free(rest);
	}

	return MOD_CONT;
}

static int command_bs_bot(User *u)
{
	char *buf = moduleGetLastBuffer();
	char *sub = myStrGetToken(buf, ' ', 0);
	char *botname = myStrGetToken(buf, ' ', 1);

	if (sub && botname && !stricmp(sub, "DEL") && !stricmp(botname, ChanServ.nick))
	{
		notice_user(s_BotServ, u, "You can NOT delete ChanServ. Instead, unload m_chanserv.");
		return MOD_STOP;
	}

	if (sub)
		free(sub);
	if (botname)
		free(botname);

	return MOD_CONT;
}

int AnopeInit(int argc, char **argv)
{
	int i;
	ChannelInfo *ci;

	moduleAddAuthor("Adam");
	moduleAddVersion("1.1");

	join2set = ircd->join2set;
	ircd->join2set = 0;

	ChanServ.nick = sstrdup(s_ChanServ);
	ChanServ.user = sstrdup(ServiceUser);
	ChanServ.host = sstrdup(ServiceHost);
	ChanServ.real = sstrdup(desc_ChanServ);
	ChanServ.flags = ChanServ.chancount = 0;
	ChanServ.created = ChanServ.lastmsg = time(NULL);
	ChanServ.next = ChanServ.prev = NULL;

	if (s_BotServ)
	{
		insert_bot(&ChanServ);
		++nbots;
	}

	moduleAddEventHook(createEventHook(EVENT_CHAN_REGISTERED, event_chan_registered));
	moduleAddEventHook(createEventHook(EVENT_JOIN_CHANNEL, event_join_channel));
	moduleAddEventHook(createEventHook(EVENT_PART_CHANNEL, event_part_channel));
	moduleAddEventHook(createEventHook(EVENT_DB_SAVING, event_db_saving));

	moduleAddMessage(createMessage("PRIVMSG", message_privmsg), MOD_HEAD);
	moduleAddMessage(createMessage("!", message_privmsg), MOD_HEAD);

	if (s_BotServ)
	{
		moduleAddCommand(BOTSERV, createCommand("BOT", command_bs_bot, NULL, -1, -1, -1, -1, -1), MOD_HEAD);
	}

	for (i = 0; i < 256; ++i)
	{
		for (ci = chanlists[i]; ci; ci = ci->next)
		{
			if (!s_BotServ || !ci->bi)
			{
				ci->bi = &ChanServ;
				++ChanServ.chancount;

				if (ci->c)
				{
					anope_cmd_join(s_ChanServ, ci->c->name, ci->c->creation_time);
					anope_cmd_bot_chan_mode(s_ChanServ, ci->c->name);
				}
			}
		}
	}

	return MOD_CONT;
}

void AnopeFini()
{
	int i;
	ChannelInfo *ci;

	ircd->join2set = join2set;

	for (i = 0; i < 256; ++i)
	{
		for (ci = chanlists[i]; ci; ci = ci->next)
		{
			if (ci->bi == &ChanServ)
			{
				ci->bi = NULL;
				--ChanServ.chancount;

				if (ci->c)
				{
					anope_cmd_part(s_ChanServ, ci->c->name, "Module unloaded!");
				}
			}
		}
	}

	if (s_BotServ)
	{
		if (ChanServ.next)
			ChanServ.next->prev = ChanServ.prev;
		if (ChanServ.prev)
			ChanServ.prev->next = ChanServ.next;
		else
			botlists[tolower(*ChanServ.nick)] = ChanServ.next;

		--nbots;
	}

	free(ChanServ.nick);
	free(ChanServ.user);
	free(ChanServ.host);
	free(ChanServ.real);
}

