/**
 * This module does many routing related things on your network. First
 * and primarily, it keeps track of the servers on your network. Whenever
 * a server joins, it adds it to it's database. The database can track
 * the server admin, hub, linking port, last time the server was seen, 
 * and testlink information. Testlink information is configurable. It will
 * always track the start time of a testlink, and the number of times that
 * server split. You can also set it to track a certain number of durations
 * of recent splits for a testlink'ed server. The module will attempt to
 * relink any server that splits, if it is set to, but it can be used simply
 * to track information. It can also be programmed for a delay now.
 *
 * This module has been tested with Liquid, Ultimate, and Bahamut.
 *
 * BUG REPORTS, COMMENTS, PRAISE ARE APPRECIATED! :-p
 *
 * Based on os_routing v2.0
 * By: DaPrivateer
 */

#include "services.h"
#include "pseudo.h"
#include "module.h" 

#define MYNAME "os_routing"
#define AUTHOR "katsklaw" 
#define VERSION "2.2"

/****************************************
 * Begin Configuration Block            *
 ****************************************/

/* Set this to the port your network uses for
   servers linking [Required] */
#define DEFPORT 5000

/* This should be the name of the server that services
   connects to. This is necessary because services
   does not keep track of servers that are online.
   [Required] */
#define RT_LINKSERV "link.server.here"

/* This sets how often ROUTING will automatically
   run it's SYNCH routine (in seconds) [Required] */
#define SYNCH_TIME  900

/* Uncomment and define this if you want routing
   to delay a certain number of seconds before 
   attempting to relink a split server. If you
   want it to attempt immediate relink, comment
   out this line. [Optional] */
#define LINK_DELAY 20

/* Uncomment and define this if you want routing
   to stop trying to connect a server after a 
   certain number of attempts. If you want there
   to be no limit, comment out this line. [Optional] */
#define MAX_ATTEMPTS 8

/* Uncomment and define this if you want routing
   to track the most recent split durations of
   testlinked servers. The number should be the
   number of split times you want to track. When
   the system runs out of space, it writes the 
   oldest split time to the log before deleting it.
   If you don't want to track split durations, comment
   out this line. [Optional] */
#define SPLIT_TIME_TRACK 10

/* Uncomment and define this if you want routing
  to not save it's database. It will still load
  a database if one exists, but it won't save any
  new changes. This line should remain commented
  unless you really need it to not save. [Optional] */
//#define NO_SAVE

/* This is the name of the database
   file on the server. [Required] */
#define RT_DATABASE "routing.db"

/* This is the version of the database
   file on the server. End users SHOULD 
   NOT modify this. [Required] */
#define RT_DB_VER   2

/* These are the messages used by ROUTING
   when communicating with users. If you 
   edit them, do NOT modify the number of
   anything starting with a %! You should
   leave these alone unless you really need
   to modify them. */
#define RT_NOACCESS         "Permission denied."
#define RT_ADD_SYNTAX       "Syntax: \002/OperServ ROUTING ADD \037server\037 [\037port\037] { \037hub\037 | NULL }\002"
#define RT_INV_PORT         "Invalid port number. Valid ports are from 1 to 65536."
#define RT_ADD_EXISTS       "Server %s already exists!"
#define RT_SRV_ADDED        "Server %s has been added"
#define RT_SRV_UNKNOWN      "Server %s not found in the database"
#define RT_SRV_VIEW_HDR     "Server information:"
#define RT_SRV_VIEW_NAME    "Server Name: %s"
#define RT_SRV_VIEW_ADMIN   "Server Admin: %s"
#define RT_SRV_VIEW_TL      "This server is a \002Test-Link\002"
#define RT_SRV_VIEW_LINK    "Server Status: %s"
#define RT_SRV_VIEW_PORT    "Linking Port: %d"
#define RT_SRV_VIEW_HUB     "Linking Hub:  %s"
#define RT_SRV_VIEW_LAST    "Last Seen Time: %s"
#define RT_SRV_VIEW_HOLD    "Hold Status: %s"
#define RT_SRV_TL_START     "Testlink began on %s"
#define RT_SRV_TL_SPLIT     "This server has split %d times"
#define RT_SRV_TL_LHEAD     "Split durations:"
#define RT_SRV_TL_TIME      "#%d lasted %s"
#define RT_DEL_SYNTAX       "Syntax: \002/OperServ ROUTING DEL \037server\037\002"
#define RT_SRV_DELETED      "Server %s has been deleted"
#define RT_VIEW_SYNTAX      "Syntax: \002/OperServ ROUTING VIEW \037server\037\002"
#define RT_LIST_HEAD        "Num   Name      Status   Hold"
#define RT_LIST_LINE        "%d    %s        %s       %s"
#define RT_LIST_LINE_TL     "%d    %s (TL)   %s       %s"
#define RT_LIST_END         "End of list"
#define RT_LIST_ADM_HEAD    "Server Admin List"
#define RT_LIST_ADM_LINE    "%s"
#define RT_LIST_ADM_NOAD    "%d server(s) did not have an admin set"
#define RT_LIST_ADM_NONE    "No server admins are set in the database"
#define RT_LIST_NONE        "No servers matched your request"
#define RT_LIST_SYNTAX      "Syntax: \002/OperServ ROUTING LIST [ { ALL | ACTIVE | SPLIT | HUBLESS | TESTLINK | LEAFS | ADMIN } ]\002"
#define RT_LIST_LEAF_SYNTAX "Syntax: \002/OperServ ROUTING LIST LEAFS \037hub\037\002"
#define RT_AUTH_SYNTAX      "Syntax: \002/OperServ ROUTING AUTH \037server\037\002"
#define RT_SRV_AHELD        "Server %s is already has a HOLD status: %s"
#define RT_SRV_HELD         "Server %s is now on HOLD"
#define RT_SYNTAX           "Syntax: \002/OperServ ROUTING { ON | OFF | ADD | SET | DEL | VIEW | LIST | LINK | AUTH | SYNCH }\002"
#define RT_SET_SYNTAX       "Syntax: \002/OperServ ROUTING SET \037server\037 { NAME | PORT | HUB | HOLD | ADMIN | TESTLINK } \037value\037\002"
#define RT_SET_NAME_SYNTAX  "Syntax: \002/OperServ ROUTING SET \037server\037 NAME \037new name\037\002"
#define RT_SET_NAME         "Server name has been changed to %s"
#define RT_SET_PORT_SYNTAX  "Syntax: \002/OperServ ROUTING SET \037server\037 PORT { 1 - 65536 }\002"
#define RT_SET_PORT         "Linking port has been changed to %d"
#define RT_SET_HUB_SYNTAX   "Syntax: \002/OperServ ROUTING SET \037server\037 HUB \037new hub\037\002"
#define RT_SET_HUB          "Linking hub has been changed to %s"
#define RT_SET_HUB_SELF     "You can not set a server's hub to itself!"
#define RT_SET_HOLD_SYNTAX  "Syntax: \002/OperServ ROUTING SET \037server\037 HOLD { OFF | TEMP | PERM }\002"
#define RT_SET_HOLD         "Hold status has been changed to %s"
#define RT_SET_TL_SYNTAX    "Syntax: \002/OperServ ROUTING SET \037server\037 TESTLINK { ON | OFF }\002"
#define RT_SET_TL           "Server is %s listed as a Test-Link"
#define RT_SET_TL_ON_ALRDY  "This server is already a Test-Link"
#define RT_SET_TL_OFF_ALRDY "This server is NOT listed as a Test-Link"
#define RT_SET_ADMIN_SYNTAX "Syntax: \002/OperServ ROUTING SET \037server\037 ADMIN { \037nick\037 | NULL }\002"
#define RT_SET_ADMIN        "Server admin is now %s"
#define RT_LIMIT_SRA        "Limited to \002Services Root Administrators\002"
#define RT_LIMIT_SA         "Limited to \002Services Administrators\002"
#define RT_LIMIT_SO         "Limited to \002Services Operators\002"
#define RT_ON_SYNTAX        "Syntax: \002/OperServ ROUTING ON\002"
#define RT_ON_ALREADY       "ROUTING is already ON!"
#define RT_ON               "ROUTING has been turned ON"
#define RT_OFF_SYNTAX       "Syntax: \002/OperServ ROUTING OFF\002"
#define RT_OFF_ALREADY      "ROUTING is already OFF!"
#define RT_OFF              "ROUTING has been turned OFF"
#define RT_USE_AUTH         "Use AUTH to achieve this - \002/OperServ HELP ROUTING AUTH\002"
#define RT_SYNCH_SYNTAX     "Syntax: \002/OperServ ROUTING SYNCH\002"
#define RT_SYNCH            "Attempting to re-SYNCH server data"
#define RT_LINK_SYNTAX      "Syntax: \002/OperServ ROUTING LINK \037server\037\002"
#define RT_LINK_NOHUB       "Cannot Link: No hub on file for server %s"
#define RT_ACTIVE           "Active"
#define RT_SPLIT            "\002SPLIT\002"
#define RT_JUPED            "\002JUPE'd\002"

/****************************************
 * End Configuration Block              *
 ****************************************/

//******* DO NOT MODIFY ANYTHING BELOW THIS LINE *********
//*******   UNLESS YOU KNOW WHAT YOU ARE DOING!  *********

//For our list function
#define RT_LIST_ALL     1
#define RT_LIST_ACT     2
#define RT_LIST_SPLIT   3
#define RT_LIST_HUBLESS 4
#define RT_LIST_TL      5

//List with params
#define RT_LIST_LEAFS   6
#define RT_LIST_ADMIN   7

//Define our structs so they are easier to use
typedef struct ServerInfo_ SrvInfo;
typedef struct ServerHash_ SrvHash;
typedef struct TLInfo_     TLInfo;

#ifdef SPLIT_TIME_TRACK
#if SPLIT_TIME_TRACK < 1
#undef SPLIT_TIME_TRACK
#endif
#endif

//Server information storage
struct ServerInfo_ {
	char *name;
	int32 port;
	int link;
	SrvInfo *to;
	char *hub;
	int hold;
	int leafs;
	TLInfo *testlink;
	char *admin;
	int attempts;
	time_t last;
};

//Hash for server info
struct ServerHash_ {
	SrvInfo *serv;
	SrvHash *next;
};

struct TLInfo_ {
	time_t start;
	int16 splits;
#ifdef SPLIT_TIME_TRACK
	int32 times[SPLIT_TIME_TRACK];
#endif
};
	

//Where we actually store the date
SrvHash *servers[MAX_CMD_HASH];

//Keep track of our status
int ImOn=0;
int ImSynch=0;
time_t LastSynch=0;

//Interactive Functions
int m_routing(User *u);
int m_jupe(User *u);
int m_mystats(User *u);
int m_update(User *u);
int m_identify(User *u);

