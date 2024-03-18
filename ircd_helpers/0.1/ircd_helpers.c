/* 
*  Module's base taken from ns_ajoin by Viper 
*
*  This module is a Helpers system and have a fantasy command that only uses in help channel. 
*  Have two option that's LOGIN system and ID(identify) 
*
*  If you select "ALL", when you select this, both using identify and login. Or
*  If you select "ID" or "LOGIN", you will use only one system.
*  ID system check user's level on help channel when he uses identify command.
*
*  You are add or remove new login to use username and password.
*  If Any Helper login the system, get some mode, vHost etc.
*
*  Two parameters is using in this module. First HelpOp(1) Second HelpSop(1). Numbers is flags and uses when adds helpers.
*  When you are add new login, module add level on helpchannel access list for username. Default levels are HelpOp = 5 HelpSop =10.
*  
* 
* Greet is fantasy command and only use helpchannel.
*
* Greet given voice and sent greet msg to user.  !greet user. 
* 
* Pls, Edit default options.
*/ 


/* Some code and database taken ns_ajoin by Viper */

/**
 * Configuration directives that should be copy-pasted to services.conf

# HELPERS [OPTIONAL]
# Module: ircd_helpers
#
# You are use three parametres. ALL, LOGIN, ID
# If you select ALL, when uses, both identify and login .
#
# LOGIN is default option.
HELPERS "ALL"

**/
/* --------- Default Options --------- */

/* helpers ajoin channel */
#define HChannel "#helpers"
/* helpers default options */
#define HOPHOST "Helper.Localhost"
#define HOPMODE "+hW"
/* helperSop default options */
#define HSOPHOST "HelpDesk.Localhost"
#define HSOPMODE "+hWfq"
/* Default Messages */

#define GREETMSG "Hi, Welcome the offical help channel. Can I Help You?"

int greet = /* 1 is ON 0 is OFF */ 1;

/* This lvl will add on helpchannel's access list  */
int hoplvl = /* HelperOperator  */ 5;
int hsoplvl = /* HelperSuperOperator  */ 10;

/* End Of Options */
/* ---------------------------------------------------------------------------------------------- */
#include "module.h"

#define AUTHOR "ysfm"
#define VERSION "$Id: ircd_helpers  V0.1 17-08-2009 ysfm $"



/* Languages */
#define LANG_NUM_STRINGS 					7

#define LANG_HELPERS_DESC						0
#define LANG_HELPERS_SYNTAX					1
#define LANG_HELPERS_SYNTAX_EXT				2
#define LANG_HELPERS_ADD					3
#define LANG_HELPERS_DEL					4
#define LANG_HELPERS_LOGIN_SUCCESS					5
#define LANG_HELPERS_DISABLED					6

/* Flags to set in database */
#define HELPERS_ON        (1 << 0)
#define HELPERS_SILENT    (1 << 1)
#define HELPERSDBVERSION 1
/* Database seperators */
#define SEPARATOR  ':'          /* End of a flags, seperates flagss from values */
#define BLOCKEND   '\n'         /* End of a block, e.g. a whole nick/nick or a subblock */
#define VALUEEND   '\000'       /* End of a value */
#define SUBSTART   '\010'       /* Beginning of a new subblock, closed by a BLOCKEND */

/* Database reading return values */
#define DB_READ_SUCCESS   0
#define DB_READ_ERROR     1
#define DB_EOF_ERROR      2
#define DB_VERSION_ERROR  3
#define DB_READ_BLOCKEND  4
#define DB_READ_SUBSTART  5

#define DB_WRITE_SUCCESS  0
#define DB_WRITE_ERROR    1
#define DB_WRITE_NOVAL    2

/* Database Key, Value max length */
#define MAXKEYLEN 128
#define MAXVALLEN 1024
int NSHELPERSMax = 2;

/* Structs */
typedef struct db_file_ DBFile;
typedef struct HELPERSpassw_ HELPERSPassw;
typedef struct HELPERSentry_ HELPERSEntry;

struct db_file_ {
	FILE *fptr;             /* Pointer to the opened file */
	int db_version;         /* The db version of the datafiles (only needed for reading) */
	int core_db_version;    /* The current db version of this anope source */
	char service[256];      /* StatServ/etc. */
	char filename[256];     /* Filename of the database */
	char temp_name[262];    /* Temp filename of the database */
};

struct HELPERSpassw_ {
	HELPERSPassw *prev, *next;
	char *nick;
	char *flags;
};

struct HELPERSentry_ {
	/* Specifies storage location in HELPERSTable */
	int row, col;
	HELPERSEntry *prev, *next;
	int ajnicks;
	HELPERSPassw *nicks;
	/* Flags store HELPERS settings.
	 * Bits are used as followed:
	 *   User wants to be autojoined            {bit 0}
	 *   User wants autojoin to be silent       {bit 1}
	 */
	uint16 flags;
	/* Used to check during the cleanup whether the entry is still being used.
	 * This is always 0 except when the DB saving function has completed going
	 * over all NickCore's. The HELPERS entries with in_use set to 0 at that time
	 * are no longer used and cleared. In the other entries in_use is reset to 0. */
	int in_use;
};


char *DefHELPERSDB = "ircd_helpers.db";
char *ModDataKey = "HELPERS";
uint16 DefHELPERSFlags = 1;

