##############################################################################
##### French language file.                                              #####
##############################################################################
# Module Name    : bs_fantasy_ext
# Author         : Viper <Viper@Anope.org>
# Creation Date  : 14/09/2008
# Last Updated   : 12/09/2012
#
# More info on http://modules.anope.org and http://forum.anope.org
#
##############################################################################
#
# French language file. By S.Boily (PePsY). Last modified: August 1st 2012
# 100% re-written with proper french, a couple of translations still missing.
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
	Accès refusé: Seuls les fondateurs de canal peuvent utiliser cette commande.

LANG_EXCEPTS_UNSUPPORTED
	Les exceptions de canal ne sont pas supportées sur ce réseau.
LANG_INVITEMODE_UNSUPPORTED
	Les invitations de canal ne sont pas supportées sur ce réseau.
LANG_HOPS_UNSUPPORTED
	Les demi-ops (HalfOps) ne sont pas supportés sur ce réseau.

LANG_CMD_NOT_AV
	Cette commande n'est présentement pas disponible. Svp contactez l'équipe du réseau
	pour plus d'information.

LANG_CMD_DISABLED
	Cette commande a été désactivée. Svp contactez l'équipe du réseau pour plus
	d'information.

LANG_UNKNOWN_CMD
	Commande inconnue: %s

LANG_INVALID_TIME
	%s est un format de temps non valide.

LANG_REQ_NICK_OR_MASK
	Cette commande requiert soit un surnom ou un masque (user@host) valide.


# Human readable expiration not covered by the core
EXPIRES_S
	expire dans %d secondes
EXPIRES_1S
	expire en %d seconde


# Levels
LANG_LEVELS_ERR_NO_VAL
	Erreur: aucune valeur fournie avec le paramètre SET.
LANG_LEVELS_ERR_VAL
	Erreur: la valeur fournie n'est pas numérique.

# Badwords
LANG_BADWORDS_ERR_STYLE
	Erreur: le style fourni n'est pas valide. Svp spécifiez START, SINGLE, ANY ou END seulement.

# Invite
LANG_INVITE_NO_USER
	Cet usager n'est pas en ligne.
LANG_INVITE_IS_ON
	Cet usager est déjà sur le canal!
LANG_INVITE_YOURSELF
	Vous ne pouvez pas vous inviter vous-mêmes!


#
# Operlist
#
LANG_GOLIST_OPER_ONLY
	IRCop
LANG_GOLIST_OPER_AND_SO
	IRCop + Op des services
LANG_GOLIST_OPER_AND_SA
	IRCop + Administrateur des services
LANG_GOLIST_OPER_AND_SRA
	IRCop + Administrateur racine des services
LANG_GOLIST_NONE
	Aucun IRCop n'est présentement disponible.
LANG_GOLIST_HEADER
	-----------------------------------------------------------------------
	Les opérateurs IRC globaux suivants sont en ligne:
	-----------------------------------------------------------------------
LANG_GOLIST_FOOTER
	-----------------------------------------------------------------------
	Total d'IRCops disponibles: %d
	-----------------------------------------------------------------------
	 -> Toujours tenter de contacter un IRCop dans le canal avant de lui
	    parler en privé.
	 -> Essayez d'éviter de parler à un IRCop en privé sans tout d'abord
	    avoir sa permission.
	 -> Svp faites /whois sur un IRCop avant de tenter de communiquer avec
	    lui.
	 -> Lire (et respecter) tout message d'une ligne AWAY.
	-----------------------------------------------------------------------

#
# Adminlist
#
LANG_ADLIST_SA
	Administrateur des services
LANG_ADLIST_SRA
	Administrateur racine des services
LANG_ADLIST_NONE
	Aucun administrateur n'est présentement disponible.
LANG_ADLIST_HEADER
	-----------------------------------------------------------------------
	Les administrateurs suivants sont en ligne:
	-----------------------------------------------------------------------
LANG_ADLIST_FOOTER
	-----------------------------------------------------------------------
	Total des administrateurs disponibles: %d
	-----------------------------------------------------------------------
	 -> Toujours tenter de contacter un administrateur dans le canal avant
	    de lui parler en privé.
	 -> Essayez d'éviter de parler à un administrateur en privé sans tout
	    d'abord avoir sa permission.
	 -> Svp faites /whois sur un administrateur avant de tenter de
	    communiquer avec lui.
	 -> Lire (et respecter) tout message d'une ligne AWAY.
	-----------------------------------------------------------------------

# TB
LANG_TBAN_RESPONSE
	Banni %s temporairement de %s, %s.

#TSHUN
LANG_TSHUN_ADDED
	SHUN appliqué temporairement sur l'usager %s.
LANG_TSHUN_DELETION
	SHUN temporaire enlevé sur l'usager %s.
LANG_TSHUN_TARGET_NEXIST
	L'usager %s n'existe pas actuellement.
LANG_TSHUN_TARGET_OPER
	La commande ne peut pas être utilisée sur des opérateurs IRC.

# SHUN
LANG_SHUN_ADDED
	SHUN ajouté pour %s@%s.
LANG_SHUN_DELETION
	Le SHUN sur %s@%s a été effacé, s'il existait.

# Suspend
LANG_SUSPEND_NO_REASON
	Une raison est obligatoire pour suspendre (SUSPEND) un canal sur ce réseau.

# Sync
LANG_SYNC_DONE
	La liste d'usagers (userlist) a été synchronisée avec la liste des accès (accesslist).

# Unban
LANG_UNBAN_NOTE
	Tentative pour débannir %s.
	Cependant, ceci ne garantit pas un débannissement complet, car dans l'optique de
	conserver privés les adresses IP ou alpha numériques des usagers, les
	bannissements qui correspondent à ces adresses peuvent ne pas avoir été enlevés.



#
#
# Help messages
#
#

LANG_HELP
	Syntaxe: %Fhelp
	Ceci est l'aide pour les commandes fantaisistes de %BS.
LANG_CMDLIST
	Syntaxe: %Fcmdlist
	Montre les commandes fantaisistes disponibles.


LANG_CMD_LIST_HEADER
	Commandes fantaisistes disponibles:
LANG_CMD_LIST_FOOTER
	Notez que toutes les fonctions ou services possibles ne sont pas couverts ici...
LANG_CMD_LIST_GENERAL
	Commandes générales: %s
LANG_CMD_LIST_CHAN_AXX
	Commandes d'accès de canal: %s
LANG_CMD_LIST_CHG_SET
	Configurations de canal: %s
LANG_CMD_LIST_ACTIONS
	Actions de canal: %s
LANG_CMD_LIST_IRCOP
	Commandes pour opérateurs IRC: %s
LANG_CMD_LIST_EXTMODS
	Autres modules: %s

LANG_HELP_UNKNOWN_CMD
	Aucune aide n'est disponible pour cette commande.

LANG_HELP_CMDS_GEN
	Commandes générales:
	   %Fhelp      -   Aide pour les commandes fantaisistes %BS
	   %Fcmdlist   -   Liste de toutes les commandes fantaisistes disponibles
	   %Ftopic     -   Show the channel's topic
	   %Fircops    -   Montre une liste de tous les opérateurs IRC globaux disponibles
	   %Fstaff     -   Montre une liste de tous les opérateurs IRC globaux disponibles
	   %Fminfo     -   Montre de l'information sur ce module
	   %Fsversion  -   Montre l'information d'Anope en général
	   %Finfo      -   Montre l'information sur le canal
	
	Note: Ces commandes peuvent être non disponibles, désactivées ou leur accès peut être
	restreint.

