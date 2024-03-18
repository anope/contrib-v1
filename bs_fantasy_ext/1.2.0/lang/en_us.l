##############################################################################
##### English language file.                                             #####
##############################################################################
# Module Name    : bs_fantasy_ext
# Author         : Viper <Viper@Anope.org>
# Creation Date  : 14/09/2008
# Last Updated   : 24/01/2009
#
# More info on http://modules.anope.org and http://forum.anope.org
#
##############################################################################
#
# English language file. Maintained by Module Author.
#
##############################################################################
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
##############################################################################
#
# When translating this file to another language, keep in mind that the
# order of parameters for sprintf() is fixed in the source code, so your
# messages need to take the same parameters in the same order as the
# English messages do. If you can't get a natural translation
# of a message without changing the order of the parameters, let me know
# (viper@anope.org) which message is causing a problem and I'll see
# what I can do.
#
#
# Also in help messages, please try to limit line lengths to 90 characters
# of text (not including the leading tab).  This length was chosen because
# it does not cause line wrap under default settings on most current IRC
# clients.  Remember that format characters (control-B, control-_) are not
# included in that 90-character limit (since they don't show on the user's
# screen).  Also remember that format specifiers (%s, etc.) will expand
# when displayed, so remember to take this into account; you can assume
# that the length of a pseudoclient name (ex. %NS replacement) will be eight
# characters, so reduce the maximum line length by 6 for every %xS on a
# line.
#
# To translators or those brave enough to edit this (and risk crashing):
# The following specifiers are available:
#   - %R    Will be translated to "/" or "/msg", depending on UseStrictPrivMsg.
#   - %F    Will be translated to the BSFantasyCharacter.
#   - %NS   Will be translated to the NickServ nick.
#   - %CS   Will be translated to the ChanServ nick.
#   - %OS   Will be translated to the OperServ nick.
#   - %BS   Will be translated to the BotServ nick.
#   - %MS   Will be translated to the MemoServ nick.
#   - %HS   Will be translated to the HostServ nick.
#
# Finally, remember to put a tab at the beginning of every line of text
# (including empty lines).  This has to be a tab, not spaces.
#
##############################################################################

LANG_NAME
	English

#
#
# Generic Messages
#
#

#
# Error Messages
#

LANG_ONLY_FOUNDERS
	Access denied: Only founders can use this command.

LANG_EXCEPTS_UNSUPPORTED
	Channel Excepts are not supported on this network.
LANG_INVITEMODE_UNSUPPORTED
	Channel Invites are not supported on this network.
LANG_HOPS_UNSUPPORTED
	Halfops are not supported on this network.

LANG_CMD_NOT_AV
	This command is currently not available. Please contact the Network staff for more info.

LANG_CMD_DISABLED
	This command has been disabled. Pls Contact Network Staff for more info.

LANG_UNKNOWN_CMD
	Unknown command: %s

LANG_INVALID_TIME
	%s is an invalid timestamp.

LANG_REQ_NICK_OR_MASK
	This command requires either a nickname or a valid user@host mask.


# Levels
LANG_LEVELS_ERR_NO_VAL
	Error: no value provided with SET parameter.
LANG_LEVELS_ERR_VAL
	Error: value provided was non-numerical.

# Badwords
LANG_BADWORDS_ERR_STYLE
	Error: Invalid style specified. Please specify START, SINGLE, ANY or END only.

# Invite
LANG_INVITE_NO_USER
	No such user is currently online.
LANG_INVITE_IS_ON
	User is already on channel!
LANG_INVITE_YOURSELF
	You can't invite yourself!


#
# Operlist
#
LANG_GOLIST_OPER_ONLY
	Oper
LANG_GOLIST_OPER_AND_SO
	Oper + Services Oper
LANG_GOLIST_OPER_AND_SA
	Oper + Services Administrator
LANG_GOLIST_OPER_AND_SRA
	Oper + Services Root Administrator
LANG_GOLIST_NONE
	No Opers are currently available
LANG_GOLIST_HEADER
	-----------------------------------------------------------------------
	The following global IRC Operators are online:
	-----------------------------------------------------------------------
LANG_GOLIST_FOOTER
	-----------------------------------------------------------------------
	Total Available Opers: %d 
	-----------------------------------------------------------------------
	 -> Always try to contact an IRCop in channel before PM'ing
	 -> Try to prevent PM'ing an IRCop without getting permission first.
	 -> Please /whois an IRCop before trying to communicate with them.
	 -> Read (and respect) any away line information.
	-----------------------------------------------------------------------

#
# Adminlist
#
LANG_ADLIST_SA
	Services Administrator
LANG_ADLIST_SRA
	Services Root Administrator
LANG_ADLIST_NONE
	No Administrators are currently available
LANG_ADLIST_HEADER
	-----------------------------------------------------------------------
	The following Administrators are online:
	-----------------------------------------------------------------------
LANG_ADLIST_FOOTER
	-----------------------------------------------------------------------
	Total Available Admins: %d 
	-----------------------------------------------------------------------
	 -> Always try to contact an Admin in channel before PM'ing
	 -> Try to prevent PM'ing an Admin without getting permission first.
	 -> Please /whois an Admin before trying to communicate with them.
	 -> Read (and respect) any away line information.
	-----------------------------------------------------------------------

# KB
LANG_KB_KICK_DENIED
	Permission to kick %s is denied.

# TB
LANG_TBAN_RESPONSE
	Temporary banned %s from %s, will auto-expire in %s.

#TSHUN
LANG_TSHUN_ADDED
	Temporary SHUN placed on user %s.
LANG_TSHUN_DELETION
	Removed temporary SHUN on user %s.