char *DefHELPERS = "LOGIN";
char *HELPERSDB;
char *HELPERS;
int counter;

HELPERSEntry *HELPERSTable[1024];

int do_help(User *u);
int do_kisayol(int argc, char **argv);
int do_yardimci(User *u);
int do_identify(User *u);
int null_func(User *u);


void clean_nickname(char *cn);
int HELPERS_del_callback(User *u, int num, va_list args);

int new_open_db_read(DBFile *dbptr, char **flags, char **value);
int new_open_db_write(DBFile *dbptr);
void new_close_db(FILE *fptr, char **flags, char **value);
int new_read_db_entry(char **flags, char **value, FILE * fptr);
int new_write_db_entry(const char *flags, DBFile *dbptr, const char *fmt, ...);
int new_write_db_endofblock(DBFile *dbptr);
void fill_db_ptr(DBFile *dbptr, int version, int core_version, char service[256], char filename[256]);

static HELPERSPassw *addHELPERSPassw(HELPERSEntry *ae, char *passw, char *flags);
HELPERSPassw *findHELPERSPassw(HELPERSEntry *ae, char *passw);
static int clearHELPERSPassws(HELPERSEntry *ae);

static HELPERSEntry *createHELPERSEntry(NickCore *nc);
HELPERSEntry *getHELPERSEntry(NickCore *nc);
static int deleteHELPERSEntry(HELPERSEntry *ae);
void freeUnusedEntries();

int do_save(int argc, char **argv);
int db_backup(int argc, char **argv);
void load_HELPERS_db(void);
void save_HELPERS_db(void);
HELPERSEntry *go_to_next_entry(uint16 skipped, HELPERSEntry *next, HELPERSEntry *ae);

int valid_ircd(void);

void load_config(void);
int reload_config(int argc, char **argv);
void add_languages(void);


/* ------------------------------------------------------------------------------- */

/**
 * Create the command, and tell anope about it.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv) {
	Command *c;
	EvtHook *hook;
	ChannelInfo *cx = cs_findchan(HelpChannel);
	alog("[\002ircd_helpers\002] Loading module...");

	counter = 0;

	if (!valid_ircd()) {
		alog("[\002ircd_helpers\002] ERROR: IRCd not supported by this module");
		alog("[\002ircd_helpers\002] Unloading module...");
		return MOD_STOP;
	}

	if (!moduleMinVersion(1,7,20,1324)) {
		alog("[\002ircd_helpers\002] Your version of Anope isn't supported. Please update to a newer release.");
		return MOD_STOP;
	}
	if ((cx->flags & CI_XOP)) {
		alog("[\002ircd_helpers\002] HelpChannel use XOP system. Plesae, turn of this option");
		return MOD_STOP;
	}
	if (!HelpChannel) {
		alog("[\002ircd_helpers\002] HelpChannel not defined. Pls Checks your conf file(services.conf)");
		return MOD_STOP;
	}
	/* Create FANTASY commands */
        hook = createEventHook(EVENT_BOT_FANTASY, do_kisayol);
	moduleAddEventHook(hook);
	/* Create HELPERS command */
	c = createCommand("HELPERS", do_yardimci, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(HELPSERV,c,MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002ircd_helpers\002] Cannot create HELPERS command...");
		return MOD_STOP;
	}


	c = createCommand("IDENTIFY", do_identify, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV,c,MOD_TAIL) != MOD_ERR_OK) {
		alog("[\002ircd_helpers\002] Cannot hook to IDENTIFY command...");
		return MOD_STOP;
	}

	moduleAddHelp(c,do_help);



	
	/* Hook to some events.. */
	hook = createEventHook(EVENT_RELOAD, reload_config);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002ircd_helpers\002] Can't hook to EVENT_RELOAD event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_DB_SAVING, do_save);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002ircd_helpers\002] Can't hook to EVENT_DB_SAVING event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_DB_BACKUP, db_backup);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002ircd_helpers\002] Can't hook to EVENT_DB_BACKUP event");
		return MOD_STOP;
	}

	load_config();
	add_languages();
	load_HELPERS_db();

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	alog("[\002ircd_helpers\002] Module loaded successfully...");

	return MOD_CONT;
}


/**
 * Unload the module
 **/
void AnopeFini(void) {
	HELPERSEntry *ae = NULL, *next = NULL;
	int i;

	if (HELPERSDB)
		save_HELPERS_db();

	/* clear the memory.... */
	for (i = 0; i < 1024; i++) {
		for (ae = HELPERSTable[i]; ae; ae = next) {
			next = ae->next;
			deleteHELPERSEntry(ae);
		}
	}

	if (HELPERSDB)
		free(HELPERSDB);

	alog("[\002ircd_helpers\002] Unloading module...");
}


/* ------------------------------------------------------------------------------- */


int do_help(User *u) {
	moduleNoticeLang(s_HelpServ, u, LANG_HELPERS_SYNTAX_EXT);
	return MOD_CONT;
}

