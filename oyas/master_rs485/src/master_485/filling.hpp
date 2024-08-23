/***
 * @file filling.hpp
 * @brief Gestion du contexte pour le remplissage
 *
 * */
#ifndef FILLING_HEADER_INCLIDED
#define FILLING_HEADER_INCLIDED

#include "globals.h"

#define MAX_FILLING_EQTS                    (15)

/**
 * @class SpecialFillElmSettings
 * @brief Gestion des reglages speciaux pour certains OYAs (plus ou moins de remplissage, ...)
 * */
class SpecialFillElmSettings
{
  public:
    int addr;           ///< Adresse de l'oya
    int lowTimeout_s;   ///< Timeout pour atteinte niveau bas
    int fillAfterLow_s; ///< Tempo de remplissage a partir du niveau BAS

    SpecialFillElmSettings(int addr,int lowTimeout_s,int fillAfterLow_s)
    {
      this->addr=addr;
      this->lowTimeout_s=lowTimeout_s;
      this->fillAfterLow_s=fillAfterLow_s;
    }
};

/**
 * Pour le cote barbec, les 3 derniers oyas doivent remplir moins (plus petits)
 * */
#if (defined(NODE_BARBEC) && !defined(NODE_REDUIT))
  #define TIMEOUT_LOW_S                      (80)
  #define TIME_FILLING_AFTER_LOW_S           (60)

  SpecialFillElmSettings tabSpecialFillElmSettings[]={
      SpecialFillElmSettings(6,40,42),
      SpecialFillElmSettings(7,40,50),
      SpecialFillElmSettings(8,40,50)
  };
  /**
   * Pour le cote reduit, les adresses ???  oyas doivent remplir moins (plus petits)
   * @8 et @9 passent a low en 9s puis a high en 44s (Soit 53s pour passer de vide a high)
   * */
#elif (!defined(NODE_BARBEC) && defined(NODE_REDUIT))
  #define TIMEOUT_LOW_S                      (40)
  #define TIME_FILLING_AFTER_LOW_S           (30)

  SpecialFillElmSettings tabSpecialFillElmSettings[]={ };

#else
  #error CONFIGURATION INCONNUE !!
#endif

/**
 * @class FillElm
 * @brief Contexte de remplissage d'un OYA
 * */
class FillElm
{
  public:
    int addr;                         ///< Adresse de l'OYA
    int time_s;                       ///< Temps de remplissage depuis le debut du processus
    unsigned long timeout_low_s;      ///< Timeout pour atteindre LOW
    unsigned long time_after_low_s;   ///< Temps de remplissage apres LOW
    bool low_timed_out;               ///< A ete en timeout de LOW au debut du processus
    bool ended;                       ///< A ete au bout du processus normalement

    void reset()
    {
      addr=0;
      time_s=0;
      timeout_low_s=TIMEOUT_LOW_S;
      time_after_low_s=TIME_FILLING_AFTER_LOW_S;
      low_timed_out=false;
      ended=false;
    }

    FillElm() {reset();}
};

/**
 * @class Filling
 * @brief Classe de gestion des contextes de remplissage des OYAs
 * */
class Filling
{
  private:
    FillElm* tab[MAX_FILLING_EQTS];
    int nbToFill;
    int inxToFill;

    void swap(int i,int j)
    {
      FillElm *p=tab[i];
      tab[i]=tab[j];
      tab[j]=p;
    }

    /**
     * @brief Applique les reglages speciaux au contexte d'OYA passe en parametre
     * @param[in] pElm Pointeur vers un contexte d'OYA
     *
     * Recherche s'il est necessaire d'appliquer des reglages speciaux pour cet OYA
     * */
    void setSpecialFillElmSettings(FillElm *pElm)
    {
      for (unsigned int j=0;j<sizeof(tabSpecialFillElmSettings)/sizeof(SpecialFillElmSettings);j++)
      {
        if (tabSpecialFillElmSettings[j].addr==pElm->addr)
        {
          pElm->timeout_low_s=tabSpecialFillElmSettings[j].lowTimeout_s;
          pElm->time_after_low_s=tabSpecialFillElmSettings[j].fillAfterLow_s;
          break;
        }
      }
    }

    /**
     * @brief Ajoute un contexte de remplissage pour un OYA
     * @param[in] pOya Oya a ajouter au processus de remplissage
     * */
    void add(Oya *pOya)
    {
      if (pOya==nullptr)
        return;

      if (nbToFill>=MAX_FILLING_EQTS)
        return;

      bool found=false;
      for (int i=0;i<nbToFill;i++)
      {
        if (tab[i]->addr==pOya->addr)
        {
          found=true;
          break;
        }
      }

      if (found==false)
      {
        tab[nbToFill]->reset();
        tab[nbToFill]->addr=pOya->addr;

        setSpecialFillElmSettings(tab[nbToFill]);

        nbToFill++;
      }
    }

