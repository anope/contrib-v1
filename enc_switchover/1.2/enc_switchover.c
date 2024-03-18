/**
 * -----------------------------------------------------------------------------
 * Name    : enc_switchover
 * Author  : Viper  <Viper@Anope.org>
 * Date    : 25/08/2007  (Last update: 10/05/2010)
 * Version : 1.2
 * -----------------------------------------------------------------------------
 * Tested     : Anope-1.8.4 SVN + UnrealIRCd 3.2.6
 * Requires   : Anope-1.8.5 or higher
 * -----------------------------------------------------------------------------
 *
 *                     ! ! ! ! ! ! ! ! ! ! ! ! ! !
 *                     ! ! ! READ THIS FIRST ! ! !
 *                     ! ! ! ! ! ! ! ! ! ! ! ! ! !
 *
 * This module allows users to gradually migrate the databases from one
 * encryption method to another. For example: migrate from the broken md5
 * implementation to the newer correct implementation. It also allows
 * users to migrate from md5 to sha1 or to no encryption at all.
 *
 * When switching from no encryption to an encryption (enc_old, enc_md5 or enc_sha1),
 * the module will simply take the plain text passwords from the database, encrypt them
 * and unload the module again. This means that switching from no encryption is done
 * almost instantly.
 * However it is very important that you DO NOT LOAD THE MODULE TWICE WHEN OldEncModule
 * is set to ENC_NONE because when loaded for the second time it will take the already
 * encrypted passwords and make a hash of the hash...
 * Therefore it is recommended that you ALWAYS LOAD THE MODULE MANUALLY.
 * Just in case services restart before the directive is removed or so...
 *
 * When switching from any other encryption type, the module works by gradually,
 * replacing a password encrypted with the old encryption module (OldEncModule)
 * by the password encrypted with the new module (EncModule) as it is used to identify with.
 * This means each nickname has to be identified for to allow the module to replace
 * the password. Therefore it is recommended to keep the module loaded for
 * at least the time specified in NSExpire or CSExpire, depending on which is the longest
 * (Just pick the longest of the two...).
 * Keep in mind that nicknames with the noexpire option may not be used in that
 * time period yet not be used either, leaving them with a password encrypted with
 * the old method when all other users have been migrated. If this module is then
 * unloaded - which is highly recommended since it should not longer be necessary -
 * those passwords will have to be reset manually.
 *
 * The module does exactly the same as described above for channels through the
 * chanserv identify command.
 *
 * Also note that once this module is loaded and starts migrating the database
 * from one encryption method to another it can NOT be aborted in the middle of
 * the migration process without losing data. So if you are in the middle of a
 * migration to md5 and suddenly decide you want sha1, you will have to wait untill
 * the migration to md5 is completed before starting another one.
 * This is because the module does not track which nicknames' passwords have already
 * been converted when anope restarts to the new encryption method and does only provide the
 * ability to check against ONE prevrious encryption method..
 * It works by comparing the password encrypted with EncModule, if that fails, it
 * compares it with OldEncModule, if that fails IDENTIFY fails.
 *
 * Another scenario in which this module might be useful is when merging DBs with a
 * different encryption type. If either is using enc_none, then it should be converted
 * seperately and then merged with the other. However if you have a enc_sha1 and a enc_md5
 * DB this module can be used to convert the passwords of either of the 2 databases
 * after they have been merged.
 *
 * The module hooks to the IDENTIFY command for both nickserv and chanserv
 * before all other modules are called, including the core ones. (If everything goes well..)
 * Other modules like ns_sidentify that also affect the identification process
 * should also be avoided during the migration process.
 *
 * !!! BE SURE TO MAKE A BACKUP OF YOUR DATABASES BEFORE STARTING THE MIGRATION !!!
 *
 * -----------------------------------------------------------------------------
 *
 * Usage Instructions (for migrating between 2 encryption types):
 *
 *      0.   Ensure you have read the 'READ THIS FIRST' chapter.
 *      1.   Make and install the module as any other module using using
 *           'make modules' and 'make install'.
 *      2.   Update in services.conf:
 *                - Copy the configuration directives to services.conf (See below.)
 *                - Set OldEncModule to what is your current EncModule.
 *                - Set EncModule to your desired encryption type.
 *      3.   Restart anope.
 *      4.   Load the module. (Autoload is only recommended when OldEncModule is 
 *           NOT enc_none!!! be sure to use the delayed autoload!)
 *      5.   When migration is complete (immediately when moving from enc_none,
 *           or after some time in all other cases): remove the OldEncModule from
 *           your services.conf to avoid accidental usage.
 *
 * For more info or help and assistance contact me on irc.anope.org in #anope
 *
 * -----------------------------------------------------------------------------
 *
 * Changelog:
 *
 *    1.2    Fixed OS CONVERSION WAITING showing converted instead of unconverted accounts.
 *           Fixed 'OldEncModule changed' warning being shown on every OS RELOAD.
 *           Fixed missing hooks for some NS cmds that also have a password.
 *
 *    1.1    Added tracking of which groups/channels don't have new pass enc yet.
 *           Updated to work with Anope-1.8.0.
 *           Fixed buffer overflow in sha1 encryption..
 *           Fixed compiler warnings.
 *           When using enc_none as new encryption module we now prevent the use
 *               of getpass on passes that haven't been confirmed updated.
 *
 *    1.0    Initial release.
 *
 * -----------------------------------------------------------------------------
 * All encryption subroutines have been copied from the Anope Core
 *
 * (C) 2003-2009 Anope Team
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church.
 *
 * SHA-1 implementation
 * By Steve Reid <steve@edmweb.com>
 * 100% Public Domain
 *
 * MD5C.C - RSA Data Security, Inc., MD5 message-digest algorithm
 *
 * MD5 taken from IRC Services
 * (C) 1996-2002 Andrew Church.
 * E-mail: <achurch@achurch.org>
 * Parts written by Andrew Kempe and others.
 *
 * -----------------------------------------------------------------------------
 **/

/**
 * TODO:
 *
 *  << Nothing I can think of... >>
 *
 **/

/**
 * Configuration directives that should be copy-pasted to services.conf

# OldEncModule [OPTIONAL - REQUIRED by enc_switchover]
# Module: enc_switchover
#
# When using the enc_switchover module to gradually migrate you databases from one
# encryption method to another, use this directive to specify the encryption
# method you are migrating FROM. The new encryption method should be specified in EncModule.
#
# Read the documentation that comes with the enc_switchover module BEFORE using it!
#
#     Plain Text / None         -  enc_none
#     Previous (broken) MD5     -  enc_old
#     MD5                       -  enc_md5
#     SHA1                      -  enc_sha1
#
#OldEncModule "enc_none"

 *
 **/