LANG_TSHUN_TARGET_NEXIST
	User %s does not currently exist.
LANG_TSHUN_TARGET_OPER
	Command cannot be used on IRC operators.

# SHUN
LANG_SHUN_ADDED
	SHUN added for %s@%s.
LANG_SHUN_DELETION
	SHUN on %s@%s has been deleted, if it existed.

# Suspend
LANG_SUSPEND_NO_REASON
	A reason is mandatory to SUSPEND a channel on this network.

# Sync
LANG_SYNC_DONE
	Synchronized userlist with accesslist.

# Unban
LANG_UNBAN_NOTE
	Attempted to unban %s.
	However this does not guarantee a complete unban because 
	in order to safeguard the users' privacy some bans matching 
	the users IP or host may not have been removed.



#
#
# Help messages
#
#

LANG_HELP
	Syntax: %Fhelp
	This is the help for the %BS fantasy commands.
LANG_CMDLIST
	Syntax: %Fcmdlist
	Shows available fantasy commands

LANG_HELP_HEADER
	----- Fantasy Commands Help -----
LANG_HELP_FOOTER
	--- End of cmd listing ---


LANG_CMD_LIST_HEADER
	Available fantasy Commands:
LANG_CMD_LIST_FOOTER
	Note that not all possible functions of services are covered here...
LANG_CMD_LIST_GENERAL
	General commands: %s
LANG_CMD_LIST_CHAN_AXX
	Channel access commands: %s
LANG_CMD_LIST_CHG_SET
	Channel settings: %s
LANG_CMD_LIST_ACTIONS
	Channel Actions: %s
LANG_CMD_LIST_IRCOP
	IRC Operator Commands: %s

LANG_HELP_UNKNOWN_CMD
	No help is available for this command.

LANG_HELP_CMDS_GEN
	General Commands:
	   %Fhelp      -   Help for %BS fantasy commands
	   %Fcmdlist   -   A list of all available fantasy commands
	   %Fircops    -   Shows a list of all available global IRCops
	   %Fstaff     -   Shows a list of all available global IRCops
	   %Fminfo     -   Shows info about this module
	   %Fsversion  -   Shows info Anope in general
	   %Finfo      -   Shows information about the channel
	
	Note: These commands may be unavailable, disabled or access to them may be restricted,
LANG_HELP_CMDS_ACC
	Access Control Commands:\n
	   %Fsop       -   Provides access to the SOP (SuperOP) list
	   %Faop       -   Provides access to the AOP (AutoOP) list
	   %Fhop       -   Provides access to the HOP (HalfOP) list (May not be supported on this network)
	   %Fvop       -   Provides access to the VOP (VOicePeople) list
	   %Faccess    -   Provides access to the access list (when XOP is OFF)
	   %Flevels    -   Provides access to the levels list controlling the rights of each access level
	
	Note: These commands may be unavailable, disabled or access to them may be restricted,
LANG_HELP_CMDS_SETTINGS
	Channel Setting Commands: 
	   %Fakick     -   Maintains the akick list
	   %Fbadwords  -   Maintains the badwords list
	   %Fset       -   Changes channel's settings. (cf. '%Fhelp set')
	   %Fbkick     -   Change the %BS kick settings.(cf. '%Fhelp bkick')
	
	Note: These commands may be unavailable, disabled or access to them may be restricted,
LANG_HELP_CMDS_ACTIONS
	Channel Action Commands:
	   %Fclear     -   Clears the specified option on the channel.
	   %Ftopic     -   Sets the channel's topic
	   %Ftappend   -   Appends text to the topic
	   %Finvite    -   Invites a user into the channel
	   %Fup        -   Gives the user the modes he has access to
	   %Fdown      -   Removes all the users' modes
	   %Fkick      -   Kick a user from the channel
	   %Fkb        -   Kick and ban (a) user(s) from the channel
	   %Ftkb       -   Kick and temporarily ban (a) user(s) from the channel
	
	Note: These commands may be unavailable, disabled or access to them may be restricted,
LANG_HELP_CMDS_PUTMODES
	Channel Modes Commands:
	   %Fban       -   Bans (a) user(s)/mask from the channel without kicking
	   %Ftban      -   Temporarily bans (a) user(s)/mask from the channel without kicking
	   %Funban     -   Removes the ban on a user.
	   %Fmute      -   Mutes / Bans the user (IRCd Dependant)
	   %Funmute    -   Unmutes / unbans the user (IRCd Dependant)
	   %Fop        -   Op yourself or another user
	   %Fdeop      -   Deop yourself or another user
	   %Fvoice     -   Voice yourself or another user
	   %Fdevoice   -   Devoice yourself or another user
	   %Fprotect   -   Protect yourself or another user (May not be supported on this network)
	   %Fdeprotect -   Deprotect yourself or another user (May not be supported on this network)
	   %Fowner     -   Give yourself the owner mode (May not be supported on this network)
	   %Fdeowner   -   Remove your owner mode (May not be supported on this network)
	   %Fhalfop    -   Halfop yourself or another user (May not be supported on this network)
	   %Fdehalfop  -   Dehalfop yourself or another user (May not be supported on this network)
	
	Note: These commands may be unavailable, disabled or access to them may be restricted,
LANG_HELP_CMDS_IRCOP
	IRC Operator Commands:
	   %Fakill     -   Will force a ban on a mask for the entire network.
	   %Fkill      -   'Kills' the user
	   %Fmode      -   Changes the channels' modes
	   %Fignore    -   Makes services ignore a user
	
	Note: These commands may be unavailable, disabled or access to them may be restricted,

LANG_HELP_MORE_INFO
	For more info on a command type %Fhelp <command>