//Help Functions
void AddHelp();
int RT_Help(User *u);
int RT_Help_ON(User *u);
int RT_Help_OFF(User *u);
int RT_Help_ADD(User *u);
int RT_Help_SET(User *u);
int RT_Help_SET_NAME(User *u);
int RT_Help_SET_PORT(User *u);
int RT_Help_SET_HUB(User *u);
int RT_Help_SET_HOLD(User *u);
int RT_Help_SET_ADMIN(User *u);
int RT_Help_SET_TL(User *u);
int RT_Help_DEL(User *u);
int RT_Help_VIEW(User *u);
int RT_Help_LIST(User *u);
int RT_Help_LINK(User *u);
int RT_Help_AUTH(User *u);
int RT_Help_SYNCH(User *u);
void RT_OS_Help(User *u);

//Event Handlers
int handle_squit(char *source, int ac, char **av);
int handle_server(char *source, int ac, char **av);
int handle_364(char *source, int ac, char **av);
int handle_365(char *source, int ac, char **av);
int handle_402(char *source, int ac, char **av);

//Misc Functions
int isServer(char *name);
int updateServStatus(void);
void linkServer(SrvInfo *serv);
void relinkServers(void);
void checkLeafs(SrvInfo *serv);
char *myDoTime(int32 rhs);
char *servStatus(SrvInfo *serv);
int countSplits(void);
void listServers(int what, User *u);
void listServersParam(int what, User *u, void *param);
void listAdmins(User *u);
static int CompareEntries(SList *list, void *entry1, void *entry2);
void doExpire(void);
#ifdef LINK_DELAY
	static int CompareSrvEntries(SList *list, void *entry1, void *entry2);
	void relinkServersDelay(void);
#endif

//Hash Utilities
SrvInfo *createServer(char *name, int32 port, char *hub);
SrvInfo *createServerFull(char *name, int32 port, char *hub, int hold, int32 last, char *ad);
SrvInfo *findServer  (char *name);
SrvInfo *findServerByID (int id);
int addSrvHash (SrvInfo *serv);
int delSrvHash (SrvInfo *serv);
#ifdef SPLIT_TIME_TRACK
	void addTLInfo(SrvInfo *serv, int32 start, int16 splits, int32 times[]);
#else
	void addTLInfo(SrvInfo *serv, int32 start, int16 splits);
#endif

//Database Functions
void load_rt_db(void);
void load_rt_db_v1(dbFILE *f);
void load_rt_db_v2(dbFILE *f);
void save_rt_db(void);

//Delay link server data
#ifdef LINK_DELAY
	SListOpts SPOpts = { SLISTF_SORT, &CompareSrvEntries, NULL, NULL };
	SList splits;
	int FSplit=0;
#endif

//**********************************************************************************

/****************************************
 * Module Init/Term Functions           *
 ****************************************/

//Module initialization
int AnopeInit(int argc, char **argv) { 
    Command *c; 
	Message *msg;
    int status = 0, i=0;

    //Handles servers quitting
	msg = createMessage ("SQUIT", handle_squit);
	alog("%s: Adding handler for SQUIT; Result: %d", MYNAME, moduleAddMessage(msg,MOD_TAIL));
	
	//Handles servers joining
	msg = createMessage ("SERVER", handle_server);
	alog("%s: Adding handler for SERVER; Result: %d", MYNAME, moduleAddMessage(msg,MOD_TAIL));
	
	//Handles RPL_LIST for synching
	msg = createMessage ("364", handle_364);
	alog("%s: Adding handler for 364; Result: %d", MYNAME, moduleAddMessage(msg,MOD_HEAD));

	//Handles RPL_LISTEND for synching
	msg = createMessage ("365", handle_365);
	alog("%s: Adding handler for 365; Result: %d", MYNAME, moduleAddMessage(msg,MOD_HEAD));

	//Handles NOSUCHSERVER for synching
	msg = createMessage ("402", handle_402);
	alog("%s: Adding handler for 402; Result: %d", MYNAME, moduleAddMessage(msg,MOD_HEAD));

	c = createCommand("ROUTING", m_routing, NULL,-1,-1,-1,-1,-1);
    status += moduleAddCommand(OPERSERV, c, MOD_TAIL);
	moduleAddHelp(c, RT_Help);

	c = createCommand("JUPE", m_jupe, is_services_admin,-1,-1,-1,-1,-1);
	status += moduleAddCommand(OPERSERV, c, MOD_HEAD);
	
	c = createCommand("STATS", m_mystats, NULL,-1,-1,-1,-1,-1);
	status += moduleAddCommand(OPERSERV, c, MOD_TAIL);

	c = createCommand("UPDATE", m_update, NULL,-1,-1,-1,-1,-1);
    status += moduleAddCommand(OPERSERV, c, MOD_TAIL);

	c = createCommand("RESTART", m_update, NULL,-1,-1,-1,-1,-1);
    status += moduleAddCommand(OPERSERV, c, MOD_TAIL);
	
	c = createCommand("SHUTDOWN", m_update, NULL,-1,-1,-1,-1,-1);
    status += moduleAddCommand(OPERSERV, c, MOD_TAIL);

	c = createCommand("IDENTIFY", m_identify, NULL,-1,-1,-1,-1,-1);
    status += moduleAddCommand(NICKSERV, c, MOD_TAIL);

    if (status == 0) {
        alog("[%s.so] Loaded successfully", MYNAME);
    } else {
        alog("[%s.so] FAILED to load - result %d", MYNAME, status);
    }

	for (i=0; i<MAX_CMD_HASH; i++)
		servers[i]=NULL;
	
#ifdef LINK_DELAY
	slist_init(&splits);
	splits.opts = &SPOpts;
#endif

	load_rt_db();

	updateServStatus();

	AddHelp();
	moduleSetOperHelp(RT_OS_Help);

#ifdef START_OFF
	ImOn = 0;
#else
	ImOn = 1;
#endif
	send_cmd(s_OperServ, "MODE OperServ +o");

    moduleAddAuthor(AUTHOR); 
    moduleAddVersion(VERSION);
    return MOD_CONT;
} 

//Module termination
void AnopeFini(void) {
	doExpire();
	save_rt_db();

    alog("[%s.so] Unloaded", MYNAME);
}

/****************************************
 * Interactive Functions                *
 ****************************************/

