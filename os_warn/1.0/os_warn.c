/* OperServ: WARN
 *
 * (C) 2009 Zach Mitchell
 * Contact: zach@drivenirc.com
 *
 * This module is a simple module that really
 * only has use on larger networks. This command,
 * /os WARN <nick>, will be used to annonymously warn
 * a user that they have been reported to the staff
 * and should stop misbehaving before the network takes
 * the situation farther. This prevents opers from having
 * to personally PM the user or join their channel.
 *
 * Again, if you have no use for it don't complain, just
 * don't load it and you'll be fine! :P
 */

#include "module.h"

#define AUTHOR "DigitalMan"
#define VERSION "$Id os_warn.c 1.0 19-02-2009 DigitalMan $"

#define LNG_NUM_STRINGS		4
#define LNG_OPER_HELP		0
#define LNG_OPER_HELP_WARN		1
#define LNG_OPER_WARN_SYNTAX	2
#define LNG_OPER_WARN_MSG		3

int my_os_warn(User * u);
void my_os_help(User * u);
int my_os_help_warn(User * u);
void my_add_languages();

static Module *me;

class OSWarn : public Module
{
 public:
	OSWarn(const std::string &modname, const std::string &creator) : Module(modname, creator)
	{
		Command *c;

		me = this;

		this->SetAuthor(AUTHOR);
		this->SetVersion(VERSION);
		this->SetType(THIRD);

		c = createCommand("WARN", my_os_warn, NULL, -1, -1, -1, -1, -1);
		this->AddCommand(OPERSERV, c, MOD_HEAD);
		moduleAddHelp(c, my_os_help_warn);
		this->SetOperHelp(my_os_help);

		const char* langtable_en_us[] = {
		/* LNG_OPER_HELP */
		"    WARN        Annonymously warn a user about their behavior",
		/* LNG_OPER_HELP_WARN */
		"This command allows you to annonymously warn a user and advise\n"
		"them to change their behavior before further action is taken by\n"
		"the network staff such as a kill, ban, or suspension.",
		/* LNG_OPER_WARN_SYNTAX */
		"Syntax: WARN nick\n",
		/* LNG_OPER_WARN_MSG */
		"ATTENTION: It is reported that you have not been abiding by the\n"
		"network rules. If you continue this behavior further action may be\n"
		"taken. If you have any questions, feel free to contact us in the\n"
		"network help channel. Thank you.",
		};

		this->InsertLanguage(LANG_EN_US, LNG_NUM_STRINGS, langtable_en_us);

		}
};

void my_os_help(User * u)
{
	me->NoticeLang(s_OperServ, u, LNG_OPER_HELP);
}

int my_os_help_warn(User * u)
{
	me->NoticeLang(s_OperServ, u, LNG_OPER_WARN_SYNTAX);
	me->NoticeLang(s_OperServ, u, LNG_OPER_HELP_WARN);
	return MOD_STOP;
}

int my_os_warn(User * u)
{
	char *buffer = moduleGetLastBuffer();
	User *u2;

	char *nick;

	nick = myStrGetToken(buffer, ' ', 0);
	
	if (nick && !(u2 = finduser(nick))) {
		notice_lang(s_OperServ, u, NICK_X_NOT_IN_USE, nick);
	} else {
		me->NoticeLang(s_NickServ, u2, LNG_OPER_WARN_MSG);
		alog("%s has been warned.", nick);
	}
	
	if (nick) delete [] nick;

	return MOD_CONT;
}

MODULE_INIT("os_warn", OSWarn)