LANG_HELP_LIST
	To get a list of available commands for what you want to do, type the following: 
	
	  %Fhelp general  - Shows general commands available. No special access required.
	  %Fhelp chaccess - Shows commands to manipulate channel access and levels list(s).
	  %Fhelp config   - Configure the channels' settings and lists.
	  %Fhelp act      - Commands that will make services perform certain actions.
	  %Fhelp ircop    - Commands only available to IRC Operators.
	
	To get a list of all available commands, type '%Fhelp all'

# Access levels
LANG_ACCESS_LEVELS
	User access levels:
	
	By default, the following access levels are defined:
	 *** Founder *** 
	 Full access to administrative functions and opping upon entering the channel.
	 Note that only one person may have founder status (it cannot be given using the
	 ACCESS command {This is not true if cs_fop is loaded!}).
	 ***      10  *** 
	 Access to AKICK command and automatic opping.
	 ***       5  *** 
	 Automatic opping upon entering the channel.
	 ***       4  ***
	 Automatic halfopping (+h) upon entering the channel. (If supported by IRCd.)
	 ***       3  ***
	 Automatic voicing (+v) upon entering the channel.
	 ***       0  *** 
	 No special privileges; Can be opped by other ops (unless secure-ops is set).
	 ***      <0  ***
	 May not be opped.
	
	These levels may be changed, or new ones added, using the
	LEVELS command. For more information, cf. HELP LEVELS.

# Admins
LANG_ADMINS_SYNTAX
	Syntax: %Fadmins
	This will return list of all available Services and Network Administrators..

# Akick
LANG_AKICK_SYNTAX 
	Syntax: %Fakick {add | stick | unstick | del | list | view | enforce | clear} [nick-or-usermask] [reason]
LANG_AKICK_SYNTAX_EXT
	Syntax: %Fakick add {mask | user} [reason]
	        %Fakick {stick | unstick | del} mask
	        %Fakick {list | view} [mask]
	        %Fakick {enforce | clear}

# Akill
LANG_AKILL_SYNTAX
	Syntax: %Fakill AKILL {add | del | list | view | clear} [+expiry] [mask | entry-list] [reason]
LANG_AKILL_SYNTAX_EXT
	Syntax: %Fakill add [+expiry] mask reason
	        %Fakill del {mask | entry-num | list}
	        %Fakill {list | view} [mask | list]
	        %Fakill clear
	IRC Operators only. This command may be disabled.

# Append
LANG_APPENDT_SYNTAX
	Syntax: {%Fappendtopic | %Ftappend} text to be appended to topic
LANG_APPENDT_SYNTAX_EXT
	Syntax: {%Fappendtopic | %Ftappend} text to be appended to topic
	Appends the given text to the channel's current topic.
	This command is most useful in conjunction with SET TOPICLOCK

# Badwords
LANG_BADWORDS_SYNTAX
	Syntax: %Fbadwords {add | del | list | clear} [word | entry-list] [single | start | end | any]
LANG_BADWORDS_SYNTAX_EXT
	Syntax: %Fbadwords add word [single | start | end | any]
	        %Fbadwords del {word | entry-num | list}
	        %Fbadwords list [mask | list]
	        %Fbadwords clear

# Ban
LANG_BAN_SYNTAX
	Syntax: %Fban [nick | mask]
LANG_BAN_SYNTAX_EXT
	Syntax: %Fban [nick | mask]
	Bans the given nickname or usermask from the channel.

#
# BKick
#
LANG_BKICK_SYNTAX
	Syntax: %Fbkick option {on | off} [ttb [params]]
	Available options: bolds, badwords, caps, colors, flood, repeat, reverses, underlines.
LANG_BKICK_SYNTAX_EXT
	Syntax: %Fbkick option {on | off} [ttb [params]]
	Available options:
	        bolds       -  Sets whether the bot kicks bolds.
	        badwords    -  Sets whether the bots kicks for words in the badwords list.
	        caps        -  Sets whether the bot kicks for CAPS.
	        colors      -  Sets whether the bot kicks for colors.
	        flood       -  Sets whether the bot kicks for floods.
	        repeat      -  Sets whether the bot kicks for repeats.
	        reverses    -  Sets whether the bot kicks for reverses.
	        underlines  -  Sets whether the bot kicks for underlines.
	
	ttb is the number of times a user can be kicked before he gets banned.
	Don't give ttb to disable the ban system once activated.
LANG_BKICK_BOLDS_SYNTAX
	Syntax: %Fbkick bolds {on | off} [ttb]
LANG_BKICK_BOLDS_SYNTAX_EXT
	Syntax: %Fbkick bolds {on | off} [ttb]
	When enabled, this option tells the bot to kick users who use bolds.
	services will send a notice to the channel whenever the OP or DEOP commands are used.
	ttb is the number of times a user can be kicked before he gets banned.
	Don't give ttb to disable the ban system once activated.
LANG_BKICK_BADWORDS_SYNTAX
	Syntax: %Fbkick badwords {on | off} [ttb]
LANG_BKICK_BADWORDS_SYNTAX_EXT
	Syntax: %Fbkick badwords {on | off} [ttb]
	When enabled, this option tells the bot to kick users who say certain words on the channel.
	You can define bad words for your channel using the %Fbadwords command.
	ttb is the number of times a user can be kicked before he gets banned.
	Don't give ttb to disable the ban system once activated.
LANG_BKICK_CAPS_SYNTAX
	Syntax: %Fbkick caps {on | off} [ttb [min [percent]]]
