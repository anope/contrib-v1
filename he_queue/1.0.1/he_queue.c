/* he_queue: HelpServ Queue Manager
   Written by Sheepeep (irc.arloria.net)
   Feel free to drop by for any sensible reason
   I'd love to know who's using this, who finds
   It useful, and how people would improve it.

   You are free to use and modify this module
   for any legal purpose so long as this header
   remains intact.

   Thanks to Guy Soffer for being around when
   I needed help, the guy is invaluable.
*/

#include "module.h"
#define AUTHOR "Sheepeep"
#define VERSION "1.0.1"

#define LANG_STRING_TOTAL 64

/* Main help */
#define LANG_HELP_MAIN_MANAGERS 0
#define LANG_HELP_MAIN_OPERATORS 1
#define LANG_HELP_MAIN_USERS 2

/* User help syntax */
#define LANG_HELP_HELPME_SYNTAX 3
#define LANG_HELP_CLOSE_SYNTAX 4
#define LANG_HELP_INFO_SYNTAX 5

/* User help description */
#define LANG_HELP_HELPME_DESC 6
#define LANG_HELP_CLOSE_DESC 7
#define LANG_HELP_INFO_DESC 8

/* Staff help syntax */
#define LANG_STAFFHELP_CLOSE_SYNTAX 9
#define LANG_STAFFHELP_TAKE_SYNTAX 10
#define LANG_STAFFHELP_NEXT_SYNTAX 11
#define LANG_STAFFHELP_VIEW_SYNTAX 12
#define LANG_STAFFHELP_INFO_SYNTAX 13
#define LANG_STAFFHELP_REASSIGN_SYNTAX 14
#define LANG_STAFFHELP_CONFIG_SYNTAX 15

/* Staff help description */
#define LANG_STAFFHELP_CLOSE_DESC 16
#define LANG_STAFFHELP_TAKE_DESC 17
#define LANG_STAFFHELP_NEXT_DESC 18
#define LANG_STAFFHELP_VIEW_DESC 19
#define LANG_STAFFHELP_INFO_DESC 20
#define LANG_STAFFHELP_REASSIGN_DESC 21
#define LANG_STAFFHELP_CONFIG_DESC 22

/* Helpme notices */
#define LANG_HELPME_REPEATENTRY 23
#define LANG_HELPME_REPLY 24
#define LANG_HELPME_UPDATE 25
#define LANG_HELPME_MAXUPDATES 26

/* Close notices */
#define LANG_CLOSE_REPLY 27
#define LANG_CLOSE_NOTFOUND 28
#define LANG_CLOSE_NOTHELPER 29

/* Take notices */
#define LANG_TAKE_USERREPLY 30
#define LANG_TAKE_STAFFREPLY 31
#define LANG_TAKE_NOTFOUNDREPLY 32
#define LANG_TAKE_NOQUEUE 33
#define LANG_TAKE_REPEATPICKUP 34

/* View notices */
#define LANG_VIEW_BEGIN 35
#define LANG_VIEW_DATA 36
#define LANG_VIEW_NOUSERS 37
#define LANG_VIEW_END 38

/* Info notices */
#define LANG_INFO_BEGIN 39
#define LANG_INFO_OPEN 40
#define LANG_INFO_UPDATE 41
#define LANG_INFO_HELPER 42
#define LANG_INFO_MSG 43
#define LANG_INFO_NOQUEUE 44
#define LANG_INFO_NOTFOUND 45

/* Reassign notices */
#define LANG_REASSIGN_STAFFREPLY 46
#define LANG_REASSIGN_USERREPLY 47
#define LANG_REASSIGN_NOUSERSPECIFIED 48
#define LANG_REASSIGN_NOHELPERSPECIFIED 49
#define LANG_REASSIGN_NOHELPER 50
#define LANG_REASSIGN_NOTHELPING 51
#define LANG_REASSIGN_NOTASSIGNED 52
#define LANG_REASSIGN_REPEATASSIGN 53

/* Config notices */
#define LANG_CONFIG_BEGIN 54
#define LANG_CONFIG_VOICEONPICKUP 55
#define LANG_CONFIG_HELPCHANNEL 56
#define LANG_CONFIG_EVENTS 57
#define LANG_CONFIG_HELPEXTERNAL 58

/* General notices */
#define LANG_HELP_NOEXTERNALREPLY 59
#define LANG_HELP_NOACCESS 60

/* Event notices */
#define LANG_HELPME_CHANGE_NICK 61
#define LANG_HELPME_CHANGE_HELPERNICK 62

/* Separator */
#define LANG_SEPARATOR 63


int load_config();

void help_get_langs();

void he_help_main(User *u);
int he_help_helpme(User *u);
int he_help_close(User *u);
int he_help_take(User *u);
int he_help_next(User *u);
int he_help_view(User *u);
int he_help_info(User *u);
int he_help_reassign(User *u);
int he_help_config(User *u);

int help_perm_regular(User *u);
int help_perm_operator(User *u);
int help_perm_manager(User *u);

int help_helpme_ticket(User *u);
int help_helpme_take(User *u);
int help_helpme_close(User *u);
int help_helpme_reassign(User *u);

int new_ticket(User *u, char *ticketmsg);
int take_ticket(User *u, char *ticketbuffer);
int close_ticket(User *u, char *ticketname);
int view_tickets(User *u);
int view_user(User *u);
int reassign_ticket(User *u, char *ticketmsg);
int helpserv_config(User *u);

int he_hook_connect(int count, char **av);
int he_hook_join(int count, char **av);
int he_hook_nick(int count, char **av);
int he_hook_quit(int count, char **av);

int he_new_id(User *u);

int HelpExternal = 0;
int HelpServEventHooks = 0;
int HelpServVoiceOnPickup = 0;
int HelpServJoinChannel = 0;

char *HelpServOutputChannel;
char *HelpServManagers;
char *HelpChannel;
char *HelpServMinimumLevel;

unsigned int conn_yday = 1;
unsigned int conn_id = 1;

typedef struct Helpqueue {
   char c_id[18];
   char nick[32];
   time_t start;
   time_t lastupdate;
   char msg[5][450];
   char helper[32];
   char helper_c_id[18];
   int messages;
   struct Helpqueue *next;
} Helpqueue;

struct Helpqueue *hqueue;