LANG_HELP_CMDS_ACC
	Commandes de contrôle d'accès:
	   %Fsop       -   Donne accès à la liste SOP (SuperOp)
	   %Faop       -   Donne accès à la liste AOP (AutoOp)
	   %Fhop       -   Donne accès à la liste HOP (HalfOp) (Peut ne pas être supporté
	                    sur ce réseau)
	   %Fvop       -   Donne accès à la liste VOP (usagers avec VOICE)
	   %Faccess    -   Donne accès à la liste d'accès numérique (quand XOP est à OFF)
	   %Flevels    -   Donne accès à la liste des niveaux qui contrôlent les droits de
	                    chaque niveau d'accès
	
	Note: Ces commandes peuvent être non disponibles, désactivées ou leur accès peut être
	restreint.

LANG_HELP_CMDS_SETTINGS
	Commandes de configuration de canal:
	   %Fakick     -   Gère la liste des bannissements (AKICK)
	   %Fbadwords  -   Gère la liste des mots interdits
	   %Fset       -   Permets de changer la configuration du canal (ex.: '%Fhelp set')
	   %Fbkick     -   Permets de changer la configuration des KICKs de %BS.
	                    (ex.: '%Fhelp bkick')
	
	Note: Ces commandes peuvent être non disponibles, désactivées ou leur accès peut être
	restreint.

LANG_HELP_CMDS_ACTIONS
	Commandes d'actions de canal:
	   %Fclear     -   Enlever les options spécifiées d'un canal.
	   %Ftopic     -   Mets un sujet à un canal (TOPIC)
	   %Fctopic    -   Clear the channel's topic
	   %Ftappend   -   Ajoute du texte au sujet de canal existant (TOPIC)
	   %Frappend   -   Replaces the text last appended to the topic
	   %Ftprepend  -   Prepends text to the topic
	   %Frprepend  -   Replaces the text last prepended to the topic
	   %Finvite    -   Invite un usager dans un canal
	   %Fup        -   Donne à un usager les modes auxquels il a droit
	   %Fdown      -   Enleve tous les modes d'un usager
	   %Fkick      -   Kick a user or all users matching a mask from the channel
	   %Fkb        -   Expulse et banni un ou plusieurs usagers d'un canal (KICK+BAN)
	   %Ftkb       -   Expulse et banni temporairement un ou plusieurs usagers d'un canal
	                    (KICK+BAN)
	
	Note: Ces commandes peuvent être non disponibles, désactivées ou leur accès peut être
	restreint.

LANG_HELP_CMDS_PUTMODES
	Commandes de modes de canal:
	   %Fban       -   Banni un masque d'adresse d'un canal sans utiliser l'expulsion
	                    (BAN sans KICK)
	   %Ftban      -   Banni temporairement un masque d'adresse d'un canal sans utiliser
	                    l'expulsion (BAN temporaire sans KICK)
	   %Funban     -   Removes the ban(s) on a user/mask.
	   %Fmute      -   Réduit au silence un usager en le bannissant (dépend du IRCd)
	   %Funmute    -   Permets de soulever l'interdiction de parler d'un usager en le
	                    débannissant (dépend du IRCd)
	   %Fop        -   Donner à un usager ou à vous-mêmes le statut d'opérateur de canal
	   %Fdeop      -   Enlever à un usager ou à vous-mêmes le statut d'opérateur de canal
	   %Fvoice     -   Donner à un usager ou à vous-mêmes le mode VOICE
	   %Fdevoice   -   Enlever à un usager ou à vous-mêmes le mode VOICE
	   %Fprotect   -   Donner le statut de protégé à un usager ou à vous-mêmes
	                    (peut ne pas être supporté sur ce réseau)
	   %Fdeprotect -   Enlever le statut de protégé à un usager ou à vous-mêmes
	                    (peut ne pas être supporté sur ce réseau)
	   %Fowner     -   Donner à un usager ou à vous-mêmes le statut de fondateur de canal
	                    (peut ne pas être supporté sur ce réseau)
	   %Fdeowner   -   Enlever le statut de fondateur de canal à un usager ou à vous-mêmes
	                    sur un canal (peut ne pas être supporté sur ce réseau)
	   %Fhalfop    -   Donner à un usager ou à vous-mêmes le statut demi-OP (HalfOp)
	                    (peut ne pas être supporté sur ce réseau)
	   %Fdehalfop  -   Enlever le statut de demi-OP (HalfOp) à un usager ou à vous-mêmes
	                    (peut ne pas être supporté sur ce réseau)
	
	Note: Ces commandes peuvent être non disponibles, désactivées ou leur accès peut être
	restreint.

LANG_HELP_CMDS_IRCOP
	Commandes d'opérateur IRC (IRCop):
	   %Fakill     -   Forcera un bannissement sur un masque d'adresse sur le réseau entier
	   %Fkill      -   Expulse un usager du réseau (KILL)
	   %Fmode      -   Change les modes d'un canal
	   %Fignore    -   Force les services à ignorer un usager
	
	Note: Ces commandes peuvent être non disponibles, désactivées ou leur accès peut être
	restreint.

LANG_HELP_MORE_INFO
	Pour plus d'information sur une commande, faites %Fhelp <commande>
LANG_HELP_LIST
	Pour obtenir une liste des commandes disponibles reliées à ce que vous voulez faire,
	utilisez la syntaxe suivante:
	
	  %Fhelp general  - Montre les commandes générales disponibles. Aucun accès spécial
	                    n'est requis.
	  %Fhelp chaccess - Montre les commandes pour manipuler la liste d'accès et la liste
	                    des niveaux d'un canal.
	  %Fhelp config   - Configure les options et les listes d'un canal.
	  %Fhelp act      - Commandes qui produiront certaines actions par les services.
	  %Fhelp ircop    - Commandes seulement disponibles aux opérateurs IRC (IRCop)
	
	Pour obtenir une liste de toutes les commandes disponibles, faites '%Fhelp all'

# Access levels
LANG_ACCESS_LEVELS
	Niveaux d'accès d'usager:
	
	Par défaut, les niveaux d'accès suivants sont définis:
	 *** Fondateur ***
	 Accès total aux fonctions administratives et statut d'opérateur donné en entrant sur
	 le canal. À noter qu'une seule personne peut avoir le statut de fondateur (le statut
	 ne peut pas être donné par la commande ACCESS {ceci n'est pas vrai si cs_fop est
	 chargé!}).
	 ***      10  ***
	 Accès à la commande AKICK et statut d'opérateur donné en entrant sur le canal.
	 ***       5  ***
	 Statut d'opérateur de canal donné automatiquement en entrant.
	 ***       4  ***
	 Statut de demi-op (HalfOp, +h) donné automatiquement en entrant (si supporté par
	 le IRCd.)
	 ***       3  ***
	 Statut de VOICE (+v) donné automatiquement en entrant sur le canal.
	 ***       0  ***
	 Aucun privilège spécial; peut recevoir le statut d'opérateur par un autre opérateur
	 (sauf si secure-ops est actif).
	 ***      <0  ***
	 Ne peut pas recevoir le statut d'opérateur de canal.
	
	Ces niveaux peuvent être changés, ou de nouveau ajoutés, en utilisant la commande
	LEVELS. Pour plus d'information, voir HELP LEVELS.

