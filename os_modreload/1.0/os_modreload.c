#include "module.h"

#define AUTHOR      "hmtX^mokkori"
#define VERSION     "$Id: os_modreload.c 1.0 $"
#define NAME        "os_modreload"

void queueModuleOperation(Module *m, ModuleOperation op, User *u); /* modules.c */

int do_modreload(User *u);
void myOperServHelp(User *u);
int mySyntaxHelp(User *u);

int AnopeInit(int argc, char **argv)
{
    Command *c;

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);

    c = createCommand("MODRELOAD", do_modreload, is_services_root, -1, -1, -1, -1, -1);
    moduleAddHelp(c, mySyntaxHelp);
    moduleAddCommand(OPERSERV, c, MOD_UNIQUE);
    
    moduleSetOperHelp(myOperServHelp);

    return MOD_CONT;
}

void AnopeFini(void)
{

}

int mySyntaxHelp(User *u)
{
    if (is_services_root(u))
    {
        notice_user(s_OperServ, u, "Syntax: \002MODRELOAD FileName\002");
        notice_user(s_OperServ, u, "This command reloads the module named FileName from the modules");
        notice_user(s_OperServ, u, "directory.");
    }
    
    return MOD_CONT;
}

void myOperServHelp(User *u)
{
    if (is_services_root(u))
        notice_user(s_OperServ, u, "    MODRELOAD   Reload a module");
}

int do_modreload(User *u)
{
Module *m;
char *buf, *name;

    buf = moduleGetLastBuffer();
    name = myStrGetToken(buf, ' ', 0);
    if (name && findModule(name))
    {
        /* queue works backwards */
        m = createModule(name);
        queueModuleOperation(m, MOD_OP_LOAD, u);

        if (!queueModuleUnload(name, u))
            notice_lang(s_OperServ, u, OPER_MODULE_REMOVE_FAIL, name);
    }
    
    if (name)
        free(name);

    return MOD_CONT;
}

/* EOF */
