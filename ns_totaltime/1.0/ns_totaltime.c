/* 
* ns_totaltime 1
*
* This module record user total online time 
* 
* And show user's total time on ns info
*
* Changes:
*  v,1
** Deleted chech cs_register fonk.
*
*
*/ 

/* Some code taken from ns_ajoin by Viper */



/* Dont change any option after this line. */




/* This options not recommended. This very dangerous. */
/* Pls first off all read to README */
/* This options not recommended. This very dangerous. Because if you activated it, you must edit some core code */
int acctime = 1; /* hours */
int accesscheck = 0; /* 0 is off , 1 is on */
/* end of check access options */
/* ---------------------------------------------------------------------------------------------- */

#include "module.h"

#define AUTHOR "ysfm"
#define VERSION "$Id: ns_totaltime  V1.0 24-12-2009 ysfm $"






/* Flags to set in database */
#define TOTALTIME_ON        (1 << 0)
#define TOTALTIME_SILENT    (1 << 1)
#define TOTALTIMEDBVERSION 1
/* Database seperators */
#define SEPARATOR  ':'          /* End of a timeall, seperates timealls from values */
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
int NSTOTALTIMEMax = 25;

/* Structs */
typedef struct db_file_ DBFile;
typedef struct TOTALTIMEchanx_ TOTALTIMEPrefiX;
typedef struct TOTALTIMEentry_ TOTALTIMEEntry;

struct db_file_ {
	FILE *fptr;             /* Pointer to the opened file */
	int db_version;         /* The db version of the datafiles (only needed for reading) */
	int core_db_version;    /* The current db version of this anope source */
	char service[256];      /* StatServ/etc. */
	char filename[256];     /* Filename of the database */
	char temp_name[262];    /* Temp filename of the database */
};

struct TOTALTIMEchanx_ {
	TOTALTIMEPrefiX *prev, *next;
	char *nick;
	char *timeall;
};

struct TOTALTIMEentry_ {
	/* Specifies storage location in TOTALTIMETable */
	int row, col;
	TOTALTIMEEntry *prev, *next;
	int ajnicks;
	TOTALTIMEPrefiX *nicks;
	/* Flags store TOTALTIME settings.
	 * Bits are used as followed:
	 *   User wants to be autojoined            {bit 0}
	 *   User wants autojoin to be silent       {bit 1}
	 */
	uint16 timeall;
	/* Used to check during the cleanup whether the entry is still being used.
	 * This is always 0 except when the DB saving function has completed going
	 * over all NickCore's. The TOTALTIME entries with in_use set to 0 at that time
	 * are no longer used and cleared. In the other entries in_use is reset to 0. */
	int in_use;
};


char *DefTOTALTIMEDB = "ns_totaltime.db";
char *ModDataKey = "TOTALTIME";
uint16 DefTOTALTIMEFlags = 1;

char *DefTOTALTIME = "LOGIN";
char *TOTALTIMEDB;
char *TOTALTIME;
int counter;

TOTALTIMEEntry *TOTALTIMETable[1024];


int do_komut(User *u);
int do_total(int argc, char **argv);
int do_giris(int argc, char **argv);
int do_accadd(int argc, char **argv);
int do_cont(int argc, char **argv);




int null_func(User *u);


void clean_nickname(char *cn);

int TOTALTIME_del_callback(User *u, int num, va_list args);

int new_open_db_read(DBFile *dbptr, char **timeall, char **value);
int new_open_db_write(DBFile *dbptr);
void new_close_db(FILE *fptr, char **timeall, char **value);
int new_read_db_entry(char **timeall, char **value, FILE * fptr);
int new_write_db_entry(const char *timeall, DBFile *dbptr, const char *fmt, ...);
int new_write_db_endofblock(DBFile *dbptr);
void fill_db_ptr(DBFile *dbptr, int version, int core_version, char service[256], char filename[256]);

static TOTALTIMEPrefiX *addTOTALTIMEPrefiX(TOTALTIMEEntry *ae, char *chanx, char *timeall);
TOTALTIMEPrefiX *findTOTALTIMEPrefiX(TOTALTIMEEntry *ae, char *chanx);
static int deleteTOTALTIMEPrefiX(TOTALTIMEEntry *ae, TOTALTIMEPrefiX *ac);
static int clearTOTALTIMEPrefiXs(TOTALTIMEEntry *ae);

