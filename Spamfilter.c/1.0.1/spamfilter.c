/* spamfilter.c written for use on chatter.lu network
   Author retains all original rights to work.
   You may modify/redistribute this code under
   the GPL agreement provided you maintain 
   Author's credit.
   This code become from ipadd_module. 
   I thank his author David Chafin for this little module which      enables me to create my first module inspired by David Chafin.*/

#include "module.h"
#include <stdio.h>

#define AUTHOR "irc0p"
#define VERSION "v1.0"

/*Function Declaration */
int spamfilter(int argc, char **argv);

int AnopeInit(int argc, char **argv) {
EvtHook *hook;       
int status;          

hook = createEventHook(EVENT_BOT_FANTASY, spamfilter);
status = moduleAddEventHook(hook);

if (status != MOD_ERR_OK) {
alog("Error binding to event EVENT_BOT_FANTASY [%d]", status);
return MOD_STOP;
}

hook = createEventHook(EVENT_BOT_FANTASY_NO_ACCESS, spamfilter);
status = moduleAddEventHook(hook);

if (status != MOD_ERR_OK) {
alog("Error binding to event EVENT_BOT_FANTASY [%d]", status);
return MOD_STOP;
}


moduleAddAuthor(AUTHOR);       /* Add the author information  */
moduleAddVersion(VERSION);     /* Add the version information */
moduleSetType(THIRD);          /* Flag as Third party module  */

/* Loading Success! */
alog("Spamfilter modul successfully loaded.");
return MOD_CONT;
}

/**************************************************************/

void AnopeFini(void) {
/* A message about unload */
alog("Spamfilter modul successfully unloaded.");
}

/**************************************************************/

int spamfilter(int argc, char **argv) {
User *u;
ChannelInfo *ci;
FILE * pFile;

/* Set below to actual spamfilter.conf file to be used */
pFile = fopen ("/home/your_irc/Unreal3.2/spamfilter.conf","a");

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

if (!stricmp("spamfilter", argv[0])) {
if (is_services_admin(u)) {
if ( argc == 4 ) {
if ( pFile != NULL ) {

fprintf(pFile, "#Added by %s\n", u->nick);
fprintf(pFile, "spamfilter {\n");
fprintf(pFile, "regex \"%s\";\n", argv[3]);
fprintf(pFile, "target { private; channel; };\n");
fprintf(pFile, "action block;\n");
fprintf(pFile, "reason \"No spam on our network!\";\n");
fprintf(pFile, "};\n");
fprintf(pFile, "\n");
fclose (pFile);

system ("/home/ircd/Unreal3.2/./unreal rehash");   

notice(whosends(ci), ci->name, "!spamfilter used by %s: adding the following URL: %s. Server will be rehasht automatically!", 


u->nick, argv[3]);
return MOD_CONT;

}else{

alog("Could not open spamfilter.conf! Aborting.");
notice(whosends(ci), u->nick, "No such file exists!");
return MOD_STOP;
}

}else{

notice(whosends(ci), u->nick, "You must supply an #channel.");
return MOD_STOP;
}

}else{

notice(whosends(ci), u->nick, "Permission denied.");
return MOD_STOP;
 }
}

/* All Done */
return MOD_CONT;

}