int m_routing(User *u) {
	char *args;
	char *cmd; 
	char *name, *hub;
	int32 port = 0;
#ifdef SPLIT_TIME_TRACK
	int i;
#endif
	time_t temp;
	SrvInfo *serv = NULL;
	SrvInfo *serv2 = NULL;
	NickCore *nc;
	struct tm *last;
	struct tm *myTime;
	char buf[512];

	args = moduleGetLastBuffer();
	cmd = strtok(args," ");
	
	if (!u)				//msg from non-existent user?
		return MOD_CONT;

	if (!cmd) {  //Uhh, they didn't tell us what to do?
		if (is_services_oper(u)) {
			notice(s_OperServ, u->nick, RT_SYNTAX);
		} else {
			notice(s_OperServ, u->nick, RT_NOACCESS);
		}

		return MOD_CONT;
	}
	
	if (!stricmp(cmd,"ON")) {
		if (!is_services_admin(u)) {
			notice(s_OperServ, u->nick, RT_NOACCESS);
			return MOD_CONT;
		}

		if (ImOn) {
			notice(s_OperServ, u->nick, RT_ON_ALREADY);
			return MOD_CONT;
		}

		ImOn=1;
		notice(s_OperServ, u->nick, RT_ON);
		alog("%s: %s turned ROUTING ON", MYNAME, u->nick);
		wallops(s_OperServ, "Routing: %s turned ROUTING ON", u->nick);

		return MOD_CONT;
	}

	if (!stricmp(cmd,"OFF")) {
		if (!is_services_admin(u)) {
			notice(s_OperServ, u->nick, RT_NOACCESS);
			return MOD_CONT;
		}

		if (!ImOn) {
			notice(s_OperServ, u->nick, RT_OFF_ALREADY);
			return MOD_CONT;
		}

		ImOn=0;
		notice(s_OperServ, u->nick, RT_OFF);
		alog("%s: %s turned ROUTING OFF", MYNAME, u->nick);
		wallops(s_OperServ, "Routing: %s turned ROUTING OFF", u->nick);

		return MOD_CONT;
	}
	
	if (!stricmp(cmd,"ADD")) {
		name = strtok(NULL," ");
		hub = strtok(NULL," ");

		if (!is_services_admin(u)) {
			notice(s_OperServ, u->nick, RT_NOACCESS);
			return MOD_CONT;
		}

		if ((!name) || (!hub)) {
			notice(s_OperServ, u->nick, RT_ADD_SYNTAX);
			return MOD_CONT;
		}
		
		if (!isServer(hub)) {  //Are they giving us a port number?
			port = (int32)strtol(hub, (char **)NULL, 10);
			if ((port < 1) || (port > 65536)) {
				notice(s_OperServ, u->nick, RT_INV_PORT);
				return MOD_CONT;
			}
			hub = strtok(NULL," ");
		} else {
			port = DEFPORT;
		}
		if ((!hub) || (!isServer(hub)) || (!isServer(name))) { //Has to be a valid server name
			notice(s_OperServ, u->nick, RT_ADD_SYNTAX);
			return MOD_CONT;
		}
		if (findServer(name)) {  //Already exists; don't add again
			notice(s_OperServ, u->nick, RT_ADD_EXISTS, name);
			return MOD_CONT;
		}
		if (!stricmp(hub,"NULL"))
			hub = NULL;

		if ((serv2=findServer(hub))==NULL) {
			notice(s_OperServ, u->nick, RT_SRV_UNKNOWN, name);
			return MOD_CONT;
		}

		serv = createServer(name,port,serv2->name);
		addSrvHash(serv);

		alog("%s: %s added %s (Port: %d - Hub %s)", MYNAME, u->nick, name, port, hub ? serv2->name : "No hub specified");
		notice(s_OperServ, u->nick, RT_SRV_ADDED, name);

		return MOD_CONT;
	}

	if (!stricmp(cmd,"SET")) {
		name = strtok(NULL," ");
		cmd = strtok(NULL," ");
		hub = strtok(NULL," ");

		if (!is_services_admin(u)) {
			notice(s_OperServ, u->nick, RT_NOACCESS);
			return MOD_CONT;
		}
		
		if (!cmd) {  //Uhh, they didn't tell us what to do?
			if (is_services_admin(u)) {
				notice(s_OperServ, u->nick, RT_SYNTAX);
			} else {
				notice(s_OperServ, u->nick, RT_NOACCESS);
			}

			return MOD_CONT;
		}

		if (!stricmp(cmd,"NAME")) {

			if (!is_services_root(u)) {
				notice(s_OperServ, u->nick, RT_NOACCESS);
				return MOD_CONT;
			}
			if ((!name) || (!hub)) {
				notice(s_OperServ, u->nick, RT_SET_NAME_SYNTAX);
				return MOD_CONT;
			}
			if ((serv=findServer(name))==NULL) {
				notice(s_OperServ, u->nick, RT_SRV_UNKNOWN, name);
				return MOD_CONT;
			}
			if (!isServer(hub)) {
				notice(s_OperServ, u->nick, RT_SET_NAME_SYNTAX);
				return MOD_CONT;
			}
			
			alog("%s: %s changed the name of %s to %s", MYNAME, u->nick, serv->name, name);
			notice(s_OperServ, u->nick, RT_SET_NAME, hub);
			free(serv->name);
			serv->name = sstrdup(hub);

			return MOD_CONT;
		}

		if (!stricmp(cmd,"PORT")) {
			
			if (!is_services_admin(u)) {
				notice(s_OperServ, u->nick, RT_NOACCESS);
				return MOD_CONT;
			}
			if ((!name) || (!hub)) {
				notice(s_OperServ, u->nick, RT_SET_PORT_SYNTAX);
				return MOD_CONT;
			}
			if ((serv=findServer(name))==NULL) {
				notice(s_OperServ, u->nick, RT_SRV_UNKNOWN, name);
				return MOD_CONT;
			}
			port = (int32)strtol(hub, (char **)NULL, 10);
			if ((port < 1) || (port > 65536)) {
				notice(s_OperServ, u->nick, RT_INV_PORT);
				return MOD_CONT;
			}
			serv->port = port;
			alog("%s: %s set connect port of %s to %d", MYNAME, u->nick, serv->name, port);
			notice(s_OperServ, u->nick, RT_SET_PORT, port);
			
			return MOD_CONT;
		}

		if (!stricmp(cmd,"HUB")) {
			if (!is_services_admin(u)) {
				notice(s_OperServ, u->nick, RT_NOACCESS);
				return MOD_CONT;
			}
			if ((!name) || (!hub)) {
				notice(s_OperServ, u->nick, RT_SET_HUB_SYNTAX);
				return MOD_CONT;
			}
			if ((serv=findServer(name))==NULL) {
				notice(s_OperServ, u->nick, RT_SRV_UNKNOWN, name);
				return MOD_CONT;
			}
			if ((serv2=findServer(hub))==NULL) {
				notice(s_OperServ, u->nick, RT_SRV_UNKNOWN, hub);
				return MOD_CONT;
			}

			if (serv2==serv) {
				notice(s_OperServ, u->nick, RT_SET_HUB_SELF);
				return MOD_CONT;
			}

			alog("%s: %s changed the hub for %s to %s", MYNAME, u->nick, serv->name, serv2->name);
			notice(s_OperServ, u->nick, RT_SET_HUB, serv2->name);
			free(serv->hub);

			serv->hub = sstrdup(serv2->name);

			return MOD_CONT;
		}

		if (!stricmp(cmd,"HOLD")) {
			if (!is_services_admin(u)) {
				if (is_services_oper(u) && (atoi(hub)==1)) {
					notice(s_OperServ, u->nick, RT_USE_AUTH);
					return MOD_CONT;
				}
				notice(s_OperServ, u->nick, RT_NOACCESS);
				return MOD_CONT;
			}

			if ((!name) || (!hub)) {
				notice(s_OperServ, u->nick, RT_SET_HOLD_SYNTAX);
				return MOD_CONT;
			}
			if ((serv=findServer(name))==NULL) {
				notice(s_OperServ, u->nick, RT_SRV_UNKNOWN, name);
				return MOD_CONT;
			}
			
			port=-1;
			if (!stricmp(hub,"OFF"))
				port=0;
			else if (!stricmp(hub,"TEMP"))
				port=1;
			else if (!stricmp(hub,"PERM"))
				port=2;

			if (port==-1) {
				notice(s_OperServ, u->nick, RT_SET_HOLD_SYNTAX);
				return MOD_CONT;
			}

			serv->hold=(int)port;
			if (port) {
				alog("%s: %s SET HOLD for %s (%s)", MYNAME, u->nick, serv->name, (port==1) ? "TEMPORARY" : "PERMANENT");
			} else {
				alog("%s: %s REMOVED HOLD for %s", MYNAME, u->nick, serv->name);
			}
			notice(s_OperServ, u->nick, RT_SET_HOLD, (port==2) ? "PERMANENT" : ((port==1) ? "TEMPORARY" : "OFF"));
			
			return MOD_CONT;
		}

		if (!stricmp(cmd,"ADMIN")) {
			if (!is_services_admin(u)) {
				notice(s_OperServ, u->nick, RT_NOACCESS);
				return MOD_CONT;
			}
			if ((!name) || (!hub)) {
				notice(s_OperServ, u->nick, RT_SET_ADMIN_SYNTAX);
				return MOD_CONT;
			}
			if ((serv=findServer(name))==NULL) {
				notice(s_OperServ, u->nick, RT_SRV_UNKNOWN, name);
				return MOD_CONT;
			}
			if (stricmp(hub,"NULL")) {
				if ((nc=findcore(hub))==NULL) {
					notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, hub);
					return MOD_CONT;
				}
			
				alog("%s: %s changed the admin for %s to %s", MYNAME, u->nick, serv->name, nc->display);
				notice(s_OperServ, u->nick, RT_SET_ADMIN, nc->display);
				serv->admin = sstrdup(nc->display);

				return MOD_CONT;
			} else {
				alog("%s: %s deleted the admin for %s", MYNAME, u->nick, serv->name);
				notice(s_OperServ, u->nick, RT_SET_ADMIN, "NULL");
				serv->admin = NULL;

				return MOD_CONT;
			}
		}

		if (!stricmp(cmd,"TESTLINK")) {
			if (!is_services_admin(u)) {
				notice(s_OperServ, u->nick, RT_NOACCESS);
				return MOD_CONT;
			}
			if ((!name) || (!hub)) {
				notice(s_OperServ, u->nick, RT_SET_TL_SYNTAX);
				return MOD_CONT;
			}
			if ((serv=findServer(name))==NULL) {
				notice(s_OperServ, u->nick, RT_SRV_UNKNOWN, name);
				return MOD_CONT;
			}
			if (!stricmp(hub,"ON")) {
				if (serv->testlink) {
					notice(s_OperServ, u->nick, RT_SET_TL_ON_ALRDY);
					return MOD_CONT;
				}
				alog("%s: %s changed %s TESTLINK status to ON", MYNAME, u->nick, serv->name);
				notice(s_OperServ, u->nick, RT_SET_TL, "now");
				if ((serv->testlink = malloc(sizeof(TLInfo)))==NULL)
					fatal("Out Of Memory!");
				serv->testlink->start = time(NULL);
				serv->testlink->splits = 0;

#ifdef SPLIT_TIME_TRACK
				for (i=0; i < SPLIT_TIME_TRACK; i++)
					serv->testlink->times[i]=0;
#endif

				return MOD_CONT;
			}

			if (!stricmp(hub,"OFF")) {
				if (!serv->testlink) {
					notice(s_OperServ, u->nick, RT_SET_TL_OFF_ALRDY);
					return MOD_CONT;
				}
				alog("%s: %s changed %s TESTLINK status to OFF", MYNAME, u->nick, serv->name);
				myTime = localtime(&serv->testlink->start);
				strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, myTime);
				alog("%s: Testlink for %s started on %s", MYNAME, serv->name, buf);
				alog("%s: %s split %d time(s) during testlink", MYNAME, serv->name, serv->testlink->splits);
#ifdef SPLIT_TIME_TRACK
				if (serv->testlink->splits) {
					alog("%s: Listing split history for %s:", MYNAME, serv->name);
					for (i=0; i<SPLIT_TIME_TRACK; i++)
						if (serv->testlink->times[i] > 0)
							alog("%s: Split %d lasted %s", MYNAME, i+1, myDoTime(serv->testlink->times[i]));
				}
#endif
				temp=time(NULL);
				myTime = localtime(&temp);
				strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, myTime);
				alog("%s: Testlink for %s ended on %s", MYNAME, serv->name, buf);
				notice(s_OperServ, u->nick, RT_SET_TL, "no longer");
				free(serv->testlink);
				serv->testlink=NULL;

				return MOD_CONT;
			}

			notice(s_OperServ, u->nick, RT_SET_TL_SYNTAX);
			return MOD_CONT;
		}

		notice(s_OperServ, u->nick, RT_SET_SYNTAX);
		return MOD_CONT;
	}

	if (!stricmp(cmd,"DEL")) {
		name = strtok(NULL," ");

		if (!is_services_admin(u)) {
			notice(s_OperServ, u->nick, RT_NOACCESS);
			return MOD_CONT;
		}
		if (!name) {
			notice(s_OperServ, u->nick, RT_DEL_SYNTAX);
			return MOD_CONT;
		}

		if ((serv=findServer(name))==NULL) {
			notice(s_OperServ, u->nick, RT_SRV_UNKNOWN, name);
			return MOD_CONT;
		}
		
		notice(s_OperServ, u->nick, RT_SRV_DELETED, serv->name);
		alog("%s: %s deleted server %s", MYNAME, u->nick, serv->name);
		
		delSrvHash(serv);
		
		return MOD_CONT;
	}

	if (!stricmp(cmd,"VIEW")) {
		name = strtok(NULL," ");

		if (!is_services_oper(u)) {
			notice(s_OperServ, u->nick, RT_NOACCESS);
			return MOD_CONT;
		}

		if (!(serv=findServer(name))) {
			notice(s_OperServ, u->nick, RT_SRV_UNKNOWN, name);
			return MOD_CONT;
		}

		notice(s_OperServ, u->nick, RT_SRV_VIEW_HDR);
		notice(s_OperServ, u->nick, RT_SRV_VIEW_NAME, serv->name);
		notice(s_OperServ, u->nick, RT_SRV_VIEW_ADMIN, serv->admin ? serv->admin : "Not On File");
		if (serv->testlink) {
			notice(s_OperServ, u->nick, RT_SRV_VIEW_TL);
			myTime = localtime(&serv->testlink->start);
			strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, myTime);
			notice(s_OperServ, u->nick, RT_SRV_TL_START, buf);
			notice(s_OperServ, u->nick, RT_SRV_TL_SPLIT, serv->testlink->splits);

#ifdef SPLIT_TIME_TRACK
			if (serv->testlink->splits > 0) {
				notice(s_OperServ, u->nick, RT_SRV_TL_LHEAD);
				for (i=0; i<SPLIT_TIME_TRACK; i++)
					if (serv->testlink->times[i] > 0)
						notice(s_OperServ, u->nick, RT_SRV_TL_TIME, i+1, myDoTime(serv->testlink->times[i]));
				notice(s_OperServ, u->nick, RT_LIST_END);
			}
#endif
		}

		notice(s_OperServ, u->nick, RT_SRV_VIEW_LINK, servStatus(serv));
		notice(s_OperServ, u->nick, RT_SRV_VIEW_PORT, serv->port);
		notice(s_OperServ, u->nick, RT_SRV_VIEW_HUB, serv->hub ? serv->hub : "Not On File");
		last = localtime(&serv->last);
        strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, last);
        notice(s_OperServ, u->nick, RT_SRV_VIEW_LAST, buf);
		notice(s_OperServ, u->nick, RT_SRV_VIEW_HOLD, (serv->hold==2) ? "Permanent" : ((serv->hold) ? "Temporary" : "None"));
	
		return MOD_CONT;
	}
	
	if (!stricmp(cmd,"LIST")) {
		name = strtok(NULL," ");
		
		if (!is_services_oper(u)) {
			notice(s_OperServ, u->nick, RT_NOACCESS);
			return MOD_CONT;
		}

		if (!name || !stricmp(name,"ALL")) {  //if they don't specify, we'll just give 'em everything
			listServers(RT_LIST_ALL, u);

			return MOD_CONT;
		}

		if (!stricmp(name,"ACTIVE")) {
			listServers(RT_LIST_ACT, u);

			return MOD_CONT;
		}

		if (!stricmp(name,"SPLIT")) {
			listServers(RT_LIST_SPLIT, u);

			return MOD_CONT;
		}

		if (!stricmp(name,"HUBLESS")) {
			listServers(RT_LIST_HUBLESS, u);

			return MOD_CONT;
		}

		if (!stricmp(name,"TESTLINK")) {
			listServers(RT_LIST_TL, u);

			return MOD_CONT;
		}

		if (!stricmp(name,"LEAFS")) {
			hub = strtok(NULL," ");

			if (!hub) {
				notice(s_OperServ, u->nick, RT_LIST_LEAF_SYNTAX);
				return MOD_CONT;
			}

			if ((serv=findServer(hub))==NULL) {
				notice(s_OperServ, u->nick, RT_SRV_UNKNOWN, hub);
				return MOD_CONT;
			}
					
			listServersParam(RT_LIST_LEAFS, u, serv->name);

			return MOD_CONT;
		}

		if (!stricmp(name,"ADMIN")) {
			hub = strtok(NULL," ");

			if (!hub)
				nc = NULL;
			else if ((nc = findcore(hub))==NULL) {
					notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, hub);
					return MOD_CONT;
				}

			listServersParam(RT_LIST_ADMIN, u, nc);

			return MOD_CONT;
		}

		notice(s_OperServ, u->nick, RT_LIST_SYNTAX);

		return MOD_CONT;
	}

	if (!stricmp(cmd,"LINK")) {
		name = strtok(NULL," ");

		if (!is_services_oper(u)) {
			notice(s_OperServ, u->nick, RT_NOACCESS);
			return MOD_CONT;
		}

		if (!(serv=findServer(name))) {
			notice(s_OperServ, u->nick, RT_SRV_UNKNOWN, name);
			return MOD_CONT;
		}

		if (!serv->hub) {
			notice(s_OperServ, u->nick, RT_LINK_NOHUB, serv->name);
			return MOD_CONT;
		}

		if (serv->hold) {
			serv->hold=0;
			alog("%s: Removing hold status from %s due to LINK command from %s", MYNAME, serv->name, u->nick);
		}

#ifdef MAX_ATTEMPTS
		if (serv->attempts >= MAX_ATTEMPTS)
			serv->attempts = MAX_ATTEMPTS-1;
#endif
		linkServer(serv);

		return MOD_CONT;
	}
	
	if (!stricmp(cmd,"AUTH")) {
		name = strtok(NULL," ");

		if (!is_services_oper(u)) {
			notice(s_OperServ, u->nick, RT_NOACCESS);
			return MOD_CONT;
		}

		if (!name) {
			notice(s_OperServ, u->nick, RT_AUTH_SYNTAX);
			return MOD_CONT;
		}

		if ((serv=findServer(name))==NULL) {
			notice(s_OperServ, u->nick, RT_SRV_UNKNOWN, name);
			return MOD_CONT;
		}

		if (serv->hold) {
			notice(s_OperServ, u->nick, RT_SRV_AHELD, serv->name, (serv->hold==1) ? "Temporary" : "Permanent");
			return MOD_CONT;
		}

		serv->hold=1;
		notice(s_OperServ, u->nick, RT_SRV_HELD, serv->name);
		alog("%s: %s put a temporary HOLD on %s", MYNAME, u->nick, serv->name);

		return MOD_CONT;
	}

	if (!stricmp(cmd,"SYNCH")) {
		if (!is_services_oper(u)) {
			notice(s_OperServ, u->nick, RT_NOACCESS);
			return MOD_CONT;
		}

		notice(s_OperServ, u->nick, RT_SYNCH);
		wallops(s_OperServ, "Routing: %s requested SYNCH of server data", u->nick);
		alog("%s: %s requested a SYNCH of server data", MYNAME, u->nick);

		updateServStatus();

		return MOD_CONT;
	}

	if (is_services_oper(u)) {
		notice(s_OperServ, u->nick, RT_SYNTAX);
	} else {
		notice(s_OperServ, u->nick, RT_NOACCESS);
	}

	return MOD_CONT;
}

