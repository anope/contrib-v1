#include "module.h"

#define AUTHOR "Alek + Vorex"
#define VERSION "0.3B"

// Credits:
// Russian Roulette was adapted from mhweaver's mirc script version
// Wheel of Abuse was adapted from mhweaver's mirc script version

// next version
// Hot Potato was adapted from neostats gameserv

/* #######################################
 * #######################################
 * Edit these settings
 * ####################################### */

// Set to 1 if you want russian roulette to kill users, otherwise set to 0 for a kick
//#define RUSS_KILL 0
// Set to 1 if you want wheel of abuse turned on (it does alot of abusive commands)
//#define WHEEL_OF_ABUSE 1

/* #######################################
 * Do not edit anything further
 * #######################################
 * ####################################### */
#define hash(chan)      ((tolower((chan)[0])&31)<<5 | (tolower((chan)[1])&31))

int my_privmsg(char *source, int ac, char **av);
CommandHash *Gameserv_cmdTable[MAX_CMD_HASH];


int psuedocancel(int ac, char **argv);
void addClient(char *nick, char *realname);
void addMessageList(void);
void delClient(char *nick);
char *s_GameServ = "GameServ";
void gameserv(User * u, char *buf);
void gamechanmsgs(User *u, ChannelInfo *ci, char *buf);

/* GameServ struct */
typedef struct gameinfo_ GameInfo;

struct gameinfo_ {
    GameInfo *prev, *next;
    char *chan;
    int join;
    int chamber;   
    long hilow;
    int rps;
    int rpsgo;
    int potato;
    int potatogo;
};

GameInfo *gilists[1024];

GameInfo *findgame(const char *chan){
    GameInfo *gi;
    
    for (gi = gilists[hash(chan)]; gi; gi = gi->next){
        if (!stricmp(chan, gi->chan))
           return gi;
    }
    
    return NULL;
}

GameInfo *newgame(const char *chan){
    GameInfo *gi;
    int index;       
    gi = scalloc(1, sizeof(GameInfo));
    gi->chan = strdup(chan);
    index = hash(chan);

        gi->prev = NULL;
        gi->next = gilists[index];
    if (gi->next)
        gi->next->prev = gi;
        gilists[index] = gi;
   
    return gi;
}

/* Commands */
int gs_join(User *u);
int gs_help(User *u);
int gs_help_join(User *u);
int gs_help_rps(User *u);
int gs_rps(User *u);
// next version int gs_potato(User *u);

void AnopeInit(void)
{
    Message *msg = NULL;
    int status;
    msg = createMessage("PRIVMSG", my_privmsg);
    status = moduleAddMessage(msg, MOD_HEAD);
    if (status == MOD_ERR_OK) {
        addClient(s_GameServ, "GameServ");
        addMessageList();    
    }
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    alog("ircd_gameserv.so: loaded, message status [%d]", status);
}

void AnopeFini(void)
{
 alog("ircd_gameserv.so: unloading");   
 delClient(s_GameServ);
}

int my_privmsg(char *source, int ac, char **av)
{
    ChannelInfo *ci;
    GameInfo *gi;
    User *u;
    char *s;

    /* First, some basic checks */
    if (ac != 2)
        return MOD_CONT;        /* bleh */
    if (!(u = finduser(source))) {
        return MOD_CONT;
    }                           /* non-user source */
    
    if (*av[0] == '#') {
        if (s_GameServ && (ci = cs_findchan(av[0])) && (gi = findgame(av[0])))
          if (gi->join && ci->c){
            gamechanmsgs(u, ci, av[1]);
          return MOD_CONT;
          }
     return MOD_CONT;
    }
    /* Channel message */
    /* we should prolly honour the ignore list here, but i cba for this... */
    s = strchr(av[0], '@');
    if (s) {
        *s++ = 0;
        if (stricmp(s, ServerName) != 0)
            return MOD_CONT;
    }
    if ((stricmp(av[0], s_GameServ)) == 0) {
        gameserv(u, av[1]);
        return MOD_CONT;
    } else {                    /* ok it isnt us, let the old code have it */
        return MOD_CONT;
    }
return MOD_CONT;
}

