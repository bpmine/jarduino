/**
 * @file frames.h
 * @brief Protocol frames management
 */
#ifndef FRAMES_H_
#define FRAMES_H_

#define MAX_BUFFER_SIZE    (40)

typedef struct
{
    unsigned char data[MAX_BUFFER_SIZE];
    unsigned char size;
    unsigned char pos;
    unsigned char cs;
    unsigned char cs_enabled;
} T_FRAME;

typedef enum
{
  WAITING=0,        ///< Attente d'un octet de synchro STX
  PENDING,          ///< Reception en cours, trame non complete
  BAD_SIZE,         ///< Taille erronee
  BAD_CS,           ///< Checksum errone
  BAD_FRAME,        ///< Mauvais format de trame
  UNHANDLED_FRAME,  ///< Trame non geree lors de la reception (return false du receiver)
  FRAME_OK          ///< La trame est ok. @warning Penser a faire reset apres.
} E_FRAME_ERR;

extern void frames_init(T_FRAME *pFrame);

extern void frames_build_oya(T_FRAME *pFrame,unsigned char addr,unsigned char status,unsigned short tick_ms,unsigned char volt,unsigned short time_s,unsigned short errs);
extern void frames_build_pump(T_FRAME *pFrame,unsigned char addr,unsigned char status,unsigned short tick_ms,unsigned char volt,unsigned short time_s,unsigned short errs,unsigned short flow);
extern void frames_build_pong(T_FRAME *pFrame,unsigned char val);

extern E_FRAME_ERR frames_on_receive_byte(T_FRAME *pFrame,unsigned char b);

extern void frames_on_receive_sync(void);
extern void frames_on_receive_cmds(unsigned short cmds,unsigned char addr);
extern void frames_on_receive_ping(unsigned char val);

#endif