    /*void reorderToFill(void)
    {
      for (int i=0;i<nbToFill;i++)
      {
        for (int j=0;j<nbToFill;j++)
        {
          Oya *a=api_get_oya(tab[i]->addr);
          Oya *b=api_get_oya(tab[j]->addr);

          if ( (a!=nullptr) && (b!=nullptr) )
          {
            if ( (b->low==true) && (a->low==false) )
              swap(i,j);
            else if ( (b->high==true) && (a->high==false) )
              swap(i,j);
          }
        }
      }
    }*/

  public:
    Filling()
    {
      inxToFill=0;
      nbToFill=0;
      for (int i=0;i<MAX_FILLING_EQTS;i++)
        tab[i]=new FillElm();
    }

    /**
     * @brief Vide la liste des OYAs a remplir
     * */
    void reset(void)
    {
      inxToFill=0;
      nbToFill=0;
      for (int i=0;i<MAX_FILLING_EQTS;i++)
        tab[i]->reset();
    }

    /**
     * @brief Creation de la liste des contextes d'OYA a remplir
     * */
    void createNewFillingList(void)
    {
      reset();

      int pos;
      Oya *oya=api_find_first_oya(pos);
      while (oya!=nullptr)
      {
        /// @remark On a ajoute a la liste les OYAs en ligne sur le RS485 dont le LOW est false
        if ( (oya->comm_ok) && (oya->high==false) && (oya->low==false) )
          add(oya);

        oya=api_find_next_oya(pos);
      }

      //reorderToFill();
    }

    /**
     * @brief Retourne l'index du contexte d'OYA suivant a remplir
     * */
    int getNextInxToFill(void)
    {
      if (nbToFill==0)
        return 0;

      int oldInx=inxToFill;
      int nxt=oldInx+1;

      while (1)
      {
        if (nxt>nbToFill)
          nxt=0;

        if (nxt==oldInx)
        {
          return -1;
        }

        Oya *o=api_get_oya(tab[nxt]->addr);
        if (o!=nullptr)
        {
          /// @remark Le prochain OYA doit être en ligne, ne pas avoir fini le process et ne pas avoir ete en timeout de LOW
          if ((o->comm_ok==true) && (o->high==false) && (tab[nxt]->ended==false) && (tab[nxt]->low_timed_out==false))
          {
            return nxt;
          }
        }

        nxt++;
      }
    }

    void setInxToFill(int nxt)
    {
      if ( ( nxt>=0) && (nxt<MAX_FILLING_EQTS) )
        inxToFill=nxt;
    }

    void setEnded(void)
    {
      if ( ( inxToFill>=0) && (inxToFill<MAX_FILLING_EQTS) )
        tab[inxToFill]->ended=true;
    }

    void setLowTimedOut(void)
    {
      if ( ( inxToFill>=0) && (inxToFill<MAX_FILLING_EQTS) )
        tab[inxToFill]->low_timed_out=true;
    }

    unsigned long getCurLowResetTimeout_ms(void)
    {
      if ( ( inxToFill>=0) && (inxToFill<MAX_FILLING_EQTS) )
        return tab[inxToFill]->timeout_low_s*1000UL;
      else
        return TIMEOUT_LOW_S*1000UL;
    }

    unsigned long getCurFillingAfterLow_ms(void)
    {
      if ( ( inxToFill>=0) && (inxToFill<MAX_FILLING_EQTS) )
        return tab[inxToFill]->time_after_low_s*1000UL;
      else
        return TIME_FILLING_AFTER_LOW_S*1000UL;
    }

    int getAddrOf(int inx)
    {
      if ( (inx>=0) && (inx<MAX_FILLING_EQTS) )
        return tab[inx]->addr;
      else
        return -1;
    }

    int getCurAddr(void)
    {
      return getAddrOf(inxToFill);
    }

    int findFirstAddr(int &pos)
    {
      pos=-1;
      return findNextAddr(pos);
    }

    int findNextAddr(int &pos)
    {
      pos++;
      if (pos<nbToFill)
      {
        return tab[pos]->addr;
      }
      else
      {
        return -1;
      }
    }

    bool hasToFill(void)
    {
      return nbToFill>0?true:false;
    }

    void countTime1s(void)
    {
      if ( (inxToFill>=0) && (inxToFill<nbToFill) )
        tab[inxToFill]->time_s++;
    }
};

Filling Filling;

#endif
