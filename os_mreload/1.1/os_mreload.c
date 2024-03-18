/** os_mreload - Reloads a module.
 *
 * Maintainer/Author: Taros
 * Date: 2009-02-09
 *  
 * Unloads and then loads a module of your choice.
 * No more modunloading and then modloading the same module.
 * Written for Anope 1.9 by Taros on the Tel'Laerad M&D Team
 * <http://tellaerad.net>
 */
/******************************************************************************/

#include "module.h"

#define AUTHOR "Taros"
#define VERSION "1.1"

void myHelp(User * u);
void myFullHelpSyntax(User * u);
int myFullHelp(User * u);

int do_mreload(User * u);

void mAddLanguages();

static Module *me;

#define LANG_NUM_STRINGS	3
#define MRELOAD_HELP		   0
#define MRELOAD_SYNTAX		 1
#define MRELOAD_HELP_DETAIL	2

class OSMReload : public Module
{
 public:
	OSMReload(const std::string &modname, const std::string &creator) : Module(modname, creator)
	{
		Command *c;

		me = this;

		this->SetOperHelp(myHelp);
		c = createCommand("MRELOAD", do_mreload, is_services_root, -1, -1, -1, -1, -1);
		moduleAddHelp(c, myFullHelp);
		this->AddCommand(OPERSERV, c, MOD_HEAD);

		this->SetAuthor(AUTHOR);
		this->SetVersion(VERSION);
                this->SetPermanent(true);

		const char* langtable_en_us[] = {
		"    MRELOAD    Reload a module.",
		"Syntax: MRELOAD module",
		"Unloads and reloads the given module.",
		};

		this->InsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
	}
};

void myHelp(User * u)
{
	me->NoticeLang(s_OperServ, u, MRELOAD_HELP);
}

void myFullHelpSyntax(User * u)
{
	me->NoticeLang(s_OperServ, u, MRELOAD_SYNTAX);
}

int myFullHelp(User * u)
{
	myFullHelpSyntax(u);
	ircdproto->SendMessage(findbot(s_OperServ), u->nick, " ");
	me->NoticeLang(s_OperServ, u, MRELOAD_HELP_DETAIL);
	return MOD_CONT;
}

int do_mreload(User * u)
{

	char *name;
	int status;

	name = strtok(NULL, "");

	if (!name)
	{
		notice_user(s_OperServ, u, "Syntax: MRELOAD module");
		return MOD_CONT;
	}

	Module *m = findModule(name);
	if (!m)
	{
		notice_user(s_OperServ, u, "You need to specify a loaded module.");
		return MOD_CONT;
	}

	alog("Trying to unload module [%s]", name);

	status = ModuleManager::UnloadModule(m, u);

	if (status == MOD_ERR_NOUNLOAD)
	{
		return MOD_CONT;
	}

	if (status != MOD_ERR_OK)
	{
		notice_lang(s_OperServ, u, OPER_MODULE_REMOVE_FAIL, name);
		return MOD_CONT;
	}

	status = ModuleManager::LoadModule(name, u);
	if (status != MOD_ERR_OK)
	{
		notice_lang(s_OperServ, u, OPER_MODULE_LOAD_FAIL, name);
	}

	return MOD_CONT;

}

MODULE_INIT("os_mreload", OSMReload)