static TOTALTIMEEntry *createTOTALTIMEEntry(NickCore *nc);
TOTALTIMEEntry *getTOTALTIMEEntry(NickCore *nc);
static int deleteTOTALTIMEEntry(TOTALTIMEEntry *ae);
void freeUnusedEntries();

int do_save(int argc, char **argv);
int db_backup(int argc, char **argv);
void load_TOTALTIME_db(void);
void save_TOTALTIME_db(void);
TOTALTIMEEntry *go_to_next_entry(uint16 skipped, TOTALTIMEEntry *next, TOTALTIMEEntry *ae);

int valid_ircd(void);

void load_config(void);
int reload_config(int argc, char **argv);



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
	int status;


	alog("[\002ns_totaltime\002] Loading module...");

	counter = 0;

	if (!valid_ircd()) {
		alog("[\002ns_totaltime\002] ERROR: IRCd not supported by this module");
		alog("[\002ns_totaltime\002] Unloading module...");
		return MOD_STOP;
	}

	if (!moduleMinVersion(1,7,20,1324)) {
		alog("[\002ns_totaltime\002] Your version of Anope isn't supported. Please update to a newer release.");
		return MOD_STOP;
	}



	
	/* access change */
        hook = createEventHook(EVENT_USER_LOGOFF, do_total);
	moduleAddEventHook(hook);
	/* access del */
        hook = createEventHook(EVENT_NICK_IDENTIFY, do_giris);
	moduleAddEventHook(hook);
	hook = createEventHook(EVENT_ACCESS_ADD, do_accadd);
	moduleAddEventHook(hook);
	hook = createEventHook(EVENT_NEWNICK, do_cont);
	moduleAddEventHook(hook);

	c = createCommand("INFO", do_komut, NULL, -1, -1, -1, -1, -1);
	status = moduleAddCommand(NICKSERV, c, MOD_TAIL);


	
	/* Hook to some events.. */
	hook = createEventHook(EVENT_RELOAD, reload_config);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002ns_totaltime\002] Can't hook to EVENT_RELOAD event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_DB_SAVING, do_save);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002ns_totaltime\002] Can't hook to EVENT_DB_SAVING event");
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_DB_BACKUP, db_backup);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002ns_totaltime\002] Can't hook to EVENT_DB_BACKUP event");
		return MOD_STOP;
	}

	load_config();
	load_TOTALTIME_db();

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	alog("[\002ns_totaltime\002] Module loaded successfully...");

	return MOD_CONT;
}


/**
 * Unload the module
 **/
void AnopeFini(void) {
	TOTALTIMEEntry *ae = NULL, *next = NULL;
	int i;

	if (TOTALTIMEDB)
		save_TOTALTIME_db();

	/* clear the memory.... */
	for (i = 0; i < 1024; i++) {
		for (ae = TOTALTIMETable[i]; ae; ae = next) {
			next = ae->next;
			deleteTOTALTIMEEntry(ae);
		}
	}

	if (TOTALTIMEDB)
		free(TOTALTIMEDB);

	alog("[\002ns_totaltime\002] Unloading module...");
}


/* ------------------------------------------------------------------------------- */

int do_komut(User * u)
{
    char *text = NULL;
    char *nick = NULL;
    char *prefixon = "total";
    NickAlias *na = NULL;
	TOTALTIMEEntry *ae = NULL;
			TOTALTIMEPrefiX *ac = NULL;
    int zaman;
        /* Get the last buffer anope recived */
        text = moduleGetLastBuffer();
        if (text) {
            nick = myStrGetToken(text, ' ', 0);
            if (nick) {
                /* ok we've found the user */
                if ((na = findnick(nick))) {
                    /* If we have any info on this user */
			

			ae = getTOTALTIMEEntry(findnick(nick)->nc);
			ac = findTOTALTIMEPrefiX(ae, prefixon);
			if (ac) {
			 zaman = atoi(ac->timeall); 
                        notice_user(s_NickServ, u, "      	 Total Online Time: %d days %d hours %d secs ve %d mins.", (zaman / 86400), ((zaman % 86400) / 3600), (((zaman % 86400) % 3600) / 60), ((((zaman % 86400) % 3600) % 60) % 60));
			}
		
                    /* NickCore not found! */
                } else {
                    /* we dont care! */
                }
                free(nick);
            }
        }
    return MOD_CONT;
}




