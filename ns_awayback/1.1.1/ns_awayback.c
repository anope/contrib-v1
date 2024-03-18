/*
 * ns_awayback.c - Sets users away/back in the rooms they op/halfop in.
 * Copyright (C) 2008 Ankit K. Vani
 *
 * Database to store ChanServ AWAYBACK setting for channels:
 */
	#define DATABASE		"ns_awayback.db"
/*
 * Define SETAQ if away/back is to be applied to +aq too
 */
	#define SETAQ			1
/*
 * For help, use /msg NickServ HELP AWAY
 *               /msg NickServ HELP BACK
 *               /msg NickServ HELP FAWAY	[services-oper-only]
 *               /msg NickServ HELP FBACK	[services-oper-only]
 *               /msg ChanServ HELP AWAYBACK
 *
 * Tested on Anope-1.7.21
 *
 * Send bug reports to ankit@nevitus.com
 *
 *	CHANGES
 *
 *	v1.0.0 2008-01-25 - First release.
 *	v1.0.1 2008-01-26 - Fixed a bug in ChanServ AWAYBACK command
 *	                    where in some cases, allocated memory is
 *	                    not freed.
 *	                    AWAYBACK will send an alog on use.
 *	v1.1.0 2008-02-27 - Channels with AWAYBACK setting as off are
 *	                    no longer written to the database. Older
 *	                    databases will no longer work as database syntax
 *	                    has been changed from "#channel on/off" to
 *	                    "#channel".
 *	                    Added ChanServ AWAYBACK LIST command to list all
 *	                    channels that have AWAYBACK option set -
 *	                    available only to services admins.
 *	                    Fixed quite a few bugs, some being pretty big
 *	                    (don't know how no bugs were observed for a
 *	                    month O_O).
 *	v1.1.1 2008-05-02 - AWAY/BACK can also be used on +aq if SETAQ is
 *	                    defined.
 *
 ***********************************************************************/


/* DO NOT EDIT ANYTHING BELOW THIS LINE UNLESS YOU KNOW WHAT YOU ARE DOING */


#include "module.h"

/* Module information */
#define AUTHOR	"Ankit"
#define VERSION	"$Id: ns_awayback.c v1.1.1 2008-05-02 Ankit $"

/* String definitions */
#define M_NICK_HELP			0
#define M_NICK_HELP_OPER	1
#define SYNTAX_AWAY			2
#define HELP_AWAY			3
#define SYNTAX_BACK			4
#define HELP_BACK			5
#define SYNTAX_FAWAY		6
#define HELP_FAWAY			7
#define SYNTAX_FBACK		8
#define HELP_FBACK			9
#define M_CHAN_HELP			10
#define SYNTAX_O_AWAYBACK	11
#define SYNTAX_AWAYBACK		12
#define HELP_AWAYBACK		13

#define YOU_SET_AB			14
#define YOU_FORCE_AB		15
#define NICK_RECV_AB		16
#define AFFECTED_NONE		17
#define DID_CS_AWAYBACK		18
#define CUR_AWAYBACK		19

#define M_NUM_STRINGS		20

/* Maximum number of channels that can be passed via (F)AWAY/(F)BACK commands */
#define MAX_CHANS_PASSED	21

/* ab() int parameter */
#define BACK	0
#define AWAY	1

/* Declarations */
int do_away    ( User* );
int do_back    ( User* );
int do_faway   ( User* );
int do_fback   ( User* );
int do_awayback( User* );

void ns_help     ( User* );
void cs_help     ( User* );
int help_away    ( User* );
int help_back    ( User* );
int help_faway   ( User* );
int help_fback   ( User* );
int help_awayback( User* );

int SaveData  ( int, char** );
int BackupData( int, char** );

void addLanguages( void );
int LoadData     ( void );
void ab          ( User*, char*, char*, int );


