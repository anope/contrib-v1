#include "module.h"

#define AUTHOR "Thomas Edwards (TMFKSOFT)"
#define VERSION "1.1.0"

/*
Add the following lines to services.conf

# 	HelpServ TOS Module
# TOS <tos_link>  [REQUIRED]
#     The link defined wil be returned to a user upon /msg HelpServ TOS
TOS "http://mynet.net/tos.php"

*/

/*
HE_TOS v1.0
	Module created!

HE_TOS v1.1
	Changes:
		+ General cleaning up, corrected any grammar mistakes.
		+ The module now displays the command in HelpServs "HELP" command.
		+ The module now loads from services.conf
	TODO:
		Add !tos to fantasy.
		
	Errors on linux fixed by Fleck.
*/

/* Create variables. */
char *tos_conf = NULL;
char *setting = NULL;

/* Create functions */
int tos(User *u);
void he_tos_help(User *u);
int he_tos_help_syntax(User *u);
int reload_config(int argc, char **argv);
int load_config(void);


int AnopeInit(int argc, char **argv) {
    EvtHook    *hook = NULL;
	/* We create the command we're going to use */
    Command *c;
    c = createCommand("tos", tos, NULL, -1, -1, -1, -1, -1);

	hook = createEventHook(EVENT_RELOAD, reload_config);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002he_tos\002] Can't hook to EVENT_RELOAD event");
		return MOD_STOP;
	}
	
	/* Lets load from the config. */
	if (!load_config())
		return MOD_STOP;

    /* We add the command to HelpServ and log it */
    alog("[\002he_tos\002] Add command 'TOS' status: %d", moduleAddCommand(HELPSERV, c, MOD_HEAD));

	/* We say HelpServ to allow our function to be in the HELP too! */
    moduleSetHelpHelp(he_tos_help);
	moduleAddHelp(c,he_tos_help_syntax);
	
    /* We tell Anope who we are and what version this module is */
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);

    /* Loading succeeded */
    return MOD_CONT;
}

int tos(User *u) {
    /* Return TOS Link */
    notice(s_HelpServ, u->nick, "\002%s\002 you can find the Terms Of Service at \002%s\002", u->nick, tos_conf);
	alog("[\002he_tos\002] %s had a peek at the TOS link!", u->nick);
	
    /* Halt processing */
    return MOD_CONT;
}

void he_tos_help(User *u) {
	/* Spit out the TOS help in HelpServ's "HELP" */
	notice(s_HelpServ, u->nick, "\002/msg HelpServ TOS\002");
    notice(s_HelpServ, u->nick, "     Returns IRC Network TOS Link.");
}

int he_tos_help_syntax(User *u) {
	/* Spit out the TOS syntax help in HelpServ's "HELP TOS" */
    notice(s_HelpServ, u->nick, "Syntax: \002TOS\002");
	notice(s_HelpServ, u->nick, " ");
    notice(s_HelpServ, u->nick, "This command returns the IRC networks");
    notice(s_HelpServ, u->nick, "TOS link to the user.");
	return MOD_STOP;
}

void AnopeFini(void) {
	if(setting) 
		free(setting);
	alog("[\002he_tos\002] Module unloaded.");
}
int load_config(void) {
	int moduleGetConfigDirective(Directive *d);
	Directive confvalues[] = {
		{ "TOS", { { PARAM_STRING, PARAM_RELOAD, &tos_conf } } }
	};
	moduleGetConfigDirective(confvalues);
	if (tos_conf) {
		alog("[\002he_tos\002] TOS link is set to '%s' in the config, loaded! :D", tos_conf);
		return true;
	} else {
		alog("[\002he_tos\002] TOS Link is not set! Set it and reload the config! ~Commits suicide~");
		return false;
	}
}

int reload_config(int argc, char **argv) {
	/* services.conf reloaded. Reload directives. */
	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			alog("[\002he_tos\002] Reloading configuration directives...");
			load_config();
		}
	}
	return MOD_CONT;
}
/* EOF */