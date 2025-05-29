/**
 * @file async.hpp
 *
 * @brief Gestion des commandes Asynchrones sur le bus
 *
 * Cette classe permet de suivre l'avancement des commandes asynchrones sur le bus 485
 * Les commandes asynchrones sont envoyées après les requetes synchrones du cycle et avant le ASYNC
 *
 * Une instance AsyncCmd, visible de partout est disponible pour les utilisateurs.
**/
#ifndef ASYNC_HEADER_INCLUDED
#define ASYNC_HEADER_INCLUDED

#include "prot.h"

/**
 * @class AsyncCmd
 * @brief Gestion des commandes asynchrones
 * */
class AsyncCmd
{
  private:
    unsigned char cmd;
    unsigned char addr;
    unsigned char val;
    bool sent;
    bool pending;
    bool acked;

    /**
     * @brief Remise à zero (Annulation de toute commande en cours)
     **/
    void reset(void)
    {
      this->cmd=0;
      this->addr=0;
      this->val=0;
      sent=false;
      pending=false;
      acked=false;
    }

    /**
     * @brief Envoi d'une commande asynchrone
     * @param[in] cmd Type de commande
     * @param[in] addr Adresse de l'esclave a qui envoyer la commande
     * @param[in] val Parametre optionnel de la commande
     * @return true si la commande a pu etre envoyee sinon false (commande deja en cours par exemple)
     **/
    bool send_cmd(unsigned char cmd,unsigned char addr,unsigned char val=0)
    {
      if (pending==false)
      {
        reset();
        this->cmd=cmd;
        this->addr=addr;
        this->val=val;
        pending=true;   ///<Mise en attente d'une reponse

        return true;
      }

      return false;
    }

  public:
    AsyncCmd() {reset();};

    /**
     * @brief Indique si une commande est deja en cours
     * @return true Si commande en cours
     * */
    bool is_pending(void) {return pending;};

    /**
     * @brief Retourne la derniere commande envoyee
     * @return Commande en cours
     * */
    unsigned char get_cmd(void) {return cmd;}

    /**
     * @brief Retourne l'adresse de la derniere commande
     * @return Adresse de la derniere commande
     * */
    unsigned char get_addr(void) {return addr;}

    /**
     * @brief Remise a zero de la duree totale de remplissage (Memorisee en EEPROM sur Arduino et ATMEGA328. En RAM sur MSP430)
     * @param[in] addr Adresse de l'esclave a remettre a zero
     * */
    bool send_raz_total(unsigned char addr) {return send_cmd(MSG_RAZ_TIME, addr);}

    /**
     * @brief Remise a zero du nombre d'erreurs (Memorisee en EEPROM sur Arduino et ATMEGA328. En RAM sur MSP430)
     * @param[in] addr Adresse de l'esclave a remettre a zero
     * */
    bool send_raz_errs(unsigned char addr) {return send_cmd(MSG_RAZ_ERR, addr);}

    /**
     * @brief Envoi d'un PING a un esclage qui doit repondre PONG
     * @param[in] addr Adresse de l'esclave
     * */
    bool send_ping(unsigned char addr) {return send_cmd(MSG_PING, addr);}

    /**
     * @brief Signale la bonne emission de la commande asynchrone
     * */
    void set_sended(void)
    {
      if (pending==true)
      {
        if (cmd==MSG_PING)
        {
          sent=true;
        }
        else
        {
          reset();
        }
      }
      else
      {
        reset();
      }
    }

    /**
     * @brief Signale la bonne reception de la reponse a une commande asynchrone
     * @warning Cette fonction doit etre appellee apres reception d'une trame concernee
     * */
    void set_acked(void) {reset();}

    /**
     * @brief Signale un timeout pendant que l'on attend une reponse a une commande asynchrone
     * */
    void set_tmt(void) {reset();}
};

AsyncCmd AsyncCmd;

#endif