int AnopeInit(int argc, char **argv) {
   Command *c;
   EvtHook *hook;
   moduleAddAuthor(AUTHOR);
   moduleAddVersion(VERSION);

   if(load_config() != MOD_CONT) {
     return MOD_STOP;
   }
   c = createCommand("helpme", help_helpme_ticket, NULL, -1, -1, -1, -1, -1);
   moduleAddHelp(c, he_help_helpme);
   moduleAddCommand(HELPSERV, c, MOD_HEAD);

   c = createCommand("close", help_helpme_close, NULL, -1, -1, -1, -1, -1);
   moduleAddCommand(HELPSERV, c, MOD_HEAD);
   moduleAddHelp(c, he_help_close);

   c = createCommand("take", help_helpme_take, help_perm_operator, -1, -1, -1, -1, -1);
   moduleAddCommand(HELPSERV, c, MOD_HEAD);
   moduleAddHelp(c, he_help_take);

   c = createCommand("next", help_helpme_take, help_perm_operator, -1, -1, -1, -1, -1);
   moduleAddCommand(HELPSERV, c, MOD_HEAD);
   moduleAddHelp(c, he_help_next);

   c = createCommand("view", view_tickets, help_perm_operator, -1, -1, -1, -1, -1);
   moduleAddCommand(HELPSERV, c, MOD_HEAD);
   moduleAddHelp(c, he_help_view);

   c = createCommand("info", view_user, help_perm_operator, -1, -1, -1, -1, -1);
   moduleAddCommand(HELPSERV, c, MOD_HEAD);
   moduleAddHelp(c, he_help_info);

   c = createCommand("reassign", help_helpme_reassign, help_perm_manager, -1, -1, -1, -1, -1);
   moduleAddCommand(HELPSERV, c, MOD_HEAD);
   moduleAddHelp(c, he_help_reassign);

   c = createCommand("config", helpserv_config, help_perm_manager, -1, -1, -1, -1, -1);
   moduleAddCommand(HELPSERV, c, MOD_HEAD);
   moduleAddHelp(c, he_help_config);

   moduleSetHelpHelp(he_help_main);

   if(HelpServJoinChannel) {
      anope_cmd_join(s_HelpServ, HelpChannel, time(NULL));
   }
   if(HelpServEventHooks) {
      hook = createEventHook(EVENT_CHANGE_NICK, he_hook_nick);
      if(moduleAddEventHook(hook) != MOD_ERR_OK) {
         alog("Warning: [he_queue] Couldn't hook EVENT_CHANGE_NICK correctly. HelpServ will *not* be able to track nicknames with open help requests");
      }
      hook = createEventHook(EVENT_NEWNICK, he_hook_connect);
      if(moduleAddEventHook(hook) != MOD_ERR_OK) {
         alog("Warning: [he_queue] Couldn't hook EVENT_NEWNICK correctly. HelpServ will *not* be able to track nicknames with open help requests");
      }

      hook = createEventHook(EVENT_JOIN_CHANNEL, he_hook_join);
      if(moduleAddEventHook(hook) != MOD_ERR_OK) {
         alog("Warning: [he_queue] Couldn't hook EVENT_JOIN_CHANNEL correctly. HelpServ will *not* automatically assign tickets to users");
      }

      hook = createEventHook(EVENT_USER_LOGOFF, he_hook_quit);
      if(moduleAddEventHook(hook) != MOD_ERR_OK) {
         alog("Warning: [he_queue] Couldn't hook EVENT_USER_LOGOFF correctly. HelpServ will *not* automatically close tickets when a user leaves IRC");
      }

      hook = createEventHook(EVENT_PART_CHANNEL, he_hook_quit);
      if(moduleAddEventHook(hook) != MOD_ERR_OK) {
         alog("Warning: [he_queue] Couldn't hook EVENT_PART_CHANNEL correctly. HelpServ will *not* automatically close tickets when a user leaves the help channel");
      }

      hook = createEventHook(EVENT_CHAN_KICK, he_hook_quit);
      if(moduleAddEventHook(hook) != MOD_ERR_OK) {
         alog("Warning: [he_queue] Couldn't hook EVENT_CHAN_KICK correctly. HelpServ will *not* automatically close tickets when a helper kicks the user from the help channel");
      }
   }
   else
   {
      alog("Warning: [he_queue] Starting with event hooks disabled. Enable HelpServEventHooks in your services configuration file to re-enable.");
   }

   help_get_langs();
   return MOD_CONT;
}

void AnopeFini() {
   moduleEventDelHook("EVENT_JOIN_CHANNEL");
   moduleEventDelHook("EVENT_USER_LOGOFF");
   moduleEventDelHook("EVENT_CHAN_KICK");
   moduleEventDelHook("EVENT_PART_CHANNEL");
   moduleEventDelHook("EVENT_NEWNICK");
   moduleEventDelHook("EVENT_CHANGE_NICK");

   if(HelpServJoinChannel) {
      anope_cmd_part(s_HelpServ, HelpChannel, NULL);
   }

   if(hqueue) {
      free(hqueue);
   }
   if(HelpServOutputChannel) {
      free(HelpServOutputChannel);
   }
   if(HelpServManagers) {
      free(HelpServManagers);
   }
   if(HelpServMinimumLevel) {
      free(HelpServMinimumLevel);
   }
}

int load_config() {
   int i;
   Directive config[][1] = {
      {{"HelpChannel", {{PARAM_STRING, PARAM_RELOAD, &HelpChannel}}}},
      {{"HelpServMinimumLevel", {{PARAM_STRING, PARAM_RELOAD, &HelpServMinimumLevel}}}},
      {{"HelpServManagers", {{PARAM_STRING, PARAM_RELOAD, &HelpServManagers}}}},
      {{"HelpExternal", {{PARAM_SET, PARAM_RELOAD, &HelpExternal}}}},
      {{"HelpServJoinChannel", {{PARAM_SET, PARAM_RELOAD, &HelpServJoinChannel}}}},
      {{"HelpServVoiceOnPickup", {{PARAM_SET, PARAM_RELOAD, &HelpServVoiceOnPickup}}}},
      {{"HelpServEventHooks", {{PARAM_SET, PARAM_RELOAD, &HelpServEventHooks}}}}
   };
   for (i = 0; i < 7; i++) {
      moduleGetConfigDirective(config[i]);
   }

   if(!HelpChannel) {
      alog("HelpServ: No help channel specified. Loading will not continue. %s",HelpChannel);
      return MOD_STOP;
   }

   if(!HelpServMinimumLevel) {
      alog("HelpServ: No HelpServMinimumLevel specified. Loading will not continue");
      return MOD_STOP;
   }
   return MOD_CONT;
}