# Admins
LANG_ADMINS_SYNTAX
	Syntaxe: %Fadmins
	Ceci affichera la liste de tous les administrateurs de services et du réseau qui sont
	disponibles.

# Akick
LANG_AKICK_SYNTAX
	Syntaxe: %Fakick {add | stick | unstick | del | list | view | enforce | clear} [surnom-ou-masque] [raison]
LANG_AKICK_SYNTAX_EXT
	Syntaxe: %Fakick add {masque | usager} [raison]
	         %Fakick {stick | unstick | del} masque
	         %Fakick {list | view} [masque]
	         %Fakick {enforce | clear}

# Akill
LANG_AKILL_SYNTAX
	Syntaxe: %Fakill {add | del | list | view | clear} [+expiration] [masque | entrée#] [raison]
LANG_AKILL_SYNTAX_EXT
	Syntaxe: %Fakill add [+expiration] masque raison
	         %Fakill del {masque | entrée# | liste}
	         %Fakill {list | view} [masque | liste]
	         %Fakill clear
	Pour opérateurs IRC seulement (IRCop). Cette commande peut être désactivée.

# Append
LANG_APPENDT_SYNTAX
	Syntaxe: {%Fappendtopic | %Ftappend} texte à ajouter au TOPIC existant
LANG_APPENDT_SYNTAX_EXT
	Syntaxe: {%Fappendtopic | %Ftappend} texte à ajouter au TOPIC existant
	Ajoute le texte donné à la suite du sujet de canal (TOPIC) existant.
	Cette commande est plus utile en conjonction avec SET TOPICLOCK

# Badwords
LANG_BADWORDS_SYNTAX
	Syntaxe: %Fbadwords {add | del | list | clear} [mot | entrée#] [single | start | end | any]
LANG_BADWORDS_SYNTAX_EXT
	Syntaxe: %Fbadwords add mot [single | start | end | any]
	         %Fbadwords del {mot | entrée# | liste}
	         %Fbadwords list [masque | liste]
	         %Fbadwords clear

# Ban
LANG_BAN_SYNTAX
	Syntaxe: %Fban [surnom | masque]
LANG_BAN_SYNTAX_EXT
	Syntaxe: %Fban [surnom | masque]
	Banni le surnom ou le masque donné d'un canal.

#
# BKick
#
LANG_BKICK_SYNTAX
	Syntaxe: %Fbkick option {on | off} [ttb [paramètre]]
	Options disponibles: bolds, badwords, caps, colors, flood, repeat, reverses, underlines.
LANG_BKICK_SYNTAX_EXT
	Syntaxe: %Fbkick option {on | off} [ttb [paramètre]]
	Options disponibles:
	        bolds       -  Activer ou non l'expulsion (KICK) pour le texte en gras.
	        badwords    -  Activer ou non l'expulsion (KICK) pour les mots interdits dans
	                        la liste de mots interdits du canal.
	        caps        -  Activer ou non l'expulsion (KICK) pour le texte en majuscules.
	        colors      -  Activer ou non l'expulsion (KICK) pour les couleurs.
	        flood       -  Activer ou non l'expulsion (KICK) pour l'inondation de texte.
	        repeat      -  Activer ou non l'expulsion (KICK) pour les lignes répétées.
	        reverses    -  Activer ou non l'expulsion (KICK) pour le texte à couleurs
	                        inversées.
	        underlines  -  Activer ou non l'expulsion (KICK) pour le texte souligné.
	
	ttb est le nombre de fois qu'un usager peut être expulsé (KICK) avant d'être banni.
	Omettez ttb pour désactiver le système de bannissement une fois activé.
LANG_BKICK_BOLDS_SYNTAX
	Syntaxe: %Fbkick bolds {on | off} [ttb]
LANG_BKICK_BOLDS_SYNTAX_EXT
	Syntaxe: %Fbkick bolds {on | off} [ttb]
	Quand activée, cette option indique au robot d'expulser les usagers qui utilisent le
	texte en gras. Les services vont envoyer une notice au canal à chaque fois que les
	commandes OP et DEOP sont utilisées.
	ttb est le nombre de fois qu'un usager peut être expulsé (KICK) avant d'être banni.
	Omettez ttb pour désactiver le système de bannissement une fois activé.
LANG_BKICK_BADWORDS_SYNTAX
	Syntaxe: %Fbkick badwords {on | off} [ttb]
LANG_BKICK_BADWORDS_SYNTAX_EXT
	Syntaxe: %Fbkick badwords {on | off} [ttb]
	Quand activée, cette option indique au robot d'expulser les usagers qui utilisent
	certains mots dans le canal. Vous pouvez définir ces mots pour votre canal en
	utilisant la commande %Fbadwords.
	ttb est le nombre de fois qu'un usager peut être expulsé (KICK) avant d'être banni.
	Omettez ttb pour désactiver le système de bannissement une fois activé.
LANG_BKICK_CAPS_SYNTAX
	Syntaxe: %Fbkick caps {on | off} [ttb [min [pourcentage]]]
LANG_BKICK_CAPS_SYNTAX_EXT
	Syntaxe: %Fbkick caps {on | off} [ttb [min [pourcentage]]]
	Quand activée, cette option indique au robot d'expulser les usagers qui envoient du
	texte en majuscules. Le robot expulse seulement s'il y a au moins min lettres
	en majuscules et qu'elles constituent au moins pourcentage% du total de lignes de
	texte (si non fourni, la valeur par défaut est de 10 caractères et 25%).
	ttb est le nombre de fois qu'un usager peut être expulsé (KICK) avant d'être banni.
	Omettez ttb pour désactiver le système de bannissement une fois activé.
LANG_BKICK_COLORS_SYNTAX
	Syntaxe: %Fbkick colors {on | off} [ttb]
LANG_BKICK_COLORS_SYNTAX_EXT
	Syntaxe: %Fbkick colors {on | off} [ttb]
	Quand activée, cette option indique au robot d'expulser les usagers qui utilisent du
	texte en couleurs.
	ttb est le nombre de fois qu'un usager peut être expulsé (KICK) avant d'être banni.
	Omettez ttb pour désactiver le système de bannissement une fois activé.
LANG_BKICK_FLOOD_SYNTAX
	Syntaxe: %Fbkick flood {on | off} [ttb [nb_de_lignes [secs]]]
LANG_BKICK_FLOOD_SYNTAX_EXT
	Syntaxe: %Fbkick flood {on | off} [ttb [nombre_de_lignes [secs]]]
	Quand activée, cette option indique au robot d'expulser les usagers qui inondent le
	canal en utilisant au moins nombre_de_lignes en secs secondes. (Si non fournie,
	la configuration par défaut est de 6 lignes en 10 secondes).
	(if not given, it defaults to 6 lines in 10 seconds).
	ttb est le nombre de fois qu'un usager peut être expulsé (KICK) avant d'être banni.
	Omettez ttb pour désactiver le système de bannissement une fois activé.
LANG_BKICK_REPEAT_SYNTAX
	Syntaxe: %Fbkick repeat {on | off} [ttb [nombre]]
LANG_BKICK_REPEAT_SYNTAX_EXT
	Syntaxe: %Fbkick repeat {on | off} [ttb [nombre]]
	Quand activée, cette option indique au robot d'expulser les usagers qui envoi le même
	texte à répétition nombre fois (si non fourni, la valeur par défaut est de 3).
	ttb est le nombre de fois qu'un usager peut être expulsé (KICK) avant d'être banni.
	Omettez ttb pour désactiver le système de bannissement une fois activé.
LANG_BKICK_REVERSES_SYNTAX
	Syntaxe: %Fbkick reverses {on | off} [ttb]
LANG_BKICK_REVERSES_SYNTAX_EXT
	Syntaxe: %Fbkick reverses {on | off} [ttb]
	Quand activée, cette option indique au robot d'expulser les usagers qui utilisent du
	texte en couleurs inversées.
	ttb est le nombre de fois qu'un usager peut être expulsé (KICK) avant d'être banni.
	Omettez ttb pour désactiver le système de bannissement une fois activé.
LANG_BKICK_UNDERLINES_SYNTAX
	Syntaxe: %Fbkick underlines {on | off} [ttb]
LANG_BKICK_UNDERLINES_SYNTAX_EXT
	Syntaxe: %Fbkick underlines {on | off} [ttb]
	Quand activée, cette option indique au robot d'expulser les usagers qui utilisent
	du texte souligné.
	ttb est le nombre de fois qu'un usager peut être expulsé (KICK) avant d'être banni.
	Omettez ttb pour désactiver le système de bannissement une fois activé.
LANG_BKICK_UNKNOWN_OPTION
	BKICK option inconnue: %s.


# Clear
LANG_CLEAR_SYNTAX
	Syntaxe: %Fclear {modes | bans | excepts | invites | ops | hops | voices | users}
LANG_CLEAR_SYNTAX_EXT
	Syntaxe: %Fclear [paramètre]
	
	Les paramètres suivants sont acceptés:
	MODES   - Réinitialise tous les modes du canal (c.à.d.: i,k,l,m,n,p,s,t).
	BANS    - Enlève tous les bannissements (BAN) du canal.
	EXCEPTS - Enlève toutes les exceptions de bannissement du canal.
	INVITES - Enlève toutes les invitations du canal.
	OPS     - Enlève le statut d'opérateur de canal à tous ceux qui le possèdent.
	HOPS    - Enlève le statut de demi-opérateur (HalfOp) à tous ceux qui le possèdent.
	VOICES  - Enlève le statut de VOICE (+v) à tous ceux qui le possèdent.
	USERS   - Expulse (KICK) tous les usagers du canal.
	
	Note: tous les paramètres peuvent ne pas être supportés, dépend du IRCd.

# CTopic
LANG_CTOPIC_SYNTAX
	Syntax: %Fctopic
LANG_CTOPIC_SYNTAX_EXT
	Syntax: %Fctopic
	
	Clears the channels topic.
	This command is most useful in conjunction with SET TOPICLOCK

# Down
LANG_DOWN_SYNTAX
	Syntaxe: %Fdown [surnom] [niveau]
LANG_DOWN_SYNTAX_EXT
	Syntaxe: %Fdown [surnom] [niveau]
	
	Enlève tous les modes. Si le canal est modéré (+m), VOICE sera réinitialisé, à moins
	d'avoir visé un autre usager par le fondateur. Il est aussi possible de spécifier un
	niveau. Ce paramètre peut être l'un des suivants:
	 - q - Fondateur de canal (modes +qo) (dépend du IRCd)
	 - a - Opérateur protégé / Administrateur (modes +ao) (dépend du IRCd)
	 - o - Opérateur de canal (mode +o)
	 - h - Demi-opérateur (HalfOp) (mode +h) (dépend du IRCd)
	 - v - 'Voice' (mode +v)
	Si un niveau cible est donné, les privilèges courants des usagers seront remplacés
	par celui ciblé.
	
	Note: Un surnom peut être spécifié seulement par le fondateur du canal ou par un
	administrateur racine des services (ROOT).

# Ignore
LANG_IGNORE_SYNTAX
	Syntaxe: %Fignore {add | del | list | clear} [expiration] [surnom | masque]
LANG_IGNORE_SYNTAX_EXT
	Syntaxe: %Fignore {add | del} [expiration] [surnom | masque]
	         %Fignore {list | clear}
	
	Permets aux administrateurs des services de forcer les services à ignorer un surnom
	pour une certaine quantité de temps ou jusqu'au prochain redémarrage des services.
	Le format de temps par défaut est en secondes. Les combinaisons d'unités de temps ne sont
	pas permises.
	Pour que l'ignore fonctionne jusqu'au prochain redémarrage des services, entrez 0
	pour le paramètre de temps d'expiration.

# Info
LANG_INFO_SYNTAX
	Syntaxe: %Finfo [all]
LANG_INFO_SYNTAX_EXT
	Syntaxe: %Finfo [all]
	Liste l'information à propos d'un canal.
	L'option 'all' est limitée aux fondateurs et aux administrateurs des services.

# Invite
LANG_INVITE_SYNTAX
	Syntaxe: %Finvite [surnom]
LANG_INVITE_SYNTAX_EXT
	Syntaxe: %Finvite [surnom]
	Invite un usager sur le canal s'il est présent sur le réseau.

# KB
LANG_KICKBAN_SYNTAX
	Syntaxe: {%Fkickban | %Fkb} [surnom | [surnom!]ident@hôte] [raison]
LANG_KICKBAN_SYNTAX_EXT
	Syntaxe: {%Fkickban | %Fkb} [surnom!]ident@hôte] [raison]
	
	Expulsera et bannira du canal un usager ou tous les usagers correspondants au masque
	donné. Si un masque est donné et que quelqu'un qui ne devrait pas être expulsé selon
	les configurations du canal, correspond aussi à ce masque, la commande sera abandonnée
	avant que quiconque soit expulsé ou banni.

# KICK
LANG_KICK_SYNTAX
	Syntaxe: {%Fkick | %Fk} [surnom | surnom[!ident@hôte]] [raison]
LANG_KICK_SYNTAX_EXT
	Syntaxe: {%Fkick | %Fk} surnom[!ident@hôte]] [raison]
	
	Expulsera du canal l'usager spécifié ou tous les usagers correspondants au masque.

# Kill
LANG_KILL_SYNTAX
	Syntaxe: %Fkill surnom raison
LANG_KILL_SYNTAX_EXT
	Syntaxe: %Fkill surnom raison
	
	Forcera la déconnexion d'un usager du réseau (même chose que /kill).
	Opérateurs IRC seulement. Cette commande peut être désactivée.

# Module Info
LANG_MINFO
	Syntaxe: {%Fminfo | %Fmodinfo}
	Montre l'information sur le module qui fournit les commandes fantaisistes.

# Mute / Unmute
LANG_MUTE_SYNTAX
	Syntaxe: %Fmute [surnom | masque] (!dépendant du IRCd!)
LANG_MUTE_SYNTAX_EXT
	Syntaxe: %Fmute [surnom | masque]
	
	Bannis du canal ou réduit au silence le surnom ou les usagers correspondants au masque.
	(Dépendant du IRCd)

LANG_UNMUTE_SYNTAX
	Syntaxe: %Funmute [surnom | masque] (!dépendant du IRCd!)
LANG_UNMUTE_SYNTAX_EXT
	Syntaxe: %Funmute [surnom | masque]
	
	Débanni du canal ou redonne le droit de parole à un surnom ou aux usagers
	correspondants au masque. (dépendant du IRCd)

# Mode
LANG_MODE_SYNTAX
	Syntaxe: %Fmode mode(s)
LANG_MODE_SYNTAX_EXT
	Syntaxe: %Fmode mode(s)
	
	Installera les modes spécifiés sur un canal.
	Opérateurs IRC seulement. Cette commande peut être désactivée.IRC Operators only.

# Prepend
LANG_PREPENDT_SYNTAX
	Syntax: {%Fprependtopic | %Ftprepend} text to be prepended to topic
LANG_PREPENDT_SYNTAX_EXT
	Syntax: {%Fprependtopic | %Ftprepend} text to be prepended to topic
	
	Prepends the given text to the channel's current topic.
	This command is most useful in conjunction with SET TOPICLOCK

# Rappend
LANG_RAPPEND_SYNTAX
	Syntaxe: {%Frappend | %Ftrapp} texte à remplacer au dernier ajout au TOPIC
LANG_RAPPEND_SYNTAX_EXT
	Syntaxe: {%Frappend | %Ftrapp} texte à remplacer au dernier ajout au TOPIC
	
	Remplace le dernier bout de texte qui avait été ajouté à l'ancien TOPIC existant.
	Si aucun texte n'est donné, le dernier bout de texte ajouté antérieurement sera
	enlevé. Cette commande est plus utile en conjonction avec  SET TOPICLOCK

# Rprepend
LANG_RPREPEND_SYNTAX
	Syntax: {%Frprepend | %Ftrprep} text to replace last prepend to topic
LANG_RPREPEND_SYNTAX_EXT
	Syntax: {%Frprepend | %Ftrprep} text to replace last prepend to topic
	
	Replaces the last prepend to the topic with the given text.
	If no replacement text is given, the last prepend is removed.
	This command is most useful in conjunction with SET TOPICLOCK

#
# SET
#
LANG_SET_SYNTAX
	Syntaxe: %Fset option paramètres
	Options disponibles: founder, successor, desc, email, url, entrymsg, bantype, mlock,
	        keeptopic, topiclock, opnotice, peace, private, restricted, secure, secureops,
	        securefounder, signkick, xop, noexpire (limité aux administrateurs des services)
	        dontkickops, dontkickvoices, greet, fantasy, symbiosis, nobot (limité aux
	        administrateurs des services)
LANG_SET_SYNTAX_EXT
	Syntaxe: %Fset option paramètres
	Options disponibles:
	        founder       -  Relie le canal à un fondateur
	        successor     -  Relie le canal à un successeur
	        desc          -  Installe une description du canal
	        url           -  Associe un lien hypertexte (URL) avec le canal
	        email         -  Associe une adresse de courriel avec le canal
	        entrymsg      -  Instaure un message d'entrée à envoyer à ceux qui entrent
	                            dans le canal
	        bantype       -  Décide du type de masque qui sera utilisé par les services
	                            pour le bannissement (BAN)
	        mlock         -  Verrouiller ou non les modes du canal
	        keeptopic     -  Conserver le TOPIC du canal lorsqu'il n'est plus utilisé
	        topiclock     -  Obliger ou non à utiliser la commande TOPIC pour changer
	                            celui-ci.
	        opnotice      -  Envoyer ou non une notice ou non quand les commandes OP et
	                            DEOP sont utilisées.
	        peace         -  Réguler ou non l'utilisation des commandes critiques
	        private       -  Cacher ou non le canal de la commande LIST
	        restricted    -  Restreindre ou non l'accès au canal
	        secure        -  Activer ou non les options de sécurité des services
	        secureops     -  Restreindre ou non le contrôle aux usagers ayant le statut OP
	        securefounder -  Restreindre ou non le contrôle du statut de fondateur de canal
	        signkick      -  Montre ou non le surnom qui a exécuté la commande KICK
	        xop           -  Basculer le système des privilèges des usagers en mode XOP
	                            ou en niveaux numériques
	        noexpire      -  Empêche ou non le canal d'expirer normalement. (limité aux
	                            administrateurs de services)
	        dontkickops   -  Protège ou non les opérateurs du canal contre les expulsions
	                            (KICKs) par les robots des services.
	        dontkickvoices - Protège ou non les usagers VOICE (+v) contre les expulsions
	                            (KICKs) par les robots des services.
	        greet         -  Active ou non le message d'entrée
	        fantasy       -  Active ou non les commandes fantaisistes du canal (lire
	                            l'aide tout d'abord!)
	        symbiosis     -  Permettre ou non au robot d'agir comme un robot réel.
	        nobot         -  Prévenir ou non qu'un robot soit assigné à un canal (limité
	                            aux administrateurs des services)

