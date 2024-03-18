##############################################################################
##### French language file.                                              #####
##############################################################################
# Module Name    : bs_fantasy_ext
# Author         : Viper <Viper@Anope.org>
# Creation Date  : 14/09/2008
# Last Updated   : 23/12/2011
#
# More info on http://modules.anope.org and http://forum.anope.org
#
##############################################################################
#
# French language file provided by Seb Inconnu.
#
# ! This file is currently outdated and missing some translations.
# ! I am looking for someone to update the translations and check them against en_us.
# ! Contact me @ viper@anope.org.
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
	Français

#
#
# Generic Messages
#
#

#
# Error Messages
#

LANG_ONLY_FOUNDERS
	Accès refusé : Seuls les Owners peuvent utiliser cette commande.

LANG_EXCEPTS_UNSUPPORTED
	Les Chans Excepts ne sont pas supportés sur ce réseau.
LANG_INVITEMODE_UNSUPPORTED
	Les Chans Invites ne sont pas supportés sur ce réseau.
LANG_HOPS_UNSUPPORTED
	Le Grade Halfops n'est pas supporté par ce réseau.

LANG_CMD_NOT_AV
	Cette commande n'est pas disponible. Contacter le staff du réseau pour plus d'informations.

LANG_CMD_DISABLED
	Cette commande a été désactivée, Merci de contacter Le staff du réseau pour plus d'informations.

LANG_UNKNOWN_CMD
	Commandes inconnues: %s

LANG_INVALID_TIME
	%s et un format de temp invalide.

LANG_REQ_NICK_OR_MASK
	Cette commande requierre un pseudo ou un mask user@host valide.


# Levels
LANG_LEVELS_ERR_NO_VAL
	Erreur: Pas de valeur donnée au paramètre SET.
LANG_LEVELS_ERR_VAL
	Erreur: Valeur donnée non-numérique.

# Badwords
LANG_BADWORDS_ERR_STYLE
	Erreur: Style invalide. Utiliser uniquement START, SINGLE, ANY ou END.

# Invite
LANG_INVITE_NO_USER
	Cet utilisateur n'est pas connecté.
LANG_INVITE_IS_ON
	Cet utilisateur est déjà sur le Chan !
LANG_INVITE_YOURSELF
	Vous ne pouvez pas vous inviter vous même !


#
# Operlist
#
LANG_GOLIST_OPER_ONLY
	IRCop
LANG_GOLIST_OPER_AND_SO
	IRCop + Services
LANG_GOLIST_OPER_AND_SA
	IRCop + Services Administrateur
LANG_GOLIST_OPER_AND_SRA
	IRCOperateur + Services Root Administrateur
LANG_GOLIST_NONE
	Pas d'IRCops disponiblent pour l'instant
LANG_GOLIST_HEADER
	-----------------------------------------------------------------------
	Les IRCop suivant sont en ligne:
	-----------------------------------------------------------------------
LANG_GOLIST_FOOTER
	-----------------------------------------------------------------------
	Total d'IRCops disponiblent : %d 
	-----------------------------------------------------------------------
	-> Toujours essayer de contacter un IRCop sur un chan avant de le PV
	-> Eviter de PV un IRCop sans avoir sa permission avant.
	-> faire un /whois sur IRCop avant de vouloir lui parler.
	-> Lire (et respecter) un message d'away présent dans le Whois.
	-----------------------------------------------------------------------

#
# Adminlist
#
LANG_ADLIST_SA
	Services Administrateur
LANG_ADLIST_SRA
	Services Root Administrateur
LANG_ADLIST_NONE
	Aucun administrateur disponible actuellement
LANG_ADLIST_HEADER
	-----------------------------------------------------------------------
	Les Administrateurs suivant sont en ligne:
	-----------------------------------------------------------------------
LANG_ADLIST_FOOTER
	-----------------------------------------------------------------------
	Total d'Admins disponibles : %d
	-----------------------------------------------------------------------
	-> Toujours essayer de contacter un Admin sur un chan avant de le PV
	-> Eviter de PV un Admin sans avoir sa permission avant.
	-> faire un /whois sur Admin avant de vouloir lui parler.
	-> Lire (et respecter) un message d'away present dans le Whois.
	-----------------------------------------------------------------------

# TB
LANG_TBAN_RESPONSE
	Le ban temporaire %s de %s, %s.

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
	in order to safeguard the users' privacy bans matching
	the users IP or host may not have been removed.



#
#
# Help messages
#
#

LANG_HELP
	SYNTAX: %Fhelp
	Voici l'aide pour les commandes fantasy botserv.
LANG_CMDLIST
	Syntax: %Fcmdlist
	Montre la liste des commandes Fantasy

LANG_HELP_HEADER
	----- Aide pour les commandes Fantasy -----
LANG_HELP_FOOTER
	----- Fin de la liste des commandes -----


LANG_CMD_LIST_HEADER
	Commandes Fantasy disponiblent :
LANG_CMD_LIST_FOOTER
	Toutes les fonctions de services ne sont pas détaillées ici...
LANG_CMD_LIST_GENERAL
	Commandes général: %s
LANG_CMD_LIST_CHAN_AXX
	Commandes de l'accès au Chan: %s
LANG_CMD_LIST_CHG_SET
	Commandes de réglage de Chan: %s
LANG_CMD_LIST_ACTIONS
	Commandes d'actions sur le Chan: %s