void he_help_main(User *u) {
   if(help_perm_manager(u)) {
      moduleNoticeLang(s_HelpServ, u, LANG_HELP_MAIN_MANAGERS);
   }
   else if(help_perm_operator(u)) {
      moduleNoticeLang(s_HelpServ, u, LANG_HELP_MAIN_OPERATORS);
   }
   else {
      moduleNoticeLang(s_HelpServ, u, LANG_HELP_MAIN_USERS);
   }
}

int he_help_helpme(User *u) {
   moduleNoticeLang(s_HelpServ, u, LANG_HELP_HELPME_SYNTAX);
   moduleNoticeLang(s_HelpServ, u, LANG_SEPARATOR);
   moduleNoticeLang(s_HelpServ, u, LANG_HELP_HELPME_DESC);
   return MOD_CONT;
}

int he_help_close(User *u) {
   moduleNoticeLang(s_HelpServ, u, LANG_HELP_CLOSE_SYNTAX);
   moduleNoticeLang(s_HelpServ, u, LANG_SEPARATOR);
   moduleNoticeLang(s_HelpServ, u, LANG_HELP_CLOSE_DESC);
   return MOD_CONT;
}

int he_help_take(User *u) {
   if(help_perm_operator(u)) {
      moduleNoticeLang(s_HelpServ, u, LANG_STAFFHELP_TAKE_SYNTAX);
      moduleNoticeLang(s_HelpServ, u, LANG_SEPARATOR);
      moduleNoticeLang(s_HelpServ, u, LANG_STAFFHELP_TAKE_DESC);
   }
   else
   {
      moduleNoticeLang(s_HelpServ, u, LANG_HELP_NOACCESS);
   }
   return MOD_CONT;
}

int he_help_next(User *u) {
   if(help_perm_operator(u)) {
      moduleNoticeLang(s_HelpServ, u, LANG_STAFFHELP_NEXT_SYNTAX);
      moduleNoticeLang(s_HelpServ, u, LANG_SEPARATOR);
      moduleNoticeLang(s_HelpServ, u, LANG_STAFFHELP_NEXT_DESC);
   }
   else
   {
      moduleNoticeLang(s_HelpServ, u, LANG_HELP_NOACCESS);
   }
   return MOD_CONT;
}

int he_help_view(User *u) {
   if(help_perm_operator(u)) {
      moduleNoticeLang(s_HelpServ, u, LANG_STAFFHELP_VIEW_SYNTAX);
      moduleNoticeLang(s_HelpServ, u, LANG_SEPARATOR);
      moduleNoticeLang(s_HelpServ, u, LANG_STAFFHELP_VIEW_DESC);
   }
   else
   {
      moduleNoticeLang(s_HelpServ, u, LANG_HELP_NOACCESS);
   }
   return MOD_CONT;
}

int he_help_info(User *u) {
   if(help_perm_operator(u)) {
      moduleNoticeLang(s_HelpServ, u, LANG_STAFFHELP_INFO_SYNTAX);
      moduleNoticeLang(s_HelpServ, u, LANG_SEPARATOR);
      moduleNoticeLang(s_HelpServ, u, LANG_STAFFHELP_INFO_DESC);
   }
   else
   {
      moduleNoticeLang(s_HelpServ, u, LANG_HELP_NOACCESS);
   }
   return MOD_CONT;
}

int he_help_reassign(User *u) {
   if(help_perm_manager(u)) {
      moduleNoticeLang(s_HelpServ, u, LANG_STAFFHELP_REASSIGN_SYNTAX);
      moduleNoticeLang(s_HelpServ, u, LANG_SEPARATOR);
      moduleNoticeLang(s_HelpServ, u, LANG_STAFFHELP_REASSIGN_DESC);
   }
   else
   {
      moduleNoticeLang(s_HelpServ, u, LANG_HELP_NOACCESS);
   }
   return MOD_CONT;
}

int he_help_config(User *u) {
   if(help_perm_operator(u)) {
      moduleNoticeLang(s_HelpServ, u, LANG_STAFFHELP_CONFIG_SYNTAX);
      moduleNoticeLang(s_HelpServ, u, LANG_SEPARATOR);
      moduleNoticeLang(s_HelpServ, u, LANG_STAFFHELP_CONFIG_DESC);
   }
   else
   {
      moduleNoticeLang(s_HelpServ, u, LANG_HELP_NOACCESS);
   }
   return MOD_CONT;
}

