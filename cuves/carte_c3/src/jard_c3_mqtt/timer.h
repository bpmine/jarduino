/**
 * @file timer.h
 * @brief EN-TETE - Gestion d'un timer en ms
*/
#ifndef TIMER_HEADER_INCLUDED
#define TIMER_HEADER_INCLUDED

/**
 * @class Timer
 * @brief Classe timer (en ms)
*/
class Timer
{
  private:
    unsigned long tick0_ms;     ///< Tick de reference pour la mesure du temps
    unsigned long duration_ms;  ///< Duree du timer pre-programmee
    bool running;               ///< Etat running du timer
    bool single;                ///< Time a simple declenchement (si false, le timer declenche periodiquement et se rearme tout seul)
  
  public:
    /**
     * @brief Constructeur
     * @param[in] duration_ms Duree avant declenchement (en ms)
     * @param[in] single true si le timer ne doit pas se rearmer automatiquement
    */
    Timer(unsigned long duration_ms=1000,bool single=true);

    /**
     * @brief Fonction a appeler periodiquement aussi rapidement que possible
     * @return true lorsque le timer arrive a echeance (Le tick suivant renverra false)
    */
    bool tick(void);

    /**
     * @brief Regle le temps avant déclenchement
     * @param[in] duration_ms Temps avant declenchement (en ms)
    */
    void setDuration_ms(unsigned long duration_ms); 

    /**
     * @brief Stop du timer
    */
    void stop(void);

    /**
     * @brief Demarrage du timer (il va declencher au bout de duration_ms)
    */
    void start(void);   

    /**
     * @brief Indique si le timer est en fonctionnement
     * @return true si le timer est en fonctionnement
    */
    bool isRunning(void);

    /**
     * @brief Retourne le temps restant avant declenchement (en ms)
     * @return Temps restant avant declenchement (en ms)
    */
    unsigned long getRemaining_ms(void);

    /**
     * @brief Retourne le temps de declenchement pré-programmé
     * @return Temps de declenchement du timer (en ms)
    */
    unsigned long getDuration_ms(void);
};
#endif
