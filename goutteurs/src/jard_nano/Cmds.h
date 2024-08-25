/*
 * Cmds.h
 */
#ifndef CMDS_HEADER_INCLUDED
#define CMDS_HEADER_INCLUDED

class IJardCmd;
class HardwareSerial;
class Cmds
{
private:
	HardwareSerial *m_pSerial;    ///< Config port serie a utiliser

public:
  IJardCmd *m_pJardCmd;         ///< Interface vers les commandes jarduino
  
	/**
	 * @brief Initialise le module interpreteur de commandes avec un port serie
	 * @param pSerial Port serie utiliser
	 * */
	void init(HardwareSerial *pSerial,IJardCmd *pJardCmd);

	/**
	 * @brief A appeller avec serialEvent de Arduino lorsque des datas sont dispos sur le port serie
	 * */
	void onSerialEvent();

	Cmds();
};

extern Cmds cmds;

#endif
