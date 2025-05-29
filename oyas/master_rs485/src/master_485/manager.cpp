/**
 * @file manager.cpp
 * @brief Machine � �tats principale: Gestion des menus et des modes de fonctionnement
 * */
#include "manager.h"

#include "api.h"
#include "states.hpp"
#include "btn.hpp"
#include "pins.h"
#include "slaves.hpp"

#include "filling.hpp"
#include "logger.h"
#include "wificomm.h"

#include <RTClib.h>
static DS1307 _rtc;

#define ONE_MINUTE    (1 * 60 * 1000UL)
#define ONE_SECOND    (1000UL)

#define TIMEOUT_START_EV_MS                 (ONE_SECOND)
#define TIMEOUT_STOP_EV_AFTER_STOP_PUMP_MS  (ONE_SECOND)
#define TIMEOUT_STAY_IN_DISPLAY_MS          (ONE_MINUTE)
#define TIMEOUT_WAIT_STEADY_BEFORE_START_MS (6 * 1000UL)
#define TIMEOUT_STOP_PREV_EV_MS             (500)
#define TIMEOUT_CHECK_RTC_PERIOD            (30 * 1000UL)
#define TIMEOUT_CHECK_WIFI_DURATION         (60 * 1000UL)
#define TIMEOUT_READ_RS485                  (6 * 1000UL)

Btn _btn;
int mode_aff=MODE_AFF_IDLE;
Timer tmr1S(1000,false);
bool _flgRtcTriggered=false;

/**
 * @class StateGestion
 * @brief Classe de base � tous les �tats de cette machine
 * Contient des r�f�rences vers tous les �tats et vers la machine
 * */
class StateGestion:public State
{
  private:
      static bool event_1s;

   public:
      static StateMachine _machine;

      static State* stIdle;
      static State* stReadBus;
      static State* stWifiCheck;
      static State* stWifiRemote;
      static State* stDisplay;
      static State* stStartFill;
      static State* stStartEV;
      static State* stStartPump;
      static State* stFillLow;
      static State* stFillWait;
      static State* stChangeEV;
      static State* stStopPump;
      static State* stStopEV;

      static unsigned long seconds;
      static bool event1S(void)
      {
        if (event_1s==true)
        {
          event_1s=false;
          return true;
        }
        return false;
      }

      static void setEvent1s(void)
      {
        seconds++;
        event_1s=true;
      }

      StateGestion() :State(&StateGestion::_machine) {}
};

StateMachine StateGestion::_machine;

/**
 * @class StateIdle
 * @brief Etat d'attente (Bouton, Wifi, ...)
 * */
class StateIdle:public StateGestion
{
    void onEnter() override
    {
      api_master(false);
      Comm.setPower(false);
      _machine.startTimeOut(TIMEOUT_CHECK_RTC_PERIOD);
    }

    void onRun() override
    {
      mode_aff=MODE_AFF_IDLE;

      if (_btn.isRising())
        _machine.setState(stDisplay);
    }

    void onTimeout() override
    {
      DateTime now = _rtc.now();
      char strDte[30];
      sprintf(strDte,"%02d/%02d/%04d %02d:%02d:%02d",
         now.day(),
         now.month(),
         now.year(),
         now.hour(),
         now.minute(),
         now.second()
      );
      //logger.print("Check RTC: ");
      //logger.println(strDte);

      _machine.startTimeOut(TIMEOUT_CHECK_RTC_PERIOD);

      /// @remark Remplissage tous les Dimanche, mercredi et vendredi
      ///if ( ( (now.dayOfWeek()==0) || (now.dayOfWeek()==3) || (now.dayOfWeek()==5) ) && (now.minute()==0) && (now.hour()==12) )

      /// @remark Remplissage tous les midi
      if ( (now.minute()==0) && (now.hour()==12) )
      {
        if (!_flgRtcTriggered)
        {
          _flgRtcTriggered=true;
          _machine.setState(stStartFill);
        }
      }
      ///@remark Toutes les 15 minutes, lecture bus rs485 puis envoi Wifi
      else if ( (now.minute()%15) ==0)
      {
        if (!_flgRtcTriggered)
        {
          _flgRtcTriggered=true;
          _machine.setState(stReadBus);
        }
      }
      else
      {
        _flgRtcTriggered=false;
      }
    }
};

