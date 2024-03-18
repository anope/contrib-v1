/**
 * -----------------------------------------------------------------------------
 * Name    : os_redirect
 * Author  : VisioN  <vision@myirc.net>
 * Date: 24/1/2012   Last Updated: 15/1/2012
 * Version : 1.0
 * -----------------------------------------------------------------------------
 * Limitations : Any IRCd which supports SVSPART and SVSJOIN
 * Requires    : Anope 1.8.7
 * Tested      : Anope 1.8.7 + UnrealIRCd 3.2.8 & UnrealIRCD 3.2.9
 * -----------------------------------------------------------------------------
 * This module creates the /os redirect command.
 * The command syntax is /os redirect user #from_channel #to_channel
 * This command makes the user part the #from_channel and join the #to_channel.
 * 
 * -----------------------------------------------------------------------------
 * Changelog:
 *
 *   1.0 - Initial Module Release
 *
 *   
 *
 * -----------------------------------------------------------------------------
 **/


#include "module.h"
#define AUTHOR "VisioN"
#define VERSION "1.0"
 
 int do_redirect (User *u);
 
 int AnopeInit(int argc, char **argv) {
     /* We create the command we're going to use */
     int req_access = 1;
     Directive conf[]= 
     {
               {"RedirectAccess",{ { PARAM_INT, PARAM_RELOAD, &req_access } } }
     };

     moduleGetConfigDirective(conf);
     
     Command *c;
     if ((req_access!=0) && (req_access!=1) && (req_access!=2)) req_access=1;
     if (req_access==0)
     {                  
          c = createCommand("redirect", do_redirect,NULL, -1, -1, -1, -1, -1);
          alog("Loading os_redirect with restriction to noone");
     }
     else if (req_access==1)
     { 
          c = createCommand("redirect", do_redirect,is_services_oper, -1, -1, -1, -1, -1);
          alog("Loading os_redirect with restriction to services operators");
     }
     else if (req_access==2) 
     {
          c = createCommand("redirect", do_redirect,is_services_admin, -1, -1, -1, -1, -1);
          alog("Loading os_redirect with restriction to services administrators");
     }
          
 
     /* We add the command to HelpServ and log it */
     alog("os_redirect: Add command 'redirect' status: %d", moduleAddCommand(OPERSERV, c, MOD_HEAD));
 
     /* We tell Anope who we are and what version this module is */
     moduleAddAuthor(AUTHOR);
     moduleAddVersion(VERSION);
 
     /* Loading succeeded */
     return MOD_CONT;
 }
 
 void AnopeFini(void) {
     /* Nothing to clean up */
 }
 
 int do_redirect (User *u) {
     char *buf = moduleGetLastBuffer();
     User *target= NULL;
     Channel *c;
     ChannelInfo *ci;
     char *nick = myStrGetToken (buf,' ', 0);
     char *chan1 = myStrGetToken (buf,' ', 1);
     char *chan2 = myStrGetToken (buf,' ', 2);
     
     if (!u ||!chan1||!nick||!chan2)
     {
                  if (nick) free(nick);
                  if (chan1) free(chan1);
                  if (chan2) free(chan2);
                  return MOD_STOP;
     }
     if (!(target = finduser(nick))) 
     {
                  notice (s_OperServ, u->nick,"No such user to redirect");
                  if (nick) free(nick);
                  if (chan1) free(chan1);
                  if (chan2) free(chan2);
                  return MOD_STOP;
     }
     if (!(ci = cs_findchan(chan1)))
     {
              if (nick) free(nick);
              if (chan1) free(chan1);
              if (chan2) free(chan2);
              return MOD_STOP;
     }
     if (!(c = findchan(ci->name)))
     {
                 notice (s_OperServ, u->nick,"No such channel to be redirected from.");
                 if (nick) free(nick);
                 if (chan1) free(chan1);
                 if (chan2) free(chan2);
                 return MOD_STOP;        
     }
     if (!(is_on_chan(c,target)))
     {
                 notice (s_OperServ, u->nick,"User %s is not in this channel", target->nick);
                 if (nick) free(nick);
                 if (chan1) free(chan1);
                 if (chan2) free(chan2);
                 return MOD_STOP;
     }
    
    notice (s_OperServ, u->nick,"Redirecting user %s from channel %s to channel %s",target->nick,chan1,chan2);
    wallops(c->ci->bi->nick, "%s redirected %s from channel %s to channel %s .", u->nick, nick, c->name , chan2); 
	anope_cmd_svspart(c->ci->bi->nick, nick, c->name);
	anope_cmd_svsjoin(s_OperServ, target->nick, chan2, NULL);
    notice (s_OperServ, target->nick,"You have been redirected from channel %s to channel %s by operator %s", chan1,chan2,u->nick);
    if (nick) free(nick);
    if (chan1) free(chan1);
    if (chan2) free(chan2);
 
     /* Halt processing */
     return MOD_STOP;
 }
 