void addClient(char *nick, char *realname)
{
    NEWNICK(nick, "GameServ", ServiceHost, realname, "+", 1);
}

void delClient(char *nick)
{
    send_cmd(s_GameServ, "QUIT :Powering off");
}

void addMessageList(void)
{
         Command *c;
         c = createCommand("join", gs_join, NULL, -1, -1, -1, -1, -1);
         alog("ircd_gameserv.so: Add command 'join' Status %d", moduleAddCommand(Gameserv_cmdTable, c, MOD_HEAD));
         c = createCommand("help", gs_help, NULL, -1, -1, -1, -1, -1);
         alog("ircd_gamserv.so: Add command 'help' Status %d", moduleAddCommand(Gameserv_cmdTable, c, MOD_HEAD));
         c = createCommand("rps", gs_rps, NULL, -1, -1, -1, -1, -1);
         alog("ircd_gameserv.so: Add command 'rps' Status %d", moduleAddCommand(Gameserv_cmdTable, c, MOD_HEAD));
/* next version         c = createCommand("hotpotato", gs_hotpotato, NULL, -1, -1, -1, -1, -1);
         alog("ircd_gameserv.so: Add command 'hotpotato' Status %d", moduleAddCommand(Gameserv_cmdTable, c, MOD_HEAD)); */

}

/*****************************************************************************/
/* Main GameServ routine. */
void gameserv(User * u, char *buf)
{
  char *cmd, *s;
    cmd = strtok(buf, " ");

    if (!cmd) {
        return;
    } else if (stricmp(cmd, "\1PING") == 0) {
        if (!(s = strtok(NULL, "")))
            s = "\1";
        notice(s_GameServ, u->nick, "\1PING %s", s);
    } else if (skeleton) {
        notice_lang(s_GameServ, u, SERVICE_OFFLINE, s_GameServ);
    } else {
        mod_run_cmd(s_GameServ, u, Gameserv_cmdTable, cmd);
    }
}