/* Initialize module */
int AnopeInit( int argc, char **argv )
{
	int status;
	Command *c_away		= NULL,
	        *c_back		= NULL,
	        *c_faway	= NULL,
	        *c_fback	= NULL,
	        *c_awayback	= NULL;

	EvtHook *hook = NULL;

	moduleAddAuthor( AUTHOR );
	moduleAddVersion( VERSION );
	moduleSetType( THIRD );


	c_away     = createCommand( "AWAY",     do_away,     NULL,             -1, -1, -1, -1, -1 );
	c_back     = createCommand( "BACK",     do_back,     NULL,             -1, -1, -1, -1, -1 );
	c_faway    = createCommand( "FAWAY",    do_faway,    is_services_oper, -1, -1, -1, -1, -1 );
	c_fback    = createCommand( "FBACK",    do_fback,    is_services_oper, -1, -1, -1, -1, -1 );
	c_awayback = createCommand( "AWAYBACK", do_awayback, NULL,             -1, -1, -1, -1, -1 );

	if ( ( status = moduleAddCommand( NICKSERV, c_away, MOD_UNIQUE ) ) )
	{
		alog( "[ns_awayback] Unable to create NickServ AWAY command: %d", status );
		return MOD_STOP;
	}

	if ( ( status = moduleAddCommand( NICKSERV, c_back, MOD_UNIQUE ) ) )
	{
		alog( "[ns_awayback] Unable to create NickServ BACK command: %d", status );
		return MOD_STOP;
	}

	if ( ( status = moduleAddCommand( NICKSERV, c_faway, MOD_UNIQUE ) ) )
	{
		alog( "[ns_awayback] Unable to create NickServ FAWAY command: %d", status );
		return MOD_STOP;
	}

	if ( ( status = moduleAddCommand( NICKSERV, c_fback, MOD_UNIQUE ) ) )
	{
		alog( "[ns_awayback] Unable to create NickServ FBACK command: %d", status );
		return MOD_STOP;
	}

	if ( ( status = moduleAddCommand( CHANSERV, c_awayback, MOD_UNIQUE ) ) )
	{
		alog( "[ns_awayback] Unable to create ChanServ AWAYBACK command: %d", status );
		return MOD_STOP;
	}


	hook = createEventHook( EVENT_DB_SAVING, SaveData );
	status = moduleAddEventHook( hook );

	hook = createEventHook( EVENT_DB_BACKUP, BackupData );
	status = moduleAddEventHook( hook );


	addLanguages();
	moduleSetNickHelp( ns_help );
	moduleSetChanHelp( cs_help );
	moduleAddHelp    ( c_away,     help_away );
	moduleAddHelp    ( c_back,     help_back );
	moduleAddHelp    ( c_faway,    help_faway );
	moduleAddHelp    ( c_fback,    help_fback );
	moduleAddHelp    ( c_awayback, help_awayback );

	LoadData();

	return MOD_CONT;
}

/* Unload module */
void AnopeFini( void )
{
	char *av[1] = { NULL };

	av[0] = sstrdup( EVENT_START );
	SaveData( 1, av );

	free( av[0] );
}


/*
	Command implementations
*/

/* NickServ AWAY */
int do_away( User* u )
{
	char* text		= NULL;
	char* channels	= NULL;

	text = moduleGetLastBuffer();
	channels = myStrGetToken( text, ' ', 0 );

	ab( u, NULL, channels, AWAY );

	if ( channels ) free( channels );

	return MOD_CONT;
}

/* NickServ BACK */
int do_back( User* u )
{
	char* text		= NULL;
	char* channels	= NULL;

	text = moduleGetLastBuffer();
	channels = myStrGetToken( text, ' ', 0 );

	ab( u, NULL, channels, BACK );

	if ( channels ) free( channels );

	return MOD_CONT;
}

/* NickServ FAWAY */
int do_faway( User* u )
{
	char* text		= NULL;
	char* target	= NULL;
	char* channels	= NULL;

	text = moduleGetLastBuffer();

	if ( text )
	{
		target = myStrGetToken( text, ' ', 0 );
		channels = myStrGetToken( text, ' ', 1 );

		if ( !target )
			moduleNoticeLang( s_NickServ, u, SYNTAX_FAWAY );
		else
			ab( u, target, channels, AWAY );
	}
	else
		moduleNoticeLang( s_NickServ, u, SYNTAX_FAWAY );

	if ( target ) free( target );
	if ( channels ) free( channels );

	return MOD_CONT;
}