//Handles /OS JUPE *
int m_jupe(User *u) {
	char *args = NULL;
    char *jserver = NULL;
    char *reason = NULL;
	SrvInfo *serv;
    char rbuf[256];

	if (moduleGetLastBuffer()) {
		args = sstrdup(moduleGetLastBuffer());
		jserver = myStrGetToken(args,' ',0);
		reason = myStrGetToken(args,' ',1);
	}

    if (!args || !jserver) {
        syntax_error(s_OperServ, u, "JUPE", OPER_JUPE_SYNTAX);
		return MOD_STOP;
	}
	if (!isValidHost(jserver, 3)) {
		notice_lang(s_OperServ, u, OPER_JUPE_HOST_ERROR);
		return MOD_STOP;
	}
	
	snprintf(rbuf, sizeof(rbuf), "Juped by %s%s%s", u->nick, reason ? ": " : "", reason ? reason : "");

	send_cmd(NULL, "SQUIT %s :%s", jserver, rbuf);
#ifdef IRC_PTLINK
	send_cmd(NULL, "SERVER %s 1 Anope.Services%s :%s", jserver, version_number, rbuf);
#else
	send_cmd(NULL, "SERVER %s 2 :%s", jserver, rbuf);
#endif

	if ((serv=findServer(jserver)))
		serv->link=2;
	
	if (WallOSJupe)
		wallops(s_OperServ, "\2%s\2 used JUPE on \2%s\2", u->nick, jserver);

    return MOD_STOP;
}

//Handles /OS STATS ALL
int m_mystats(User *u) {
	int count=0, i=0, mem=0;
	char *args = moduleGetLastBuffer();
	char *cmd = strtok(args," ");
	SrvHash *current=NULL;	
	
	if (!cmd || stricmp(cmd,"ALL"))
		return MOD_CONT;
	
	if (!is_services_admin(u))
		return MOD_CONT;

	for (i=0; i<MAX_CMD_HASH; i++) {
		current=servers[i];
		while(current) {
			if (current->serv->name)
				mem += strlen(current->serv->name) + 1;
			if (current->serv->hub)
				mem += strlen(current->serv->hub) + 1;
			if (current->serv->admin)
				mem += strlen(current->serv->admin) +1;
			if (current->serv->testlink)
				mem += sizeof(TLInfo);
			count++;
			current=current->next;
		}
	}

	mem += count * sizeof(SrvInfo);
	mem += 512;
	mem /= 1024;

	notice(s_OperServ, u->nick, "Routing : \002%d\002 records, \002%d\002 kB", count, mem);
	
	return MOD_CONT;
}

//Saves databases when someone does /os update, or shuts down/restarts services
int m_update(User *u) {
	doExpire();

	save_rt_db();

	return MOD_CONT;
}

//Checks if servers should be synch'ed
int m_identify(User *u) {

#ifdef LINK_DELAY
	if ((FSplit != 0) && ((time(NULL) - FSplit) >= LINK_DELAY)) {
		if (splits.count > 0)
			relinkServersDelay();
	}
#endif

	if ((time(NULL) - LastSynch) >= SYNCH_TIME) {
		doExpire();
		save_rt_db();
		updateServStatus();
	}

	return MOD_CONT;
}
/****************************************
 * Help Functions                       *
 ****************************************/

//Add's help
void AddHelp() {
	Command *c = NULL;

	c = createCommand("ROUTING ON", NULL, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV,c,MOD_TAIL);
	moduleAddHelp(c, RT_Help_ON);

	c = createCommand("ROUTING OFF", NULL, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV,c,MOD_TAIL);
	moduleAddHelp(c, RT_Help_OFF);

	c = createCommand("ROUTING ADD", NULL, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV,c,MOD_TAIL);
	moduleAddHelp(c, RT_Help_ADD);

	c = createCommand("ROUTING SET", NULL, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV,c,MOD_TAIL);
	moduleAddHelp(c, RT_Help_SET);

	c = createCommand("ROUTING SET NAME", NULL, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV,c,MOD_TAIL);
	moduleAddHelp(c, RT_Help_SET_NAME);

	c = createCommand("ROUTING SET PORT", NULL, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV,c,MOD_TAIL);
	moduleAddHelp(c, RT_Help_SET_PORT);

	c = createCommand("ROUTING SET HUB", NULL, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV,c,MOD_TAIL);
	moduleAddHelp(c, RT_Help_SET_HUB);

	c = createCommand("ROUTING SET HOLD", NULL, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV,c,MOD_TAIL);
	moduleAddHelp(c, RT_Help_SET_HOLD);

	c = createCommand("ROUTING SET ADMIN", NULL, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV,c,MOD_TAIL);
	moduleAddHelp(c, RT_Help_SET_ADMIN);

	c = createCommand("ROUTING SET TESTLINK", NULL, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV,c,MOD_TAIL);
	moduleAddHelp(c, RT_Help_SET_TL);

	c = createCommand("ROUTING DEL", NULL, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV,c,MOD_TAIL);
	moduleAddHelp(c, RT_Help_DEL);

	c = createCommand("ROUTING VIEW", NULL, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV,c,MOD_TAIL);
	moduleAddHelp(c, RT_Help_VIEW);

	c = createCommand("ROUTING LIST", NULL, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV,c,MOD_TAIL);
	moduleAddHelp(c, RT_Help_LIST);

	c = createCommand("ROUTING LINK", NULL, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV,c,MOD_TAIL);
	moduleAddHelp(c, RT_Help_LINK);
	
	c = createCommand("ROUTING AUTH", NULL, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV,c,MOD_TAIL);
	moduleAddHelp(c, RT_Help_AUTH);

	c = createCommand("ROUTING SYNCH", NULL, NULL, -1, -1, -1, -1, -1);
	moduleAddCommand(OPERSERV,c,MOD_TAIL);
	moduleAddHelp(c, RT_Help_SYNCH);
	
	return;
}