int do_total(int argc, char **argv)
{



       
	NickAlias *na;
	char *nick2 = NULL;
	char *prefixon = "total";
	char *prefixid = "ident";
	int sonuc;
	int deger;
	int eski;
	int giris;
	TOTALTIMEEntry *ae = NULL;
			TOTALTIMEPrefiX *ac = NULL;
			TOTALTIMEPrefiX *id = NULL;
	char deger1[1024];
	char deger2[1024];
    	if (argc < 1)
        return MOD_CONT;

      	nick2 = myStrGetToken(argv[0], ' ', 0);


	
        if (!(na = findnick(nick2)))
	  
            return MOD_CONT;
			
			
			ae = getTOTALTIMEEntry(findnick(nick2)->nc);
			ac = findTOTALTIMEPrefiX(ae, prefixon);
			id = findTOTALTIMEPrefiX(ae, prefixid);
			
			if (deger < 0 ) 
			deger=0;
			  
			if (!ae)
				ae = createTOTALTIMEEntry(findnick(nick2)->nc);
			if (id) {
			giris = atoi(id->timeall);
			deger = (time(NULL) - giris);
			deleteTOTALTIMEPrefiX(ae, id);
			if (ac) {
				/* update existing entry */
					eski = atoi(ac->timeall);
					sonuc = eski + deger;
					sprintf(deger1, "%d", sonuc);
					free(ac->timeall);
					ac->timeall = sstrdup(deger1);
					
				
			 } else {	
	
				if (ae->ajnicks == NSTOTALTIMEMax)
				alog("debug: Error reading version number on  End of file detected");
				else {
				sprintf(deger2, "%d", deger);
				addTOTALTIMEPrefiX(ae, prefixon, deger2);
				
			    }
			 }
		}

		Anope_Free(nick2);

 return MOD_CONT;
}






int do_giris(int argc, char **argv)
{



       
	NickAlias *na;
	char *nick2 = NULL;
	char *prefixid = "ident";
	int deger;
		TOTALTIMEEntry *ae = NULL;
			TOTALTIMEPrefiX *ac = NULL;
	char deger1[1024];
	char deger2[1024];
    	if (argc < 1)
        return MOD_CONT;

      	nick2 = myStrGetToken(argv[0], ' ', 0);


	
        if (!(na = findnick(nick2)))
	  
            return MOD_CONT;
		

			ae = getTOTALTIMEEntry(findnick(nick2)->nc);
			ac = findTOTALTIMEPrefiX(ae, prefixid);
			deger = (time(NULL));
			if (deger < 0 ) 
			deger=0;
			  
			if (!ae)
				ae = createTOTALTIMEEntry(findnick(nick2)->nc);
			if (ac) {
				/* update existing entry */
					sprintf(deger1, "%d", deger);
					free(ac->timeall);
					ac->timeall = sstrdup(deger1);
					
				
			 } else {	
	
				if (ae->ajnicks == NSTOTALTIMEMax)
				alog("debug: Error reading version number on  End of file detected");
				else {
				sprintf(deger2, "%d", deger);
				addTOTALTIMEPrefiX(ae, prefixid, deger2);
				
			    }
		}

		Anope_Free(nick2);

 return MOD_CONT;
}

int do_cont(int argc, char **argv)
{



      TOTALTIMEEntry *ae = NULL;
			TOTALTIMEPrefiX *id = NULL; 
	NickAlias *na;
	char *nick2 = NULL;
	char *prefixid = "ident";
	
	
    	if (argc < 1)
        return MOD_CONT;

      	nick2 = myStrGetToken(argv[0], ' ', 0);


	
        if (!(na = findnick(nick2)))
	  
            return MOD_CONT;
			
			
			ae = getTOTALTIMEEntry(findnick(nick2)->nc);
			id = findTOTALTIMEPrefiX(ae, prefixid);
			
						  
			if (ae) {
				
			if (id) {
			deleteTOTALTIMEPrefiX(ae, id);
			return MOD_CONT;
		}
			}

		Anope_Free(nick2);

 return MOD_CONT;
}





