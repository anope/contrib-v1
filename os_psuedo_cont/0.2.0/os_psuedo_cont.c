 /*-----------------------------------------------------------
  * Name: os_psuedo_cont "operserv psuedoclient controller"
  * ReCoder: THX1138 
  * Date: 05/20/2009
  * -----------------------------------------------------------
  * Base of this module is derived from two seperate modules. 
  * os_client.c by: mastergamer - editable psuedoclients
  * os_clientjoin.c by: SGR - mass services join/part
  * 
  * Fixed removal of QLine when psuedoclients are deleted. (os_client.c)
  * Integrated services protections entirely and to a more logical position. (os_client,c) 
  * Removed modes applied to channel on join. (os_clientjoin.c)
  * Reintegrated ability to mass join/part botserv bots but in semi-controlled manor.
  * Restricted Usage to occupied registered channels.
  * -----------------------------------------------------------
  * Allows IRCops to create/join/part/delete psuedoclients
  * Syntax: CLIENT (JOIN|PART) <nick> <chan>  // note: also works for services bots 
  * Syntax: CLIENT ADD <nick> <ident> <host> <realname>
  * Syntax: CLIENT DEL <nick> deletes a psuedoclient  // core services are protected
  * Syntax: CLIENT (SJOIN|SPART) <chan> mass-join/part services psuedoclients
  * Syntax: CLIENT (BSJOIN|BSPART) <chan> mass-join/part botserv psuedoclients
  * Services clients have been protected from this module and cannot be deleted.
  * -----------------------------------------------------------
  * NOTE: Please use caution when using this module.
  *
  *  TESTED ON: Anope 1.8.0 - Unreal3.2.8.1
  *  No other versions known to be supported.
  * -----------------------------------------------------------
  * 0.2.0 - complete re-write. 05/30/09
  */
 #include "module.h"
