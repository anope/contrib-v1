/***************************************************************************
 **  os_superadmin_pass.c  ********** Author: GeniusDex ** Version: 2.0.1 **
 ***************************************************************************
 *                                                                         *
 * Service:     OperServ                                                   *
 * Module:      SuperAdmin Password Protection                             *
 * Version:     2.0.1                                                      *
 * License:     GPL [GNU Public License]                                   *
 * Author:      GeniusDex                                                  *
 * E-mail:      geniusdex@anope.org                                        *
 * Past author: SGR <Alex_SGR@ntlworld.com>                                *
 * Description: Protect access to SuperAdmin with a password               *
 *                                                                         *
 *   This module protects the SET SUPERADMIN command with a password, so   *
 *   that nobody without the password can become a SuperAdmin.             *
 *                                                                         *
 *   To set the password used to become a SuperAdmin, add a SuperAdminPass *
 *   directive to your configuration file. This has one string argument,   *
 *   being the password. For example:                                      *
 *                                                                         *
 *       SuperAdminPass "topsecret"                                        *
 *                                                                         *
 *   This protection is only for Services Administrators by default. If    *
 *   also want Services Roots to supply a password, you can enable this by *
 *   adding a SuperAdminPassRoot directive to your configuration file,     *
 *   without any arguments. For example:                                   *
 *                                                                         *
 *       SuperAdminPassRoot                                                *
 *                                                                         *
 *   These directives cannot be changed with OperServ's RELOAD command;    *
 *   you will have to reload the module instead.                           *
 *                                                                         *
 ***************************************************************************
 * Languages:                                                              *
 *   English     GeniusDex   <geniusdex@anope.org>                         *
 *   Spanish                                                               *
 *   Portugese                                                             *
 *   French                                                                *
 *   Turkish                                                               *
 *   Italian                                                               *
 *   German                                                                *
 *   Catalan                                                               *
 *   Greek                                                                 *
 *   Dutch       GeniusDex   <geniusdex@anope.org>                         *
 *   Russian                                                               *
 * All open languages default to English                                   *
 ***************************************************************************
 **  CHANGES  *****************************************  VERSION HISTORY  **
 ***************************************************************************
 *** 2.0.1 ************************************************** 15/09/2005 ***
 * -Added an AnopeFini function to avoid compiling errors on win32         *
 *** 2.0.0 ************************************************** 03/09/2005 ***
 * -Added language strings for feedback                                    *
 * -Big cleanup by letting the core do a lot of the work it should         *
 * -Converted configuration code to services.conf directives               *
 * -Took over old code from SGR                                            *
 ***************************************************************************
 ****************** Don't change anything below this line ******************
 **************************************************************************/

#include "module.h"

#define AUTHOR "GeniusDex"
#define VERSION "2.0.1"

#define LANG_NUM_STRINGS   2
#define LANG_NO_PASSWORD        0
#define LANG_INVALID_PASSWORD   1

int my_operserv_set(User * u);
void my_add_languages(void);
void my_load_config(void);

char *SuperAdminPass = NULL;
int SuperAdminPassRoot = 0;

int AnopeInit(int argc, char **argv)
{
	Command *c;
	int status;
	
	my_add_languages();
	my_load_config();
	
	/* Should i add a configuration option to also allow is_services_oper
	 * here? Or would that make this module do too much? -GD
	 */
	c = createCommand("set", my_operserv_set, is_services_admin, -1, -1, -1, -1, -1);
	if ((status = moduleAddCommand(OPERSERV, c, MOD_HEAD))) {
		alog("[os_superadmin_pass] Unable to hook to 'OperServ SET'");
		return MOD_STOP;
	}
	
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	
	return MOD_CONT;
}

void AnopeFini(void)
{
	/* Nothing to do here */
}

int my_operserv_set(User *u)
{
	char *text;
	char *setting;
	char *value;
	char *pass;
	int ret = MOD_CONT;
	
	if (!SuperAdminPass)
		return MOD_CONT;
	
	text = moduleGetLastBuffer();
	setting = myStrGetToken(text, ' ', 0);
	value = myStrGetToken(text, ' ', 1);
	pass = myStrGetToken(text, ' ', 2);
	
	if (setting && (stricmp(setting, "superadmin") == 0)) {
		if (value && (stricmp(value, "on") == 0)) {
			if (!is_services_root(u) || (SuperAdminPassRoot && is_services_root(u))) {
				if (!pass) {
					moduleNoticeLang(s_OperServ, u, LANG_NO_PASSWORD);
					ret = MOD_STOP;
				} else if (strcmp(pass, SuperAdminPass) != 0) {
					moduleNoticeLang(s_OperServ, u, LANG_INVALID_PASSWORD);
					ret = MOD_STOP;
				}
				/* If both checks are passed, we're a valid superadmin, and
				 * we can let the core do it's job.
				 */
			}
		}
	}
	
	if (setting)
		free(setting);
	if (value)
		free(value);
	if (pass)
		free(pass);
	
	return ret;
}

void my_add_languages(void)
{
	char *langtable_en_us[] = {
		/* LANG_NO_PASSWORD */
		"You have to provide a password to enable SuperAdmin on this\n"
		"network. Please do so by appending it after the SET SUPERADMIN\n"
		"line. For example: \002/os SET SUPERADMIN ON MySecretPass\002.",
		/* LANG_INVALID_PASSWORD */
		"The supplied password is incorrect."
	};
	
	char *langtable_nl[] = {
		/* LANG_NO_PASSWORD */
		"Het is op dit netwerk vereist om een wachtwoord op te geven\n"
		"bij het inschakelen van SuperAdmin. Voeg dit wachtwoord toe\n"
		"achter de SET SUPERADMIN regel. Bijvoorbeeld zoals dit:\n"
		"\002/os SET SUPERADMIN ON MijnGeheimeWachtwoord\002.",
		/* LANG_INVALID_PASSWORD */
		"Het opgegeven wachtwoord is incorrect."
	};
	
	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
	moduleInsertLanguage(LANG_NL, LANG_NUM_STRINGS, langtable_nl);
}

void my_load_config(void)
{
    Directive confvalue_one[] = {
		{"SuperAdminPass", {{PARAM_STRING, PARAM_RELOAD, &SuperAdminPass}}}
    };
    Directive confvalue_two[] = {
        {"SuperAdminPassRoot", {{PARAM_SET, PARAM_RELOAD, &SuperAdminPassRoot}}}
    };

	if (SuperAdminPass)
		free(SuperAdminPass);
	
    moduleGetConfigDirective(confvalue_one);
    moduleGetConfigDirective(confvalue_two);
}

/* EOF */
