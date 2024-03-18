/*
 * Written by Adam (Adam@Anope.org)
 *
 * This module lets you change the default channel levels of newly registered channels.
 * There are 36 optional configuration settings this module adds:
 *
 * AUTODEOP			[Level]
 * AUTOHALFOP		[Level]
 * AUTOOP			[Level]
 * AUTOPROTECT		[Level]
 * AUTOVOICE		[Level]
 * NOJOIN			[Level]
 * SIGNKICK			[Level]
 * ACCLIST			[Level]
 * ACCCHANGE		[Level]
 * AKICK			[Level]
 * SET				[Level]
 * BAN				[Level]
 * BANME			[Level]
 * CLEAR			[Level]
 * GETKEY			[Level]
 * HALFOP			[Level]
 * HALFOPME			[Level]
 * INFO				[Level]
 * KICK				[Level]
 * KICKME			[Level]
 * INVITE			[Level]
 * OPDEOP			[Level]
 * OPDEOPME			[Level]
 * PROTECT			[Level]
 * PROTECTME		[Level]
 * TOPIC			[Level]
 * UNBAN			[Level]
 * VOICE			[Level]
 * VOICEME			[Level]
 * MEMO				[Level]
 * ASSIGN			[Level]
 * BADWORDS			[Level]
 * FANTASIA			[Level]
 * GREET			[Level]
 * NOKICK			[Level] 
 * SAY				[Level]
 * 
 * Not defining one of these in your configuration will have no effect, this module will leave
 * it as the default Anope level.
 * If you want something to be owner only (disabled), make the level 10000
 * Note: A level can not be 0!
 */

#include "module.h"

#define AUTHOR "Adam"
#define VERSION "1.0"

int AUTODEOP, AUTOHALFOP, AUTOOP, AUTOPROTECT, AUTOVOICE, NOJOIN, SIGNKICK;
int ACCLIST, ACCCHANGE, AKICK, SET, BAN, BANME, CLEAR, GETKEY, HALFOP;
int HALFOPME, INFO, KICK, KICKME, INVITE, OPDEOP, OPDEOPME, PROTECT, PROTECTME;
int TOPIC, UNBAN, VOICE, VOICEME, MEMO, ASSIGN, BADWORDS, FANTASIA, GREET, NOKICK, SAY;

int do_reload(int argc, char **argv);
int do_register(int argc, char **argv);
void loadConfig(void);
void setLevels(ChannelInfo *ci);

typedef struct _myLevels myLevels;

struct _myLevels {
	char *name;
	int *var;
};

myLevels Levels[] = {
	{"AUTODEOP", &AUTODEOP},
	{"AUTOHALFOP", &AUTOHALFOP},
	{"AUTOOP", &AUTOOP},
	{"AUTOPROTECT", &AUTOPROTECT},
	{"AUTOVOICE", &AUTOVOICE},
	{"NOJOIN", &NOJOIN},
	{"SIGNKICK", &SIGNKICK},
	{"ACC-LIST", &ACCLIST},
	{"ACC-CHANGE", &ACCCHANGE},
	{"AKICK", &AKICK},
	{"SET", &SET},
	{"BAN", &BAN},
	{"BANME", &BANME},
	{"CLEAR", &CLEAR},
	{"GETKEY", &GETKEY},
	{"HALFOP", &HALFOP},
	{"HALFOPME", &HALFOPME},
	{"INFO", &INFO},
	{"KICK", &KICK},
	{"KICKME", &KICKME},
	{"INVITE", &INVITE},
	{"OPDEOP", &OPDEOP},
	{"OPDEOPME", &OPDEOPME},
	{"PROTECT", &PROTECT},
	{"PROTECTME", &PROTECTME},
	{"TOPIC", &TOPIC},
	{"UNBAN", &UNBAN},
	{"VOICE", &VOICE},
	{"VOICEME", &VOICEME},
	{"MEMO", &MEMO},
	{"ASSIGN", &ASSIGN},
	{"BADWORDS", &BADWORDS},
	{"FANTASIA", &FANTASIA},
	{"GREET", &GREET},
	{"NOKICK", &NOKICK},
	{"SAY", &SAY},
	{ NULL }
};

int AnopeInit(int argc, char **argv) {
	EvtHook *evt;

	evt = createEventHook(EVENT_RELOAD, do_reload);
	moduleAddEventHook(evt);

	evt = createEventHook(EVENT_CHAN_REGISTERED, do_register);
	moduleAddEventHook(evt);

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	loadConfig();

	return MOD_CONT;
}

void AnopeFini(void) {

}

int do_reload(int argc, char **argv) {
	loadConfig();
	return MOD_CONT;
}

int do_register(int argc, char **argv) {
	ChannelInfo *ci;

	if (!argc)
		return MOD_STOP;

	ci = cs_findchan(argv[0]);

	if (ci)
		setLevels(ci);

	return MOD_CONT;
}

void loadConfig(void) {
	int x;

	for (x = 0; Levels[x].name != NULL; x++) {
		Directive confvalue[][1] = {
			{{Levels[x].name, {{PARAM_INT, PARAM_RELOAD, Levels[x].var}}}}
		};
		moduleGetConfigDirective(confvalue[0]);

		if (debug)
			alog("debug: cs_default_levels: loadConfig() detected %s to be %i", Levels[x].name, *Levels[x].var);
	}
}

void setLevels(ChannelInfo *ci) {
	int x, y;

	for (x = 0; Levels[x].name != NULL; x++) {
		if (*Levels[x].var) {
			for (y = 0; levelinfo[y].what >= 0; y++) {
				if (!stricmp(levelinfo[y].name, Levels[x].name)) {
					if (*Levels[x].var == ACCESS_FOUNDER)
						ci->levels[levelinfo[y].what] = ACCESS_INVALID;
					else
						ci->levels[levelinfo[y].what] = *Levels[x].var;
				}
			}
		}
	}
}