#include "module.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define AUTHOR "Viper"
#define VERSION "1.2"

/* Language defines */
#define LANG_NUM_STRINGS 					11

#define LANG_CONVERSION_DESC				0
#define LANG_CONVERSION_SYNTAX				1
#define LANG_CONVERSION_SYNTAX_EXT			2
#define LANG_CONVERSION_STATS_N				3
#define LANG_CONVERSION_STATS_C				4
#define LANG_CONVERSION_WAITING_NH			5
#define LANG_CONVERSION_WAITING_N			6
#define LANG_CONVERSION_WAITING_NF			7
#define LANG_CONVERSION_WAITING_CH			8
#define LANG_CONVERSION_WAITING_C			9
#define LANG_CONVERSION_WAITING_CF			10


#define TO_COLLIDE		0           /* Collide the user with this nick */
#define TO_RELEASE		1           /* Release a collided nick */


/* Constants for MD5Transform routine. */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

/* F, G, H and I are basic MD5 functions. */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits. */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }

#define XTOI(c) ((c)>9 ? (c)-'A'+10 : (c)-'0')

/* SHA1 defines */
/* #define LITTLE_ENDIAN            * This should be #define'd if true. */
/* #define SHA1HANDSOFF             * Copies data before messing with it. */
#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
#ifdef LITTLE_ENDIAN
  #define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
	|(rol(block->l[i],8)&0x00FF00FF))
#else
  #define blk0(i) block->l[i]
#endif

#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
	^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);



/* Structs */
typedef unsigned int UINT4;

/* MD5 context. */
typedef struct {
	UINT4 state[4];                                   /* state (ABCD) */
	UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */
	unsigned char buffer[64];                         /* input buffer */
} MD5_CTX;

typedef void *POINTER;

typedef struct {
    uint32 state[5];
    uint32 count[2];
    unsigned char buffer[64];
} SHA1_CTX;


/* Constants */
char *ModDataKey = "encSwitch";
static unsigned char PADDING[64] = {
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


/* Variables */
char *OldEncModule = NULL;


/* Functions */
int do_immed_encrypt();

int do_ns_identify(User *u);
int do_ns_gg(User *u);
int do_cs_identify(User *u);
int do_conv_status(User *u);

int do_help(User *u);
void do_help_list(User *u);

int old_enc_check_password(const char *plaintext, const char *password);
int checkupdate_ns_password(NickAlias *na, const char *plaintext);
void update_ns_password(NickCore *nc, const char *password);
int checkupdate_cs_password(ChannelInfo *ci, const char *plaintext);
void update_cs_password(ChannelInfo *ci, const char *password);

int do_ns_getpass(User *u);
int do_cs_getpass(User *u);

void load_config(void);
int reload_config(int argc, char **argv);
void add_languages(void);

int plain_encrypt(const char *src,int len,char *dest,int size);
int plain_encrypt_in_place(char *buf, int size);
int plain_encrypt_check_len(int passlen, int bufsize);
int plain_decrypt(const char *src, char *dest, int size);
int plain_check_password(const char *plaintext, const char *password);

int md5_encrypt(const char *src, int len, char *dest, int size);
int md5_encrypt_in_place(char *buf, int size);
int md5_encrypt_check_len(int passlen, int bufsize);
int md5_decrypt(const char *src, char *dest, int size);
int md5_check_password(const char *plaintext, const char *password);

void binary_to_hex(unsigned char *bin, char *hex, int length);

void MD5Init(MD5_CTX *context);
void MD5Update(MD5_CTX *context, unsigned char *input, unsigned int inputLen);
void MD5Final(unsigned char digest[16], MD5_CTX *context);
void MD5Transform(UINT4 state[4], unsigned char block[64]);
void Encode(unsigned char *output, UINT4 *input, unsigned int len);
void Decode(UINT4 *output, unsigned char *input, unsigned int len);

int old_encrypt(const char *src, int len, char *dest, int size);
int old_encrypt_in_place(char *buf, int size);
int old_encrypt_check_len(int passlen, int bufsize);
int old_check_password(const char *plaintext, const char *password);
int old_decrypt(const char *src, char *dest, int size);

void SHA1Transform(uint32 state[5], unsigned char const buffer[64]);
void SHA1Init(SHA1_CTX* context);
void SHA1Update(SHA1_CTX* context, unsigned char const * data, uint32 len);
void SHA1Final(unsigned char digest[20], SHA1_CTX* context);

int sha1_encrypt(const char *src, int len, char *dest, int size);
int sha1_encrypt_in_place(char *buf, int size);
int sha1_encrypt_check_len(int passlen, int bufsize);
int sha1_decrypt(const char *src, char *dest, int size);
int sha1_check_password(const char *plaintext, const char *password);

/* ------------------------------------------------------------------------------- */

/**
 * Create the command, and tell anope about it.
 * @param argc Argument count
 * @param argv Argument list
 * @return MOD_CONT to allow the module, MOD_STOP to stop it
 **/
int AnopeInit(int argc, char **argv) {
	int status;
	Command *c;
	EvtHook *hook;

	alog("[\002enc_switchover\002] Loading module...");

	if (!moduleMinVersion(1,8,4,1899)) {
		alog("[\002enc_switchover\002] Your version of Anope isn't supported. Please update to a newer release.");
		return MOD_STOP;
	}

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);

	add_languages();
	load_config();
	status = do_immed_encrypt();

	/* If the old encryption type is enc_none, we can encrypt
	 * everything right away since we have plain text passes in the db. */
	if (status == MOD_STOP) {
		alog("[\002enc_switchover\002] All passwords have been encrypted...");
		alog("[\002enc_switchover\002] DO NOT LOAD THIS MODULE AGAIN WITH CURRENT CONFIGURATION SETTINGS !!!");
		alog("[\002enc_switchover\002] Remove or comment out OldEncModule before starting services again!");
		alog("[\002enc_switchover\002] If you do not your database will be destroyed beyond repair next time this module is loaded.");
		return MOD_STOP;
	} else if (status == -1) {
		return MOD_STOP;
	}

	/* Create identify command.. */
	c = createCommand("ID", do_ns_identify, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002enc_switchover\002] Cannot create NS ID command...");
		return MOD_STOP;
	}

	c = createCommand("IDENTIFY", do_ns_identify, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002enc_switchover\002] Cannot create NS IDENTIFY command...");
		return MOD_STOP;
	}

	c = createCommand("SIDENTIFY", do_ns_identify, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002enc_switchover\002] Cannot create NS SIDENTIFY command...");
		return MOD_STOP;
	}

	c = createCommand("IDENTIFY", do_cs_identify, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(CHANSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002enc_switchover\002] Cannot create CS IDENTIFY command...");
		return MOD_STOP;
	}

	c = createCommand("ID", do_cs_identify, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(CHANSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002enc_switchover\002] Cannot create CS SIDENTIFY command...");
		return MOD_STOP;
	}

	/* Also hook to commands like GROUP and GHOST... */
	c = createCommand("GHOST", do_ns_gg, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002enc_switchover\002] Cannot create NS GHOST command...");
		return MOD_STOP;
	}

	c = createCommand("GROUP", do_ns_gg, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002enc_switchover\002] Cannot create NS GROUP command...");
		return MOD_STOP;
	}

	c = createCommand("RECOVER", do_ns_gg, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002enc_switchover\002] Cannot create NS RECOVER command...");
		return MOD_STOP;
	}

	c = createCommand("RELEASE", do_ns_gg, NULL, -1, -1, -1, -1, -1);
	if (moduleAddCommand(NICKSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002enc_switchover\002] Cannot create NS RELEASE command...");
		return MOD_STOP;
	}

	/* Intercept and check GETPASS commands if new module is enc_none.. */
	if (!stricmp(EncModule, "enc_none")) {
		c = createCommand("GETPASS", do_ns_getpass, is_services_admin, -1, -1, -1, -1, -1);
		if (moduleAddCommand(NICKSERV, c, MOD_HEAD) != MOD_ERR_OK) {
			alog("[\002enc_switchover\002] Cannot create NS GETPASS command...");
			return MOD_STOP;
		}

		c = createCommand("GETPASS", do_cs_getpass, is_services_admin, -1, -1, -1, -1, -1);
		if (moduleAddCommand(CHANSERV, c, MOD_HEAD) != MOD_ERR_OK) {
			alog("[\002enc_switchover\002] Cannot create CS GETPASS command...");
			return MOD_STOP;
		}
	}

	/* Create commands to get conversion status.. */
	c = createCommand("CONVERSION", do_conv_status, is_services_admin, -1, -1, -1, -1, -1);
	if (moduleAddCommand(OPERSERV, c, MOD_HEAD) != MOD_ERR_OK) {
		alog("[\002enc_switchover\002] Cannot create OS CONVERSION command...");
		return MOD_STOP;
	}
	moduleAddHelp(c,do_help);
	moduleSetOperHelp(do_help_list);

	/* Hook to a config reload.. */
	hook = createEventHook(EVENT_RELOAD, reload_config);
	if (moduleAddEventHook(hook) != MOD_ERR_OK) {
		alog("[\002enc_switchover\002] Can't hook to EVENT_RELOAD event");
		return MOD_STOP;
	}

	alog("[\002enc_switchover\002] Module loaded successfully...");
	return MOD_CONT;
}

