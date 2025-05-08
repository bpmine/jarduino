#ifndef PINS_HEADER_INCLUDED
#define PINS_HEADER_INCLUDED

#define PIN_CPT_LVL_LOW   (2)
#define PIN_CPT_LVL_HIGH  (3)
#define PIN_CPT_FLOW      (4)
#define PIN_TX_EN         (6)
#define PIN_CMD_EV        (7)
#define PIN_LED1          (8)
//#define PIN_LOG_TX        (9)
#define PIN_DHT22         (10)
//#define PIN_LOG_RX        (11)

#define PIN_MES_V         (A1)
#define PIN_ADDR_A1       (A4)
#define PIN_ADDR_A2       (A2)
#define PIN_ADDR_A3       (A5)
#define PIN_ADDR_A4       (A3)

#define PIN_DGB_SYNC      (A0)      ///< @remark A 1 pendant les mesures (temp/hum)
#define PIN_DGB_CMD       (12)      ///< @remark Change d'etat à chaque reception de commande

#endif