/* Fantasy commands */
int do_kisayol(int argc, char **argv)
{
    User *u;
    ChannelInfo *ci;
    char *hedef = NULL;

    if (argc < 3)
        return MOD_CONT;
    /* greet message*/
    if ((stricmp(argv[0], "greet") == 0) && (greet == 1)) {
        u = finduser(argv[1]);
        ci = cs_findchan(argv[2]);
        if (!u || !ci || !check_access(u, ci, CA_AUTOOP) || (!(ci = cs_findchan(HelpChannel))))
            return MOD_CONT;

        if (argc >= 4)
            hedef = myStrGetToken(argv[3], ' ', 0);
        if (hedef){
           bot_raw_mode(u, ci, "v", hedef);
        anope_cmd_privmsg(ci->bi->nick, ci->name, "\002%s\002 %s", hedef, GREETMSG);
            ci->bi->lastmsg = time(NULL);
      }
        if (hedef)
       free(hedef);
    return MOD_CONT;
    }
   
    return MOD_CONT;
}


/* Helpers List */

int do_yardimci(User *u) {
	char *buffer, *cmd, *hnick, *passw, *flags;

	buffer = moduleGetLastBuffer();
	cmd = myStrGetToken(buffer, ' ', 0);
	hnick = myStrGetToken(buffer, ' ', 1);
	passw = myStrGetToken(buffer, ' ', 2);
	flags = myStrGetToken(buffer, ' ', 3);
	NickAlias *na;
	
	int is_servadmin = is_services_admin(u);
	
	ChannelInfo *cx = cs_findchan(HelpChannel);
	ChanAccess *access;
	int level1;
	int i;
	
	if (!(stricmp(HELPERS, "LOGIN") == 0) && !(stricmp(HELPERS, "ALL") == 0))
		moduleNoticeLang(s_HelpServ, u, LANG_HELPERS_DISABLED);
            
	else if (!cmd)
		moduleNoticeLang(s_HelpServ, u, LANG_HELPERS_SYNTAX);

		else if (stricmp(cmd, "ADD") == 0) {
		if (readonly)
			notice_lang(s_HelpServ, u, READ_ONLY_MODE);
		else if (!is_servadmin && (cx->flags & CI_SECUREFOUNDER ? !is_real_founder(u,cx) :
                   !is_founder(u, cx))) 
        	notice_lang(s_HelpServ, u, PERMISSION_DENIED);
            	else if (!passw || !hnick || !flags)
			moduleNoticeLang(s_HelpServ, u, LANG_HELPERS_SYNTAX);
		else if (!(stricmp(flags, "1") == 0) && !(stricmp(flags, "2") == 0))
			moduleNoticeLang(s_HelpServ, u, LANG_HELPERS_SYNTAX);

		else if (!(na = findnick(hnick)))
			notice_lang(s_HelpServ, u, NICK_X_NOT_REGISTERED, hnick);

		else {
	/* Channel Level */
	if (stricmp(flags, "1") == 0) {
	level1 = hoplvl;
	} else { 
	level1 = hsoplvl;
	}
			HELPERSEntry *ae = NULL;
			HELPERSPassw *ac = NULL;

			ae = getHELPERSEntry(findnick(hnick)->nc);
			ac = findHELPERSPassw(ae, passw);

			if (!ae)
				ae = createHELPERSEntry(findnick(hnick)->nc);
		      access = cx->access;
		      NickCore *nc;
		      nc = findnick(hnick) ->nc ;
			if (ac) {
				/* update existing entry */
				if (!flags && ac->flags) {
					free(ac->flags);
					ac->flags = NULL;
				} else if (flags) {
					free(ac->flags);
					ac->flags = sstrdup(flags);
				}
		     
		      
				if (stricmp(flags, "1") == 0) {
			 moduleNoticeLang(s_HelpServ, u, LANG_HELPERS_ADD, hnick, "HelperOp", flags);
	;
				
				} else {
			moduleNoticeLang(s_HelpServ, u, LANG_HELPERS_ADD, hnick, "HelperSOp", flags);

	
			} 
				} else {
	
				if (ae->ajnicks == NSHELPERSMax)
					notice (s_HelpServ, u->nick, "%s don't change", hnick);
				else {
		     
					clearHELPERSPassws(ae);
                                        addHELPERSPassw(ae, passw, flags);
					if (stricmp(flags, "1") == 0) {
			moduleNoticeLang(s_HelpServ, u, LANG_HELPERS_ADD, hnick, "HelperOp", flags);
	
				} else {
			moduleNoticeLang(s_HelpServ, u, LANG_HELPERS_ADD, hnick, "HelperSOp", flags);
                        
	
				}
			    }
			}
		save_data = 1;

	    /* Add access on help Channel */
	    for (access = cx->access, i = 0; i < cx->accesscount;
             access++, i++) {
            if (access->nc == nc) {
                /* Don't allow lowering from a level >= ulev */
               
                access->level = level1;
               
               
                
                return MOD_CONT;
            }
        }

       

        if (i < CSAccessMax) {
            cx->accesscount++;
            cx->access =
                srealloc(cx->access,
                        sizeof(ChanAccess) * cx->accesscount);
        } else {
            notice_lang(s_HelpServ, u, CHAN_ACCESS_REACHED_LIMIT,
                        CSAccessMax);
            return MOD_CONT;
        }

        access = &cx->access[i];
        access->nc = nc;
        access->in_use = 1;
        access->level = level1;
        access->last_seen = 0;
	alog("%s: %s added by %s to helpers list. His/Her Level %d on HelpChannel(%s)", s_HelpServ, hnick,  
	u->nick, level1, cx->name);
 
	    }

	
	
	} else if (stricmp(cmd, "DEL") == 0) {
		

		if (readonly)
			notice_lang(s_HelpServ, u, READ_ONLY_MODE);
		else if (!is_servadmin && (cx->flags & CI_SECUREFOUNDER ? !is_real_founder(u,cx) :
                   !is_founder(u, cx))) 
        	notice_lang(s_HelpServ, u, PERMISSION_DENIED);
            	else if (!hnick)
			moduleNoticeLang(s_HelpServ, u, LANG_HELPERS_SYNTAX);
		else if (!(na = findnick(hnick)))
			notice_lang(s_HelpServ, u, NICK_X_NOT_REGISTERED, hnick);
		else {
			HELPERSEntry *ae = NULL;
			ae = getHELPERSEntry(findnick(hnick)->nc);
		      access = cx->access;
		      NickCore *nc;
		      nc = findnick(hnick) ->nc ;
				if (!ae)
				notice (s_HelpServ, u->nick, "Now, %s isn't in helpers list", hnick);
				else {
				clearHELPERSPassws(ae);
				moduleNoticeLang(s_HelpServ, u, LANG_HELPERS_DEL, hnick);
		      
			}
			
	      /* Remove access on help Channel */
	      for (i = 0; i < cx->accesscount; i++) {
                if (cx->access[i].nc == nc)
                    break;
	      }
            
	      access = &cx->access[i];
            
       
                
                access->nc = NULL;
                access->in_use = 0;
		
		 alog("%s: %s removed by %s to helpers list. ", s_HelpServ, hnick,  u->nick);
		
		}

		/* LOG IN */
		}   else if (stricmp(cmd, "LOGIN") == 0) {
			
		if (readonly)
			notice_lang(s_HelpServ, u, READ_ONLY_MODE);
		else if (!passw || !hnick)
			notice (s_HelpServ, u->nick, "Username or password not found", hnick);
		else if (!(na = findnick(hnick)))
			notice (s_HelpServ, u->nick, "%s not found", hnick);
		else if (!u->na) 
			notice_lang(s_HelpServ, u, NICK_NOT_REGISTERED);
			else {
			HELPERSEntry *ae = NULL;
			HELPERSPassw *ac = NULL;
			ae = getHELPERSEntry(findnick(hnick)->nc);
			ac = findHELPERSPassw(ae, passw);
		 if (!ae) 
			notice (s_HelpServ, u->nick, "%s not found", hnick);
	
		else if (ac) {
		
		
				/* giris kontrol */
				if (strchr(ac->flags,'1')) {
		send_cmd(s_HostServ, "CHGHOST %s %s", u->nick, HOPHOST); 
		send_cmd(s_OperServ, "SVSMODE %s +%s", u->nick, HOPMODE);
		moduleNoticeLang(s_HelpServ, u, LANG_HELPERS_LOGIN_SUCCESS, "HelperOp");
		send_cmd(s_OperServ, "SVSJOIN %s %s", u->nick, HChannel);

				} else {
		send_cmd(s_HostServ, "CHGHOST %s %s", u->nick, HSOPHOST); 
		send_cmd(s_OperServ, "SVSMODE %s +%s", u->nick, HSOPMODE);
		moduleNoticeLang(s_HelpServ, u, LANG_HELPERS_LOGIN_SUCCESS, "HelperSOp");
		send_cmd(s_OperServ, "SVSJOIN %s %s", u->nick, HChannel);
				}
			}


		else {
					 notice_lang(s_HelpServ, u, PASSWORD_INCORRECT);
					 bad_password(u);
			}
	}
	
    	} else {
		moduleNoticeLang(s_HelpServ, u, LANG_HELPERS_DESC);
		moduleNoticeLang(s_HelpServ, u, LANG_HELPERS_SYNTAX);
	}
	
	if (cmd)
		free(cmd);
	if (hnick)
		free(hnick);
	if (passw)
		free(passw);
	if (flags)
		free(flags);

	return MOD_CONT;
}