int do_accadd(int argc, char **argv)
{
    User *u;
    char *chanx = NULL;
    char *nick2 = NULL;
    char *lvl = NULL;
    char *added = NULL;
		 NickCore *nc;
    char *prefixon = "total";
    int zaman;
	 int level;
	int i;
      long a;
	 ChannelInfo *cx = cs_findchan(chanx);
      ChanAccess *access;
		TOTALTIMEEntry *ae = NULL;
			TOTALTIMEPrefiX *ac = NULL;
    
    if (argc < 3)
        return MOD_CONT;

      chanx = myStrGetToken(argv[0], ' ', 0);
      added = myStrGetToken(argv[1], ' ', 0);
      nick2 = myStrGetToken(argv[2], ' ', 0);
      lvl = myStrGetToken(argv[3], ' ', 0);
      u = finduser(argv[1]);
     
      
      
        if (!added || !chanx || !nick2 || !lvl || (accesscheck == 0))
            return MOD_CONT;
			
			
			
			ae = getTOTALTIMEEntry(findnick(nick2)->nc);
			ac = findTOTALTIMEPrefiX(ae, prefixon);
			

			if (ae)
			  
			if (ac) {
				/* chech totaltime */
			
				 nc = findnick(nick2) ->nc ;
				zaman = atoi(ac->timeall);
				access = cx->access;
				a = 3600 * acctime;
				if (zaman < a) {
				  /* error */
				   notice_user(s_ChanServ, u, "%s didn't added access list. Because this user dont have enough total online time",nick2);
				   
				  for (i = 0; i < cx->accesscount; i++) {
				  if (cx->access[i].nc == nc)
				  break;
				  }
            
				  access = &cx->access[i];
            
       
                
				  access->nc = NULL;
				  access->in_use = 0;
				  
				    return MOD_CONT;
				} else {
				 
				  /* access added's messages */
				  
				  level = atoi(lvl);
				   alog("%s: %s!%s@%s  set access level %s to %s (group %s) on channel %s", s_ChanServ, u->nick, u->username, u->host,  lvl, nick2, nc->display, cx->name);
			       notice_lang(s_ChanServ, u, CHAN_ACCESS_ADDED, nc->display,
                    cx->name, level);
				}
				return MOD_CONT;
			} 
			
		
Anope_Free(chanx);
Anope_Free(added);
Anope_Free(nick2);
Anope_Free(lvl);
 return MOD_CONT;
}



















/***************************************************************************************************************
 *         Generic DataBase Functions  (Taken from swhois by Trystan)
 ***************************************************************************************************************/


int new_open_db_read(DBFile *dbptr, char **timeall, char **value) {
	*timeall = malloc(MAXKEYLEN);
	*value = malloc(MAXVALLEN);

	if (!(dbptr->fptr = fopen(dbptr->filename, "rb"))) {
		if (debug) {
			alog("debug: Can't read %s database %s : errno(%d)", dbptr->service,
				dbptr->filename, errno);
		}
		free(*timeall);
		*timeall = NULL;
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
		free(*timeall);
		*timeall = NULL;
		free(*value);
		*value = NULL;
		return DB_READ_ERROR;
	} else if (feof(dbptr->fptr)) {
		if (debug) {
			alog("debug: Error reading version number on %s: End of file detected",
				dbptr->filename);
		}
		free(*timeall);
		*timeall = NULL;
		free(*value);
		*value = NULL;
		return DB_EOF_ERROR;
	} else if (dbptr->db_version < 1) {
		if (debug) {
			alog("debug: Invalid version number (%d) on %s", dbptr->db_version, dbptr->filename);
		}
		free(*timeall);
		*timeall = NULL;
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


void new_close_db(FILE *fptr, char **timeall, char **value) {
	if (timeall && *timeall) {
		free(*timeall);
		*timeall = NULL;
	}
	if (value && *value) {
		free(*value);
		*value = NULL;
	}

	if (fptr) {
		fclose(fptr);
	}
}


int new_read_db_entry(char **timeall, char **value, FILE *fptr) {
	char *string = *timeall;
	int character;
	int i = 0;

	**timeall = '\0';
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
			string[i] = '\0';   /* end of timeall */
			string = *value;    /* beginning of value */
			i = 0;              /* start with the first character of our value */
		} else {
			if ((i == (MAXKEYLEN - 1)) && (string == *timeall)) {   /* max timeall length reached, continuing with value */
				string[i] = '\0';       /* end of timeall */
				string = *value;        /* beginning of value */
				i = 0;          /* start with the first character of our value */
			} else if ((i == (MAXVALLEN - 1)) && (string == *value)) {  /* max value length reached, returning */
				string[i] = '\0';
				return DB_READ_SUCCESS;
			} else {
				string[i] = character;  /* read string (timeall or value) */
				i++;
			}
		}
	}
}


