#include "module.h"
#define AUTHOR "n00bie"
#define VERSION "$Id: cs_mypower.c v1.0.1 18-01-2007 n00bie $"

/*
*******************************************************************************************
** Module	: cs_mypower.c
** Author	: n00bie (n00bie@rediffmail.com)
** Version	: 1.0.1
** Release	: 2nd September, 2007
** Update	: 18th January, 2008
** Italian Update : 27th Dicember, 2008
** Author   : Autiero Claudio (robotic.stealth@alice.it)
*******************************************************************************************
** Description:
**
** This module allow you to see what commands you can or cannot perform on a channel.
**
** Descrizione:
** Questo modulo ti permette di vedere quali comandi puoi o non puoi effettuare su un canale.                
*******************************************************************************************
** Providing Command:
**
** /msg ChanServ HELP MYPOWER
** /msg ChanServ MYPOWER #channel
**
** Tested: 1.7.21
**
** Comandi disponibili
**
** /msg ChanServ HELP MYPOWER
** /msg ChanServ MYPOWER #canale
**
** Testato: 1.7.21/1.8rc
*******************************************************************************************
** This program is free software; you can redistribute it and/or modify it under the
** terms of the GNU General Public License as published by the Free Software
** Foundation; either version 1, or (at your option) any later version. 
**
** This program is distributed in the hope that it will be useful, but WITHOUT ANY
** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS for
** a PARTICULAR PURPOSE. THIS MODULE IS DISTRIBUTED 'AS IS'. NO WARRANTY OF ANY
** KIND IS EXPRESSED OR IMPLIED. YOU USE AT YOUR OWN RISK. THE MODULE AUTHOR WILL 
** NOT BE RESPONSIBLE FOR DATA LOSS, DAMAGES, LOSS OF PROFITS OR ANY OTHER KIND OF 
** LOSS WHILE USING OR MISUSING THIS MODULE. 
**
** See the GNU General Public License for more details.
*******************************************************************************************
** Changelog:
** v1.0.0	- First Public Release
** v1.0.1	- 
**			  • Halfops can now use the command (thanks to ____ for reporting)
**			  • Fixed a missing typo on CA_ACCESS_CHANGE.
*******************************************************************************************
** This module have no configurable option.
*/

int myPower(User *u);
int myChanServPowerHelp(User *u);
void myChanServHelp(User *u);

