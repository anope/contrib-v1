 #include "module.h"
 
 #define AUTHOR "$Id: os_massserv  V0.1 27-12-2009 ysfm $"
 #define VERSION "0.0.1"
 

 int do_massserv(User *u);
 
 int AnopeInit(int argc, char **argv) {
   
     Command *c;
     c = createCommand("MASS", do_massserv, is_services_admin, -1, -1, -1, -1, -1);
     alog("do_massserv : Module Loaded ", moduleAddCommand(OPERSERV, c, MOD_HEAD));
 
     moduleAddAuthor(AUTHOR);
     moduleAddVersion(VERSION);
 
     return MOD_CONT;
 }
 
 void AnopeFini(void) {
   
 }
 
 int do_massserv(User *u) 
{
    ChannelInfo *ci;
	char *param1, *param2, *chan, *param4;
	int i;
	User *u2;

	param1 = moduleGetLastBuffer();
	param2 = myStrGetToken(param1, ' ', 0);
	chan = myStrGetToken(param1, ' ', 1);
	param4 = myStrGetToken(param1, ' ', 2);
    Channel *c;
    int modes = 0;

	if (!param2 || !chan)
		 notice(s_OperServ, u->nick, "Syntax: MASS [ JOIN | PART | VOICE | DEVOICE ] [\037channel\037 || \037excepts\037] ");
	
	else if (*chan == '&') 
			notice(s_OperServ, u->nick, "This Command dont use to local channel");
	else if (*chan != '#')
			notice(s_OperServ, u->nick, "%s isn't a channel. Try # %s", chan);
	else if (!anope_valid_chan(chan))
			notice_lang(s_NickServ, u, CHAN_X_INVALID, chan);
	else if (!(ci = cs_findchan(chan)))
			notice_lang(s_NickServ, u, CHAN_X_NOT_REGISTERED, chan);
	else if (ci->flags & CI_VERBOTEN)
			notice_lang(s_NickServ, u, CHAN_X_FORBIDDEN, chan);
	else if (ci->flags & CI_SUSPENDED)
			notice_lang(s_NickServ, u, CHAN_X_FORBIDDEN, chan);
	
	
	else if (stricmp(param2, "JOIN") == 0) {
			for (i = 0; i < 1024; i++) {
				for (u2 = userlist[i]; u2; u2 = u2->next) {
					anope_cmd_svsjoin(s_NickServ, u2->nick, chan, NULL);
				}
			}
	}	else if (stricmp(param2, "PART") == 0) {
		modes |= anope_get_invis_mode();
		  if (chan && (c = findchan(chan))) {
        struct c_userlist *cu;
		for (cu = c->users; cu; cu = cu->next) {
            if (modes && !(cu->user->mode & modes))
                continue;
			anope_cmd_svspart(s_NickServ, cu->user->nick, chan);
            
				 }
		  }

	}	else if (stricmp(param2, "VOICE") == 0) {
	
		  modes |= anope_get_invis_mode();
		  if (chan && (c = findchan(chan))) {
          struct c_userlist *cu;
		  for (cu = c->users; cu; cu = cu->next) {
            if (modes && !(cu->user->mode & modes))
                continue;
			send_cmd(s_OperServ, "MODE %s +v %s", chan, cu->user->nick);   
            
				 }
		  }
	}	else if (stricmp(param2, "DEVOICE") == 0) {
		  modes |= anope_get_invis_mode();
		  if (chan && (c = findchan(chan))) {
          struct c_userlist *cu;
		  for (cu = c->users; cu; cu = cu->next) {
            if (modes && !(cu->user->mode & modes))
                continue;
			send_cmd(s_OperServ, "MODE %s -v %s", chan, cu->user->nick);   
            
				 }
		  }

	} else {
	notice(s_OperServ, u->nick, "Syntax: MASS { JOIN | PART | VOICE | DEVOICE } [\037channel\037 ] ", u->nick);
	}

     return MOD_STOP;


}
 
 /* EOF */
