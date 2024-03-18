/*************************************************************************
         bs_fantasy_kickreason
         
**  NOTICE ** This module is still in a beta testing stage. It *should* work
              fine, but don't whinge if it crashes!
   ___________________________________________________
                      INFORMATION
   ___________________________________________________ 
   Module Author: VisioN                               
   Module Version: 1.0-beta                             
   Tested: UnrealIRCd3.2.9-rc2 + Anope 1.8.7-git       
   Support: PM VisioN @ irc.anope.org or e-mail vision@myirc.net
   ___________________________________________________
                      DESCRIPTION
   ___________________________________________________
   This module allows BotServ to kick users from a channel
   with a predefined reason, making kicking faster for the
   channel operators , and ensuring some kind of consistency
   as all the channel operators can kick with the same reasons.
   ___________________________________________________
                       SYNTAX   
   ___________________________________________________
    Syntax:  !kickr [NICK] [KICKER]   or
             !kr [NICK] [KICKER]
    (for example:   !kickr VisioN badlang  or 
                    !kr VisioN badlang  )
    Bellow is a list of all the kickers you can use
   ___________________________________________________
                      KICKERS LIST
   ___________________________________________________
           NAME              DESCRIPTION
           badlang           Kicks for bad language (insults etc)
           caps              Kicks for capital character usage
           flood             Kicks for flooding
           bold              Kicks for bold character usage
           advertise         Kicks for SPAM/advertising
           repeat            Kicks for repetition
           idle              Kicks for idling (useful for helpchans)
           badnick           Kicks for bad nickname
   ___________________________________________________
                       INSTALLATION
   ___________________________________________________
    1st step: Edit the kick reasons for each kicker according
    to your needs:
     -------- start editing --------                                            */
     #define BADLANG "Do not use that kind of language in this channel."
     #define CAPS "Don not use CAPITAL letters in this channel."
     #define FLOOD "Flooding is not allowed in the channel. Use a pastebin!"
     #define BOLD "Don not use BOLD letters in the channel."
     #define ADVERTISE "Spam/Advertising is not allowed in this channel."
     #define REPEAT "Do not repeat yourself, one time is enough!"
     #define IDLE "Idling is not allowed in this channel."
     #define BADNICK "Choose a more polite nickname and rejoin the channel."
/*   -------- stop editing --------
     2nd step: Place the module (bs_fantasy_kickreason.c) in anope* folder
     and compile as usual. Then /os modload bs_fantasy_kickreason
   ___________________________________________________
                        FINAL NOTES
   ___________________________________________________   
   This is my first attempt to make a module so the code
   might not be as good as you'd expect. I really want to
   say a BIG BIG thanks to Viper from the Anope Team who
   guided me through the whole process of creating the module
   ___________________________________________________
                           TODO 
   ___________________________________________________
   - Add the command !bkickr and !bkr for ban along with the kick
   - Add configuration directive to make ALL kickers and kickreasons customisable
   - Everything else that you guys suggest and I can code ;)
/********************************************************/
/*    DON'T CHANGE ANYTHING BELOW THIS POINT      */


#include "module.h"
#define AUTHOR "VisioN"
#define VERSION "VisioN 1.0-beta"

/* functions */
int fantasy_defkick(int argc, char **argv);
int fantasy_defkick_help(int ac, char **av);

int AnopeInit(int argc, char **argv)
{
    EvtHook *hook;

    moduleAddAuthor(AUTHOR);
    moduleAddVersion (VERSION);

    hook = createEventHook(EVENT_BOT_FANTASY, fantasy_defkick);
    moduleAddEventHook(hook);

    return MOD_CONT;
}

/* clean up leftovers when unloading the module */
void AnopeFini(void)
{
}

/**
 * Handle kickr/kr fantasy commands.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT or MOD_STOP
 **/