void help_get_langs() {
   char *texts[] = {
      /* 0 : LANG_HELP_MAIN_MANAGERS */
      " \n"
      "           \002Help and Support Commands\n"
      "    HELPME     Send a request for help to help staff\n"
      "    CLOSE      Close an active help ticket\n"
      "    TAKE       Take a specific user as a help request\n"
      "    NEXT       Take the next user in the queue\n"
      "    VIEW       See the current HelpServ queue\n"
      "    INFO       View information on a user in the queue\n"
      "    REASSIGN   Change the helper for a help ticket\n"
      "    CONFIG     View the current HelpServ configuration",
      /* 1 : LANG_HELP_MAIN_OPERS */
      " \n"
      "           \002Help and Support Commands\n"
      "    HELPME     Send a request for help to help staff\n"
      "    CLOSE      Close an active help ticket\n"
      "    TAKE       Take a specific user as a help request\n"
      "    NEXT       Take the next user in the queue\n"
      "    VIEW       See the current HelpServ queue\n"
      "    INFO       View information on a user in the queue\n",
      /* 2 : LANG_HELP_MAIN_USERS */
      " \n"
      "           \002Help and Support Commands\n"
      "    HELPME     Send a request for help to help staff\n"
      "    CLOSE      Closes your open help ticket\n",
      /* 3 : LANG_HELP_HELPME_SYNTAX */
      "Syntax: \002HELPME\002",
      /* 4 : LANG_HELP_CLOSE_SYNTAX */
      "Syntax: \002CLOSE\002",
      /* 5 : LANG_HELP_INFO_SYNTAX */
      "Syntax: \002INFO\002",
      /* 6 : LANG_HELP_HELPME_DESC */
      "Sends a request for help to designated help staff\n"
      "You will then be invited to join the help channel",
      /* 7 : LANG_HELP_CLOSE_DESC */
      "Closes an active help ticket with the help staff,\n"
      "indicating that no further assistance is required.",
      /* 8 : LANG_HELP_INFO_DESC */
      "Shows information about your open ticket.",
      /* 9 : LANG_STAFFHELP_CLOSE_SYNTAX */
      "Syntax: \002CLOSE \37username\37",
      /* 10 : LANG_STAFFHELP_TAKE_SYNTAX */
      "Syntax: \002TAKE [\37username\37]",
      /* 11 : LANG_STAFFHELP_NEXT_SYNTAX */
      "Syntax: \002NEXT\002",
      /* 12 : LANG_STAFFHELP_VIEW_SYNTAX */
      "Syntax: \002VIEW\002",
      /* 13 : LANG_STAFFHELP_INFO_SYNTAX */
      "Syntax: \002INFO \37username\37\002",
      /* 14 : LANG_STAFFHELP_REASSIGN_SYNTAX */
      "Syntax: \002REASSIGN \37username\37 \37helper\37\002",
      /* 15 : LANG_STAFFHELP_CONFIG_SYNTAX */
      "Syntax: \002CONFIG\002",
      /* 16 : LANG_STAFFHELP_CLOSE_DESC */
      "Close a user's help ticket, indicating that\n"
      "they have been helped and no longer require assistance.",
      /* 17 : LANG_STAFFHELP_TAKE_DESC */
      "Pick up a specific user in the queue\n"
      "This command will also allow you to reassign a user in\n"
      "the queue from one helper to yourself. If no nickname\n"
      "is given, this will behave like the NEXT command.",
      /* 18 : LANG_STAFFHELP_NEXT_DESC */
      "Pick up the next user in the queue, assigning you as\n"
      "their help helper. If a nickname is given, this\n"
      "will behave in the same way as the TAKE command.",
      /* 19 : LANG_STAFFHELP_VIEW_DESC */
      "View all of the current tickets in the queue, with\n"
      "information on when the ticket was opened and who\n"
      "they are currently assigned to.",
      /* 20 : LANG_STAFFHELP_INFO_DESC */
      "View all information regarding a user in the help queue\n"
      "Such as messages they have sent to HelpServ.",
      /* 21 : LANG_STAFFHELP_REASSIGN_DESC */
      "Forces a helper to pick up a ticket from another user.\n"
      "Available to HelpServ Managers only.",
      /* 22 : LANG_STAFFHELP_CONFIG_DESC */
      "View the current HelpServ configuration.\n"
      "Available to HelpServ Managers only.",
      /* 23 : LANG_HELPME_REPEATENTRY */
      "You have already opened a help ticket. To provide more information about an existing ticket, use /msg %s HELPME \37your message\37.",
      /* 24 : LANG_HELPME_REPLY */
      "Hello %s, welcome to %s. Your help ticket has been added to the queue and you will be assigned a helper shortly.\n"
      "If there are other users who require assistance, you may help the help staff by updating your help ticket with information about your query.\n"
      "To update your ticket with more information, use /msg %s HELPME \37your message\37.",
      /* 25 : LANG_HELME_UPDATE */
      "Thank you for updating your help ticket with the information you have provided.",
      /* 26 : LANG_HELPME_MAXUPDATES */
      "You have already updated your help ticket the maximum number of five times. No new information about your ticket can be added.",
      /* 27 : LANG_CLOSE_REPLY */
      "The help ticket has now been closed.",
      /* 28 : LANG_CLOSE_NOTFOUND */
      "Could not find an open help request from %s.",
      /* 29 : LANG_CLOSE_NOTHELPER */
      "You must be the user's active helper in order to close their ticket for them.",
      /* 30 : LANG_TAKE_USERREPLY */
      "You have been assigned the staff member of \002%s\002 to help you.\n"
      "Please message %s with the nature of your problem or request.",
      /* 31 : LANG_TAKE_STAFFREPLY */
      "You have picked up the help ticket from %s.",
      /* 32 : LANG_TAKE_NOTFOUNDREPLY */
      "User with the nickname %s was not found. Cannot pick up %s.",
      /* 33 : LANG_TAKE_NOQUEUE */
      "There are currently no users who require assistance.",
      /* 34 : LANG_TAKE_REPEATPICKUP */
      "You are already the assigned helper to %s.",
      /* 35 : LANG_VIEW_BEGIN */
      "Showing all users with open help requests.",
      /* 36 : LANG_VIEW_DATA */
      "Ticket from %s (Helper: %s) opened %s.",
      /* 37 : LANG_VIEW_NOUSERS */
      "There are currently no users in the help queue.",
      /* 38 : LANG_VIEW_END */
      "%d tickets were found.",
      /* 39 : LANG_INFO_BEGIN */
      "Showing HelpServ information for user %s (%s).",
      /* 40 :  LANG_INFO_OPEN */
      "Opened:       %s",
      /* 41 : LANG_INFO_UPDATE */
      "Last update:  %s",
      /* 42 : LANG_INFO_HELPER */
      "Assigned to:  %s",
      /* 43 : LANG_INFO_MSG */
      "Message: %s",
      /* 44 : LANG_INFO_NOQUEUE */
      "There are currently no users in the queue.",
      /* 45 : LANG_INFO_NOTFOUND */
      "User with the nickname %s was not found in the help queue.",
      /* 46 : LANG_REASSIGN_STAFFREPLY */
      "%s has been reassigned to %s for help.",
      /* 47 : LANG_REASSIGN_USERREPLY */
      "Your ticket has been reassigned. Your new helper is %s.",
      /* 48 : LANG_REASSIGN_NOUSERSPECIFIED */
      "You did not provide the name of a user with an active help ticket.",
      /* 49 : LANG_REASSIGN_NOHELPERSPECIFIED */
      "You did not provide the name of a helper to reassign the ticket to.",
      /* 50 : LANG_REASSIGN_NOHELPER */
      "Could not find a helper with the name %s.",
      /* 51 : LANG_REASSIGN_NOTHELPING */
      "%s does not appear to be in the help channel.",
      /* 52 : LANG_REASSIGN_NOTASSIGNED */
      "%s has not been assigned a helper, or the helper is no longer available.",
      /* 53 : LANG_REASSIGN_REPEATASSIGN */
      "%s is already the active helper for %s",
      /* 54 : LANG_CONFIG_BEGIN */
      "Showing HelpServ configuration",
      /* 55 : LANG_CONFIG_VOICEONPICKUP */
      "Voice on pickup:      %s",
      /* 56 : LANG_CONFIG_HELPCHANNEL */
      "Help channel:         %s",
      /* 57 : LANG_CONFIG_EVENTS */
      "Internal events:      %s",
      /* 58 : LANG_CONFIG_HELPEXTERNAL */
      "Help externally:      %s",
      /* 59 : LANG_HELP_NOEXTERNALREPLY */
      "This command cannot be used externally. Please join %s in order to use this command.",
      /* 60 : LANG_HELP_NOACCESS */
      "You do not have permission to use this command.",
      /* 61 : LANG_HELPME_CHANGE_NICK */
      "User with ID %s has changed their nickname from %s to %s.",
      /* 62 : LANG_HELPME_CHANGE_HELPERNICK */
      "Your helper, %s, has changed their nickname to %s.",
      /* 63 : LANG_SEPARATOR */
      " \n"
   };
   moduleInsertLanguage(LANG_EN_US, LANG_STRING_TOTAL, texts);
}