# Set subcommands
## Chanserv options
LANG_SET_FOUNDER_SYNTAX
	Syntaxe: %Fset founder surnom
LANG_SET_FOUNDER_SYNTAX_EXT
	Syntaxe: %Fset founder surnom
	Change le fondateur d'un canal. Le nouveau surnom doit être enregistré.
LANG_SET_SUCCESSOR_SYNTAX
	Syntaxe: %Fset successor surnom
LANG_SET_SUCCESSOR_SYNTAX_EXT
	Syntaxe: %Fset successor nick
	Change le successeur d'un canal. Si le surnom du fondateur expire ou est effacé
	pendant que le canal est encore enregistré, le successeur deviendra le nouveau
	fondateur du canal. Le nouveau surnom doit être enregistré.
LANG_SET_DESC_SYNTAX
	Syntaxe: %Fset desc description
LANG_SET_DESC_SYNTAX_EXT
	Syntaxe: %Fset desc description
	Installe une description du canal, qui est visible avec les commandes INFO et LIST.
LANG_SET_URL_SYNTAX
	Syntaxe: %Fset url [lien (URL)]
LANG_SET_URL_SYNTAX_EXT
	Syntaxe: %Fset url [lien (URL)]
	Associe le lien hypertexte (URL) avec un canal.
	Ce lien sera affiché à chaque fois qu'un usager demande de l'information sur le canal
	par la commande INFO.
	Si aucun lien n'est fourni, l'ancien sera effacé.
