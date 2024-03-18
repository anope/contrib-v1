#include "module.h"

#define AUTHOR "mastergamer"
#define VERSION "1.0.0"

/* Set this to what you want the bot's nick to be */
#define BotNick "LogChanMon"
/* End of configuration /*

/* Do not edit below unless you know EXACTLY what you are doing! */

int AnopeInit( int argc, char **argv )
{
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    
    if( !LogChannel )
    {
      alog( "[bs_logchanmon] No LogChannel was found. Unloading module." );
      return MOD_STOP;
    }

    anope_cmd_nick( BotNick, "Service to protect the service log channel.", "+BS" );
    anope_cmd_join( BotNick, LogChannel, time( NULL ) );
    anope_cmd_mode( BotNick, LogChannel, "+ao %s %s", BotNick, BotNick );
    return MOD_CONT;
}

void AnopeFini( void )
{
     anope_cmd_quit( BotNick, "Service Exiting." );
}

/* EOF */