int help_helpme_ticket(User *u) {
   char *msg = moduleGetLastBuffer();
   new_ticket(u, msg);
   return MOD_CONT;
}

int help_helpme_close(User *u) {
   char *msg = moduleGetLastBuffer();
   close_ticket(u, msg);
   return MOD_CONT;
}

int help_helpme_take(User *u) {
   char *msg = moduleGetLastBuffer();
   take_ticket(u, msg);
   return MOD_CONT;
}

int help_helpme_reassign(User *u) {
   char *msg = moduleGetLastBuffer();
   reassign_ticket(u, msg);
   return MOD_CONT;
}

int new_ticket(User *u, char *ticketmsg) {
   Helpqueue *p = hqueue;
   Channel *help;
   char *he_id = NULL;

   help = findchan(HelpChannel);

   if(!HelpExternal && !is_on_chan(help, u)) {
      moduleNoticeLang(s_HelpServ, u, LANG_HELP_NOEXTERNALREPLY, HelpChannel);
      return MOD_CONT;
   }

   he_id = moduleGetData(&u->moduleData, "c_id");
   if(he_id == NULL) {
      alog("Recreating ID for %s", u->nick);
      if(he_new_id(u) != MOD_ERR_OK) {
         return MOD_CONT;
      }
      he_id = moduleGetData(&u->moduleData, "c_id");
   }

   if(hqueue == NULL) {
      hqueue = smalloc(sizeof(Helpqueue));
      p = hqueue;
      p->next = NULL;
      *(p->nick) = 0;
      *(p->c_id) = 0;
   }
   else
   {
      while(p->next != NULL) {
         if(strcmp(p->nick,u->nick) == 0) {
            /* This is an update message */
            break;
         }
         else
         {
            p = p->next;
         }
      }

      /* Handle said update */
      if(strcmp(p->nick,u->nick) == 0) {
         if(ticketmsg && strlen(ticketmsg) > 0) {
            if(p->messages >= 5) {
               moduleNoticeLang(s_HelpServ, u, LANG_HELPME_MAXUPDATES);
            }
            else
            {
               strncpy(p->msg[p->messages], ticketmsg, 450);
               p->lastupdate = time(NULL);
               p->messages++;
               moduleNoticeLang(s_HelpServ, u, LANG_HELPME_UPDATE);
            }
         }
         return MOD_CONT;
      }
      else
      {
         p->next = smalloc(sizeof(Helpqueue));
         p = p->next;
      }
   }
   p->next = NULL;
   strncpy(p->nick, u->nick, 32);
   p->messages = 0;
   strcpy(p->msg[0],"");
   strcpy(p->c_id, he_id);
   /* Only likely to matter with HelpExternal enabled */
   if(ticketmsg && strlen(ticketmsg) > 0) {
      strncpy(p->msg[0], ticketmsg, 450);
      p->messages++;
   }
   strcpy(p->helper,"");
   strcpy(p->helper_c_id,"");
   p->start = time(NULL);
   p->lastupdate = time(NULL);
   if(ticketmsg) {
     alog("%s has opened a new help ticket (Message: %s)", p->nick, ticketmsg);
   }
   else
   {
      alog("%s has opened a new help ticket (No message)", p->nick);
   }
   moduleNoticeLang(s_HelpServ, u, LANG_HELPME_REPLY, p->nick, HelpExternal ? "our help system" : HelpChannel, s_HelpServ);
   return MOD_CONT;
}

int take_ticket(User * u, char *ticketbuffer) {
   char *ticketname;
   char timebuf[BUFSIZE];
   char *he_id;
   struct tm *tm;
   Channel *help = findchan(HelpChannel);
   Helpqueue *p = hqueue;
   User *ticket_u;

   if(!HelpExternal && !is_on_chan(help, u)) {
      moduleNoticeLang(s_HelpServ, u, LANG_HELP_NOEXTERNALREPLY, HelpChannel);
      return MOD_CONT;
   }

   if(hqueue == NULL) {
      moduleNoticeLang(s_HelpServ, u, LANG_TAKE_NOQUEUE);
      return MOD_CONT;
   }

   he_id = moduleGetData(&u->moduleData, "c_id");
   if(he_id == NULL) {
      alog("Recreating ID for %s", u->nick);
      if(he_new_id(u) != MOD_ERR_OK) {
         return MOD_CONT;
      }
      he_id = moduleGetData(&u->moduleData, "c_id");
   }

   ticketname = myStrGetToken(ticketbuffer, ' ', 0);

   if(!ticketname) {
      ticketname = p->nick;
   }
   else
   {
      while(p->next != NULL) {
         if(stricmp(p->nick, ticketname) == 0) {
            break;
         }
         if(p->next != NULL) {
            p = p->next;
         }
      }
      if(stricmp(p->nick,ticketname) != 0 && strlen(ticketname) > 0) {
         moduleNoticeLang(s_HelpServ, u, LANG_TAKE_NOTFOUNDREPLY);
         free(he_id);
         free(ticketname);
         return MOD_CONT;
      }
   }

   ticket_u = finduser(ticketname);
   if(ticket_u != NULL) {
      if(strcmp(p->helper,"") == 0) {
         strncpy(p->helper, u->nick, 32);
         strcpy(p->helper_c_id, he_id);
         alog("%s has taken the help ticket request from %s", p->helper, p->nick);
      }
      else
      {
         if(strcmp(p->helper,u->nick) == 0)  {
            moduleNoticeLang(s_HelpServ, u, LANG_TAKE_REPEATPICKUP, p->nick);
            free(he_id);
            free(ticketname);
            return MOD_CONT;
         }
         strncpy(p->helper, u->nick, 32);
         strcpy(p->helper_c_id, he_id);
         alog("%s has been reassigned from %s to %s", p->nick, p->helper, p->nick);
      }

      tm = localtime(&p->start);
      strftime_lang(timebuf, sizeof(timebuf), ticket_u, STRFTIME_DATE_TIME_FORMAT, tm);
      anope_cmd_notice_ops(s_HelpServ, HelpChannel, "The ticket opened by %s has been picked up by helper %s (Opened: %s)", p->nick, p->helper, timebuf);
      if(HelpServVoiceOnPickup && is_on_chan(help, ticket_u)) {
         anope_cmd_mode(s_ChanServ, help->name, "%s %s", "+v", p->nick);
      }
      moduleNoticeLang(s_HelpServ, u, LANG_TAKE_STAFFREPLY, ticket_u->nick);
      moduleNoticeLang(s_HelpServ, ticket_u, LANG_TAKE_USERREPLY, u->nick, u->nick);
   }
   else
   {
      moduleNoticeLang(s_HelpServ, u, LANG_TAKE_NOTFOUNDREPLY, ticketname, ticketname);
   }
   free(he_id);
   free(ticketname);
   return MOD_CONT;
}