//Main help function
int RT_Help(User *u) {
	notice(s_OperServ, u->nick, RT_SYNTAX);
	notice(s_OperServ, u->nick, "\002\002");
	notice(s_OperServ, u->nick, "Routing allows your services server to keep");
	notice(s_OperServ, u->nick, "track of servers that are online, and servers that");
	notice(s_OperServ, u->nick, "are split. Routing will attempt to relink split");
	notice(s_OperServ, u->nick, "servers, as long as it is on. Server maintainence");
	notice(s_OperServ, u->nick, "functions are limited to \002Services Administrators\002.");
	notice(s_OperServ, u->nick, "Listing and AUTH are limited to \002 Services Operators\002.");
	notice(s_OperServ, u->nick, "Servers are added to the database as they are introduced to");
	notice(s_OperServ, u->nick, "the network. The default port is used, and the hub is left");
	notice(s_OperServ, u->nick, "null.");
	notice(s_OperServ, u->nick, "\002\002");
	notice(s_OperServ, u->nick, "ON   - Turn ROUTING on");
	notice(s_OperServ, u->nick, "OFF  - Turn ROUTING off");
	notice(s_OperServ, u->nick, " -----------------------");
	notice(s_OperServ, u->nick, "ADD   - Add a server to watch");
	notice(s_OperServ, u->nick, "SET   - Set various server options");
	notice(s_OperServ, u->nick, "DEL   - Delete a server from the database");
	notice(s_OperServ, u->nick, "VIEW  - View information on a server");
	notice(s_OperServ, u->nick, "LIST  - List servers");
	notice(s_OperServ, u->nick, "LINK  - Link a server to its hub");
	notice(s_OperServ, u->nick, "AUTH  - Authorize a server split");
	notice(s_OperServ, u->nick, "SYNCH - Force a resynch of server data");
	notice(s_OperServ, u->nick, "For more help, \002/%s HELP ROUTING \037command\037\002", s_OperServ);

	return MOD_CONT;
}

//Help for ON
int RT_Help_ON(User *u) {
	notice(s_OperServ, u->nick, RT_ON_SYNTAX);
	notice(s_OperServ, u->nick, "\002\002");
	notice(s_OperServ, u->nick, "Sets ROUTING on. When this is set, ROUTING will attempt");
	notice(s_OperServ, u->nick, "to link servers when they split.");
	notice(s_OperServ, u->nick, RT_LIMIT_SA);

	return MOD_CONT;
}

//Help for OFF
int RT_Help_OFF(User *u) {
	notice(s_OperServ, u->nick, RT_OFF_SYNTAX);
	notice(s_OperServ, u->nick, "\002\002");
	notice(s_OperServ, u->nick, "Sets ROUTING off. When this is set, ROUTING will still");
	notice(s_OperServ, u->nick, "keep track of servers, but it will not attempt to relink");
	notice(s_OperServ, u->nick, "servers which split.");
	notice(s_OperServ, u->nick, RT_LIMIT_SA);

	return MOD_CONT;
}

int RT_Help_ADD(User *u) {
	notice(s_OperServ, u->nick, RT_ADD_SYNTAX);
	notice(s_OperServ, u->nick, "\002\002");
	notice(s_OperServ, u->nick, "Forcibly adds a server to the database. The server must be a");
	notice(s_OperServ, u->nick, "valid server name (containing at least one period). Hub must");
	notice(s_OperServ, u->nick, "either be a valid server name \002that is in the database\002 or");
	notice(s_OperServ, u->nick, "the world \002NULL\002. Port must be a valid port between 1 and ");
	notice(s_OperServ, u->nick, "65536. If port is not specified, the default of %d will be used.", DEFPORT);
	notice(s_OperServ, u->nick, RT_LIMIT_SO);

	return MOD_CONT;
}

int RT_Help_SET(User *u) {
	notice(s_OperServ, u->nick, RT_SET_SYNTAX);
	notice(s_OperServ, u->nick, "\002\002");
	notice(s_OperServ, u->nick, "Sets various server parameters. See the list below for a list of");
	notice(s_OperServ, u->nick, "parameters that can be set with this function. Most functions are");
	notice(s_OperServ, u->nick, "limited to \002Services Administrators\002.");
	notice(s_OperServ, u->nick, "NAME     - Set the name of a server");
	notice(s_OperServ, u->nick, "PORT     - Set the port number of a server");
	notice(s_OperServ, u->nick, "HUB      - Set the hub for a server");
	notice(s_OperServ, u->nick, "HOLD     - Set the hold status for a server");
	notice(s_OperServ, u->nick, "ADMIN    - Set the admin for a server");
	notice(s_OperServ, u->nick, "TESTLINK - Set the testlink status for a server");
	notice(s_OperServ, u->nick, "For more help, \002/%s HELP ROUTING SET \037option\037\002", s_OperServ);

	return MOD_CONT;
}

int RT_Help_SET_NAME(User *u) {
	notice(s_OperServ, u->nick, RT_SET_NAME_SYNTAX);
	notice(s_OperServ, u->nick, "\002\002");
	notice(s_OperServ, u->nick, "Sets the name of the given server. It is NOT recommended that this");
	notice(s_OperServ, u->nick, "option be set manually. Servers should create themselves when they");
	notice(s_OperServ, u->nick, "join the network.");
	notice(s_OperServ, u->nick, RT_LIMIT_SRA);

	return MOD_CONT;
}

int RT_Help_SET_PORT(User *u) {
	notice(s_OperServ, u->nick, RT_SET_PORT_SYNTAX);
	notice(s_OperServ, u->nick, "\002\002");
	notice(s_OperServ, u->nick, "Sets the port number for the given server to link on. A valid");
	notice(s_OperServ, u->nick, "port is in the range of 1 - 65536.");
	notice(s_OperServ, u->nick, RT_LIMIT_SA);

	return MOD_CONT;
}

int RT_Help_SET_HUB(User *u) {
	notice(s_OperServ, u->nick, RT_SET_HUB_SYNTAX);
	notice(s_OperServ, u->nick, "\002\002");
	notice(s_OperServ, u->nick, "Sets the hub for the given server. The new hub must be a server");
	notice(s_OperServ, u->nick, "in the database, or the word \002NULL\002.");
	notice(s_OperServ, u->nick, RT_LIMIT_SA);

	return MOD_CONT;
}

int RT_Help_SET_HOLD(User *u) {
	notice(s_OperServ, u->nick, RT_SET_HOLD_SYNTAX);
	notice(s_OperServ, u->nick, "\002\002");
	notice(s_OperServ, u->nick, "Sets the hold status for the given server. Valid holds are:");
	notice(s_OperServ, u->nick, "OFF  - The server will be linked");
	notice(s_OperServ, u->nick, "TEMP - The server will not be linked at it's next split");
	notice(s_OperServ, u->nick, "PERM - The server will not be linked EVER");
	notice(s_OperServ, u->nick, RT_LIMIT_SA);

	return MOD_CONT;
}

int RT_Help_SET_ADMIN(User *u) {
	notice(s_OperServ, u->nick, RT_SET_ADMIN_SYNTAX);
	notice(s_OperServ, u->nick, "\002\002");
	notice(s_OperServ, u->nick, "Sets the admin for the given server. The admin nick must be");
	notice(s_OperServ, u->nick, "registered with %s. If the keyword NULL is used, the server", s_NickServ);
	notice(s_OperServ, u->nick, "admin will be deleted.");
	notice(s_OperServ, u->nick, RT_LIMIT_SA);

	return MOD_CONT;
}

int RT_Help_SET_TL(User *u) {
	notice(s_OperServ, u->nick, RT_SET_TL_SYNTAX);
	notice(s_OperServ, u->nick, "\002\002");
	notice(s_OperServ, u->nick, "Sets the test-link status for the given server.");
	notice(s_OperServ, u->nick, "When a server is testlinked, services will track the");
	notice(s_OperServ, u->nick, "number of times it splits during the testlink period,");
	notice(s_OperServ, u->nick, "the duration of the testlink period, and, if defined");
	notice(s_OperServ, u->nick, "in ROUTING configuration, the most recent split durations.");
	notice(s_OperServ, u->nick, "Valid options are ON and OFF.");
	notice(s_OperServ, u->nick, RT_LIMIT_SA);

	return MOD_CONT;
}

int RT_Help_DEL(User *u) {
	notice(s_OperServ, u->nick, RT_DEL_SYNTAX);
	notice(s_OperServ, u->nick, "\002\002");
	notice(s_OperServ, u->nick, "Deletes a server from the database. This should only be done");
	notice(s_OperServ, u->nick, "when a server will no longer be linking, or if server data");
	notice(s_OperServ, u->nick, "becomes corrupt.");
	notice(s_OperServ, u->nick, RT_LIMIT_SA);

	return MOD_CONT;
}

int RT_Help_VIEW(User *u) {
	notice(s_OperServ, u->nick, RT_VIEW_SYNTAX);
	notice(s_OperServ, u->nick, "\002\002");
	notice(s_OperServ, u->nick, "Displays information about a server in the database.");
	notice(s_OperServ, u->nick, RT_LIMIT_SO);

	return MOD_CONT;
}

int RT_Help_LIST(User *u) {
	notice(s_OperServ, u->nick, RT_LIST_SYNTAX);
	notice(s_OperServ, u->nick, "\002\002");
	notice(s_OperServ, u->nick, "Lists the servers in the database that mach the given criteria");
	notice(s_OperServ, u->nick, "If no criteria is specified, ALL is assumed.");
	notice(s_OperServ, u->nick, "ALL      - List all servers");
	notice(s_OperServ, u->nick, "ACTIVE   - List active servers");
	notice(s_OperServ, u->nick, "SPLT     - List split servers");
	notice(s_OperServ, u->nick, "HUBLESS  - List servers that don't have a set hub");
	notice(s_OperServ, u->nick, "TESTLINK - List testlinked servers");
	notice(s_OperServ, u->nick, "LEAFS    - List leaf servers for a hub");
	notice(s_OperServ, u->nick, "ADMIN    - List servers by admin");
	notice(s_OperServ, u->nick, "Note that with \002LEAFS\002 you MUST specify a hub server after");
	notice(s_OperServ, u->nick, "the LEAFS keyword. For \002ADMIN\002, you can specify an admin nick");
	notice(s_OperServ, u->nick, "that you would like to list servers that are associated with. Without");
	notice(s_OperServ, u->nick, "specifying a nick, OperServ will return a list of all server admins");
	notice(s_OperServ, u->nick, "that it has on file.");
	notice(s_OperServ, u->nick, RT_LIMIT_SO);

	return MOD_CONT;
}

