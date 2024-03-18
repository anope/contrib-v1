#include "module.h"
#define AUTHOR "mastergamer"
#define VERSION "1.1.1"

/* Configuration Options */

#define PROTECTSERVICES 1 //If this is 1, services like ChanServ and OperServ, as well as BotServ bots will not be affected by this module. It's probably a good idea to leave this on.

/* End of configuration */

/* ## VERSION CHANGELOG ## */
/*
  v1.0.0 -- First public release
  v1.0.1 -- Fixed a bug that occured when an incorrect number of parameters were given.
  v1.1   -- Changed the code so that BotServ bots are protected as well as Core clients. Removed use of strtok().
*/

int client_func( User *u );
int help_full( User *u );
void help( User *u );


int AnopeInit( int argc, char **argv )
{
  Command *c;
  moduleAddAuthor( AUTHOR );
  moduleAddVersion( VERSION );
  
  c = createCommand( "CLIENT",client_func, is_services_oper,-1,-1,-1,-1,-1 );
  moduleAddCommand( OPERSERV,c,MOD_HEAD );
  moduleAddHelp( c, help_full );
  
  moduleSetOperHelp( help );
  
  
  return MOD_CONT;
}

int client_func( User *u )
{
	char *buf = moduleGetLastBuffer();
	char *cmd = myStrGetToken(buf, ' ', 0);
	char *nick = myStrGetToken(buf, ' ', 1);


  if( !cmd )
  {
    notice( s_OperServ, u->nick, "\2Syntax: CLIENT (ADD|JOIN|DEL|PART) <nick> [chan]\2" );
    return MOD_STOP;
  }
  
  if( nickIsServices(nick, 1) && PROTECTSERVICES == 1 ) //Thanks to n00bie for telling me about nickIsServices() :)
  {
      notice( s_OperServ, u->nick, "Services clients have been protected from this module" );
      return MOD_STOP;
  }

  if( stricmp( cmd, "ADD" ) == 0 )
  {
    char *user = myStrGetToken( buf, ' ', 2 );
    char *host = myStrGetToken( buf, ' ', 3 );
    char *real = myStrGetToken( buf, ' ', 4 );
    if( !user || !host || !real )
    {
      notice( s_OperServ, u->nick, "Syntax: CLIENT ADD <nick> <user> <host> <realname>" );
      return MOD_STOP;
    }
    anope_cmd_bot_nick( nick, user, host, real, "+BqS" );
    notice( s_OperServ, u->nick, "Client %s has been created", nick );
  }
  if( stricmp( cmd, "JOIN" ) == 0 )
  {
    char *chan = myStrGetToken(buf, ' ', 2);
    anope_cmd_join( nick, chan, time( NULL ) );
    notice( s_OperServ, u->nick, "Client \2%s\2 has joined channel \2%s\2", nick, chan );
  }
  if( stricmp( cmd, "DEL" ) == 0 )
  {
    anope_cmd_quit( nick, "I have been ordered to quit by %s", u->nick );
    notice( s_OperServ, u->nick, "Client %s has been deleted", nick );
  }
  
  if( stricmp( cmd, "PART" ) == 0 )
  {
    char *chan = myStrGetToken(buf, ' ', 2);
    anope_cmd_part( nick, chan, "%s told me to leave the channel", u->nick );
    notice( s_OperServ, u->nick, "Client \2%s\2 has parted channel \2%s\2", nick, chan );
  }
  return MOD_CONT;
}

void AnopeFini( void )
{
}

int help_full( User *u )
{
  notice( s_OperServ, u->nick, "Syntax: CLIENT (JOIN|DEL|PART) <nick> [chan]" );
  notice( s_OperServ, u->nick, "Syntax: CLIENT ADD <nick> <user> <host> <realname>" );
  notice( s_OperServ, u->nick, "   " );
  notice( s_OperServ, u->nick, "<nick> is the client's nick, <user> is it's ident, <host> is it's hostmask" );
  notice( s_OperServ, u->nick, "and <realname> is it's realname." );
  notice( s_OperServ, u->nick, "Allows Opers to create/join/part/delete services psuedoclients" );
  notice( s_OperServ, u->nick, "Obviously, JOIN and PART require the [chan] parameter, but the others do not." );

  return MOD_CONT;
}

void help( User *u )
{
  notice( s_OperServ, u->nick, "    CLIENT      Commands to manage psuedoclients" );
}

/* EOF */