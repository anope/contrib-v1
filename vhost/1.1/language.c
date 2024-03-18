#include "language.h"

const boolean coloredText_iSupport[16]  = {1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0};
const cText_t coloredText[16][6]        = {
    /* english */
    {{"[\037New\037]", "[\037Updated\037]", "[\037Activated\037]", "[\037Rejected\037]", "Host Request for %s activated by %s (%s)", "Host Request for %s rejected by %s (%s)"},
    {"\00312[\037New\037]\003", "\00311[\037Updated\037]\003", "\00309[\037Activated\037]\003", "\00304[\037Rejected\037]\003", "\00309Host Request for %s activated by %s (%s)\003", "\00304Host Request for %s rejected by %s (%s)\003"}},
    /* japanese (jis encoding) */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
    /* japanese (euc encoding) */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
    /* japanese (sjis encoding)  */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
    /* spanish */
    {{"[\037Nuevo\037]", "[\037Actualizado\037]", "[\037Activado\037]", "[\037Rechazado\037]", "Petición de Host para %s activado por %s (%s)", "Petición de Host de %s rechazado por %s (%s)"},
    {"\00312[\037Nuevo\037]\003", "\00311[\037Actualizado\037]\003", "\00309[\037Activado\037]\003", "\00304[\037Rechazado\037]\003", "\00309Petición de Host para %s activado por %s (%s)\003", "\00304Petición de Host para %s rechazado por %s (%s)\003"}},
    /* portuguese */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
    /* french */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
    /* turkish */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
    /* italian */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
    /* german */
    {{"[\037Neu\037]", "[\037Verändert\037]", "[\037Aktiviert\037]", "[\037Abgelehnt\037]", "vHost für %s wurde gesetzt von %s (%s)", "vHost für %s wurde abgelehnt von %s (%s)"},
    {"\00312[\037Neu\037]\003", "\00311[\037Verändert\037]\003", "\00309[\037Aktiviert\037]\003", "\00304[\037Abgelehnt\037]\003", "\00309vHost für %s wurde gesetzt von %s (%s)\003", "\00304vHost für %s wurde abgelehnt von %s (%s)\003"}},
    /* catalan */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
    /* greek */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
    /* dutch */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
    /* russian */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
    /* hungarian */
    {{"[\037Új\037]", "[\037Frissített\037]", "[\037Aktivált\037]", "[\037Elutasított\037]", "A host kérelem a(z) %s nick-hez aktiválásra került %s által (%s)", "A host kérelem a(z) %s nick-hez elutasításra került %s által (%s)"},
    {"\00312[\037Új\037]\003", "\00311[\037Frissített\037]\003", "\00309[\037Aktivált\037]\003", "\00304[\037Elutasított\037]\003", "\00309A host kérelem a(z) %s nick-hez elutasításra került %s (%s)\003 által", "\00304A host kérelem a(z) %s nick-hez elutasításra került %s által (%s)\003"}},
    /* polish */
    {{NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL}},
};

int my__def_language(void)
{
    if (coloredText_iSupport[NSDefLanguage])
        return NSDefLanguage;
    else
        return LANG_EN_US;
}

