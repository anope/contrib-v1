/*#########################################################################
#  cs_migrate
#  Provides commands to 'migrate' channel settings from one channel to
#  another.
#  
#  Author    : mark / DNS
#  Date      : 10/05/2009
#  Version   : 1.0.11
#  Works on  : Anope 1.7.9 or greater
#              Anope 1.8
###########################################################################
#  Changelog
#    1.0.1
#      Initial release
#    1.0.11
#      Added Command to ChanServ Help List and a German Translation
#########################################################################*/

#include "module.h"

#define AUTHOR "mark / DNS"
#define VERSION "1.0.11"

#define LANG_NUM_STRINGS		4
#define LANG_MIGRATE_DESC		0
#define LANG_MIGRATE_SYNTAX		1
#define LANG_MIGRATE_SYNTAX_EX	2
#define LANG_MIGRATE_SUCCESS	3

void do_help_list(User *u);

void do_help_list(User *u) {
	moduleNoticeLang(s_ChanServ, u, LANG_MIGRATE_DESC);
}

char *langtable_en_us[] =
{
	/* LANG_MIGRATE_DESC */
        "    MIGRATE  Copies settings from a channel to another one",
	/* LANG_MIGRATE_SYNTAX */
	"Syntax: MIGRATE sourcechan destchan",
	/* LANG_MIGRATE_SYNTAX_EX */
	"Syntax: MIGRATE sourcechan destchan\n\n"
	"Copies the settings from sourcechan to destchan. This includes\n"
	"SET options, access lists, autokick lists, badwords lists, but\n"
	"excludes founder, successor and founder passwords.",
	/* LANG_MIGRATE_SUCCESS */
	"Channel settings migrated."
};

char *langtable_de[] =
{
	/* LANG_MIGRATE_DESC */
        "    MIGRATE  Kopiert Einstellungen von einem Channel zu einen anderen",
	/* LANG_MIGRATE_SYNTAX */
	"Syntax: MIGRATE Quell-Channel Ziel-Channel",
	/* LANG_MIGRATE_SYNTAX_EX */
	"Syntax: MIGRATE Quell-Channel Ziel-Channel\n\n"
	"Kopiert Einstellungen vom Quell-Channel zum Ziel-Channel. Das beinhaltet\n"
	"SET Optionen, Access Listen, Autokick Listen, Badwords Listen, aber\n"
	"nicht den Founder, Successor und die Founder Passwörter.",
	/* LANG_MIGRATE_SUCCESS */
	"Die Channel Einstellungen wurden übernommen."
};

int AnopeInit(void);
void AnopeFini(void);
int do_migrate(User *u);
int help_migrate(User *u);

