/*
 * pins.h
 */
#ifndef PINS_INCLUDED
#define PINS_INCLUDED

#define PIN_CMD_PMP1      (3)
#define PIN_CMD_PMP2      (2)
#define PIN_LED_PMP1      (4)
#define PIN_LED_PMP2      (5)
#define PIN_LED_BATT      (6)
#define PIN_LED_SUN       (7)
#define PIN_LED_CPU       (8)

#define PIN_BTN_PMP1      (9)
#define PIN_BTN_PMP2      (10)
#define PIN_BTN_ONOFF     (11)

#define PIN_DHT_DATA      (12)

#define PIN_MES_POW_SUN   (A0)
#define PIN_MES_POW_BATT  (A1)

#define IB_BTN_PMP1   (0)
#define IB_BTN_PMP2   (1)
#define IB_BTN_ON     (2)

#define OB_CMD_PMP1   (0)
#define OB_CMD_PMP2   (1)
#define OB_LED_PMP1   (2)
#define OB_LED_PMP2   (3)
#define OB_LED_BATT   (4)
#define OB_LED_SUN    (5)
#define OB_LED_CPU    (6)

#define MB_BATT_OK    (0)
#define MB_SUN_OK     (1)
#define MB_COMM_OK    (2)
#define MB_VEILLE     (3)
#define MB_SLEEP      (4)
#define MB_RES2       (5)
#define MB_RES3       (6)
#define MB_RES4       (7)

#define MB_PMP1_FORCED (8)
#define MB_PMP1_ENABLE (9)
#define MB_PMP1_AUTO   (10)
#define MB_PMP1_REMOTE (11)
#define MB_PMP1_RM_CMD (12)

#define MB_PMP2_FORCED (13)
#define MB_PMP2_ENABLE (14)
#define MB_PMP2_AUTO   (15)
#define MB_PMP2_REMOTE (16)
#define MB_PMP2_RM_CMD (17)

#endif
