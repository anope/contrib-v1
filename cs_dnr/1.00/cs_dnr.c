/**
 * cs_dnr.c
 *
 * Adds support for wildcarded channels that can't be 
 * registered, through the use of a do-not-register list.
 *
 * Command is restricted to services opers+ and those with
 * access in the defined staff channel by default. 
 *
 * This module is based on the do-not-register commands in srvx.
 *
 * Some code is taken from srvx (http://www.srvx.net)
 *
 * Hash table headers are available from http://uthash.sourceforge.net
 * but have been packaged with the module.
 *
 *	SYNTAX: NOREGISTER [#pattern] [reason]
 *			If both a channel pattern and reason are given
 *			then the entry will be added to the DNR list.
 *			Wildcards * and ? are supported.
 *			If only a channel is given it will look for
 *			matching patterns in the DNR list.
 *			If no arguments are given then the entire DNR
 *			list will be displayed.
 *
 *	SYNTAX: ALLOWREGISTER <#pattern>
 *			Searches for, and removes if present, the 
 *			the specified pattern from the DNR list,
 *			enabling the channel to be registered.
 *
 * Staff attempting to register DNR channels will be shown
 * the list of all dnr patterns that match the channel.
 * 
 * Users attempting to register DNR channels will receive
 * a message once informing them the channel cannot be
 * registered.
 *
 * For now you'll need to remove any dnrs matching a registration
 * if you wish to allow the user to register a dnr channel (and then
 * add them back).
 *
 **/
 
#include "module.h"
#include "uthash.h"

#define AUTHOR "Katlyn"
#define VERSION "1.00"

#define STAFFCHAN "#staff"
#define STAFFAXS 1 /* Minmum access required in STAFFHCHAN to use commands */

#define DNR_LIST_HEADER "The following do-not-registers were found:"
#define DNR_LIST_ITEM "\002%s\002 is do-not-register (set %s by %s): %s"
#define DNR_LIST_COUNT "Found \002%d\002 matches."
#define DNR_BAD_CHAN "\002%s\002 is not a valid channel name."
#define DNR_VIEW_ITEM "\002%s\002 is do-not-register (set %s by %s): %s"
#define DNR_NO_MATCH "Nothing matched the criteria of your search."
#define DNR_ITEM_UPDATE "Updated entry for \002%s\002 on the do-not-register list."
#define DNR_BAD_UPDATE "Unable to update \002%s\002 on the do-not-register list - entry removed."
#define DNR_ITEM_ADDED "\002%s\002 has been added to the do-not-register list."
#define DNR_BAD_ADD "Error adding \002%s\002 to the do-not-register list."
#define DNR_NO_REG "This channel cannot be registered."
#define AR_MORE_PARAM "\002allowregister\002 requires more parameters."
#define AR_ITEM_DEL "\002%s\002 has been removed from the do-not-register list."
#define AR_BAD_ITEM "\002%s\002 is not in the do-not-register list."

int is_staff(User *u);
int valid_channel_name(const char *name);
int cs_noregister(User *u);
int cs_allowregister(User *u);
int cs_register(User *u);
int cs_oregister(User *u);
int match_ircglob(const char *text, const char *glob);

int cs_help_noregister(User *u);
int cs_help_allowregister(User *u);

int loadDNR();
int backDNR(int argc, char **argv);
int saveDNR(int argc, char **argv);

char *dnrDB = "cs_dnr.db";

struct dnr_list
{
	char pattern[33];
	char reason[200];
	char setter[33];
	time_t time;
	UT_hash_handle hh;
};

struct dnr_list *dnrl = NULL;

int add_dnr(char *dnrpattern, char *reason, char *setter, time_t added) 
{
	struct dnr_list *dnr;
	dnr = malloc(sizeof(struct dnr_list));
	strncpy(dnr->pattern, dnrpattern, 33);
	strncpy(dnr->reason, reason, 200);
	strncpy(dnr->setter, setter, 33);
	dnr->time = added;
	HASH_ADD_STR( dnrl, pattern, dnr );
	return 1;
}

struct dnr_list *find_dnr(char *match) 
{
	struct dnr_list *dnr;

	HASH_FIND_STR( dnrl, match, dnr );
	return dnr;
}

void delete_dnr(struct dnr_list *dnr) 
{
	HASH_DEL( dnrl, dnr);
	free(dnr);
}

int pattern_sort(struct dnr_list *a, struct dnr_list *b) 
{
	return strcmp(a->pattern,b->pattern);
}