/* NickServ FBACK */
int do_fback( User* u )
{
	char* text		= NULL;
	char* target	= NULL;
	char* channels	= NULL;

	text = moduleGetLastBuffer();

	if ( text )
	{
		target = myStrGetToken( text, ' ', 0 );
		channels = myStrGetToken( text, ' ', 1 );

		if ( !target )
			moduleNoticeLang( s_NickServ, u, SYNTAX_FBACK );
		else
			ab( u, target, channels, BACK );
	}
	else
		moduleNoticeLang( s_NickServ, u, SYNTAX_FBACK );

	if ( target ) free( target );
	if ( channels ) free( channels );

	return MOD_CONT;
}


/*
	The heart of the module
*/

void ab( User* usr, char* target, char* chans, int away )
{
	if ( !nick_identified( usr ) )
	{
		notice_lang( s_NickServ, usr, NICK_IDENTIFY_REQUIRED, s_NickServ );
		return;
	}

	int force = 0;
	User* u			= NULL;
	char *setting	= NULL;

	if ( !target )
		u = usr;
	else
	{
		u = finduser( target );

		if ( !u )
		{
			notice_lang( s_NickServ, usr, NICK_X_NOT_IN_USE, target );
			return;
		}

		if ( u != usr )
			force = 1;
	}

	/* channels passed as parameters */
	int numChans = 0;
	char* ch[MAX_CHANS_PASSED];

	int used = 0;

	if ( chans )
	{
		/* channels are specified - check channels and use */

		for ( numChans = 0; numChans < MAX_CHANS_PASSED; numChans++ )
		{
			ch[numChans] = NULL;
			ch[numChans] = myStrGetToken( chans, ',', numChans );

			if ( !ch[numChans] )
				break;
		}

		if ( !numChans )
		{
			if ( target )
				moduleNoticeLang( s_NickServ, usr, ( away ? SYNTAX_FAWAY : SYNTAX_FBACK ) );
			else
				moduleNoticeLang( s_NickServ, usr, ( away ? SYNTAX_AWAY : SYNTAX_BACK ) );

			return;
		}

		Channel *c = NULL;

		int i;
		for ( i = 0; i < numChans; i++ )
		{
			c = findchan( ch[i] );
			if ( ch[i] ) free( ch[i] );
			
			if ( !c || !c->ci )
				continue;

			if ( !is_on_chan( c, u ) )
				continue;

			setting = moduleGetData( &c->ci->moduleData, "AwayBackSetting" );
			if ( !setting || stricmp( setting, "on" ) )
			{
				if ( setting )
				{
					free( setting );
					setting = NULL;
				}
				continue;
			}

#ifdef SETAQ
			if ( is_real_founder( u, c->ci ) )
				anope_cmd_mode( whosends( c->ci ), c->name, "%s %s %s %s", ( away ? "+v-oq" : "+oq-v" ), u->nick, u->nick, u->nick );
			else if ( check_access( u, c->ci, CA_AUTOPROTECT ) )
				anope_cmd_mode( whosends( c->ci ), c->name, "%s %s %s %s", ( away ? "+v-oa" : "+oa-v" ), u->nick, u->nick, u->nick );
			else
#endif
			if ( check_access( u, c->ci, CA_AUTOOP ) )
				anope_cmd_mode( whosends( c->ci ), c->name, "%s %s %s", ( away ? "+v-o" : "+o-v" ), u->nick, u->nick );
			else if ( check_access( u, c->ci, CA_AUTOHALFOP ) )
				anope_cmd_mode( whosends( c->ci ), c->name, "%s %s %s", ( away ? "+v-h" : "+h-v" ), u->nick, u->nick );
			else
				continue;

			used++;
		}

		if ( setting ) free( setting );
	}
	else
	{
		/* channels are not specified - use all possible channels */

		struct u_chanlist *cl = NULL;

		for ( cl = u->chans; cl; cl = cl->next )
		{
			if ( !cl->chan || !cl->chan->ci )
				continue;

			setting = moduleGetData( &cl->chan->ci->moduleData, "AwayBackSetting" );
			if ( !setting || stricmp( setting, "on" ) )
			{
				if ( setting )
				{
					free( setting );
					setting = NULL;
				}
				continue;
			}

#ifdef SETAQ
			if ( is_real_founder( u, cl->chan->ci ) )
				anope_cmd_mode( whosends( cl->chan->ci ), cl->chan->name, "%s %s %s %s", ( away ? "+v-oq" : "+oq-v" ), u->nick, u->nick, u->nick );
			else if ( check_access( u, cl->chan->ci, CA_AUTOPROTECT ) )
				anope_cmd_mode( whosends( cl->chan->ci ), cl->chan->name, "%s %s %s %s", ( away ? "+v-oa" : "+oa-v" ), u->nick, u->nick, u->nick );
			else
#endif
			if ( check_access( u, cl->chan->ci, CA_AUTOOP ) )
				anope_cmd_mode( whosends( cl->chan->ci ), cl->chan->name, "%s %s %s", ( away ? "+v-o" : "+o-v" ), u->nick, u->nick );
			else if ( check_access( u, cl->chan->ci, CA_AUTOHALFOP ) )
				anope_cmd_mode( whosends( cl->chan->ci ), cl->chan->name, "%s %s %s", ( away ? "+v-h" : "+h-v" ), u->nick, u->nick );
			else
				continue;

			used++;
		}

		if ( setting ) free( setting );
	}

	if ( !used )
	{
		moduleNoticeLang( s_NickServ, usr, AFFECTED_NONE, ( away ? "AWAY" : "BACK" ) );
		return;
	}
	else if ( force )
	{
		moduleNoticeLang( s_NickServ, u, NICK_RECV_AB, ( away ? "AWAY" : "BACK" ), used, usr->nick );
		moduleNoticeLang( s_NickServ, usr, YOU_FORCE_AB, u->nick, ( away ? "AWAY" : "BACK" ), used );
		alog( "%s has been forcefully set as being \002%s\002 on %d channels by oper %s.", u->nick, ( away ? "AWAY" : "BACK" ), used, usr->nick );
	}
	else
		moduleNoticeLang( s_NickServ, u, YOU_SET_AB, ( away ? "AWAY" : "BACK" ), used );
}