int AnopeInit(int argc, char **argv)
{
	Command *c;
	int status = 0;
	c = createCommand("MYPOWER", myPower, NULL, -1, -1, -1, -1, -1);
	moduleAddHelp(c, myChanServPowerHelp);
	moduleSetChanHelp(myChanServHelp);
	status = moduleAddCommand(CHANSERV, c, MOD_TAIL);
	if (status == MOD_ERR_OK) {
		alog("cs_mypower: Modulo caricato con successo.");
		alog("cs_mypower: \2/msg %s HELP MYPOWER\2", s_ChanServ);
	} else {
		return MOD_STOP;
	}
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void AnopeFini(void)
{
	alog("cs_mypower: Modulo scaricato.");
}

void myChanServHelp(User *u)
{
	notice_user(s_ChanServ, u, "    MYPOWER  Vedi cosa puoi o non puoi fare su un canale  "); //See what you can or cannot do on a channel
}

int myChanServPowerHelp(User *u)
{
	notice_user(s_ChanServ, u, "Sintassi: \2MYPOWER\2 \037#canale\037");
	notice_user(s_ChanServ, u, " ");
	notice_user(s_ChanServ, u, "Questo comando ti permette di vedere cosa puoi o");
	notice_user(s_ChanServ, u, "non puoi fare su un canale.");
	notice_user(s_ChanServ, u, "E' un comando limitato ad utenti di grado \2halfop\2 o superiore.");
//	notice_user(s_ChanServ, u, "This command allow you to see what commands you can or");
//	notice_user(s_ChanServ, u, "cannot perform on a channel.");
//	notice_user(s_ChanServ, u, "Limited to users with \2halfop\2 access or above.");
	return MOD_CONT;
}

int myPower(User *u)
{
	NickAlias *na = findnick(u->nick);
	ChannelInfo *ci;
	char *buf = moduleGetLastBuffer();
	char *chan = myStrGetToken(buf, ' ', 0);
	if (!chan) {
		notice_user(s_ChanServ, u, "Sintassi: \2MYPOWER\2 \037#canale\037");
	} else if (!(na = u->na)) {
		notice_lang(s_NickServ, u, NICK_NOT_REGISTERED);
	} else if (!nick_identified(u)) {
		notice_lang(s_NickServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
	} else if (!(ci = cs_findchan(chan))) {
       notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
	} else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, ci->name);
    } else if (!(check_access(u, ci, CA_HALFOP) || check_access(u, ci, CA_HALFOPME))) {
        notice_lang(s_ChanServ, u, PERMISSION_DENIED);
	} else {
		notice_user(s_ChanServ, u, "****** I tuoi poteri su %s ******", chan);
		notice_user(s_ChanServ, u, " ");
		if (check_access(u, ci, CA_AUTOVOICE)) {
//notice_user(s_ChanServ, u, "You CAN be automatically be given voice status.");
            notice_user(s_ChanServ, u, "Hai la possibilita' di avere lo status di voice '+' .");
		}
		if (check_access(u, ci, CA_AUTOHALFOP)) {
//notice_user(s_ChanServ, u, "You CAN be automatically be given half-operator status.");
		notice_user(s_ChanServ, u, "Hai la possibilita' di avere lo status di halfop '%' .");
        }
		if (check_access(u, ci, CA_AUTOOP)) {
//notice_user(s_ChanServ, u, "You CAN be automatically be given operator status.");
            notice_user(s_ChanServ, u, "Hai la possibilita' di avere lo status di operatore '@' .");
		}
		if (check_access(u, ci, CA_AUTOPROTECT)) {
//notice_user(s_ChanServ, u, "You CAN be automatically be given protected status.");
            notice_user(s_ChanServ, u, "Hai la possibilita' di avere lo status di Super OPs '&' .");
		}
		if (check_access(u, ci, CA_MEMO)) {
//notice_user(s_ChanServ, u, "You CAN list/read a channel memos.");
            notice_user(s_ChanServ, u, "Tu puoi vedere list e leggere read i memo del canale.");
		} else {   
//notice_user(s_ChanServ, u, "You CANNOT list/read a channel memos.");
            notice_user(s_ChanServ, u, "Tu non puoi vedere list e leggere read i memo del canale.");
		}
		if (check_access(u, ci, CA_ACCESS_LIST)) {
//notice_user(s_ChanServ, u, "You CAN view the channel access list.");
			notice_user(s_ChanServ, u, "Tu puoi vedere le ACL Access Channel List.");
		}
		if (check_access(u, ci, CA_ACCESS_CHANGE)) {
//notice_user(s_ChanServ, u, "You CAN modify the channel access list.");
            notice_user(s_ChanServ, u, "Tu puoi modificare le ACL Access Channel List.");
		} else {
//notice_user(s_ChanServ, u, "You CANNOT modify the channel access list.");
            notice_user(s_ChanServ, u, "Tu non puoi modificare le ACL Access Channel List.");
		}
		if (check_access(u, ci, CA_CLEAR)) {
//notice_user(s_ChanServ, u, "You CAN use the CLEAR command.");
            notice_user(s_ChanServ, u, "Tu puoi usare il comando CLEAR.");
		} else {
//notice_user(s_ChanServ, u, "You CANNOT use the CLEAR command.");
			notice_user(s_ChanServ, u, "Tu non puoi usare il comando CLEAR.");
		}
		if (check_access(u, ci, CA_SET)) {
//notice_user(s_ChanServ, u, "You CAN use the SET command.");
            notice_user(s_ChanServ, u, "Tu puoi usare i comandi SET.");
		} else {
//notice_user(s_ChanServ, u, "You CANNOT use the SET command.");
            notice_user(s_ChanServ, u, "Tu non puoi usare il comando SET.");
        }
		if (check_access(u, ci, CA_AKICK)) {
			notice_user(s_ChanServ, u, "Tu puoi usare il comando AKICK.");
//notice_user(s_ChanServ, u, "You CAN use the AKICK command.");
		} else {
			notice_user(s_ChanServ, u, "Tu non puoi usare il comando AKICK.");
//notice_user(s_ChanServ, u, "You CANNOT use the AKICK command.");
		}
		if (check_access(u, ci, CA_PROTECT)) {
			notice_user(s_ChanServ, u, "Tu puoi usare il comando PROTECT ed DEPROTECT.");
//notice_user(s_ChanServ, u, "You CAN use the PROTECT/DEPROTECT command.");
		} else {
			notice_user(s_ChanServ, u, "Tu non puoi usare il comando PROTECT ed DEPROTECT.");
//notice_user(s_ChanServ, u, "You CANNOT use the PROTECT/DEPROTECT command.");
		}
		if (check_access(u, ci, CA_BAN)) {
			notice_user(s_ChanServ, u, "Tu puoi usare il comando BAN.");
//notice_user(s_ChanServ, u, "You CAN use the BAN command.");
		} else {
			notice_user(s_ChanServ, u, "Tu non puoi usare il comando BAN.");
//notice_user(s_ChanServ, u, "You CANNOT use the BAN command.");
		}
		if (check_access(u, ci, CA_KICK)) {
			notice_user(s_ChanServ, u, "Tu puoi usare il comando KICK.");
//notice_user(s_ChanServ, u, "You CAN use the KICK command.");
		} else {
			notice_user(s_ChanServ, u, "Tu non puoi usare il comando KICK.");
//notice_user(s_ChanServ, u, "You CANNOT use the KICK command.");
		}
		if (check_access(u, ci, CA_SIGNKICK)) {
			notice_user(s_ChanServ, u, "Tu puoi usare il comando KICK (segretamente).");
//notice_user(s_ChanServ, u, "You CAN use the KICK command (secretly).");
		} else {
            notice_user(s_ChanServ, u, "Tu non puoi usare il comando KICK (segretamente).");   
//notice_user(s_ChanServ, u, "You CANNOT use the KICK command (secretly).");
		}
		if (check_access(u, ci, CA_OPDEOP)) {
			notice_user(s_ChanServ, u, "Tu puoi usare i comandi OP e DEOP.");
//notice_user(s_ChanServ, u, "You CAN use the OP/DEOP command.");
		} else {
            notice_user(s_ChanServ, u, "Tu non puoi usare i comandi OP e DEOP.");
//notice_user(s_ChanServ, u, "You CANNOT use the OP/DEOP command.");
		}
		if (check_access(u, ci, CA_HALFOP)) {
			notice_user(s_ChanServ, u, "Tu puoi usare i comandi HALFOP e DEHALFOP.");
//notice_user(s_ChanServ, u, "You CAN use the HALFOP/DEHALFOP command.");
		} else {
			notice_user(s_ChanServ, u, "Tu non puoi usare i comandi HALFOP e DEHALFOP.");
//notice_user(s_ChanServ, u, "You CANNOT use the HALFOP/DEHALFOP command.");
		}
		if (check_access(u, ci, CA_VOICE)) {
			notice_user(s_ChanServ, u, "Tu puoi usare i comandi VOICE e DEVOICE.");
//notice_user(s_ChanServ, u, "You CAN use the VOICE/DEVOICE command.");
		} else {
			notice_user(s_ChanServ, u, "Tu non puoi usare i comandi VOICE e DEVOICE.");
//notice_user(s_ChanServ, u, "You CANNOT use the VOICE/DEVOICE command.");
		}
		if (check_access(u, ci, CA_UNBAN)) {
			notice_user(s_ChanServ, u, "Tu puoi usare il comando UNBAN.");
//notice_user(s_ChanServ, u, "You CAN use the UNBAN command.");		
        } else {
			notice_user(s_ChanServ, u, "Tu non puoi usare il comando UNBAN.");
//notice_user(s_ChanServ, u, "You CANNOT use the UNBAN command.");
		}
		if (check_access(u, ci, CA_INVITE)) {
			notice_user(s_ChanServ, u, "Tu puoi usare il comando INVITE.");
//notice_user(s_ChanServ, u, "You CAN use the INVITE command.");		
        } else {
			notice_user(s_ChanServ, u, "Tu non puoi usare il comando INVITE.");
//notice_user(s_ChanServ, u, "You CANNOT use the INVITE command.");
        }
		if (check_access(u, ci, CA_GETKEY)) {
			notice_user(s_ChanServ, u, "Tu puoi usare il comando GETKEY.");
//notice_user(s_ChanServ, u, "You CAN use the GETKEY command.");
		} else {
			notice_user(s_ChanServ, u, "Tu non puoi usare il comando GETKEY.");
//notice_user(s_ChanServ, u, "You CANNOT use the GETKEY command.");
		}
		if (check_access(u, ci, CA_TOPIC)) {
			notice_user(s_ChanServ, u, "Tu puoi usare il comando TOPIC.");
//notice_user(s_ChanServ, u, "You CAN use the TOPIC command.");
		} else {
			notice_user(s_ChanServ, u, "Tu non puoi usare il comando TOPIC.");
//notice_user(s_ChanServ, u, "You CANNOT use the TOPIC command.");
		}
		if (check_access(u, ci, CA_INFO)) {
			notice_user(s_ChanServ, u, "Tu puoi vedere tutte le informazioni del canale, usando l'opzione ALL.");
//notice_user(s_ChanServ, u, "You CAN see the full channel INFO using the ALL option.");
		} else {
			notice_user(s_ChanServ, u, "Tu non puoi vedere tutte le informazioni del canale.");
//notice_user(s_ChanServ, u, "You CANNOT see the full channel INFO using the ALL option.");
		}
		if (ci->bi) {
			if (check_access(u, ci, CA_GREET)) {
				notice_user(s_ChanServ, u, "Tu puoi essere 'salutato' dal bot quando entri.");
//notice_user(s_ChanServ, u, "You CAN be 'greeted' by a bot on join.");
			} else {
				notice_user(s_ChanServ, u, "Tu non puoi esser 'salutato' dal bot quando entri.");
//notice_user(s_ChanServ, u, "You CANNOT be 'greeted' by a bot on join.");
			}
			if (check_access(u, ci, CA_FANTASIA)) {
				notice_user(s_ChanServ, u, "Tu puoi utilizzare i comandi FANTASY del bot");
//notice_user(s_ChanServ, u, "You CAN use the bot FANTASY commands.");
			} else {
				notice_user(s_ChanServ, u, "Tu non puoi usare il comandi FANTASY del bot.");
//notice_user(s_ChanServ, u, "You CANNOT use the bot FANTASY commands.");
			}
			if (check_access(u, ci, CA_BADWORDS)) {
				notice_user(s_ChanServ, u, "Tu puoi modificare la lista delle BADWORDS del bot.");
//notice_user(s_ChanServ, u, "You CAN modify the bot BADWORDS list.");
			} else {
				notice_user(s_ChanServ, u, "Tu non puoi modificare la lista delle BADWORDS del bot.");
//notice_user(s_ChanServ, u, "You CANNOT modify the bot BADWORDS list.");
			}
			if (check_access(u, ci, CA_ASSIGN)) {
				notice_user(s_ChanServ, u, "Tu puoi assegnare ASSIGN e togliere UNASSIGN un bot.");
//notice_user(s_ChanServ, u, "You CAN ASSIGN/UNASSIGN a bot.");
			} else {
				notice_user(s_ChanServ, u, "Tu non puoi assegnare ASSIGN o togliere UNASSIGN un bot.");
//notice_user(s_ChanServ, u, "You CANNOT ASSIGN/UNASSIGN a bot.");
			}
			if (check_access(u, ci, CA_SAY)) {
				notice_user(s_ChanServ, u, "Tu puoi usare il comando SAY del bot.");
//notice_user(s_ChanServ, u, "You CAN use the bot SAY command.");
			} else {
				notice_user(s_ChanServ, u, "Tu non puoi usare il comando SAY del bot.");
//notice_user(s_ChanServ, u, "You CANNOT use the bot SAY command.");
			}
			if (check_access(u, ci, CA_NOKICK)) {
				notice_user(s_ChanServ, u, "Tu non puoi esser kickato tramite KICK del bot.");
				notice_user(s_ChanServ, u, "   (Per altre informazioni dai uno sguardo: /%s HELP KICK)", s_BotServ);
//notice_user(s_ChanServ, u, "You will not be kicked by the bot kickers.");
//notice_user(s_ChanServ, u, "   (For more info see: /%s HELP KICK)", s_BotServ);
			} else {
				notice_user(s_ChanServ, u, "Tu puoi esser kickato tramite KICK del bot.");
//notice_user(s_ChanServ, u, "You CAN be kicked by the bot kickers.");

			}
		}
		notice_user(s_ChanServ, u, "************************************");
	}
	if (chan)
		free(chan);
	return MOD_CONT;
}
/* EOF */