/**
 * Unload the module
 **/
void AnopeFini(void) {
	alog("[\002enc_switchover\002] Unloading module...");

	if (OldEncModule)
		free(OldEncModule);
}


/* ------------------------------------------------------------------------------- */

/**
 * We return MOD_STOP if we are succesful, in any other case, just let the loading continue..
 **/
int do_immed_encrypt() {
	int i;
	NickCore *nc;
	ChannelInfo *ci;

	if (!OldEncModule) {
		alog("[\002enc_switchover\002] OldEncModule is not specified.");
		return -1;
	}

	/* If the previous enc type is anything other then enc_none, continue loading. */
	if (stricmp(OldEncModule, "enc_none"))
		return MOD_CONT;

	alog("[\002enc_switchover\002] Passwords are stored plain text in db. Encrypting... ");

	/* Go over the entire NickCore list and encrypt the passwords..*/
	for (i = 0; i < 1024; i++) {
		for (nc = nclists[i]; nc; nc = nc->next) {
			/* Just a sanity check..*/
			if (!nc->pass) {
				alog("[\002enc_switchover\002] No password present for User %s. (May be forbidden)", nc->display);
				continue;
			}

			update_ns_password(nc, nc->pass);
		}
	}

	/* Do the same for the channels.. */
    for (i = 0; i < 256; i++) {
        for (ci = chanlists[i]; ci; ci = ci->next) {
			if (ci->flags & CI_VERBOTEN)
				continue;

			if (!ci->founderpass) {
				alog("[\002enc_switchover\002] No password present for Channel %s. (May be forbidden)", ci->name);
				continue;
			}
			update_cs_password(ci, ci->founderpass);
		}
	}

	return MOD_STOP;
}

/* ------------------------------------------------------------------------------- */

/**
 * The /ns identify command.
 * @param u The user who issued the command
 * @param MOD_CONT to continue processing other modules, MOD_STOP to stop processing.
 **/
int do_ns_identify(User *u) {
	NickAlias *na;
	char *buffer, *pass;

	if (!OldEncModule)
		return MOD_CONT;

	buffer = moduleGetLastBuffer();
	pass = myStrGetToken(buffer, ' ', 0);

	/* Check if the core can't handle it... */
	if (pass && (na = u->na) && !(na->status & NS_VERBOTEN) && !(na->nc->flags & NI_SUSPENDED)
			&& !nick_identified(u)) {
		if (checkupdate_ns_password(na, pass)) {
			update_ns_password(na->nc, pass);
		}
	}

	if (pass)
		free(pass);

	/* We don't handle the identifying itself.. let the core do it. */
	return MOD_CONT;
}

/**
 * Parser for commands like group and ghost with nick & pass given.
 * @param u The user who issued the command
 * @param MOD_CONT to continue processing other modules, MOD_STOP to stop processing.
 **/
int do_ns_gg(User *u) {
	NickAlias *na;
	char *buffer, *nick, *pass;

	if (!OldEncModule)
		return MOD_CONT;

	buffer = moduleGetLastBuffer();
	nick = myStrGetToken(buffer, ' ', 0);
	pass = myStrGetToken(buffer, ' ', 1);

	/* Check if the core can't handle it... */
	/* We won't parse ghosts or so where only a nick is provided, this only works if the user is ID'd anyways.. */
	if (nick && pass && (na = findnick(nick)) && !(na->status & NS_VERBOTEN) && !(na->nc->flags & NI_SUSPENDED)
			&& (!na->u || (na->u && (na->status & NS_IDENTIFIED)))) {
		if (checkupdate_ns_password(na, pass)) {
			update_ns_password(na->nc, pass);
		}
	}

	if (nick)
		free(nick);
	if (pass)
		free(pass);

	/* We don't handle the identifying itself.. let the core do it. */
	return MOD_CONT;
}