LANG_CMD_LIST_IRCOP
	Commandes IRC Operateur: %s
LANG_CMD_LIST_EXTMODS
	Other modules: %s

LANG_HELP_UNKNOWN_CMD
	Pas d'aide disponible pour cette commande.

LANG_HELP_CMDS_GEN
	Commandes Générales :
	  %Fhelp      -   Aide de botserv pour les commandes
	  %Fcmdlist   -   Montre la liste de toutes les commandes FANTASY
	  %Fircops    -   Montre la liste de tous les IRCops disponibles
	  %Fstaff     -   Montre la liste de tous les IRCops disponibles
	  %Fminfo     -   Montre les informations à propos du module
	  %Fsversion  -   Montre les informations sur Anope en général
	  %Finfo      -   Montre les informations à propos du Chan
	
	Note: Ces commandes peuvent être indisponibles, désactivées ou à accès restreint
LANG_HELP_CMDS_ACC
	Commandes de Contrôle d'Accès:
	  %Fsop       -   Permet de gérer la liste des SOP (SuperOP)
	  %Faop       -   Permet de gérer la liste des AOP (AutoOP)
	  %Fhop       -   Permet de gérer la liste des HOP (HalfOP)(peut ne pas être géré par le réseau)
	  %Fvop       -   Permet de gérer la liste des VOP (VOicePeople)
	  %Faccess    -   Permet de gérer la liste des AXX (quand XOP est désactivé)
	  %Flevels    -   Permet de gérer les levels du Chan
	
	Note: Ces commandes peuvent être indisponibles, désactivées ou à accès restreint
LANG_HELP_CMDS_SETTINGS
	Commands de Règlage du chan:
	  %Fakick     -   Gère la liste de akick
	  %Fbadwords  -   Gère la liste des mots interdits
	  %Fset       -   Change les réglages du Chan. (cf. '%Fhelp set')
	  %Fbkick     -   Change le réglage des kicks Botserv.(cf. '%Fhelp bkick')
	
	Note: Ces commandes peuvent être indisponibles, désactivées ou à accès restreint
LANG_HELP_CMDS_ACTIONS
	Commandes d'Action du Chan:
	  %Fclear     -   Efface l'option spécifiée sur le chan.
	  %Ftopic     -   Règle le topic du chan
	  %Ftappend   -   Ajoute le texte au topic
	  %Finvite    -   Invite un utilisateur sur le chan
	  %Fup        -   Attribue les grades auxquels l'utilisateur a accès
	  %Fdown      -   Enlève tous les grades d'un utilisateur
	  %Fkick      -   Kick un utilisateur sur le Chan
	  %Fkb        -   Kick et ban un ou des utilisateur(s) sur le Chan
	  %Ftkb       -   Kick et temporaire ban un ou des utilisateur(s) sur le Chan
	
	Note: Ces commandes peuvent être indisponibles, désactivées ou à accès restreint
