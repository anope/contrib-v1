MISC_SQLCMD v2.00
=================

 Author: heinz (heinz @ anope . org)
 Website: http://modules.anope.org/profile.php?id=7


WHAT IS MISC_SQLCMD?
====================

 misc_sqlcmd allows you to send commands to Anope via a table in the MySQL database. This allows for
 two-way operation and manipulation of Anope whilst it's running and connected to a network.
 
 The module operates every X seconds (default 10), and polls the command table for new instructions.
 These instructions are then processed in order they were received, and performed inside the core of
 Anope.

 
INSTALLATION:
=============

 NOTE: To install the svn-version of this module you will need a copy
 of subversion to hand.

 1) In your anope-1.x.x/src/modules folder, type:


 svn co https://teh-heinz0r.co.uk/svn/misc_sqlcmd/trunk misc_sqlcmd

 (If prompted, you'll need to accept the SSL certificate. It's
  signed by CAcert which is why subversion doesn't like it)


 3) In your main anope folder, type make modules
 4) Now type make install
 5) Add the configuration directives specified below to your services.conf
    file, and edit as required.
 6) On IRC, type /os modload misc_sqlcmd
 
 NOTE: You will need to have MySQL configured and enabled in Anope before
 you can use this module.
 
 NOTE: You will need to manually import the misc_sqlcmd.sql file into your
 main Anope database before loading this module.

 This module is continually being updated. To ensure you have the latest
 version, type 'svn up' in the misc_sqlcmd directory. If some files have 
 been updated, you will need to recompile and re-load the module, as per
 the instructions above.
 

CONFIGURATION OPTIONS:
======================

 This module has 2 configuration options which need to be added
 to the services.conf file before the module will work.

 - SQLCmdUpdateTimeout 10
	Sets the delay between polls (in seconds).
	A Lower number will cause more queries, but 
	less delay - the lower this delay, the more
	load your MySQL DB will be under.

 - SQLCmdChecksumSalt "misc_sqlcmd_module"
	Set the checksum salt used to compute the checksum of
	all incoming commands. The checksum value for each 
	command must be correct for the command to be processed.

	It is advised that you change the salt to something less
	obvious. If the salt is guessed or found, a user can add
	his or her own commands to the database (assuming they can
	access that too).
 

AVAILABLE COMMAND LIST AND PARAMETERS:
======================================

 NICK_REG - Register a nickname
 * av[0] - Nickname
 * av[1] - Password
 * av[2] - E-Mail Address
 
 NICK_CONF - Confirm a nickname
 * av[0] - Nickname
 * av[1] - Password
 * av[2] - Passcode
 
 NICK_GROUP - Group a nickname
 * av[0] - Existing Nickname
 * av[1] - Password
 * av[2] - Nick to group
 
 NICK_DROP - Drop a nickname
 * av[0] - Nickname
 * av[1] - Password

 MEMO_SEND - Send a memo
 * av[0] - Sender Nickname
 * av[1] - Sender Password
 * av[2] - Receiver Nickname
 * av[3] - Memo Text
 
 MEMO_DEL - Delete a memo/set of memos
 * av[0] - Nickname
 * av[1] - Password
 * av[2] - Memo number (one memo per command!)
 
 MEMO_CLEAR - Delete all memos
 * av[0] - Nickname
 * av[1] - Password

 BOT_ASSIGN - Assign a BotServ bot to a channel
 * av[0] - Nickname
 * av[1] - Password
 * av[2] - Channel name
 * av[3] - Channel password (if user has CA_ASSIGN level, set to "na")
 * av[4] - Bot to assign
 
 BOT_UNASSIGN - Unassign a BotServ bot from a channel
 * av[0] - Nickname
 * av[1] - Password
 * av[2] - Channel name
 * av[3] - Channel password (if user has CA_ASSIGN level, set to "na")

 BOT_SAY - Make botserv say something to a channel
 * av[0] - Nickname
 * av[1] - Password
 * av[2] - Channel name
 * av[3] - Channel password (if user has CA_SAY level, set to "na")
 * av[4] - Message to send
 
 BOT_ACT - Make botserv act something in a channel
 * av[0] - Nickname
 * av[1] - Password
 * av[2] - Channel name
 * av[3] - Channel password (if user has CA_SAY level, set to "na")
 * av[4] - Message to act

 CHAN_REG - Register a channel
 * av[0] - Nickname
 * av[1] - Password
 * av[2] - Channel name
 * av[3] - Channel password
 * av[4] - Channel description

 CHAN_ADD_SOP - Add a SOP to channel
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password (if user has access, set to "na")
 * av[4] - User to add
 
 CHAN_ADD_AOP - Add a AOP to channel
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password (if user has access, set to "na")
 * av[4] - User to add
 
 CHAN_ADD_HOP - Add a HOP to channel
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password (if user has access, set to "na")
 * av[4] - User to add
 
 CHAN_ADD_VOP - Add a VOP to channel
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password (if user has access, set to "na")
 * av[4] - User to add
 
 CHAN_DEL_SOP - Delete a SOP from channel
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password (if user has access, set to "na")
 * av[4] - User to delete
 
 CHAN_DEL_AOP - Delete a AOP from channel
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password (if user has access, set to "na")
 * av[4] - User to delete
 
 CHAN_DEL_HOP - Delete a HOP from channel
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password (if user has access, set to "na")
 * av[4] - User to delete
 
 CHAN_DEL_VOP - Delete a VOP from channel
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password (if user has access, set to "na")
 * av[4] - User to delete
 
 CHAN_TOPIC - Change the topic of a channel
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password (if user has CA_TOPIC level, set to "na")
 * av[4] - New topic
 
 CHAN_DROP - Drop a channel
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password

 PING - Ping the module
 * av[0] - A unique string  
 