void gamechanmsgs(User *u, ChannelInfo *ci, char *buf){
    char *cmd; 
    GameInfo *gi = findgame(ci->name);
    if (!u)
      return;
      if (!strnicmp(buf, "\1PING", 5))
         notice(s_GameServ, u->nick, buf);

      if (!strnicmp(buf, "\1ACTION ", 8))
         buf += 8;

      if (buf && *buf == '!'){
         cmd = myStrGetToken(buf,' ', 0);
      if (cmd){
      /* !hi-low */
      if (!stricmp(cmd, "!hi-low")){
      	char *numbers = myStrGetToken(buf, ' ', 1);
        long number;
        if (numbers){
                if (!(number = atoi(numbers)))
                    return;
        } else if (!numbers)
            number = 10;
   
        if (number < 0)
            number = 10;

        if (gi->hilow)
            gi->hilow = 0;
            srand(time(NULL));    
            gi->hilow = (1 + (rand() % number));
            privmsg(s_GameServ, ci->name, "Guess a number between 1 and %d using !guess <number>", number);
           return;
      }
   if (!stricmp(cmd, "!guess")){  
      char *yours = myStrGetToken(buf, ' ', 1);
      long you;
      
      if (yours)
         you = atoi(yours);
      else
         return; 
   
       if (gi->hilow == 0){
         privmsg(s_GameServ, ci->name, "You must initialize the game using !hi-low");
         return; 
       }
    
      if (gi->hilow < you)
        privmsg(s_GameServ, ci->name, "%s Your number was to high", u->nick);
      else if (gi->hilow > you)
        privmsg(s_GameServ, ci->name, "%s Your number was to low", u->nick);
      else {
       privmsg(s_GameServ, ci->name, "Thats right, %s is the one", u->nick);
       gi->hilow = 0;
      }
   }
  /* Wheel of abusez0r! */
  #ifdef WHEEL_OF_ABUSE
   if (!stricmp(cmd, "!spin")){
    int wheel = 1 + rand() % 12;

    /* Will need to add in callbacks, will do later */
     privmsg(s_GameServ, ci->name, "\001Action spins the Wheel of Abuse for: %s", u->nick);
     privmsg(s_GameServ, ci->name, "Round and round she goes, where she stops, nobody knows...!");
     privmsg(s_GameServ, ci->name, "The Wheel of Abuse has stopped on...");
    if (wheel == 1) {
     privmsg(s_GameServ, ci->name, "1: Peer's gonna eat you!!!!");
     send_cmd(NULL, "SVSKILL %s :Connection reset by peer", u->nick);
    }
    if (wheel == -1) {
     privmsg(s_GameServ, ci->name, "2: Join/Part Flood");
 	 // ########################
    }
    if (wheel == 3 || wheel == 2) {
     privmsg(s_GameServ, ci->name, "3: Part all channels");
	 send_cmd(NULL, "SVSJOIN %s 0", u->nick);
    }
    if (wheel == 4) {
     int wtime = 120 + rand() % 600;

     privmsg(s_GameServ, ci->name, "4: /gline for random amount of time");
     send_cmd(NULL, "TKL + G %s %s %s %ld %ld :%s", u->nick, GetHost(u), "GameServ", (long int) time(NULL) + wtime, 
             time(NULL), "Reward for spinning the wheel of abuse!");
	 /* this won't work send_cmd(NULL, "GLINE %s %ds :Reward for spinning the wheel of abuse!", u->nick); */
    }
    if (wheel == 5) {
     int wtime = 120 + rand() % 600;

     privmsg(s_GameServ, ci->name, "5: /shun for random amount of time");
     send_cmd(NULL, "TKL + S %s %s %s %ld %ld :%s", u->nick, GetHost(u), "GameServ", (long int) time(NULL) + wtime, 
             time(NULL), "Reward for spinning the wheel of abuse!"); 
 	 /* send_cmd(NULL, "SHUN %s %ds :Reward for spinning the wheel of abuse!", u->nick); */
    }
    if (wheel == 6) {
     privmsg(s_GameServ, ci->name, "6: Absolutely nothing");
    }
    if (wheel == 7) {
     int complete = 0;
     int rndchans = 0;
     int chango = 0;
     int roundz0r = 0;

     privmsg(s_GameServ, ci->name, "7: Join a bunch of random channels, then /part all of 'em several times");
	 while(complete != 1)  {
		if (rndchans != 15)	{
		 chango = 120 + rand() % 600;
		 send_cmd(NULL, "SVSJOIN %s #%d", u->nick, chango);
		 rndchans++;
		}
		else {
			if (roundz0r != 1) {
				send_cmd(NULL, "SVSJOIN %s 0", u->nick);
				roundz0r = 1;
				rndchans = 0;
			}
			else {
				send_cmd(NULL, "SVSJOIN %s 0", u->nick);
				complete = 1;
			}
		}
	 }
    }
    if (wheel == 8) {
     privmsg(s_GameServ, ci->name, "8: Abuse line added to /whois info");
     send_cmd(NULL, "swhois %s :is being defecated on by services", u->nick);
    }
    if (wheel == 9) {
	 int ignoretime = 0;

     privmsg(s_GameServ, ci->name, "9: Services ignore for random amount of time");
     srand(time(NULL));    
     ignoretime = (1 + rand() % 120);
     add_ignore(u->nick, ignoretime);
    }
    if (wheel == 10) {
	 char *argv[0];
	 char abusednick[NICKMAX];
	 int abusednum = time(NULL);

     privmsg(s_GameServ, ci->name, "10: Random Nick Change");
         snprintf(abusednick, NICKMAX, "Abused%d", abusednum++);    
 	 argv[0] = u->nick;

	 send_cmd(NULL, "SVSNICK %s %s :0", u->nick, abusednick);
     NEWNICK(u->nick, NSEnforcerUser, NSEnforcerHost, "I got abused by the Wheel of Abuse :D", "+", 0);
           
   	 cancel_user(u);
     moduleAddCallback("psuedocancel", time(NULL) + 300, psuedocancel, 1, argv);
 
    }
	// new stuff
    if (wheel == 11) {
     privmsg(s_GameServ, ci->name, "11: /kill");
     kill_user(s_GameServ, u->nick, "Reward for spinning the wheel of abuse!");
    }
    if (wheel == 12) {
     privmsg(s_GameServ, ci->name, "12: Nicklist Corruption!");
 	 send_cmd(NULL, "353 %s %s :----You- ---Are-- --Gay--- -Mkay----", u->nick, ci->name);
  	 send_cmd(NULL, "366 %s %s", u->nick, ci->name);
    }
    if (wheel == 13) {
     privmsg(s_GameServ, ci->name, "13: Pseudo PM Flood!");
	 // should prob check which serv's are actually loaded....but cba
     privmsg(s_GameServ, u->nick, "Floodz0r!!11oneoneeleven");
     privmsg(s_ChanServ, u->nick, "Floodz0r!!11oneoneeleven");
     privmsg(s_OperServ, u->nick, "Floodz0r!!11oneoneeleven");
     privmsg(s_HelpServ, u->nick, "Floodz0r!!11oneoneeleven");
     privmsg(s_HostServ, u->nick, "Floodz0r!!11oneoneeleven");
     privmsg(s_MemoServ, u->nick, "Floodz0r!!11oneoneeleven");
     privmsg(s_NickServ, u->nick, "Floodz0r!!11oneoneeleven");
     privmsg(s_GlobalNoticer, u->nick, "Floodz0r!!11oneoneeleven");

    }
  }
  #endif
	  /*  next version
  if (!stricmp(cmd, "!hot potato")){
	char **argv;
	argv[0] = ci->name;

    privmsg(s_GameServ, ci->name, "\001Action Initiates Hot Potato. (/msg gameserv help hotpotato) game will begin in 30sec.");
    moduleAddCallback("gs_potatogo", time(NULL) + 30, gs_potatogo, 1, argv[0]);
    gi->potatogo = 1;
    return;
  } */
   if (!stricmp(cmd, "!8ball")){
    int ball = 1 + rand() % 12;
    if (ball == 1)
     privmsg(s_GameServ, ci->name, "%s: Sorry, unlikely.", u->nick);
    if (ball == 2)
     privmsg(s_GameServ, ci->name, "%s: Almost certain.", u->nick);
    if (ball == 3)
     privmsg(s_GameServ, ci->name, "%s: Yes.", u->nick);
    if (ball == 4)
     privmsg(s_GameServ, ci->name, "%s: No.", u->nick);
    if (ball == 5)
     privmsg(s_GameServ, ci->name, "%s: All I know is that Vorex is a hoe.", u->nick);
    if (ball == 12)
     privmsg(s_GameServ, ci->name, "%s: Not on your life.", u->nick);
    if (ball == 6)
     privmsg(s_GameServ, ci->name, "%s: Maybe so.", u->nick);
    if (ball == 7)
     privmsg(s_GameServ, ci->name, "%s: Reading foggy -- try again later.", u->nick);
    if (ball == 8)
     privmsg(s_GameServ, ci->name, "%s: It is decidedly so.", u->nick);
    if (ball == 9)
     privmsg(s_GameServ, ci->name, "%s: Maybe so.", u->nick);
    if (ball == 10)
     privmsg(s_GameServ, ci->name, "%s: Absolutely not.", u->nick);
    if (ball == 11)
     privmsg(s_GameServ, ci->name, "%s: You never know.  Neither do I.", u->nick);
  }
  // coinflip
  if (!stricmp(cmd, "!coinflip")){
    int side = 1 + rand() % 2;
    if (side == 1)
     privmsg(s_GameServ, ci->name, "Heads");
    if (side == 2)
     privmsg(s_GameServ, ci->name, "Tails");
  }
  // dice
  if (!stricmp(cmd, "!dice")){
     int dice_num;
     char *num = myStrGetToken(buf, ' ', 1);
     if (!num){     
      privmsg(s_GameServ, ci->name, "Please specificy a number");
     }else if (!(dice_num = atoi(num))){
      privmsg(s_GameServ, ci->name, "Please specificy a number");
     } else {
      privmsg(s_GameServ, ci->name, "The roll of the dice reveals %d", (int) rand() % dice_num); 
     }                
  }
  
  if (!stricmp(cmd, "!help")){
    gs_help(u);
  }

  if (!stricmp(cmd, "!rps")){
    privmsg(s_GameServ, ci->name, "\001Action Initiates rock paper scissors. (/msg gameserv help rps)");
    gi->rpsgo = 1;
    return;
  }

   
  if (!stricmp(cmd, "!roulette")){
    if (gi->chamber){
      #ifdef RUSS_KILL
	  kill_user(s_GameServ, u->nick, "BANG - Don't stuff bullets into a loaded gun");
      #else
      anope_cmd_kick(s_GameServ, ci->name, u->nick, "BANG - Don't stuff bullets into a loaded gun");
      #endif
	  return;
    }
    
     privmsg(s_GameServ, ci->name, "\001Action loads the gun and sets it on the table");
     gi->chamber = 1 + rand() % 6;
     return;
  }

  if (!stricmp(cmd, "!shoot")){
    if (gi->chamber <= 0) {
      privmsg(s_GameServ, ci->name, "Please type !roulette to start a new round");
      return; 
    }
    
    gi->chamber--;
    if (gi->chamber == 0) {
        privmsg(s_GameServ, ci->name, "Bang!!!");
        privmsg(s_GameServ, ci->name, "Better luck next time, %s", u->nick);
    #ifdef RUSS_KILL
	kill_user(s_GameServ, u->nick, "BANG!!!!");
	#else
      anope_cmd_kick(s_GameServ, ci->name, u->nick, "BANG!!!!");
	#endif
    } else
      privmsg(s_GameServ, ci->name, "Click");
  }
  return;
}
}
}