int close_ticket(User * u, char *ticketname) {
   Helpqueue *p = hqueue;
   Helpqueue *q = hqueue;

   if(!help_perm_operator(u) || (!ticketname && help_perm_operator(u))) {
      ticketname = u->nick;
   }
   if(hqueue == NULL) {
      moduleNoticeLang(s_HelpServ, u, LANG_CLOSE_NOTFOUND, ticketname);
      return MOD_CONT;
   }

   while(p->next != NULL) {
      if(stricmp(p->nick, ticketname) == 0) {
         break;
      }

      if(p->next != NULL) {
         q = p;
         p = p->next;
      }
   }
   if(stricmp(p->nick, ticketname) == 0) {
      if((strcmp(p->nick,ticketname) != 0 && strcmp(p->helper,ticketname) != 0) && !help_perm_manager(u)) {
         moduleNoticeLang(s_HelpServ, u, LANG_CLOSE_NOTHELPER);
         return MOD_CONT;
      }
      if(p == hqueue && p->next == NULL) {
         hqueue = NULL;
      }
      else if(p == hqueue && p->next != NULL) {
         q = p->next;
         free(hqueue);
         hqueue = q;
      }
      else
      {
         q->next = p->next;
         free(p);
      }
      if(!help_perm_operator(u)) {
         alog("%s has closed their help ticket", ticketname);
      }
      else
      {
         alog("%s has closed the ticket from %s", u->nick, ticketname);
      }
      moduleNoticeLang(s_HelpServ, u, LANG_CLOSE_REPLY);
   }
   else
   {
      moduleNoticeLang(s_HelpServ, u, LANG_CLOSE_NOTFOUND, ticketname);
   }
   return MOD_CONT;
}

int view_tickets(User *u) {
   Helpqueue *p = hqueue;
   int count = 0;
   struct tm *tm;
   char timebuf[BUFSIZE];

   if(hqueue == NULL) {
      moduleNoticeLang(s_HelpServ, u, LANG_VIEW_NOUSERS);
      return MOD_CONT;
   }

   moduleNoticeLang(s_HelpServ, u, LANG_VIEW_BEGIN);

   while(p != NULL) {
      count++;
      tm = localtime(&p->start);
      strftime_lang(timebuf, sizeof(timebuf), u, STRFTIME_DATE_TIME_FORMAT, tm);
      if(strlen(p->helper) > 0) {
         moduleNoticeLang(s_HelpServ, u, LANG_VIEW_DATA, p->nick, p->helper, timebuf);
      }
      else
      {
         moduleNoticeLang(s_HelpServ, u, LANG_VIEW_DATA, p->nick, "Not assigned", timebuf);
      }
      p = p->next;
   }
   moduleNoticeLang(s_HelpServ, u, LANG_VIEW_END, count);
   return MOD_CONT;
}

int view_user(User *u) {
   int msgcount = 0;
   char *ticketname;
   char *ticketbuffer = moduleGetLastBuffer();
   char timebuf_open[BUFSIZE];
   char timebuf_update[BUFSIZE];

   struct tm tm;
   struct tm tm_upd;
   Helpqueue *p = hqueue;

   if(hqueue == NULL) {
      moduleNoticeLang(s_HelpServ, u, LANG_INFO_NOQUEUE);
      return MOD_CONT;
   }
   if(!ticketbuffer) {
      moduleNoticeLang(s_HelpServ, u, LANG_STAFFHELP_INFO_SYNTAX);
      return MOD_CONT;
   }
   ticketname = myStrGetToken(ticketbuffer, ' ', 0);
   p = hqueue;

   if(!ticketname) {
      moduleNoticeLang(s_HelpServ, u, LANG_HELP_INFO_SYNTAX);
   }
   while(p->next != NULL) {
      if(stricmp(p->nick,ticketname) == 0) {
         break;
      }
      else
      {
         p = p->next;
      }
   }

   if(stricmp(p->nick,ticketname) == 0) {
      tm = *localtime(&p->start);
      strftime_lang(timebuf_open, sizeof(timebuf_open), u, STRFTIME_DATE_TIME_FORMAT, &tm);
      tm_upd = *localtime(&p->lastupdate);
      strftime_lang(timebuf_update, sizeof(timebuf_update), u, STRFTIME_DATE_TIME_FORMAT, &tm_upd);
      moduleNoticeLang(s_HelpServ, u, LANG_INFO_BEGIN, p->nick, p->c_id);
      moduleNoticeLang(s_HelpServ, u, LANG_INFO_OPEN, timebuf_open);
      moduleNoticeLang(s_HelpServ, u, LANG_INFO_UPDATE, timebuf_update);
      if(strlen(p->helper) > 0) {
         moduleNoticeLang(s_HelpServ, u, LANG_INFO_HELPER, p->helper);
      }
      else
      {
         moduleNoticeLang(s_HelpServ, u, LANG_INFO_HELPER, "Not assigned");
      }
      while(strlen(p->msg[msgcount]) > 0) {
         moduleNoticeLang(s_HelpServ, u, LANG_INFO_MSG, p->msg[msgcount]);
         msgcount++;
      }
   }
   else
   {
      moduleNoticeLang(s_HelpServ, u, LANG_INFO_NOQUEUE, ticketname);
   }
   if(ticketname) {
      free(ticketname);
   }

   return MOD_CONT;
}