void sort_by_pattern() 
{
	HASH_SORT(dnrl, pattern_sort);
}


int is_staff(User *u)
{
	ChannelInfo *ci;
	
	if (is_services_oper(u))
	{
		return 1;
	}
	else
	{
		if(ci = cs_findchan(STAFFCHAN))
		{
			if(get_access(u, ci) >= STAFFAXS)
			{
				return 1;
			}
		}
	}
	
	return 0;
}

int valid_channel_name(const char *name) 
{
	unsigned int ii;
	
	if (*name !='#')
	{
		return 0;
	}
	
	for (ii=1; name[ii]; ++ii) 
	{
		if ((name[ii] > 0) && (name[ii] <= 32))
			return 0;
		if (name[ii] == ',')
			return 0;
		if (name[ii] == '\xa0')
			return 0;
	}

	return 1;
}

int match_ircglob(const char *text, const char *glob)
{
	const char *m = glob, *n = text;
	const char *m_tmp = glob, *n_tmp = text;
	int star_p;

	for (;;) switch (*m) 
	{
		case '\0':
			if (!*n)
				return 1;
		backtrack:
			if (m_tmp == glob)
				return 0;
			m = m_tmp;
			n = ++n_tmp;
			break;
		case '\\':
			m++;
			/* allow escaping to force capitalization */
			if (*m++ != *n++)
				goto backtrack;
			break;
		case '*': case '?':
			for (star_p = 0; ; m++) 
			{
				if (*m == '*')
					star_p = 1;
				else if (*m == '?') 
				{
					if (!*n++)
					goto backtrack;
				} 
				else
					break;
			}
			
			if (star_p) 
			{
				if (!*m)
					return 1;
				else if (*m == '\\') 
				{
					m_tmp = ++m;
					if (!*m)
						return 0;
					for (n_tmp = n; *n && *n != *m; n++) ;
				} 
				else 
				{
					m_tmp = m;
					for (n_tmp = n; *n && tolower(*n) != tolower(*m); n++) ;
				}
			}
			/* and fall through */
		default:
			if (!*n)
				return *m == '\0';
			
			if (tolower(*m) != tolower(*n))
				goto backtrack;
			m++;
			n++;
			break;
	}
}


