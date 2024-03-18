#include "module.h"

int NoExpireOnly = 0;

void ReadConfig();

int (*EventKick)(char *, int, char **);
int (*EventSQuit)(char *, int, char **);
int (*EventQuit)(char *, int, char **);
int (*EventKill)(char *, int, char **);

int MyKick(char *source, int ac, char **av);
int MySQuit(char *source, int ac, char **av);
int MyQuit(char *source, int ac, char **av);
int MyKill(char *source, int ac, char **av);

void (*RealPart)(char *, char *, char *);
void MyFakePart(char *nick, char *chan, char *buf);

int (*RealDrop)(User *);
int MyFakeDrop(User *u);

int do_part_channel(int argc, char **argv);
int do_bot_unassign(int argc, char **argv);
int do_chan_expire(int argc, char **argv);

int AnopeInit(int argc, char **argv)
{
	EvtHook *hook;
	Message *m;
	Command *c;

	moduleAddAuthor("Adam");
	moduleAddVersion("1.1");

	if (!s_BotServ)
	{
		alog("Error: m_permchan does not work with BotServ disabled");
		return MOD_STOP;
	}

	ReadConfig();

	RealPart = ircdproto.ircd_cmd_part;
	ircdproto.ircd_cmd_part = MyFakePart;

	m = findMessage(IRCD, "KICK");
	if (m)
	{
		EventKick = m->func;
		m->func = MyKick;
	}

	m = findMessage(IRCD, "SQUIT");
	if (m)
	{
		EventSQuit = m->func;
		m->func = MySQuit;
	}

	m = findMessage(IRCD, "QUIT");
	{
		EventQuit = m->func;
		m->func = MyQuit;
	}

	m = findMessage(IRCD, "KILL");
	if (m)
	{
		EventKill = m->func;
		m->func = MyKill;
	}

	if (UseTokens)
	{
		 m = findMessage(IRCD, "H");
		 if (m)
		 {
		 	m->func = MyKick;
		 }

		 m = findMessage(IRCD, "-");
		 if (m)
		 {
		 	m->func = MySQuit;
		 }

		 m = findMessage(IRCD, ",");
		 if (m)
		 {
			m->func = MyQuit;
		 }

		 m = findMessage(IRCD, ".");
		 if (m)
		 {
		 	m->func = MyKill;
		 }
	}

	hook = createEventHook(EVENT_PART_CHANNEL, do_part_channel);
	moduleAddEventHook(hook);
	hook = createEventHook(EVENT_BOT_UNASSIGN, do_bot_unassign);
	moduleAddEventHook(hook);
	hook = createEventHook(EVENT_CHAN_EXPIRE, do_chan_expire);
	moduleAddEventHook(hook);

	c = findCommand(CHANSERV, "DROP");
	if (c)
	{
		RealDrop = c->routine;
		c->routine = MyFakeDrop;
	}

	return MOD_CONT;
}

void AnopeFini()
{
	Channel *c, *next;
	Message *m;
	Command *cmd;

	ircdproto.ircd_cmd_part = RealPart;

	m = findMessage(IRCD, "KICK");
	if (m)
	{
		m->func = EventKick;
	}

	m = findMessage(IRCD, "SQUIT");
	if (m)
	{
		m->func = EventSQuit;
	}

	m = findMessage(IRCD, "QUIT");
	if (m)
	{
		m->func = EventQuit;
	}

	m = findMessage(IRCD, "KILL");
	if (m)
	{
		m->func = EventKill;
	}

	if (UseTokens)
	{
		m = findMessage(IRCD, "H");
		{
			m->func = EventKick;
		}

		m = findMessage(IRCD, "-");
		if (m)
		{
			m->func = EventSQuit;
		}

		m = findMessage(IRCD, ",");
		if (m)
		{
			m->func = EventQuit;
		}

		m = findMessage(IRCD, ".");
		if (m)
		{
			m->func = EventKill;
		}
	}

	cmd = findCommand(CHANSERV, "DROP");
	if (cmd)
	{
		cmd->routine = RealDrop;
	}

	for (c = firstchan(); c; c = next)
	{
		next = nextchan();

		if (c->usercount == 0 && c->ci && c->ci->bi)
		{
			anope_cmd_part(c->ci->bi->nick, c->name, NULL);
			if (c->users)
			{
				free(c->users);
				c->users = NULL;
			}
			chan_delete(c);
		}
	}
}

void ReadConfig()
{
	Directive confvalues[][1] = {
		{{"PermChanNoExpireOnly", {{PARAM_SET, PARAM_RELOAD, &NoExpireOnly}}}}
	};

	moduleGetConfigDirective(confvalues[0]);
}