/**
 * @class StateReadBus
 * @brief Etat de lecture du Bus RS485
 * */
class StateReadBus:public StateGestion
{
    void onEnter() override
    {
      logger.println("Enter Read RS485...");
      api_master(true);
      Comm.setPower(true);
      _machine.startTimeOut(TIMEOUT_READ_RS485);
    }
    void onRun() override
    {
      if (_btn.isRising())
        _machine.setState(stDisplay);
    }
    void onTimeout() override
    {
      logger.println("Leave Read RS485...");
      Comm.sendData();
      api_master(false);
      _machine.setState(stWifiCheck);
    }
};


/**
 * @class StateWifiCheck
 * @brief Etat d'attente (Bouton, Wifi, ...)
 * */
class StateWifiCheck:public StateGestion
{
    void onEnter() override
    {
      logger.println("Enter WifiCheck...");
      _machine.startTimeOut(TIMEOUT_CHECK_WIFI_DURATION);
    }
    void onRun() override
    {
      mode_aff=MODE_AFF_CHECK_WIFI;

      if (Comm.isRemoteActive()==true)
        _machine.setState(stWifiRemote);
      else if (_btn.isRising())
        _machine.setState(stDisplay);
      else if (Comm.isAlive()==true)
        _machine.startTimeOut(TIMEOUT_CHECK_WIFI_DURATION);

    }
    void onTimeout() override
    {
        _machine.setState(stIdle);
    }
};


/**
 * @class StateWifiRemote
 * @brief Mode acces a distance par Wifi
 * On active le maitre en entrant, on le desactive en sortant
 * Dans ce mode, on applique les commandes en provenance du reseau
 * */
class StateWifiRemote:public StateGestion
{
    void onEnter() override
    {
      logger.println("Enter Wifi remote");
      api_master(true);
    }

    void onRun() override
    {
      mode_aff=MODE_AFF_REMOTE;

      if (Comm.isRemoteActive()==false)
        _machine.setState(stWifiCheck);
      else
      {
        unsigned short cmds=Comm.getCommands();
        api_set_commands(cmds);
      }
    }

    void onLeave() override
    {
      logger.println("Leave Wifi Remote");
      api_master(false);
    }
};

/**
 * @class StateDisplay
 * @brief Etat d'affichage des OYAs
 *
 * Dans cet etat, on ne commande aucun OYA (ni pompe)
 * On affiche l'�tat de chaque esclave.
 * On rend actif l'interface Wifi mais il n'est pas possible de prendre le contr�le � distance
 **/
class StateDisplay:public StateGestion
{
    void onEnter() override
    {
      logger.println("Enter display");
      api_master(true);
      Comm.setPower(true);
      _machine.startTimeOut(TIMEOUT_STAY_IN_DISPLAY_MS);
    }
    void onRun() override
    {
      mode_aff=MODE_AFF_DISPLAY;

      if (_btn.isRising())
      {
        api_master(false);
        _machine.setState(stIdle);
      }
      else if (_btn.isLongPressed())
        _machine.setState(stStartFill);
    }
    void onLeave() override
    {
      logger.println("Leave display");
    }
    void onTimeout() override
    {
      api_master(false);
      _machine.setState(stIdle);
    }
};

/**
 * @class StateStartFill
 * @brief Etat de demarrage d'un nouveau processus de remplissage
 *
 * - Attente stabilite du reseau
 * - Initialisation de la liste des contextes de remplissage (OYAs a remplir)
 * */