int do_identify(User *u)
{
     ChannelInfo *cx = cs_findchan(HelpChannel);
    

 if ((stricmp(HELPERS, "ID") == 0) || (stricmp(HELPERS, "ALL") == 0)){
 if (nick_identified(u) && (check_access(u, cx, CA_AUTOOP))) {
	send_cmd(s_HostServ, "CHGHOST %s %s", u->nick, HOPHOST); 
	send_cmd(s_OperServ, "SVSMODE %s +%s", u->nick, HOPMODE);
	moduleNoticeLang(s_HelpServ, u, LANG_HELPERS_LOGIN_SUCCESS, "HelperOp");
	send_cmd(s_OperServ, "SVSJOIN %s %s", u->nick, HChannel);
} 
else if (nick_identified(u) && (check_access(u, cx, CA_AUTOPROTECT))) {
	send_cmd(s_HostServ, "CHGHOST %s %s", u->nick, HSOPHOST); 
	send_cmd(s_OperServ, "SVSMODE %s +%s", u->nick, HSOPMODE);
	moduleNoticeLang(s_HelpServ, u, LANG_HELPERS_LOGIN_SUCCESS, "HelperSOp");
	send_cmd(s_OperServ, "SVSJOIN %s %s", u->nick, HChannel);				
}
}	
 return MOD_CONT;
}


