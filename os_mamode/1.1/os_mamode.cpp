/**
 * -----------------------------------------------------------
 * Name:    os_mamode
 * Author:  LEthaLity 'Lee' (lee@lethality.me.uk)
 * Date:    10th August 2009
 * Version: 1.1
 * -----------------------------------------------------------
 * This module is a 1.9 Port of os_amode and os_massmode.
 * Credit goes to Katsclaw and Sorcerer respectively.
 * This module allows a ServiceAdmin to set a mode on ALL 
 * registered channels. 
 * -----------------------------------------------------------
 * No other configuration options.
 *
 * -----------------------------------------------------------
 * Changes:
 * 1.1 Main Function resembles os_massmode for anope-1.8
 * created by Sorcerer more. Renamed from os_massmode to 
 * os_mamode because anope mod site is evil :P
 *
 * 1.0 First Release, based on Katsklaws os_amode
 **/

#include "module.h"

#define AUTHOR "LEthaLity"
#define VERSION "1.1"

/* Functions */
int do_mamode(User * u);  //our main mode set
int mamode_syntax(User *u);  //returns correct syntax on incorrect useage
void os_help(User *u); //Adds MaMode to /os help 
int os_help_mamode(User * u);  //Adds detailed help on /os help mamode

static Module *me;

class MaMode : public Module
{
 public:
	 MaMode(const std::string &modname, const std::string &creator) : Module(modname, creator)
	 {
		 Command *c;
     	 c = createCommand("MAMODE", do_mamode, is_services_admin,-1,-1,-1,-1,-1); //Creating the command, for service admin
		    this->AddCommand(OPERSERV, c, MOD_HEAD);
		 	moduleAddHelp(c, os_help_mamode);  //Adding the help
			this->SetOperHelp(os_help);

		me = this;
                this->SetAuthor(AUTHOR);
                this->SetVersion(VERSION);
	 }
};

int mamode_syntax(User *u) { 
	notice_user(s_OperServ, u, "Syntax: \x2MAMODE <mode>\x2");
	notice_user(s_OperServ, u, "  ");
	notice_user(s_OperServ, u, "Set a mode on all channels");
	return MOD_CONT;
}
void os_help(User *u) {
	notice_user(s_OperServ, u, "    MAMODE    Set a mode on ALL Channels");
}
int os_help_mamode(User *u) {
	if (is_services_admin(u)) {
		notice_user(s_OperServ, u, "Syntax: \x2MAMODE <mode>\x2");
		notice_user(s_OperServ, u, " ");
		notice_user(s_OperServ, u, "Allows you to add or remove modes on ALL");
		notice_user(s_OperServ, u, "channels.");
	}
	return 1;
}

int do_mamode(User *u)
{
		char *temp = moduleGetLastBuffer();
	    char *modes = myStrGetTokenRemainder(temp,' ',0);
if (!modes) {
	notice_user(s_OperServ, u, "You did not specify any modes");
	notice_user(s_OperServ, u, "Syntax: \x2mamode +(-)modes\x2");

	} else {
			if ((modes[0] != '+') && (modes[0] != '-')) {
	notice_user(s_OperServ, u, "Invalid mode prefix, only + and - is allowed.");
	notice_user(s_OperServ, u, "Syntax: \x2mamode +(-)modes\x2");

	} else {
		do_mass_mode(modes);
		alog("[os_mamode]: \x2%s\x2 set all channels \x2%s\x2 using MaMode", u->nick, modes);
	free(modes);
	return MOD_CONT;
/* Better checking for incorrect useage likely to be added above soon */
     }
  }
  return MOD_CONT;
}	

MODULE_INIT("os_mamode", MaMode)

