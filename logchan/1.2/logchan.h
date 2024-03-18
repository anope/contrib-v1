/*
 * Thanks to Trystan, for helping me with the usermodes logging *
 * Thanks to iZach, for testing my module on his network *
 * Thanks to the original author of log_jpk, who first inspired me to write this *
 * Thanks to Rob, for helping me debug my code *
*/

int LogChanInit(void);
int BindEvents(void);

/* Connects, Disconnects, Kills, Nick Changes */ 
int log_newnick(int argc, char **argv);
int log_user_logoff(char *source, int argc, char **argv);
int log_nick_change(char *source, int argc, char **argv);
int log_user_kill(char *source, int argc, char **argv);

/* Joins, Parts, Kicks */ 
int log_joins(int argc, char **argv);
int log_parts(int argc, char **argv);
int log_kicks(char *source, int argc, char **argv);

/* User Mode && SNotices */
int logchan_umode(char *source, int ac, char **av);
void my_set_umode(User * user, int ac, char **av);
int log_sno(char *source, int argc, char **argv);

/* BotServ */
int log_botadd(int argc, char **argv);
int log_botdel(int argc, char **argv);
int log_botAssign(int argc, char **argv);
int log_botUnassign(int argc, char **argv);
int log_botKick(int argc, char **argv);
int log_botBan(int argc, char **argv);

/* Restart, Shutdown */
int log_restart(int argc, char **argv);
int log_shutdown(int argc, char **argv);

/* DefCon, Topic */
int log_defcon(int argc, char **argv);
int log_topic(int argc, char **argv);

/* Away */
int log_away(char *source, int argc, char **argv);

/* Modes */
int log_chan_mode(char *source, int argc, char **argv); 

/* GlobOps, TKL */
int log_globops(char *source, int argc, char **argv);
int log_tkl(char *source, int argc, char **argv); 