int RT_Help_LINK(User *u) {
	notice(s_OperServ, u->nick, RT_LINK_SYNTAX);
	notice(s_OperServ, u->nick, "\002\002");
	notice(s_OperServ, u->nick, "Links a server to its set hub, on the set port.");
	notice(s_OperServ, u->nick, RT_LIMIT_SO);

	return MOD_CONT;
}

int RT_Help_AUTH(User *u) {
	notice(s_OperServ, u->nick, RT_AUTH_SYNTAX);
	notice(s_OperServ, u->nick, "\002\002");
	notice(s_OperServ, u->nick, "Authorizes a server split. When this command is set, services");
	notice(s_OperServ, u->nick, "will NOT attempt to link the AUTH'd server the next time it ");
	notice(s_OperServ, u->nick, "splits.");
	notice(s_OperServ, u->nick, RT_LIMIT_SO);

	return MOD_CONT;
}

int RT_Help_SYNCH(User *u) {
	notice(s_OperServ, u->nick, RT_SYNCH_SYNTAX);
	notice(s_OperServ, u->nick, "\002\002");
	notice(s_OperServ, u->nick, "Attempts to resynch server data with the closest server. This");
	notice(s_OperServ, u->nick, "is automatically done whenever the module is booted. If a desynch");
	notice(s_OperServ, u->nick, "is evident, or suspected, running this can \002USUALLY\002 correct");
	notice(s_OperServ, u->nick, "the error.");
	notice(s_OperServ, u->nick, RT_LIMIT_SO);

	return MOD_CONT;
}

void RT_OS_Help(User *u) {
	if (is_services_oper(u))
		notice(s_OperServ, u->nick, "    ROUTING    Maintain services routing control");

	return;
}

/****************************************
 * Event Handlers                       *
 ****************************************/

//Handles SQUIT message from network
int handle_squit(char *source, int ac, char **av) {
	SrvInfo *serv;

	if ((!source) || (!av[0])) //Well, something is fucked - better to bail then segfault!
		return MOD_CONT;

	serv=findServer(av[0]);
	
	if (serv && (serv->link != 2))  //Set time, even if we aren't on
		serv->last = time(NULL);
	
	if ((!serv) && ImOn) {  //Uh oh! We can't find this server! COMPLAIN!
		wallops(s_OperServ, "Routing: \002%s\002 just left the network, but is not registered with me. Please fix this!", av[0]);
		alog("%s: %s just left the network, but is not registered with me. Please fix this!", MYNAME, av[0]);
		return MOD_CONT;
	}

	if (!serv)
		return MOD_CONT;
	
	if (serv->link==1)
		serv->link=0;
	if (serv->testlink && (serv->link != 2))
		serv->testlink->splits++;

	if (av[1] && !stricmp(av[1],"Excessive TS delta")) {
		wallops(s_OperServ, "Routing: Placing a hold on \002%s\002 due to TS delta. Fix time on this server!", serv->name);
		alog("%s: Auto-holding \002%s\002 due to TS delta. Fix time on this server!", MYNAME, serv->name);
		serv->hold=2;

		return MOD_CONT;
	}

	if (av[1] && !stricmp(av[1],"Too many servers")) {
		wallops(s_OperServ, "Routing: Placing a hold on \002%s\002 due to non-hub server introduction.", serv->name);
		alog("%s: Auto-holding \002%s\002 due to non-hub server introduce.", MYNAME, serv->name);
		serv->hold=2;

		return MOD_CONT;
	}

	if (serv->leafs)
		checkLeafs(serv);

#ifndef LINK_DELAY
	linkServer(serv);
#else
	if (slist_indexof(&splits,serv) < 0)
		slist_add(&splits,serv);
	if (FSplit == 0)
		FSplit = serv->last;
#endif

	return MOD_CONT;
}

//Handles SERVER message from network
int handle_server(char *source, int ac, char **av) {
	SrvInfo *serv;
#if defined(LINK_DELAY) || defined(SPLIT_TIME_TRACK)
	int i=0;
#endif

	if ((!source) || (!av[0]))  //Well, something is fucked - better to bail then segfault!
		return MOD_CONT;

	serv = findServer(av[0]);

	if (!serv) {  //Uh oh! We can't find this server! COMPLAIN!
		if (ImOn) { //If I'm not, Opers don't wanna hear me complain
			wallops(s_OperServ, "Routing: \002%s\002 joined the network, but is not registered with me. Please fix this!", av[0]);
			alog("%s: %s joined the network, but is not registered with me. Please fix this!", MYNAME, av[0]);
		}

		serv=createServer(av[0], DEFPORT, NULL);
		addSrvHash(serv);
	}

#ifdef SPLIT_TIME_TRACK
	if (serv->testlink && (serv->to != serv)) {
		if (serv->testlink->times[SPLIT_TIME_TRACK-1] > 0)
			alog("%s: Reached maximum split tracking for %s - removing split time of %s", MYNAME, serv->name, 
				myDoTime(serv->testlink->times[SPLIT_TIME_TRACK-1]));
		for (i=SPLIT_TIME_TRACK-2; i>=0; i--)
			serv->testlink->times[i+1] = serv->testlink->times[i];
		serv->testlink->times[0] = time(NULL) - serv->last;
		wallops(s_OperServ, "Test-link server %s rejoined the network - it was split for %s", serv->name, 
			myDoTime(serv->testlink->times[0]));
	}
#endif

	serv->last = time(NULL);
	serv->link = 1;
	if (serv->hold == 1)  //If it had a temporary hold on it, let's release it
		serv->hold=0;
	serv->to = findServer(source);
	if (serv->to)
		serv->to->leafs=1;

#ifdef LINK_DELAY
	if ((i=slist_indexof(&splits,serv)) != -1)
		slist_delete(&splits,i);
	if (splits.count == 0)
		FSplit=0;
#endif
	serv->attempts=0;


	return MOD_CONT;
}

//Handles RPL_LIST - for synching
/* av[0] = destination
   av[1] = clientserver
   av[2] = linked to
   av[3] = Hopcount (including :)
   av[4] = Server info
   */
int handle_364(char *source, int ac, char **av) {
	SrvInfo *serv;
	
	if (!av[1])  //WTF?
		return MOD_STOP;

	if (!stricmp(av[1],ServerName))
		return MOD_STOP;  //It would be pointless to add ourself to the list...

	if ((serv=findServer(av[1]))==NULL) {
		//Server doesn't exist?
		serv=createServerFull(av[1], DEFPORT, NULL, 0, time(NULL), NULL);
		serv->link=1;
		addSrvHash(serv);
	}

	serv->last=time(NULL);
	serv->link=1;
	serv->to = findServer(av[2]);
	if (serv->to)
		serv->to->leafs=1;
	if (serv->to==serv)
		serv->to=NULL;

	return MOD_STOP; //Nothing else should have to deal with this!
}

//Handles RPL_LISTEND - for synching
int handle_365(char *source, int ac, char **av) {
	//Ok, we got all we needed... our temp client can drop now

	ImSynch=0;  //We are done synching, clearly

	send_cmd(NULL, ":RoutingServices QUIT :No Longer Needed");

	relinkServers();

	return MOD_STOP;  //Nothing else should need to deal with this!
}

//Handles NOSUCHSERVER - for synching
int handle_402(char *source, int ac, char **av) {
	if (!ImSynch)
		return MOD_CONT;

	if (!av[1] || !av[0])
		return MOD_CONT;

	if (stricmp(av[1],RT_LINKSERV))
		return MOD_CONT;

	ImSynch=0;  //Error, so we are done synching

	wallops(s_OperServ, "Routing: An error occured while synching - synch failed!");
	alog("%s: An error occured while SYNCHing - %s not found", MYNAME, RT_LINKSERV);

	send_cmd(NULL, ":RoutingServices QUIT :No Longer Needed");

	return MOD_STOP;  //Nothing else should need to deal with this!
}

/****************************************
 * Misc Functions - DO NOT EDIT!        *
 ****************************************/

//Is this a server?
//To be a server, basically, it needs at least 1 '.' in the name...
int isServer(char *name) {
	return (int)((strchr(name, '.')) || (!stricmp(name,"NULL")));
}

//Update server status, just in case we go out of synch :-)
int updateServStatus(void) {
	alog("%s: Attempting to re-synch online server data", MYNAME);

	ImSynch=1;  //Indicate that we are synching
	LastSynch = time(NULL);
	
	NEWNICK("RoutingServices","services",ServiceHost,"Routing Bot","+io",0);
	send_cmd(NULL, ":RoutingServices LINKS %s *", RT_LINKSERV);

	return MOD_CONT;
}

//Relink a split server
void linkServer(SrvInfo *serv) {
	SrvInfo *serv2 = NULL;
#ifdef LINK_DELAY
	int i;
#endif

#ifdef MAX_ATTEMPTS
	if (serv->attempts >= MAX_ATTEMPTS) {
		serv->hold=1;
		wallops(s_OperServ, "Routing: Reached MAX_ATTEMPTS for linking \002%s\002 - placing hold on server", serv->name);
		alog("%s: Auto-holding %s due to MAX_ATTEMPTS", MYNAME, serv->name);

#ifdef LINK_DELAY
		if ((i=slist_indexof(&splits,serv)) != -1)
			slist_delete(&splits,i);
		if (splits.count == 0)
			FSplit=0;
#endif

		return;
	}
#endif

	if (!ImOn)  //If im not on, don't do anything about it
		return;

	if (serv->hold) { //Hold status; Don't link it, split was okay'ed
		wallops(s_OperServ, "Routing: %s split, but has a hold status of %d - not relinking", serv->name, serv->hold);
		alog("%s: %s split, but has a hold status of %d - not relinking", MYNAME, serv->name, serv->hold);
		return;
	}

	if (!serv->hub) { //Hrm, that's strange. No hub
		wallops(s_OperServ, "Routing: \002%s\002 just left the network, but I don't know what it's hub is. Please fix this!", serv->name);
		alog("%s: %s just left the network, but I don't what it's hub is. Please fix this!", MYNAME, serv->name);
		return;
	}

	if ((serv2=findServer(serv->hub))==NULL) {
		//I can't find the hub in my database.. COMPLAIN!
		wallops(s_OperServ, "Routing: %s's hub is set as %s, but that doesn't seem to exist!", serv->name, serv->hub);
		//Well, it's not in our database, but no harm in trying...
		serv->attempts++;
		send_cmd(s_OperServ, "CONNECT %s %d %s", serv->name, serv->port, serv->hub);
		alog("%s: %s split. Attemping to re-link it.", MYNAME, serv->name);

		return;
	}

	if (!serv2->link) {
		linkServer(serv2);
	}

	//Everything looks ok. Let's try to connect it
	serv->attempts++;
	send_cmd(s_OperServ, "CONNECT %s %d %s", serv->name, serv->port, serv->hub);
	alog("%s: %s split. Attemping to re-link it.", MYNAME, serv->name);

	return;
}