LANG_HELP_CMDS_PUTMODES
	Commands de Mode du Chan:
	  %Fban       -   Bannit l'utilisateur sur le Chan sans le kicker.
	  %Ftban      -   Bannit l'utilisateur temporaire sur le Chan sans le kicker.
	  %Funban     -   Débannit l'utilisateur sur le Chan
	  %Fmute      -   Bannit l'utilisateur ou le rend muet (dépend de L'IRCD)
	  %Funmute    -   Débannit l'utilisateur ou lève le mode muet (dépend de L'IRCD)
	  %Fop        -   Op vous-même ou un autre utilisateur
	  %Fdeop      -   Deop vous-même ou un autre utilisateur
	  %Fvoice     -   Voice vous-même ou un autre utilisateur
	  %Fdevoice   -   Devoice vous-même ou un autre utilisateur
	  %Fprotect   -   Protect vous-même ou un autre utilisateur (Peut ne pas être disponible)
	  %Fdeprotect -   Deprotect vous-même ou un autre utilisateur ((Peut ne pas être disponible)
	  %Fowner     -   Attribue le grade owner (Peut ne pas être disponible)
	  %Fdeowner   -   Enlève votre grade d'owner (Peut ne pas être disponible)
	  %Fhalfop    -   Halfop vous-même ou un autre utilisateur (Peut ne pas être disponible)
	  %Fdehalfop  -   Dehalfop vous-même ou un autre utilisateur (Peut ne pas être disponible)
	
	Note: Ces commandes peuvent être indisponibles, désactivées ou à accès restreint
LANG_HELP_CMDS_IRCOP
	Commandes IRC Operateur :
	  %Fakill     -   Force un ban sur le mask pour tout le serveur.
	  %Fkill      -   Deconnecte l'utilisateur
	  %Fmode      -   Règle les modes de Chan
	  %Fignore    -   Règle les services pour ignorer un utilisateur
	
	Note: Ces commandes peuvent être indisponibles, désactivées ou à accès restreint

LANG_HELP_MORE_INFO
	Pour avoir de l'aide sur une commande %Fhelp <commande>
LANG_HELP_LIST
	Pour avoir une liste des commandes possibles pour ce que vous voulez faire, tapez:
	
	 %Fhelp general  - Montre les commandes générales disponibles. Aucun accès requis.
	 %Fhelp chaccess - Montre les commandes pour modifier la liste d'accès ou des levels d'un chan.
	 %Fhelp config   - Configure les réglages d'un chan.
	 %Fhelp act      - Liste des commandes d'action possibles par les services.
	 %Fhelp ircop    - Commandes vablables seulement pour les IRC Operateurs.
	
	Pour avoir la liste de toutes les commandes disponibles, tape '%Fhelp all'

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
	SYNTAX: %Fadmins
	This will return list of all available Services and Network Administrators..

# Akick
LANG_AKICK_SYNTAX
	SYNTAX: %Fakick {add | stick | unstick | del | list | view | enforce | clear} [pseudo-or-usermask] [raison]
LANG_AKICK_SYNTAX_EXT
	SYNTAX: %Fakick add [mask | user] [raison]
	       %Fakick {stick | unstick | del} mask
	       %Fakick {list | view} [mask]
	       %Fakick {enforce | clear}

# Akill
LANG_AKILL_SYNTAX
	SYNTAX: %Fakill {add | del | list | view | clear} [+expiry] [mask | entry-list] [raison]
LANG_AKILL_SYNTAX_EXT
	SYNTAX: %Fakill add [+expiry] mask raison
	        %Fakill del {mask | num de list | list}
	        %Fakill {list | view} [mask | list]
	        %Fakill clear
	IRC Opérateurs seulement. Cette commande peut être désactivée.

# Append
LANG_APPENDT_SYNTAX
	SYNTAX: {%Fappendtopic | %Ftappend} texte à ajouter au topic
LANG_APPENDT_SYNTAX_EXT
	SYNTAX: {%Fappendtopic | %Ftappend} texte à ajouter au topic
	Ajoute le texte donné au topic actuel du Chan.
	Cette commande s'utilise avec SET TOPICLOCK

# Badwords
LANG_BADWORDS_SYNTAX
	SYNTAX: %Fbadwords {add | del | list | clear} [mot | entry-list] [single | start | end | any]
LANG_BADWORDS_SYNTAX_EXT
	SYNTAX: %Fbadwords add mot [single | start | end | any]
	        %Fbadwords del {mot | num de list | list}
	        %Fbadwords list [mask | list]
	        %Fbadwords clear

# Ban
LANG_BAN_SYNTAX
	SYNTAX: %Fban [pseudo | mask]
LANG_BAN_SYNTAX_EXT
	SYNTAX: %Fban [pseudo | mask]
	Banni le pseudo ou le mask donné du Chan.

#
# BKick
#
LANG_BKICK_SYNTAX
	SYNTAX: %Fbkick option {on | off} [ttb [params]]
	Options disponibles: bolds, badwords, caps, colors, flood, repeat, reverses, underlines.
LANG_BKICK_SYNTAX_EXT
	SYNTAX: %Fbkick option {on | off} [ttb [params]]
	Available options:
	        bolds       -  Règle le bot pour kicker les Gras.
	        badwords    -  Règle le bot pour kicker les Mots Interdits.
	        caps        -  Règle le bot pour kicker les Majuscules.
	        colors      -  Règle le bot pour kicker les Couleurs.
	        flood       -  Règle le bot pour kicker le flood.
	        repeat      -  Règle le bot pour kicker les Répétitions.
	        reverses    -  Règle le bot pour kicker les inversés.
	        underlines  -  Règle le bot pour kicker les Soulignés.
	
	ttb est le nombre de fois qu'un utilisateur peut être kické
	avant d'être banni. Laissez ttb vierge pour désactiver
LANG_BKICK_BOLDS_SYNTAX
	SYNTAX: %Fbkick bolds {on | off} [ttb]
LANG_BKICK_BOLDS_SYNTAX_EXT
	SYNTAX: %Fbkick bolds {on | off} [ttb]
	Quand l'option est selectionnée, le bot pourra kicker les personnes
	qui écrivent en gras. les services enveront une notice quand la commande OP DEOP sera utilisée.
	ttb est le nombre de fois qu'un utilisateur peut être kicker avant d'être banni.
	Laissez ttb vierge pour désactiver le système de ban automatique.
LANG_BKICK_BADWORDS_SYNTAX
	SYNTAX: %Fbkick badwords {on | off} [ttb]
LANG_BKICK_BADWORDS_SYNTAX_EXT
	SYNTAX: %Fbkick badwords {on | off} [ttb]
	Quand l'option est selectionnée, le bot pourra kicker les personnes qui utilisent
	des mots interdits. Vous pouvez definir la liste de mots interdits en tapant %Fbadwords command.
	ttb est le nombre de fois qu'un utilisateur peut être kické avant d'être banni.
	Laissez ttb vierge pour désactiver le système de ban automatique.
LANG_BKICK_CAPS_SYNTAX
	SYNTAX: %Fbkick caps {on | off} [ttb [min [percent]]]
LANG_BKICK_CAPS_SYNTAX_EXT
	SYNTAX: %Fbkick caps {on | off} [ttb [min [percent]]]
	Quand l'option est selectionnée, le bot pourra kicker les personnes qui
	écrivent en majuscule. Le bot kicke si il y a minimum min majuscule caps
	et qu'elles constituent au moins percent% de la ligne de texte entière
	"(si ces paramètres ne sont pas donnés, c'est par défaut 10 caractères et 25%).
	ttb est le nombre de fois qu'un utilisateur peut être kické avant d'être banni.
	Laissez ttb vierge pour désactiver le système de ban automatique.
LANG_BKICK_COLORS_SYNTAX
	SYNTAX: %Fbkick colors {on | off} [ttb]
LANG_BKICK_COLORS_SYNTAX_EXT
	SYNTAX: %Fbkick colors {on | off} [ttb]
	Quand l'option est selectionnée, le bot pourra kicker les personnes qui ecrivent en couleur.
	ttb est le nombre de fois qu'un utilisateur peut être kické avant d'être banni.
	Laissez ttb vierge pour désactiver le système de ban automatique.
LANG_BKICK_FLOOD_SYNTAX
	SYNTAX: %Fbkick flood {on | off} [ttb [ln [secs]]]
LANG_BKICK_FLOOD_SYNTAX_EXT
	SYNTAX: %Fbkick flood {on | off} [ttb [ln [secs]]]
	Quand l'option est selectionnée, le bot pourra kicker les personnes
	qui flood sur le chan en ecrivant ln lignes en secs secondes.
	(si vous ne paramétrez pas, c'est 6 lignes en 10 secondes par défaut).
	ttb est le nombre de fois qu'un utilisateur peut être kické avant d'être banni.
	Laissez ttb vierge pour désactiver le système de ban automatique.
LANG_BKICK_REPEAT_SYNTAX
	SYNTAX: %Fbkick repeat {on | off} [ttb [num]]
LANG_BKICK_REPEAT_SYNTAX_EXT
	SYNTAX: %Fbkick repeat {on | off} [ttb [num]]
	Quand l'option est selectionnée, le bot pourra kicker les personnes
	qui se répètent num fois (si num n'est pas designé, c'est 3 par défaut)."
	ttb est le nombre de fois qu'un utilisateur peut être kické avant d'être banni.
	Laissez ttb vierge pour désactiver le système de ban.
LANG_BKICK_REVERSES_SYNTAX
	SYNTAX: %Fbkick reverses {on | off} [ttb]
LANG_BKICK_REVERSES_SYNTAX_EXT
	SYNTAX: %Fbkick reverses {on | off} [ttb]
	Quand l'option est selectionnée, le bot pourra kicker les personnes qui ecrivent en inversé.
	ttb est le nombre de fois qu'un utilisateur peut être kické avant d'être banni.
	Laissez ttb vierge pour désactiver le système de ban.
LANG_BKICK_UNDERLINES_SYNTAX
	SYNTAX: %Fbkick underlines {on | off} [ttb]
LANG_BKICK_UNDERLINES_SYNTAX_EXT
	SYNTAX: %Fbkick underlines {on | off} [ttb]
	Quand l'option est selectionnée, le bot pourra kicker les personnes qui ecrivent en souligné.
	ttb est le nombre de fois qu'un utilisateur peut être kické avant d'être banni.
	Laissez ttb vierge pour désactiver le système de ban.
LANG_BKICK_UNKNOWN_OPTION
	Option BKICK inconnue: %s.


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
	SYNTAX: %Fdown [pseudo] [level]
LANG_DOWN_SYNTAX_EXT
	SYNTAX: %Fdown [pseudo] [level]
	Enleve tout les modes au pseudo. Si le Chan est modéré, cela enlevera les voices,
	jusqu'a ce qu'un utilisateur soit de nouveaux designé par l'owner.
	
	It is also possible to specify a target level.
	The target level can be any one of the following:
	 - q - Channel founder (modes +qo) (IRCd Dependant)
	 - a - Protected Operator / Administrator (modes +ao) (IRCd Dependant)
	 - o - Channel Operator (mode +o)
	 - h - Channel Halfop (mode +h) (IRCd Dependant)
	 - v - \"Voice\" (mode +v)
	If a target level is specified, the users' current privileges will be replaced by the target one.
	
	Note: Un pseudo peut seulement etre specifié par le owner du Chan ou un SRA.

# Ignore
LANG_IGNORE_SYNTAX
	SYNTAX: %Fignore {add | del | list | clear} [time] [pseudo | mask]
LANG_IGNORE_SYNTAX_EXT
	SYNTAX: %Fignore {add | del} [expiry] [pseudo | mask]
	        %Fignore {list | clear}
	Autorise les Services Admins à rendre un pseudo ignoré par les services pour
	un certain temps ou jusqu au redémarrage. Le temps par défaut
	à un format en secondes. Les combinaisons d'unité de temps ne sont pas permises.
	Pour que les services ignorent indéfiniment, utiliser 0 pour le temps.

# Info
LANG_INFO_SYNTAX
	SYNTAX: %Finfo [all]
LANG_INFO_SYNTAX_EXT
	SYNTAX: %Finfo [all]
	Donne la liste des informations pour le Chan.
	L'option 'all' est limitée au propriétaire et aux Services Admins.

# Invite
LANG_INVITE_SYNTAX
	SYNTAX: %Finvite [pseudo]
LANG_INVITE_SYNTAX_EXT
	SYNTAX: %Finvite [pseudo]
	IInvite 'Pseudo' sur le Chan si celui ci est présent sur le réseau

# KB
LANG_KICKBAN_SYNTAX
	SYNTAX: {%Fkickban | %Fkb} [pseudo | [pseudo!]ident@host] [raison]
LANG_KICKBAN_SYNTAX_EXT
	SYNTAX: {%Fkickban | %Fkb} [pseudo!]ident@host] [raison]
	Ceci kickera et bannira la personne désignée ou toutes les personnes ayant
	le même mask sur le Chan. Si un mask est donné et que quelqu'un ne devrait
	pas être kické' à cause des réglages du chan, la commande est anulée.
	Personne n'est kické ou banni.

# KICK
LANG_KICK_SYNTAX
	SYNTAX: {%Fkick | %Fk} [nick | nick[!ident@host]] [reason]
LANG_KICK_SYNTAX_EXT
	SYNTAX: {%Fkick | %Fk} nick[!ident@host]] [reason]
	Ceci kickera la personne désignée ou toutes les personnes ayant le même mask sur le Chan.