LANG_SET_EMAIL_SYNTAX
	Syntaxe: %Fset email [courriel]
LANG_SET_EMAIL_SYNTAX_EXT
	Syntaxe: %Fset email [courriel]
	Associe une adresse de courriel avec le canal.
	Cette adresse sera affichée à chaque fois qu'un usager demande de l'information sur le
	canal par la commande INFO.
	Si aucune adresse n'est fournie, l'ancienne sera effacée.
LANG_SET_ENTRYMSG_SYNTAX
	Syntaxe: %Fset entrymsg [message_d'entrée],
LANG_SET_ENTRYMSG_SYNTAX_EXT
	Syntaxe: %Fset entrymsg [message_d'entrée]
	Instaure un message à être envoyé par /notice à tous les usagers qui entrent dans le
	canal.
	Si aucun message n'est fourni, plus aucun message d'entrée ne sera envoyé.
LANG_SET_BANTYPE_SYNTAX
	Syntaxe: %Fset bantype {0 | 1 | 2 | 3}
LANG_SET_BANTYPE_SYNTAX_EXT
	Syntaxe: %Fset bantype {0 | 1 | 2 | 3}
	Décide du type de masque qui sera utilisé par les services lorsqu'ils auront besoin
	de bannir un ou plusieurs usagers du canal.
	
	bantype est un nombre entre 0 et 3, qui signifie:
	0: bannir sous la forme *!ident@hôte
	1: bannir sous la forme *!*ident@hôte
	2: bannir sous la forme *!*@hôte
	3: bannir sous la forme *!*ident@*.domaine
LANG_SET_MLOCK_SYNTAX
	Syntaxe: %Fset mlock [modes]
LANG_SET_MLOCK_SYNTAX_EXT
	Syntaxe: %Fset mlock [modes]
	Verouiller ou non les modes d'un canal.
	Ceci vous permet de définir certains modes de canal pour être toujours appliqués ou
	pas, ou libre d'être activées ou non.
LANG_SET_KEEPTOPIC_SYNTAX
	Syntaxe: %Fset keeptopic {on | off}
LANG_SET_KEEPTOPIC_SYNTAX_EXT
	Syntaxe: %Fset keeptopic {on | off}
	Active ou non la rétention du TOPIC d'un canal. Quand activé, le topic restera en
	mémoire même après que le dernier usager quitte le canal, et sera restauré la
	prochaine fois que le canal sera créé.
LANG_SET_TOPICLOCK_SYNTAX
	Syntaxe: %Fset topiclock {on | off}
LANG_SET_TOPICLOCK_SYNTAX_EXT
	Syntaxe: %Fset topiclock {on | off}
	Active ou non le verrouillage du TOPIC d'un canal. Quand activé, les services ne
	permettront pas que le TOPIC soit changé, excepté par la commande TOPIC.
LANG_SET_OPNOTICE_SYNTAX
	Syntaxe: %Fset opnotice {on | off}
LANG_SET_OPNOTICE_SYNTAX_EXT
	Syntaxe: %Fset opnotice {on | off}
	Active ou non la notification aux opérateurs d'un canal. Quand activé, les services
	enverront une notice au canal lorsque les commandes OP et DEOP sont utilisées.
LANG_SET_PEACE_SYNTAX
	Syntaxe: %Fset peace {on | off}
LANG_SET_PEACE_SYNTAX_EXT
	Syntaxe: %Fset peace {on | off}
	Active ou non l'option de régulation des commandes critiques sur un canal. Quand
	activé, un usager ne pourra pas expulser (KICK), bannir (BAN), ou enlever le statut
	d'un autre usager qui a un niveau supérieur ou égal à lui via les services.
LANG_SET_PRIVATE_SYNTAX
	Syntaxe: %Fset private {on | off}
LANG_SET_PRIVATE_SYNTAX_EXT
	Syntaxe: %Fset private {on | off}
	Active ou non l'option privée pour un canal.
	Quand activé, %R%CS LIST n'inclura pas le canal dans aucune liste.
LANG_SET_RESTRICTED_SYNTAX
	Syntaxe: %Fset restricted {on | off}
LANG_SET_RESTRICTED_SYNTAX_EXT
	Syntaxe: %Fset restricted {on | off}
	Active ou non l'option d'accès restreint pour un canal.
	Quand activé, les usagers qui n'auraient normalement pas le droit d'être opérateur de
	canal (les usagers avec un niveau d'accès négatif et, si secure ops est actif, les
	usagers qui ne sont pas sur la liste d'accès) seront expulsés (KICK) et bannis (BAN)
	du canal.
LANG_SET_SECURE_SYNTAX
	Syntaxe: %Fset secure {on | off}
LANG_SET_SECURE_SYNTAX_EXT
	Syntaxe: %Fset secure {on | off}
	Active ou non les caractéristiques de sécurité de %CS pour un canal. Quand activé,
	seuls les usagers ayant leur surnom enregistré et étant identifiés avec leur mot de
	passe obtiendront l'accès qui leur est destiné dans la liste d'accès du canal.
LANG_SET_SECUREOPS_SYNTAX
	Syntaxe: %Fset secureops {on | off}
LANG_SET_SECUREOPS_SYNTAX_EXT
	Syntaxe: %Fset secureops {on | off}
	Active ou non l'option SECURE OPS d'un canal.
	Quand activé, les usagers qui ne sont pas sur la liste d'accès ne pourront pas
	obtenir le statut d'opérateur de canal.
LANG_SET_SECUREFOUNDER_SYNTAX
	Syntaxe: %Fset securefounder {on | off}
LANG_SET_SECUREFOUNDER_SYNTAX_EXT
	Syntaxe: %Fset securefounder {on | off}
	Active ou non l'option SECURE FOUNDER d'un canal. Quand activé, seul le véritable
	fondateur sera capable d'effacer le canal, en changer son mot de passe, son fondateur
	et son successeur, et non les autres même s'ils sont identifiés avec les services.
LANG_SET_SIGNKICK_SYNTAX
	Syntaxe: %Fset signkick {on | niveau | off}
LANG_SET_SIGNKICK_SYNTAX_EXT
	Syntaxe: %Fset signkick {on | niveau | off}
	Active ou non la signature des expulsions (KICK) pour un canal. Si activé, les
	expulsions (KICK) envoyées via la commande 'KICK' des services, auront leur auteur
	mentionné dans la raison du 'KICK'. Si vous utilisez 'LEVEL', ceux qui ont un
	niveau supérieur ou égal au niveau de 'SIGNKICK' ne seront pas concernés par cette
	option. Voir '%Fhelp levels' pour plus d'information.
LANG_SET_XOP_SYNTAX
	Syntaxe: %Fset xop {on | off}
LANG_SET_XOP_SYNTAX_EXT
	Syntaxe: %Fset xop {on | off}
	Active ou non le système de liste xOP pour un canal. Quand activé, vous devrez
	utiliser les commandes 'AOP/SOP/VOP' pour gérer les privilèges des usagers. Sinon,
	vous devrez utiliser la commande 'ACCESS'.
LANG_SET_NOEXPIRE_SYNTAX
	Syntaxe: %Fset noexpire {on | off}
LANG_SET_NOEXPIRE_SYNTAX_EXT
	Syntaxe: %Fset noexpire {on | off}
	Empêche ou non le canal d'expirer normalement.
	Cette commande est limitée aux administrateurs des services.

## Botserv options
LANG_SET_DONTKICKOPS_SYNTAX
	Syntaxe: %Fset dontkickops {on | off}
LANG_SET_DONTKICKOPS_SYNTAX_EXT
	Syntaxe: %Fset dontkickops {on | off}
	Active ou non la protection des opérateurs sur un canal. Quand activé, les opérateurs
	ne seront jamais expulsés par le robot de service même s'ils n'ont pas de correspondance
	avec le niveau 'NOKICK'.
LANG_SET_DONTKICKVOICES_SYNTAX
	Syntaxe: %Fset dontkickvoices {on | off}
LANG_SET_DONTKICKVOICES_SYNTAX_EXT
	Syntaxe: %Fset dontkickvoices {on | off}
	Active ou non la protection des usagers avec 'VOICE' (+v) sur un canal. Quand
	activé, les usagers avec 'VOICE' (+v) ne seront jamais expulsés par le robot de
	service même s'ils n'ont pas de correspondance avec le niveau 'NOKICK'.
LANG_SET_GREET_SYNTAX
	Syntaxe: %Fset greet {on | off}
LANG_SET_GREET_SYNTAX_EXT
	Syntaxe: %Fset greet {on | off}
	Active ou non le message d'entrée d'un canal. Quand activé, le robot enverra le
	message d'entrée aux usagers qui joignent le canal, s'ils ont un niveau d'accès
	suffisant pour le voir.
LANG_SET_FANTASY_SYNTAX
	Syntaxe: %Fset fantasy {on | off}
LANG_SET_FANTASY_SYNTAX_EXT
	Syntaxe: %Fset fantasy {on | off}
	Active ou non le mode fantaisiste sur un canal. Quand activé, les usagers pourront
	utiliser les commandes %Fop, %Fcmdlist et %Fhelp. À noter que les usagers voulant
	utiliser les commandes fantaisistes DOIVENT avoir un niveau d'accès suffisamment
	élevé pour 'FANTASIA' et un autre niveau dépendamment de la commande si requis.
	
	IMPORTANT: Désactiver ceci désactivera toutes les commandes fantaisistes sur le canal
	et la réactivation devra se faire via %BS.
LANG_SET_SYMBIOSIS_SYNTAX
	Syntaxe: %Fset symbiosis {on | off}
LANG_SET_SYMBIOSIS_SYNTAX_EXT
	Syntaxe: %Fset symbiosis {on | off}
	Active ou non le mode symbiose sur un canal. Quand activé, le robot fera tout ce qui
	est normalement fait par %CS sur les canaux, comme 'MODE', 'KICK', et même le
	message d'entrée.
	Note: Quand désactivé, la plupart des commandes fantaisistes seront tout de même
	accessibles.
LANG_SET_NOBOT_SYNTAX
	Syntaxe: %Fset nobot {on | off}
LANG_SET_NOBOT_SYNTAX_EXT
	Syntaxe: %Fset nobot {on | off}
	Cette option rend le canal impossible à recevoir un robot. Si un robot est déjà
	assigné au canal, il ne le sera plus aussitôt cette option activée.

LANG_SET_UNKNOWN_OPTION
	Option SET inconnue: %s.

# Shun
LANG_SHUN_SYNTAX
	Syntaxe: %Fshun [add | del] [+expiration] {surnom | usager@hôte} [raison]
LANG_SHUN_SYNTAX_EXT
	Syntaxe: %Fshun [add] [+expiration] {surnom | usager@hôte} [raison]
	         %Fshun del {surnom | usager@hôte}
	
	Cette commande ajoute ou enlève une interdiction de parole (SHUN) sur le surnom ou le
	masque donné. Si 'ADD' ou 'DEL' n'est pas spécifié, 'ADD' est assumé par défaut.
	Le temps donné est considéré être en jours, à moins qu'un autre intervalle soit
	spécifié. Les alternatives possibles sont: secondes (s), minutes (m), heures (h), et
	jours (d). À noter que les combinaisons ne sont pas possibles! Seulement la première
	partie sera lue. Si aucune raison n'est spéficiée, celle par défaut sera utilisée. Si
	le masque débute par '+', un temps d'expiration DOIT être donné!
	
	À noter que 'SHUN DEL' est seulement garanti de fonctionner pour les 'SHUN'
	ajoutés par la commande du même nom et seulement si l'usager est encore en ligne.
	Dans tous les autres cas, utilisez usager@masque. Quand vous utilisez 'SHUN'
	'add' ou 'del', 'ADD' doit être donné.

# Staff
LANG_STAFF
	Syntaxe: {%Fstaff | %Fircops}
	Ceci montrera la liste des opérateurs IRC (IRCops) qui sont disponibles.

# Status mode commands (!op/!deop/...)
LANG_STATUS_MODE_SYNTAX
	Syntax: %F%s [[nick | mask] [[nick | mask] [...]]]
LANG_STATUS_MODE_SYNTAX_EXT
	Syntax: %F%s [[nick | mask] [[nick | mask] [...]]]
	
	Changes status modes on the channel for the given user(s) or all users matching given mask(s).
	This command accepts a space separated list of nicks, masks or a combination thereof.
	Depending on channel settings, this command will not take modes from users with a
	higher access level than the issuer.

# Suspend
LANG_SUSPEND_SYNTAX
	Syntaxe: %Fsuspend [raison]
LANG_SUSPEND_SYNTAX_EXT
	Syntaxe: %Fsuspend [raison]
	Empêche quiconque d'utiliser un canal, mais préserve toutes les données enregistrées
	s'y rapportant jusqu'à temps que la suspension soit levée.

# Sversion
LANG_SVERSION
	Syntaxe: %Fsversion
	Montre de l'information sur la version les services IRC Anope.

# Sync
LANG_SYNC_SYNTAX
	Syntaxe: %Fsync
LANG_SYNC_SYNTAX_EXT
	Syntaxe: %Fsync
	
	Cette commande donnera à tous les usagers présentement dans le canal le niveau
	d'accès qu'ils ont selon la liste d'accès. Les usagers qui ont un niveau supérieur à
	celui qu'il sont censés avoir, seront rabaissés à ce niveau.
	
	L'utilisation de cette commande est réservée au fondateur du canal.

# TB
LANG_TBAN_SYNTAX
	Syntaxe: {%Ftb | %Ftban} [surnom | [surnom!]ident@hôte] [temps]
LANG_TBAN_SYNTAX_EXT
	Syntaxe: {%Ftb | %Ftban} [surnom | [surnom!]ident@hôte] [temps]
	
	Ceci bannira (BAN) le surnom ou le masque donné du canal pour un temps spécifié. Si
	aucun temps n'est donné, il sera considéré comme 1 heure par défaut. Le temps devrait
	être donné selon ce format: <nombre><(s)/m/h/d>.
	Exemple: %Ftb Viper 10m ce qui bannira Viper pour 10 minutes.
	
	Note: si les services sont redémarrés avant que le bannissement expire, il sera alors
	automatiquement enlevé.

# TKB
LANG_TKBAN_SYNTAX
	Syntaxe: %Ftkb [surnom | [surom!]ident@hôte] [temps] [raison]
LANG_TKBAN_SYNTAX_EXT
	Syntaxe: %Ftkb ou %Ftkban [surnom | [surnom!]ident@hôte] [temps] [raison]
	
	Ceci expulsera (KICK) et bannira (BAN) temporairement du canal le ou les usagers
	correspondants au masque pour un temps donné. Si un masque est donné et touche un
	usager qui ne devrait pas être banni selon les configurations du canal, la commande
	est annulée avant que quiconque ne soit expulsé. Aussi, sachez que cette commande
	ne fonctionnera pas pour les adresses IPs pour des raisons de protection de l'anonymat.
	Si le temps n'est pas fourni ou en est un qui est non valide, 1 heure sera instaurée
	comme valeur par défaut. Le temps devrait être donné selon ce format:
	<nombre><(s)/m/h/d>. Exemple: %Ftkb Viper 10m Bye bannira et expulsera Viper pour
	10 minutes.
	
	Note: si les services sont redémarrés avant que le bannissement expire, il sera alors
	automatiquement enlevé.

# Topic
LANG_TOPIC_SYNTAX
	Syntaxe: %Ftopic [topic]
	Syntaxe: %Fctopic
LANG_TOPIC_SYNTAX_EXT
	Syntaxe: %Ftopic [topic]
	Syntaxe: %Fctopic
	
	Instaure un sujet de canal (TOPIC). Si aucun n'est fourni, celui actif est affiché dans
	le canal. La commande CTOPIC sert à enlever entièrement le 'TOPIC'.
	Cette commande est plus utile en conjonction avec SET TOPICLOCK.

# TSHUN
LANG_TSHUN_SYNTAX
	Syntaxe: %Ftshun [add | del] surnom [raison]
LANG_TSHUN_SYNTAX_EXT
	Syntaxe: %Ftshun [add] [surnom] [raison]
	         %Ftshun del [surnom]
	
	Cette commande ajoute ou efface un 'shun' temporaire sur l'usager donné. Si 'add'
	ou 'del' n'est pas spécifié, 'add' est utilisé. Cette commande ne peut pas être
	utilisée sur des opérateurs IRC (IRCops). À noter que ce 'shun' sera enlevé
	automatiquement quand l'usager se déconnectera et n'affectera qu'une session.

# Unban
LANG_UNBAN_SYNTAX
	Syntax: %Funban [nick | mask]
LANG_UNBAN_SYNTAX_EXT
	Syntax: %Funban [nick | mask]
	
	Removes bans on the given mask from the channel or removes all bans matching given user.
	
	Note: checking against a users IP address is done for opers or when unbanning yourself.

# UP
LANG_UPDOWN_SYNTAX
	Syntaxe: {%Fup | %Fdown} [surnom]
LANG_UP_SYNTAX
	Syntaxe: %Fup [surnom]
LANG_UP_SYNTAX_EXT
	Syntaxe: %Fup [surnom]
	
	Donne à l'usager les modes auxquels il a accès. Ceci enlèvera les modes dont l'usager
	n'est pas sencé avoir accès.
	
	Note: Un surnom ne peut seulement être spécifié par le fondateur du canal ou par un
	administrateur racine des services (ROOT).


#
# XOP & ACCESS, LEVELS
#
LANG_SOP_SYNTAX
	Syntaxe: %Fsop {add | del | list | clear} [surnom | numéro_d'entrée]
LANG_SOP_SYNTAX_EXT
	Syntaxe: %Fsop add surnom
	         %Fsop del {surnom | numéro_d'entrée | list}
	         %Fsop list [masque | list]
	         %Fsop clear
	
	Gère la liste SOP (SuperOP) d'un canal. La liste SOP donne aux usagers
	tous les droits donnés à ceux de la liste AOP, et ajoute ceux nécessaires pour
	utiliser les listes 'AutoKick' et 'BadWords', envoyer des mémos de canal, etc.
	
	Cette commande peut avoir été désactivée pour votre canal, et dans ce cas vous devez
	utiliser le format d'accès numérique. Voir 'HELP ACCESS' pour l'information sur
	la liste d'accès au format numérique, et 'HELP SET XOP' pour savoir comment
	basculer entre les deux formats.

LANG_AOP_SYNTAX
	Syntaxe: %Faop {add | del | list | clear} [surnom | numéro_d'entrée]
LANG_AOP_SYNTAX_EXT
	Syntaxe: %Faop add surnom
	         %Faop del {surnom | numéro_d'entrée | list}
	         %Faop list [masque | list]
	         %Faop clear
	
	Gère la liste AOP (AutoOP) d'un canal. La liste AOP donne aux usagers
	les droits de recevoir le statut d'opérateur de canal (+o) automatiquement, de se
	débannir ou s'inviter eux-mêmes si besoin est, de recevoir leur message d'entrée,
	etc.
	
	Cette commande peut avoir été désactivée pour votre canal, et dans ce cas vous devez
	utiliser le format d'accès numérique. Voir 'HELP ACCESS' pour l'information sur
	la liste d'accès au format numérique, et 'HELP SET XOP' pour savoir comment
	basculer entre les deux formats.

LANG_HOP_SYNTAX
	Syntaxe: %Fhop {add | del | list | clear} [surnom | numéro_d'entrée]
LANG_HOP_SYNTAX_EXT
	Syntaxe: %Fhop add surnom
	         %Fhop del {surnom | numéro_d'entrée | list}
	         %Fhop list [masque | list]
	         %Fhop clear
	
	Gère la liste HOP (HalfOP) d'un canal. La liste HOP donne aux usagers
	le droit d'être demi-op (+h) sur le canal.
	
	Cette commande peut avoir été désactivée pour votre canal, et dans ce cas vous devez
	utiliser le format d'accès numérique. Voir 'HELP ACCESS' pour l'information sur
	la liste d'accès au format numérique, et 'HELP SET XOP' pour savoir comment
	basculer entre les deux formats.

LANG_VOP_SYNTAX
	Syntaxe: %Fvop {add | del | list | clear} [surnom | numéro_d'entrée]
LANG_VOP_SYNTAX_EXT
	Syntaxe: %Fvop add surnom
	         %Fvop del {surnom | numéro_d'entrée | list}
	         %Fvop list [masque | list]
	         %Fvop clear
	
	Gère la liste VOP (Voice) d'un canal. La liste VOP donne aux usagers
	le droit d'être automatiquement 'voiced' (+v) ou de s'approprier ce statut s'ils
	ne l'ont pas.
	
	Cette commande peut avoir été désactivée pour votre canal, et dans ce cas vous devez
	utiliser le format d'accès numérique. Voir 'HELP ACCESS' pour l'information sur
	la liste d'accès au format numérique, et 'HELP SET XOP' pour savoir comment
	basculer entre les deux formats.

LANG_ACCESS_SYNTAX
	Syntaxe: %Faccess {add | del | list | clear} [surnom [niveau] | masque | list]
LANG_ACCESS_SYNTAX_EXT
	Syntaxe: %Faccess add surnom niveau
	         %Faccess del {surnom | numéro_d'entrée | list}
	         %Faccess list [masque | list]
	         %Faccess clear
	
	Gère la liste d'accès au format numérique d'un canal. La liste d'accès spécifie
	quels usagers ont le droit d'avoir le statut d'opérateur de canal ou accéder aux
	commandes sur un canal.
	Différents niveaux d'usagers permettent différentes sous-classes de privilèges; voir
	%R%CS HELP ACCESS LEVELS pour plus d'information. Tout surnom n'étant pas sur
	la liste d'accès, a un niveau d'accès de 0.

LANG_LEVELS_SYNTAX
	Syntaxe: %Flevels {set |dis[able] | list | reset} [item [niveau]]
LANG_LEVELS_SYNTAX_EXT
	Syntaxe: %Flevels set type niveau
	         %Flevels {dis | disable} type
	         %Flevels list
	         %Flevels reset
	
	La commande LEVELS permet un contrôle pointilleux en regard au format d'accès
	numérique utilisé sur un canal. Avec cette commande, vous pouvez définir le niveau
	d'accès requis pour les fonctions administratives. (Les commandes 'SET FOUNDER' et
	'SET PASSWORD' ainsi que la commande expliquée actuellement, sont toujours
	réservées au fondateur du canal.)
	
	LEVELS SET permet le niveau d'accès pour une fonction ou un groupe de fonctions
	d'être changé.
	LEVELS DISABLE (ou DIS comme diminutif) désactive une option automatique
	ou interdit l'accès à une fonction par quiconque d'autre que le fondateur du canal.
	LEVELS LIST montre les niveaux d'accès courants pour chaque fonction ou groupe
	de fonctions.
	LEVELS RESET réinitialise les niveaux à leurs valeurs par défaut d'un nouveau
	canal créé. (Voir %R%CS HELP ACCESS LEVELS pour plus d'information.)
	
	Pour une liste des options et fonctions qui peuvent être configurées, voir
	'HELP LEVELS DESC'.

