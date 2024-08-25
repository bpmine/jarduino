/*
 * autom.h
 */
#ifndef AUTOM_INCLUDED
#define AUTOM_INCLUDED

#define MAX_ANALOG_VALS (4)

typedef struct
{
  int pin;
  unsigned char bNumMB;
} T_PIN_MAPPING;

/**
 * @class Analog
 * @brief Gestion d'une valeur analogique
*/
class Analog
{
  private:
    unsigned short m_vals[MAX_ANALOG_VALS]; ///< Buffer des valeurs analogiques pour faire la moyenne
    unsigned char m_bSize;                  ///< Taille utilisee pour faire la moyenne
    unsigned char m_bCur;                   ///< Position derniere valeur analogique mesuree
    
  public:
    Analog();
    Analog(unsigned char bSize);

    /**
     * @brief Capture d'une valeur analogique
     * @param val Valeur capturee
    */
    void latch(unsigned short val);

    /**
     * @brief Reinitialisation (Effacer la memoire) 
    */
    void reinit(void);

    /**
     * @brief Retourne la dernière valeur
     * @return Derniere valeur mesuree
    */
    unsigned short get(void);  
};

class MB
{
  private:
    unsigned char m_bSize;
    unsigned char *m_pLast;
    unsigned char *m_pCur;
    unsigned char *m_pRising;
    unsigned char *m_pFalling;
  
  public:
    MB();

    void init(unsigned char *pLast,unsigned char *pCur,unsigned char *pRising,unsigned char *pFalling,unsigned char i_bSize);

    void reinit(void);
    void reset(unsigned char i_bNum);
    void set(unsigned char i_bNum);
    void fromMB(unsigned char i_bNum,MB &src,unsigned char i_bNumSrc);
    void fromBool(unsigned char i_bNum,bool flg);

    void start_latch(void);
    void end_latch(void);

    bool get(unsigned char i_bNum);
    bool getRising(unsigned char i_bNum);
    bool getFalling(unsigned char i_bNum);

    void getStateStr(char *o_strState,int i_maxSize);
};

extern MB mbs_inputs;     ///< Mots d'entree
extern MB mbs;            ///< Mots internes
extern MB mbs_outputs;    ///< Mots de sortie

/**
 * @brief Initialisation de l'automate
*/
extern void init_autom(void);

#endif
