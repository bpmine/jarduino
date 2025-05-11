# Logiciel de tests "Testsbare"

Ce logiciel permet de mettre en route la carte.

C'est le premier logiciel à utiliser après la câblage des composants.

## Première programmation du micro ATMmega328P

### Programmation des fusibles

A la première programmation du micro, il faut programmer les fusibles de la manière suivante:

Fuse Register | Value
--- | ---
EXTENDED | FD
HIGH | D2
LOW | FF

Cela permet de régler l'horloge sur le quartz externe et de couper le micro si Vcc<2.7V.

Pour programmer les fusibles:
- Aller dans Tools->Device Programming;
- Sélectionner la sonde AVRISP mkII et le device  ATmega328P;
- Faire Apply et lire la signature (Si cela ne marce pas, alors on ne peut pas communiquer avec le micro: Revoir les soudures ?);
- Aller dans Fuses, régler les fusibles puis faire Program

### Chargement du programme de test dans le micro

- Ouvrir le projet
- Aller dans Tools->Device Programming;
- Sélectionner la sonde AVRISP mkII et le device  ATmega328P;
- Faire Apply et lire la signature (Si cela ne marce pas, alors on ne peut pas communiquer avec le micro: Revoir les soudures ?);
- Aller dans Memories
- Dans la partie Flash, faire Program

## Objectif et mode d'emploi

On peut ou non utiliser la puce RS485 avec le #define ENABLE_RS485 (recompiler le projet).

- Sans ENABLE_RS485, la carte fait clignoter la LED toutes les secondes (permet de donc de vérifier l'horloge)
- Avec le #define ENABLE_RS485, la carte envoi un 'A' toutes les secondes sur le RS485.

Donc il faut juste vérifier que la LED clignote et éventuellement la réception du 'A' sur le bus.