/* ChanServ AWAYBACK */
int do_awayback( User* u )
{
	char *text		= NULL;
	char *channel	= NULL;
	char *option	= NULL;

	text = moduleGetLastBuffer();
	channel = myStrGetToken( text, ' ', 0 );
	option = myStrGetToken( text, ' ', 1 );

	if ( !text || !channel || ( option && stricmp( option, "on" ) && stricmp( option, "off" ) ) )
	{
		if( is_services_admin( u ) )
			moduleNoticeLang( s_ChanServ, u, SYNTAX_O_AWAYBACK );
		else
			moduleNoticeLang( s_ChanServ, u, SYNTAX_AWAYBACK );

		if ( option ) free( option );
		if ( channel ) free( channel );

		return MOD_CONT;
	}
	
	if ( !stricmp( channel, "list" ) )
	{
		if ( !is_services_admin( u ) )
		{
			notice_lang( s_ChanServ, u, ACCESS_DENIED );

			if ( option ) free( option );
			free( channel );

			return MOD_CONT;
		}

		ChannelInfo* ci;
		char* setting;
		int i, shown = 0;

		notice( s_NickServ, u->nick, "Channels with \002AWAYBACK\002 enabled:" );

		for ( i = 0; i < 256; i++ )
		{
			for ( ci = chanlists[i]; ci; ci = ci->next )
			{
				if ( ( setting = moduleGetData( &ci->moduleData, "AwayBackSetting" ) ) )
				{
					/* v1.1.0 update - Only write if AwayBackSetting is ON */

					if ( !stricmp( setting, "on" ) )
					{
						notice( s_NickServ, u->nick, "  %s", ci->name );
						shown++;
					}

					free( setting );
				}
			}
		}

		notice( s_NickServ, u->nick, "End of list - %d/%d channels shown.", shown, shown );
	
		if ( option ) free( option );
		free( channel );
		return MOD_CONT;
	}

	ChannelInfo *ci = NULL;
	ci = cs_findchan( channel );

	if ( !ci )
	{
		notice_lang( s_ChanServ, u, CHAN_X_NOT_REGISTERED, channel );

		if ( option ) free( option );
		free( channel );

		return MOD_CONT;
	}
	else if ( !is_founder( u, ci ) && !is_services_admin( u ) )
	{
		notice_lang( s_ChanServ, u, ACCESS_DENIED );

		if ( option ) free( option );
		free( channel );

		return MOD_CONT;
	}

	if ( !option )
	{
		option = moduleGetData( &ci->moduleData, "AwayBackSetting" );

		if ( !option )
			moduleNoticeLang( s_ChanServ, u, CUR_AWAYBACK, "off", ci->name );
		else
		{
			if ( !stricmp( option, "on" ) )
				moduleNoticeLang( s_ChanServ, u, CUR_AWAYBACK, "on", ci->name );
			else
				moduleNoticeLang( s_ChanServ, u, CUR_AWAYBACK, "off", ci->name );
			free( option );
		}

		free( channel );
		return MOD_CONT;
	}

	moduleAddData( &ci->moduleData, "AwayBackSetting", option );
	moduleNoticeLang( s_ChanServ, u, DID_CS_AWAYBACK, ( !stricmp( option, "on" ) ? "enabled" : "disabled" ), ci->name );
	alog( "%s set AWAYBACK \002%s\002 for %s.", u->nick, ( !stricmp( option, "on" ) ? "on" : "off" ), ci->name );

	free( option );
	free( channel );

	return MOD_CONT;
}