int MyKick(char *source, int ac, char **av)
{
	Channel *c;

	if (ac != 3)
		return MOD_STOP;
	
	EventKick(source, ac, av);

	c = findchan(av[0]);
	if (c && c->usercount == 0 && c->users)
	{
		free(c->users);
		c->users = NULL;
	}

	return MOD_CONT;
}

int MySQuit(char *source, int ac, char **av)
{
	Channel *c;

	if (ac != 2)
		return MOD_STOP;
	
	EventSQuit(source, ac, av);

	for (c = firstchan(); c; c = nextchan())
	{
		if (c->usercount == 0 && c->users)
		{
			free(c->users);
			c->users = NULL;
		}
	}

	return MOD_CONT;
}

int MyQuit(char *source, int ac, char **av)
{
	User *u;
	int chanac = 0, i;
	char **chanav = NULL;
	struct u_chanlist *c;
	Channel *chan;

	if (ac != 1)
		return MOD_STOP;

	if ((u = finduser(source)))
	{
		for (c = u->chans; c; c = c->next)
		{
			++chanac;
			chanav = srealloc(chanav, sizeof(char *) * chanac);
			chanav[chanac - 1] = sstrdup(c->chan->name);
		}
	}
	
	EventQuit(source, ac, av);

	for (i = 0; i < chanac; ++i)
	{
		chan = findchan(chanav[i]);
		if (chan && chan->usercount == 0 && chan->users)
		{
			free(chan->users);
			chan->users = NULL;
		}

		free(chanav[i]);
	}
	if (chanac)
		free(chanav);
	
	return MOD_CONT;
}

int MyKill(char *source, int ac, char **av)
{
	User *u;
	int chanac = 0, i;
	char **chanav = NULL;
	struct u_chanlist *c;
	Channel *chan;

	if (ac != 2)
		return MOD_STOP;
	
	if ((u = finduser(av[0])))
	{
		for (c = u->chans; c; c = c->next)
		{
			++chanac;
			chanav = srealloc(chanav, sizeof(char *) * chanac);
			chanav[chanac - 1] = sstrdup(c->chan->name);
		}
	}
	
	EventKill(source, ac, av);

	for (i = 0; i < chanac; ++i)
	{
		chan = findchan(chanav[i]);
		if (chan && chan->usercount == 0 && chan->users)
		{
			free(chan->users);
			chan->users = NULL;
		}

		free(chanav[i]);
	}
	if (chanac)
		free(chanav);

	return MOD_CONT;
}

void MyFakePart(char *nick, char *chan, char *buf)
{
	Channel *c = findchan(chan);

	if (c && c->ci && c->ci->bi && !c->users && !quitmsg)
	{
		if (NoExpireOnly && !(c->ci->flags & CI_NO_EXPIRE))
		{
			RealPart(nick, chan, buf);
			return;
		}

		c->users = scalloc(sizeof(struct c_userlist), 1);
		c->users->next = NULL;
		c->users->prev = NULL;
		c->users->user = NULL;
		c->users->ud = NULL;
		c->users->ud = NULL;
	}
	else
		RealPart(nick, chan, buf);
}

int MyFakeDrop(User *u)
{
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	ChannelInfo *ci;

	if (chan)
	{
		if ((ci = cs_findchan(chan)))
		{
			if (ci->c && ci->c->usercount == 0)
			{
				if (is_services_admin(u) || (ci->flags & CI_SECUREFOUNDER ? is_real_founder(u, ci) : is_founder(u, ci)))
				{
					RealPart(ci->bi->nick, ci->name, NULL);
					chan_delete(ci->c);
				}
			}
		}
		free(chan);
	}

	RealDrop(u);

	return MOD_CONT;
}

int do_part_channel(int argc, char **argv)
{
	Channel *c;

	if (argc < 3)
		return MOD_STOP;
	
	if (!strcmp(argv[0], EVENT_STOP))
	{
		c = findchan(argv[2]);

		if (c && c->usercount == 0 && c->users)
		{
			free(c->users);
			c->users = NULL;
		}
	}

	return MOD_CONT;
}

int do_bot_unassign(int argc, char **argv)
{
	ChannelInfo *ci;

	if (argc < 2)
		return MOD_STOP;

	ci = cs_findchan(argv[0]);

	if (ci && ci->c && ci->c->usercount == 0)
	{
		RealPart(ci->bi->nick, ci->name, NULL);
		chan_delete(ci->c);
	}

	return MOD_CONT;
}

int do_chan_expire(int argc, char **argv)
{
	ChannelInfo *ci;

	if (!argc)
		return MOD_STOP;

	ci = cs_findchan(argv[0]);
	if (ci && ci->bi && ci->c && ci->c->usercount == 0)
	{
		RealPart(ci->bi->nick, ci->name, NULL);
		chan_delete(ci->c);
	}

	return MOD_CONT;
}