# Kill
LANG_KILL_SYNTAX
	SYNTAX: %Fkill pseudo raison
LANG_KILL_SYNTAX_EXT
	SYNTAX: %Fkill pseudo raison
	Cette commande force l'utilisateur à se deconnecter du réseau (identique à /kill).
	IRC Opérateurs seulement. Cette commande peut être désactivée

# Module Info
LANG_MINFO
	SYNTAX: {%Fminfo | %Fmodinfo}
	Montre les informations du module des commandes Fantasy.

# Mute / Unmute
LANG_MUTE_SYNTAX
	SYNTAX: %Fmute [pseudo | mask] (dépend de l'IRCD)
LANG_MUTE_SYNTAX_EXT
	SYNTAX: %Fmute [pseudo | mask]
	Banni ou rend muet le pseudo ou le mask donné pour le Chan. (dépend de L'IRCD)

LANG_UNMUTE_SYNTAX
	SYNTAX: %Funmute [pseudo | mask] (dépend de l'IRCD)
LANG_UNMUTE_SYNTAX_EXT
	SYNTAX: %Funmute [pseudo | mask]
	Debanni ou enlève Le mode muet sur le pseudo ou le mask donné pour le Chan. (dépend de l'IRCD)

# Mode
LANG_MODE_SYNTAX
	SYNTAX: %Fmode mode(s)
LANG_MODE_SYNTAX_EXT
	SYNTAX: %Fmode mode(s)
	Cette commande modifie les modes de Chan.
	IRC Opérateurs seulement. Cette commande peut être désactivée

# Rappend
LANG_RAPPEND_SYNTAX
	Syntax: {%Frappend | %Ftrapp} text to replace last append to topic
LANG_RAPPEND_SYNTAX_EXT
	Syntax: {%Frappend | %Ftrapp} text to replace last append to topic
	Replaces the last append to the topic with the given text.
	If no replacement text is given, the last append is removed.
	This command is most useful in conjunction with SET TOPICLOCK

#
# SET
#
LANG_SET_SYNTAX
	SYNTAX: %Fset option parametres
	Available options: founder, successor, desc, email, url, entrymsg, bantype, mlock,
	        keeptopic, topiclock, opnotice, peace, private, restricted, secure, secureops,
	        securefounder, signkick, xop, noexpire (limited to SA)
	        dontkickops, dontkickvoices, greet, fantasy, symbiosis, nobot (limited to SA)
LANG_SET_SYNTAX_EXT
	SYNTAX: %Fset option parameters
	Available options:
	        founder       -  Désigne le nouvel Owner du Chan
	        successor     -  Désigne le successeur de l'Owner pour un Chan
	        desc          -  Règle la description du Chan
	        url           -  Associe une URL au Chan
	        email         -  Associe un E-mail au Chan
	        entrymsg      -  Règle le message envoyé aux utilisateurs quand ils entrent dans un Chan
	        bantype       -  Regle le type de ban que les services appliquent
	        mlock         -  Verrouille les modes de chan choisis
	        keeptopic     -  Maintient le Topic quand le Chan n'est pas utilisé
	        topiclock     -  Le Topic peut seulement être modifié avec TOPIC
	        opnotice      -  Envoie une notice quand la commande OP/DEOP est utilisée
	        peace         -  Régule l'usage des commandes de sanction
	        private       -  Cache le chan dans la commande LIST
	        restricted    -  Restreint l'accès du Chan
	        secure        -  Active les options de sécurité des services
	        secureops     -  Contrôle strict du statut d'op
	        securefounder -  Contrôle strict du statut d'owner
	        signkick      -  Signature des kicks qui ne sont pas faits par la commande KICK
	        xop           -  Met le système d'accès en mode XOP
	        noexpire      -  Règle l'expiration du Chan. (limité au Service Admin)
	
	        dontkickops   -  Protège les ops des kicks par les services.
	        dontkickvoices - Protège les voices des kicks par les services.
	        greet         -  Autorise les messages greet quand on rejoint un Chan.
	        fantasy       -  Autorise les commandes fantasy sur le Chan. (Lire l'aide avant !)
	        symbiosis     -  Autorise le bot à agir comme un vrai bot.
	        nobot         -  Empêche l'assignation d'un bot sur le Chan. (limité au Service Admin)

# Set subcommands
## Chanserv options
LANG_SET_FOUNDER_SYNTAX
	SYNTAX: %Fset founder pseudo
LANG_SET_FOUNDER_SYNTAX_EXT
	SYNTAX: %Fset founder pseudo
	Change le propriétaire du Chan. Le nouveau propriétaire doit être un pseudo enregistré.
LANG_SET_SUCCESSOR_SYNTAX
	SYNTAX: %Fset successor pseudo
LANG_SET_SUCCESSOR_SYNTAX_EXT
	SYNTAX: %Fset successor pseudo
	Change le successeur du propriétaire. Si le pseudo du propriétaire expire ou est droppé
	Tant que le Chan est enregistré le successeur peut devenir le propriétaire du Chan.
	Le pseudo du Successeur doit être enregistré.
LANG_SET_DESC_SYNTAX
	SYNTAX: %Fset desc description
LANG_SET_DESC_SYNTAX_EXT
	SYNTAX: %Fset desc description
	Règle la description du Chan, elle se voit sur la commande LIST et INFO.
LANG_SET_URL_SYNTAX
	SYNTAX: %Fset url [url]
LANG_SET_URL_SYNTAX_EXT
	SYNTAX: %Fset url [url]
	Associe L'URL donnée au Chan.
	Cette URL sera visible sur la commande INFO du Chan.
	Si aucune URL n'est precisée, cela effacera toute URL associée au Chan.
LANG_SET_EMAIL_SYNTAX
	SYNTAX: %Fset email [email]
LANG_SET_EMAIL_SYNTAX_EXT
	SYNTAX: %Fset email [email]
	Associe L'E-mail donné au Chan.
	Cet E-mail sera visible sur la commande INFO du Chan.
	Si aucun E-mail n'est precisé, cela effacera tout E-mail associé au Chan.
LANG_SET_ENTRYMSG_SYNTAX
	SYNTAX: %Fset entrymsg [Message d'entré],
LANG_SET_ENTRYMSG_SYNTAX_EXT
	SYNTAX: %Fset entrymsg [Message d'entré]
	Règle le message qui sera envoyé par /notice aux utilisateurs qui rejoindront le chan.
	Si aucun message n'est precisé, il n'y aura pas de notice envoyée.
LANG_SET_BANTYPE_SYNTAX
	SYNTAX: %Fset bantype {0 | 1 | 2 | 3}
LANG_SET_BANTYPE_SYNTAX_EXT
	SYNTAX: %Fset bantype {0 | 1 | 2 | 3}
	Règle le type de ban utilisé par les services quand ils auront besoin de bannir un utilisateur du Chan.
	
	Le type de ban sera un nombre entre 0 et 3 qui signifie :
	0: ban de cette forme *!user@host
	1: ban de cette forme *!*user@host
	2: ban de cette forme *!*@host
	3: ban de cette forme *!*user@*.domain
LANG_SET_MLOCK_SYNTAX
	SYNTAX: %Fset mlock [modes]
LANG_SET_MLOCK_SYNTAX_EXT
	SYNTAX: %Fset mlock [modes]
	Règle le mode-lock pour le Chan.
	Ceci vous permet de définir les modes de Chan qui seront maintenus.
LANG_SET_KEEPTOPIC_SYNTAX
	SYNTAX: %Fset keeptopic {on | off}
LANG_SET_KEEPTOPIC_SYNTAX_EXT
	SYNTAX: %Fset keeptopic {on | off}
	Met ou Enlève le maintien du Topic d'un Chan. Quand l'option KEEPTOPIC est activée,
	le topic du Chan sera gardé après que le dernier utilisateur quitte le Chan,
	et sera restauré à la prochaine création du Chan.
LANG_SET_TOPICLOCK_SYNTAX
	SYNTAX: %Fset topiclock {on | off}
LANG_SET_TOPICLOCK_SYNTAX_EXT
	SYNTAX: %Fset topiclock {on | off}
	Met ou enlève le verrouillage du topic pour un Chan. Quand l'option TOPICLOCK est activée,
	les services ne permettront pas la modification du Topic sur le Chan.
LANG_SET_OPNOTICE_SYNTAX
	SYNTAX: %Fset opnotice {on | off}
LANG_SET_OPNOTICE_SYNTAX_EXT
	SYNTAX: %Fset opnotice {on | off}
	Met ou enlève la notice d'Op pour un Chan. Quand l'option OPNOTICE est activée,
	les services enverront une notice au Chan pour prévenir de l utilisation de la commande OP/DEOP.
LANG_SET_PEACE_SYNTAX
	SYNTAX: %Fset peace {on | off}
LANG_SET_PEACE_SYNTAX_EXT
	SYNTAX: %Fset peace {on | off}
	Met ou enlève l'option peace pour le Chan. Quand l'option PEACE est activée,
	un utilisateur ne pourra pas kicker, bannir ou enlever le statut d'un utilisateur qui aura
	un niveau supérieur ou égal au sien.
LANG_SET_PRIVATE_SYNTAX
	SYNTAX: %Fset private {on | off}
LANG_SET_PRIVATE_SYNTAX_EXT
	SYNTAX: %Fset private {on | off}
	Met ou enlève l'option private pour le Chan. Quand l'option PRIVATE est activée,
	la commande %R%CS LIST ne montrera le Chan dans aucune liste.
LANG_SET_RESTRICTED_SYNTAX
	SYNTAX: %Fset restricted {on | off}
LANG_SET_RESTRICTED_SYNTAX_EXT
	SYNTAX: %Fset restricted {on | off}
	Met ou enlève l'option de restriction pour un Chan.
	Quand l'option RESTRICTED est activée, les utilisateurs qui n'ont pas d'accès Opérateur
	(utilisateurs avec accès négatif et, si secure ops est actif, utilisateurs sans accès)
	seront immediatement kickés et bannis du Chan.
LANG_SET_SECURE_SYNTAX
	SYNTAX: %Fset secure {on | off}
LANG_SET_SECURE_SYNTAX_EXT
	SYNTAX: %Fset secure {on | off}
	Met ou enlève l'option de sécurité %CS pour un Chan. Quand l'option SECURE est activée,
	Seuls les utilisateurs qui ont un pseudo enregistré et identifié aux services, 
	auront un accès au Chan sous contrôle de la liste d'accès.
LANG_SET_SECUREOPS_SYNTAX
	SYNTAX: %Fset secureops {on | off}
LANG_SET_SECUREOPS_SYNTAX_EXT
	SYNTAX: %Fset secureops {on | off}
	Met ou enlève l'option secure ops option pour un Chan.
	Quand l'option SECUREOPS est activée, les utilisateurs qui ne sont pas dans la liste d'accès,
	ne pourront pas avoir le statut d'Op.
LANG_SET_SECUREFOUNDER_SYNTAX
	SYNTAX: %Fset securefounder {on | off}
LANG_SET_SECUREFOUNDER_SYNTAX_EXT
	SYNTAX: %Fset securefounder {on | off}
	Met ou enlève l'option secure founder pour un Chan. Quand l'option SECURE FOUNDER est activée,
	seul le propriétaire et son successeur pourront dropper le Chan, changer son password,
	pas les propriétaires identifiés auprés des services.
LANG_SET_SIGNKICK_SYNTAX
	SYNTAX: %Fset signkick {on | level | off}
LANG_SET_SIGNKICK_SYNTAX_EXT
	SYNTAX: %Fset signkick {on | level | off}
	Met ou enlève la signature des kicks pour un Chan. Quand l'option SIGNKICK est activée, les kicks mis
	par les services afficheront le pseudo et la raison de celui qui utilise la commande de kick.
	Si vous utilisez les LEVELS, ceux qui auront un level supérieur ou égal au level SIGNKICK
	n'auront pas de kick signé sur le Chan. Voir '%Fhelp levels' pour plus d'information.
LANG_SET_XOP_SYNTAX
	SYNTAX: %Fset xop {on | off}
LANG_SET_XOP_SYNTAX_EXT
	SYNTAX: %Fset xop {on | off}
	Met ou enlève le système xOP pour un Chan. Quand l'option XOP est activée, vous devez utiliser les commandes
	AOP/SOP/VOP pour donner les accès aux utilisateurs, autrement utiliser la commande ACCESS.
LANG_SET_NOEXPIRE_SYNTAX
	SYNTAX: %Fset noexpire {on | off}
LANG_SET_NOEXPIRE_SYNTAX_EXT
	SYNTAX: %Fset noexpire {on | off}
	Règle le paramètre d'expiration du Chan. Quand l'option NOEXPIRE est activée le Chan n'expirera pas.
	
	Cette commande est réservée aux Administrateurs de Services.

## Botserv options
LANG_SET_DONTKICKOPS_SYNTAX
	SYNTAX: %Fset dontkickops {on | off}
LANG_SET_DONTKICKOPS_SYNTAX_EXT
	SYNTAX: %Fset dontkickops {on | off}
	Active ou désactive la protection des ops sur un chan.
	Quand l'option DONTKICKOPS est activée, cette option empêche le bot de kicker les ops
	même s'ils ne sont pas concernés par le level NOKICK.
LANG_SET_DONTKICKVOICES_SYNTAX
	SYNTAX: %Fset dontkickvoices {on | off}
LANG_SET_DONTKICKVOICES_SYNTAX_EXT
	SYNTAX: %Fset dontkickvoices {on | off}
	Active ou désactive la protection des voices sur un chan.
	Quand l'option DONTKICKVOICES est activée, cette option empêche le bot de kicker les voices
	même s'ils ne sont pas concernés par le level NOKICK.
LANG_SET_GREET_SYNTAX
	SYNTAX: %Fset greet {on | off}
LANG_SET_GREET_SYNTAX_EXT
	SYNTAX: %Fset greet {on | off}
	Active ou désactive le  Message d'Accueil sur un chan. Quand l'option GREET est activée,
	elle fait afficher au bot le message d'accueil de chaque utilisateur qui a un accès suffisant au chan.
LANG_SET_FANTASY_SYNTAX
	SYNTAX: %Fset fantasy {on | off}
LANG_SET_FANTASY_SYNTAX_EXT
	SYNTAX: %Fset fantasy {on | off}
	Active ou désactive le mode FANTASY sur un chan. Quand l'option FANTASY est activée, cette option
	permet aux utilisateurs d'utiliser les commandes %Fop, %Fcmdlist et %Fhelp.
	Noter que les utilisateurs qui veulent utliliser les commandes FANTASY doivent avoir 
	suffisamment d'accès pour les levels FANTASIA et le level qui dépend de la commande
	
	IMPORTANT: Désactiver cette option désactivera les commandes FANTASY donc
	reactiver devra être fait par %BS.
LANG_SET_SYMBIOSIS_SYNTAX
	SYNTAX: %Fset symbiosis {on | off}
LANG_SET_SYMBIOSIS_SYNTAX_EXT
	SYNTAX: %Fset symbiosis {on | off}
	Active ou désactive la symbiose sur un chan. Quand l'option SYMBIOSIS est activée,
	cette option fait faire au bot tout ce qui est normalement effectué par %CS sur les chans,
	tels que les MODEs,les KICKs, et même le message d'entrée. 
	Note: Quand l'option est désactivée, la plupart des commandes fantasy seront possibles avec le bot.
LANG_SET_NOBOT_SYNTAX
	SYNTAX: %Fset nobot {on | off}
LANG_SET_NOBOT_SYNTAX_EXT
	SYNTAX: %Fset nobot {on | off}
	Cette option fait qu'un bot ne peut pas être assigné sur le Chan. Si un bot est
	quand même assigné, il sera désassigné automatiquement si cette option est activée.

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
	SYNTAX: {%Fstaff | %Fircops}
	Cette commande donne la liste des IRC Operateurs disponibles.

# Suspend
LANG_SUSPEND_SYNTAX
	Syntax: %Fsuspend [reason]
LANG_SUSPEND_SYNTAX_EXT
	Syntax: %Fsuspend [reason]
	Disallows anyone from using the channel but preserves the channel
	data and settings untill the channel in unsuspended.

# Sversion
LANG_SVERSION
	SYNTAX: %Fsversion
	Montre les informations sur la version et le build des Services Anope.

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
	SYNTAX: {%Ftb | %Ftban} [pseudo | [pseudo!]ident@host] [temps]
LANG_TBAN_SYNTAX_EXT
	SYNTAX: {%Ftb | %Ftban} [pseudo | [pseudo!]ident@host] [temps]
	
	Ceci bannira le pseudo ou le mask specifié dans le Chan pour
	la durée précisée. Si le temps n'est pas donné, il sera d'une heure par defaut.
	Le temps devra etre donné dans le format suivant <nombre><(s)/m/h/d>.
	Exemple: %Ftb Viper 10m ceci me bannira pour 10 minutes.
	
	Note: Si les services sont redemarrés avant l'expiration du ban,
	il ne sera pas automatiquement enlevé

# TKB
LANG_TKBAN_SYNTAX
	SYNTAX: %Ftkb [pseudo | [pseudo!]ident@host] [temps] [raison]
LANG_TKBAN_SYNTAX_EXT
	SYNTAX: %Ftkb or %Ftkban [pseudo | [pseudo!]ident@host] [temps] [raison]
	
	Ceci Kickera et bannira l'utilisateur spécifié ou tout ceux qui correspondront
	au mask donné dans le Chan pour le temps donné. 
	Si un mask est donné et qu'un utilisateur qui correspond aux
	reglages de Chan ne doit pas etre banni, la commande sera annulé avant que le premier kick sois fait.
	Si le temps n'est pas precisé ou donné dans un mauvais format, il sera d'une heure par defaut.
	Le temp devra etre donné dans le format suivant <nombre><(s)/m/h/d>.
	Exemple: %Ftkb Viper 10m Bye ceci me kickera et bannira pour 10 minutes.
	
	Note: Si les services sont redemarrés avant l'expiration du ban
	il ne sera pas automatiquement enlevé

# Topic
LANG_TOPIC_SYNTAX
	Syntax: %Ftopic [topic]
	Syntax: %Fctopic
LANG_TOPIC_SYNTAX_EXT
	Syntax: %Ftopic [topic]
	Syntax: %Fctopic
	
	Sets the channel topic. If no topic is given, the current topic is displayed in the channel.
	The CTOPIC command is provided to clear the topic.
	Cette commande s'utilise avec SET TOPICLOCK

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
	SYNTAX: {%Fup | %Fdown} [pseudo]
LANG_UP_SYNTAX
	SYNTAX: %Fup [pseudo]
LANG_UP_SYNTAX_EXT
	SYNTAX: %Fup [pseudo]
	Donne à l'utilisateur les modes auquel il a droit. Ceci ne donnera pas de grade si
	l'utilisateur n'a pas d'accès.
	
	Note: Un pseudo peut seulement etre specifié par le owner du Chan ou un SRA.


#
# XOP & ACCESS, LEVELS
#
LANG_SOP_SYNTAX
	Syntax: %Fsop {add | del | list | clear} [pseudo | entry-list]
LANG_SOP_SYNTAX_EXT
	Syntax: %Fsop add pseudo
	        %Fsop del {pseudo | num de list | list}
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
	Syntax: %Faop {add | del | list | clear} [pseudo | entry-list]
LANG_AOP_SYNTAX_EXT
	Syntax: %Faop add pseudo
	        %Faop del {pseudo | num de list | list}
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
	Syntax: %Fhop {add | del | list | clear} [pseudo | entry-list]
LANG_HOP_SYNTAX_EXT
	Syntax: %Fhop add pseudo
	        %Fhop del {pseudo | num de list | list}
	        %Fhop list [mask | list]
	        %Fhop clear
	
	Maintains the HOP (HalfOP) list for the channel. The HOP
	list gives users the right to be auto-halfopped on your channel
	
	This command may have been disabled for your channel, and
	in that case you need to use the access list. See HELP ACCESS
	for information about the access list, and HELP SET XOP to know how
	to toggle between the access list and xOP list systems.

LANG_VOP_SYNTAX
	Syntax: %Fvop {add | del | list | clear} [pseudo | entry-list]
LANG_VOP_SYNTAX_EXT
	Syntax: %Fvop add pseudo
	        %Fvop del {pseudo | num de list | list}
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
	Syntax: %Faccess {add | del | list | clear} [pseudo [level] | mask | list]
LANG_ACCESS_SYNTAX_EXT
	Syntax: %Faccess add pseudo level
	        %Faccess del {pseudo | num de list | list}
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