class StateStartFill:public StateGestion
{
    void onEnter() override
    {
      api_master(true);
      Comm.setPower(true);
      logger.println("Enter Start fill");
      Filling.reset();
      event1S(); ///< Mettre a 0 l'event 1s (synchro sur la prochaine)

      _machine.startTimeOut(TIMEOUT_WAIT_STEADY_BEFORE_START_MS);
    }

    void onRun() override
    {
      mode_aff=MODE_AFF_AUTO;

      if (_btn.isRising())
        _machine.setState(stDisplay);
    }

    void onTimeout() override
    {
      Filling.createNewFillingList();

      logger.print("Order: ");
      int pos;
      int ad=Filling.findFirstAddr(pos);
      while (ad!=-1)
      {
        logger.print("@");
        logger.print(ad);
        logger.print(" ");
        ad=Filling.findNextAddr(pos);
      }
      logger.println("");

      if (Filling.hasToFill()==false)
      {
        logger.println("Nothing to fill!");
        _machine.setState(stDisplay);
      }
      else
        _machine.setState(stStartEV);
    }
};

/**
 * @class StateStartEV
 * @brief Etat d'ouverture de l'electrovanne de l'OYA en cours et tempo
 * */
class StateStartEV:public StateGestion
{
    void onEnter() override
    {
      int addr=Filling.getCurAddr();
      if (addr!=-1)
      {
        api_set_oya(addr, true);

        logger.print("Enter Start EV: @");
        logger.println(addr);

        _machine.startTimeOut(TIMEOUT_START_EV_MS);
      }
      else
      {
        logger.println("FIRST OYA NOT FOUND!");
        _machine.setState(stDisplay);
      }
    }
    void onTimeout()
    {
      _machine.setState(stStartPump);
    }
};

/**
 * @class StateStartPump
 * @brief Etat de demarrage de la pompe
 * */
class StateStartPump:public StateGestion
{
    void onEnter() override
    {
      logger.println("Enter Start Pump");
      api_set_pompe(true);
      _machine.setState(stFillLow);
    }
};


/**
 * @class StateFillLow
 * @brief Etat de remplissage de l'OYA jusqu'au niveau LOW (ou timeout)
 **/
class StateFillLow:public StateGestion
{
    void onEnter() override
    {
      Comm.sendData();
      _machine.startTimeOut(Filling.getCurLowResetTimeout_ms());
      logger.print("Enter Fill until LOW (");
      logger.print(Filling.getCurLowResetTimeout_ms());
      logger.println(" ms)");
      event1S();
    }
    void onRun() override
    {
      mode_aff=MODE_AFF_AUTO;

      if (event1S()==true)
        Filling.countTime1s();

      if (_btn.isRising())
        _machine.setState(stStopPump);

      Oya * oya=api_get_oya(Filling.getCurAddr());
      if (oya!=nullptr)
      {
        if (oya->low==true)
          _machine.setState(stFillWait);
        else if (oya->high==true)
          _machine.setState(stChangeEV);
      }
    }
    void onTimeout()
    {
      logger.print("TIMEOUT LOW SLAVE @");
      logger.println(Filling.getCurAddr());
      Filling.setLowTimedOut();
      _machine.setState(stChangeEV);
    }
};

/**
 * @class StateFillWait
 * @brief Etat de remplissage chronometre ou HIGH (depuis le LOW)
 * */
class StateFillWait:public StateGestion
{
    void onEnter() override
    {
      Comm.sendData();
      _machine.startTimeOut(Filling.getCurFillingAfterLow_ms());
      logger.print("Fill TIMED: ");
      logger.print(Filling.getCurFillingAfterLow_ms());
      logger.println(" ms");
    }
    void onRun() override
    {
      mode_aff=MODE_AFF_AUTO;

      if (event1S()==true)
        Filling.countTime1s();

      if (_btn.isRising())
        _machine.setState(stStopPump);

      Oya * oya=api_get_oya(Filling.getCurAddr());
      if (oya!=nullptr)
      {
        if (oya->high==true)
        {
          Filling.setEnded();
          _machine.setState(stChangeEV);
        }
      }
    }
    void onTimeout()
    {
      Filling.setEnded();
      _machine.setState(stChangeEV);
    }
};