//Relink any split servers without hold
void relinkServers(void) {
	int i;
	SrvHash *current;

	for (i=0; i<MAX_CMD_HASH; i++) {
		current = servers[i];
		while(current) {
			if (!current->serv->link && !current->serv->hold) {
				linkServer(current->serv);
			}
			current = current->next;
		}
	}

	return;
}

//Checks for test-linked leaf splits DUE TO HUB
void checkLeafs(SrvInfo *serv) {
	SrvHash *cur;
	int i;

	for (i=0; i<MAX_CMD_HASH; i++) {
		cur=servers[i];
		while (cur) {
			if (cur->serv->to && (cur->serv->to == serv)
				&& cur->serv->testlink) {
				cur->serv->testlink->splits--;
				cur->serv->to=cur->serv;
			}
			cur=cur->next;
		}
	}

	return;
}

//Takes a time difference and tells us what it is in ENGLISH
char *myDoTime(int32 rhs) {
	int years=0, days=0, mins=0, secs=0, i=0;
	char buf[256];

	if (rhs > 31536000) {
		years = rhs/31536000;
		rhs = rhs%31536000;
	}
	if (rhs > 86400) {
		days = rhs/86400;
		rhs = rhs%86400;
	}
	mins = rhs/60;
	secs = rhs%60;

	if (years) {
		sprintf(&buf[i],"%d years ",years);
		i=strlen(buf);
	}
	if (days) {
		sprintf(&buf[i],"%d days ",days);
		i=strlen(buf);
	}
	if (mins) {
		sprintf(&buf[i],"%d minutes ",mins);
		i=strlen(buf);
	}
	if (secs) {
		sprintf(&buf[i],"%d seconds ",secs);
		i=strlen(buf);
	}
	return sstrdup(&buf[0]);
}

//Returns the status of a server
char *servStatus(SrvInfo *serv) {
	if (!serv)
		return "Unknown";

	if (serv->link == 1)
		return RT_ACTIVE;

	if (serv->link == 0)
		return RT_SPLIT;

	if (serv->link == 2)
		return RT_JUPED;

	return "Unknown";
}

//Returns the number of servers with SPLIT status and NO HOLD
int countSplits(void) {
	int i,c=0;
	SrvHash *current;

	for (i=0; i<MAX_CMD_HASH; i++) {
		current = servers[i];
		while(current) {
			if (!current->serv->link && !current->serv->hold)
				c++;
			current = current->next;
		}
	}

	return c;
}

//List servers with given criteria
void listServers(int what, User *u) {
	int i;
	int c=0, d=0;
	SrvHash *current;

	notice(s_OperServ, u->nick, RT_LIST_HEAD);

	for (i=0; i<MAX_CMD_HASH; i++) {
		current = servers[i];
		while(current) {
			c++;
			switch(what) {
				case RT_LIST_ALL:
					d++;
					if (current->serv->testlink) {
						notice(s_OperServ, u->nick, RT_LIST_LINE_TL, c, current->serv->name, servStatus(current->serv), 
							(current->serv->hold) ? ((current->serv->hold==2) ? "PERMANENT" : "TEMPORARY") : "NONE");
					} else {
						notice(s_OperServ, u->nick, RT_LIST_LINE, c, current->serv->name, servStatus(current->serv), 
							(current->serv->hold) ? ((current->serv->hold==2) ? "PERMANENT" : "TEMPORARY") : "NONE");
					}
					break;
				case RT_LIST_ACT:
					if (current->serv->link) {
						d++;
						if (current->serv->testlink) {
							notice(s_OperServ, u->nick, RT_LIST_LINE_TL, c, current->serv->name, servStatus(current->serv), 
								(current->serv->hold) ? ((current->serv->hold==2) ? "PERMANENT" : "TEMPORARY") : "NONE");
						} else {
							notice(s_OperServ, u->nick, RT_LIST_LINE, c, current->serv->name, servStatus(current->serv), 
								(current->serv->hold) ? ((current->serv->hold==2) ? "PERMANENT" : "TEMPORARY") : "NONE");
						}
					}
					break;
				case RT_LIST_SPLIT:
					if (!current->serv->link) {
						d++;
						if (current->serv->testlink) {
							notice(s_OperServ, u->nick, RT_LIST_LINE_TL, c, current->serv->name, servStatus(current->serv), 
								(current->serv->hold) ? ((current->serv->hold==2) ? "PERMANENT" : "TEMPORARY") : "NONE");
						} else {
							notice(s_OperServ, u->nick, RT_LIST_LINE, c, current->serv->name, servStatus(current->serv), 
								(current->serv->hold) ? ((current->serv->hold==2) ? "PERMANENT" : "TEMPORARY") : "NONE");
						}
					}
					break;

				case RT_LIST_HUBLESS:
					if (!current->serv->hub) {
						d++;
						if (current->serv->testlink) {
							notice(s_OperServ, u->nick, RT_LIST_LINE_TL, c, current->serv->name, servStatus(current->serv), 
								(current->serv->hold) ? ((current->serv->hold==2) ? "PERMANENT" : "TEMPORARY") : "NONE");
						} else {
							notice(s_OperServ, u->nick, RT_LIST_LINE, c, current->serv->name, servStatus(current->serv), 
								(current->serv->hold) ? ((current->serv->hold==2) ? "PERMANENT" : "TEMPORARY") : "NONE");
						}
					}
					break;
				case RT_LIST_TL:
					if (current->serv->testlink) {
						d++;
						notice(s_OperServ, u->nick, RT_LIST_LINE_TL, c, current->serv->name, servStatus(current->serv), 
							(current->serv->hold) ? ((current->serv->hold==2) ? "PERMANENT" : "TEMPORARY") : "NONE");
					}
					break;
			}
			current = current->next;
		}
	}

	if (!d) {
		notice(s_OperServ, u->nick, RT_LIST_NONE);
	} else {
		notice(s_OperServ, u->nick, RT_LIST_END);
	}

	return;
}

//List servers with given criteria
void listServersParam(int what, User *u, void *param) {
	int i;
	int c=0, d=0;
	SrvHash *current;

	if (!param && (what==RT_LIST_ADMIN)) {
		listAdmins(u);
		return;
	}

	if (!param)
		return;

	notice(s_OperServ, u->nick, RT_LIST_HEAD);

	for (i=0; i<MAX_CMD_HASH; i++) {
		current = servers[i];
		while(current) {
			c++;
			switch(what) {
				case RT_LIST_LEAFS:
					if (current->serv->hub && !stricmp(current->serv->hub, (char *)param)) {
						d++;
						if (current->serv->testlink) {
							notice(s_OperServ, u->nick, RT_LIST_LINE_TL, c, current->serv->name, servStatus(current->serv), 
								(current->serv->hold) ? ((current->serv->hold==2) ? "PERMANENT" : "TEMPORARY") : "NONE");
						} else {
							notice(s_OperServ, u->nick, RT_LIST_LINE, c, current->serv->name, servStatus(current->serv), 
								(current->serv->hold) ? ((current->serv->hold==2) ? "PERMANENT" : "TEMPORARY") : "NONE");
						}
					}
					break;
				case RT_LIST_ADMIN:
					if (current->serv->admin && !stricmp(current->serv->admin,((NickCore *)param)->display)) {
						d++;
						if (current->serv->testlink) {
							notice(s_OperServ, u->nick, RT_LIST_LINE_TL, c, current->serv->name, servStatus(current->serv), 
								(current->serv->hold) ? ((current->serv->hold==2) ? "PERMANENT" : "TEMPORARY") : "NONE");
						} else {
							notice(s_OperServ, u->nick, RT_LIST_LINE, c, current->serv->name, servStatus(current->serv), 
								(current->serv->hold) ? ((current->serv->hold==2) ? "PERMANENT" : "TEMPORARY") : "NONE");
						}
					}
					break;
			}
			current = current->next;
		}
	}

	if (!d) {
		notice(s_OperServ, u->nick, RT_LIST_NONE);
	} else {
		notice(s_OperServ, u->nick, RT_LIST_END);
	}

	return;
}

//List all server admin nicks
void listAdmins(User *u) {
	SList admins;
	SrvHash *cur=NULL;
	NickCore *nc;
	int i=0, d=0;
	SListOpts SOpts = { SLISTF_SORT, &CompareEntries, NULL, NULL };

	slist_init(&admins);
	admins.opts = &SOpts;

	for (i=0; i < MAX_CMD_HASH; i++) {
		cur=servers[i];
		while (cur) {
			if (cur->serv->admin && ((nc=findcore(cur->serv->admin)))) {
				if (slist_indexof(&admins,nc) < 0)
					slist_add(&admins,nc);
			} else if (!cur->serv->admin)
				d++;
			cur=cur->next;
		}
	}

	notice(s_OperServ, u->nick, RT_LIST_ADM_HEAD);

	for (i=0; i<admins.count; i++) {
		nc = (NickCore *)admins.list[i];
		if (nc && nc->display)
			notice(s_OperServ, u->nick, RT_LIST_ADM_LINE, nc->display);
	}

	if (d > 0)
		notice(s_OperServ, u->nick, RT_LIST_ADM_NOAD, d);

	if (admins.count < 1)
		notice(s_OperServ, u->nick, RT_LIST_ADM_NONE);
	else 
		notice(s_OperServ, u->nick, RT_LIST_END);

	slist_clear(&admins,0);

	return;
}

//Used to sort the SList
static int CompareEntries(SList *list, void *entry1, void *entry2) {
    NickCore *nc1 = entry1, *nc2 = entry2;

    if (!nc1 || !nc2)
        return -1;
    return stricmp(nc1->display, nc2->display);
}

//Checks for expired nicks
void doExpire(void) {
	SrvHash *cur;
	int i;

	for (i=0; i<MAX_CMD_HASH; i++) {
		cur=servers[i];
		while(cur) {
			if (cur->serv->admin) {
				if (!findcore(cur->serv->admin)) {
					free(cur->serv->admin);
					cur->serv->admin = NULL;
					alog("%s: Admin nick for %s has expired. Removing admin.", MYNAME, cur->serv->name);
				}
			}
			cur = cur->next;
		}
	}

	return;
}

#ifdef LINK_DELAY

//Used to sort the SList
static int CompareSrvEntries(SList *list, void *entry1, void *entry2) {
    SrvInfo *srv1 = entry1, *srv2 = entry2;

    if (!srv1 || !srv2)
        return -1;
    return stricmp(srv1->name, srv2->name);
}