/***********************************************************/

int gs_join(User *u){
    ChannelInfo *ci;
    GameInfo *gi;
    
    char *chan = strtok(NULL, " ");
    char *cmd = strtok(NULL, " ");

    if (!chan || !cmd)
        return MOD_CONT;

    if (!(ci = cs_findchan(chan))){
        notice(s_GameServ, u->nick, "Sorry that channel does not appear to exist");
        return MOD_CONT;
    }
    
    if (!((is_founder(u, ci) || (is_services_oper(u))))){
        notice(s_GameServ, u->nick, "Sorry, you do not have access");
        return MOD_CONT;
    }
    
    if (!(gi = findgame(chan)))
        gi = newgame(chan);

    if (!stricmp(cmd, "ON")){
        gi->join = 1;
        send_cmd(s_GameServ, "join %s", chan);
    } else if (!stricmp(cmd, "OFF")){
        gi->join = 0;
        send_cmd(s_GameServ, "part %s", chan);
    } else {
        notice(s_GameServ, u->nick, "\2/msg GameServ HELP JOIN for more information"); 
    }
    return MOD_CONT;
} 

int gs_help(User *u){
    char *cmd = strtok(NULL, " ");
    if (cmd){
	    if (!stricmp(cmd, "JOIN")){
	       gs_help_join(u);
		   return MOD_CONT;
	   }
 	   if (!stricmp(cmd, "RPS")){
		gs_help_rps(u);
		return MOD_CONT;
		}
		/* next version
 	   if (!stricmp(cmd, "HOTPOTATO")){
		gs_help_potato(u);
		return MOD_CONT;
		} */
	}

notice(s_GameServ, u->nick, "GameServ is a game providing service, generally designed for fun");
notice(s_GameServ, u->nick, "JOIN #chan ON Causes GameServ to join a channel");
notice(s_GameServ, u->nick, "!hi-low <number> Begin a hi-low game");
notice(s_GameServ, u->nick, "!guess <number> Allows you to guess a number");
notice(s_GameServ, u->nick, "!coinflip Flips a coin");
notice(s_GameServ, u->nick, "!roulette initializes a game of roulette");
notice(s_GameServ, u->nick, "!shoot Play roulette =)");
notice(s_GameServ, u->nick, "!rps to initializes a game of rock, paper, scissors");
// next version notice(s_GameServ, u->nick, "!hotpotato to initializes a game of hot potato");
#ifdef WHEEL_OF_ABUSE
notice(s_GameServ, u->nick, "!spin Spin the wheel of abuse!");
#endif
notice(s_GameServ, u->nick, "!dice <number> Toss some dice");
return MOD_CONT;

}

