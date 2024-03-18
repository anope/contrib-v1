#include "module.h"
#define AUTHOR "mastergamer"
#define VERSION "1.0.1"

/* Configuration Options */

#define PROTECTSERVICES 1 //If this is 1, services like ChanServ and OperServ will not be affected by this module. It's probably a good idea to leave this on.

/* End of configuration */

/* ## VERSION CHANGELOG ## */
/*
  v1.0.0 -- First public release
  v1.0.1 -- Fixed a bug that occured when an incorrect number of parameters were given.
*/

int client_func( User *u );
int help_full( User *u );
char *nick, *chan, *cmd;
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
  cmd  = strtok( NULL, " " );
  nick = strtok( NULL, " " );
  chan = strtok( NULL, " " );
  if( !cmd )
  {
    help_full(u);
    return MOD_STOP;
  }
  if( stricmp( cmd, "ADD" ) == 0 )
  {
    anope_cmd_nick( nick, "Spawned Psuedoclient", "+BS" );
    notice( s_OperServ, u->nick, "Client %s has been created", nick );
  }
  if( stricmp( cmd, "JOIN" ) == 0 )
  {
    if( stricmp( nick, "ChanServ" ) == 0 || stricmp( nick, "NickServ" ) == 0 || stricmp( nick, "BotServ" ) == 0 || stricmp( nick, "OperServ" ) == 0 || stricmp( nick, "HostServ" ) == 0 || stricmp( nick, "HelpServ" ) == 0 || stricmp( nick, "MemoServ" ) == 0 || stricmp( nick, "Global" ) == 0 || stricmp( nick, "DevNull" ) == 0 && PROTECTSERVICES == 1 )
    {
      notice( s_OperServ, u->nick, "Core services clients have been protected from this module" );
    }
    
    else
    {
      anope_cmd_join( nick, chan, time( NULL ) );
      notice( s_OperServ, u->nick, "Client \2%s\2 has joined channel \2%s\2", nick, chan );
    }
  }
  if( stricmp( cmd, "DEL" ) == 0 )
  {
        if( stricmp( nick, "ChanServ" ) == 0 || stricmp( nick, "NickServ" ) == 0 || stricmp( nick, "BotServ" ) == 0 || stricmp( nick, "OperServ" ) == 0 || stricmp( nick, "HostServ" ) == 0 || stricmp( nick, "HelpServ" ) == 0 || stricmp( nick, "MemoServ" ) == 0 || stricmp( nick, "Global" ) == 0 || stricmp( nick, "DevNull" ) == 0 && PROTECTSERVICES == 1 )
    {
      notice( s_OperServ, u->nick, "Core services clients have been protected from this module" );
    }
    
    else
    {
      anope_cmd_quit( nick, "I have been ordered to quit by %s", u->nick );
      notice( s_OperServ, u->nick, "Client %s has been deleted", nick );
    }
  }
  
  if( stricmp( cmd, "PART" ) == 0 )
  {
    if( stricmp( nick, "ChanServ" ) == 0 || stricmp( nick, "NickServ" ) == 0 || stricmp( nick, "BotServ" ) == 0 || stricmp( nick, "OperServ" ) == 0 || stricmp( nick, "HostServ" ) == 0 || stricmp( nick, "HelpServ" ) == 0 || stricmp( nick, "MemoServ" ) == 0 || stricmp( nick, "Global" ) == 0 || stricmp( nick, "DevNull" ) == 0 && PROTECTSERVICES == 1 )
    {
      notice( s_OperServ, u->nick, "Core services clients have been protected from this module" );
    }
    
    else
    {
      anope_cmd_part( nick, chan, "%s told me to leave the channel", u->nick );
      notice( s_OperServ, u->nick, "Client \2%s\2 has parted channel \2%s\2", nick, chan );
    }
  }
  return MOD_CONT;
}

void AnopeFini( void )
{
  anope_cmd_quit( nick, "Module Unloaded" );
}

int help_full( User *u )
{
  notice( s_OperServ, u->nick, " Syntax: CLIENT (ADD|JOIN|DEL|PART) <nick> [chan]" );
  notice( s_OperServ, u->nick, "   " );
  notice( s_OperServ, u->nick, " Allows Opers to create/join/part/delete services psuedoclients" );
  notice( s_OperServ, u->nick, " Obviously, JOIN and PART require the [chan] parameter, but the others do not." );

  return MOD_CONT;
}

void help( User *u )
{
  notice( s_OperServ, u->nick, "    CLIENT      Commands to manage psuedoclients" );
}

/* EOF */