void my__add_languages(void)
{
/* English translation */
char *langtable_en_us[] = {
    /* LNG_REQUEST_SYNTAX */
    "Syntax: \002REQUEST \037vHost\037\002",
    /* LNG_REQUESTED */
    "Your vHost has been requested. Please wait for an operator to activate it.",
    /* LNG_REQUEST_WAIT */
    "Please wait %d seconds before requesting a new vHost.",
    /* LNG_REQUEST_MEMO */
    "[auto memo] vHost \002%s\002 has been requested.",
    /* LNG_ACTIVATE_SYNTAX */
    "Syntax: \002{ACTIVATE|A} \037nick\037 [\037reason\037]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Note: Wildcard nick matching is enabled. Supported wildcard characters are '?' and '*'.\n"
#else
    "  Note: Wildcard nick matching is disabled.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Note: Regex nick matching is enabled. Prefix match string with '!'.",
#else
    "  Note: Regex nick matching is disabled.",
#endif
    /* LNG_ACTALL_SYNTAX */
    "Syntax: \002ACTALL [\037reason\037]\002",
    /* LNG_ACTIVATED */
    "vHost for '%s' has been activated.",
    /* LNG_ACTIVATE_MEMO */
    "[auto memo] Your requested vHost has been approved and set.",
    /* LNG_ACTIVATE_MEMO_REASON */
    "[auto memo] Your requested vHost has been approved and set. Reason: %s",
    /* LNG_REJECT_SYNTAX */
    "Syntax: \002{REJECT|R} \037nick\037 [\037reason\037]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Note: Wildcard nick matching is enabled. Supported wildcard characters are '?' and '*'.\n"
#else
    "  Note: Wildcard nick matching is disabled.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Note: Regex nick matching is enabled. Prefix match string with '!'.",
#else
    "  Note: Regex nick matching is disabled.",
#endif
    /* LNG_REJALL_SYNTAX */
    "Syntax: \002REJALL [\037reason\037]\002",
    /* LNG_REJECTED */
    "vHost for '%s' has been rejected.",
    /* LNG_REJECT_MEMO */
    "[auto memo] Your requested vHost has been rejected.",
    /* LNG_REJECT_MEMO_REASON */
    "[auto memo] Your requested vHost has been rejected. Reason: %s",
    /* LNG_NO_REQUEST */
    "No request for nick '%s' found.",
    /* LNG_HELP */
    "    REQUEST     Request a vHost for your nick",
    /* LNG_HELP_SETTER */
    "    ACTIVATE    Approve the requested vHost of a user\n"
    "                Shortcut \002A\002 is available\n"
    "    ACTALL      Approve all requested vHosts\n"
    "    REJECT      Reject the requested vHost of a user\n"
    "                Shortcut \002R\002 is available\n"
    "    REJALL      Reject all requested vHosts\n"
    "    WAITING     Convenience command for LIST +req\n"
    "                Shortcut \002W\002 is available\n"
    "    CONFIG      Access various settings",
    /* LNG_HELP_REQUEST */
    "Request the given vHost to be activated for your nick by the\n"
    "network administrators. Please be patient while your request\n"
    "is being considered.",
    /* LNG_HELP_ACTIVATE */
    "Activate the requested vHost for the given nick.",
    /* LNG_HELP_ACTALL */
    "Activate all requested vHosts.",
    /* LNG_HELP_ACTIVATE_MEMO */
    "A memo informing the user will also be sent.",
    /* LNG_HELP_REJECT */
    "Reject the requested vHost for the given nick.",
    /* LNG_HELP_REJECT_MEMO */
    "A memo informing the user will also be sent.",
    /* LNG_HELP_REJALL */
    "Reject all requested vHosts.",
    /* LNG_HELP_REJALL_MEMO */
    "A memo informing the users will also be sent.",
    /* LNG_WAITING_SYNTAX */
    "Syntax: \002{WAITING|W} [{\037nick\037|\037num\037}]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Note: Wildcard nick matching is enabled. Supported wildcard characters are '?' and '*'.\n"
#else
    "  Note: Wildcard nick matching is disabled.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Note: Regex nick matching is enabled. Prefix match string with '!'.",
#else
    "  Note: Regex nick matching is disabled.",
#endif
    /* LNG_HELP_WAITING */
    "This command is provided for convenience. It is essentially\n"
    "the same as performing a  LIST +req  .\n"
    " \n"
    "Output can be limited either by specifying a nick, in\n"
    "which case only the request belonging to that nick will\n"
    "be displayed, or by specifying a number, in which case\n"
    "the number of entries displayed will not exceed this number.\n"
    "Numbers 0 and smaller will display the total number of open\n"
    "vHost requests.",
    /* LNG_REQUEST_FORBIDDEN */
    "The vHost you requested is forbidden.",
    /* LNG_REQUEST_FORBIDDEN_REASON */
    "The vHost you requested is forbidden. Reason: %s",
    /* LNG_CONFIG_SYNTAX */
    "Syntax: \002CONFIG \037option\037 [\037parameters\037]\002",
    /* LNG_HELP_CONFIG */
    "Allows access to various settings. \037option\037 can be one of:\n"
    " \n"
    "    EXCEPTION   Modify list of exceptions\n"
    "    REGEX       Modify list of regexes\n"
    "    SAVE_DB     Manually dump request and regex databases to files\n"
    "    LIST        List config values\n"
    "    SET         Adjust runtime variables\n"
    " \n"
    "Details for: \002CONFIG EXCEPTION\002\n"
    "Syntax: \002CONFIG EXCEPTION {ADD \037nick\037|DEL {\037#num\037|\037nick\037}|LIST}\002\n"
    " \n"
    "Allows modifying the exception list.\n"
    " \n"
    "    LIST        Display exception list, includes exception number and set-by nick for use in \002DEL\002\n"
    "    ADD         Add an exception. All group members will be exempt aswell.\n"
    "    DEL         Delete an exception by reference, see \002LIST\002\n"
    " \n"
    "Details for: \002CONFIG REGEX\002\n"
    "Syntax: \002CONFIG REGEX {ADD \037reason\037 \037regex\037|DEL {\037#num\037|\037nick\037}|LIST}\002\n"
    " \n"
    "Allows modifying the regex list.\n"
    " \n"
    "    LIST        Display regex list, includes regex number and creator nick for use in \002DEL\002\n"
    "    ADD         Add a regex string. A reason \002has\002 to be specified, use 'none' for default. BE CAREFUL!\n"
    "                To supply a reason consisting of multiple words replace spaces with '_'\n"
    "    DEL         Delete a regex string by reference, see \002LIST\002\n"
    " \n"
    "Details for: \002CONFIG SET\002\n"
    "Syntax: \002CONFIG SET \037option\037 \037value\037\002\n"
    " \n"
    "Allows temporary adjustments to runtime variables. Changes will be lost on services\n"
    "config reloads and restarts. \037option\037 can be one of:\n"
    " \n"
    "    MEMOUSER          Send memos to users on various occasions.\n"
    "    MEMOOPER          Send memos to opers on various occasions.\n"
    "    MEMOSETTERS       Send memos to host setters on various occasions.\n"
    "    DISPLAYMODE       Enable/Disable extended channel output.\n"
    "    DISPLAYCOLOR      Enable/Disable colored channel output.\n"
    "    DISPLAYMAX        Change maximum number of new requests that will be displayed.\n"
    "    TIMER             Change interval between announces.\n"
    "    REQUESTDELAY      Change delay between allowed vHost requests.\n"
    "                      \037Note\037: Services Admins are exempt from the delay.\n"
    "    DENYIDENTUPDATE   Deny ident@host requests.\n"
    " \n"
    "All options besides \002TIMER\002, \002REQUESTDELAY\002 and \002DISPLAYMAX\002 accept\n"
    "values: \0021\002 (ON) and \0020\002 (OFF).\n"
    "\002TIMER\002 accepts: \0020\002 (OFF) and \002>0\002 (INTERVAL). Minimum interval\n"
    "is 10 seconds, values between 1-9 will be adjusted accordingly.\n" 
    "\002REQUESTDELAY\002 accepts: \0020\002 (OFF) and \002>0\002 (DELAY).\n"
    "\002DISPLAYMAX\002 accepts: \0020\002 (NO LIMIT) and \002>0\002 (LIMITED).",
    /* LNG_VHOST_SYNTAX */
    "Available Options:\n"
    "- \002{WAITING|W} {\037nick\037|\037num\037}\002   - lists open vHost requests\n"
    "- \002{ACTIVATE|A} \037nick\037\002         - activate a request\n"
    "- \002{REJECT|R} \037nick\037 [\037reason\037]\002 - reject a request\n"
#ifdef SUPPORT_WILDCARD
    "  Note: Wildcard nick matching is enabled. Supported wildcard characters are '?' and '*'.\n"
#else
    "  Note: Wildcard nick matching is disabled.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Note: Regex nick matching is enabled. Prefix match string with '!'.\n"
#else
    "  Note: Regex nick matching is disabled.\n"
#endif
    " \n"
    "- \002ACTALL [\037reason\037]\002            - activate all requests\n"
    "- \002REJALL [\037reason\037]\002            - reject all requests",
    /* LNG_NO_REQUESTS */
    "No requests found.",
    /* LNG_NOT_ENOUGH_REQUESTS */
    "Not enough requests.",
    /* LNG_NO_MATCHES */
    "No requests matching '%s' found.",
    /* LNG_REGEX_DISABLED */
    "Support for regex nick matching is disabled.",
    /* LNG_WILDCARD_DISABLED */
    "Support for wildcard nick matching is disabled.",
    /* LNG_REGEX_ERROR */
    "Error in '%s': %s",
    /* LNG_CONF_SAVE_DB */
    "Done. Check services channel for possible errors.",
    /* LNG_CONF_REGEX_ADD_FAIL */
    "Could not add regex '%s'",
    /* LNG_CONF_REGEX_ADD */
    "Regex '%s' added.",
    /* LNG_CONF_REGEX_LIST */
    "#%d: '%s' by %s",
    /* LNG_CONF_REGEX_LIST_END */
    "End of regex list.",
    /* LNG_CONF_REGEX_DELETED */
    "All matching regexes deleted.",
    /* LNG_CONF_TEMPORARY */
    "Setting %s temporarily to: %d (%s)",
    /* LNG_VHOST_LISTING */
    "[\037%s\037] Listing open vHost requests...",
    /* LNG_VHOST_REQUEST_COUNT */
    "Request Count: \002%d\002",
    /* LNG_VHOST_REQUEST_DISPLAYED */
    "- Displayed %d request%s(Count: \002%d\002)",
    /* LNG_VHOST_REMINDER */
    "[\037Reminder\037] You have %d vHost%sawaiting approval. To view: !%s waiting",
    /* LNG_KEY_NICK */
    "Nick",
    /* LNG_KEY_OFFLINE */
    "offline",
    /* LNG_KEY_CURRENT */
    "Current",
    /* LNG_KEY_NONE */
    "none",
    /* LNG_KEY_REQUEST */
    "Request",
    /* LNG_KEY_AGO */
    "ago",
    /* LNG_KEY_OPER */
    "Oper",
    /* LNG_KEY_HOST */
    "Host",
    /* LNG_KEY_REASON */
    "Reason",
    /* LNG_REQUEST_ALREADY_APPROVED */
    "Your request '%s' has already been approved. Please type '/msg HostServ on' if the host isn't active.",
    /* LNG_REQUEST_REMOVED_HOST */
    "Pending vHost request for %s automatically removed. New request matched already approved host.",
    /* LNG_REQUEST_REMOVED_GROUP */
    "Pending vHost request for %s automatically removed. New request matched already approved host of grouped nick %s.",
    /* LNG_REQUEST_NOT_SHOWN */
    "[\037Notice\037] New vHost(s) have been requested that have not been displayed.",
    /* LNG_REQUEST_UPDATE_DENIED */
    "Your request has been denied. To change your ident:\n"
    "   1. Request: '/msg HostServ request *@%s'.\n"
    "   2. Change your ident in your client and reconnect.",
    /* LNG_REQUEST_AUTO_APPROVED */
    "Your request has been approved automatically",
    /* LNG_HOST_REJECTED */
    "Your requested vHost of \002%s\002 has been rejected. Reason: %s.",
    /* LNG_HOST_IDENT_REJECTED */
    "Your requested vHost of \002%s\002@\002%s\002 has been rejected. Reason: %s.",
    /* LNG_HOST_ACTIVATED */
    "Your vhost of \002%s\002 is now activated. Reason: %s.",
    /* LNG_HOST_IDENT_ACTIVATED */
    "Your vhost of \002%s\002@\002%s\002 is now activated. Reason: %s.",
    /* LNG_REQUEST_AUTO_REJECTED */
    "Your request has been rejected automatically",
    /* LNG_CONF_EXCEPTION_ADD_FAIL */
    "Could not add exception for nick '%s'.",
    /* LNG_CONF_EXCEPTION_ADD */
    "Exception for nick '%s' added.",
    /* LNG_CONF_EXCEPTION_LIST */
    "#%d: '%s' by %s",
    /* LNG_CONF_EXCEPTION_LIST_END */
    "End of exception list.",
    /* LNG_CONF_EXCEPTION_DELETED */
    "Matching exception deleted.",
    /* LNG_CONF_EXCEPTION_NOT_FOUND */
    "Entry not found on exception list.",
    /* LNG_REQUEST_REMOVED_GONE */
    "Pending vHost request for %s automatically removed. User does not exist anymore.",
    /* LNG_CONFIG_REGEX_SYNTAX */
    "Syntax: \002CONFIG REGEX {ADD \037reason\037 \037regex\037|DEL {\037#num\037|\037nick\037}|LIST}\002",
    /* LNG_CONFIG_EXCEPTION_SYNTAX */
    "Syntax: \002CONFIG EXCEPTION {ADD \037nick\037|DEL {\037#num\037|\037nick\037}|LIST}\002",
    /* LNG_USER_IS_EXEMPT */
    "Exception for user by"
};

/* Spanish translation */
char *langtable_es[] = {
    /* LNG_REQUEST_SYNTAX */
    "Sintaxis: \002REQUEST \037vHost\037\002",
    /* LNG_REQUESTED */
    "Su petición de vHost ha sido enviada. Por favor espere a que un operador lo active.",
    /* LNG_REQUEST_WAIT */
    "Por favor espere %d segundos antes de solicitar otro vHost.",
    /* LNG_REQUEST_MEMO */
    "[auto memo] vHost \002%s\002 ha sido solicitado.",
    /* LNG_ACTIVATE_SYNTAX */
    "Sintaxis: \002{ACTIVATE|A} \037nick\037 [\037razón\037]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Nota: Comodines para completado nicks están habilitados. Soportados caracteres comodines como '?' y '*'.\n"
#else
    "  Nota: Comodines para completado nicks están deshabilitados.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Nota: Expresiones regulares para completado de nicks habilitadas. El prefijo para completar series es '!'.",
#else
    "  Nota: Expresiones regulares para completado de nicks estan deshabilitadas.",
#endif
    /* LNG_ACTALL_SYNTAX */
    "Sintaxis: \002ACTALL [\037razón\037]\002",
    /* LNG_ACTIVATED */
    "El vHost de '%s' ha sido activado.",
    /* LNG_ACTIVATE_MEMO */
    "[auto memo] Su petición de vHost ha sido aprobada y ha sido activada.",
    /* LNG_ACTIVATE_MEMO_REASON */
    "[auto memo] Su petición de vHost ha sido aprobada y ha sido activada. Razón: %s",
    /* LNG_REJECT_SYNTAX */
    "Sintaxis: \002{REJECT|R} \037nick\037 [\037razón\037]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Nota: Comodines para completado nicks están habilitados. Soportados caracteres comodines como '?' y '*'.\n"
#else
    "  Nota: Comodines para completado nicks están deshabilitados.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Nota: Expresiones regulares para completado de nicks habilitadas. El prefijo para completar series es '!'.",
#else
    "  Nota: Expresiones regulares para completado de nicks estan deshabilitadas.",
#endif
    /* LNG_REJALL_SYNTAX */
    "Sintaxis: \002REJALL [\037razón\037]\002",
    /* LNG_REJECTED */
    "El vHost de '%s' ha sido rechazado.",
    /* LNG_REJECT_MEMO */
    "[auto memo] El vHost que ha solicitado ha sido rechazado.",
    /* LNG_REJECT_MEMO_REASON */
    "[auto memo] El vHost que ha solicitado ha sido rechazado. Razón: %s",
    /* LNG_NO_REQUEST */
    "No se encontraron peticiones del nick '%s'.",
    /* LNG_HELP */
    "    REQUEST     Solicitar un vHost para tu nick",
    /* LNG_HELP_SETTER */
    "    ACTIVATE    Aprobar la petición de vHost para un usuario\n"
    "                El atajo \002A\002 esta disponible\n"
    "    ACTALL      Aprobar todas las peticiones de vHosts\n"
    "    REJECT      Rechazar la petición de vHost de un usuario\n"
    "                El atajo \002R\002 esta disponible\n"
    "    REJALL      Rechazar todas las peticiones de vHosts\n"
    "    WAITING     Comando conveniente para LIST +req\n"
    "                El atajo \002W\002 esta disponible\n"
    "    CONFIG      Acceso a varias configuraciones",
    /* LNG_HELP_REQUEST */
    "Las peticiones de vHost para su nick necesitan ser activadas por los\n"
    "administradores de la red. Por favor sea paciente con su peticiones\n"
    "serán consideradas.",
    /* LNG_HELP_ACTIVATE */
    "Activa el vHost pedido para el nick dado",
    /* LNG_HELP_ACTALL */
    "Activar todas las peticiones de vHosts.",
    /* LNG_HELP_ACTIVATE_MEMO */
    "Un memo informando al usuario tambien le será enviado.",
    /* LNG_HELP_REJECT */
    "Rechaza el vHost pedido para el nick dado.",
    /* LNG_HELP_REJECT_MEMO */
    "Un memo informando al usuario tambien le será enviado.",
    /* LNG_HELP_REJALL */
    "Rechazar todas las peticiones de vHosts.",
    /* LNG_HELP_REJALL_MEMO */
    "Un memo informando a los usuarios tambien les sera enviado.",
    /* LNG_WAITING_SYNTAX */
    "Sintaxis: \002{WAITING|W} [{\037nick\037|\037num\037}]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Nota: Comodines para completado nicks están habilitados. Soportados caracteres comodines como '?' y '*'.\n"
#else
    "  Nota: Comodines para completado nicks están deshabilitados.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Nota: Expresiones regulares para completado de nicks habilitadas. El prefijo para completar series es '!'.",
#else
    "  Nota: Expresiones regulares para completado de nicks estan deshabilitadas.",
#endif
    /* LNG_HELP_WAITING */
    "Este comando es usado por conveniencia. Es esencialmente\n"
    "el mismo rendimiento que LIST +req  .\n"
    " \n"
    "La salida puede ser limitada ya sea especificando un nick, en\n"
    "tal caso solo el pedido perteneciente a ese nick sera\n"
    "mostrado, o especificando un numero, en tal caso\n"
    "el numero de entradas mostradas no excederan este numero.\n"
    "Los numeros 0 y menores mostraran el numero total de\n"
    "vHost pedidos.",
    /* LNG_REQUEST_FORBIDDEN */
    "El vHost que ha pedido esta prohibido.",
    /* LNG_REQUEST_FORBIDDEN_REASON */
    "El vHost que ha pedido esta prohibido. Razón: %s",
    /* LNG_CONFIG_SYNTAX */
    "Sintaxis: \002CONFIG \037opción\037 [\037parametros\037]\002",
    /* LNG_HELP_CONFIG */
    "Permite accesar a varias configuraciones. Las \037opciones\037 pueden ser:\n"
    " \n"
    "    REGEX       Modificar las listas de Expresiones Regulares\n"
    "    SAVE_DB     Recolección manual de pedidos y bases de datos de expresiones regulares a archivos\n"
    "    LIST        Mostrar los valores de la configuración\n"
    "    SET         Ajuste de variables de tiempo de ejecución\n"
    " \n"
    "Detalles para: \002CONFIG REGEX\002\n"
    "Sintaxis: \002CONFIG REGEX {ADD \037razón\037 \037regex\037|DEL {\037#num\037|\037nick\037}|LIST}\002\n"
    " \n"
    "Permite la modificación de la lista de expresiones regulares.\n"
    " \n"
    "    LIST        Muestra una lista de expresiones regulares, incluido los numeros en expresionas regulares y nick creador para uso en \002DEL\002\n"
    "    ADD         Agrega una serie de expresiones regulares. Una razón \002tiene\002 que ser especificada, use 'none' para el predeterminado. SEA CUIDADOSO!\n"
    "                Para suplir una razón consistente en múltiples palabras sustituya los espacios con '_'\n"
    "    DEL         Borra una serie de expresiones regulares por referencia, ver \002LIST\002\n"
    " \n"
    "Detalles para: \002CONFIG SET\002\n"
    "Sintaxis: \002CONFIG SET \037opción\037 \037valor\037\002\n"
    " \n"
    "Permite ajustes temporales en las variables. Los cambios se perderan en los servicios\n"
    "cuando la configuración sea recargada o reiniciada. Las \037opciones\037 pueden ser:\n"
    " \n"
    "    MEMOUSER          Enviar memos a los usuarios en varias ocasiones.\n"
    "    MEMOOPER          Enviar memos a los operadores en varias ocasiones.\n"
    "    MEMOSETTERS       Enviar memos a los seteadores de vHost en varias ocasiones.\n"
    "    DISPLAYMODE       Activar/Desactivar el canal extendido de salida.\n"
    "    DISPLAYCOLOR      Activar/Desactivar los colores en el canal de salida.\n"
    "    DISPLAYMAX        Cambiar el número máximo para que nuevas peticiones sean mostradas.\n"
    "    TIMER             Cambiar el intervalo del tiempo de los anuncios.\n"
    "    REQUESTDELAY      Cambiar el retardo entre las peticiones permitidas para los vHost.\n"
    "                      \037Nota\037: Administradores de Servicios estan excentos del retardo.\n"
    "    DENYIDENTUPDATE   Deny ident@host requests.\n"
    " \n"
    "Todas las opciones ademas de \002TIMER\002, \002REQUESTDELAY\002 y \002DISPLAYMAX\002 son aceptadas\n"
    "valores: \0021\002 (ON) y \0020\002 (OFF).\n"
    "\002TIMER\002 acepta: \0020\002 (OFF) y \002>0\002 (INTERVALO). Intervalo minimo\n"
    "es de 10 segundos, valores entre 1-9 seran ajustados adecuadamente.\n" 
    "\002REQUESTDELAY\002 acepta: \0020\002 (OFF) y \002>0\002 (DELAY).\n"
    "\002DISPLAYMAX\002 acepta: \0020\002 (NO LIMIT) y \002>0\002 (LIMITED).",
    /* LNG_VHOST_SYNTAX */
    "Opciones Disponibles:\n"
    "- \002{WAITING|W} {\037nick\037|\037num\037}\002  - lista las peticiones de vHosts\n"
    "- \002{ACTIVATE|A} \037nick\037\002        - activar una petición\n"
    "- \002{REJECT|R} \037nick\037 [\037razón\037]\002 - rechazar una petición\n"
#ifdef SUPPORT_WILDCARD
    "  Nota: Comodines para completado nicks están habilitados. Soportados caracteres comodines como '?' y '*'.\n"
#else
    "  Nota: Comodines para completado nicks están deshabilitados.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Nota: Expresiones regulares para completado de nicks habilitadas. El prefijo para completar series es '!'.\n"
#else
    "  Nota: Expresiones regulares para completado de nicks estan deshabilitadas.\n"
#endif
    " \n"
    "- \002ACTALL [\037razón\037]\002             - activar todas las peticiones\n"
    "- \002REJALL [\037razón\037]\002             - rechazar todas las peticiones",
    /* LNG_NO_REQUESTS */
    "No se encontraron peticiones.",
    /* LNG_NOT_ENOUGH_REQUESTS */
    "No hay suficientes peticiones.",
    /* LNG_NO_MATCHES */
    "No se encontraron peticiones para '%s'.",
    /* LNG_REGEX_DISABLED */
    "Soporte de expresiones regulares de autocompletado de nicks esta deshabilitado.",
    /* LNG_WILDCARD_DISABLED */
    "Soporte de comodines para autocomentado de nick esta deshabilitado.",
    /* LNG_REGEX_ERROR */
    "Error en '%s': %s",
    /* LNG_CONF_SAVE_DB */
    "Hecho. Revise los servicios del canal para posibles errores.",
    /* LNG_CONF_REGEX_ADD_FAIL */
    "No se pudo agregar la expresión regular '%s'",
    /* LNG_CONF_REGEX_ADD */
    "Expresión regular '%s' agregada.",
    /* LNG_CONF_REGEX_LIST */
    "#%d: '%s' por %s",
    /* LNG_CONF_REGEX_LIST_END */
    "Fin de la lista de expresiones regulares.",
    /* LNG_CONF_REGEX_DELETED */
    "Totas las expresiones regulares de autocompletado han sido borradas.",
    /* LNG_CONF_TEMPORARY */
    "Ajuste %s temporal a: %d (%s)",
    /* LNG_VHOST_LISTING */
    "[\037%s\037] Listando peticiones de vHost abiertas...",
    /* LNG_VHOST_REQUEST_COUNT */
    "Cuenta de las peticiones: \002%d\002",
    /* LNG_VHOST_REQUEST_DISPLAYED */
    "- Mostrando %d petición%s(Cuenta: \002%d\002)",
    /* LNG_VHOST_REMINDER */
    "[\037Recordatorio\037] Tu tienes %d vHost%sesperando por aprobación. Para Verlos: !%s waiting",
    /* LNG_KEY_NICK */
    "Nick",
    /* LNG_KEY_OFFLINE */
    "desconectado",
    /* LNG_KEY_CURRENT */
    "Actual",
    /* LNG_KEY_NONE */
    "ninguno",
    /* LNG_KEY_REQUEST */
    "Petición",
    /* LNG_KEY_AGO */
    "antes",
    /* LNG_KEY_OPER */
    "Oper",
    /* LNG_KEY_HOST */
    "Host",
    /* LNG_KEY_Razón */
    "Razón",
    /* LNG_REQUEST_ALREADY_APPROVED */
    "Tu petición de '%s' ha sido aprovada. Por favor escribe '/msg HostServ on' si el vHost no ha sido activado.",
    /* LNG_REQUEST_REMOVED_HOST */
    "vHost pendiente de %s ha sido automaticamente removido. Nueva petición de vHost ha sido aprovado.",
    /* LNG_REQUEST_REMOVED_GROUP */
    "vHost pendientes solicitados %s han sido removidos automaticamente. Nueva petición encontrada ha sido aprobada para el host del nick agrupado %s.",
    /* LNG_REQUEST_NOT_SHOWN */
    "[\037Noticia\037] Nuevo(s) vHost(s) han sido pedidos pero no han sido aun mostrados.",
    /* LNG_REQUEST_UPDATE_DENIED */
    "Tu petición ha sido denegada. Para cambiar su ident:\n"
    "   1. Petición: '/msg HostServ request *@%s'.\n"
    "   2. Cambia tu ident en tu cliente y reconectate.",
    /* LNG_REQUEST_AUTO_APPROVED */
    "Tu petición ha sido aprobada automaticamente",
    /* LNG_HOST_REJECTED */
    "Tu petición del vHost para \002%s\002 ha sido rechazada. Razón: %s.",
    /* LNG_HOST_IDENT_REJECTED */
    "Tu petición del vHost para \002%s\002@\002%s\002 ha sido rechazada. Razón: %s.",
    /* LNG_HOST_ACTIVATED */
    "Tu vhost \002%s\002 ya está activada. Razón: %s.",
    /* LNG_HOST_IDENT_ACTIVATED */
    "Tu vhost \002%s\002@\002%s\002 ya está activada. Razón: %s.",
    /* LNG_REQUEST_AUTO_REJECTED */
    "Tu pedido ha sido rechazado automáticamente",
    /* LNG_CONF_EXCEPTION_ADD_FAIL */
    "No se puede agregar excepciones para el nick '%s'.",
    /* LNG_CONF_EXCEPTION_ADD */
    "Excepción para el nick '%s' agregada.",
    /* LNG_CONF_EXCEPTION_LIST */
    "#%d: '%s' by %s",
    /* LNG_CONF_EXCEPTION_LIST_END */
    "Fin de lista de excepciones.",
    /* LNG_CONF_EXCEPTION_DELETED */
    "Excepción concordante eliminada.",
    /* LNG_CONF_EXCEPTION_NOT_FOUND */
    "Entrada no encontrada en lista de excepciones.",
    /* LNG_REQUEST_REMOVED_GONE */
    "Pedido pendiente de vHost para %s removida automáticamente. El usuario ya no existe.",
    /* LNG_CONFIG_REGEX_SYNTAX */
    "Sintaxis: \002CONFIG REGEX {ADD \037razón\037 \037regex\037|DEL {\037#num\037|\037nick\037}|LIST}\002",
    /* LNG_CONFIG_EXCEPTION_SYNTAX */
    "Sintaxis: \002CONFIG EXCEPTION {ADD \037nick\037|DEL {\037#num\037|\037nick\037}|LIST}\002\n",
    /* LNG_USER_IS_EXEMPT */
    "Excepción para usuario por"
};

/* German translation */
char *langtable_de[] = {
    /* LNG_REQUEST_SYNTAX */
    "Syntax: \002REQUEST \037vHost\037\002",
    /* LNG_REQUESTED */
    "Dein vHost wurde beantragt. Bitte warte bis ein Operator über ihn entschieden hat.",
    /* LNG_REQUEST_WAIT */
    "Bitte warte %d Sekunden bevor du einen neuen vHost beantragst.",
    /* LNG_REQUEST_MEMO */
    "[auto memo] vHost \002%s\002 wurde beantragt.",
    /* LNG_ACTIVATE_SYNTAX */
    "Syntax: \002{ACTIVATE|A} \037Nick\037 [\037Grund\037]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Anmerkung: Wildcard Nick Matching ist aktiviert. Unterstützte Wildcard Charaktere sind '?' und '*'.\n"
#else
    "  Anmerkung: Wildcard Nick Matching ist deaktiviert.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Anmerkung: Regex Nick Matching ist aktiviert. Suchbegriffe müssen mit '!' beginnen.",
#else
    "  Anmerkung: Regex Nick Matching ist deaktiviert.",
#endif
    /* LNG_ACTALL_SYNTAX */
    "Syntax: \002ACTALL [\037Grund\037]\002",
    /* LNG_ACTIVATED */
    "vHost für '%s' wurde aktiviert.",
    /* LNG_ACTIVATE_MEMO */
    "[auto memo] Dein beantragter vHost wurde akzeptiert und gesetzt.",
    /* LNG_ACTIVATE_MEMO_REASON */
    "[auto memo] Dein beantragter vHost wurde akzeptiert und gesetzt. Grund: %s",
    /* LNG_REJECT_SYNTAX */
    "Syntax: \002{REJECT|R} \037Nick\037 [\037Grund\037]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Anmerkung: Wildcard Nick Matching ist aktiviert. Unterstützte Wildcard Charaktere sind '?' und '*'.\n"
#else
    "  Anmerkung: Wildcard Nick Matching ist deaktiviert.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Anmerkung: Regex Nick Matching ist aktiviert. Suchbegriffe müssen mit '!' beginnen.",
#else
    "  Anmerkung: Regex Nick Matching ist deaktiviert.",
#endif
    /* LNG_REJALL_SYNTAX */
    "Syntax: \002REJALL [\037Grund\037]\002",
    /* LNG_REJECTED */
    "vHost für '%s' wurde abgelehnt.",
    /* LNG_REJECT_MEMO */
    "[auto memo] Dein beantragter vHost wurde abgelehnt.",
    /* LNG_REJECT_MEMO_REASON */
    "[auto memo] Dein beantragter vHost wurde abgelehnt. Grund: %s",
    /* LNG_NO_REQUEST */
    "Es wurde kein vHost-Antrag für '%s' gefunden.",
    /* LNG_HELP */
    "    REQUEST     Beantrage einen vHost für deinen Nick",
    /* LNG_HELP_SETTER */
    "    ACTIVATE    Akzeptier den beantragten vHost eines Users\n"
    "                Das Kürzel \002A\002 ist verfügbar\n"
    "    ACTALL      Akzeptier alle beantragten vHosts\n"
    "    REJECT      Lehn den beantragten vHost eines Users ab\n"
    "                Das Kürzel \002R\002 ist verfügbar\n"
    "    REJALL      Lehn alle beantragten vHosts ab\n"
    "    WAITING     Befehl mit der selben Funktion wie LIST +req\n"
    "                Das Kürzel \002W\002 ist verfügbar\n"
    "    CONFIG      Veränder verschiedene Einstellungen",
    /* LNG_HELP_REQUEST */
    "Beantrage einen vHost für deinen Nick. Über dein Antrag wird\n"
    "daraufhin von Netzwerk Administratoren entschieden, was je nachdem\n"
    "einige Zeit dauern kann.",
    /* LNG_HELP_ACTIVATE */
    "Akzeptier den beantragten vHost des Users.",
    /* LNG_HELP_ACTALL */
    "Akzeptier alle beantragten vHosts.",
    /* LNG_HELP_ACTIVATE_MEMO */
    "Dem User wird, falls offline, eine Memo geschickt.",
    /* LNG_HELP_REJECT */
    "Lehn den beantragten vHost des Users ab.",
    /* LNG_HELP_REJECT_MEMO */
    "Dem User wird, falls offline, eine Memo geschickt.",
    /* LNG_HELP_REJALL */
    "Lehn alle beantragten vHosts ab.",
    /* LNG_HELP_REJALL_MEMO */
    "Dem User wird, falls offline, eine Memo geschickt.",
    /* LNG_WAITING_SYNTAX */
    "Syntax: \002{WAITING|W} [{\037Nick\037|\037Nummer\037}]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Anmerkung: Wildcard Nick Matching ist aktiviert. Unterstützte Wildcard Charaktere sind '?' und '*'.\n"
#else
    "  Anmerkung: Wildcard Nick Matching ist deaktiviert.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Anmerkung: Regex Nick Matching ist aktiviert. Suchbegriffe müssen mit '!' beginnen.",
#else
    "  Anmerkung: Regex Nick Matching ist deaktiviert.",
#endif
    /* LNG_HELP_WAITING */
    "Dieser Befehl wird aus reiner Bequemlichkeit angeboten. Er entspricht\n"
    "der Ausführung von  LIST +req  .\n"
    " \n"
    "Ergebnisse können eingeschränkt werden entweder durch Angabe\n"
    "eines Nicks, in welchem Fall nur dessen spezieller Antrag\n"
    "angezeigt wird, oder durch Angabe einer Nummer, in welchem\n"
    "Fall die Anzahl angezeigter Anträge diese Nummer nicht überschreiten\n"
    "wird. Nummern kleiner oder gleich 0 geben Auskunft über die Anzahl\n"
    "aller derzeit offenen vHost-Anträge.",
    /* LNG_REQUEST_FORBIDDEN */
    "Dein beantragter vHost ist nicht gestattet.",
    /* LNG_REQUEST_FORBIDDEN_REASON */
    "Dein beantragter vHost ist nicht gestattet. Grund: %s",
    /* LNG_CONFIG_SYNTAX */
    "Syntax: \002CONFIG \037Option\037 [\037Parameter\037]\002",
    /* LNG_HELP_CONFIG */
    "Erlaubt Zugriff auf verschiedene Optionen. \037Option\037 kann sein:\n"
    " \n"
    "    EXCEPTION   Veränder die Ausnahme-Liste\n"
    "    REGEX       Veränder die Regex-Liste\n"
    "    SAVE_DB     Führe eine Sicherung der Antrag- und Regex-Datenbanken manuell aus\n"
    "    LIST        Zeige Konfigurations-Variablen an\n"
    "    SET         Justiere verschiedene Variablen in Echtzeit\n"
    " \n"
    "Details für: \002CONFIG EXCEPTION\002\n"
    "Syntax: \002CONFIG EXCEPTION {ADD \037Nick\037|DEL {\037#Nummer\037|\037Nick\037}|LIST}\002\n"
    " \n"
    "Erlaubt Veränderung der Ausnahme-Liste.\n"
    " \n"
    "    LIST        Zeige Ausnahme-Liste, beinhaltet Ausnahme Nummer und hinzugefügt-von Nick für \002DEL\002 Befehl\n"
    "    ADD         Füge eine Ausnahme hinzu. Dies wirkt sich auch auf alle Gruppenmitglieder aus.\n"
    "    DEL         Lösche eine Ausnahme per Referenz, siehe \002LIST\002\n"
    " \n"
    "Details für: \002CONFIG REGEX\002\n"
    "Syntax: \002CONFIG REGEX {ADD \037Grund\037 \037Regex\037|DEL {\037#Nummer\037|\037Nick\037}|LIST}\002\n"
    " \n"
    "Erlaubt Veränderung der Regex-Liste.\n"
    " \n"
    "    LIST        Zeite Regex-Liste, beinhaltet Regex Nummer und Ersteller-Nick für \002DEL\002 Befehl\n"
    "    ADD         Füge eine Regex Zeichenfolge hinzu. Ein Grund \002muss\002 angegeben werde, benutze 'none' für einen Standardgrund. SEI VORSICHTIG!\n"
    "                Um einen längeren Grund anzugeben ersetze Leerzeichen mit '_'\n"
    "    DEL         Lösche eine Regex Zeichenfolge per Referenz, siehe \002LIST\002\n"
    " \n"
    "Details für: \002CONFIG SET\002\n"
    "Syntax: \002CONFIG SET \037Option\037 \037Wert\037\002\n"
    " \n"
    "Erlaubt vorübergehende Justierung verschiedener Variablen. Veränderungen gehen verloren\n"
    "während erneutem Laden der Konfigurationsdatei und Neustarts. \037Option\037 kann sein:\n"
    " \n"
    "    MEMOUSER          Sende Memos an Benutzer zu verschiedenen Anlässen.\n"
    "    MEMOOPER          Sende Memos an Oper zu verschiedenen Anlässen.\n"
    "    MEMOSETTERS       Sende Memos an Host Setter zu verschiedenen Anlässen.\n"
    "    DISPLAYMODE       Aktiviere/Deaktiviere erweiterte Channel Anzeige.\n"
    "    DISPLAYCOLOR      Aktiviere/Deaktiviere farbige Channel Anzeige.\n"
    "    DISPLAYMAX        Verändere die maximale Anzahl an neuen Anträgen die angezeigt wird.\n"
    "    TIMER             Verändere den Intervall zwischen Erinnerungen.\n"
    "    REQUESTDELAY      Verändere, wielange ein Benutzer zwischen 2 vHost Anträgen warten muss.\n"
    "                      \037Anmerkung\037: Services Admins sind hiervon ausgenommen.\n"
    "    DENYIDENTUPDATE   Verbiete ident@host Anträge.\n"
    " \n"
    "Alle Optionen ausser \002TIMER\002, \002REQUESTDELAY\002 und \002DISPLAYMAX\002 akzeptieren\n"
    "folgende Werte: \0021\002 (ON) und \0020\002 (OFF).\n"
    "\002TIMER\002 akzeptiert: \0020\002 (OFF) und \002>0\002 (INTERVAL). Minimaler Interval\n"
    "ist 10 Sekunden, Werte zwischen 1-9 werden entsprechend justiert.\n" 
    "\002REQUESTDELAY\002 akzeptiert: \0020\002 (OFF) und \002>0\002 (DELAY).\n"
    "\002DISPLAYMAX\002 akzeptiert: \0020\002 (NO LIMIT) und \002>0\002 (LIMITED).",
    /* LNG_VHOST_SYNTAX */
    "Verfügbare Optionen:\n"
    "- \002{WAITING|W} {\037Nick\037|\037Nummer\037}\002 - Zeige alle offenen vHost-Anträge an\n"
    "- \002{ACTIVATE|A} \037Nick\037\002          - Akzeptier einen Antrag\n"
    "- \002{REJECT|R} \037Nick\037 [\037Grund\037]\002   - Lehn einen Antrag ab\n"
#ifdef SUPPORT_WILDCARD
    "  Anmerkung: Wildcard Nick Matching ist aktiviert. Unterstützte Wildcard Charaktere sind '?' und '*'.\n"
#else
    "  Anmerkung: Wildcard Nick Matching ist deaktiviert.\n"
#endif
#ifdef SUPPORT_REGEX
    "  Anmerkung: Regex Nick Matching ist aktiviert. Suchbegriffe müssen mit '!' beginnen.\n"
#else
    "  Anmerkung: Regex Nick Matching ist deaktiviert.\n"
#endif
    " \n"
    "- \002ACTALL [\037Grund\037]\002            - Akzeptier alle beantragten vHosts\n"
    "- \002REJALL [\037Grund\037]\002            - Lehn alle beantragten vHosts ab",
    /* LNG_NO_REQUESTS */
    "Keine Anträge vorhanden.",
    /* LNG_NOT_ENOUGH_REQUESTS */
    "Nicht genügend Anträge vorhanden.",
    /* LNG_NO_MATCHES */
    "Es wurden keine Anträge für den Such-String '%s' gefunden.",
    /* LNG_REGEX_DISABLED */
    "Unterstützung für Regex Nick Matching ist deaktiviert.",
    /* LNG_WILDCARD_DISABLED */
    "Unterstützung für Wildcard Nick Matching ist deaktiviert.",
    /* LNG_REGEX_ERROR */
    "Fehler in '%s': %s",
    /* LNG_CONF_SAVE_DB */
    "Fertig. Mögliche Fehler wurden im Services-Channel bekanntgegeben.",
    /* LNG_CONF_REGEX_ADD_FAIL */
    "Konnte Regex '%s' nicht hinzufügen",
    /* LNG_CONF_REGEX_ADD */
    "Regex '%s' hinzugefügt.",
    /* LNG_CONF_REGEX_LIST */
    "#%d: '%s' von %s",
    /* LNG_CONF_REGEX_LIST_END */
    "Ende der Regex-Liste.",
    /* LNG_CONF_REGEX_DELETED */
    "Alle gefundenen Regexes wurden gelöscht.",
    /* LNG_CONF_TEMPORARY */
    "Setze %s vorübergehend zu: %d (%s)",
    /* LNG_VHOST_LISTING */
    "[\037%s\037] Zeige offene vHost Anträge...",
    /* LNG_VHOST_REQUEST_COUNT */
    "Offene Anträge: \002%d\002",
    /* LNG_VHOST_REQUEST_DISPLAYED */
    "- Angezeigte Anträge: %d%s(Insgesamt: \002%d\002)", // horrible %s string, this should be changed
    /* LNG_VHOST_REMINDER */
    "[\037Erinnerung\037] Es warten %d vHost%sauf Begutachtung. Um sie anzuzeigen: !%s waiting",
    /* LNG_KEY_NICK */
    "Nick",
    /* LNG_KEY_OFFLINE */
    "offline",
    /* LNG_KEY_CURRENT */
    "Aktuell",
    /* LNG_KEY_NONE */
    "keiner",
    /* LNG_KEY_REQUEST */
    "Antrag",
    /* LNG_KEY_AGO */
    "zuvor",
    /* LNG_KEY_OPER */
    "Oper",
    /* LNG_KEY_HOST */
    "Host",
    /* LNG_KEY_REASON */
    "Grund",
    /* LNG_REQUEST_ALREADY_APPROVED */
    "Dein Antrag '%s' wurde bereits akzeptiert.\n"
    "Falls der Host nicht aktiv sein sollte, führe '/msg HostServ on' aus.",
    /* LNG_REQUEST_REMOVED_HOST */
    "Offener vHost Antrag für %s wurde automatisch gelöscht. Neuer Antrag entspricht derzeitigem vHost.",
    /* LNG_REQUEST_REMOVED_GROUP */
    "Offener vHost Antrag für %s wurde automatisch gelöscht. Neuer Antrag entspricht derzeitigem vHost des gruppierten Nicks %s.",
    /* LNG_REQUEST_NOT_SHOWN */
    "[\037Achtung\037] Ein oder mehrere vHosts wurden beantragt, welche nicht angezeigt werden konnten.",
    /* LNG_REQUEST_UPDATE_DENIED */
    "Dein Antrag wurde abgelehnt. Um deine Ident zu ändern folge den Anweisungen:\n"
    "   1. Beantrage: '/msg HostServ request *@%s'.\n"
    "   2. Ändere die Ident in deinem Klienten und verbinde dich neu.",
    /* LNG_REQUEST_AUTO_APPROVED */
    "Dein Antrag wurde automatisch aktiviert",
    /* LNG_HOST_REJECTED */
    "Dein beantragter vHost \002%s\002 wurde abgelehnt. Grund: %s.",
    /* LNG_HOST_IDENT_REJECTED */
    "Dein beantragter vHost \002%s\002@\002%s\002 wurde abgelehnt. Grund: %s.",
    /* LNG_HOST_ACTIVATED */
    "Dein beantragter vHost \002%s\002 wurde akzeptiert. Grund: %s.",
    /* LNG_HOST_IDENT_ACTIVATED */
    "Dein beantragter vHost \002%s\002@\002%s\002 wurde akzeptiert. Grund: %s.",
    /* LNG_REQUEST_AUTO_REJECTED */
    "Dein Antrag wurde automatisch abgelehnt",
    /* LNG_CONF_EXCEPTION_ADD_FAIL */
    "Konnte keine Ausnahme für '%s' hinzufügen.",
    /* LNG_CONF_EXCEPTION_ADD */
    "Ausnahme für '%s' hinzugefügt.",
    /* LNG_CONF_EXCEPTION_LIST */
    "#%d: '%s' von %s",
    /* LNG_CONF_EXCEPTION_LIST_END */
    "Ende der Ausnahme-Liste.",
    /* LNG_CONF_EXCEPTION_DELETED */
    "Gefundener Eintrag wurde gelöscht.",
    /* LNG_CONF_EXCEPTION_NOT_FOUND */
    "Kein Eintrag in der Ausnahme-Liste gefunden.",
    /* LNG_REQUEST_REMOVED_GONE */
    "Offener vHost Antrag für %s wurde automatisch gelöscht. Benutzer existiert nicht mehr.",
    /* LNG_CONFIG_REGEX_SYNTAX */
    "Syntax: \002CONFIG REGEX {ADD \037Grund\037 \037Regex\037|DEL {\037#Nummer\037|\037Nick\037}|LIST}\002",
    /* LNG_CONFIG_EXCEPTION_SYNTAX */
    "Syntax: \002CONFIG EXCEPTION {ADD \037Nick\037|DEL {\037#Nummer\037|\037Nick\037}|LIST}\002",
    /* LNG_USER_IS_EXEMPT */
    "Ausnahme-Regel für den Benutzer hinzugefügt von"
};

/* Hungarian translation */
char *langtable_hun[] = {
    /* LNG_REQUEST_SYNTAX */
    "Szintaktika: \002REQUEST \037vHost\037\002",
    /* LNG_REQUESTED */
    "A vHost-od igénylésre került. Kérjük várj, amíg egy operátor aktiválja.",
    /* LNG_REQUEST_WAIT */
    "Kérjük várj %d másodpercet, mielott új vHost-ot kérsz.",
    /* LNG_REQUEST_MEMO */
    "[auto memo] A(z) \002%s\002 vHost igénylésre került.",
    /* LNG_ACTIVATE_SYNTAX */
    "Szintaktika: \002{ACTIVATE|A} \037nick\037 [\037indok\037]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Megj.: A wildcard nick szurés aktív. A támogatott joker karakterek: '?' és '*'.\n"
#else
    "  Megj.: A wildcard nick szurés inaktív. \n"
#endif
#ifdef SUPPORT_REGEX
    "  Megj.: A regex nick szurés aktív. Kezdd a stringet a következovel: '!'.",
#else
    "  Megj.: A regex nick szurés inaktív.",
#endif
    /* LNG_ACTALL_SYNTAX */
    "Szintaktika: \002ACTALL [\037indok\037]\002",
    /* LNG_ACTIVATED */
    "A vHost '%s'-hez aktiválásra került.",
    /* LNG_ACTIVATE_MEMO */
    "[auto memo] A kért vHost-od jóváhagyásra és beállításra került.",
    /* LNG_ACTIVATE_MEMO_REASON */
    "[auto memo] Az kért vHost-od jóváhagyásra és beállításra került. Indok: %s",
    /* LNG_REJECT_SYNTAX */
    "Szintaktika: \002{REJECT|R} \037nick\037 [\037indok\037]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Megj,: A wildcard nick szurés aktív. A támogatott joker karakterek: '?' és '*'.\n"
#else
    "  Megj.: A wildcard nick szurés inaktív. \n"
#endif
#ifdef SUPPORT_REGEX
    "  Megj.: A regex nick szurés aktív. Kezdd a stringet a következovel: '!'.",
#else
    "  Megj.: A regex nick szurés inaktív.",
#endif
    /* LNG_REJALL_SYNTAX */
    "Szintaktika: \002REJALL [\037indok\037]\002",
    /* LNG_REJECTED */
    "A vHost '%s'-hez elutasításra került.",
    /* LNG_REJECT_MEMO */
    "[auto memo] A kért vHost-od elutasításra került.",
    /* LNG_REJECT_MEMO_REASON */
    "[auto memo] A kért vHost-od elutasításra került. Indok: %s",
    /* LNG_NO_REQUEST */
    "Nem találtunk igénylést a(z) '%s' nick-hez.",
    /* LNG_HELP */
    "    REQUEST     vHost igénylése a nick-edhez",
    /* LNG_HELP_SETTER */
    "    ACTIVATE    Egy user igényelt vHost-jának jóváhagyása\n"
    "                Használható az \002A\002 rövidítés\n"
    "    ACTALL      Az összes igényelt vHost jóváhagyása\n"
    "    REJECT      Egy user igényelt vHost-jának elutasítása\n"
    "                Használható az \002R\002 rövidítés\n"
    "    REJALL      Az összes igényelt vHost elutasítása\n"
    "    WAITING     Kényelmi parancs a következohöz: LIST +req\n"
    "                Használható a \002W\002 rövidítés\n"
    "    CONFIG      Különbözo beállításokhoz való hozzáférés",
    /* LNG_HELP_REQUEST */
    "Kérd a nick-edhez megadott vHost aktiválását\n"
    "a hálózati adminisztrátoroknál. Kérjük légy türelemmel amíg\n"
    "a kérésed elbírálásra kerül.",
    /* LNG_HELP_ACTIVATE */
    "A nick-hez megadott vHost aktiválása.",
    /* LNG_HELP_ACTALL */
    "Az összes igényelt vHost aktiválása.",
    /* LNG_HELP_ACTIVATE_MEMO */
    "A user-eket tájékoztató memo is küldésre kerül.",
    /* LNG_HELP_REJECT */
    "A nick-hez megadott vHost elutasítása.",
    /* LNG_HELP_REJECT_MEMO */
    "A user-t tájékoztató memo is küldésre kerül.",
    /* LNG_HELP_REJALL */
    "Az összes igényelt vHost elutasítása.",
    /* LNG_HELP_REJALL_MEMO */
    "A user-eket tájékoztató memo is küldésre kerül.",
    /* LNG_WAITING_SYNTAX */
    "Szintaktika: \002{WAITING|W} [{\037nick\037|\037szám\037}]\002\n"
#ifdef SUPPORT_WILDCARD
    "  Megj,: A wildcard nick szurés aktív. A támogatott joker karakterek: '?' és '*'.\n"
#else
    "  Megj.: A wildcard nick szurés inaktív. \n"
#endif
#ifdef SUPPORT_REGEX
    "  Megj.: A regex nick szurés aktív. Kezdd a stringet a következovel: '!'.",
#else
    "  Megj.: A regex nick szurés inaktív.",
#endif
    /* LNG_HELP_WAITING */
    "Ez egy kényelmi parancs. Alapvetoen ugyanaz,\n"
    "mint egy  LIST +req  .\n"
    " \n"
    "A kimenet korlátozható vagy egy nick megadásával,\n"
    "mely esetben csak a nick-hez tartozó igénylések lesznek\n"
    "láthatóak, vagy egy szám megadásával, mely esetben\n"
    "a bejegyzések száma nem lesz több ennél a számnál.\n"
    "A 0 és a kisebb számok az összes nyitott vHost igénylés\n"
    "listázását eredményezik.",
    /* LNG_REQUEST_FORBIDDEN */
    "Az általad kért vHost le van tiltva.",
    /* LNG_REQUEST_FORBIDDEN_REASON */
    "Az általád kért vHost le van tiltva. Indok: %s",
    /* LNG_CONFIG_SYNTAX */
    "Szintaktika: \002CONFIG \037opció\037 [\037paraméterek\037]\002",
    /* LNG_HELP_CONFIG */
    "Különbözo beállításokat tesz lehetové. Az \037opció\037 lehet:\n"
    " \n"
    "    KIVÉTEL     Kivltel-lista módosítása\n"
    "    REGEX       Regex-ek kistájának módosítása\n"
    "    SAVE_DB     Igénylések és regex adatbázisok manuális mentése file-okba\n"
    "    LIST        Konfig értékek mutatása\n"
    "    SET         Runtime változók állítása\n"
    " \n"
    "\002CONFIG EXCEPTION\002 részletezése\n"
    "Szintaktika: \002CONFIG EXCEPTION {ADD \037nick\037|DEL {\037#szám\037|\037nick\037}|LIST}\002\n"
    " \n"
    "A kivétel-lista módosításához:\n"
    " \n"
    "    LIST        Kivétel-lista megtekintése, benne a kivétel számával és a beállító nick-kel a \002DEL\002-ben való használathoz\n"
    "    ADD         Kivétel hozzáadása. Minden csoporttagra is érvényes lesz.\n"
    "    DEL         Kivétel törlése hivatkozás alapján, lásd \002LIST\002\n"
    " \n"
    "\002CONFIG REGEX\002 részletezése\n"
    "Szintaktika: \002CONFIG REGEX {ADD \037indok\037 \037regex\037|DEL {\037#szám\037|\037nick\037}|LIST}\002\n"
    " \n"
    "Lehetové teszi a regex lista módosítását.\n"
    " \n"
    "    LIST        Regex lista, tartalmazza a regex számot, a készíto nick-jét \002DEL\002-hez\n"
    "    ADD         Regex string hozzáadása. Egy okot \002meg kell\002 adni, használj 'none'-t alapból. LÉGY ÓVATOS!\n"
    "                Több szót tartalmazó indoklásnál használj '_' jelet space helyett\n"
    "    DEL         Regex string törlése referencia alapján, lásd \002LIST\002\n"
    " \n"
    "\002CONFIG SET\002\n részletezése"
    "Szintaktika: \002CONFIG SET \037opció\037 \037érték\037\002\n"
    " \n"
    "Lehetové teszi a runtime változók ideiglenes módosítását. A módosítások a services\n"
    "konfiguráció újratöltéseknél és újraindításoknál elvesznek. Az \037opció\037 lehet:\n"
    " \n"
    "    MEMOUSER          Memo-t küld a usernek különbözo alkalmakkor.\n"
    "    MEMOOPER          Memo-t küld az opereknek különbözo alkalmakkor.\n"
    "    MEMOSETTERS       Memo-t küld a host setter-eknek különbözo alkalmakkor.\n"
    "    DISPLAYMODE       Engedélyezi/Letiltja a kibovített csatorna kimenetetet.\n"
    "    DISPLAYCOLOR      Enegedélyezi/Letiltja a színes csatorna kimenetet.\n"
    "    DISPLAYMAX        Megváltoztatja a megjelenítendo új igénylések max számát.\n"
    "    TIMER             Megváltoztatja a bejelentések közti idotartamot.\n"
    "    REQUESTDELAY      Megváltoztatja az engedélyezett vHost igények közti késleltetést.\n"
    "                      \037Megj.\037: Services Admin-ok kivételt képeznek a késleltetés alól.\n"
    "    DENYIDENTUPDATE   Elutasítja az ident@host kéréseket.\n"
    " \n"
    "Minden opció a \002TIMER\002, a \002REQUESTDELAY\002 és a \002DISPLAYMAX\002 mellett elfogadja\n"
    "az \0021\002 (ON) and \0020\002 (OFF) értékeket.\n"
    "A \002TIMER\002 elfogadja: \0020\002 (OFF) and \002>0\002 (INTERVAL). Minimum köztes ido\n"
    "10 másodperc, az 1-9 közötti értékek korrigálásra kerülnek.\n"
    "\002REQUESTDELAY\002 elfogadja: \0020\002 (OFF) and \002>0\002 (DELAY).\n"
    "\002DISPLAYMAX\002 elfogadja: \0020\002 (NO LIMIT) and \002>0\002 (LIMITED).",
    /* LNG_VHOST_SYNTAX */
    "Elérheto opciók:\n"
    "- \002{WAITING|W} {\037nick\037|\037szám\037}\002   - kilistázza a nyitott vHost kérelmeket\n"
    "- \002{ACTIVATE|A} \037nick\037\002         - aktivál egy kérelmet\n"
    "- \002{REJECT|R} \037nick\037 [\037indok\037]\002   - elutasít egy kérelmet\n"
#ifdef SUPPORT_WILDCARD
    "  Megj.: A wildcard nick szurés aktív. A támogatott joker karakterek: '?' és '*'.\n"
#else
    "  Megj.: A wildcard nick szurés inaktív. \n"
#endif
#ifdef SUPPORT_REGEX
    "  Megj.: A regex nick szurés aktív. Kezdd a stringet a következovel: '!'.\n"
#else
    "  Megj.: A regex nick szurés inaktív.\n"
#endif
    " \n"
    "- \002ACTALL [\037indok\037]\002            - az összes kérelem aktiválása\n"
    "- \002REJALL [\037indok\037]\002            - az összes kérelem elutaítása",
    /* LNG_NO_REQUESTS */
    "Nincs kérelem.",
    /* LNG_NOT_ENOUGH_REQUESTS */
    "Nincs elég kérelem.",
    /* LNG_NO_MATCHES */
    "Nem található '%s'-nek megfelelo kérelem.",
    /* LNG_REGEX_DISABLED */
    "Regex nick szurés inaktív.",
    /* LNG_WILDCARD_DISABLED */
    "Wildcard nick szurés inkaktív.",
    /* LNG_REGEX_ERROR */
    "Hiba '%s'-ben: %s",
    /* LNG_CONF_SAVE_DB */
    "Kész. Ellenorizd a services csatornát a lehetséges hibákért.",
    /* LNG_CONF_REGEX_ADD_FAIL */
    "'%s' regex-et nem sikerült hozzáadni",
    /* LNG_CONF_REGEX_ADD */
    "'%s' regex hozzáadásra került.",
    /* LNG_CONF_REGEX_LIST */
    "#%d: '%s' by %s",
    /* LNG_CONF_REGEX_LIST_END */
    "Regex lista vége.",
    /* LNG_CONF_REGEX_DELETED */
    "Az összes megfelelo regex törölve.",
    /* LNG_CONF_TEMPORARY */
    "%s beállítás átmenetileg a következo lett: %d (%s)",
    /* LNG_VHOST_LISTING */
    "[\037%s\037] Nyitott vHost kérelmek listázása...",
    /* LNG_VHOST_REQUEST_COUNT */
    "Kérelmek száma: \002%d\002",
    /* LNG_VHOST_REQUEST_DISPLAYED */
    "- %d kiírt kérelem%s(Összes: \002%d\002)",
    /* LNG_VHOST_REMINDER */
    "[\037Emlékezteto\037] %d vHost%sjóváhagyásra vár. Megtekintés: !%s waiting",
    /* LNG_KEY_NICK */
    "Nick",
    /* LNG_KEY_OFFLINE */
    "offline",
    /* LNG_KEY_CURRENT */
    "Jelenlegi",
    /* LNG_KEY_NONE */
    "nincs",
    /* LNG_KEY_REQUEST */
    "Kérelem",
    /* LNG_KEY_AGO */
    "ezelott",
    /* LNG_KEY_OPER */
    "Oper",
    /* LNG_KEY_HOST */
    "Host",
    /* LNG_KEY_REASON */
    "Indok",
    /* LNG_REQUEST_ALREADY_APPROVED */
    "A kérésed '%s'-re már jóváhagyásra került. Írj be '/msg HostServ on' ha a host nem aktív.",
    /* LNG_REQUEST_REMOVED_HOST */
    "A függoben lévo vHost kérelem %s-re automatikusan eltávolításra került. Az új kérelem egy már jóváhagyott host-ot érint.",
    /* LNG_REQUEST_REMOVED_GROUP */
    "A függoben lévo vHost kérelem %s-re automatikusan eltávolításra került. Az új kérelem a(z) %s csoportosított nick-hez tartozó jóváhagyott host-ot érint.",
    /* LNG_REQUEST_NOT_SHOWN */
    "[\037Figyelmeztetés\037] Új vHost(okat) igényeltek, amik nem kerültek megjelenítésre.",
    /* LNG_REQUEST_UPDATE_DENIED */
    "Kérelem megtagadva. Az ident megváltoztatásához:\n"
    "   1. Kérelem: '/msg HostServ request *@%s'.\n"
    "   2. Változtass ident-et a kliensedben és kapcsolódj újra.",
    /* LNG_REQUEST_AUTO_APPROVED */
    "A kérelmed automatikusan elfogadásra került",
    /* LNG_HOST_REJECTED */
    "Az kért \002%s\002 vHost-od elutasításra került. Indok: %s.",
    /* LNG_HOST_IDENT_REJECTED */
    "Az kért \002%s\002@\002%s\002 vHost elutasításra került. Indok: %s.",
    /* LNG_HOST_ACTIVATED */
    "A(z) \002%s\002 vhost-od aktiválásra került. Indok: %s.",
    /* LNG_HOST_IDENT_ACTIVATED */
    "A(z) \002%s\002@\002%s\002 vhost-od aktiválásra került. Indok: %s.",
    /* LNG_REQUEST_AUTO_REJECTED */
    "A kérésed automatikusan elutasításra került",
    /* LNG_CONF_EXCEPTION_ADD_FAIL */
    "Nem lehetett kivételt hozzáadni a(z) '%s' nickhez.",
    /* LNG_CONF_EXCEPTION_ADD */
    "'%s' nickhez a kivétel hozzáadva.",
    /* LNG_CONF_EXCEPTION_LIST */
    "#%d: '%s', hozzáadó: %s",
    /* LNG_CONF_EXCEPTION_LIST_END */
    "Kivétel-lista vége.",
    /* LNG_CONF_EXCEPTION_DELETED */
    "Megfelelo kivételek törölve.",
    /* LNG_CONF_EXCEPTION_NOT_FOUND */
    "Nincs ilyen bejegyzés a kivétel-listában.",
    /* LNG_REQUEST_REMOVED_GONE */
    "A függoben lévo vHost kérés a(z) %s nickhez automatikusan eltávolításra került. A felhasználó már nem létezik.",
    /* LNG_CONFIG_REGEX_SYNTAX */
    "Szintaktika: \002CONFIG REGEX {ADD \037indok\037 \037regex\037|DEL {\037#szám\037|\037nick\037}|LIST}\002",
    /* LNG_CONFIG_EXCEPTION_SYNTAX */
    "Szintaktika: \002CONFIG EXCEPTION {ADD \037nick\037|DEL {\037#szám\037|\037nick\037}|LIST}\002",
    /* LNG_USER_IS_EXEMPT */
    "A felhasználó kivételét beállította: "
};
    moduleInsertLanguage(LANG_EN_US, LNG_NUM_STRINGS, langtable_en_us);
    moduleInsertLanguage(LANG_ES, LNG_NUM_STRINGS, langtable_es);
    moduleInsertLanguage(LANG_DE, LNG_NUM_STRINGS, langtable_de);
    moduleInsertLanguage(LANG_HUN, LNG_NUM_STRINGS, langtable_hun);
}

/* EOF */
