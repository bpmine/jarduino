#ifndef PROT_HEADER_INCLUDED
#define PROT_HEADER_INCLUDED

#define SOH (0x01)
#define STX (0x02)

#define STATUS_CMD     		(0x01)
#define STATUS_ON     		(0x02)
#define STATUS_LVL_LOW    	(0x04)
#define STATUS_LVL_HIGH   	(0x08)

#define MSG_CMD        	'm'		///< Commande [ST1][ST2][@]
#define MSG_PUMP        'p'		///< Infos etat pompe [@][ST][TEMP][HUM][TIME_LSB][TIME_MSB][ERRS_LSB][ERRS_MSB][FLOW_LSB][FLOW_MSB]
#define MSG_OYA         'o'		///< Infos etat oya [@][ST][TEMP][HUM][TIME_LSB][TIME_MSB][ERRS_LSB][ERRS_MSB]
#define MSG_PING        'i'		///< Demande de ping [@][Id]
#define MSG_PONG        'y'		///< Reponse de ping (pong) [@][~Id]
#define MSG_RAZ_TIME    't'		///< RAZ du temps total [@][Id]
#define MSG_RAZ_ERR	    'z'		///< RAZ du nombre d'erreurs [@][Id]

#define ADDR_SYNC		'S'		///< Synchro fin de cycle (pour laisser du temps aux esclave de faire leur traitement)

#endif
