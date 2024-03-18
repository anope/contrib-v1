 /**
  * cs_recordcount.c: Anope Chanserv Module for Users Record Count
  *
  * Copyright (C) 2009 Andrei Rosseti <mdx@unsignal.com>
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  *
  **/
#include "module.h"

#define AUTHOR "mdx"
#define VERSION "$Id: cs_recordcount.c 1 2009-01-28 23:45:22 mdx $"

char *DataRCount = "recordcount";
char *DataTimeRCount = "t_recordcount";
char *DbRecordCount = "cs_recordcount.db";

void mLoadData(void);

int do_save(int argc, char **argv);
int do_backup(int argc, char **argv);
void save_recordcount_db(void);

int do_record_info(User *u);

int count_join(int argc, char **argv);


int AnopeInit(int argc, char **argv)
{
    Command *c;
    EvtHook *hook = NULL;
    
    int status;
        
    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
	
    c = createCommand("Info", do_record_info, NULL, -1, -1, -1, -1, -1);
    status = moduleAddCommand(CHANSERV, c, MOD_TAIL);
    
    hook = createEventHook(EVENT_JOIN_CHANNEL, count_join);
    status = moduleAddEventHook(hook);
    
    hook = createEventHook(EVENT_DB_SAVING, do_save);
    status = moduleAddEventHook(hook); 
    
    hook = createEventHook(EVENT_DB_BACKUP, do_backup);
    status = moduleAddEventHook(hook);    
    
    if (debug)
        alog("[cs_recordcount] Loading database...");
	
    mLoadData();
    
    alog("cs_recordcount loaded"); 
    
    return MOD_CONT;
}

void AnopeFini(void)
{
    char *av[1];

    av[0] = sstrdup(EVENT_START);
    do_save(1, av);
    
    free(av[0]);
}

int do_backup(int argc, char **argv)
{
    ModuleDatabaseBackup(DbRecordCount);
                            
    return MOD_CONT;
}


void mLoadData(void)
{
    FILE *fp;

    ChannelInfo *ci = NULL;
	
    char *chan = NULL;
    char *record  = NULL;
    char *time    = NULL;
    char buffer[2000];

    if ((fp = fopen(DbRecordCount, "r")) == NULL) {

      	alog("cs_recordcount: ERROR: Unable to open the database.");			 
	
    } else 
    {
	while (fgets(buffer, 1500, fp)) 
	{
	    chan   = myStrGetToken(buffer, ' ', 0);
	    record = myStrGetToken(buffer, ' ', 1);
	    time   = myStrGetTokenRemainder(buffer, ' ', 2);
	    
	    if (chan) 
	    {
	    
		if (record)
		{	
			if (time)
			{
				if ((ci = cs_findchan(chan))) {
				
					moduleAddData(&ci->moduleData, DataRCount, record);
					moduleAddData(&ci->moduleData, DataTimeRCount, time);
				}
				
				free(time);
			}
			
			free(record);
		}
		
		free(chan);
	    }
	    
	}
		alog("cs_recordcount: success: data loaded successfull.");			 
	
	fclose(fp);
    }
   
}

void save_recordcount_db(void)
{
	ChannelInfo *ci = NULL;
	 FILE *fp;

	 char *record = NULL;
	 char *time   = NULL;
 	 int i = 0;
		
	if ((fp = fopen(DbRecordCount, "w")) == NULL) {

		alog("cs_recordcount: ERROR: Unable to open the database.");
		
		anope_cmd_global(s_OperServ,
                		 "cs_recordcount: ERROR: Unable to open the database!");

	} else 
	{

		for (i = 0; i < 256; i++) {

		    for (ci = chanlists[i]; ci; ci = ci->next) {
        		if ((record = moduleGetData(&ci->moduleData, DataRCount)) && (time = moduleGetData(&ci->moduleData, DataTimeRCount))) {
        		    fprintf(fp, "%s %s %s", ci->name, record, time);
			    free(record);
			    free(time);
        		}
		    }

		}

		fclose(fp);		
	}
}

int do_save(int argc, char **argv)
{
	if ((argc >= 1) && (!stricmp(argv[0], EVENT_STOP)))
                save_recordcount_db();
		
	  return MOD_CONT;
}


int count_join(int argc, char **argv) 
{
	char buf[BUFSIZE];
	char *db_rcount;
	int recordcount = 0;
	int usercount   = 0;
	
	User *u;
	ChannelInfo *ci;
	
	u = finduser(argv[1]);

        if (argc != 3)
                return MOD_CONT;

        if (stricmp(argv[0], EVENT_STOP))
                return MOD_CONT;
	
	if (argc == 3) 
	{
			
		if ((ci = cs_findchan(argv[2]))) 
		{

			usercount = ci->c->usercount;


			if ((db_rcount = moduleGetData(&ci->moduleData, DataRCount)))
			{
				recordcount = atoi(db_rcount);
				free(db_rcount);
			}


			if (usercount > recordcount)
			{
				recordcount 	= usercount;

				snprintf(buf, BUFSIZE, "%d", recordcount);

				moduleAddData(&ci->moduleData, "recordcount", buf);				

				snprintf(buf, BUFSIZE - 1, "%ld", (long int) time(NULL));
				moduleAddData(&ci->moduleData, DataTimeRCount, buf);
			}

		}

		return MOD_CONT;
		
	}
	
	return MOD_CONT;
}

int do_record_info(User * u) {

    char timebuf[64];
    char *recordcount;
    
    struct tm *tm;
    time_t t_recordcount;
   
    char *chan = NULL;
    char *param = NULL;
    char *text = NULL;
    
    ChannelInfo *ci;
    
    text = moduleGetLastBuffer();
    
    chan  = myStrGetToken(text, ' ', 0);
    param = myStrGetToken(text, ' ', 1);
    
    if (chan) 
    {
    	if ((ci = cs_findchan(chan))) 
    	{
            if (param)
            {
	        if ((stricmp(param, "ALL") == 0) || (check_access(u, ci, CA_INFO) || is_services_admin(u)))
	        {
		    if ((recordcount = moduleGetData(&ci->moduleData, DataRCount))) 
		    {		
			t_recordcount = atol(moduleGetData(&ci->moduleData, DataTimeRCount));
			
			tm     = localtime(&t_recordcount);
			
			strftime_lang(timebuf, sizeof(timebuf), u, STRFTIME_DATE_TIME_FORMAT, tm);
            		notice_user(s_ChanServ, u, "	Users Peak: \2%s\2 in %s", recordcount, timebuf);
			
                        free(recordcount);
                    }
                }
                
                free(param);
	    }
	}
	
	free(chan);
    }
	    
    return MOD_CONT; 
}

