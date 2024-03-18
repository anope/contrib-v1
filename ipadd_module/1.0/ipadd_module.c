/* IPADD written for use on ParanoiaNet.com
   Author retains all original rights to work.
   You may modify/redistribute this code under
   the GPL agreement provided you maintain 
   Author's credit.
*/

#include "module.h"
#include <stdio.h>

#define AUTHOR "David Chafin"
#define VERSION "v1.0a"

/*Function Declaration */
int ip_add(int argc, char **argv);


int AnopeInit(int argc, char **argv)
{
    EvtHook *hook;       
    int status;          

    hook = createEventHook(EVENT_BOT_FANTASY, ip_add);
    status = moduleAddEventHook(hook);

    if (status != MOD_ERR_OK) {
        alog("Error binding to event EVENT_BOT_FANTASY [%d]", status);
        return MOD_STOP;
    }

	hook = createEventHook(EVENT_BOT_FANTASY_NO_ACCESS, ip_add);
	status = moduleAddEventHook(hook);
    if (status != MOD_ERR_OK) {
        alog("Error binding to event EVENT_BOT_FANTASY [%d]", status);
        return MOD_STOP;
    }

    moduleAddAuthor(AUTHOR);       /* Add the author information  */
    moduleAddVersion(VERSION);     /* Add the version information */
    moduleSetType(THIRD);          /* Flag as Third party module  */

    /* Loading Success! */
    alog("ipadd successfully loaded");
    return MOD_CONT;
}

/*************************************************************************/

void AnopeFini(void)
{
 /* A message about unload */
 alog("ipadd unloaded");
}

/*************************************************************************/

int ip_add(int argc, char **argv)
{
    User *u;
    ChannelInfo *ci;
    FILE * pFile;
    /* Set below to actual allow.conf file to be used */
    pFile = fopen ("/home/dave/pfile.cf","a");
                          

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

    if (!stricmp("ipadd", argv[0])) {
     if (is_services_admin(u)) {
       if ( argc == 4 ) {
           if ( pFile != NULL )
               {
                fprintf(pFile, "#Added by %s\n", u->nick);
                fprintf(pFile, "allow {\n");
                fprintf(pFile, "      ip        %s;\n", argv[3]);
                fprintf(pFile, "      hostname  *@*;\n");
                fprintf(pFile, "      class     clients;\n");
                fprintf(pFile, "      maxperip  3;\n");
                fprintf(pFile, "};\n");
                fclose (pFile);
                notice(whosends(ci), ci->name, "!ipadd used by %s: adding: %s", 
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