int AnopeInit(void)
{
	Command *c;
	
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	
	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
        moduleInsertLanguage(LANG_DE, LANG_NUM_STRINGS, langtable_de);
	
	c = createCommand("MIGRATE", do_migrate, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(CHANSERV, c, MOD_HEAD);
	moduleSetChanHelp(do_help_list);
        moduleAddHelp(c, help_migrate);
	
	return MOD_CONT;
}

void AnopeFini(void)
{
	return;
}

int do_migrate(User *u)
{
	ChannelInfo *source;
	ChannelInfo *dest;
	char *source_chan = strtok(NULL, " ");
	char *dest_chan = strtok(NULL, " ");
	
	if (!source_chan)
	{
		moduleNoticeLang(s_ChanServ, u, LANG_MIGRATE_SYNTAX);
		return MOD_STOP;
	}
	if (!dest_chan)
	{
		moduleNoticeLang(s_ChanServ, u, LANG_MIGRATE_SYNTAX);
		return MOD_STOP;
	}
	
	if (!(source = cs_findchan(source_chan)))
	{
		notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, source_chan);
		return MOD_STOP;
	}
	if (!(dest = cs_findchan(dest_chan)))
	{
		notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, dest_chan);
		return MOD_STOP;
	}
	
	if (source->flags & CI_VERBOTEN)
	{
		notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, source_chan);
		return MOD_STOP;
	}
	if (dest->flags & CI_VERBOTEN)
	{
		notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, dest_chan);
		return MOD_STOP;
	}
	
	/* Not happening unless you're a services admin or founder */
	if (!is_services_admin(u) && ((source->flags & CI_SECUREFOUNDER ? !is_real_founder(u, source) : !is_founder(u, source)) && (dest->flags & CI_SECUREFOUNDER ? !is_real_founder(u, dest) : !is_founder(u, dest))))
	{
		notice_lang(s_ChanServ, u, ACCESS_DENIED);
		return MOD_STOP;
	}
	
	if (dest->desc) free(dest->desc);
	if (dest->url) free(dest->url);
	if (dest->email) free(dest->email);
	if (dest->mlock_key) free(dest->mlock_key);
	if (dest->mlock_flood) free(dest->mlock_flood);
	if (dest->mlock_redirect) free(dest->mlock_redirect);
	if (dest->entry_message) free(dest->entry_message);
	if (dest->access) free(dest->access);
	if (dest->akick) free(dest->akick);
	if (dest->badwords) free(dest->badwords);
	
	dest->mlock_on = source->mlock_on;
	dest->mlock_off = source->mlock_off;
	dest->mlock_limit = source->mlock_limit;
	dest->bantype = source->bantype;
	dest->flags = (source->flags & 0xF87F);		/* Removes unwanted flags like forbidden/suspended/noexpire */
	dest->capsmin = source->capsmin;
	dest->capspercent = source->capspercent;
	dest->floodlines = source->floodlines;
	dest->floodsecs = source->floodsecs;
	dest->repeattimes = source->repeattimes;
	dest->botflags = source->botflags;			/* There aren't any 'undesirable' bot flags so we can just copy over */

	dest->desc = (source->desc) ? sstrdup(source->desc) : NULL;
	dest->url = (source->url) ? sstrdup(source->url) : NULL;
	dest->email = (source->email) ? sstrdup(source->email) : NULL;
	dest->mlock_key = (source->mlock_key) ? sstrdup(source->mlock_key) : NULL;
	dest->mlock_flood = (source->mlock_flood) ? sstrdup(source->mlock_flood) : NULL;
	dest->mlock_redirect = (source->mlock_redirect) ? sstrdup(source->mlock_redirect) : NULL;
	dest->entry_message = (source->entry_message) ? sstrdup(source->entry_message) : NULL;
	
	dest->accesscount = source->accesscount;
	dest->access = (dest->accesscount) ? scalloc(dest->accesscount, sizeof(ChanAccess)) : NULL;
	memcpy(dest->access, source->access, (dest->accesscount * sizeof(ChanAccess)));

	dest->akickcount = source->akickcount;
	dest->akick = (dest->akickcount) ? scalloc(dest->akickcount, sizeof(AutoKick)) : NULL;
	memcpy(dest->akick, source->akick, (dest->akickcount * sizeof(AutoKick)));

	dest->bwcount = source->bwcount;
	dest->badwords = (dest->bwcount) ? scalloc(dest->bwcount, sizeof(BadWord)) : NULL;
	memcpy(dest->badwords, source->badwords, (dest->bwcount * sizeof(BadWord)));
	
	/* ci->levels will always be CA_SIZE long as far as I'm aware, so no need to free() and reallocate like above */
	memcpy(dest->levels, source->levels, (CA_SIZE * sizeof(*dest->levels)));
	memcpy(dest->ttb, source->ttb, (TTB_SIZE * sizeof(*dest->ttb)));		/* As levels */
	
	check_modes(dest->c);
	moduleNoticeLang(s_ChanServ, u, LANG_MIGRATE_SUCCESS);
	return MOD_STOP;
}

int help_migrate(User *u)
{
	moduleNoticeLang(s_ChanServ, u, LANG_MIGRATE_SYNTAX_EX);
	return MOD_STOP;
}