THINGS THAT NEED IMPLEMENTING:
==============================

 NICK_SET_PASS - Change a nickname password
 * av[0] - Nickname
 * av[1] - Old password
 * av[2] - New password

 NICK_SET_EMAIL - Change the nickname email
 * av[0] - Nickname
 * av[1] - Password
 * av[2] - New E-mail

 NICK_SET_URL - Change the nickname URL
 * av[0] - Nickname
 * av[1] - Password
 * av[2] - New URL
 
 NICK_SET_GREET - Change the nickname greet
 * av[0] - Nickname
 * av[1] - Password
 * av[2] - New greet message
 
 NICK_SET_KILL - Set nickname KILL options
 * av[0] - Nickname
 * av[1] - Password
 * av[2] - Option [OFF/ON/QUICK/IMMED]

 NICK_SET_SECURE - Set nickname SECURE options
 * av[0] - Nickname
 * av[1] - Password
 * av[2] - Option [OFF/ON]

 CHAN_SET_PASS - Change a channel password
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password (if user has access, set to "na")
 * av[4] - New channel password

 CHAN_SET_DESC - Change a channel description
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password (if user has access, set to "na")
 * av[4] - New channel description

 CHAN_ACCESS - Add/Change/Remove access from channel
 * av[0] - Nickname
 * av[1] - Nick password
 * av[2] - Channel name
 * av[3] - Channel password (if user has access, set to "na")
 * av[4] - User
 * av[5] - Level 
 

EXAMPLE CONTROL QUERIES:
========================

 * Register a nickname:
        INSERT INTO anope_sqlcmd (`cmd`, `params`, `timestamp`, `chksum`) VALUES ('NICK_REG', 'anope2 anope1234321 heinz@anope.org', 1161885170, 'ab619673cbb6b0757de8e3ba54092dc0');
 * Send a Memo:
        INSERT INTO anope_sqlcmd (`cmd`, `params`, `timestamp`, `chksum`) VALUES ('MEMO_SEND', 'anope2 anope1234321 anope This is a test memo!', 1161885239, 'bf5bf125463b3faddb345280e78c60f7')
 * Make Botserv say something:
        INSERT INTO anope_sqlcmd (`cmd`, `params`, `timestamp`, `chksum`) VALUES ('BOT_SAY', 'anope anope1234321 #test mypass1234 Hi there, I am botserv!', 1161885298, '72fac1b65def508e36339c642adb3e93')

 For code examples of how to use this module, please see the examples directory.


STATUS VALUES:
==============

For a list of status values and what they mean, see misc_sqlcmd_errors.h


HOW THE CHECKSUM IS COMPUTED:
=============================

 The checksum allows us to verify the integrity of the commands we receive via MySQL, as long as the
 "checksum salt" is kept secret.
 
 Each message stored in the commands table needs a valid checksum to be processed, and it's computed
 by:
 
        * Taking the cmd, params and timestamp fields, and putting them in a formatted string:
        
                cmd:params:timestamp
               
        * The secret "checksum salt" (specified in the module) is then appended to the string:
        
                cmd:params:timestamp:checksum_salt
                
        * The whole string is then MD5 hashed, and stored in the checksum field.
        
 If the "checksum salt" is comprimised, then the user can manually insert his or her own commands
 into the database. The checksum salt is specified in the services.conf file, and in a secure
 location in your website scripts or client program.


