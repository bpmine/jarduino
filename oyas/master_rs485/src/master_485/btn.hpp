/**
 * @file btn.hpp
 * @brief Gestion d'un bouton avec filtrage
 * */
#ifndef BTN_HEADER_INCLUDED
#define BTN_HEADER_INCLUDED

#include <arduino.h>

#define FILTER_BTN_SHORT_US		(2000UL)      ///< Seuil tempo appui court
#define FILTER_BTN_LONG_US		(4000000UL)   ///< Seuil tempo appui long

/**
 * @class Btn
 * @brief Classe de gestion d'un bouton (avec filtrage)
 * */
class Btn
{
  private:
    unsigned long t0;
    bool rawState;
    bool inv;
    int pin;
    bool filteredState;
    bool longState;
    bool shortState;
    bool risingEdge;
    bool fallingEdge;

  public:
    Btn()
    {
      this->pin=-1;
      this->inv=false;
      t0=micros();
      rawState=false;
      filteredState=false;
      shortState=false;
      longState=false;
      risingEdge=false;
      fallingEdge=false;
    }

    void begin(int pin,int mode,bool inv=false)
    {
      this->pin=pin;
      pinMode(pin,mode);

      this->inv=inv;
      rawState=inv;
      filteredState=inv;
      longState=false;
      risingEdge=false;
      fallingEdge=false;
      shortState=false;

      t0=micros();
    }

    void reset(void)
    {
      longState=false;
      risingEdge=false;
      fallingEdge=false;
      shortState=false;
      t0=micros();
    }

    /**
     * @brief A appeller aussi souvent que possible
     * */
    void loop(void)
    {
      if (pin<0)
    	return;

      bool newState=digitalRead(pin)==HIGH?true:false;
      if (inv==true)
        newState=!newState;

      unsigned long t=micros();
      unsigned long delta;
      if (t>=t0)
        delta=t-t0;
      else
        delta=0xFFFFFFFF-t0+t;

      if (newState==rawState)
      {
        if (rawState==true)
        {
          if (delta>FILTER_BTN_SHORT_US)
          {
            if (filteredState!=rawState)
            {
              risingEdge=true;
              shortState=false;
            }

            filteredState=rawState;
          }

          if (delta>FILTER_BTN_LONG_US)
            longState=true;
        }
      }
      else
      {
        rawState=newState;

        if (rawState==false)
        {
          if (filteredState==true)
          {
            fallingEdge=true;

            if (longState==false)
              shortState=true;
          }

          filteredState=false;
        }

        longState=false;
        risingEdge=false;
        fallingEdge=false;

        t0=micros();
      }
    }

    /**
     * @brief Indique que le bouton est (actuellement) presse
     * @return true si le bouton est presse
     * */
    bool isPressed()
    {
      return filteredState;
    }

    /**
     * @brief Indique un front montant (retombe a false au prochain appel)
     * @return true si front montant
     * */
    bool isRising(void)
    {
      bool ret=risingEdge;
      risingEdge=false;
      return ret;
    }

    /**
     * @brief Indique un front descendant (retombe a false au prochain appel)
     * @return true si front descendant
     * */
    bool isFalling(void)
    {
      bool ret=fallingEdge;
      fallingEdge=false;
      return ret;
    }

    /**
     * @brief Indique un appui long
     * @return true si appui long
     * */
    bool isLongPressed(void)
    {
      return longState;
    }

    /**
     * @brief Indique un appui court
     * @return true si appui court
     * */
    bool isShortPressed(void)
    {
      return shortState;
    }

};

#endif