int reassign_ticket(User *u, char *msg) {
   char *ticketname;
   char *helpername;
   char *he_id;

   User *ticket_u;
   User *helper;
   struct tm *tm;
   Channel *help = findchan(HelpChannel);
   Helpqueue *p = hqueue;

   char timebuf[BUFSIZE];

   if(hqueue == NULL) {
      moduleNoticeLang(s_HelpServ, u, LANG_TAKE_NOQUEUE);
      return MOD_CONT;
   }

   ticketname = myStrGetToken(msg, ' ', 0);
   helpername = myStrGetToken(msg, ' ', 1);

   if(!ticketname) {
      moduleNoticeLang(s_HelpServ, u, LANG_REASSIGN_NOUSERSPECIFIED);
      moduleNoticeLang(s_HelpServ, u, LANG_STAFFHELP_REASSIGN_SYNTAX);
   }
   else if(!helpername) {
      moduleNoticeLang(s_HelpServ, u, LANG_REASSIGN_NOHELPERSPECIFIED);
      moduleNoticeLang(s_HelpServ, u, LANG_STAFFHELP_REASSIGN_SYNTAX);
   }

   if(!ticketname || !helpername) {
      free(ticketname);
      free(helpername);
      return MOD_CONT;
   }
   helper = finduser(helpername);

   if(!helper) {
      moduleNoticeLang(s_HelpServ, u, LANG_REASSIGN_NOHELPER);
      free(ticketname);
      free(helpername);
      return MOD_CONT;
   }

   if((HelpExternal == 0 && !is_on_chan(help, helper)) || !help_perm_operator(helper)) {
      moduleNoticeLang(s_HelpServ, u, LANG_REASSIGN_NOTHELPING, helper->nick);
      free(ticketname);
      free(helpername);
      return MOD_CONT;
   }
   while(p->next != NULL) {
      if(stricmp(p->nick, ticketname) == 0) {
         break;
      }
      p = p->next;
   }   ticket_u = finduser(ticketname);
   if(stricmp(p->nick,ticketname) != 0 && strlen(ticketname) > 0) {
      moduleNoticeLang(s_HelpServ, u, LANG_CLOSE_NOTFOUND, ticketname);
   }
   else if((ticket_u = finduser(ticketname))) {
      if(strcmp(p->helper,"") == 0) {
         moduleNoticeLang(s_HelpServ, u, LANG_REASSIGN_NOTASSIGNED, ticketname);
      }
      else
      {
         if(stricmp(p->helper,helper->nick) == 0)  {
            moduleNoticeLang(s_HelpServ, u, LANG_REASSIGN_REPEATASSIGN, p->helper, p->nick);
            free(ticketname);
            free(helpername);
            return MOD_CONT;
         }
         alog("%s has been reassigned by %s (From %s to %s)", p->nick, u->nick, p->helper, helper->nick);
         he_id = moduleGetData(&helper->moduleData, "c_id");
         if(!he_id) {
            he_new_id(helper);
            he_id = moduleGetData(&helper->moduleData, "c_id");
            if(!he_id) {
               free(ticketname);
               free(helpername);
               return MOD_CONT;
            }
         }
         strncpy(p->helper, helper->nick, 32);
         strcpy(p->helper_c_id, he_id);

         tm = localtime(&p->start);
         strftime_lang(timebuf, sizeof(timebuf), ticket_u, STRFTIME_DATE_TIME_FORMAT, tm);
         anope_cmd_notice_ops(s_HelpServ, HelpChannel, "The ticket opened by %s has been reassigned to %s by %s (Opened: %s)", p->nick, p->helper, u->nick, timebuf);

         moduleNoticeLang(s_HelpServ, u, LANG_REASSIGN_STAFFREPLY, u->nick, p->helper);
         moduleNoticeLang(s_HelpServ, ticket_u, LANG_REASSIGN_USERREPLY, p->helper);
      }
   }
   else
   {
      moduleNoticeLang(s_HelpServ, u, LANG_TAKE_NOTFOUNDREPLY, ticketname, ticketname);
   }

   free(ticketname);
   free(helpername);
   return MOD_CONT;
}

int helpserv_config(User *u) {
   if(!help_perm_manager(u)) {
      moduleNoticeLang(s_HelpServ, u, LANG_HELP_NOACCESS);
      return MOD_CONT;
   }
   moduleNoticeLang(s_HelpServ, u, LANG_CONFIG_BEGIN);
   moduleNoticeLang(s_HelpServ, u, LANG_CONFIG_VOICEONPICKUP, HelpServVoiceOnPickup ? "Enabled" : "Disabled");
   moduleNoticeLang(s_HelpServ, u, LANG_CONFIG_HELPCHANNEL, HelpChannel);
   moduleNoticeLang(s_HelpServ, u, LANG_CONFIG_EVENTS, HelpServEventHooks ? "Enabled" : "Disabled");
   moduleNoticeLang(s_HelpServ, u, LANG_CONFIG_HELPEXTERNAL, HelpExternal ? "Enabled" : "Disabled");
   return MOD_CONT;
}

int help_perm_regular(User *u) {
   Channel *help = findchan(HelpChannel);

   if(HelpExternal) {
      return 1;
   }
   else
   {
      if(!is_on_chan(help, u)) {
         moduleNoticeLang(s_HelpServ, u, LANG_HELP_NOEXTERNALREPLY, HelpChannel);
         return 0;
      }
      else
      {
         return 1;
      }
   }
}

int help_perm_manager(User *u) {
   /* Services admins are automatically considered HelpServ Managers */
   char *manager;
   char *managers;
   if(!nick_identified(u)) {
      return 0;
   }
   if(is_services_admin(u)) {
      return 1;
   }
   if(HelpServManagers) {
      managers = (char*)smalloc(strlen(HelpServManagers));
      strcpy(managers,HelpServManagers);
      manager = strtok(managers, " ");
      while(manager != NULL) {
         if(stricmp(manager,u->nick) == 0) {
            return 1;
         }
         manager = strtok(NULL, " ");
      }
      free(managers);
   }
   return 0;
}

int help_perm_operator(User *u) {
   /* Services admins are automatically considered HelpServ Operators */
   ChannelInfo *ci;
   ci = cs_findchan(HelpChannel);

   if(!ci) {
      alog("[he_queue] Note: Help channel not found");
   }
   if(!u) {
      alog("[he_queue] Error: help_perm_operator failed sanity check (No user found), function will not continue");
      return 0;
   }
   if(is_services_oper(u)) {
      return 1;
   }
   if(strcmp(HelpServMinimumLevel,"halfop") == 0) {
      if(ircd->halfop) {
         if(check_access(u, ci, CA_AUTOHALFOP)) {
            return 1;
         }
      }
      else
      {
         alog("Warning: [he_queue] HelpServMinimumLevel of HALFOP specified but IRCd does not appear to support mode. Consider setting HelpServMimumLevel of OP");
         if(check_access(u, ci, CA_AUTOOP)) {
            return 1;
         }
      }
   }
   else if(strcmp(HelpServMinimumLevel,"op") == 0) {
      if(check_access(u, ci, CA_AUTOOP)) {
         return 1;
      }
   }
   else if(strcmp(HelpServMinimumLevel,"admin") == 0) {
      if(ircd->admin || ircd->protect) {
         if(check_access(u, ci, CA_AUTOPROTECT)) {
            return 1;
         }
      }
      else
      {
         alog("Warning: [he_queue] HelpServMinimumLevel of ADMIN specified but IRCd does not appear to support mode. Consider setting HelpServMimumLevel of OP");
         if(check_access(u, ci, CA_AUTOOP)) {
            return 1;
         }
      }
   }
   return help_perm_manager(u);
}