MODULE IN ACTION:
=================

[Oct 26 20:40:07.701146 2006] debug: executing callback: sqlcmd
[Oct 26 20:40:07.701206 2006] debug: [misc_sqlcmd] Query: SELECT * FROM `anope_sqlcmd` WHERE `status` = 0 ORDER BY `timestamp` ASC
[Oct 26 20:40:07.701915 2006] debug: [misc_sqlcmd] There are 1 functions awaiting processing...
[Oct 26 20:40:07.702132 2006] debug: [misc_sqlcmd]  Command 36: NICK_REG - Params: my_nick mynickpass my@email.address - Timestamp: 1161891605 - Checksum: 2bcca71d038c9cb7b3407f8767a6a7f1
[Oct 26 20:40:07.702214 2006] debug: [misc_sqlcmd] Query: SELECT MD5('NICK_REG:my_nick mynickpass my@email.address:1161891605:misc_sqlcmd_module')
[Oct 26 20:40:07.702528 2006] debug: [misc_sqlcmd] Command 36 has a valid checksum - Processing...
[Oct 26 20:40:07.702669 2006] debug: [misc_sqlcmd] Processing nickname registration for 'my_nick'
[Oct 26 20:40:07.702759 2006] NickServ: Nick my_nick has been requested
[Oct 26 20:40:07.702823 2006] debug: [misc_sqlcmd] Nickname request for 'my_nick' generated.. Auto-confirming..
[Oct 26 20:40:07.702886 2006] debug: [misc_sqlcmd] Confirming nickname request for 'my_nick'...
[Oct 26 20:40:07.702950 2006] NickServ: group my_nick has been created
[Oct 26 20:40:07.703019 2006] enc_md5: hashed from [mynickpass] to [068C946B628E1232934CA74E8DFFC7AC]
[Oct 26 20:40:07.703216 2006] debug: Emitting event "nick_registered" (1 args)
[Oct 26 20:40:07.703274 2006] debug: [misc_sqlcmd] Nickname registration for 'my_nick' completed successfully!
[Oct 26 20:40:07.703346 2006] debug: [misc_sqlcmd] Command processed with status 1: Command completed successfully!
[Oct 26 20:40:07.703408 2006] debug: [misc_sqlcmd] Query: UPDATE `anope_sqlcmd` SET `status` = 1, `status_msg` = 'Command completed successfully!' WHERE `id` = 36
[Oct 26 20:40:07.703946 2006] debug: added module CallBack: [sqlcmd] due to execute at 1161891617
 
[Oct 26 20:47:45.001434 2006] debug: executing callback: sqlcmd
[Oct 26 20:47:45.001520 2006] debug: [misc_sqlcmd] Query: SELECT * FROM `anope_sqlcmd` WHERE `status` = 0 ORDER BY `timestamp` ASC
[Oct 26 20:47:45.002333 2006] debug: [misc_sqlcmd] There are 1 functions awaiting processing...
[Oct 26 20:47:45.002436 2006] debug: [misc_sqlcmd]  Command 39: NICK_REG - Params: my_nick mynickpass my@email.address - Timestamp: 1161892055 - Checksum: 44fdb41e313eb51021950c07285d7cee
[Oct 26 20:47:45.002484 2006] debug: [misc_sqlcmd] Query: SELECT MD5('NICK_REG:my_nick mynickpass my@email.address:1161892055:misc_sqlcmd_module')
[Oct 26 20:47:45.002876 2006] debug: [misc_sqlcmd] Command 39 has a valid checksum - Processing...
[Oct 26 20:47:45.002955 2006] debug: [misc_sqlcmd] Processing nickname registration for 'my_nick'
[Oct 26 20:47:45.003003 2006] debug: [misc_sqlcmd] Command processed with status -4: Nickname already exists!
[Oct 26 20:47:45.003032 2006] debug: [misc_sqlcmd] Query: UPDATE `anope_sqlcmd` SET `status` = -4, `status_msg` = 'Nickname already exists!' WHERE `id` = 39
[Oct 26 20:47:45.003562 2006] debug: added module CallBack: [sqlcmd] due to execute at 1161892075