LANG_BKICK_CAPS_SYNTAX_EXT
	Syntax: %Fbkick caps {on | off} [ttb [min [percent]]]
	When enabled, this option tells the bot to kick users who are talking in CAPS.
	The bot kicks only if there are at least min caps and they constitute at least
	percent% of the total text line (if not given, it defaults to 10 characters and 25%).
	ttb is the number of times a user can be kicked before he gets banned.
	Don't give ttb to disable the ban system once activated.
LANG_BKICK_COLORS_SYNTAX
	Syntax: %Fbkick colors {on | off} [ttb]
LANG_BKICK_COLORS_SYNTAX_EXT
	Syntax: %Fbkick colors {on | off} [ttb]
	When enabled, this option tells the bot to kick users who use colors.
	ttb is the number of times a user can be kicked before he gets banned.
	Don't give ttb to disable the ban system once activated.
LANG_BKICK_FLOOD_SYNTAX
	Syntax: %Fbkick flood {on | off} [ttb [ln [secs]]]
LANG_BKICK_FLOOD_SYNTAX_EXT
	Syntax: %Fbkick flood {on | off} [ttb [ln [secs]]]
	When enabled, this option tells the bot to kick users who are flooding the channel
	using at least ln lines in secs seconds. 
	(if not given, it defaults to 6 lines in 10 seconds).
	ttb is the number of times a user can be kicked before he gets banned.
	Don't give ttb to disable the ban system once activated.
LANG_BKICK_REPEAT_SYNTAX
	Syntax: %Fbkick repeat {on | off} [ttb [num]]
LANG_BKICK_REPEAT_SYNTAX_EXT
	Syntax: %Fbkick repeat {on | off} [ttb [num]]
	When enabled, this option tells the bot to kick users who are repeating
	themselves num times (if num is not given, it defaults to 3)."
	ttb is the number of times a user can be kicked before he gets banned.
	Don't give ttb to disable the ban system once activated.
LANG_BKICK_REVERSES_SYNTAX
	Syntax: %Fbkick reverses {on | off} [ttb]
LANG_BKICK_REVERSES_SYNTAX_EXT
	Syntax: %Fbkick reverses {on | off} [ttb]
	When enabled, this option tells the bot to kick users who use reverses.
	ttb is the number of times a user can be kicked before he gets banned.
	Don't give ttb to disable the ban system once activated.
LANG_BKICK_UNDERLINES_SYNTAX
	Syntax: %Fbkick underlines {on | off} [ttb]
LANG_BKICK_UNDERLINES_SYNTAX_EXT
	Syntax: %Fbkick underlines {on | off} [ttb]
	When enabled, this option tells the bot to kick users who use underlines.
	ttb is the number of times a user can be kicked before he gets banned.
	Don't give ttb to disable the ban system once activated.
LANG_BKICK_UNKNOWN_OPTION
	Unknown BKICK option: %s.


# Clear
LANG_CLEAR_SYNTAX
	Syntax: %Fclear {modes | bans | excepts | invites | ops | hops | voices | users}
LANG_CLEAR_SYNTAX_EXT
	Syntax: %Fclear [param]
	
	Following parameters are accepted:
	MODES   - Resets all modes on the channel (i.e. clears modes i,k,l,m,n,p,s,t).
	BANS    - Clears all bans on the channel.
	EXCEPTS - Clears all excepts on the channel.
	INVITES - Clears all invites on the channel.
	OPS     - Removes channel-operator status (mode +o) from all channel operators.
	HOPS    - Removes channel-halfoperator status (mode +h) from all channel halfoperators.
	VOICES  - Removes \"voice\" status (mode +v) from anyone with that mode set.
	USERS   - Removes (kicks) all users from the channel.
	
	Note: Not all parameters may be supported, depending on the IRCd.

# Down
LANG_DOWN_SYNTAX
	Syntax: %Fdown [nick] [level]
LANG_DOWN_SYNTAX_EXT
	Syntax: %Fdown [nick] [level]
	Unsets all modes. If the channel is moderated, it will reset voice,
	unless issued on another user by a founder. It is also possible
	to specify a target level. The target level can be any one of the following:
	 - q - Channel founder (modes +qo) (IRCd Dependant)
	 - a - Protected Operator / Administrator (modes +ao) (IRCd Dependant)
	 - o - Channel Operator (mode +o)
	 - h - Channel Halfop (mode +h) (IRCd Dependant)
	 - v - \"Voice\" (mode +v)
	If a target level is specified, the users' current privileges will be replaced by the target one.
	
	Note: A nick can only be specified by the founder of the channel or an SRA.

# Ignore
LANG_IGNORE_SYNTAX
	Syntax: %Fignore {add | del | list | clear} [time] [nick]
LANG_IGNORE_SYNTAX_EXT
	Syntax: %Fignore {add | del} [expiry] [nick]
	        %Fignore {list | clear}
	Allows Services Admins to make Services ignore a nick for a
	certain time or until the next restart. The default time
	format is seconds. Combinations of time units are not permitted.
	To make Services ignore ignore the user untill next restart type 0 as time.

# Info
LANG_INFO_SYNTAX
	Syntax: %Finfo [all]
LANG_INFO_SYNTAX_EXT
	Syntax: %Finfo [all]
	Lists information about the channel.
	The 'all' option is limited to founders and Services Admins.

# Invite
LANG_INVITE_SYNTAX
	Syntax: %Finvite [nick]
LANG_INVITE_SYNTAX_EXT
	Syntax: %Finvite [nick]
	Invites 'nick' to the channel if the user is present on the network

# KB
LANG_KICKBAN_SYNTAX
	Syntax: {%Fkickban | %Fkb} [nick | [nick!]ident@host] [reason]