int null_func(User *u) {
  return MOD_CONT;
}





/* ------------------------------------------------------------------------------- */

/***************************************************************************************************************
 *         Generic DataBase Functions  (Taken from swhois by Trystan)
 ***************************************************************************************************************/


int new_open_db_read(DBFile *dbptr, char **flags, char **value) {
	*flags = malloc(MAXKEYLEN);
	*value = malloc(MAXVALLEN);

	if (!(dbptr->fptr = fopen(dbptr->filename, "rb"))) {
		if (debug) {
			alog("debug: Can't read %s database %s : errno(%d)", dbptr->service,
				dbptr->filename, errno);
		}
		free(*flags);
		*flags = NULL;
		free(*value);
		*value = NULL;
		return DB_READ_ERROR;
	}
	dbptr->db_version = fgetc(dbptr->fptr) << 24 | fgetc(dbptr->fptr) << 16
		| fgetc(dbptr->fptr) << 8 | fgetc(dbptr->fptr);

	if (ferror(dbptr->fptr)) {
		if (debug) {
			alog("debug: Error reading version number on %s", dbptr->filename);
		}
		free(*flags);
		*flags = NULL;
		free(*value);
		*value = NULL;
		return DB_READ_ERROR;
	} else if (feof(dbptr->fptr)) {
		if (debug) {
			alog("debug: Error reading version number on %s: End of file detected",
				dbptr->filename);
		}
		free(*flags);
		*flags = NULL;
		free(*value);
		*value = NULL;
		return DB_EOF_ERROR;
	} else if (dbptr->db_version < 1) {
		if (debug) {
			alog("debug: Invalid version number (%d) on %s", dbptr->db_version, dbptr->filename);
		}
		free(*flags);
		*flags = NULL;
		free(*value);
		*value = NULL;
		return DB_VERSION_ERROR;
	}
	return DB_READ_SUCCESS;
}


int new_open_db_write(DBFile *dbptr) {
	if (!(dbptr->fptr = fopen(dbptr->filename, "wb"))) {
		if (debug) {
			alog("debug: %s Can't open %s database for writing", dbptr->service, dbptr->filename);
		}
		return DB_WRITE_ERROR;
	}

	if (fputc(dbptr->core_db_version >> 24 & 0xFF, dbptr->fptr) < 0 ||
		fputc(dbptr->core_db_version >> 16 & 0xFF, dbptr->fptr) < 0 ||
		fputc(dbptr->core_db_version >> 8 & 0xFF, dbptr->fptr) < 0 ||
		fputc(dbptr->core_db_version & 0xFF, dbptr->fptr) < 0) {
		if (debug) {
			alog("debug: Error writing version number on %s", dbptr->filename);
		}
		return DB_WRITE_ERROR;
	}
	return DB_WRITE_SUCCESS;
}


void new_close_db(FILE *fptr, char **flags, char **value) {
	if (flags && *flags) {
		free(*flags);
		*flags = NULL;
	}
	if (value && *value) {
		free(*value);
		*value = NULL;
	}

	if (fptr) {
		fclose(fptr);
	}
}


int new_read_db_entry(char **flags, char **value, FILE *fptr) {
	char *string = *flags;
	int character;
	int i = 0;

	**flags = '\0';
	**value = '\0';

	while (1) {
		if ((character = fgetc(fptr)) == EOF) { /* a problem occurred reading the file */
			if (ferror(fptr)) {
				return DB_READ_ERROR;   /* error! */
			}
			return DB_EOF_ERROR;        /* end of file */
		} else if (character == BLOCKEND) {     /* END OF BLOCK */
			return DB_READ_BLOCKEND;
		} else if (character == VALUEEND) {     /* END OF VALUE */
			string[i] = '\0';   /* end of value */
			return DB_READ_SUCCESS;
		} else if (character == SEPARATOR) {    /* END OF KEY */
			string[i] = '\0';   /* end of flags */
			string = *value;    /* beginning of value */
			i = 0;              /* start with the first character of our value */
		} else {
			if ((i == (MAXKEYLEN - 1)) && (string == *flags)) {   /* max flags length reached, continuing with value */
				string[i] = '\0';       /* end of flags */
				string = *value;        /* beginning of value */
				i = 0;          /* start with the first character of our value */
			} else if ((i == (MAXVALLEN - 1)) && (string == *value)) {  /* max value length reached, returning */
				string[i] = '\0';
				return DB_READ_SUCCESS;
			} else {
				string[i] = character;  /* read string (flags or value) */
				i++;
			}
		}
	}
}


