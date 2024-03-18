/* BAN_IP  written for use on DreamChat.fr
   Author retains all original rights to work.
   You may modify/redistribute this code under
   the GPL agreement provided you maintain 
   Author's credit.
	This code become from ipadd_module 
	I thank his author David Chafin for this little module which enables me to create my first
   */

#include "module.h"
#include <stdio.h>

#define AUTHOR "EZEki3l"
#define VERSION "v1.0"

/*Function Declaration */
int ban_ip(int argc, char **argv);

int AnopeInit(int argc, char **argv)
{
    EvtHook *hook;       
    int status;          

    hook = createEventHook(EVENT_BOT_FANTASY, ban_ip);
    status = moduleAddEventHook(hook);

    if (status != MOD_ERR_OK) {
        alog("Error binding to event EVENT_BOT_FANTASY [%d]", status);
        return MOD_STOP;
    }

	hook = createEventHook(EVENT_BOT_FANTASY_NO_ACCESS, ban_ip);
	status = moduleAddEventHook(hook);
    if (status != MOD_ERR_OK) {
        alog("Error binding to event EVENT_BOT_FANTASY [%d]", status);
        return MOD_STOP;
    }

    moduleAddAuthor(AUTHOR);       /* Add the author information  */
    moduleAddVersion(VERSION);     /* Add the version information */
    moduleSetType(THIRD);          /* Flag as Third party module  */

    /* Loading Success! */
    alog("ban_ip successfully loaded");
    return MOD_CONT;
}

/*************************************************************************/

void AnopeFini(void)
{
 /* A message about unload */
 alog("ban_ip unloaded");
}

/*************************************************************************/

int ban_ip(int argc, char **argv)
{
    User *u;
    ChannelInfo *ci;
    FILE * pFile;
    /* Set below to actual blacklist.conf file to be used */
    pFile = fopen ("/home/EZEki3l/Unreal3.2/blacklist.conf","a");
                          

    /* Check that we got at least 3 parameters */
    if (argc < 3) {
      /* return now */
      return MOD_CONT;
    }

    /* Get the user struct */
    u = finduser(argv[1]);
    /* Get the channel info struct */
    ci = cs_findchan(argv[2]);

    /* Make sure they aren't null */
    if (!u || !ci) {
       return MOD_CONT;
    }

    if (!stricmp("banip", argv[0])) {
     if (is_services_admin(u)) {
       if ( argc == 4 ) {
           if ( pFile != NULL )
               {
                fprintf(pFile, "#Added by %s\n", u->nick);
                fprintf(pFile, "ban ip {\n");
                fprintf(pFile, "      mask        %s;\n", argv[3]);
                fprintf(pFile, "      reason  \"You are not allowed on this network\";\n");
                fprintf(pFile, "};\n");
                fclose (pFile);
                notice(whosends(ci), ci->name, "!banip used by %s: adding: %s", 
                       u->nick, argv[3]);
                return MOD_CONT;
               }
            else {
               alog("Could not open file! Aborting!");
               notice(whosends(ci), u->nick, "No such file exists!");
               return MOD_STOP;
            }
            
           }
       else {
           notice(whosends(ci), u->nick, "You must supply an IP.");
           return MOD_STOP;
           }
    }
     else {
           notice(whosends(ci), u->nick, "Permission denied.");
           return MOD_STOP;
      }
     }
     
    

    /* All Done */
    return MOD_CONT;
}