LANG_KICKBAN_SYNTAX_EXT
	Syntax: {%Fkickban | %Fkb} [nick!]ident@host] [reason]
	This will kick and ban the specified user or all users matching the given 
	mask from the channel. If a mask is given and someone who should not be kicked 
	acording to the channels' settings matches it, the command is aborted before
	anyone is kicked or banned.

# Kill
LANG_KILL_SYNTAX
	Syntax: %Fkill nick reason
LANG_KILL_SYNTAX_EXT
	Syntax: %Fkill nick reason
	This will forcefully disconnect the user from the network (same as /kill).
	IRC Operators only. This command may be disabled

# Module Info
LANG_MINFO
	Syntax: {%Fminfo | %Fmodinfo}
	Shows information about the module providing the brunt of the fantasy commands.

# Mute / Unmute
LANG_MUTE_SYNTAX
	Syntax: %Fmute [nick | mask] (!IRCd Dependant!)
LANG_MUTE_SYNTAX_EXT
	Syntax: %Fmute [nick | mask]
	Bans or mutes the given nickname or usermask from the channel. (IRCd Dependant)

LANG_UNMUTE_SYNTAX
	Syntax: %Funmute [nick | mask] (!IRCd Dependant!)
LANG_UNMUTE_SYNTAX_EXT
	Syntax: %Funmute [nick | mask]
	Unbans or unmutes the given nickname or usermask from the channel. (IRCd Dependant)

LANG_SVERSION
	Syntax: %Fsversion
	Shows information about the Anope IRC Services version and build.
LANG_MODE_SYNTAX
	Syntax: %Fmode mode(s)
LANG_MODE_SYNTAX_EXT
	Syntax: %Fmode mode(s)
	This will set the specified modes on the channel
	IRC Operators only. This command may be disabled


#
# SET
#
LANG_SET_SYNTAX
	Syntax: %Fset option parameters
	Available options: founder, successor, desc, email, url, entrymsg, bantype, mlock,
	        keeptopic, topiclock, opnotice, peace, private, restricted, secure, secureops,
	        securefounder, signkick, xop, noexpire (limited to SA)
	        dontkickops, dontkickvoices, greet, fantasy, symbiosis, nobot (limited to SA)
LANG_SET_SYNTAX_EXT
	Syntax: %Fset option parameters
	Available options:
	        founder       -  Set the founder of a channel
	        successor     -  Set the successor for a channel
	        desc          -  Set the channel description
	        url           -  Associate a URL with the channel
	        email         -  Associate an E-mail address with the channel
	        entrymsg      -  Set a message to be sent to users when they enter the channel
	        bantype       -  Set how Services make bans on the channel
	        mlock         -  Lock channel modes on or off
	        keeptopic     -  Retain topic when channel is not in use
	        topiclock     -  Topic can only be changed with TOPIC
	        opnotice      -  Send a notice when OP/DEOP commands are used
	        peace         -  Regulate the use of critical commands
	        private       -  Hide channel from LIST command
	        restricted    -  restrict access to the channel
	        secure        -  activate services' security features
	        secureops     -  stricter control of chanop status
	        securefounder -  stricter control of channel founder status
	        signkick      -  Sign kicks that are done with KICK command
	        xop           -  Toggle the user privilege system
	        noexpire      -  Sets whether the given channel will expire. (limited to SA)
	
	        dontkickops   -  Protect channel ops against kicks by the services bot.
	        dontkickvoices - Protect voices against kicks by the services bot.
	        greet         -  Enable greet messages on join.
	        fantasy       -  Enable fantasy commands on the channel. (read help first !)
	        symbiosis     -  Allow the bot to act as a real bot.
	        nobot         -  Prevents a bit from being assigned to this channel. (limited to SA)

# Set subcommands
## Chanserv options
LANG_SET_FOUNDER_SYNTAX
	Syntax: %Fset founder nick
LANG_SET_FOUNDER_SYNTAX_EXT
	Syntax: %Fset founder nick
	Changes the founder of a channel. The new nickname must be a registered one.
LANG_SET_SUCCESSOR_SYNTAX
	Syntax: %Fset successor nick
LANG_SET_SUCCESSOR_SYNTAX_EXT
	Syntax: %Fset successor nick
	Changes the successor of a channel. If the founder's nickname expires or is dropped
	while the channel is still registered, the successor will become the new founder of the channel.
	The new nickname must be a registered one.
LANG_SET_DESC_SYNTAX
	Syntax: %Fset desc description
LANG_SET_DESC_SYNTAX_EXT
	Syntax: %Fset desc description
	Sets the description for the channel, which shows up with the LIST and INFO commands.
LANG_SET_URL_SYNTAX
	Syntax: %Fset url [url]
LANG_SET_URL_SYNTAX_EXT
	Syntax: %Fset url [url]
	Associates the given URL with the channel.
	This URL will be displayed whenever someone requests information on the channel with the INFO command.
	If no parameter is given, deletes any current URL for the channel.
LANG_SET_EMAIL_SYNTAX
	Syntax: %Fset email [email]
LANG_SET_EMAIL_SYNTAX_EXT
	Syntax: %Fset email [email]
	Associates the given E-mail address with the channel.
	This address will be displayed whenever someone requests information on the channel with the INFO command.
	If no parameter is given, deletes any current E-mail address for the channel.
LANG_SET_ENTRYMSG_SYNTAX
	Syntax: %Fset entrymsg [entry-message],
LANG_SET_ENTRYMSG_SYNTAX_EXT
	Syntax: %Fset entrymsg [entry-message]
	Sets the message which will be sent via /notice to userswhen they enter the channel.
	If no parameter is given, causes no message to be sent upon entering the channel.
