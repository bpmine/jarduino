/**
 * @file pins.h
 *
 * @brief Definition des broches de la carte Master RS485
 *
 * Serial2: Wifi
 * Serial3: Log
**/
#ifndef PINS_HEADER_INCLUDED
#define PINS_HEADER_INCLUDED

#define PIN_TX_EN         (6)     ///< Activation de la ligne RS485 (Pour emettre)
#define PIN_TEST_BTN      (2)     ///< Bouton de test (pull up interne)
#define PIN_PWR_ON        (8)     ///< Commande puissance sur réseau RS485 (esclaves)
#define PIN_DATA_LEDS     (4)     ///< Commande Data des LEDs
#define PIN_PWR_LEDS_INV  (5)     ///< Alimentation des LEDs (via pnp)
#define PIN_PWR_WIFI_INV  (7)     ///< Alimentation module wifi (via pnp)
#define PIN_PWR_LEDS      (9)     ///< Alimentation des LEDs (via npn)
#define PIN_PWR_WIFI      (10)    ///< Alimentation module wifi (via npn)


#endif