int new_write_db_entry(const char *flags, DBFile *dbptr, const char *fmt, ...) {
	char string[MAXKEYLEN + MAXVALLEN + 2], value[MAXVALLEN];   /* safety byte :P */
	va_list ap;
	unsigned int length;

	if (!dbptr) {
		return DB_WRITE_ERROR;
	}

	va_start(ap, fmt);
	vsnprintf(value, MAXVALLEN, fmt, ap);
	va_end(ap);

	if (!stricmp(value, "(null)")) {
		return DB_WRITE_NOVAL;
	}
	snprintf(string, MAXKEYLEN + MAXVALLEN + 1, "%s%c%s", flags, SEPARATOR, value);
	length = strlen(string);
	string[length] = VALUEEND;
	length++;

	if (fwrite(string, 1, length, dbptr->fptr) < length) {
		if (debug) {
			alog("debug: Error writing to %s", dbptr->filename);
		}
		new_close_db(dbptr->fptr, NULL, NULL);
		if (debug) {
			alog("debug: Restoring backup.");
		}
		remove(dbptr->filename);
		rename(dbptr->temp_name, dbptr->filename);
		free(dbptr);
		dbptr = NULL;
		return DB_WRITE_ERROR;
	}
	return DB_WRITE_SUCCESS;
}


int new_write_db_endofblock(DBFile *dbptr) {
	if (!dbptr) {
		return DB_WRITE_ERROR;
	}
	if (fputc(BLOCKEND, dbptr->fptr) == EOF) {
		if (debug) {
			alog("debug: Error writing to %s", dbptr->filename);
		}
		new_close_db(dbptr->fptr, NULL, NULL);
		return DB_WRITE_ERROR;
	}
	return DB_WRITE_SUCCESS;
}



void fill_db_ptr(DBFile *dbptr, int version, int core_version,
				char service[256], char filename[256]) {
	dbptr->db_version = version;
	dbptr->core_db_version = core_version;
	if (!service)
		strcpy(dbptr->service, service);
	else
		strcpy(dbptr->service, "");

	strcpy(dbptr->filename, filename);
	snprintf(dbptr->temp_name, 261, "%s.temp", filename);
	return;
}


/* ------------------------------------------------------------------------------- */

static HELPERSPassw *addHELPERSPassw(HELPERSEntry *ae, char *passw, char *flags) {
	HELPERSPassw *ac = NULL, *current = NULL, *previous = NULL;

	if (!ae || !passw)
		return NULL;

	for (current = ae->nicks; current; current = current->next) {
		/* search for the position to insert the new nick */
		if (stricmp(passw, current->nick) > 0)
			previous = current;
		else
			break;
	}

	if ((ac = malloc(sizeof(HELPERSPassw))) == NULL) {
		fatal("Out Of Memory!");
	}


	ac->next = current;
	ac->prev = previous;

	ac->nick = sstrdup(passw);
	if (flags)
		ac->flags = sstrdup(flags);
	else
		ac->flags = NULL;

	if (previous == NULL)
		ae->nicks = ac;
	else
		previous->next = ac;

	if (current)
		current->prev = ac;

	ae->ajnicks++;

	return ac;
}


HELPERSPassw *findHELPERSPassw(HELPERSEntry *ae, char *passw) {
	HELPERSPassw *current;

	if (!ae)
		return NULL;

	for (current = ae->nicks; current; current = current->next) {
		if (!stricmp(passw, current->nick))
			return current;
	}

	return NULL;
}





static int clearHELPERSPassws(HELPERSEntry *ae) {
	HELPERSPassw *ac = NULL, *next = NULL;

	if (!ae)
		return 0;

	for (ac = ae->nicks; ac; ac = next) {
		if (ac->next)
			next = ac->next;
		else
			next = NULL;

		if (ac->nick)
			free(ac->nick);
		if (ac->flags)
			free(ac->flags);

		free(ac);
	}

	ae->nicks = NULL;
	ae->ajnicks = 0;

	return 1;
}


/* ------------------------------------------------------------------------------- */

static HELPERSEntry *createHELPERSEntry(NickCore *nc) {
	HELPERSEntry *ae = NULL;
	char buf[BUFSIZE];

	if (!nc)
		return NULL;

	if ((ae = malloc(sizeof(HELPERSEntry))) == NULL) {
		fatal("Out Of Memory!");
	}

	ae->prev = NULL;
	ae->next = NULL;
	ae->row = counter;
	ae->ajnicks = 0;
	ae->in_use = 0;
	ae->flags = DefHELPERSFlags;
	ae->nicks = NULL;

	/* add it to the HELPERSTable */
	if (HELPERSTable[counter] == NULL) {
		HELPERSTable[counter] = ae;
		ae->col = 0;
	} else {
		int i = 0, last = -1;
		HELPERSEntry *t, *prev = NULL;

		for (t = HELPERSTable[counter]; t; t = t->next) {
			if (t->col != last + 1)
				break;

			last = t->col;
			i++;
			prev = t;
		}
		ae->col = i;

		if (prev) {
			if (prev->next) {
				ae->next = prev->next;
				prev->next->prev = ae;
			}
			prev->next = ae;
		}
		ae->prev = prev;
	}

	/* add to the nc, so it can be addressed relative quickly */
	snprintf(buf, BUFSIZE, "%d;%d", ae->row, ae->col);
	moduleAddData(&nc->moduleData, ModDataKey, buf);

	if (counter < 1023)
		counter++;
	else
		counter = 0;

	return ae;
}