LANG_SET_BANTYPE_SYNTAX
	Syntax: %Fset bantype {0 | 1 | 2 | 3}
LANG_SET_BANTYPE_SYNTAX_EXT
	Syntax: %Fset bantype {0 | 1 | 2 | 3}
	Sets the ban type that will be used by services whenever they need to ban someone from your channel.
	
	bantype is a number between 0 and 3 that means:
	0: ban in the form *!user@host
	1: ban in the form *!*user@host
	2: ban in the form *!*@host
	3: ban in the form *!*user@*.domain
LANG_SET_MLOCK_SYNTAX
	Syntax: %Fset mlock [modes]
LANG_SET_MLOCK_SYNTAX_EXT
	Syntax: %Fset mlock [modes]
	Sets the mode-lock parameter for the channel.
	This allows you to define certain channel modes to be always on, off or free to be either on or off.,
LANG_SET_KEEPTOPIC_SYNTAX
	Syntax: %Fset keeptopic {on | off}
LANG_SET_KEEPTOPIC_SYNTAX_EXT
	Syntax: %Fset keeptopic {on | off}
	Enables or disables the topic retention option for a channel. When topic retention is set,
	the topic for the channel will be remembered even after the  last user leaves the channel,
	and will be restored the next time the channel is created.
LANG_SET_TOPICLOCK_SYNTAX
	Syntax: %Fset topiclock {on | off}
LANG_SET_TOPICLOCK_SYNTAX_EXT
	Syntax: %Fset topiclock {on | off}
	Enables or disables the topic lock option for a channel. When topic lock is set,
	services will not allow the channel topic to be changed except via the TOPIC command.
LANG_SET_OPNOTICE_SYNTAX
	Syntax: %Fset opnotice {on | off}
LANG_SET_OPNOTICE_SYNTAX_EXT
	Syntax: %Fset opnotice {on | off}
	Enables or disables the op-notice option for a channel. When op-notice is set,
	services will send a notice to the channel whenever the OP or DEOP commands are used.
LANG_SET_PEACE_SYNTAX
	Syntax: %Fset peace {on | off}
LANG_SET_PEACE_SYNTAX_EXT
	Syntax: %Fset peace {on | off}
	Enables or disables the peace option for a channel. When peace is set,
	a user won't be able to kick, ban or remove a channel status of a user that has
	a level superior or equal to his via services.
LANG_SET_PRIVATE_SYNTAX
	Syntax: %Fset private {on | off}
LANG_SET_PRIVATE_SYNTAX_EXT
	Syntax: %Fset private {on | off}
	Enables or disables the private option for a channel.
	When private is set, a %R%CS LIST will not include the channel in any lists.
LANG_SET_RESTRICTED_SYNTAX
	Syntax: %Fset restricted {on | off}
LANG_SET_RESTRICTED_SYNTAX_EXT
	Syntax: %Fset restricted {on | off}
	Enables or disables the restricted access option for a channel.
	When restricted access is set, users who would normally be disallowed from having channel operator
	privileges (users with negative access levels and, if secure ops is set, users not on the access list)
	will instead be kicked and banned from the channel.
LANG_SET_SECURE_SYNTAX
	Syntax: %Fset secure {on | off}
LANG_SET_SECURE_SYNTAX_EXT
	Syntax: %Fset secure {on | off}
	Enables or disables %CS's security features for a channel. When SECURE is set, only users who have
	registered their nicknames with services and IDENTIFY'd with their password will be given access
	to the channel as controlled by the access list.
LANG_SET_SECUREOPS_SYNTAX
	Syntax: %Fset secureops {on | off}
LANG_SET_SECUREOPS_SYNTAX_EXT
	Syntax: %Fset secureops {on | off}
	Enables or disables the secure ops option for a channel.
	When secure ops is set, users who are not on the userlist will not be allowed chanop status.
LANG_SET_SECUREFOUNDER_SYNTAX
	Syntax: %Fset securefounder {on | off}
LANG_SET_SECUREFOUNDER_SYNTAX_EXT
	Syntax: %Fset securefounder {on | off}
	Enables or disables the secure founder option for a channel. When secure founder is set,
	only the real founder will be able to drop the channel, change its password, its founder and its
	successor, and not those who are IDENTIFY'd with services.
LANG_SET_SIGNKICK_SYNTAX
	Syntax: %Fset signkick {on | off}
LANG_SET_SIGNKICK_SYNTAX_EXT
	Syntax: %Fset signkick {on | off}
	Enables or disables signed kicks for a channel.  When SIGNKICK is set, kicks issued with
	services' KICK command will have the nick that used the command in their reason.
	If you use LEVEL, those who have a level that is superior or equal to the SIGNKICK levelon the
	channel won't have their  kicks signed. See '%Fhelp levels' for more information.
LANG_SET_XOP_SYNTAX
	Syntax: %Fset xop {on | off}
LANG_SET_XOP_SYNTAX_EXT
	Syntax: %Fset xop {on | off}
	Enables or disables the xOP lists system for a channel. When XOP is set, you have to use the AOP/SOP/VOP
	commands in order to give channel privileges to users, else you have to use the ACCESS command.,
LANG_SET_NOEXPIRE_SYNTAX
	Syntax: %Fset noexpire {on | off}
LANG_SET_NOEXPIRE_SYNTAX_EXT
	Syntax: %Fset noexpire {on | off}
	Sets whether the given channel will expire. Setting this to ON prevents the channel from expiring.
	
	This command is limited to Services Administrators.

## Botserv options
LANG_SET_DONTKICKOPS_SYNTAX
	Syntax: %Fset dontkickops {on | off}