int client_func( User *u );
int help_full( User *u );
void help( User *u );
int AnopeInit( int argc, char **argv )
{
  Command *c;
  moduleAddAuthor( "THX1138" );
  moduleAddVersion( "$Id: operserv psuedoclient controller 0.2.0 $" );
  c = createCommand( "CLIENT",client_func, is_services_oper,-1,-1,-1,-1,-1 );
  moduleAddCommand( OPERSERV,c,MOD_HEAD );
  moduleAddHelp( c, help_full );
  moduleSetOperHelp( help );
  return MOD_CONT;
}
void help( User *u )
{
  notice( s_OperServ, u->nick, "    CLIENT      Commands to manage psuedoclients" );
}
int help_full( User *u )
{
  notice( s_OperServ, u->nick, "Allows IRCops to create/join/part/delete psuedoclients" );
  notice( s_OperServ, u->nick, "Syntax: CLIENT ADD <nick> <ident> <host> <realname>" );
  notice( s_OperServ, u->nick, "Syntax: CLIENT (JOIN|PART) <nick> <chan>" );
  notice( s_OperServ, u->nick, "Syntax: CLIENT DEL <nick> deletes a psuedoclient" );
  notice( s_OperServ, u->nick, "Syntax: CLIENT (SJOIN|SPART) <chan> mass-join/part services psuedoclients" ); 
  notice( s_OperServ, u->nick, "Syntax: CLIENT (BSJOIN|BSPART) <chan> mass-join/part botserv psuedoclients" ); 
  notice( s_OperServ, u->nick, "Services clients have been protected from this module and cannot be deleted" );
  return MOD_CONT;
}
int client_func( User *u )
{    

	ChannelInfo *ci;
	char *buf = moduleGetLastBuffer();
	char *cmd = myStrGetToken(buf, ' ', 0);
	if( !cmd ) 
	{
		notice( s_OperServ, u->nick, "Syntax: CLIENT (ADD|JOIN|SJOIN|BSPART|DEL|PART|SPART|BSPART) - /msg operserv help client" );
		return MOD_STOP;
	}
	if( stricmp( cmd, "ADD" ) == 0 )
	{
		char *nick = myStrGetToken(buf, ' ', 1);
		char *user = myStrGetToken(buf, ' ', 2 );
		char *host = myStrGetToken(buf, ' ', 3 );
		char *real = myStrGetToken(buf, ' ', 4 );
		if( !nick || !user || !host || !real )
		{
			notice( s_OperServ, u->nick, "Syntax: CLIENT ADD <nick> <ident> <host> <realname>" );
			return MOD_STOP;
		}
		anope_cmd_bot_nick( nick, user, host, real, "+BqS" );
		notice( s_OperServ, u->nick, "Client %s has been created", nick );
	}
	if( stricmp( cmd, "JOIN" ) == 0 ) 
	{
		char *nick = myStrGetToken(buf, ' ', 1);
		char *chan = myStrGetToken(buf, ' ', 2);
		if ( !nick | !chan ) 
		{
			notice(s_OperServ, u->nick, "Sytax: CLIENT JOIN NICK #CHANNEL");
			return MOD_STOP;
		}
		if (!(ci = cs_findchan(chan)))
		{
			notice(s_OperServ, u->nick, "Sorry that channel does not appear to be occupied or registered.");
			return MOD_STOP;
		}
		anope_cmd_join( nick, chan, time( NULL ) );
	}
	if (stricmp( cmd,"SJOIN") == 0) 
	{
		char *chan = myStrGetToken(buf, ' ', 1);
		if (!chan) 
		{
			notice(s_OperServ, u->nick, "Syntax: CLIENT SJOIN #CHANNEL");
			return MOD_STOP;
		}
		if (!(ci = cs_findchan(chan)))
		{
			notice(s_OperServ, u->nick, "Sorry that channel does not appear to be occupied or registered.");
			return MOD_STOP;
		}
		notice(s_OperServ,u->nick, "Mass joining services psuedoclients to %s", chan  );
		if (s_ChanServ)
		{
			anope_cmd_join(s_ChanServ, chan, time(NULL));
		}
		if (s_MemoServ)
		{
			anope_cmd_join(s_MemoServ, chan, time(NULL));
		}
		if (s_NickServ)
		{
			anope_cmd_join(s_NickServ, chan, time(NULL));
		}
		if (s_BotServ)
		{
			anope_cmd_join(s_BotServ, chan, time(NULL));
		}
		if (s_HostServ)
		{
			anope_cmd_join(s_HostServ, chan, time(NULL));
		}
		if (s_OperServ)
		{
			anope_cmd_join(s_OperServ, chan, time(NULL));
		}
		if (s_DevNull)
		{
			anope_cmd_join(s_DevNull, chan, time(NULL));
		}
		if (s_HelpServ)
		{
			anope_cmd_join(s_HelpServ, chan, time(NULL));
		}
		if (s_GlobalNoticer)
		{
			anope_cmd_join(s_GlobalNoticer, chan, time(NULL));
		}
		if (s_NickServAlias)
		{
			anope_cmd_join(s_NickServAlias, chan, time(NULL));
		}
		if (s_ChanServAlias)
		{
			anope_cmd_join(s_ChanServAlias, chan, time(NULL));
		}
		if (s_BotServAlias)
		{
			anope_cmd_join(s_BotServAlias, chan, time(NULL));
		}
		if (s_MemoServAlias)
		{
			anope_cmd_join(s_MemoServAlias, chan, time(NULL));
		}
		if (s_HelpServAlias) 
		{
			anope_cmd_join(s_HelpServAlias, chan, time(NULL));
		}
		if (s_OperServAlias)
		{
			anope_cmd_join(s_OperServAlias, chan, time(NULL));
		}
		if (s_NickServAlias)
		{
			anope_cmd_join(s_NickServAlias, chan, time(NULL));
		}
		if (s_DevNullAlias)
		{
			anope_cmd_join(s_DevNullAlias, chan, time(NULL));
		}
		if (s_HostServAlias)
		{
			anope_cmd_join(s_HostServAlias, chan, time(NULL));
		}
		if (s_GlobalNoticerAlias)
		{
			anope_cmd_join(s_GlobalNoticerAlias, chan, time(NULL));
		}
		return MOD_STOP;

	}
	if (stricmp( cmd,"BSJOIN") == 0) 
	{
		int i;
		BotInfo *bi;
		char *chan = myStrGetToken(buf, ' ', 1);
		if (!chan) 
		{
			notice(s_OperServ, u->nick, "Syntax: CLIENT BSJOIN #CHANNEL");
			return MOD_STOP;
		}
		if (!(ci = cs_findchan(chan)))
		{
			notice(s_OperServ, u->nick, "Sorry that channel does not appear to be occupied or registered.");
			return MOD_STOP;
		}
		notice(s_OperServ,u->nick, "Mass joining BotServ psuedoclients to %s", chan  );
		if (!nbots) 
		{
			return;
		}
		else 
		{
			for (i = 0; i < 256; i++)
			{
				for (bi = botlists[i]; bi; bi = bi->next)
				{
					anope_cmd_join(bi->nick, chan, time(NULL));
				}
			}
		}
	}
	if( stricmp( cmd, "DEL" ) == 0 )
	{
		char *nick = myStrGetToken(buf, ' ', 1);
		if ( !nick )
		{
			notice(s_OperServ, u->nick, "Sytax: CLIENT DEL NICK");
			return MOD_STOP;
		}
		if( nickIsServices(nick, 1) )
		{
			notice( s_OperServ, u->nick, "Services clients have been protected from this module" );
			return MOD_STOP;
		}
		anope_cmd_quit( nick, "I have been ordered to quit by %s", u->nick );
		anope_cmd_unsqline(nick);
		EnforceQlinedNick(nick, nick);
		notice( s_OperServ, u->nick, "Client %s has been deleted", nick );
	}
	if( stricmp( cmd, "PART" ) == 0 )
	{
		char *nick = myStrGetToken(buf, ' ', 1);
		char *chan = myStrGetToken(buf, ' ', 2);
		if ( !nick || !chan )
		{
			notice(s_OperServ, u->nick, "Sytax: CLIENT PART NICK #CHANNEL");
			return MOD_STOP;
		}
		anope_cmd_part( nick, chan, "%s told me to leave the channel", u->nick );
		notice( s_OperServ, u->nick, "Client \2%s\2 has parted channel \2%s\2", nick, chan );
	}
	if( stricmp( cmd, "SPART" ) == 0 ) 
	{
		char *chan = myStrGetToken(buf, ' ', 1);
		if (!chan)
		{
			notice(s_OperServ, u->nick, "Syntax: CLIENT SPART #CHANNEL");
			return MOD_STOP;
		}
		notice(s_OperServ,u->nick, "Mass parting all services psuedoclients from %s.", chan );
		if (s_ChanServ) 
		{
			 send_cmd(s_ChanServ, "PART %s", chan); 
		}
		if (s_MemoServ)
		{
			 send_cmd(s_MemoServ, "PART %s", chan);
		}
		if (s_NickServ)
		{
			send_cmd(s_NickServ, "PART %s", chan);
		}
		if (s_BotServ) 
		{
			send_cmd(s_BotServ, "PART %s", chan); 
		}
		if (s_HostServ) 
		{
			send_cmd(s_HostServ, "PART %s", chan);
		}
		if (s_OperServ) 
		{
			send_cmd(s_OperServ, "PART %s", chan);
		}
		if (s_DevNull) 
		{
			send_cmd(s_DevNull,  "PART %s", chan);
		}
		if (s_HelpServ)
		{
			send_cmd(s_HelpServ, "PART %s", chan);
		}
		if (s_GlobalNoticer)
		{
			send_cmd(s_GlobalNoticer, "PART %s", chan);
		}
		if (s_NickServAlias)
		{
			send_cmd(s_NickServAlias, "PART %s", chan);
		}
		if (s_ChanServAlias)
		{
			send_cmd(s_ChanServAlias, "PART %s", chan);
		}
		if (s_BotServAlias)
		{
			send_cmd(s_BotServAlias, "PART %s", chan);
		}
		if (s_MemoServAlias)
		{
			send_cmd(s_MemoServAlias, "PART %s", chan);
		}
		if (s_HelpServAlias)
		{
			send_cmd(s_HelpServAlias, "PART %s", chan);
		}
		if (s_OperServAlias)
		{
			send_cmd(s_OperServAlias, "PART %s", chan);
		}
		if (s_DevNullAlias)
		{
			send_cmd(s_DevNullAlias, "PART %s", chan);
		}
		if (s_HostServAlias)
		{
			send_cmd(s_HostServAlias, "PART %s", chan);
		}
		if (s_GlobalNoticerAlias)
		{
		  send_cmd(s_GlobalNoticerAlias, "PART %s", chan);
		}
		return MOD_STOP;
	}
	if (stricmp( cmd,"BSPART") == 0)
	{
		int i;
		BotInfo *bi;
		char *chan = myStrGetToken(buf, ' ', 1);
		if (!chan)
		{
			notice(s_OperServ, u->nick, "Syntax: CLIENT BSPART #CHANNEL");
			return MOD_STOP;
		}
		notice(s_OperServ,u->nick, "Mass parting BotServ psuedoclients from %s", chan );
		if (!nbots) 
		{
			return;
		}
		for (i = 0; i < 256; i++)
		{
			for (bi = botlists[i]; bi; bi = bi->next)
			{
				send_cmd(bi->nick, "PART %s", chan);
			}
		}
	}
}
void AnopeFini(void)
{
	alog("os_psuedo_cont%s: module unloaded.", MODULE_EXT);
}
/* EOF */ 