HELPERSEntry *getHELPERSEntry(NickCore *nc) {
	char *t, *x, *y;
	int row, col;
	HELPERSEntry *ae = NULL;
	if (!nc)
		return NULL;

	t = moduleGetData(&nc->moduleData, ModDataKey);

	if (t == NULL) {
		return NULL;
	}
	x = myStrGetToken(t, ';', 0);
	y = myStrGetToken(t, ';', 1);

	row = atoi(x);
	col = atoi(y);

	for (ae = HELPERSTable[row]; ae; ae = ae->next) {
		if (ae->col == col)
			break;
	}

	free(t);
	free(x);
	free(y);

	return ae;
}

static int deleteHELPERSEntry(HELPERSEntry *ae) {
	if (!ae)
		return 0;

	if (HELPERSTable[ae->row] == ae)
		HELPERSTable[ae->row] = ae->next;

	if (ae->prev)
		ae->prev->next = ae->next;

	if (ae->next)
		ae->next->prev = ae->prev;

	clearHELPERSPassws(ae);
	ae->prev = NULL;
	ae->next = NULL;

	free(ae);

	return 1;
}

/**
 * When we are called, we assume used entries have in_use set to 1 (done while the
 * DB is saved) and reset it to 0 for all other entries.
 * Entries with in_use to 0 are free()'d and removed from the HELPERSTable since we
 * assume all other references to it have allready been removed.
 **/
void freeUnusedEntries() {
	HELPERSEntry *next = NULL, *ae = NULL;
	int i;

	for (i = 0; i < 1024; i++) {
		for (ae = HELPERSTable[i]; ae; ae = next) {
			if (ae->in_use == 1) {
				/* Entry is still being used... resetting */
				ae->in_use = 0;
				next = ae->next;
				continue;
			}

			/* Entry is no longer being used... free it */
			next = ae->next;
			deleteHELPERSEntry(ae);
		}
	}
}


/* ------------------------------------------------------------------------------- */

/**
 * When anope saves her databases, we do the same.
 **/
int do_save(int argc, char **argv) {
	if ((argc >= 1) && (!stricmp(argv[0], EVENT_STOP)))
		save_HELPERS_db();

	return MOD_CONT;
}


/**
 * When anope backs her databases up, we do the same.
 **/
int db_backup(int argc, char **argv) {
	if ((argc >= 1) && (!stricmp(argv[0], EVENT_STOP))) {
		alog("[ircd_helpers] Backing up database...");
		ModuleDatabaseBackup(HELPERSDB);

		/* When anope backs her databases up, we also clear unused entries from the HELPERSTable */
		freeUnusedEntries();
	}
	return MOD_CONT;
}


/**************************************************************************
 *                DataBase Handling
 **************************************************************************/

void load_HELPERS_db(void) {
	DBFile *dbptr = scalloc(1, sizeof(DBFile));
	HELPERSEntry *ae = NULL;
	NickCore *nc = NULL;
	char *flags, *value;
	int retval = 0;

	fill_db_ptr(dbptr, 0, HELPERSDBVERSION, s_HelpServ, HELPERSDB);

	/* let's remove existing temp files here, because we only load dbs on startup */
	remove(dbptr->temp_name);

	/* Open the db, fill the rest of dbptr and allocate memory for flags and value */
	if (new_open_db_read(dbptr, &flags, &value)) {
		free(dbptr);
		return;                 /* Bang, an error occurred */
	}

	while (1) {
		/* read a new entry and fill flags and value with it -Certus */
		retval = new_read_db_entry(&flags, &value, dbptr->fptr);

		if (retval == DB_READ_ERROR) {
			new_close_db(dbptr->fptr, &flags, &value);
			free(dbptr);
			return;

		} else if (retval == DB_EOF_ERROR) {
			new_close_db(dbptr->fptr, &flags, &value);
			free(dbptr);
			return;
        } else if (retval == DB_READ_BLOCKEND) {        /* DB_READ_BLOCKEND */
        	/* prevent passws from one block ending up in another... */
            ae = NULL;
		} else {		 /* DB_READ_SUCCESS */
			if (!*flags || !*value)
				continue;

			/* nick */
			if (!stricmp(flags, "ncd")) {
				if ((nc = findcore(value))) {
					ae = createHELPERSEntry(nc);
				} else
					ae = NULL;

			/* flags */
			} else if (!stricmp(flags, "flgs") && ae) {
				ae->flags = (uint16) atoi(value);

			/* nick entry */
			} else if (!stricmp(flags, "che") && ae) {
				char *passw = myStrGetToken(value, ' ', 0);
				char *chflags = myStrGetToken(value, ' ', 1);

				addHELPERSPassw(ae, passw, chflags);

				free(passw);
				if (chflags)
					free(chflags);
			} else if (!stricmp(flags, "HELPERS DB VERSION")) {
				if ((int)atoi(value) != HELPERSDBVERSION) {
					alog("[\002ircd_helpers\002] Database version does not match any database versions supported by this module.");
					alog("[\002ircd_helpers\002] Continuing with clean database...");
					break;
				}
			}
		} 					/* else */
	}					/* while */

	free(dbptr);
}


