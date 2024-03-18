#include "module.h"

#define AUTHOR "Thomas Edwards (TMFKSOFT)"
#define VERSION "1.0.0"

/*
Add the following lines to services.conf

# 	NickServ Welcome
# WELCOME_MSG <message>  [REQUIRED]
#     The link defined wil be returned to a user upon /msg HelpServ TOS
NS_WELCOME "Thanks for registering! If you require support join #help!"

*/

/*
NS_WELCOME v1.0
	Module created!

 Based from he_tos
*/

/* Create variables. */
char *welcome_msg = NULL;

/* Create functions */
int reload_config(int argc, char **argv);
int load_config(void);
int welcome_user(int argc, char **argv);


int AnopeInit(int argc, char **argv) {
    EvtHook    *hook = NULL;
	/* Hook into the Nickname regisration event */
	hook = createEventHook(EVENT_RELOAD, reload_config);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002ns_welcome\002] Can't hook to EVENT_RELOAD event");
		return MOD_STOP;
	}
	hook = createEventHook(EVENT_NICK_REGISTERED, welcome_user);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002ns_welcome\002] Can't hook to EVENT_NICK_REGISTERED event");
		return MOD_STOP;
	}
	
	/* Lets load from the config. */
	if (!load_config())
		return MOD_STOP;

    /* We tell Anope who we are and what version this module is */
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);

    /* Loading succeeded */
    return MOD_CONT;
}

void AnopeFini(void) {
	if(welcome_msg) 
		free(welcome_msg);
	welcome_msg = NULL;
	alog("[\002ns_welcome\002] Module unloaded.");
}

int welcome_user(int argc, char **argv) {
	if(welcome_msg) {
		notice(s_NickServ, argv[0],welcome_msg);
		alog("%s recieved the Nickserv Welcome Message.",argv[0]);
	}
}

int load_config(void) {
	/* Load the config */
	Directive confvalues[] = {
		{ "NS_WELCOME", { { PARAM_STRING, PARAM_RELOAD, &welcome_msg } } }
	};
	moduleGetConfigDirective(confvalues);
	if (welcome_msg) {
		alog("[\002ns_welcome\002] Nickserv welcome is set to '%s' in the config, config loaded! :D", welcome_msg);
		return true;
	} else {
		alog("[\002ns_welcome\002] Nickserv welcome is not set! Set it and reload the config! ~Commits suicide~");
		return false;
	}
}

int reload_config(int argc, char **argv) {
	/* services.conf reloaded. Reload directives. */
	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			alog("[\002ns_welcome\002] Reloading configuration directives...");
			if(welcome_msg) 
				free(welcome_msg);
			welcome_msg = NULL;
			load_config();
		}
	}
	return MOD_CONT;
}
/* EOF */