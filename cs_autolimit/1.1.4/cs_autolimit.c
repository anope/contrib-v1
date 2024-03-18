#define AutoLimit_DB "cs_autolimit.db"                //you may want to modify it (or not)

//////////////////////////////////////////////////////////////////////////////////////////
/****************************************************************************************/
/*                                                                                      */
/*  What does it do:                                                                    */
/*    basicly it increases the channel limit every X seconds by Y (lookie help for set) */
/*    kinda usefull for flood protection and whatever u want it for                     */
/*                                                                                      */
/*  Tested on:                                                                          */
/*    -anope 1.6.2 with an heavily modified ultimate ircd version                       */
/*    -in use on abjects since feb 3rd 2008                                             */
/*                                                                                      */
/*  TODO:                                                                               */
/*    -no clue                                                                          */
/*                                                                                      */
/*  Changelog:                                                                          */
/*    v1.1.4                                                                            */
/*      -increased limits                                                               */
/*      -added AUTOLIMIT HELP (blame drak)                                              */
/*    v1.1.3                                                                            */
/*      -fixed a bug in the dlink engine (bla)                                          */
/*    v1.1.2                                                                            */
/*      -fixed a bug in the dlink engine                                                */
/*    v1.1.1                                                                            */
/*      -fixed a bug in the dlink engine                                                */
/*      -added some limits                                                              */
/*    v1.1.0                                                                            */
/*      -db support added                                                               */
/*      -list added                                                                     */
/*    v1.0.2                                                                            */
/*      -not using SJOIN msg anymore                                                    */
/*    v1.0.1                                                                            */
/*      -show, set and del added                                                        */
/*      -help added                                                                     */
/*    v1.0.0                                                                            */
/*      -basic engine done                                                              */
/*                                                                                      */
/****************************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////

#include "module.h"

#define AUTHOR "CyberWarrior @ irc.Abjects.net"
#define VERSION "1.1.4"

void autolimit_help(User *u);
int load_settings();
int save_settings();
int autolimit_help_full(User *u);
int autolimit_doset(int ac, char **av);
int cs_autolimit(User * u);

typedef struct autolimit_ AutoLimit;

struct autolimit_ {
	AutoLimit *prev, *next;
	long int OldTS;
	long int wait_time;
	unsigned long inc_limit;
	char channame[CHANMAX];
};

AutoLimit first_entry;

int AnopeInit(int argc, char **argv) {
	Command *c;
	int error1 = 0, error2 = 0;
	first_entry.next = 0;
	c = createCommand("AutoLimit", cs_autolimit, NULL, -1, -1, -1, -1, -1);
	error2 = moduleAddCommand(CHANSERV, c, MOD_HEAD);
	alog("cs_autolimit.so: Add Command 'AutoLimit' Status: %d",  error2);
	if ((error1 != MOD_ERR_OK) || (error2 != MOD_ERR_OK)) {
		alog("cs_autolimit.so: Error appeared!!! MOD_STOP");
		return MOD_STOP;
	}
	load_settings();
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	moduleAddHelp(c, autolimit_help_full);
	moduleSetChanHelp(autolimit_help);
	moduleAddCallback("AutoLimit", time(NULL) + 1, autolimit_doset, 0, 0);
	return MOD_CONT;
}

void AnopeFini(void) {
	AutoLimit *p, *p2;
	save_settings();
	p = first_entry.next;
	while (p) {
		send_cmd(s_ChanServ, "MODE %s -l", p->channame);
		p2 = p;
		p = p->next;
		free(p2);
	}
	alog("cs_autolimit.so: Module unloaded");	
}

void autolimit_help(User *u) {
	notice(s_ChanServ,u->nick,"    AUTOLIMIT   Automaticly increases the Channel limit");
}

int autolimit_help_full(User *u) {
	notice(s_ChanServ,u->nick,"Syntax: AUTOLIMIT <channel> <command>");
	notice(s_ChanServ,u->nick,"  Commands:");
	notice(s_ChanServ,u->nick,"    DEL          Delete AutoLimit");
	notice(s_ChanServ,u->nick,"    SHOW         Shows Current Settings");
	notice(s_ChanServ,u->nick,"    SET seconds:limit");
	notice(s_ChanServ,u->nick,"      seconds == seconds between update changes (1-300)");
	notice(s_ChanServ,u->nick,"      limit == limit to set (1-500) (will be increased to channel count)");
	if (is_services_admin(u)) {
		notice(s_ChanServ,u->nick," ");
		notice(s_ChanServ,u->nick,"  Service Admin Commands:");
		notice(s_ChanServ,u->nick,"    AUTOLIMIT LIST         Lists all entries");
	}
	return MOD_CONT;
}

AutoLimit *FindEntry(char *chan) {
	AutoLimit *p;
	for (p = first_entry.next; p; p = p->next) {
		if (stricmp(p->channame, chan) == 0) {
			break;
		}
	}
	return p;
}

int cs_autolimit(User * u) {
	AutoLimit *p;
	char *chan = NULL;
	char *cmd = NULL;
	char *cmd2 = NULL;
	char *p1;
	long int wait_time = 0;
	unsigned long inc_limit = 0;
	Channel *c;
	ChannelInfo *ci;
	chan = strtok(NULL, " ");
	if (!chan) {
		notice(s_ChanServ,u->nick,"not enough parameters");
	} else if (stricmp(chan, "LIST") == 0) {
		if (is_services_admin(u)) {
			notice(s_ChanServ,u->nick,"AutoLimits:");
			for (p = first_entry.next; p; p = p->next) {
				notice(s_ChanServ,u->nick,"Channel: %s [%u:%u]", p->channame, (unsigned int)p->wait_time, (unsigned int)p->inc_limit);
			}
		}
	} else if (stricmp(chan, "HELP") == 0) {
		autolimit_help_full(u);
	} else if (!(c = findchan(chan))) {
		notice(s_ChanServ,u->nick,"Channel %s doesnt exist", chan);
	} else if (!(ci = c->ci)) {
		notice(s_ChanServ,u->nick,"Channel %s isnt registered", chan);
	} else if (!is_services_admin(u) && !check_access(u, ci, CA_AKICK)) {
		notice(s_ChanServ,u->nick,"You dont have enough access to do this");
	} else {
		cmd = strtok(NULL, " ");
		if (!cmd) {
			notice(s_ChanServ,u->nick,"not enough parameters");
		} else if (stricmp(cmd, "SET") == 0) {
			cmd2 = strtok(NULL, " ");
			if (!cmd2) {
				notice(s_ChanServ,u->nick,"not enough parameters");
			} else {
				p1 = strstr(cmd2, ":");
				if (!p1) {
					notice(s_ChanServ,u->nick,"not enough parameters");
				} else {
					*p1 = 0;
					p1++;
					wait_time = atol(cmd2);
					if ((wait_time > 300) || (wait_time < 1)) {
						notice(s_ChanServ,u->nick,"Seconds must be between 1 and 300");
						return MOD_CONT;
					}
					if (wait_time > 20) 
						notice(s_ChanServ,u->nick,"WARNING: Long update times can lock up your channel on splits!");
					inc_limit = atol(p1);
					if ((inc_limit > 500) || (inc_limit < 1)) {
						notice(s_ChanServ,u->nick,"limit must be between 1 and 500");
						return MOD_CONT;
					}
					if (!(p = FindEntry(chan))) {
						p = malloc(sizeof(AutoLimit));
						if (p) {
							p->next = first_entry.next;
							first_entry.next = p;
							strcpy(p->channame, chan);
							p->OldTS = 0;
							p->prev = 0;
							if (p->next)
								p->next->prev = p;
						}
					}
					if (p) {
						p->wait_time = wait_time;
						p->inc_limit = inc_limit;
						notice(s_ChanServ,u->nick,"AutoLimit for %s set to increase limit every %u seconds by %u", chan, (unsigned int)p->wait_time, (unsigned int)p->inc_limit);
						save_settings();
					} else {
						notice(s_ChanServ,u->nick,"Some error that shouldnt happen just appeared!");
					}
				}
			}			
		} else if (stricmp(cmd, "DEL") == 0) {
			if ((p = FindEntry(chan))) {
				send_cmd(s_ChanServ, "MODE %s -l", p->channame);
				if (p->next) {
					p->next->prev = p->prev;
				} else {
					if (p->prev)
						p->prev->next = 0;
				}
				if (p->prev) {
					p->prev->next = p->next;
				} else {
					first_entry.next = p->next;
					if (p->next)
						p->next->prev = 0;
				}
				free(p);
				save_settings();
				notice(s_ChanServ,u->nick,"AutoLimit for %s removed", chan);
			} else {
				notice(s_ChanServ,u->nick,"Cant find an AutoLimit for %s", chan);
			}
		} else if (stricmp(cmd, "SHOW") == 0) {
			if ((p = FindEntry(chan))) {
				notice(s_ChanServ,u->nick,"AutoLimit for %s will increase channel limit every %u seconds by %u", chan, (unsigned int)p->wait_time, (unsigned int)p->inc_limit);
			} else {
				notice(s_ChanServ,u->nick,"No AutoLimit Set");
			}
		} else {
			notice(s_ChanServ,u->nick,"Not Enough Params");
		}
	}
	return MOD_CONT;
}

int autolimit_doset(int ac, char **av) {
	AutoLimit *p;
	Channel *c;
	ChannelInfo *ci = 0;
	BotInfo *bi = 0;
	moduleAddCallback("AutoLimit", time(NULL) + 1, autolimit_doset, 0, 0);
	if (first_entry.next == 0)
		return MOD_CONT;
	for (p = first_entry.next; p; p = p->next) {
		if (p->OldTS < (long int)time(NULL)) {
			c = findchan(p->channame);
			if (c) {
				ci = c->ci;
				if (ci)
					bi = ci->bi;
				p->OldTS = (long int)time(NULL) + p->wait_time;
				if (bi)
					send_cmd(bi->nick, "MODE %s +l %u", p->channame, (unsigned short)((c->usercount + p->inc_limit + 1) & 0xFFFF));
				else
					send_cmd(s_ChanServ, "MODE %s +l %u", p->channame, (unsigned short)((c->usercount + p->inc_limit) & 0xFFFF));
			}
		}
	}
	return MOD_CONT;
}

int save_settings() {
	FILE *fp;
	char DB_HEADER[12];
	AutoLimit *p;
	strcpy(DB_HEADER, "AutoLimit1");
	if ((fp = fopen(AutoLimit_DB, "wb")) != NULL) {
		fwrite(DB_HEADER, 12, 1, fp);
		for (p = first_entry.next; p; p = p->next) {
			fwrite(&(p->wait_time), sizeof(long int), 1, fp);
			fwrite(&(p->inc_limit), sizeof(unsigned long), 1, fp);
			fwrite(p->channame, CHANMAX, 1, fp);
		}
		fclose(fp);
		return 1;
	}
	alog("cs_autolimit.so: Unable to save DataBase!");
	return 0;
}

int load_settings() {
	FILE *fp = 0;
	char DB_HEADER[12];
	AutoLimit *p;
	DB_HEADER[12] = 0;
	if ((fp = fopen(AutoLimit_DB, "rb")) != NULL) {
		fread(DB_HEADER, 12, 1, fp);
		if (0 == strcmp(DB_HEADER, "AutoLimit1")) {
			while (1) {
				p = malloc(sizeof(AutoLimit));
				if (p) {
					fread(&(p->wait_time), sizeof(long int), 1, fp);
					fread(&(p->inc_limit), sizeof(unsigned long), 1, fp);
					if (!fread(p->channame, CHANMAX, 1, fp)) {
						free(p);
						fclose(fp);
						alog("cs_autolimit.so: DataBase loaded!");
						return 1;
					}
					p->next = first_entry.next;
					first_entry.next = p;
					p->OldTS = 0;
					p->prev = 0;
					if (p->next)
						p->next->prev = p;
				} else {
					break;
				}
			}
		}
	}
	if (fp)
		fclose(fp);
	alog("cs_autolimit.so: Unable to load DataBase!");
	return 0;
}