/*
	Manage database
*/

int LoadData( void )
{
	FILE *in		= NULL;
	int len = 0;

	ChannelInfo *ci	= NULL;

	char buffer[2000];

	if ( !( in = fopen( DATABASE, "r" ) ) )
	{
		alog( "[ns_awayback] WARNING (LoadData): Could not open database file %s. (It is safe to ignore this warning if this is the first time you are using this module or no channel has AWAYBACK set)", DATABASE );
		return MOD_STOP;
	}

	while ( fgets( buffer, 1500, in ) )
	{
		len = strlen( buffer );

		/* Remove \n from end of the line */
		buffer[len - 1] = 0;

		if ( ( ci = cs_findchan( buffer ) ) )
			moduleAddData( &ci->moduleData, "AwayBackSetting", "on" );
	}

	return MOD_CONT;
}

int SaveData( int argc, char** argv )
{
	ChannelInfo *ci	= NULL;
	FILE *out		= NULL;
	char* setting	= NULL;

	if ( !( argc >= 1 ) || stricmp( argv[0], EVENT_START ) )
		return MOD_CONT;

	if ( !( out = fopen( DATABASE, "w" ) ) )
	{
		alog( "[ns_awayback] ERROR (SaveData): Could not open the database file %s!", DATABASE );
		return MOD_STOP;
	}

	int i;

	for ( i = 0; i < 256; i++ )
	{
		for ( ci = chanlists[i]; ci; ci = ci->next )
		{
			if ( ( setting = moduleGetData( &ci->moduleData, "AwayBackSetting" ) ) )
			{
				/* v1.1.0 update - Only write if AwayBackSetting is ON */

				if ( !stricmp( setting, "on" ) )
					fprintf( out, "%s\n", ci->name );

				free( setting );
			}
		}
	}

	fclose( out );
	return MOD_CONT;
}

int BackupData( int argc, char** argv )
{
	ModuleDatabaseBackup( DATABASE );
	return MOD_CONT;
}


/*
	Command HELP
*/

int help_away( User* u )
{
	moduleNoticeLang( s_NickServ, u, SYNTAX_AWAY );
	notice( s_NickServ, u->nick, " " );
	moduleNoticeLang( s_NickServ, u, HELP_AWAY );
	return MOD_CONT;
}

int help_back( User* u )
{
	moduleNoticeLang( s_NickServ, u, SYNTAX_BACK );
	notice( s_NickServ, u->nick, " " );
	moduleNoticeLang( s_NickServ, u, HELP_BACK );
	return MOD_CONT;
}

int help_faway( User* u )
{
	moduleNoticeLang( s_NickServ, u, SYNTAX_FAWAY );
	notice( s_NickServ, u->nick, " " );
	moduleNoticeLang( s_NickServ, u, HELP_FAWAY );
	return MOD_CONT;
}

