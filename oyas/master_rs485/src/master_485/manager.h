/**
 * @file manager.h
 * @brief EN TETE - Programme de gestion des OYAs
 **/
#ifndef MANAGER_HEADER_INCLUDED
#define MANAGER_HEADER_INCLUDED

/**
 * @brief Modes d'affichage
 * @{
 **/
#define MODE_AFF_IDLE       (0)     ///< Maitre allume mais RS485 et Wifi Off
#define MODE_AFF_CHECK_WIFI (1)     ///< Wifi actif (Wifi On et RS485 off)
#define MODE_AFF_DISPLAY    (2)     ///< Affichage des etats des OYAs (RS485 on)
#define MODE_AFF_REMOTE     (3)     ///< Contrôle à distance en cours (Affiche l'etat des OYAs)
#define MODE_AFF_AUTO       (4)     ///< Mode auto en action
/**
 * @}
 **/

extern int mode_aff;                ///< Mode d'affichage

/**
 * @brief Initialisation du manager
 **/
extern void manager_init(void);

/**
 * @brief Run a appeler periodiquement
 **/
extern void manager_run(void);

#endif