LANG_SET_DONTKICKOPS_SYNTAX_EXT
	Syntax: %Fset dontkickops {on | off}
	Enables or disables OP protection on the channel. When it is enabled,
	ops won't be kicked by the service bot even if they don't match the NOKICK level.
LANG_SET_DONTKICKVOICES_SYNTAX
	Syntax: %Fset dontkickvoices {on | off}
LANG_SET_DONTKICKVOICES_SYNTAX_EXT
	Syntax: %Fset dontkickvoices {on | off}
	Enables or disables VOICE protection on the channel. When it is enabled,
	voices won't be kicked by the service bot even if they don't match the NOKICK level.
LANG_SET_GREET_SYNTAX
	Syntax: %Fset greet {on | off}
LANG_SET_GREET_SYNTAX_EXT
	Syntax: %Fset greet {on | off}
	Enables or disables greet mode on a channel. When it is enabled, the bot will display greet
	messages of users joining the channel, provided they have enough access to the channel.
LANG_SET_FANTASY_SYNTAX
	Syntax: %Fset fantasy {on | off}
LANG_SET_FANTASY_SYNTAX_EXT
	Syntax: %Fset fantasy {on | off}
	Enables or disables fantasy mode on a channel. When it is enabled, users will be able to use
	commands !op, !cmdlist and !help. Note that users wanting to use fantaisist ommands MUST have
	enough level for both the FANTASIA and another level depending of the command if required
	
	IMPORTANT: Disabling this will disable all fantasy commands on this channel so re-enabling
	will have to be done through %BS.
LANG_SET_SYMBIOSIS_SYNTAX
	Syntax: %Fset symbiosis {on | off}
LANG_SET_SYMBIOSIS_SYNTAX_EXT
	Syntax: %Fset symbiosis {on | off}
	Enables or disables symbiosis mode on a channel. When it is enabled, the bot will do everything
	normally done by %CS on channels, such as MODEs, KICKs, and even the entry message.
	Note: When disabled, most fantasy commands will still be handled by the bot.
LANG_SET_NOBOT_SYNTAX
	Syntax: %Fset nobot {on | off}
LANG_SET_NOBOT_SYNTAX_EXT
	Syntax: %Fset nobot {on | off}
	This option makes a channel be unassignable. If a bot is already assigned to the channel,
	it is unassigned automatically when you enable the option.

# Shun
LANG_SHUN_SYNTAX
	Syntax: %Fshun [add | del] [+expiry] {nick | user@host} [reason]
LANG_SHUN_SYNTAX_EXT
	Syntax: %Fshun [add] [+expiry] {nick | user@host} [reason]
	        %Fshun del {nick | user@host}
	
	This command adds or deletes a shun on the given nick or username@usermask
	If 'ADD' or 'DEL' is not specified, ADD is assumed.
	The time given is considered to be in days, unless another interval is specified.
	Possible alternatives: secondes (s), minutes, (m), hours, (h) and days (d).
	Note that these may not be combined! Only the first part will be read.
	If no reason is specified, a default reason will be used.
	If the mask starts with a '+', an expiry must be given!
	
	Note that SHUN DEL is only guaranteed to work for shuns added by the SHUN command
	and only if the user is still online. In any other case, use name@mask.
	When shunning the target 'add' or 'del', 'ADD' must be given.

# Staff
LANG_STAFF
	Syntax: {%Fstaff | %Fircops}
	This will return  list of available global IRCops.

# Suspend
LANG_SUSPEND_SYNTAX
	Syntax: %Fsuspend [reason]
LANG_SUSPEND_SYNTAX_EXT
	Syntax: %Fsuspend [reason]
	Disallows anyone from using the channel but preserves the channel
	data and settings untill the channel in unsuspended.

# Sync
LANG_SYNC_SYNTAX
	Syntax: %Fsync
LANG_SYNC_SYNTAX_EXT
	Syntax: %Fsync
	This command will give all users currently in the channel the level
	they are granted by the channels' access list. Users who have a level
	greater then the one they are supposed to have will be demoted.
	
	The use ofthis command is restricted to the Channel Founder.

# TB
LANG_TBAN_SYNTAX
	Syntax: {%Ftb | %Ftban} [nick | [nick!]ident@host] [time]
LANG_TBAN_SYNTAX_EXT
	Syntax: {%Ftb | %Ftban} [nick | [nick!]ident@host] [time]
	
	This will ban the given nick or hostmask from the channel for the specified
	amount of time. If the time is not given, it will default to 1 hour.
	The time should be given in the format <digit><(s)/m/h/d>.
	Example: %Ftb Viper 10m which will ban me for 10 minutes.
	
	Note: if services are restarted before the ban expires, the ban will
	not be automatically removed

# TKB
LANG_TKBAN_SYNTAX
	Syntax: %Ftkb [nick | [nick!]ident@host] [time] [reason]
LANG_TKBAN_SYNTAX_EXT
	Syntax: %Ftkb or %Ftkban [nick | [nick!]ident@host] [time] [reason]
	
	This will kick and temporary ban the specified user or all users matching
	the given mask from the channel for the specified amount of time. 
	If a mask is given and someone who should not be banned acording to the
	channels' settings matches it, the command is aborted before anyone is kicked.
	Also be aware that this command may not work with IPs for privacy reasons."
	If the time is not given or is invalid, it will default to 1 hour.
	The time should be given in the format <digit><(s)/m/h/d>.
	Example: %Ftkb Viper 10m Bye which will kickban me for 10 minutes.
	
	Note: if services are restarted before the ban expires, the ban will
	not be automatically removed.

# Topic
LANG_TOPIC_SYNTAX
	Syntax: %Ftopic [topic]
	Syntax: %Fctopic