int help_fback( User* u )
{
	moduleNoticeLang( s_NickServ, u, SYNTAX_FBACK );
	notice( s_NickServ, u->nick, " " );
	moduleNoticeLang( s_NickServ, u, HELP_FBACK );
	return MOD_CONT;
}

int help_awayback( User* u )
{
	if( is_services_admin( u ) )
		moduleNoticeLang( s_ChanServ, u, SYNTAX_O_AWAYBACK );
	else
		moduleNoticeLang( s_ChanServ, u, SYNTAX_AWAYBACK );
	notice( s_ChanServ, u->nick, " " );
	moduleNoticeLang( s_ChanServ, u, HELP_AWAYBACK );
	return MOD_CONT;
}

void ns_help( User* u )
{
	moduleNoticeLang( s_NickServ, u, M_NICK_HELP );
	if ( is_oper( u ) )
		moduleNoticeLang( s_NickServ, u, M_NICK_HELP_OPER );
}

void cs_help( User* u )
{
	moduleNoticeLang( s_ChanServ, u, M_CHAN_HELP );
}


/* Language table */
void addLanguages( void )
{
	/* English (US) */
	char *langtable_en_us[] = {
		/* M_NICK_HELP */
		"    AWAY       Removes your op/halfop and gives you voice\n"
		"    BACK       Gives you back your op/halfop and removes voice",

		/* M_NICK_HELP_OPER */
		"    FAWAY      Forcefully set a user as AWAY\n"
		"    FBACK      Forcefully set a user as BACK",

		/* SYNTAX_AWAY */
		"Syntax: \002AWAY \037[#channel1,#channel2...]\037\002",

		/* HELP_AWAY */
		"Sets you away in the given channels you have HOP or greater access on.\n"
		"That is, services will remove your operator or halfop status and give you\n"
		"voice. If no channel is specified, all possible channels will be affected.",

		/* SYNTAX_BACK */
		"Syntax: \002BACK \037[#channel1,#channel2...]\037\002",

		/* HELP_BACK */
		"Reverses \002AWAY\002 command. Services will give you your operator or\n"
		"halfop status back and remove voice. If no channel is specified, all\n"
		"possible channels will be affected.",

		/* SYNTAX_FAWAY */
		"Syntax: \002FAWAY \037nick\037 \037[#channel1,#channel2...]\037\002",

		/* HELP_FAWAY */
		"Forcefully set a user as being \002AWAY\002. If no channel is specified,\n"
		"all possible channels will be affected.",

		/* SYNTAX_FBACK */
		"Syntax: \002FBACK \037nick\037 \037[#channel1,#channel2...]\037\002",

		/* HELP_FBACK */
		"Forcefully set a user as being \002BACK\002. If no channel is specified,\n"
		"all possible channels will be affected.",

		/* M_CHAN_HELP */
		"    AWAYBACK   Allow/deny a channel from being used for NickServ AWAY/BACK",

		/* SYNTAX_O_AWAYBACK */
		"Syntax: \002AWAYBACK <\037#channel\037 [\037on/off\037] / \037list\037>\002",
		
		/* SYNTAX_AWAYBACK */
		"Syntax: \002AWAYBACK \037#channel\037 \037[on/off]\037\002",

		/* HELP_AWAYBACK */
		"If set to on, the NickServ \002AWAY\002 and \002BACK\002 commands will be\n"
		"enabled on the channel. Default setting is \002off\002. If you ignore the\n"
		"last parameter, the current setting will be displayed.",

		/* YOU_SET_AB */
		"You are now set as being \002%s\002 in %d channels.",

		/* YOU_FORCE_AB */
		"%s has been forcefully set as being \002%s\002 in %d channels.",

		/* NICK_RECV_AB */
		"You have been forcefully set \002%s\002 in %d channels by oper %s.",

		/* AFFECTED_NONE */
		"No valid channels were found to set \002%s\002.",

		/* DID_CS_AWAYBACK */
		"NickServ commands AWAY and BACK are now \002%s\002 for %s.",

		/* CUR_AWAYBACK */
		"AWAYBACK option is currently \002%s\002 for %s.",
	};

	moduleInsertLanguage( LANG_EN_US, M_NUM_STRINGS, langtable_en_us );
}