/**
 * @class StateChangeEv
 * @brief Etat de changement d'electrovanne
 *
 * Apres avoir determine la suivante, on actionne l'EV et on arme le timeout
 * On ferme l'ancienne EV par timeout et on update le Filling
 **/
class StateChangeEv:public StateGestion
{
    int nxt;

    void onEnter() override
    {
      nxt=Filling.getNextInxToFill();
      if (nxt==-1)
      {
        logger.println("Enter change EV: Goto stop Pump");

        _machine.setState(stStopPump);
      }
      else
      {
        int addr=Filling.getAddrOf(nxt);
        if (addr!=-1)
        {
          api_set_oya(Filling.getAddrOf(nxt), true);
          logger.print("Enter change EV: Start @");
          logger.println(Filling.getAddrOf(nxt));

          _machine.startTimeOut(TIMEOUT_STOP_PREV_EV_MS);
        }
        else
        {
          logger.println("ADRESSE INTROUVABLE!");

          _machine.setState(stStopPump);
        }
      }
    }

    void onTimeout() override
    {
      api_set_oya(Filling.getCurAddr(), false);
      logger.print("Enter change EV: Stop @");
      logger.println(Filling.getCurAddr());

      Filling.setInxToFill(nxt);
      _machine.setState(stFillLow);
    }
};

/**
 * @class StateStopPump
 * @brief Etat de coupure de la pompe
 *
 * On coupe la pompe et on attend le timeout avant de couper les electrovannes
 * */
class StateStopPump:public StateGestion
{
    void onEnter() override
    {
      logger.println("Enter Stop pump");
      api_set_pompe(false);
      _machine.startTimeOut(TIMEOUT_STOP_EV_AFTER_STOP_PUMP_MS);
    }
    void onTimeout()
    {
      _machine.setState(stStopEV);
    }
};

/**
 * @class StateStopEV
 * @brief Etat de coupure de l'electrovanne (de toutes)
 * */
class StateStopEV:public StateGestion
{
    void onEnter() override
    {
      Comm.sendData();
      api_set_commands(0);
      logger.println("Enter Stop EV");
      _machine.setState(stDisplay);
    }
};

State * StateGestion::stIdle = new StateIdle();
State * StateGestion::stReadBus = new StateReadBus();
State * StateGestion::stWifiCheck = new StateWifiCheck();
State * StateGestion::stWifiRemote = new StateWifiRemote();
State * StateGestion::stDisplay = new StateDisplay();
State * StateGestion::stStartFill = new StateStartFill();
State * StateGestion::stStartEV = new StateStartEV();
State * StateGestion::stStartPump = new StateStartPump();
State * StateGestion::stFillLow = new StateFillLow();
State * StateGestion::stFillWait = new StateFillWait();
State * StateGestion::stChangeEV = new StateChangeEv();
State * StateGestion::stStopPump = new StateStopPump();
State * StateGestion::stStopEV = new StateStopEV();

bool StateGestion::event_1s=false;
unsigned long StateGestion::seconds=0;


/**
 * @brief Init du manager
 * */
void manager_init(void)
{
  StateGestion::_machine.setState(StateGestion::stIdle);
  _btn.begin(PIN_TEST_BTN,INPUT_PULLUP,true);
  tmr1S.start();
}


/**
 * @brief A appeller periodiquement pour faire fonctionner la gestion des OYAs
 */
void manager_run(void)
{
  _btn.loop();

  if (tmr1S.tick()==true)
    StateGestion::setEvent1s();

  StateGestion::_machine.run();
}