int he_hook_join(int count, char **av) {
   /* This argument count may just bite me in the ass one day
      Sooner that than have the module crash. */

   User *u;

   if(count != 3) {
      alog("Warning: [he_queue] Invalid event count in he_hook_join (Count is %d, expected 3. Event will not be processed.", count);
      return MOD_CONT;
   }
   if(stricmp(av[0], EVENT_STOP) == 0) {
      if(stricmp(av[2], HelpChannel) == 0) {
         u = finduser(av[1]);
         if(strlen(u->nick) > 0 && !help_perm_operator(u)) {
            new_ticket(u, "");
         }
      }
   }
   return MOD_CONT;

}

int he_hook_quit(int count, char **av) {
   User *u;
   User *ticket_u;
   Helpqueue *p = hqueue;

   if(hqueue == NULL) {
      return MOD_CONT;
   }

   if(count == 1) {
      /* User has signed off */
      if(!(u = finduser(av[0]))) {
         return MOD_CONT;
      }
   }
   else if(count == 2) {
      /* Kicked from channel */
      if(strcmp(av[1],HelpChannel) == 0) {
         if(!(u = finduser(av[0]))) {
            return MOD_CONT;
         }
      }
      else
      {
         return MOD_CONT;
      }
   }
   else if(count >= 3) {
      /* User has parted channel */
      if((stricmp(av[0], EVENT_STOP) == 0) && strcmp(av[2],HelpChannel) == 0) {
         u = finduser(av[1]);
      }
      else
      {
         return MOD_CONT;
      }
   }
   else
   {
      alog("Unexpected parameter count for quit event handler: %d parameters given", count);
      return MOD_CONT;
   }
   if(!u) {
      return MOD_CONT;
   }
   while(p != NULL) {
      if(strcmp(p->nick,u->nick) == 0) {
         break;
      }
      else if(strcmp(p->helper,u->nick) == 0) {
         ticket_u = finduser(p->nick);
         if(!ticket_u) {
            alog("Found open ticket from %s, but could not find a user online with this name.", p->nick);
            p = p->next;
            continue;
         }
         if(count == 1) {
             anope_cmd_notice_ops(s_HelpServ, HelpChannel, "The ticket opened by %s has been closed because their helper, %s, is no longer connected.", p->nick, p->helper);
         }
         else if(count == 2) {
            anope_cmd_notice_ops(s_HelpServ, HelpChannel, "The ticket opened by %s has been closed because their helper, %s, has been kicked from the help channel.", p->nick, p->nick);
         }
         else
         {
            anope_cmd_notice_ops(s_HelpServ, HelpChannel, "The ticket opened by %s has been closed because their helper, %s, has left the help channel", p->nick, p->helper);
         }
         strcpy(p->helper,"");
         strcpy(p->helper_c_id,"");
      }
      p = p->next;
   }
   if(p != NULL && stricmp(p->nick,u->nick) == 0) {
      if(count == 1) {
          anope_cmd_notice_ops(s_HelpServ, HelpChannel, "The ticket opened by %s has been closed because %s is no longer connected.", p->nick, p->nick);
      }
      else if(count == 2) {
         anope_cmd_notice_ops(s_HelpServ, HelpChannel, "The ticket opened by %s has been closed because %s has been kicked from the help channel.", p->nick, p->nick);
      }
      else
      {
         anope_cmd_notice_ops(s_HelpServ, HelpChannel, "The ticket opened by %s has been closed because %s has left the help channel", p->nick, p->nick);
      }
      close_ticket(u, u->nick);
   }
   return MOD_CONT;
}

int he_hook_connect(int count, char **av) {
   User *u = NULL;

   if(count != 1) {
      alog("Warning: [he_queue] Invalid event count in he_hook_connect (Count is %d, expected 1. Event will not be processed.", 1);
      return MOD_CONT;
   }

   u = finduser(av[0]);
   if(!u) {
      alog("Warning: [he_queue] Could not assign ID to user %s", av[0]);
   }
   else
   {
      he_new_id(u);
   }
   return MOD_CONT;
}

int he_hook_nick(int count, char **av) {
   User *u = NULL;
   char *data = NULL;

   Helpqueue *p = hqueue;
   User *helper = NULL;
   User *ticket_u = NULL;

   if(count != 1) {
      alog("Warning: [he_queue] Invalid event count in he_hook_nick (Count is %d, expected 1. Event will not be processed.", count);
      return MOD_CONT;
   }

   if(hqueue == NULL) {
      return MOD_CONT;
   }

   u = finduser(av[0]);
   if(u != NULL) {
      data = moduleGetData(&u->moduleData, "c_id");
      if(data != NULL) {
         while(p != NULL) {
            if(strcmp(data,p->c_id) == 0) {
               if(strlen(p->helper) > 0) {
                  helper = finduser(p->helper);
                  if(helper != NULL) {
                     moduleNoticeLang(s_HelpServ, helper, LANG_HELPME_CHANGE_NICK, p->c_id, p->nick, av[0]);
                  }
               }
               strcpy(p->nick, av[0]);
               break;
            }
            else {
               if(strlen(p->helper) > 0) {
                  if(strcmp(data,p->helper_c_id) == 0) {
                     ticket_u = finduser(p->nick);
                     if(ticket_u != NULL) {
                        moduleNoticeLang(s_HelpServ, ticket_u, LANG_HELPME_CHANGE_HELPERNICK, p->helper, av[0]);
                     }
                  }
                  strcpy(p->helper, av[0]);
               }
            }
            p = p->next;
         }
         free(data);
      }
   }
   return MOD_CONT;
}

int he_new_id(User *u) {
   struct tm tm;
   time_t currtime;
   char conn_userid[16];
   int data = 0;

   currtime = time(NULL);
   tm = *localtime(&currtime);

   if(!conn_yday || conn_yday != tm.tm_yday) {
      conn_id = 1;
      conn_yday = tm.tm_yday;
   }

   sprintf(conn_userid, "%d%d_%d", tm.tm_year, conn_yday, conn_id);
   data = moduleAddData(&u->moduleData, "c_id", conn_userid);
   if(data != MOD_ERR_OK) {
      alog("[he_queue] Unable to assign ID %s to user %s", conn_userid, u->nick);
   }
   conn_id++;
   return data;
}