void save_HELPERS_db(void) {
	DBFile *dbptr = scalloc(1, sizeof(DBFile));
	NickCore *nc;
	int i;

	fill_db_ptr(dbptr, 0, HELPERSDBVERSION, s_HelpServ, HELPERSDB);

	/* time to backup the old db */
	rename(HELPERSDB, dbptr->temp_name);

	if (new_open_db_write(dbptr)) {
		rename(dbptr->temp_name, HELPERSDB);
		free(dbptr);
		return;                /* Bang, an error occurred */
	}

	/* Store the version of the DB in the DB as well...
	 * This will make stuff a lot easier if the database scheme needs to modified. */
	new_write_db_entry("HELPERS DB VERSION", dbptr, "%d", HELPERSDBVERSION);
	new_write_db_endofblock(dbptr);

	/* Go over the entire NickCore list and check whether each one has an HELPERS entry */
    for (i = 0; i < 1024; i++) {
        for (nc = nclists[i]; nc; nc = nc->next) {
			HELPERSEntry *ae = NULL;
			HELPERSPassw *ac = NULL;

			ae = getHELPERSEntry(nc);
			if (!ae)
				continue;

			/* If there are no entries in the list and the configuration is set to the default
			 * value, there is no need to save it to the db. */
			if (ae->ajnicks == 0 && ae->flags == DefHELPERSFlags) {
				/* Delete the entry first since it s just eating up space... */
				moduleDelData(&nc->moduleData, ModDataKey);
				deleteHELPERSEntry(ae);
				continue;
			}

			new_write_db_entry("ncd", dbptr, "%s", nc->display);
			new_write_db_entry("flgs", dbptr, "%d", ae->flags);

			if (ae->ajnicks > 0) {
				for (ac = ae->nicks; ac; ac = ac->next) {
					new_write_db_entry("che", dbptr, "%s %s", ac->nick, (ac->flags ? ac->flags : ""));
				}
			}

			new_write_db_endofblock(dbptr);

			/* Marking the entry as in use... */
			ae->in_use = 1;
		}
	}

	if (dbptr) {
		new_close_db(dbptr->fptr, NULL, NULL);  /* close file */
		remove(dbptr->temp_name);       /* saved successfully, no need to keep the old one */
		free(dbptr);           /* free the db struct */
	}
}

/* ------------------------------------------------------------------------------- */

/**
 * We check if the ircd is supported
 **/
int valid_ircd(void) {
	if (!stricmp(IRCDModule, "unreal31"))
		return 1;

	if (!stricmp(IRCDModule, "unreal32"))
		return 1;

	

	if (!stricmp(IRCDModule, "inspircd11"))
		return 1;

	return 0;
}

/* ------------------------------------------------------------------------------- */

/**
 * (Re)Load the configuration directives
 **/
void load_config(void) {
	int i;

	Directive confvalues[][1] = {
		{{"HELPERS", {{PARAM_STRING, PARAM_RELOAD, &HELPERS}}}},
	};

	if (HELPERS)
	free(HELPERS);
	HELPERS = NULL;

	for (i = 0; i < 1; i++)
		moduleGetConfigDirective(confvalues[i]);
	  if (!HELPERS)
		HELPERS = sstrdup(DefHELPERS);
	if (!HELPERSDB)
		HELPERSDB = sstrdup(DefHELPERSDB);

	if (debug)
		alog ("[ircd_helpers] debug: HELPERSDB set to %s", HELPERSDB);
}


/**
 * Upon /os reload call the routines for reloading the configuration directives
 **/
int reload_config(int argc, char **argv) {
	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			alog("[ircd_helpers]: Reloading configuration directives...");
			load_config();
		}
	}
	return MOD_CONT;
}


/* ------------------------------------------------------------------------------- */

/**
 * Add language strings to the module's language db.
 **/
void add_languages(void) {
	char *langtable_en_us[] = {
		/* LANG_HELPERS_DESC */
		" HELPERS       Modify the Helpers  list",
		/* LANG_HELPERS_SYNTAX */
		" Syntax: HELPERS { ADD | DEL | LOGIN } [\037nickname\037] [\037password\037] [\037flags(1/2)\037]",
		/* LANG_HELPERS_SYNTAX_EXT */
		" Syntax: \002HELPERS ADD \037nick\037 \037password\037 \037flags\037\002 \n"
		"         \002HELPERS DEL \037nick\037\002 \n"
		"         \002HELPERS LOGIN\002 \037nick\037 \037password\037\n"
		" \n"
		" Allows the Services Root admins and help channel owner to add or remove nicknames, \n"
		" to or from the helpers list. A user whose nickname is on the helpers list \n"
		" and who has login to system will get some modes and vhost ect. \n"
		" \n"
		" The \002HELPERS ADD\002 command adds the given nickname to the\n"
		" helpers list \n"
		" \n"
		" HelpOp Flag is '1' HelpSop's '2' : \n"
		" \n"
		" The \002HELPERS DEL\002  command removes the given nick from the \n"
		" Helpers list. \n"
		" \n"
		" The \002HELPERS LOGIN\002 command log in the helpers system.\n"
		" When you log in, You have some usermodes e.g. +h etc.",
		/* LANG_HELPERS_ADD */
		" %s Added to Helpers list. His/Her level %s(%s)",
		/* LANG_HELPERS_DEL */
		" %s Removed to Helpers list.",
		/* LANG_HELPERS_LOGIN_SUCCESS */
		" Now, You're A Helper. Your Level %s ",
		/* LANG_HELPERS_DISAPLED */
		" LOGIN system disabled ",

	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}


/* EOF */