int new_write_db_entry(const char *timeall, DBFile *dbptr, const char *fmt, ...) {
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
	snprintf(string, MAXKEYLEN + MAXVALLEN + 1, "%s%c%s", timeall, SEPARATOR, value);
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

static TOTALTIMEPrefiX *addTOTALTIMEPrefiX(TOTALTIMEEntry *ae, char *chanx, char *timeall) {
	TOTALTIMEPrefiX *ac = NULL, *current = NULL, *previous = NULL;

	if (!ae || !chanx)
		return NULL;

	for (current = ae->nicks; current; current = current->next) {
		/* search for the position to insert the new nick */
		if (stricmp(chanx, current->nick) > 0)
			previous = current;
		else
			break;
	}

	if ((ac = malloc(sizeof(TOTALTIMEPrefiX))) == NULL) {
		fatal("Out Of Memory!");
	}


	ac->next = current;
	ac->prev = previous;

	ac->nick = sstrdup(chanx);
	if (timeall)
		ac->timeall = sstrdup(timeall);
	else
		ac->timeall = NULL;

	if (previous == NULL)
		ae->nicks = ac;
	else
		previous->next = ac;

	if (current)
		current->prev = ac;

	ae->ajnicks++;

	return ac;
}


TOTALTIMEPrefiX *findTOTALTIMEPrefiX(TOTALTIMEEntry *ae, char *chanx) {
	TOTALTIMEPrefiX *current;

	if (!ae)
		return NULL;

	for (current = ae->nicks; current; current = current->next) {
		if (!stricmp(chanx, current->nick))
			return current;
	}

	return NULL;
}



static int deleteTOTALTIMEPrefiX(TOTALTIMEEntry *ae, TOTALTIMEPrefiX *ac) {
	if (!ae || !ac)
		return 0;

	if (ac->prev)
		ac->prev->next = ac->next;
	else
		ae->nicks = ac->next;

	if (ac->next)
		ac->next->prev = ac->prev;

	if (ac->nick)
		free(ac->nick);
	if (ac->timeall)
		free(ac->timeall);

	free(ac);
	ae->ajnicks--;

	return 1;
}

static int clearTOTALTIMEPrefiXs(TOTALTIMEEntry *ae) {
	TOTALTIMEPrefiX *ac = NULL, *next = NULL;

	if (!ae)
		return 0;

	for (ac = ae->nicks; ac; ac = next) {
		if (ac->next)
			next = ac->next;
		else
			next = NULL;

		if (ac->nick)
			free(ac->nick);
		if (ac->timeall)
			free(ac->timeall);

		free(ac);
	}

	ae->nicks = NULL;
	ae->ajnicks = 0;

	return 1;
}


/* ------------------------------------------------------------------------------- */

static TOTALTIMEEntry *createTOTALTIMEEntry(NickCore *nc) {
	TOTALTIMEEntry *ae = NULL;
	char buf[BUFSIZE];

	if (!nc)
		return NULL;

	if ((ae = malloc(sizeof(TOTALTIMEEntry))) == NULL) {
		fatal("Out Of Memory!");
	}

	ae->prev = NULL;
	ae->next = NULL;
	ae->row = counter;
	ae->ajnicks = 0;
	ae->in_use = 0;
	ae->timeall = DefTOTALTIMEFlags;
	ae->nicks = NULL;

	/* add it to the TOTALTIMETable */
	if (TOTALTIMETable[counter] == NULL) {
		TOTALTIMETable[counter] = ae;
		ae->col = 0;
	} else {
		int i = 0, last = -1;
		TOTALTIMEEntry *t, *prev = NULL;

		for (t = TOTALTIMETable[counter]; t; t = t->next) {
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

TOTALTIMEEntry *getTOTALTIMEEntry(NickCore *nc) {
	char *t, *x, *y;
	int row, col;
	TOTALTIMEEntry *ae = NULL;
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

	for (ae = TOTALTIMETable[row]; ae; ae = ae->next) {
		if (ae->col == col)
			break;
	}

	free(t);
	free(x);
	free(y);

	return ae;
}


static int deleteTOTALTIMEEntry(TOTALTIMEEntry *ae) {
	if (!ae)
		return 0;

	if (TOTALTIMETable[ae->row] == ae)
		TOTALTIMETable[ae->row] = ae->next;

	if (ae->prev)
		ae->prev->next = ae->next;

	if (ae->next)
		ae->next->prev = ae->prev;

	clearTOTALTIMEPrefiXs(ae);
	ae->prev = NULL;
	ae->next = NULL;

	free(ae);

	return 1;
}

/**
 * When we are called, we assume used entries have in_use set to 1 (done while the
 * DB is saved) and reset it to 0 for all other entries.
 * Entries with in_use to 0 are free()'d and removed from the TOTALTIMETable since we
 * assume all other references to it have allready been removed.
 **/
void freeUnusedEntries() {
	TOTALTIMEEntry *next = NULL, *ae = NULL;
	int i;

	for (i = 0; i < 1024; i++) {
		for (ae = TOTALTIMETable[i]; ae; ae = next) {
			if (ae->in_use == 1) {
				/* Entry is still being used... resetting */
				ae->in_use = 0;
				next = ae->next;
				continue;
			}

			/* Entry is no longer being used... free it */
			next = ae->next;
			deleteTOTALTIMEEntry(ae);
		}
	}
}


/* ------------------------------------------------------------------------------- */

/**
 * When anope saves her databases, we do the same.
 **/
int do_save(int argc, char **argv) {
	if ((argc >= 1) && (!stricmp(argv[0], EVENT_STOP)))
		save_TOTALTIME_db();

	return MOD_CONT;
}


/**
 * When anope backs her databases up, we do the same.
 **/
int db_backup(int argc, char **argv) {
	if ((argc >= 1) && (!stricmp(argv[0], EVENT_STOP))) {
		alog("[ns_totaltime] Backing up database...");
		ModuleDatabaseBackup(TOTALTIMEDB);

		/* When anope backs her databases up, we also clear unused entries from the TOTALTIMETable */
		freeUnusedEntries();
	}
	return MOD_CONT;
}


/**************************************************************************
 *                DataBase Handling
 **************************************************************************/

void load_TOTALTIME_db(void) {
	DBFile *dbptr = scalloc(1, sizeof(DBFile));
	TOTALTIMEEntry *ae = NULL;
	NickCore *nc = NULL;
	char *timeall, *value;
	int retval = 0;

	fill_db_ptr(dbptr, 0, TOTALTIMEDBVERSION, s_HelpServ, TOTALTIMEDB);

	/* let's remove existing temp files here, because we only load dbs on startup */
	remove(dbptr->temp_name);

	/* Open the db, fill the rest of dbptr and allocate memory for timeall and value */
	if (new_open_db_read(dbptr, &timeall, &value)) {
		free(dbptr);
		return;                 /* Bang, an error occurred */
	}

	while (1) {
		/* read a new entry and fill timeall and value with it -Certus */
		retval = new_read_db_entry(&timeall, &value, dbptr->fptr);

		if (retval == DB_READ_ERROR) {
			new_close_db(dbptr->fptr, &timeall, &value);
			free(dbptr);
			return;

		} else if (retval == DB_EOF_ERROR) {
			new_close_db(dbptr->fptr, &timeall, &value);
			free(dbptr);
			return;
        } else if (retval == DB_READ_BLOCKEND) {        /* DB_READ_BLOCKEND */
        	/* prevent chanxs from one block ending up in another... */
            ae = NULL;
		} else {		 /* DB_READ_SUCCESS */
			if (!*timeall || !*value)
				continue;

			/* nick */
			if (!stricmp(timeall, "ncd")) {
				if ((nc = findcore(value))) {
					ae = createTOTALTIMEEntry(nc);
				} else
					ae = NULL;

			/* timeall */
			} else if (!stricmp(timeall, "flgs") && ae) {
				ae->timeall = (uint16) atoi(value);

			/* nick entry */
			} else if (!stricmp(timeall, "che") && ae) {
				char *chanx = myStrGetToken(value, ' ', 0);
				char *chtimeall = myStrGetToken(value, ' ', 1);

				addTOTALTIMEPrefiX(ae, chanx, chtimeall);

				free(chanx);
				if (chtimeall)
					free(chtimeall);
			} else if (!stricmp(timeall, "TOTALTIME DB VERSION")) {
				if ((int)atoi(value) != TOTALTIMEDBVERSION) {
					alog("[\002ns_totaltime\002] Database version does not match any database versions supported by this module.");
					alog("[\002ns_totaltime\002] Continuing with clean database...");
					break;
				}
			}
		} 					/* else */
	}					/* while */

	free(dbptr);
}


void save_TOTALTIME_db(void) {
	DBFile *dbptr = scalloc(1, sizeof(DBFile));
	NickCore *nc;
	int i;

	fill_db_ptr(dbptr, 0, TOTALTIMEDBVERSION, s_HelpServ, TOTALTIMEDB);

	/* time to backup the old db */
	rename(TOTALTIMEDB, dbptr->temp_name);

	if (new_open_db_write(dbptr)) {
		rename(dbptr->temp_name, TOTALTIMEDB);
		free(dbptr);
		return;                /* Bang, an error occurred */
	}

	/* Store the version of the DB in the DB as well...
	 * This will make stuff a lot easier if the database scheme needs to modified. */
	new_write_db_entry("TOTALTIME DB VERSION", dbptr, "%d", TOTALTIMEDBVERSION);
	new_write_db_endofblock(dbptr);

	/* Go over the entire NickCore list and check whether each one has an TOTALTIME entry */
    for (i = 0; i < 1024; i++) {
        for (nc = nclists[i]; nc; nc = nc->next) {
			TOTALTIMEEntry *ae = NULL;
			TOTALTIMEPrefiX *ac = NULL;

			ae = getTOTALTIMEEntry(nc);
			if (!ae)
				continue;

			/* If there are no entries in the list and the configuration is set to the default
			 * value, there is no need to save it to the db. */
			if (ae->ajnicks == 0 && ae->timeall == DefTOTALTIMEFlags) {
				/* Delete the entry first since it s just eating up space... */
				moduleDelData(&nc->moduleData, ModDataKey);
				deleteTOTALTIMEEntry(ae);
				continue;
			}

			new_write_db_entry("ncd", dbptr, "%s", nc->display);
			new_write_db_entry("flgs", dbptr, "%d", ae->timeall);

			if (ae->ajnicks > 0) {
				for (ac = ae->nicks; ac; ac = ac->next) {
					new_write_db_entry("che", dbptr, "%s %s", ac->nick, (ac->timeall ? ac->timeall : ""));
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
		{{"TOTALTIME", {{PARAM_STRING, PARAM_RELOAD, &TOTALTIME}}}},
	};

	if (TOTALTIME)
	free(TOTALTIME);
	TOTALTIME = NULL;

	for (i = 0; i < 1; i++)
		moduleGetConfigDirective(confvalues[i]);
	  if (!TOTALTIME)
		TOTALTIME = sstrdup(DefTOTALTIME);
	if (!TOTALTIMEDB)
		TOTALTIMEDB = sstrdup(DefTOTALTIMEDB);

	if (debug)
		alog ("[ns_totaltime] debug: TOTALTIMEDB set to %s", TOTALTIMEDB);
}


/**
 * Upon /os reload call the routines for reloading the configuration directives
 **/
int reload_config(int argc, char **argv) {
	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			alog("[ns_totaltime]: Reloading configuration directives...");
			load_config();
		}
	}
	return MOD_CONT;
}


/* ------------------------------------------------------------------------------- */


/* EOF */