LANG_TOPIC_SYNTAX_EXT
	Syntax: %Ftopic [topic]
	Syntax: %Fctopic
	
	Sets the channel topic. If no topic is given, the current topic is displayed in the channel.
	The CTOPIC command is provided to clear the topic.
	This command is most useful in conjunction with SET TOPICLOCK

# TSHUN
LANG_TSHUN_SYNTAX
	Syntax: %Ftshun [add | del] nick [reason]
LANG_TSHUN_SYNTAX_EXT
	Syntax: %Ftshun [add] [nick] [reason]
	        %Ftshun del [nick]
	
	This command adds or deletes a temporary shun on the given user.
	If add or del is not specified, add is assumed.
	This command cannot be used on IRC operators.
	Note that this shun will be automatically removed when the user disconnects
	and that it will only affect one session.

# UP
LANG_UPDOWN_SYNTAX
	Syntax: {%Fup | %Fdown} [nick]
LANG_UP_SYNTAX
	Syntax: %Fup [nick]
LANG_UP_SYNTAX_EXT
	Syntax: %Fup [nick]
	Gives the user the modes he has rights to. This will unset modes if he has any
	the user has no access to.
	
	Note: A nick can only be specified by the founder of the channel or an SRA.


#
# XOP & ACCESS, LEVELS
#
LANG_SOP_SYNTAX
	Syntax: %Fsop {add | del | list | clear} [nick | entry-list]
LANG_SOP_SYNTAX_EXT
	Syntax: %Fsop add nick
	        %Fsop del {nick | entry-num | list}
	        %Fsop list [mask | list]
	        %Fsop clear
	
	Maintains the SOP (SuperOP) list for the channel.
	The SOP list gives users all rights given by the AOP list, and adds
	those needed to use the AutoKick and the BadWords lists,
	to send and read channel memos, and so on.
	
	This command may have been disabled for your channel, and
	in that case you need to use the access list. See HELP ACCESS
	for information about the access list, and HELP SET XOP to know how
	to toggle between the access list and xOP list systems.

LANG_AOP_SYNTAX
	Syntax: %Faop {add | del | list | clear} [nick | entry-list]
LANG_AOP_SYNTAX_EXT
	Syntax: %Faop add nick
	        %Faop del {nick | entry-num | list}
	        %Faop list [mask | list]
	        %Faop clear
	
	Maintains the AOP (AutoOP) list for the channel.
	The AOP list gives users the right to be auto-opped on your channel,
	to unban or invite themselves if needed, to have their
	greet message showed on join, and so on.
	
	This command may have been disabled for your channel, and
	in that case you need to use the access list. See HELP ACCESS
	for information about the access list, and HELP SET XOP to know how
	to toggle between the access list and xOP list systems.

LANG_HOP_SYNTAX
	Syntax: %Fhop {add | del | list | clear} [nick | entry-list]
LANG_HOP_SYNTAX_EXT
	Syntax: %Fhop add nick
	        %Fhop del {nick | entry-num | list}
	        %Fhop list [mask | list]
	        %Fhop clear
	
	Maintains the HOP (HalfOP) list for the channel. The HOP
	list gives users the right to be auto-halfopped on your channel
	
	This command may have been disabled for your channel, and
	in that case you need to use the access list. See HELP ACCESS
	for information about the access list, and HELP SET XOP to know how
	to toggle between the access list and xOP list systems.

LANG_VOP_SYNTAX
	Syntax: %Fvop {add | del | list | clear} [nick | entry-list]
LANG_VOP_SYNTAX_EXT
	Syntax: %Fvop add nick
	        %Fvop del {nick | entry-num | list}
	        %Fvop list [mask | list]
	        %Fvop clear
	
	Maintains the VOP (VOicePeople) list for a channel.
	The VOP list allows users to be auto-voiced and to voice
	themselves if they aren't.
	
	This command may have been disabled for your channel, and
	in that case you need to use the access list. See HELP ACCESS
	for information about the access list, and HELP SET XOP to know how
	to toggle between the access list and xOP list systems.

LANG_ACCESS_SYNTAX
	Syntax: %Faccess {add | del | list | clear} [nick [level] | mask | list]
LANG_ACCESS_SYNTAX_EXT
	Syntax: %Faccess add nick level
	        %Faccess del {nick | entry-num | list}
	        %Faccess list [mask | list]
	        %Faccess clear
	
	Maintains the access list for a channel.  The access
	list specifies which users are allowed chanop status or
	access to commands on the channel.
	Different user levels allow for access to different subsets of
	privileges; cf. %R%CS HELP ACCESS LEVELS for more specific information.
	Any nick not on the access list has a user level of 0.

LANG_LEVELS_SYNTAX
	Syntax: %Flevels {set |dis[able] | list | reset} [item [level]]
LANG_LEVELS_SYNTAX_EXT
	Syntax: %Flevels set type level
	        %Flevels {dis | disable} type
	        %Flevels list
	        %Flevels reset
	
	The LEVELS command allows fine control over the meaning of
	the numeric access levels used for the channel.  With this command,
	you can define the access level required for most administrative functions.
	(The SET FOUNDER and SET PASSWORD commands, as well as this command, 
	are always restricted to the channel founder.) 
	
	LEVELS SET allows the access level for a function or group of functions to be changed.
	LEVELS DISABLE (or DIS for short) disables an automatic
	feature or disallows access to a function by anyone other than the channel founder.
	LEVELS LIST shows the current levels for each function or group of functions.
	LEVELS RESET resets the levels to the default levels of a newly-created channel.
	(See %R%CS HELP ACCESS LEVELS for more info.)
	
	For a list of the features and functions whose levels can be
	set, see HELP LEVELS DESC.

