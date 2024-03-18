/**
 * English Language file.
 *
 ***********
 * Module Name    : bs_fantasy_ext
 * Author         : Viper <Viper@Absurd-IRC.net>
 * Creation Date  : 17/06/2007
 *
 * More info on http://modules.anope.org and http://forum.anope.org
 ***********
 *
 * English language file maintained by Module Author.
 *
 ***********
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***********
 *
 * Last Updated   : 17/06/2007
 *
 **/

void add_english(void);

void add_english(void) {
	char *langtable_en_us[] = {
		/* LANG_HELP */
		"SYNTAX: %shelp\n"
		"This is the help for the botserv fantasy commands.",
		/* LANG_CLEAR_SYNTAX */
		"SYNTAX: %sclear [modes|bans|exepts|invites|ops|hops|voices|users]",
		/* LANG_EXCEPTS_UNSUPPORTED */
		"Channel Excepts are not supported on this network.",
		/* LANG_INVITEMODE_UNSUPPORTED */
		"Channel Invites are not supported on this network.",
		/* LANG_HOPS_UNSUPPORTED */
		"Halfops are not supported on this network.",
		/* LANG_SOP_SYNTAX */
		"SYNTAX: %ssop {add|del|list|clear} [nick|entry-list]",
		/* LANG_SOP_SYNTAX_EXT */
		"SYNTAX: \002%ssop\002 add [nick]\n"
		"        \002%ssop\002 del [nick|entry-num|list]\n"
		"        \002%ssop\002 list [mask|list]\n"
		"        \002%ssop\002 clear",
		/* LANG_AOP_SYNTAX */
		"SYNTAX: %saop {add|del|list|clear} [nick|entry-list]",
		/* LANG_AOP_SYNTAX_EXT */
		"SYNTAX: \002%saop\002 add [nick]\n"
		"        \002%saop\002 del [nick|entry-num|list]\n"
		"        \002%saop\002 list [mask|list]\n"
		"        \002%saop\002 clear",
		/* LANG_HOP_SYNTAX */
		"SYNTAX: %shop {add|del|list|clear} [nick|entry-list]",
		/* LANG_HOP_SYNTAX_EXT */
		"SYNTAX: \002%shop\002 add [nick]\n"
		"        \002%shop\002 del [nick|entry-num|list]\n"
		"        \002%shop\002 list [mask|list]\n"
		"        \002%shop\002 clear",
		/* LANG_VOP_SYNTAX */
		"SYNTAX: %svop {add|del|list|clear} [nick|entry-list]",
		/* LANG_VOP_SYNTAX_EXT */
		"SYNTAX: \002%svop\002 add [nick]\n"
		"        \002%svop\002 del [nick|entry-num|list]\n"
		"        \002%svop\002 list [mask|list]\n"
		"        \002%svop\002 clear",
		/* LANG_ACCESS_SYNTAX */
		"SYNTAX: %saccess {add|del|list|clear} [nick [level]|entry-list]",
		/* LANG_ACCESS_SYNTAX_EXT */
		"SYNTAX: \002%saccess\002 add [nick] [level]\n"
		"        \002%saccess\002 del [nick|entry-num|list]\n"
		"        \002%saccess\002 list [mask|list]"
		"        \002%saccess\002 clear",
		/* LANG_LEVELS_SYNTAX */
		"SYNTAX: %slevels {set|dis[able]|list|reset} [item [level]]",
		/* LANG_LEVELS_SYNTAX_EXT */
		"SYNTAX: \002%slevels\002 set [type] [level]\n"
		"        \002%slevels\002 {dis|disable} [type]\n"
		"        \002%slevels\002 list\n"
		"        \002%slevels\002 reset",
		/* LANG_LEVELS_ERR_NO_VAL */
		"Error: no value provided with SET parameter.",
		/* LANG_LEVELS_ERR_VAL */
		"Error: value provided was non-numerical.",
		/* LANG_ONLY_FOUNDERS */
		"Access denied: Only founders can use this command.",
		/* LANG_GOLIST_OPER_ONLY */
		"Oper",
		/* LANG_GOLIST_OPER_AND_SO */
		"Oper + Services Oper",
		/* LANG_GOLIST_OPER_AND_SA */
		"Oper + Services Administrator",
		/* LANG_GOLIST_OPER_AND_SRA */
		"Oper + Services Root Administrator",
		/* LANG_GOLIST_NONE */
		"No Opers are currently available",
		/* LANG_GOLIST_HEADER */
		"-----------------------------------------------------------------------\n"
		"The following global IRC Operators are online:\n"
		"-----------------------------------------------------------------------",
		/* LANG_GOLIST_FOOTER */
		"-----------------------------------------------------------------------\n"
		"Total Avalable Opers: %d \n"
		"-----------------------------------------------------------------------\n"
		" -> Always try to contact an IRCop in channel before PM'ing\n"
		" -> Try to prevent PM'ing an IRCop without getting permission first.\n"
		" -> Please /whois an IRCop before trying to communicate with them.\n"
		" -> Read (and respect) any away line information.\n"
		"-----------------------------------------------------------------------",
		/* LANG_AKICK_SYNTAX */
		"SYNTAX: %sakick {add|stick|unstick|del|list|view|enforce|clear} [nick-or-usermask] [reason]",
		/* LANG_AKICK_SYNTAX_EXT */
		"SYNTAX: \002%sakick\002 add [mask] [reason]\n"
		"        \002%sakick\002 {stick|unstick|del} mask\n"
		"        \002%sakick\002 {list|view} [mask]\n"
		"        \002%sakick\002 {enforce|clear}",
		/* LANG_BADWORDS_SYNTAX */
		"SYNTAX: %sbadwords {add|del|list|clear} [word|entry-list] [single|start|end|any]",
		/* LANG_BADWORDS_SYNTAX_EXT */
		"SYNTAX: \002%sbadwords\002 add [word] {single|start|end|any}\n"
		"SYNTAX: \002%sbadwords\002 del [word|entry-num|list]\n"
		"SYNTAX: \002%sbadwords\002 list [mask|list]\n"
		"SYNTAX: \002%sbadwords\002 clear",
		/* LANG_BADWORDS_ERR_STYLE */
		"Error: Invalid style specified. Please specify START, SINGLE, ANY or END only.",
		/* LANG_CMD_NOT_AV */
		"This command is currently not available. Please contact the Network staff for more info.",
		/* LANG_SET_SYNTAX */
		"SYNTAX: %sset [option] [parameters]\n"
		"Available options: founder, successor, desc, email, url, entrymsg, bantype, mlock,\n"
		"        keeptopic, topiclock, opnotice, peace, private, restricted, secure, secureops,\n"
		"        securefounder, signkick, xop, noexpire (limited to SA)\n"
		"        dontkickops, dontkickvoices, greet, fantasy, symbiosis, nobot (limited to SA)",
		/* LANG_SET_SYNTAX_EXT */
		"SYNTAX: \002%sset\002 [option] [parameters]\n"
		"Available options:\n"
		"        founder       -  Set the founder of a channel\n"
		"        successor     -  Set the successor for a channel\n"
		"        desc          -  Set the channel description\n"
		"        url           -  Associate a URL with the channel\n"
		"        email         -  Associate an E-mail address with the channel\n"
		"        entrymsg      -  Set a message to be sent to users when they enter the channel\n"
		"        bantype       -  Set how Services make bans on the channel\n"
		"        mlock         -  Lock channel modes on or off\n"
		"        keeptopic     -  Retain topic when channel is not in use\n"
		"        topiclock     -  Topic can only be changed with TOPIC\n"
		"        opnotice      -  Send a notice when OP/DEOP commands are used\n"
		"        peace         -  Regulate the use of critical commands\n"
		"        private       -  Hide channel from LIST command\n"
		"        restricted    -  restrict access to the channel\n"
		"        secure        -  activate services' security features\n"
		"        secureops     -  stricter control of chanop status\n"
		"        securefounder -  stricter control of channel founder status\n"
		"        signkick      -  Sign kicks that are done with KICK command\n"
		"        xop           -  Toggle the user privilege system\n"
		"        noexpire      -  Sets whether the given channel will expire. (limited to SA)\n"
		" \n"
		"        dontkickops   -  Protect channel ops against kicks by the services bot.\n"
		"        dontkickvoices - Protect voices against kicks by the services bot.\n"
		"        greet         -  Enable greet messages on join.\n"
		"        fantasy       -  Enable fantasy commands on the channel. (read help first !)\n"
		"        symbiosis     -  Allow the bot to act as a real bot.\n"
		"        nobot         -  Prevents a bit from being assigned to this channel. (limited to SA)",
		/* LANG_SET_FOUNDER_SYNTAX */
		"SYNTAX: %sset founder [nick]",
		/* LANG_SET_FOUNDER_SYNTAX_EXT */
		"SYNTAX: \002%sset founder\002 [nick]\n"
		"Changes the founder of a channel. The new nickname must be a registered one.",
		/* LANG_SET_SUCCESSOR_SYNTAX */
		"SYNTAX: %sset successor [nick]",
		/* LANG_SET_SUCCESSOR_SYNTAX_EXT */
		"SYNTAX: \002%sset successor\002 [nick]\n"
		"Changes the successor of a channel. If the founder's nickname expires or is dropped\n"
		"while the channel is still registered, the successor will become the new founder of the channel.\n"
		"The new nickname must be a registered one.",
		/* LANG_SET_DESC_SYNTAX */
		"SYNTAX: %sset desc [description]",
		/* LANG_SET_DESC_SYNTAX_EXT */
		"SYNTAX: \002%sset desc\002 [description]\n"
		"Sets the description for the channel, which shows up with the LIST and INFO commands.",
		/* LANG_SET_URL_SYNTAX */
		"SYNTAX: %sset url [url]",
		/* LANG_SET_URL_SYNTAX_EXT */
		"SYNTAX: \002%sset url\002 [url]\n"
		"Associates the given URL with the channel.\n"
		"This URL will be displayed whenever someone requests information on the channel with the INFO command.\n"
		"If no parameter is given, deletes any current URL for the channel.",
		/* LANG_SET_EMAIL_SYNTAX */
		"SYNTAX: %sset email [email]",
		/* LANG_SET_EMAIL_SYNTAX_EXT */
		"SYNTAX: \002%sset email\002 [email]\n"
		"Associates the given E-mail address with the channel.\n"
		"This address will be displayed whenever someone requests information on the channel with the INFO command.\n"
		"If no parameter is given, deletes any current E-mail address for the channel.",
		/* LANG_SET_ENTRYMSG_SYNTAX */
		"SYNTAX: %sset entrymsg [entry-message]\n",
		/* LANG_SET_ENTRYMSG_SYNTAX_EXT */
		"SYNTAX: \002%sset entrymsg\002 [entry-message]\n"
		"Sets the message which will be sent via /notice to userswhen they enter the channel.\n"
		"If no parameter is given, causes no message to be sent upon entering the channel.",
		/* LANG_SET_BANTYPE_SYNTAX */
		"SYNTAX: %sset bantype [0|1|2|3]",
		/* LANG_SET_BANTYPE_SYNTAX_EXT */
		"SYNTAX: \002%sset bantype\002 [0|1|2|3]\n"
		"Sets the ban type that will be used by services whenever they need to ban someone from your channel.\n"
		"\n"
		"bantype is a number between 0 and 3 that means:\n"
		"0: ban in the form *!user@host\n"
		"1: ban in the form *!*user@host\n"
		"2: ban in the form *!*@host\n"
		"3: ban in the form *!*user@*.domain",
		/* LANG_SET_MLOCK_SYNTAX */
		"SYNTAX: %sset mlock [modes]",
		/* LANG_SET_MLOCK_SYNTAX_EXT */
		"SYNTAX: \002%sset mlock\002 [modes]\n"
		"Sets the mode-lock parameter for the channel.\n"
		"This allows you to define certain channel modes to be always on, off or free to be either on or off.\n",
		/* LANG_SET_KEEPTOPIC_SYNTAX */
		"SYNTAX: %sset keeptopic [on|off]",
		/* LANG_SET_KEEPTOPIC_SYNTAX_EXT */
		"SYNTAX: \002%sset keeptopic\002 [on|off]\n"
		"Enables or disables the topic retention option for a channel. When topic retention is set,\n"
		"the topic for the channel will be remembered even after the  last user leaves the channel,\n"
		"and will be restored the next time the channel is created.",
		/* LANG_SET_TOPICLOCK_SYNTAX */
		"SYNTAX: %sset topiclock [on|off]",
		/* LANG_SET_TOPICLOCK_SYNTAX_EXT */
		"SYNTAX: \002%sset topiclock\002 [on|off]\n"
		"Enables or disables the topic lock option for a channel. When topic lock is set,\n"
		"services will not allow the channel topic to be changed except via the TOPIC command.",
		/* LANG_SET_OPNOTICE_SYNTAX */
		"SYNTAX: %sset opnotice [on|off]",
		/* LANG_SET_OPNOTICE_SYNTAX_EXT */
		"SYNTAX: \002%sset opnotice\002 [on|off]\n"
		"Enables or disables the op-notice option for a channel. When op-notice is set,\n"
		"services will send a notice to the channel whenever the OP or DEOP commands are used.",
		/* LANG_SET_PEACE_SYNTAX */
		"SYNTAX: %sset peace [on|off]",
		/* LANG_SET_PEACE_SYNTAX_EXT */
		"SYNTAX: \002%sset peace\002 [on|off]\n"
		"Enables or disables the peace option for a channel. When peace is set,\n"
		"a user won't be able to kick, ban or remove a channel status of a user that has\n"
		"a level superior or equal to his via services.",
		/* LANG_SET_PRIVATE_SYNTAX */
		"SYNTAX: %sset private [on|off]",
		/* LANG_SET_PRIVATE_SYNTAX_EXT */
		"SYNTAX: \002%sset private\002 [on|off]\n"
		"Enables or disables the private option for a channel.\n"
		"When private is set, a /%s LIST will not include the channel in any lists.",
		/* LANG_SET_RESTRICTED_SYNTAX */
		"SYNTAX: %sset restricted [on|off]",
		/* LANG_SET_RESTRICTED_SYNTAX_EXT */
		"SYNTAX: \002%sset restricted\002 [on|off]\n"
		"Enables or disables the restricted access option for a channel.\n"
		"When restricted access is set, users who would normally be disallowed from having channel operator\n"
		"privileges (users with negative access levels and, if \002secure ops\002 is set, users not on the access list)\n"
		"will instead be kicked and banned from the channel.",
		/* LANG_SET_SECURE_SYNTAX */
		"SYNTAX: %sset secure [on|off]",
		/* LANG_SET_SECURE_SYNTAX_EXT */
		"SYNTAX: \002%sset secure\002 [on|off]\n"
		"Enables or disables ChanServ's security features for a channel. When SECURE is set, only users who have\n"
		"registered their nicknames with services and IDENTIFY'd with their password will be given access\n"
		"to the channel as controlled by the access list.",
		/* LANG_SET_SECUREOPS_SYNTAX */
		"SYNTAX: %sset secureops [on|off]",
		/* LANG_SET_SECUREOPS_SYNTAX_EXT */
		"SYNTAX: \002%sset secureops\002 [on|off]\n"
		"Enables or disables the secure ops option for a channel.\n"
		"When secure ops is set, users who are not on the userlist will not be allowed chanop status.",
		/* LANG_SET_SECUREFOUNDER_SYNTAX */
		"SYNTAX: %sset securefounder [on|off]",
		/* LANG_SET_SECUREFOUNDER_SYNTAX_EXT */
		"SYNTAX: \002%sset securefounder\002 [on|off]\n"
		"Enables or disables the secure founder option for a channel. When secure founder is set,\n"
		"only the real founder will be able to drop the channel, change its password, its founder and its\n"
		"successor, and not those who are IDENTIFY'd with services.",
		/* LANG_SET_SIGNKICK_SYNTAX */
		"SYNTAX: %sset signkick [on|off]",
		/* LANG_SET_SIGNKICK_SYNTAX_EXT */
		"SYNTAX: \002%sset signkick\002 [on|off]\n"
		"Enables or disables signed kicks for a channel.  When SIGNKICK is set, kicks issued with\n"
		"services' KICK command will have the nick that used the command in their reason.\n"
		"If you use LEVEL, those who have a level that is superior or equal to the SIGNKICK levelon the\n"
		"channel won't have their  kicks signed. See '%shelp levels' for more information.",
		/* LANG_SET_XOP_SYNTAX */
		"SYNTAX: %sset xop [on|off]",
		/* LANG_SET_XOP_SYNTAX_EXT */
		"SYNTAX: \002%sset xop\002 [on|off]\n"
		"Enables or disables the xOP lists system for a channel. When XOP is set, you have to use the AOP/SOP/VOP\n"
		"commands in order to give channel privileges to users, else you have to use the ACCESS command.\n",
		/* LANG_SET_NOEXPIRE_SYNTAX */
		"SYNTAX: %sset noexpire [on|off]",
		/* LANG_SET_NOEXPIRE_SYNTAX_EXT */
		"SYNTAX: \002%sset noexpire\002 [on|off]\n"
		"Sets whether the given channel will expire. Setting this to ON prevents the channel from expiring.\n"
		" \n"
		"This command is limited to Services Administrators.",
		/* LANG_TOPIC_SYNTAX */
		"SYNTAX: %stopic [topic]",
		/* LANG_TOPIC_SYNTAX_EXT */
		"SYNTAX: \002%stopic\002 [topic]\n"
		"Sets the channel topic. If no topic is given, an empty topic is set.\n"
		"This command is most useful in conjunction with \002SET TOPICLOCK\002",
		/* LANG_INVITE_SYNTAX */
		"SYNTAX: %sinvite [nick]",
		/* LANG_INVITE_SYNTAX_EXT */
		"SYNTAX: \002%sinvite\002 [nick]\n"
		"Invites 'nick' to the channel if the user is present on the network",
		/* LANG_INVITE_NO_USER */
		"No such user is currently online.",
		/* LANG_INVITE_IS_ON */
		"User is allready on channel!",
		/* LANG_INVITE_YOURSELF */
		"You can't invite yourself!",
		/* LANG_CMD_LIST_HEADER */
		"Available fantasy Commands:",
		/* LANG_CMD_LIST_FOOTER */
		"Note that not all possible functions of services are covered here...",
		/* LANG_CMD_LIST_GENERAL */
		"General commands: %s",
		/* LANG_CMD_LIST_CHAN_AXX */
		"Channel access commands: %s",
		/* LANG_CMD_LIST_CHG_SET */
		"Channel settings: %s",
		/* LANG_CMD_LIST_ACTIONS */
		"Channel Actions: %s",
		/* LANG_CMD_LIST_IRCOP */
		"IRC Operator Commands: %s",
		/* LANG_UNKNOWN_CMD */
		"Unknown command: \002%s\002",
		/* LANG_HELP_HEADER */
		"----- Fantasy Commands Help -----",
		/* LANG_HELP_FOOTER */
		"--- End of cmd listing ---",
		/* LANG_APPENDTOPIC_SYNTAX */
		"SYNTAX: %sappendtopic or %stappend [text to be appended to topic]",
		/* LANG_APPEND_SYNTAX_EXT */
		"SYNTAX: \002%sappendtopic\002 or \002%stappend\002 [text to be appended to topic]\n"
		"Appends the given text to the channel's current topic.\n"
		"This command is most useful in conjunction with \002SET TOPICLOCK\002",
		/* LANG_STAFF */
		"SYNTAX: \002%sstaff\002 or \002%sircops\002\n"
		"This will return  list of available global IRCops.",
		/* LANG_CMDLIST */
		"SYNTAX: \002%scmdlist\002\n"
		"Shows avalable fantasy commands",
		/* LANG_MINFO */
		"SYNTAX: \002%sminfo\002 (or %smodinfo)\n"
		"Shows information about the module providing the brunt of the fantasy commands.",
		/* LANG_HELP_UNKNOWN_CMD */
		"No help is available for this command.",
		/* LANG_BAN_SYNTAX */
		"SYNTAX: %sban [nick|mask]",
		/* LANG_BAN_SYNTAX_EXT */
		"SYNTAX: \002%sban\002 [nick|mask]\n"
		"Bans the given nickname or usermask from the channel.",
		/* LANG_MUTE_SYNTAX */
		"SYNTAX: %smute [nick|mask] (IRCd Dependant)",
		/* LANG_MUTE_SYNTAX_EXT */
		"SYNTAX: \002%smute\002 [nick|mask]\n"
		"Bans or mutes the given nickname or usermask from the channel. (IRCd Dependant)",
		/* LANG_UNMUTE_SYNTAX */
		"SYNTAX: %sunmute [nick|mask] (IRCd Dependant)",
		/* LANG_UNMUTE_SYNTAX_EXT */
		"SYNTAX: \002%sunmute\002 [nick|mask]\n"
		"Unbans or unmutes the given nickname or usermask from the channel. (IRCd Dependant)",
		/* LANG_SVERSION */
		"SYNTAX: \002%ssversion\002\n"
		"Shows information about the Anope IRC Services version and build.",
		/* LANG_KILL_SYNTAX */
		"SYNTAX: %skill [nick] [reason]",
		/* LANG_KILL_SYNTAX_EXT */
		"SYNTAX: \002%skill\002 [nick] [reason]\n"
		"This will forcefully disconnect the user from the network (same as /kill).\n"
		"IRC Operators only. This command may be disabled",
		/* LANG_MODE_SYNTAX */
		"SYNTAX: %smode [mode(s)]",
		/* LANG_MODE_SYNTAX_EXT */
		"SYNTAX: \002%smode\002 [mode(s)]\n"
		"This will set the specified modes on the channel\n"
		"IRC Operators only. This command may be disabled",
		/* LANG_AKILL_SYNTAX */
		"SYNTAX: %sakill AKILL {add|del|list|view|clear} [[+expiry] {mask|entry-list} [reason]]",
		/* LANG_AKILL_SYNTAX_EXT */
		"SYNTAX: \002%sakill\002 add [+expiry] mask reason\n"
		"SYNTAX: \002%sakill\002 del {mask|entry-num|list}\n"
		"SYNTAX: \002%sakill\002 [list|view] [mask|list]\n"
		"SYNTAX: \002%sakill\002 clear\n"
		"IRC Operators only. This command may be disabled",
		/* LANG_IGNORE_SYNTAX */
		"Syntax: %signore {add|del|list|clear} [time] [nick]",
		/* LANG_IGNORE_SYNTAX_EXT */
		"SYNTAX: \002%signore\002 {add|del} [expiry] [nick]\n"
		"SYNTAX: \002%signore\002 {list|clear}\n"
		"Allows Services Admins to make Services ignore a nick for a\n"
		"certain time or until the next restart. The default time\n"
		"format is seconds. Combinations of time units are not permitted.\n"
		"To make Services ignore ignore the user untill next restart type 0 as time.",
		/* LANG_INFO_SYNTAX */
		"Syntax: %sinfo [all]",
		/* LANG_INFO_SYNTAX_EXT */
		"SYNTAX: \002%sinfo\002 [all]\n"
		"Lists information about the channel.\n"
		"The 'all' option is limited to founders and Services Admins.",
		/* LANG_CMD_DISABLED */
		"This command has been disabled. Pls Contact Network Staff for more info.",
		/* LANG_HELP_CMDS_GEN */
		"General Commands:\n"
		"   %shelp      -   Help for botserv fantasy commands\n"
		"   %scmdlist   -   A list of all available fantasy commands\n"
		"   %sircops    -   Shows a list of all available global IRCops\n"
		"   %sstaff     -   Shows a list of all available global IRCops\n"
		"   %sminfo     -   Shows info about this module\n"
		"   %ssversion  -   Shows info Anope in general\n"
		"   %sinfo      -   Shows information about the channel\n"
		"Note: These commands may be unavailable, disabled or access to them may be restricted\n",
		/* LANG_HELP_CMDS_ACC */
		"Access Control Commands:\n\n"
		"   %ssop       -   Provides access to the SOP (SuperOP) list\n"
		"   %saop       -   Provides access to the AOP (AutoOP) list\n"
		"   %shop       -   Provides access to the HOP (HalfOP) list (May not be supported on this network)\n"
		"   %svop       -   Provides access to the VOP (VOicePeople) list\n"
		"   %saccess    -   Provides access to the access list (when XOP is OFF)\n"
		"   %slevels    -   Provides access to the levels list controlling the rights of each access level\n"
		"Note: These commands may be unavailable, disabled or access to them may be restricted\n",
		/* LANG_HELP_CMDS_SETTINGS */
		"Channel Setting Commands: \n"
		"   %sakick     -   Maintains the akick list\n"
		"   %sbadwords  -   Maintains the badwords list\n"
		"   %sset       -   Changes channel's settings. (cf. '%shelp set')\n"
		"   %sbkick     -   Change the botserv kick settings.(cf. '%shelp bkick')\n"
		"Note: These commands may be unavailable, disabled or access to them may be restricted\n",
		/* LANG_HELP_CMDS_ACTIONS */
		"Channel Action Commands:\n"
		"   %sclear     -   Clears the specified option on the channel.\n"
		"   %stopic     -   Sets the channel's topic\n"
		"   %stappend   -   Appends text to the topic\n"
		"   %sinvite    -   Invites a user into the channel\n"
		"   %sup        -   Gives the user the modes he has access to\n"
		"   %sdown      -   Removes all the users' modes\n"
		"   %skick      -   Kick a user from the channel\n"
		"   %skb        -   Kick and ban (a) user(s) from the channel\n"
		"   %stkb       -   Kick and temporarily ban (a) user(s) from the channel\n"
		"Note: These commands may be unavailable, disabled or access to them may be restricted\n",
		/* LANG_HELP_CMDS_PUTMODES */
		"Channel Modes Commands:\n"
		"   %sban       -   Bans (a) user(s)/mask from the channel without kicking\n"
		"   %stban      -   Temporarily bans (a) user(s)/mask from the channel without kicking\n"
		"   %sunban     -   Removes the ban on a user.\n"
		"   %smute      -   Mutes / Bans the user (IRCd Dependant)\n"
		"   %sunmute    -   Unmutes / unbans the user (IRCd Dependant)\n"
		"   %sop        -   Op yourself or another user\n"
		"   %sdeop      -   Deop yourself or another user\n"
		"   %svoice     -   Voice yourself or another user\n"
		"   %sdevoice   -   Devoice yourself or another user\n"
		"   %sprotect   -   Protect yourself or another user (May not be supported on this network)\n"
		"   %sdeprotect -   Deprotect yourself or another user (May not be supported on this network)\n"
		"   %sowner     -   Give yourself the owner mode (May not be supported on this network)\n"
		"   %sdeowner   -   Remove your owner mode (May not be supported on this network)\n"
		"   %shalfop    -   Halfop yourself or another user (May not be supported on this network)\n"
		"   %sdehalfop  -   Dehalfop yourself or another user (May not be supported on this network)\n"
		"Note: These commands may be unavailable, disabled or access to them may be restricted\n",
		/* LANG_HELP_CMDS_IRCOP */
		"IRC Operator Commands:\n"
		"   %sakill     -   Will force a ban on a mask for the entire network.\n"
		"   %skill      -   'Kills' the user\n"
		"   %smode      -   Changes the channels' modes\n"
		"   %signore    -   Makes services ignore a user\n"
		"Note: These commands may be unavailable, disabled or access to them may be restricted\n",
		/* LANG_HELP_MORE_INFO */
		"For more info on a command type %shelp <command>",
		/* LANG_HELP_LIST */
		"To get a list of available commands for what you want to do, type the following: \n"
		" \n"
		"  %shelp general  - Shows general commands available. No special access required.\n"
		"  %shelp chaccess - Shows commands to manipulate channel access and levels list(s).\n"
		"  %shelp config   - Configure the channels' settings and lists.\n"
		"  %shelp act      - Commands that will make services perform certain actions.\n"
		"  %shelp ircop    - Commands only available to IRC Operators.\n"
		" \n"
		"To get a list of \002all\002 available commands, type '%shelp all'",
		/* LANG_SET_DONTKICKOPS_SYNTAX */
		"SYNTAX: %sset dontkickops {on | off}",
		/* LANG_SET_DONTKICKOPS_SYNTAX_EXT */
		"SYNTAX: \002%sset dontkickops\002 {on | off}\n"
		"Enables or disables OP protection on the channel. When it is enabled,\n"
		"ops won't be kicked by the service bot even if they don't match the NOKICK level.",
		/* LANG_SET_DONTKICKVOICES_SYNTAX */
		"SYNTAX: %sset dontkickvoices {on | off}",
		/* LANG_SET_DONTKICKVOICES_SYNTAX_EXT */
		"SYNTAX: \002%sset dontkickvoices\002 {on | off}\n"
		"Enables or disables VOICE protection on the channel. When it is enabled,\n"
		"voices won't be kicked by the service bot even if they don't match the NOKICK level.",
		/* LANG_SET_GREET_SYNTAX */
		"SYNTAX: %sset greet [on|off]",
		/* LANG_SET_GREET_SYNTAX_EXT */
		"SYNTAX: \002%sset greet\002 {on | off}\n"
		"Enables or disables greet mode on a channel. When it is enabled, the bot will display greet\n"
		"messages of users joining the channel, provided they have enough access to the channel.",
		/* LANG_SET_FANTASY_SYNTAX */
		"SYNTAX: %sset fantasy [on|off]",
		/* LANG_SET_FANTASY_SYNTAX_EXT */
		"SYNTAX: \002%sset fantasy\002 {on | off}\n"
		"Enables or disables fantasy mode on a channel. When it is enabled, users will be able to use\n"
		"commands !op, !cmdlist and !help. Note that users wanting to use fantaisist ommands MUST have\n"
		"enough level for both the FANTASIA and another level depending of the command if required\n"
		" \n"
		"IMPORTANT: Disabling this will disable all fantasy commands on this channel so re-enabling\n"
		"will have to be done through botserv.",
		/* LANG_SET_SYMBIOSIS_SYNTAX */
		"SYNTAX: %sset symbiosis [on|off]",
		/* LANG_SET_SYMBIOSIS_SYNTAX_EXT */
		"SYNTAX: \002%sset symbiosis\002 {on | off}\n"
		"Enables or disables symbiosis mode on a channel. When it is enabled, the bot will do everything\n"
		"normally done by ChanServ on channels, such as MODEs, KICKs, and even the entry message.\n"
		"Note: When disabled, most fantasy commands will still be handled by the bot.",
		/* LANG_SET_NOBOT_SYNTAX */
		"SYNTAX: %sset nobot [on|off]",
		/* LANG_SET_NOBOT_SYNTAX_EXT */
		"SYNTAX: \002%sset nobot\002 {on | off}\n"
		"This option makes a channel be unassignable. If a bot is already assigned to the channel,\n"
		" it is unassigned automatically when you enable the option.",
		/* LANG_BKICK_SYNTAX */
		"SYNTAX: %sbkick [option] {on | off} [ttb [params]]\n"
		"Available options: bolds, badwords, caps, colors, flood, repeat, reverses, underlines.",
		/* LANG_BKICK_SYNTAX_EXT */
		"SYNTAX: \002%sbkick\002 [option] {on | off} [ttb [params]]\n"
		"Available options:\n"
		"        bolds       -  Sets whether the bot kicks bolds.\n"
		"        badwords    -  Sets whether the bots kicks for words in the badwords list.\n"
		"        caps        -  Sets whether the bot kicks for CAPS.\n"
		"        colors      -  Sets whether the bot kicks for colors.\n"
		"        flood       -  Sets whether the bot kicks for floods.\n"
		"        repeat      -  Sets whether the bot kicks for repeats.\n"
		"        reverses    -  Sets whether the bot kicks for reverses.\n"
		"        underlines  -  Sets whether the bot kicks for underlines.\n"
		" \n"
		"ttb is the number of times a user can be kicked before he gets banned.\n"
		"Don't give ttb to disable the ban system once activated.",
		/* LANG_BKICK_BOLDS_SYNTAX */
		"SYNTAX: %sbkick bolds {on | off} [ttb]",
		/* LANG_BKICK_BOLDS_SYNTAX_EXT */
		"SYNTAX: \002%sbkick bolds\002 {on | off} [ttb]\n"
		"When enabled, this option tells the bot to kick users who use bolds.\n"
		"services will send a notice to the channel whenever the OP or DEOP commands are used.\n"
		"ttb is the number of times a user can be kicked before he gets banned.\n"
		"Don't give ttb to disable the ban system once activated.",
		/* LANG_BKICK_BADWORDS_SYNTAX */
		"SYNTAX: %sbkick badwords {on | off} [ttb]",
		/* LANG_BKICK_BADWORDS_SYNTAX_EXT */
		"SYNTAX: \002%sbkick badwords\002 {on | off} [ttb]\n"
		"When enabled, this option tells the bot to kick users who say certain words on the channel.\n"
		"You can define bad words for your channel using the %sbadwords command.\n"
		"ttb is the number of times a user can be kicked before he gets banned.\n"
		"Don't give ttb to disable the ban system once activated.",
		/* LANG_BKICK_CAPS_SYNTAX */
		"SYNTAX: %sbkick caps {on | off} [ttb [min [percent]]]",
		/* LANG_BKICK_CAPS_SYNTAX_EXT */
		"SYNTAX: \002%sbkick caps\002 {on | off} [ttb [min [percent]]]\n"
		"When enabled, this option tells the bot to kick users who are talking in CAPS.\n"
		"The bot kicks only if there are at least \002min\002 caps and they constitute at least\n"
		"\002percent%\002 of the total text line (if not given, it defaults to 10 characters and 25%).\n"
		"ttb is the number of times a user can be kicked before he gets banned.\n"
		"Don't give ttb to disable the ban system once activated.",
		/* LANG_BKICK_COLORS_SYNTAX */
		"SYNTAX: %sbkick colors {on | off} [ttb]",
		/* LANG_BKICK_COLORS_SYNTAX_EXT */
		"SYNTAX: \002%sbkick colors\002 {on | off} [ttb]\n"
		"When enabled, this option tells the bot to kick users who use colors.\n"
		"ttb is the number of times a user can be kicked before he gets banned.\n"
		"Don't give ttb to disable the ban system once activated.",
		/* LANG_BKICK_FLOOD_SYNTAX */
		"SYNTAX: %sbkick flood {on | off} [ttb [ln [secs]]]",
		/* LANG_BKICK_FLOOD_SYNTAX_EXT */
		"SYNTAX: \002%sbkick flood\002 {on | off} [ttb [ln [secs]]]\n"
		"When enabled, this option tells the bot to kick users who are flooding the channel\n"
		"using at least \002ln\002 lines in \002secs\002 seconds. \n"
		"(if not given, it defaults to 6 lines in 10 seconds).\n"
		"ttb is the number of times a user can be kicked before he gets banned.\n"
		"Don't give ttb to disable the ban system once activated.",
		/* LANG_BKICK_REPEAT_SYNTAX */
		"SYNTAX: %sbkick repeat {on | off} [ttb [num]]",
		/* LANG_BKICK_REPEAT_SYNTAX_EXT */
		"SYNTAX: \002%sbkick repeat\002 {on | off} [ttb [num]]\n"
		"When enabled, this option tells the bot to kick users who are repeating\n"
		"themselves \002num\002 times (if num is not given, it defaults to 3)."
		"ttb is the number of times a user can be kicked before he gets banned.\n"
		"Don't give ttb to disable the ban system once activated.",
		/* LANG_BKICK_REVERSES_SYNTAX */
		"SYNTAX: %sbkick reverses {on | off} [ttb]",
		/* LANG_BKICK_REVERSES_SYNTAX_EXT */
		"SYNTAX: \002%sbkick reverses\002 {on | off} [ttb]\n"
		"When enabled, this option tells the bot to kick users who use reverses.\n"
		"ttb is the number of times a user can be kicked before he gets banned.\n"
		"Don't give ttb to disable the ban system once activated.",
		/* LANG_BKICK_UNDERLINES_SYNTAX */
		"SYNTAX: %sbkick underlines {on | off} [ttb]",
		/* LANG_BKICK_UNDERLINES_SYNTAX_EXT */
		"SYNTAX: \002%sbkick underlines\002 {on | off} [ttb]\n"
		"When enabled, this option tells the bot to kick users who use underlines.\n"
		"ttb is the number of times a user can be kicked before he gets banned.\n"
		"Don't give ttb to disable the ban system once activated.",
		/* LANG_BKICK_UNKNOWN_OPTION */
		"Unknown BKICK option: %s.",
		/* LANG_ADLIST_SA */
		"Services Administrator",
		/* LANG_ADLIST_SRA */
		"Services Root Administrator",
		/* LANG_ADLIST_NONE */
		"No Administrators are currently available",
		/* LANG_ADLIST_HEADER */
		"-----------------------------------------------------------------------\n"
		"The following Administrators are online:\n"
		"-----------------------------------------------------------------------",
		/* LANG_ADLIST_FOOTER */
		"-----------------------------------------------------------------------\n"
		"Total Avalable Admins: %d \n"
		"-----------------------------------------------------------------------\n"
		" -> Always try to contact an Admin in channel before PM'ing\n"
		" -> Try to prevent PM'ing an Admin without getting permission first.\n"
		" -> Please /whois an Admin before trying to communicate with them.\n"
		" -> Read (and respect) any away line information.\n"
		"-----------------------------------------------------------------------",
		/* LANG_KICKBAN_SYNTAX */
		"SYNTAX: %skickban or %skb [nick | [nick!]ident@host] [reason]",
		/* LANG_KICKBAN_SYNTAX_EXT */
		"SYNTAX: \002%skickban\002 or \002%skb\002 [nick | [nick!]ident@host] [reason]\n"
		"This will kick and ban the specified user or all users matching the given \n"
		"mask from the channel. If a mask is given and someone who should not be kicked \n"
		"acording to the channels' settings matches it, the command is aborted before\n"
		"anyone is kicked or banned.",
		/* LANG_UP_SYNTAX */
		"SYNTAX: %sup [nick]",
		/* LANG_UP_SYNTAX_EXT */
		"SYNTAX: \002%sup\002 [nick]\n"
		"Gives the user the modes he has rights to. This will unset modes if he has any\n"
		"the user has no access to.\n"
		" \n"
		"Note: A nick can only be specified by the founder of the channel or an SRA.",
		/* LANG_DOWN_SYNTAX */
		"SYNTAX: %sdown [nick]",
		/* LANG_DOWN_SYNTAX_EXT */
		"SYNTAX: \002%sdown\002 [nick]\n"
		"Unsets all modes. If the channel is moderated, it will reset voice,\n"
		"unless issued on another user by founder.\n"
		" \n"
		"Note: A nick can only be specified by the founder of the channel or an SRA.",
		/* LANG_UPDOWN_SYNTAX */
		"SYNTAX: %sup|%sdown [nick]",
		/* LANG_TBAN_SYNTAX	*/
		"SYNTAX: %stb or %stban [nick | [nick!]ident@host] [time]",
		/* LANG_TBAN_SYNTAX_EXT */
		"SYNTAX: \002%stb\002 or \002%stban\002 [nick | [nick!]ident@host] [time]\n"
		"This will ban the given nick or hostmask from the channel for the specified\n"
		"amount of time. If the time is not given, it will default to 1 hour.\n"
		"The time should be given in the format <digit><(s)/m/h/d>.\n"
		"Example: \002%stb Viper 10m\002 which will ban me for 10 minutes.\n"
		" \n"
		"Note: if services are restarted before the ban expires, the ban will\n"
		"not be automatically removed",
		/* LANG_TBAN_RESPONSE */
		"Temporary banned %s from %s, will auto-expire in %s.",
		/* LANG_TKBAN_SYNTAX */
		"SYNTAX: %stkb [nick | [nick!]ident@host] [time] [reason]",
		/* LANG_TKBAN_SYNTAX_EXT */
		"SYNTAX: \002%stkb\002 or \002%stkban\002 [nick | [nick!]ident@host] [time] [reason]\n"
		"This will kick and temporary ban the specified user or all users matching\n"
		"the given mask from the channel for the specified amount of time. \n"
		"If a mask is given and someone who should not be banned acording to the\n"
		"channels' settings matches it, the command is aborted before anyone is kicked.\n"
		"If the time is not given or is invalid, it will default to 1 hour.\n"
		"The time should be given in the format <digit><(s)/m/h/d>.\n"
		"Example: \002%stkb Viper 10m Bye\002 which will kickban me for 10 minutes.\n"
		" \n"
		"Note: if services are restarted before the ban expires, the ban will\n"
		"not be automatically removed.",
		/* LANG_INVALID_TIME */
		"%s is an invalid timestamp.",
		/* LANG_REQ_NICK_OR_MASK */
		"This command requires either a nickname or a valid user@host mask.",
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, langtable_en_us);
}

/* EOF */
