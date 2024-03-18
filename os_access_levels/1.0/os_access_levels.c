#include "module.h"

#ifdef _WIN32
static Command *myFindCommand(CommandHash * cmdTable[], const char *name)
{
	int idx;
	CommandHash *current = NULL;
	
	if (!cmdTable || !name)
		return NULL;

	idx = CMD_HASH(name);

	for (current = cmdTable[idx]; current; current = current->next)
	{
		if (stricmp(name, current->name) == 0)
			return current->c;
	}

	return NULL;
}

#define findCommand myFindCommand
#endif

static const char *ConfigValues;
static int do_reload(int argc, char **argv);
static void LoadConfig();
static void ProcessConfig();

int AnopeInit(int argc, char **argv)
{
	EvtHook *evt;

	moduleAddAuthor("Adam");
	moduleAddVersion("1.0");
	moduleSetType(THIRD);

	LoadConfig();
	ProcessConfig();
	
	evt = createEventHook(EVENT_RELOAD, do_reload);
	moduleAddEventHook(evt);

	return MOD_CONT;
}

void AnopeFini()
{
}

static int do_reload(int argc, char **argv)
{
	LoadConfig();
	ProcessConfig();
	return MOD_CONT;
}

void LoadConfig()
{
	Directive confvalues[][1] = {
		{{"OSAccessLevels", {{PARAM_STRING, PARAM_RELOAD, &ConfigValues}}}}
	};

	moduleGetConfigDirective(confvalues[0]);
}

void ProcessConfig()
{
	int i = -1;
	char *c, *LevelS;
	int Level;

	while ((c = myStrGetToken(ConfigValues, ' ', ++i)))
	{
		Command *cmd = findCommand(OPERSERV, c);
		if (cmd)
		{
			LevelS = myStrGetToken(ConfigValues, ' ', ++i);
			if (!LevelS)
				break;
			Level = atoi(LevelS);
			free(LevelS);

			if (Level == 0)
				cmd->has_priv = NULL;
			else if (Level == 1)
				cmd->has_priv = is_oper;
			else if (Level == 2)
				cmd->has_priv = is_services_oper;
			else if (Level == 3)
				cmd->has_priv = is_services_admin;
			else if (Level == 4)
				cmd->has_priv = is_services_root;
			if (debug)
				alog("[os_access_levels]: Tied command %s to level %i", cmd->name, Level);
		}

		free(c);
	}
}

