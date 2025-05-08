/**
* @file framebuilder.h
* 
* @brief Gestion d'une trame RS485 - Octet par octet (reception) ou par valeurs (emission)
*/
#ifndef FRAMEBUILDER_HEADER_INCLUDED
#define FRAMEBUILDER_HEADER_INCLUDED

#define MAX_BUFFER_SIZE (40)


/**
* @enum T_FRAME_ERR
* @brief Resultat de la reception octet par octet
*/
typedef enum
{
  WAITING=0,			///< Attente d'un octet de synchro STX
  PENDING,			///< Reception en cours, trame non complete
  BAD_SIZE,			///< Taille erronee
  BAD_CS,			///< Checksum errone
  BAD_FRAME,		///< Mauvais format de trame
  UNHANDLED_FRAME,	///< Trame non geree lors de la reception (return false du receiver)
  FRAME_OK			///< La trame est ok. @warning Penser a faire reset apres.
} E_FRAME_ERR;

/**
 * @class FrameBuilder
 *
 * @brief Builder d'une trame
 **/
class Frame;
class FrameCmd;
class FrameOya;
class FramePump;
class FramePing;
class FramePong;
class FrameRazT;
class FrameRazE;
class IFrameReceiver;
class FrameBuilder
{
  private:
	unsigned char buffer[MAX_BUFFER_SIZE];
	unsigned char cs;
	unsigned char pos;
	bool cs_enabled;

	unsigned char hex_val(unsigned char hex);
	unsigned char decode_hex_byte(unsigned char a, unsigned char b);

	char tohexchar(unsigned char b);

	IFrameReceiver *pReceiver;

	void enableCS(void);
	void disableCS(void);
	void pack(unsigned char c);
	void pack_byte(unsigned char b);
	void pack_sbyte(char b);
	void pack_word(unsigned short word);
	void pack_cs(void);
	void calc_size(void);

	void start_build(char msg);
	void end_build(void);

	int first_read(void);
	bool read_char(char *c, int& cur);
	bool read_byte(unsigned char *uc, int& cur);
	bool read_sbyte(char* c, int& cur);
	bool read_word(unsigned short* us, int& cur);

	bool OnFrameDecode(void);

  public:
	/**
	* @brief Constructeur du builder de trame
	*/
	FrameBuilder(void);
	void reset(void);

	unsigned char size(void);
	unsigned char* getBuffer(void);
	int getMaxBufferSize(void);

	/**
	* @brief Fonctions pour construire le buffer a partir d'une trame
	* @{
	*/
	unsigned char* build(FrameCmd *pFrameCmd);
	unsigned char* build(FramePump *pFramePump);
	unsigned char* build(FrameOya *pFrameOya);
	unsigned char* build(FramePing *pFramePing);
	unsigned char* build(FramePong *pFramePong);
	unsigned char* build(FrameRazT *pFrameRazT);
  unsigned char* build(FrameRazE *pFrameRazE);
	/**
	* @}
	*/

	/**
	* @brief Defini un receiver pour gerer les trames recues (avec recv)
	* @param[in] Pointeur vers un receiver
	*/
	void setReceiver(IFrameReceiver *pReceiver);

	/**
	* @brief Reception d'un caractere
	* @param[in] byte Caractere recu
	* @return Etat de la trame (voir T_FRAME_ERR)
	* 
	* Cette fonction gere toute seule la synchronisation sur le caractere de debut de trame et 
	* les incoherences structurelles de la trame en cours de reception.
	*/
	E_FRAME_ERR recv(unsigned char byte);
};


#endif