void relinkServersDelay(void) {
	SrvInfo *serv;
	int i;
	
	FSplit = time(NULL);

	for (i=0; i<splits.count; i++) {
		serv=(SrvInfo *)splits.list[i];
		if (serv && ((time(NULL) - serv->last) >= LINK_DELAY))
			linkServer(serv);
	}

	return;
}

#endif

/****************************************
 * Hash Utilities - DO NOT EDIT!        *
 ****************************************/

//Create a new server record
SrvInfo *createServer(char *name, int32 port, char *hub) {
	SrvInfo *serv;

	if (!name)
		return 0;

	if ((serv = malloc(sizeof(SrvInfo))) == NULL)
		fatal("Out Of Memory!");

	serv->name = sstrdup(name);
	serv->port = port;
	serv->link = 0;
	if (!hub) {
		serv->hub = NULL;
	} else {
		serv->hub = sstrdup(hub);
	}
	serv->hold = 0;
	serv->last = 0;
	serv->admin = NULL;
	serv->testlink = NULL;
	serv->attempts = 0;
	serv->to = NULL;
	serv->leafs=0;

	return serv;
}

//Creates records from info in from DB
SrvInfo *createServerFull(char *name, int32 port, char *hub, int hold, int32 last, char *ad) {
	SrvInfo *serv;
	NickCore *nc;

	if (!name)
		return 0;

	if ((serv = malloc(sizeof(SrvInfo))) == NULL)
		fatal("Out Of Memory!");

	serv->name = sstrdup(name);
	serv->port = port;
	serv->link = 0;
	if (!hub) {
		serv->hub = NULL;
	} else {
		serv->hub = sstrdup(hub);
	}
	serv->hold = hold;
	serv->last = last;
	serv->testlink = NULL;
	if (ad && ((nc=findcore(ad))))
		serv->admin = sstrdup(nc->display);
	else
		serv->admin = NULL;
	serv->attempts = 0;
	serv->to = NULL;
	serv->leafs=0;

	return serv;
}

//Find an existing server
SrvInfo *findServer(char *name) {
	int index;

	SrvHash *current = NULL;

	if (!name)
		return NULL;

	if ((!isServer(name)) && ((index=atoi(name))>0))
		return findServerByID(index);

	if (!isServer(name))
		return NULL;

	index = CMD_HASH((name));

	for (current = servers[index]; current; current = current->next) {
		if (match_wild_nocase(name,current->serv->name)) {
			return current->serv;
		}
	}
	return NULL;
}

//Find server by number
SrvInfo *findServerByID(int id) {
	int cur = 0;
	int i;
	SrvHash *current = NULL;

	for (i=0; i<MAX_CMD_HASH; i++) {
		current = servers[i];
		while (current) {
			cur++;
			if (cur==id)
				return current->serv;
			current = current->next;
		}
	}

	return NULL;
}

//Add a SrvInfo to the hash
int addSrvHash(SrvInfo *serv) {
	int index;
	SrvHash *current = NULL;
	SrvHash *previous = NULL;
	SrvHash *nexthash = NULL;

	index = CMD_HASH((serv->name));

	for (current = servers[index]; current; current = current->next) {
		if (!stricmp(serv->name, current->serv->name))
			return -1;
		previous = current;
	}

	if ((nexthash = malloc(sizeof(SrvHash))) == NULL) {
		fatal("Out Of Memory!");
	}

	nexthash->next = NULL;
	nexthash->serv = serv;

	if (previous == NULL)
		servers[index] = nexthash;
	else
		previous->next = nexthash;

	return 0;
}

//Del a SrvInfo from the hash
int delSrvHash(SrvInfo *serv) {
	int index;
	SrvHash *current = NULL;
	SrvHash *previous = NULL;

	if (!serv) {
		return -1;
	}

	index = CMD_HASH((serv->name));

	for (current = servers[index]; current; current = current->next) {
		if (!stricmp(serv->name, current->serv->name)) {
			if (!previous) {
				servers[index] = current->next;
			} else {
				previous->next = current->next;
			}
			if (current->serv->name)
				free(current->serv->name);
			if (current->serv->hub)
				free(current->serv->hub);
			if (current->serv->admin)
				free(current->serv->admin);
			if (current->serv->testlink)
				free(current->serv->testlink);
			free(current);
			return 0;
		}
		previous = current;
	}
	return -1;
}

//Adds test link info from db
#ifdef SPLIT_TIME_TRACK
void addTLInfo(SrvInfo *serv, int32 start, int16 splits, int32 times[]) {
	int c;

	if ((serv->testlink = malloc(sizeof(TLInfo)))==NULL)
		fatal ("Out Of Memory!");

	serv->testlink->start = start;
	serv->testlink->splits = splits;

	for (c=0; c<SPLIT_TIME_TRACK; c++)
		serv->testlink->times[c] = times[c];

	return;
}
#else
void addTLInfo(SrvInfo *serv, int32 start, int16 splits) {
	if ((serv->testlink = malloc(sizeof(TLInfo)))==NULL)
		fatal ("Out Of Memory!");

	serv->testlink->start = start;
	serv->testlink->splits = splits;

	return;
}
#endif

/****************************************
 * Database Functions - DO NOT EDIT!    *
 ****************************************/

#define SAFE(x) do {  \
	if ((x) < 0) {  \
		if (!forceload)  \
			fatal("Read error on %s", RT_DATABASE);  \
		failed = 1;  \
		break;  \
	}  \
} while (0)

void load_rt_db(void) {
	dbFILE *f;
	int ver;

	if (!(f = open_db(s_OperServ, RT_DATABASE, "r", RT_DB_VER))) {
		return;
	}

	ver = get_file_version(f);

	if (ver == 1) {
		load_rt_db_v1(f);
	} else if (ver == 2) {
		load_rt_db_v2(f);
	}

	close_db(f);
}

void load_rt_db_v1(dbFILE *f) {
	int c;
	int failed;
	int hold;
	char *name;
	int port;
	int32 last;
	char *hub;
	SrvInfo *serv;

	alog("%s: Loading database", MYNAME);
	failed = 0;

	while(!failed && ((c = getc_db(f)) == 1)) {
		if (c==1) {
			SAFE(read_string(&name,f));
			SAFE(read_int32(&port,f));
			SAFE(read_string(&hub,f));
			if (!stricmp(hub,"NULL")) {
				free(hub);
				hub = NULL;
			}
			SAFE(read_int8(&hold,f));
			SAFE(read_int32(&last,f));
			serv = createServerFull(name,port,hub,hold,last,NULL);
			addSrvHash(serv);
			free(name);
			if (hub != NULL)
				free(hub);
		} else {
			fatal("Invalid format in %s %d", RT_DATABASE, c);
		}
	}
}

void load_rt_db_v2(dbFILE *f) {
	int c;
	int failed;
	int hold;
	char *name;
	int port;
	int tl;
	int16 splits;
	int32 tm1, tm2;
#ifdef SPLIT_TIME_TRACK
	int j;
	int i=0;
	int32 times[SPLIT_TIME_TRACK];
#endif
	char *admin;
	int32 last;
	char *hub;
	SrvInfo *serv;

	alog("%s: Loading database", MYNAME);
	failed = 0;

	while(!failed && ((c = getc_db(f)) == 1)) {
		if (c==1) {
			SAFE(read_string(&name,f));
			SAFE(read_int32(&port,f));
			SAFE(read_string(&hub,f));
			if (!stricmp(hub,"NULL")) {
				free(hub);
				hub = NULL;
			}
			SAFE(read_int8(&hold,f));
			SAFE(read_int32(&last,f));
			SAFE(read_int8(&tl,f));
			if (tl==1) {
				SAFE(read_int32(&tm2,f));
				SAFE(read_int16(&splits,f));
#ifdef SPLIT_TIME_TRACK
				for (i=0; i<SPLIT_TIME_TRACK; i++)
					times[i]=0;
				j=0;
#endif
				SAFE(read_int32(&tm1,f));
				while (tm1 != -1) {
#ifdef SPLIT_TIME_TRACK
					if (j < SPLIT_TIME_TRACK) {
						times[j]=tm1;
						j++;
					}
#endif
					SAFE(read_int32(&tm1,f));
				}
			}
			SAFE(read_string(&admin,f));
			if (!stricmp(admin,"NULL")) {
				free(admin);
				admin = NULL;
			}
			serv = createServerFull(name,port,hub,hold,last,admin);
			addSrvHash(serv);
			if (tl==1)
#ifdef SPLIT_TIME_TRACK
				addTLInfo(serv,tm2,splits,times);
#else
				addTLInfo(serv,tm2,splits);
#endif
			free(name);
			if (hub != NULL)
				free(hub);
			if (admin != NULL)
				free(admin);
		} else {
			fatal("Invalid format in %s %d", RT_DATABASE, c);
		}
	}
}

#undef SAFE

#define SAFE(x) do {  \
	if ((x) < 0) {  \
		restore_db(f);  \
		log_perror("Write error on %s", RT_DATABASE);  \
		if (time(NULL) - lastwarn > WarningTimeout) {  \
			wallops(NULL, "Write error on %s: %s", RT_DATABASE, strerror(errno));  \
			lastwarn = time(NULL);  \
		}  \
		return;  \
	}  \
} while (0)

void save_rt_db(void) {

#ifndef NO_SAVE
	dbFILE *f;
	static time_t lastwarn=0;
	int i,r;
	SrvHash *current;

	if (!(f = open_db(s_ChanServ, RT_DATABASE, "w", RT_DB_VER))) {
		return;
	}

	for (i=0; i<MAX_CMD_HASH; i++) {
		current = servers[i];
		while(current) {
			SAFE(write_int8(1,f));
			SAFE(write_string(current->serv->name,f));
			SAFE(write_int32(current->serv->port,f));
			if (!current->serv->hub) {
				SAFE(write_string("NULL",f));
			} else {
				SAFE(write_string(current->serv->hub,f));
			}
			if (current->serv->hold > 1) {
				SAFE(write_int8(current->serv->hold,f));
			} else {
				SAFE(write_int8(0,f));
			}
			SAFE(write_int32(current->serv->last,f));
			if (current->serv->testlink) {
				SAFE(write_int8(1,f));
				SAFE(write_int32(current->serv->testlink->start,f));
				SAFE(write_int16(current->serv->testlink->splits,f));
#ifdef SPLIT_TIME_TRACK
				r=0;
				while ((r < SPLIT_TIME_TRACK) && (current->serv->testlink->times[r] > 0)) {
					SAFE(write_int32(current->serv->testlink->times[r],f));
					r++;
				}
#endif
				SAFE(write_int32(-1,f));
			} else {
				SAFE(write_int8(0,f));
			}
			if (current->serv->admin)
				SAFE(write_string(current->serv->admin,f));
			else
				SAFE(write_string("NULL",f));
			current=current->next;
		}
	}
	
	SAFE(write_int8(0,f));
	close_db(f);

#endif
}

#undef SAFE