int AnopeInit(int argc, char **argv) 
{
	EvtHook *hook;
	Command *c;
	
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	
	c = createCommand("NOREGISTER", cs_noregister, is_staff, -1, -1, -1, -1, -1);
	moduleAddCommand(CHANSERV, c, MOD_HEAD);
	moduleAddHelp(c, cs_help_noregister);
	
	c = createCommand("ALLOWREGISTER", cs_allowregister, is_staff, -1, -1, -1, -1, -1);
	moduleAddCommand(CHANSERV, c, MOD_HEAD);
	moduleAddHelp(c, cs_help_allowregister);
	
	c = createCommand("REGISTER", cs_register, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(CHANSERV, c, MOD_HEAD);
	
	hook = createEventHook(EVENT_DB_SAVING, saveDNR);
	moduleAddEventHook(hook);
	
	hook = createEventHook(EVENT_DB_BACKUP, backDNR);
    moduleAddEventHook(hook);
	
	loadDNR();
	
	return MOD_CONT;
}

int cs_noregister(User *u)
{
	struct dnr_list *d;
	char timebuf[32];
	struct tm tm;
	char *channel = strtok(NULL, " ");
	char *reason = strtok(NULL, "");
	char *group = u->na->nc->display;
	int match = 0;
	
	if(!channel)
	{
		sort_by_pattern();
		
		notice_user(s_ChanServ, u, DNR_LIST_HEADER);
		
		for (d = dnrl; d != NULL; d = d->hh.next) 
		{
			tm = *localtime(&d->time);
			strftime_lang(timebuf, sizeof(timebuf), u, STRFTIME_SHORT_DATE_FORMAT, &tm);
			notice_user(s_ChanServ, u, DNR_LIST_ITEM, d->pattern, timebuf, d->setter, d->reason);
		}
		
		notice_user(s_ChanServ, u, DNR_LIST_COUNT, HASH_COUNT(dnrl));
		
		return MOD_CONT;
	}
	
	if(!valid_channel_name(channel))
	{
		notice_user(s_ChanServ, u, DNR_BAD_CHAN, channel);
		return MOD_CONT;
	}
	
	if(!reason)
	{
		notice_user(s_ChanServ, u, DNR_LIST_HEADER);
		
		for(d = dnrl; d != NULL; d = d->hh.next)
		{
			if(match_ircglob(channel, d->pattern))
			{
				tm = *localtime(&d->time);
				strftime_lang(timebuf, sizeof(timebuf), u, STRFTIME_SHORT_DATE_FORMAT, &tm);
				notice_user(s_ChanServ, u, DNR_VIEW_ITEM, d->pattern, timebuf, d->setter, d->reason);
				match++;
			}
		}
		
		if(match > 0)
		{
			notice_user(s_ChanServ, u, DNR_LIST_COUNT, match);
		}
		else
		{
			notice_user(s_ChanServ, u, DNR_NO_MATCH);
		}
		
		return MOD_CONT;
	}

	if(d = find_dnr(channel))
	{
		delete_dnr(d);
		add_dnr(channel, reason, group, time(NULL));
		notice_user(s_ChanServ, u, DNR_ITEM_UPDATE, channel);
		alog("%s: DNR '%s' updated by %s!%s@%s", s_ChanServ, channel, u->nick, u->username, u->host);
	}
	else
	{
		add_dnr(channel, reason, group, time(NULL));
		notice_user(s_ChanServ, u, DNR_ITEM_ADDED, channel);
		alog("%s: DNR '%s' added by %s!%s@%s", s_ChanServ, channel, u->nick, u->username, u->host);
	}

	return MOD_CONT;
}

int cs_register(User *u)
{
	struct dnr_list *d;
	struct tm tm;
	char *buffer;
	char timebuf[32];
	int matched_dnr = 0;
	
	buffer = moduleGetLastBuffer();
	Channel *cc;
	ChannelInfo *cinfo;
	
	if (readonly)
	{
		return MOD_CONT;
	}
	
	if (checkDefCon(DEFCON_NO_NEW_CHANNELS)) 
	{
		return MOD_CONT;
	}
	
	char *channel = myStrGetToken(buffer, ' ' , 0);
	char *password = myStrGetToken(buffer, ' ', 1);
	char *descrip = myStrGetTokenRemainder(buffer, ' ', 2);

	if(!descrip)
	{
		if(channel) free(channel);
		if(password) free(password);
		return MOD_CONT;
	}
	
	if(*channel == '&' || *channel != '#' || !anope_valid_chan(channel))
	{
		free(channel), free(password), free(descrip);
		return MOD_CONT;
	}
	
	if(!nick_recognized(u))
	{
		free(channel), free(password), free(descrip);
		return MOD_CONT;
	}
	
	if(!(cc = findchan(channel)))
	{
		free(channel), free(password), free(descrip);
		return MOD_CONT;
	}
	
	if((cinfo = cs_findchan(channel)) != NULL)
	{
		free(channel), free(password), free(descrip);
		return MOD_CONT;
	}
	
	if(!chan_has_user_status(cc, u, CUS_OP)) 
	{
		free(channel), free(password), free(descrip);
		return MOD_CONT;
	}
		
	for (d = dnrl; d != NULL; d = d->hh.next) 
	{
		if(match_ircglob(channel, d->pattern))
		{
			if(!matched_dnr)
				matched_dnr = 1;
			
			if(is_staff(u))
			{
				tm = *localtime(&d->time);
				strftime_lang(timebuf, sizeof(timebuf), u, STRFTIME_SHORT_DATE_FORMAT, &tm);
				notice_user(s_ChanServ, u, DNR_VIEW_ITEM, d->pattern, timebuf, d->setter, d->reason);
			}
			else
			{
				free(channel), free(password), free(descrip);
				notice_user(s_ChanServ, u, DNR_NO_REG, channel);
				return MOD_STOP;
			}
		}
	}
	
	if(matched_dnr)
	{
		free(channel), free(password), free(descrip);	
		return MOD_STOP;
	}
	
	free(channel), free(password), free(descrip);
	return MOD_CONT;
}

int cs_allowregister(User *u)
{
	struct dnr_list *d;
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	
	if(!chan)
	{
		notice_user(s_ChanServ, u, AR_MORE_PARAM);
		return MOD_CONT;
	}
	
	if(d = find_dnr(chan))
	{
		delete_dnr(d);
		notice_user(s_ChanServ, u, AR_ITEM_DEL, chan);
		alog("%s: DNR '%s' removed by %s!%s@%s", s_ChanServ, chan, u->nick, u->username, u->host);
		free(chan);
		
		return MOD_CONT;
	}
	
	notice_user(s_ChanServ, u, AR_BAD_ITEM, chan);
	free(chan);
	
	return MOD_CONT;
}

int loadDNR()
{
	FILE *dnr_db;
	struct dnr_list *d;
	
	char *pattern = NULL;
	char *setter = NULL;
	char *reason = NULL;
	char *timeset = NULL;
	
	char buffer[1050];
	
	if(!(dnr_db = fopen(dnrDB, "r")))
	{
		alog("[cs_dnr] Error: Unable to open DNR database (new database will be created when DNRs are next saved.");
		return MOD_CONT;
	}
	
	while (fgets(buffer, 1000, dnr_db))
	{
		pattern = myStrGetOnlyToken(buffer, ' ', 0);
		setter = myStrGetOnlyToken(buffer, ' ', 1);
		timeset = myStrGetOnlyToken(buffer, ' ', 2);
		reason = myStrGetTokenRemainder(buffer, ' ', 3);
		
		if(!reason)
		{
			alog("[cs_dnr] Error: database field missing parameters - skipping.");
			continue;
		}
		else
		{
			if(!valid_channel_name(pattern))
			{
				alog("[cs_dnr] Error: invalid channel pattern in database field - skipping.");
				free(pattern), free(setter), free(timeset), free(reason);
				continue;
			}
			
			if(d = find_dnr(pattern))
			{
				alog("[cs_dnr] Error: duplicate pattern found in database field - skipping.");
				free(pattern), free(setter), free(timeset), free(reason);
				continue;
			}
			
			if(strlen(setter) > 32)
			{
				alog("[cs_dnr] Error: invalid nickserv group in database field - skipping.");
				free(pattern), free(setter), free(timeset), free(reason);
				continue;
			}
			
			if(!(timeset[strspn(timeset, "0123456789")] == '\0'))
			{
				alog("[cs_dnr] Error: invalid timestamp in database field - skipping.");
				free(pattern), free(setter), free(timeset), free(reason);
				continue;
			}
		}
		
		add_dnr(pattern, reason, setter, atoi(timeset));
		free(pattern), free(setter), free(timeset), free(reason);
		
	}

	fclose(dnr_db);
	
	return MOD_CONT;
}

int saveDNR(int argc, char **argv)
{
	FILE *dnr_db;
	struct dnr_list *d;
	
	if (!(dnr_db = fopen(dnrDB, "w"))) 
	{
		alog("[cs_dnr] Error: unable to save to DNR database - unable to open/create db for writing");
		anope_cmd_global(s_OperServ, "\002[Error]\002 Unable to save to do-not-register database (unable to open/create db for writing)");
	}
	else
	{
		for (d = dnrl; d != NULL; d = d->hh.next) 
		{
			fprintf(dnr_db, "%s %s %lld %s\n", d->pattern, d->setter, (long long int)d->time, d->reason);
		}
	}
	
	fclose(dnr_db);

	return MOD_CONT;
}

int backDNR(int argc, char **argv)
{
	if ((argc >= 1) && (stricmp(argv[0], EVENT_START) == 0)) 
	{
		ModuleDatabaseBackup(dnrDB);
	}
	
	return MOD_CONT;
}

int cs_help_noregister(User *u)
{
	notice_user(s_ChanServ, u, "Syntax: \002NOREGISTER [#channel [reason]]");
	notice_user(s_ChanServ, u, " ");
	notice_user(s_ChanServ, u, "With no arguments, lists the current do-not-register channels.");
	notice_user(s_ChanServ, u, "With only one argument, lists any current do-not-register channels matching that channel.");
	notice_user(s_ChanServ, u, "With all arguments, adds a do-not-register channel with the specified reason. In this case, the channel name may include * or ? wildcards.");
	notice_user(s_ChanServ, u, " ");
	notice_user(s_ChanServ, u, "See also: \002ALLOWREGISTER\002");
	
	return MOD_CONT;
}

int cs_help_allowregister(User *u)
{
	notice_user(s_ChanServ, u, "Syntax: \002ALLOWREGISTER <#channel>\002");
	notice_user(s_ChanServ, u, " ");
	notice_user(s_ChanServ, u, "Removes the named channel (or channel mask) from the do-not-register list.");
	notice_user(s_ChanServ, u, " ");
	notice_user(s_ChanServ, u, "See also: \002NOREGISTER\002");
	
	return MOD_CONT;
}

/* EOF */