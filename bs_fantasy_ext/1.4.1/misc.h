/**
 * Misc methodes used by the modules - Headers
 * Misc commands the module adds or overrides - Headers
 *
 ***********
 * Module Name    : bs_fantasy_ext
 * Author         : Viper <Viper@Anope.org>
 * Creation Date  : 21/07/2006
 *
 * More info on http://modules.anope.org and http://forum.anope.org
 ***********
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***********
 *
 * Last Updated   : 04/09/2012
 *
 **/

/* Check if the variable is NULL or is \0.. we shouldn't free() in either case.
 * Many thanks to Trystan for providing this */
#define BadPtr(x) (!(x) || (*(x) == '\0'))

/* Variables */
User *dummy;

/* Functions */
static int check_opercmds(User *u);
static int my_check_access(User *u, ChannelInfo *ci, int what);

static int do_core_fantasy(int ac, User *u, ChannelInfo *ci, char *cmd, char *params);
static int do_up_down(User *u, Channel *c, char *cmd, int target);
static void do_up_down_mask(User *u, Channel *c, char *cmd, int target, char *mask);

void load_config(void);
int reload_config(int argc, char **argv);

void check_core_modules(void);
int check_modules(void);
int event_modload(int argc, char **argv);
int event_modunload(int argc, char **argv);
int event_check_cmd(int argc, char **argv);

void create_dummy(void);
void delete_dummy(void);

static void show_version(User *u, ChannelInfo *ci);
static void show_modinfo(User *u, ChannelInfo *ci);
static char* get_flags();
void update_version(void);

int check_banmask(User *u, Channel *c, char *mask);

void addTempBan(Channel *c, time_t timeout, char *banmask);
int delTempBan(int argc, char **argv);
char *make_timeleft(NickAlias *na, char *buf, int len, int t_left);

int delBan(ChannelInfo *ci, Entry *ban);

char* get_str(char *param);
int my_match_wild(char *pat, char *str);
int my_match_wild_nocase(char *pat, char *str);
char *strrstr(const char *str1, const char *str2);

int get_access_nc(NickCore *nc, ChannelInfo *ci);

/* EOF */