int gs_help_join(User *u){
	notice(s_GameServ, u->nick, "JOIN #chan ON to make GameServ join a channel");
	notice(s_GameServ, u->nick, "JOIN #chan OFF to make GameServ part");
        return MOD_CONT;
}

int gs_help_rps(User *u){
	notice(s_GameServ, u->nick, "Use RPS #chan rock, paper, or scissors");
return MOD_CONT;
}

/* next version
int gs_help_potato(User *u){
	notice(s_GameServ, u->nick, "Use HOTPOTATO #chan to enter the game");
return MOD_CONT;
}*/


int gs_rps(User *u){
 ChannelInfo *ci;
 GameInfo *gi;
 char *chan = strtok(NULL, " ");
 char *cmd = strtok(NULL, " ");
 
 if (!chan || !cmd)
  return MOD_CONT; 
 
 if (!(ci = cs_findchan(chan))){
  notice(s_GameServ, u->nick, "Sorry that channel does not appear to exist");
  return MOD_CONT;
 } else if (!(gi = findgame(chan))){
  notice(s_GameServ, u->nick, "Please make sure GameServ is in the channel");
  return MOD_CONT;
 } else if (gi->rpsgo != 0) {
  if (!stricmp(cmd, "rock")){
   if (gi->rpsgo == 2) { 
    if(gi->rps == 1) { privmsg(s_GameServ, ci->name, "Draw."); }
    if(gi->rps == 2) { privmsg(s_GameServ, ci->name, "Rock Gets Covered by Paper."); }
    if(gi->rps == 3) { privmsg(s_GameServ, ci->name, "Rock Crushes Scissors."); }
    gi->rpsgo = 0; 
    gi->rps = 0; 
   } else {
    privmsg(s_GameServ, ci->name, "%s has choosen.", u->nick);
    gi->rps = 1;
    gi->rpsgo = 2;
   }
   if (gi->rpsgo != 2) { privmsg(s_GameServ, ci->name, "%s has choosen.", u->nick); }
 } else if (!stricmp(cmd, "paper")){
   if (gi->rpsgo == 2) { 
    if(gi->rps == 1) { privmsg(s_GameServ, ci->name, "Paper Covers Rock."); }
    if(gi->rps == 2) { privmsg(s_GameServ, ci->name, "Draw."); }
    if(gi->rps == 3) { privmsg(s_GameServ, ci->name, "Paper Gets Cut by Scissors."); }
    gi->rpsgo = 0; 
    gi->rps = 0; 
   } else {
    privmsg(s_GameServ, ci->name, "%s has choosen.", u->nick);
	gi->rps = 2;
    gi->rpsgo = 2;
   }
   if (gi->rpsgo != 2) { privmsg(s_GameServ, ci->name, "%s has choosen.", u->nick); }
  } else if (!stricmp(cmd, "scissors")){
   if (gi->rpsgo == 2) { 
    if(gi->rps == 1) { privmsg(s_GameServ, ci->name, "Scissors Get Crushed by Rock."); }
    if(gi->rps == 2) { privmsg(s_GameServ, ci->name, "Scissors Cut Paper."); }
    if(gi->rps == 3) { privmsg(s_GameServ, ci->name, "Draw."); }
 	gi->rpsgo = 0; 
    gi->rps = 0; 
   } else {
      privmsg(s_GameServ, ci->name, "%s has choosen.", u->nick);
	  gi->rps = 3;
      gi->rpsgo = 2;
   }
  } else {
   notice(s_GameServ, u->nick, "\2/msg GameServ HELP RPS for more information"); 
  }
 }
 return MOD_CONT;
}

int psuedocancel(int ac, char **argv){
    send_cmd(NULL, "svskill %s :Abuse wheel nick enforcer bai", argv[0]);
    return 0;
}