int fantasy_defkick(int argc, char **argv)
{
    User *u, *u2;
    ChannelInfo *ci;
    char *target = NULL;
    char *reason = NULL;

    if (argc < 3)
        return MOD_CONT;
    if (!stricmp(argv[0], "help")) 
    {
            u = finduser(argv[1]);
       		if (argc >= 4) 
            {
			   int ret = MOD_CONT;
			   char *cmd, *param;
			   cmd = myStrGetToken(argv[3],' ',0);
			   param = myStrGetToken(argv[3],' ',1);
			   if (!stricmp(cmd, "kickr")) 
               {
                  notice(s_BotServ, u->nick, "\037Syntax\037: \002!kickr [NICK] [TRIGGER]\002   or");
                  notice(s_BotServ, u->nick, "\037Syntax\037: \002!kr [NICK] [TRIGGER]\002");
                  notice(s_BotServ, u->nick, "(For example: !kr VisioN idling  or !kickr VisioN idling)");
                  notice(s_BotServ, u->nick, "\n");
                  notice(s_BotServ, u->nick, "----------------------------------------------------------------------------------------------------------");
                  notice(s_BotServ, u->nick, "\n");
                  notice(s_BotServ, u->nick, "\037Description\037: This command allows you to kick a user from a channel, using the bot's default kick reasons.");
                  notice(s_BotServ, u->nick, "The bot supports multipule kick reasons for each of the basic abuses for channels, a list of which");
                  notice(s_BotServ, u->nick, "you can find below.");
                  notice(s_BotServ, u->nick, "\n");
                  notice(s_BotServ, u->nick, "List of triggers:");
                  notice(s_BotServ, u->nick, "\037\002Name\002\037           \037\002Description\002\037");
                  notice(s_BotServ, u->nick, "badlang        Kicks for bad language (insults etc)");
                  notice(s_BotServ, u->nick, "caps             Kicks for capital letters ");
                  notice(s_BotServ, u->nick, "flood            Kicks for channel flooding ");
                  notice(s_BotServ, u->nick, "bold             Kicks for bold letters ");
                  notice(s_BotServ, u->nick, "advertise      Kicks for Spam/Advertising ");
                  notice(s_BotServ, u->nick, "repeat          Kicks for repetition ");
                  notice(s_BotServ, u->nick, "idle               Kicks for idling (mostly for help chans) ");
                  notice(s_BotServ, u->nick, "badnick        Kicks for bad nicknames ");
                  ret = MOD_CONT;
               }
			free(cmd);
			if (param) free(param);
			return ret;
            }
   }
   else if ((stricmp(argv[0], "kickr") == 0) || (stricmp(argv[0], "kr") == 0)) 
   {
        u = finduser(argv[1]);
        ci = cs_findchan(argv[2]);
        if (!u || !ci)
            return MOD_CONT;
        if (argc >= 4) 
        {
            target = myStrGetToken(argv[3], ' ', 0);
            reason = myStrGetTokenRemainder(argv[3], ' ', 1);
        }
        if (!check_access(u, ci, CA_KICK)) 
        {
            notice(s_BotServ, u->nick, "You are not authorised to kick the selected user.");
        }
        else if (!target && check_access(u, ci, CA_KICKME)) 
        {
            notice(s_BotServ, u->nick, "\037Syntax\037: \002!kickr [NICK] [TRIGGER]\002   or");
            notice(s_BotServ, u->nick, "\037Syntax\037: \002!kr [NICK] [TRIGGER]\002");
            notice(s_BotServ, u->nick, "For detailed information about this command and");
            notice(s_BotServ, u->nick, "for a list of triggers, type \002!help kickr\002");
        } 
        else if (target && check_access(u, ci, CA_KICK)) 
        {
            if (!stricmp(target, ci->bi->nick))
                bot_raw_kick(u, ci, u->nick, "Wrong Move!");
            else 
            {
                u2 = finduser(target);
                if (u2 && ci->c && is_on_chan(ci->c, u2)) 
                {   
                    if (!reason && !is_protected(u2))
                        bot_raw_kick(u, ci, target, "Requested");
                    else if (!is_protected(u2)) 
                    {
                         if ((stricmp(reason , "badlang") == 0))  
                            bot_raw_kick(u, ci, target, BADLANG);
			             else if ((stricmp(reason , "caps") == 0)) 
                              bot_raw_kick(u, ci, target, CAPS);
			             else if ((stricmp(reason , "flood") == 0)) 
				              bot_raw_kick(u, ci, target, FLOOD);
			             else if ((stricmp(reason , "bold") == 0))
				              bot_raw_kick(u, ci, target, BOLD);
                         else if ((stricmp(reason , "advertise") == 0)) 
				              bot_raw_kick(u, ci, target, ADVERTISE);
	                     else if ((stricmp(reason , "repeat") == 0)) 
				              bot_raw_kick(u, ci, target, REPEAT);
	                     else if ((stricmp(reason , "idle") == 0)) 
				              bot_raw_kick(u, ci, target, IDLE);
	                     else if ((stricmp(reason , "badnick") == 0)) 
				              bot_raw_kick(u, ci, target, BADNICK);

                     else     
                     {
                           notice(s_BotServ, u->nick, "The number you chose does not correspond to a kick reason.");
                           notice(s_BotServ, u->nick, "If you want to kick with your own custom reason , use !kick trigger");
                     }
                 }      
                    
              }
            }
        }
    }
    if (target) free(target);
    if (reason) free(reason);
    return MOD_CONT;
}

/*  EOF  */
