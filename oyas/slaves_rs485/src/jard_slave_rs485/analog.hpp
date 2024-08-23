#ifndef ANALOG_HEADER_INCLUDED
#define ANALOG_HEADER_INCLUDED

#define MAX_ANALOG_VALS   (4)

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
    Analog()
    {
      m_bSize=MAX_ANALOG_VALS;
      m_bCur=0;
      for (int i=0;i<MAX_ANALOG_VALS;i++)
      {
        m_vals[i]=0;
      }
    }

    Analog(unsigned char bSize)
    {
      m_bSize=bSize;
      if (m_bSize==0)
        m_bSize=1;
      
      m_bCur=0;
      for (int i=0;i<MAX_ANALOG_VALS;i++)
      {
        m_vals[i]=0;
      }
    }
    
    /**
     * @brief Capture d'une valeur analogique
     * @param val Valeur capturee
    */
    void latch(unsigned short usValue)
    {
      m_vals[m_bCur]=usValue;
      m_bCur++;
      if (m_bCur>=m_bSize)
        m_bCur=0;      
    }

    /**
     * @brief Reinitialisation (Effacer la memoire) 
    */
    void reinit(void)
    {
      m_bCur=0;
      for (int i=0;i<MAX_ANALOG_VALS;i++)
      {
        m_vals[i]=0;
      }        
    }

    /**
     * @brief Retourne la dernière valeur
     * @return Derniere valeur mesuree
    */
    unsigned short get(void)
    {
      unsigned long ul=0;
      for (int i=0;i<m_bSize;i++)
      {
        ul+=m_vals[i];
      }
    
      return ul/m_bSize;      
    }
};

#endif
