#ifndef PROT_HEADER_INCLUDED
#define PROT_HEADER_INCLUDED

#define SOH (0x01)
#define STX (0x02)

#define ST_LVL_LOW    (0x01)
#define ST_LVL_HIGH   (0x02)
#define ST_CMD_ON     (0x04)
#define ST_ENABLED    (0x08)

#define FCT_PUMP        '1'
#define FCT_OYA         '2'
#define FCT_ADDR        '@'
#define FCT_ENABLE      'e'
#define FCT_READ_TIME   's'
#define FCT_RAZ_TIME    'S'
#define FCT_READ_ERR    'r'
#define FCT_RAZ_ERR     'R'
#define FCT_PING        'p'     ///< Ping: Le noeud répond
#define FCT_EMERGENCY   'A'     ///< Arret d'urgence: Tous les noeuds s'arretent et se met en securite (actionneur coupé)

#endif