int do_cs_identify(User *u) {
	char *buffer, *chan, *pass;
	ChannelInfo *ci;

	if (!OldEncModule)
		return MOD_CONT;

	buffer = moduleGetLastBuffer();
	chan = myStrGetToken(buffer, ' ', 0);
	pass = myStrGetToken(buffer, ' ', 1);

	if (chan && pass && (ci = cs_findchan(chan)) && !(ci->flags & CI_VERBOTEN) && nick_identified(u)) {
		if (checkupdate_cs_password(ci, pass))
			update_cs_password(ci, pass);
	}

	if (chan) free(chan);
	if (pass) free(pass);

	/* We don't handle the identifying itself.. let the core do it. */
	return MOD_CONT;
}

int do_conv_status(User *u) {
	char *buffer, *option = NULL;
	int i;
	NickCore *nc;
	ChannelInfo *ci;

	buffer = moduleGetLastBuffer();
	option = myStrGetToken(buffer, ' ', 0);

	if (option != NULL) {
		if (stricmp(option, "STATUS") == 0) {
			int n_count = 0, n_conv = 0, c_count = 0, c_conv = 0;

			for (i = 0; i < 1024; i++) {
				for (nc = nclists[i]; nc; nc = nc->next) {
					/* Sanity check..*/
					if (!nc->pass)
						continue;
					n_count++;
					if (moduleGetData(&nc->moduleData, ModDataKey))
						n_conv++;
				}
			}
			for (i = 0; i < 256; i++) {
				for (ci = chanlists[i]; ci; ci = ci->next) {
					if ((ci->flags & CI_VERBOTEN) || !ci->founderpass)
						continue;
					c_count++;
					if (moduleGetData(&ci->moduleData, ModDataKey))
						c_conv++;
				}
			}

			moduleNoticeLang(s_OperServ, u, LANG_CONVERSION_STATS_N, n_count, n_conv, floor(n_conv/n_count * 100));
			moduleNoticeLang(s_OperServ, u, LANG_CONVERSION_STATS_C, c_count, c_conv, floor(c_conv/c_count * 100));
		} else if (stricmp(option, "WAITING") == 0) {
			int n_conv = 0, c_conv = 0;

			moduleNoticeLang(s_OperServ, u, LANG_CONVERSION_WAITING_NH);
			for (i = 0; i < 1024; i++) {
				for (nc = nclists[i]; nc; nc = nc->next) {
					/* Sanity check..*/
					if (!nc->pass)
						continue;
					if (!moduleGetData(&nc->moduleData, ModDataKey)) {
						moduleNoticeLang(s_OperServ, u, LANG_CONVERSION_WAITING_N, nc->display);
						n_conv++;
					}
				}
			}
			moduleNoticeLang(s_OperServ, u, LANG_CONVERSION_WAITING_NF, n_conv);

			moduleNoticeLang(s_OperServ, u, LANG_CONVERSION_WAITING_CH);
			for (i = 0; i < 256; i++) {
				for (ci = chanlists[i]; ci; ci = ci->next) {
					if ((ci->flags & CI_VERBOTEN) || !ci->founderpass)
						continue;
					if (!moduleGetData(&ci->moduleData, ModDataKey)) {
						moduleNoticeLang(s_OperServ, u, LANG_CONVERSION_WAITING_C, ci->name);
						c_conv++;
					}
				}
			}
			moduleNoticeLang(s_OperServ, u, LANG_CONVERSION_WAITING_CF, c_conv);

		} else
			moduleNoticeLang(s_OperServ, u, LANG_CONVERSION_SYNTAX);

		free(option);
	} else
		moduleNoticeLang(s_OperServ, u, LANG_CONVERSION_SYNTAX);

	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

/**
 * Show the extended help on the CONVERSION command
 **/
int do_help(User *u) {
	if (is_services_admin(u))
		moduleNoticeLang(s_OperServ, u, LANG_CONVERSION_SYNTAX_EXT);

	return MOD_CONT;
}

/**
 * Add the CONVERSION command to the OperServ HELP listing
 **/
void do_help_list(User *u) {
	if (is_services_admin(u)) {
		moduleNoticeLang(s_OperServ, u, LANG_CONVERSION_DESC);
	}
}

/* ------------------------------------------------------------------------------- */

/**
 * We compare the given password to the one in the databases using
 * the old encryption algorithm specified.
 **/
int old_enc_check_password(const char *plaintext, const char *password) {
	if (!strcmp(OldEncModule,"enc_none"))
		return plain_check_password(plaintext, password);

	if (!strcmp(OldEncModule,"enc_old"))
		return old_check_password(plaintext, password);

	if (!strcmp(OldEncModule,"enc_md5"))
		return md5_check_password(plaintext, password);

	if (!strcmp(OldEncModule,"enc_sha1"))
		return sha1_check_password(plaintext, password);

	return -1;
}

/**
 * Compare the given password to what is in the database using current
 * and old encryption type.
 * Also sets flag on user if password has been updated.
 *
 * Returns 1 if password needs to be updated.
 * Returns 0 if password is up-to-date or failed to match.
 * Returns -1 if there s a problem.
 **/
int checkupdate_ns_password(NickAlias *na, const char *plaintext) {
	int res1, res2;

	if (!na || !na->nc)
		return -1;

	res1 = enc_check_password(plaintext, na->nc->pass);
	if (res1 == 1) {
		if (!moduleGetData(&na->nc->moduleData, ModDataKey))
			moduleAddData(&na->nc->moduleData, ModDataKey, "1");
		return 0;
	}

	res2 = old_enc_check_password(plaintext, na->nc->pass);
	if (res2 == 1)
		return 1;

	if (!res1 && !res2)
		return 0;

	return -1;
}

void update_ns_password(NickCore *nc, const char *password) {
	char *pwd;
	int len = strlen(password);

	if (readonly) {
		alog("[enc_switchover] Failed to update password for %s. (Services are in Read-Only mode.)", nc->display);
		return;
	}

	/* Passwords get refused if they are too long,
	 * we on the other hand are converting so truncate instead. */
	pwd = sstrdup(password);
	if (enc_encrypt_check_len(len ,PASSMAX - 1)) {
		alog("[enc_switchover] Password for %s is too long. Truncating to %d.", nc->display, PASSMAX-1);
		len = PASSMAX;
		pwd[len] = 0;
	}

	if (enc_encrypt(pwd, len, nc->pass, PASSMAX - 1) < 0) {
		memset(pwd, 0, len);
		alog("[enc_switchover] Failed to encrypt password for %s.", nc->display);

		if (pwd)
			free(pwd);
		return;
	}

	memset(pwd, 0, len);
	if (!moduleGetData(&nc->moduleData, ModDataKey))
		moduleAddData(&nc->moduleData, ModDataKey, "1");
	alog("[enc_switchover] Updated password for %s.", nc->display);

	if (pwd)
		free(pwd);
}


/**
 * Compare the given password to what is in the database using current
 * and old encryption type.
 * Also sets flag on channel if password has been updated.
 *
 * Returns 1 if password needs to be updated.
 * Returns 0 if password is up-to-date or failed to match.
 * Returns -1 if there s a problem.
 **/
int checkupdate_cs_password(ChannelInfo *ci, const char *plaintext) {
	int res1, res2;

	if (!ci)
		return -1;

	res1 = enc_check_password(plaintext, ci->founderpass);
	if (res1 == 1) {
		if (!moduleGetData(&ci->moduleData, ModDataKey))
			moduleAddData(&ci->moduleData, ModDataKey, "1");
		return 0;
	}

	res2 = old_enc_check_password(plaintext, ci->founderpass);
	if (res2 == 1)
		return 1;

	if (!res1 && !res2)
		return 0;

	return -1;
}

void update_cs_password(ChannelInfo *ci, const char *password) {
	char *pwd;
	int len = strlen(password);

	if (readonly) {
		alog("[enc_switchover] Failed to update password for %s. (Services are in Read-Only mode.)", ci->name);
		return;
	}

	pwd = sstrdup(password);
	if (enc_encrypt_check_len(len ,PASSMAX - 1)) {
		alog("[enc_switchover] Password for %s is too long. Truncating to %d.", ci->name, PASSMAX-1);
		len = PASSMAX;
		pwd[len] = 0;
	}

	if (enc_encrypt(pwd, len, ci->founderpass, PASSMAX - 1) < 0) {
		memset(pwd, 0, len);
		alog("[enc_switchover] Failed to encrypt password for %s.", ci->name);

		if (pwd)
			free(pwd);
		return;
	}

	memset(pwd, 0, len);
	if (!moduleGetData(&ci->moduleData, ModDataKey))
		moduleAddData(&ci->moduleData, ModDataKey, "1");
	alog("[enc_switchover] Updated password for %s.", ci->name);

	if (pwd)
		free(pwd);
}

/* ------------------------------------------------------------------------------- */

int do_ns_getpass(User *u) {
	char *buffer, *nick;
	NickAlias *na = NULL;
	NickRequest *nr = NULL;

	if (!OldEncModule || !stricmp(EncModule, "enc_none"))
		return MOD_CONT;

	buffer = moduleGetLastBuffer();
	nick = myStrGetToken(buffer, ' ', 0);

	if (nick && !(na = findnick(nick))) {
		if ((nr = findrequestnick(nick))) {
			notice(s_NickServ, u->nick, "Unable to recover passwords of nick requests");
			notice(s_NickServ, u->nick, "while database is being converted.");
			alog("%s: %s!%s@%s tried to use GETPASS on %s but function is disallowed on nickrequests during DB conversion.",
					s_NickServ, u->nick, u->username, u->host, nick);
			return MOD_STOP;
		}
	} else if (na && !(na->status & NS_VERBOTEN) && !(NSSecureAdmins &&
			nick_is_services_admin(na->nc) && !is_services_root(u)) &&
			!(NSRestrictGetPass && !is_services_root(u))) {
		if (!moduleGetData(&na->nc->moduleData, ModDataKey)) {
			notice(s_NickServ, u->nick, "Unable to recover nick password.");
			alog("%s: %s!%s@%s tried to use GETPASS on %s but password is still encrypted.",
					s_NickServ, u->nick, u->username, u->host, nick);
			if (WallGetpass) {
				anope_cmd_global(s_NickServ,
						"\2%s\2 tried to use GETPASS on \2%s\2 but password is still encrypted.",
						u->nick, nick);
			}
			free(nick);
			return MOD_STOP;
		}
	}

	if (nick) free(nick);
	return MOD_CONT;
}


int do_cs_getpass(User *u) {
	char *buffer, *chan;
	ChannelInfo *ci;

	if (!OldEncModule || !stricmp(EncModule, "enc_none"))
		return MOD_CONT;

	buffer = moduleGetLastBuffer();
	chan = myStrGetToken(buffer, ' ', 0);

	if (chan && (ci = cs_findchan(chan)) && !(ci->flags & CI_VERBOTEN) &&
			!(CSRestrictGetPass && !is_services_root(u))) {
		if (!moduleGetData(&ci->moduleData, ModDataKey)) {
			notice(s_ChanServ, u->nick, "Unable to recover channel password.");
			alog("%s: %s!%s@%s tried to use GETPASS on %s but password is still encrypted.",
					s_ChanServ, u->nick, u->username, u->host, ci->name);
			if (WallGetpass) {
				anope_cmd_global(s_ChanServ,
						"\2%s\2 tried to use GETPASS on channel \2%s\2 but password is still encrypted",
						u->nick, chan);
			}
			free(chan);
			return MOD_STOP;
		}
	}

	if (chan) free(chan);
	return MOD_CONT;
}

/* ------------------------------------------------------------------------------- */

/**
 * (Re)Load the configuration directives
 **/
void load_config(void) {
	int i;
	char *tmp;

	Directive confvalues[][1] = {
		{{"OldEncModule", {{PARAM_STRING, PARAM_RELOAD, &tmp}}}},
	};

	/* Note that we do not support changing OldEncModule at runtime! */
	for (i = 0; i < 1; i++)
		moduleGetConfigDirective(confvalues[i]);

	if (!tmp) {
		alog ("[\002enc_switchover\002] OldEncModule is \002not\002 set!!!");
		alog ("[\002enc_switchover\002] Module disabled..");
	}

	if (tmp && strcmp(tmp,"enc_none") && strcmp(tmp,"enc_old")
			&& strcmp(tmp,"enc_md5") && strcmp(tmp,"enc_sha1")) {
		alog ("[\002enc_switchover\002] Invalid value for OldEncModule!!!");
		alog ("[\002enc_switchover\002] Module disabled..");

		free(tmp);
		tmp = NULL;
	}

	if (tmp && !strcmp(tmp, EncModule)) {
		alog ("[\002enc_switchover\002] OldEncModule and EncModule are identical!");
		alog ("[\002enc_switchover\002] Module disabled..");

		free(tmp);
		tmp = NULL;
	}
	
	if (tmp && OldEncModule && !strcmp(tmp, OldEncModule)) {
		alog ("[\002enc_switchover\002] \002OldEncModule setting was changed at runtime. This is NOT supported!!\002");
		free(tmp);
		tmp = NULL;
	}

	if (OldEncModule) free(OldEncModule);
	OldEncModule = tmp;

	if (debug && OldEncModule)
		alog ("[enc_switchover] debug: OldEncModule set to %s", OldEncModule);
	else if (debug)
		alog ("[enc_switchover] debug: OldEncModule missing!!!");
}


/**
 * Upon /os reload call the routines for reloading the configuration directives
 **/
int reload_config(int argc, char **argv) {
	if (argc >= 1) {
		if (!stricmp(argv[0], EVENT_START)) {
			alog("[enc_switchover]: Reloading configuration directives...");
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
		/* LANG_CONVERSION_DESC */
		"    CONVERSION  Shows the status of the database conversion.",
		/* LANG_CONVERSION_SYNTAX */
		" Syntax: CONVERSION [ STATUS | WAITING ]",
		/* LANG_CONVERSION_SYNTAX_EXT */
		" Syntax: CONVERSION [ STATUS | WAITING ]\n"
		" \n"
		" This command allows services admins to monitor the progres of the\n"
		" database conversion between 2 encryption types.\n"
		" \n"
		" The STATUS option shows the number of accounts that have been processed.\n"
		" \n"
		" The WAITING options lists all accounts and/or channels that have not \n"
		" yet been processed.\n"
		" Warning: This may cause a FLOOD !!! Do NOT use if a large number of\n"
		" accounts/channels hasn't been converted yet!\n"
		" \n"
		" Note that the atual progres might be greater then indicated by these\n"
		" results as only conversions verified since the last restart are counted.",
		/* LANG_CONVERSION_STATS_N */
		" Out of %d nick accounts, %d have been converted (%d%).",
		/* LANG_CONVERSION_STATS_C */
		" Out of %d channels, %d have been converted (%d%).",
		/* LANG_CONVERSION_WAITING_NH */
		" Following nick groups may not yet have been converted:",
		/* LANG_CONVERSION_WAITING_N */
		" Display nick: %s",
		/* LANG_CONVERSION_WAITING_NF */
		" Total: %d nick groups.",
		/* LANG_CONVERSION_WAITING_CH */
		" Following channels may not yet have been converted:",
		/* LANG_CONVERSION_WAITING_C */
		" Channel: %s",
		/* LANG_CONVERSION_WAITING_CF */
		" Total: %d channels.",
	};

	char *langtable_nl[] = {
		/* LANG_CONVERSION_DESC */
		"    CONVERSION  Geeft de voortgang van de database conversie weer.",
		/* LANG_CONVERSION_SYNTAX */
		" Syntax: CONVERSION [ STATUS | WAITING ]",
		/* LANG_CONVERSION_SYNTAX_EXT */
		" Syntax: CONVERSION [ STATUS | WAITING ]\n"
		" \n"
		" Dit command geeft Services Administrators de mogelijkheid de voortgang\n"
		" van de database conversie tussen 2 encryptie algoritmen te volgen.\n"
		" \n"
		" De STATUS optie geeft het aantal accounts weer dat verwerkt is.\n"
		" \n"
		" De WAITING optie geeft alle accounts en/of kanalen weer die nog niet\n"
		" verwerkt zijn.\n"
		" Warning: This may cause a FLOOD !!! Do NOT use if a large number of\n"
		" accounts/channels hasn't been converted yet!\n"
		" \n"
		" Merk op dat de werkelijke vooruitgang groter kan zijn dan deze die wordt\n"
		" gerapporteerd. Enkel de gecontroleerde resultaten sinds de laatste herstart\n"
		" worden meegerekend.",
		/* LANG_CONVERSION_STATS_N */
		" Van de %d nick groepen zijn er reeds %d geconverteerd (%d%).",
		/* LANG_CONVERSION_STATS_C */
		" Van de %d kanalen zijn er reeds %d geconverteerd (%d%).",
		/* LANG_CONVERSION_WAITING_NH */
		" Volgende nick accounts zijn mogelijk  nog niet geconverteerd:",
		/* LANG_CONVERSION_WAITING_N */
		" Hoofd nick: %s",
		/* LANG_CONVERSION_WAITING_NF */
		" Totaal: %d nick groepen.",
		/* LANG_CONVERSION_WAITING_CH */
		" Volgende kanalen zijn mogelijk  nog niet geconverteerd:",
		/* LANG_CONVERSION_WAITING_C */
		" Kanaal: %s",
		/* LANG_CONVERSION_WAITING_CF */
		" Totaal: %d kanalen.",
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
	moduleInsertLanguage(LANG_NL, LANG_NUM_STRINGS, langtable_nl);
}

/* ------------------------------------------------------------------------------- */

int plain_encrypt(const char *src,int len,char *dest,int size) {
	if(size>=len) {
		memset(dest,0,size);
		strncpy(dest,src,len);
		dest[len] = '\0';
		return 0;
	}

	return -1;
}

int plain_encrypt_in_place(char *buf, int size) {
	return 0;
}

int plain_encrypt_check_len(int passlen, int bufsize) {
	if(bufsize>=passlen) {
		return 0;
	}

	return bufsize;
}

int plain_decrypt(const char *src, char *dest, int size) {
	memset(dest,0,size);
	strncpy(dest,src,size);
	return 1;
}

int plain_check_password(const char *plaintext, const char *password) {
	if(strcmp(plaintext,password)==0) {
		return 1;
	}

	return 0;
}

/* ------------------------------------------------------------------------------- */

/**
 * The correct md5 implementation.
 * We use the same MD5 subroutines as the old implementation.
 **/

int md5_encrypt(const char *src, int len, char *dest, int size) {
	MD5_CTX context;
	char tmp[33];

	if (size < 16)
		return -1;

	MD5Init(&context);
	MD5Update(&context, (unsigned char *)src, len);
	MD5Final((unsigned char *)dest, &context);

	if(debug) {
		memset(tmp,0,33);
		binary_to_hex((unsigned char *)dest,tmp,16);
		/* Dont log source if we were encrypting in place :) */
		if (memcmp(src, dest, 16) != 0) {
			alog("[enc_switchover] md5: hashed from [%s] to [%s]",src,tmp);
		} else {
			alog("[enc_switchover] md5: hashed password to [%s]",tmp);
		}
	}

	return 0;
}


int md5_encrypt_in_place(char *buf, int size) {
	return md5_encrypt(buf, strlen(buf), buf, size);
}


int md5_encrypt_check_len(int passlen, int bufsize) {
	if (bufsize < 16)
		fatal("[enc_switchover] md5: md5_check_len(): buffer too small (%d)", bufsize);
	return 0;
}


int md5_decrypt(const char *src, char *dest, int size) {
	return 0;
}


int md5_check_password(const char *plaintext, const char *password) {
	char buf[BUFSIZE];

	if (md5_encrypt(plaintext, strlen(plaintext), buf, sizeof(buf)) < 0)
		return -1;
	if (memcmp(buf, password, 16) == 0)
		return 1;

	return 0;
}

/*************************************************************************/

void binary_to_hex(unsigned char *bin, char *hex, int length) {
	static const char trans[] = "0123456789ABCDEF";
	int i;

	for(i = 0; i < length; i++) {
		hex[i  << 1]      = trans[bin[i] >> 4];
		hex[(i << 1) + 1] = trans[bin[i] & 0xf];
	}

	hex[i << 1] = '\0';
}

/**
 * MD5 initialization. Begins an MD5 operation, writing a new context.
 **/
void MD5Init(MD5_CTX *context) {
	context->count[0] = context->count[1] = 0;
	/* Load magic initialization constants. */
	context->state[0] = 0x67452301;
	context->state[1] = 0xefcdab89;
	context->state[2] = 0x98badcfe;
	context->state[3] = 0x10325476;
}

/**
 * MD5 block update operation. Continues an MD5 message-digest
 * operation, processing another message block, and updating the
 * context.
 **/
void MD5Update (MD5_CTX *context, unsigned char *input, unsigned int inputLen) {
	unsigned int i, index, partLen;

	/* Compute number of bytes mod 64 */
	index = (unsigned int)((context->count[0] >> 3) & 0x3F);

	/* Update number of bits */
	if ((context->count[0] += ((UINT4)inputLen << 3))
		< ((UINT4)inputLen << 3))
			context->count[1]++;
	context->count[1] += ((UINT4)inputLen >> 29);

	partLen = 64 - index;

	/* Transform as many times as possible. */
	if (inputLen >= partLen) {
		memcpy
			((POINTER)&context->buffer[index], (POINTER)input, partLen);
		MD5Transform (context->state, context->buffer);

		for (i = partLen; i + 63 < inputLen; i += 64)
			MD5Transform (context->state, &input[i]);

		index = 0;
	}
	else
		i = 0;

	/* Buffer remaining input */
	memcpy
		((POINTER)&context->buffer[index], (POINTER)&input[i],
		inputLen-i);
}

/**
 * MD5 finalization. Ends an MD5 message-digest operation, writing the
 * the message digest and zeroizing the context.
 **/
void MD5Final (unsigned char digest[16], MD5_CTX *context) {
	unsigned char bits[8];
	unsigned int index, padLen;

	/* Save number of bits */
	Encode (bits, context->count, 8);

	/* Pad out to 56 mod 64. */
	index = (unsigned int)((context->count[0] >> 3) & 0x3f);
	padLen = (index < 56) ? (56 - index) : (120 - index);
	MD5Update (context, PADDING, padLen);

	/* Append length (before padding) */
	MD5Update (context, bits, 8);

	/* Store state in digest */
	Encode (digest, context->state, 16);

	/* Zeroize sensitive information. */
	memset ((POINTER)context, 0, sizeof (*context));
}

/**
 * MD5 basic transformation. Transforms state based on block.
 **/
void MD5Transform(UINT4 state[4], unsigned char block[64]) {
	UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

	Decode (x, block, 64);

	/* Round 1 */
	FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
	FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
	FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
	FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
	FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
	FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
	FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
	FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
	FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
	FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
	FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
	FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
	FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
	FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
	FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
	FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

	/* Round 2 */
	GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
	GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
	GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
	GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
	GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
	GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
	GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
	GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
	GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
	GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
	GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
	GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
	GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
	GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
	GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
	GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

	/* Round 3 */
	HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
	HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
	HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
	HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
	HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
	HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
	HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
	HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
	HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
	HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
	HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
	HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
	HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
	HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
	HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
	HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

	/* Round 4 */
	II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
	II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
	II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
	II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
	II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
	II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
	II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
	II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
	II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
	II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
	II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
	II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
	II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
	II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
	II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
	II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;

	/* Zeroize sensitive information. */
	memset ((POINTER)x, 0, sizeof (x));
}

/**
 * Encodes input (UINT4) into output (unsigned char).
 * Assumes len is a multiple of 4.
 **/
void Encode(unsigned char *output, UINT4 *input, unsigned int len) {
	unsigned int i, j;

	for (i = 0, j = 0; j < len; i++, j += 4) {
		output[j] = (unsigned char)(input[i] & 0xff);
		output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
		output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
		output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
	}
}

/**
 * Decodes input (unsigned char) into output (UINT4).
 * Assumes len is a multiple of 4.
 **/
void Decode(UINT4 *output, unsigned char *input, unsigned int len) {
	unsigned int i, j;

	for (i = 0, j = 0; j < len; i++, j += 4)
		output[i] = ((UINT4)input[j]) | (((UINT4)input[j+1]) << 8) |
			(((UINT4)input[j+2]) << 16) | (((UINT4)input[j+3]) << 24);
}


/* ------------------------------------------------------------------------------- */

/**
 * Encrypt `src' of length `len' and store the result in `dest'.  If the
 * resulting string would be longer than `size', return -1 and leave `dest'
 * unchanged; else return 0.
 **/
int old_encrypt(const char *src, int len, char *dest, int size) {
	MD5_CTX context;
	char digest[33];
	char tmp[33];
	int i;

	if (size < 16)
		return -1;

	memset(&context, 0, sizeof(context));
	memset(&digest, 0, sizeof(digest));

	MD5Init(&context);
	MD5Update(&context, (unsigned char *)src, len);
	MD5Final((unsigned char *)digest, &context);
	for (i = 0; i < 32; i += 2)
		dest[i / 2] = XTOI(digest[i]) << 4 | XTOI(digest[i + 1]);

	if(debug) {
		memset(tmp,0,33);
		binary_to_hex((unsigned char *)dest,tmp,16);
		alog("[enc_switchover] md5_old: Converted [%s] to [%s]",src,tmp);
	}

	return 0;

}


/**
 * Shortcut for encrypting a null-terminated string in place.
 **/
int old_encrypt_in_place(char *buf, int size) {
	return old_encrypt(buf, strlen(buf), buf, size);
}

int old_encrypt_check_len(int passlen, int bufsize) {
	if (bufsize < 16)
		fatal("[enc_switchover] md5_old: old_check_len(): buffer too small (%d)", bufsize);
	return 0;
}


/**
 * Compare a plaintext string against an encrypted password.
 * Return 1 if they match, 0 if not, and -1 if something went wrong.
 **/
int old_check_password(const char *plaintext, const char *password) {
	char buf[BUFSIZE];

	if (old_encrypt(plaintext, strlen(plaintext), buf, sizeof(buf)) < 0)
		return -1;
	if (memcmp(buf, password, 16) == 0)
		return 1;

	return 0;
}

int old_decrypt(const char *src, char *dest, int size) {
	return 0;
}


/* ------------------------------------------------------------------------------- */

/* Hash a single 512-bit block. This is the core of the algorithm. */
void SHA1Transform(uint32 state[5], unsigned char const buffer[64]) {
	uint32 a, b, c, d, e;
	typedef union {
		unsigned char c[64];
		uint32 l[16];
	} CHAR64LONG16;
	CHAR64LONG16* block;

#ifdef SHA1HANDSOFF
	static unsigned char workspace[64];
	block = (CHAR64LONG16*)workspace;
	memcpy(block, buffer, 64);
#else
	block = (CHAR64LONG16*)buffer;
#endif

	/* Copy context->state[] to working vars */
	a = state[0];
	b = state[1];
	c = state[2];
	d = state[3];
	e = state[4];

	/* 4 rounds of 20 operations each. Loop unrolled. */
	R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
	R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
	R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
	R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
	R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
	R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
	R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
	R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
	R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
	R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
	R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
	R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
	R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
	R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
	R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
	R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
	R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
	R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
	R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
	R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);

	/* Add the working vars back into context.state[] */
	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;

	/* Wipe variables */
	a = b = c = d = e = 0;
}


/* SHA1Init - Initialize new context */
void SHA1Init(SHA1_CTX* context) {
	/* SHA1 initialization constants */
	context->state[0] = 0x67452301;
	context->state[1] = 0xEFCDAB89;
	context->state[2] = 0x98BADCFE;
	context->state[3] = 0x10325476;
	context->state[4] = 0xC3D2E1F0;
	context->count[0] = context->count[1] = 0;
}


/* Run your data through this. */
void SHA1Update(SHA1_CTX* context, unsigned char const * data, uint32 len) {
	uint32 i, j;

	j = (context->count[0] >> 3) & 63;
	if ((context->count[0] += len << 3) < (len << 3)) context->count[1]++;
	context->count[1] += (len >> 29);

	if ((j + len) > 63) {
		memcpy(&context->buffer[j], data, (i = 64-j));
		SHA1Transform(context->state, context->buffer);
		for ( ; i + 63 < len; i += 64) {
			SHA1Transform(context->state, &data[i]);
		}
		j = 0;
	} else
		i = 0;
	memcpy(&context->buffer[j], &data[i], len - i);
}


/* Add padding and return the message digest. */
void SHA1Final(unsigned char digest[20], SHA1_CTX* context) {
	uint32 i;
	unsigned char finalcount[8];

	for (i = 0; i < 8; i++) {
		finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)]
		 >> ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
	}
	SHA1Update(context, (unsigned char *)"\200", 1);
	while ((context->count[0] & 504) != 448) {
		SHA1Update(context, (unsigned char *)"\0", 1);
	}
	SHA1Update(context, finalcount, 8);  /* Should cause a SHA1Transform() */
	for (i = 0; i < 20; i++) {
		digest[i] = (unsigned char)
		 ((context->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
	}

	/* Wipe variables */
	i = 0;
	memset(context->buffer, 0, 64);
	memset(context->state, 0, 20);
	memset(context->count, 0, 8);
	memset(&finalcount, 0, 8);

#ifdef SHA1HANDSOFF  /* make SHA1Transform overwrite it's own static vars */
	SHA1Transform(context->state, context->buffer);
#endif
}

/***********************************************************************/

int sha1_encrypt(const char *src, int len, char *dest, int size) {
	SHA1_CTX context;
	char tmp[41];

	if (size < 20)
		return -1;

	memset(dest,0,size);

	SHA1Init(&context);
	SHA1Update(&context, (const unsigned char *)src, len);
	SHA1Final((unsigned char *)dest, &context);

	if(debug) {
		memset(tmp,0,41);
		binary_to_hex((unsigned char *)dest,tmp,20);

		/* Dont log source if we were encrypting in place :) */
		if (memcmp(src, dest, 20) != 0) {
			alog("[enc_switchover] sha1: hashed from [%s] to [%s]",src,tmp);
		} else {
			alog("[enc_switchover] sha1: hashed password to [%s]",tmp);
		}
	}

	return 0;
}


int sha1_encrypt_in_place(char *buf, int size) {
	char tmp[41];
	memset(tmp,0,41);
	if(sha1_encrypt(buf, strlen(buf), tmp, size)==0) {
		memcpy(buf,tmp,size);
	} else {
		return -1;
	}
	return 0;
}


int sha1_encrypt_check_len(int passlen, int bufsize) {
	if (bufsize < 20)
		fatal("[enc_switchover] sha1: sha1_check_len(): buffer too small (%d)", bufsize);
	return 0;
}


int sha1_decrypt(const char *src, char *dest, int size) {
	return 0;
}


int sha1_check_password(const char *plaintext, const char *password) {
	char buf[BUFSIZE];

	if (sha1_encrypt(plaintext, strlen(plaintext), buf, sizeof(buf)) < 0)
		return -1;
	if (memcmp(buf, password, 20) == 0)
		return 1;
	return 0;
}

/* EOF */
