# Goutteurs (Jarduino2)

![Goutteurs](./images/goutteurs.png)

Jarduino2 est une évolution du premier jarduino fabriqué en 2019.

Le premier jarduino était construit avec des platines de prototypage et un arduino MKR Zero Wifi.

Ce nouveau jarduino est construit à partir d'un Nano. On peut le configurer à l'aide d'une interface MODBUS sur l'USB ou bien sur le port série d'extension.
Il est possible de lui ajouter une extension Wifi pour le relier au réseau.

## Vue d'ensemble

![Goutteurs](./images/goutteurs2.png)

Deux systèmes jarduinos étaient actifs en 2024, couvrant deux zones de jardin.
Chaque système est alimenté par une cuve à l'aide d'une pompe ou deux pompes.

Un coffret de commande permet de mettre ou non en service le système et de déclencher un arrosage en mode manuel.

Chaque coffret peut se programmer à l'aide d'un logiciel PC connecté à l'USB de la carte de commande. Cela permet d'activer ou non le mode automatique et de choisir l'heure de déclenchement et la durée d'arrosage.

Le coffret dispose de deux sorties pouvant commander deux pompes. Chaque voie peut alimenter jusqu'à 20 goutteurs. 

## Hardware

### Caractéristiques

  - Alimentation 12V;
  - Entrée de mesure de tension de panneau solaire;
  - 2 sorties 5W pour pompe (De type aquarium);
  - Sonde de température/Humidité;
  - LEDs et interrupteurs.

### Schéma

## Logiciels

### Logiciel embarqué dans la carte de commande
[Logiciel gestion des goutteurs](./src)

### Logiciel PC pour configurer le système

## Protocole MODBUS

Le jarduino implémente un serveur/esclave MODBUS.

Seules les functions suivantes sont implémentées:

- 01 READ COIL STATUS
- 02 READ INPUT STATUS
- 03 READ HOLDING REGISTERS
- 04 READ INPUT REGISTERS
- 15 WRITE MULTIPLE COILS
- 16 WRITE MULTIPLE REGISTERS

### Table des COILs (Lecture/Ecriture)

Adresse | Description
--- | ---
10 | Forçage POMPE 1
11 | Autorisation/Activation POMPE 1
12 | Mode automatique POMPE 1
13 | Mode Distant POMPE 1
14 | Commande distante POMPE 1
--- | ---
20 | Forçage POMPE 2
21 | Autorisation/Activation POMPE 2
22 | Mode automatique POMPE 2
23 | Mode Distant POMPE 2
24 | Commande distante POMPE 2

Ces bits sont enregistrés en EEPROM sauf ceux qui correspondent au forçage des pompes et à leur commande distante.

NB: La commande à distance des pompe est appliquée seulement si le maître modbus communique toujours avec le jarduino nano.

### Table des Entrees (Lecture)

Adresse | Description
--- | ---
0 | Etat LED pompe 1
1 | Etat LED pompe 2
2 | Tension battery OK
3 | Tension panneau solaire OK
4 | Est en mode veille

### Table des INPUT REGISTERs (Lecture)

Adresse | Description
--- | ---
0 | Niveau de batterie en dxV
1 | Niveau du panneau solaire en dxV
2 | Temperature en °C
3 | Humidité en %
4 | Temps restant avant détection Comm perdue (en ms)
5 | Temps restant avant veille (en ms)
--- | ---
10 | Temps restant timer de pompe 1 (en min)
--- | ---
20 | Temps restant timer de pompe 2 (en min)
--- | ---
100 | Nombre total de boots
101 | Temps de fonctionnement total POMPE 1 (en H)
102 | Temps de fonctionnement total POMPE 1 (en H)
103 | Temps total d'ensoleillement (en H)
104 | Nombre total de pressions sur le bouton POMPE 1
104 | Nombre total de pressions sur le bouton POMPE 2
--- | ---
200 | Version (2 pour jarduino 2)
201 | Numéro de série du Jarduino
202 | Version du logiciel nano

### Table des HOLDING REGISTERs (Lecture/Ecriture)

Adresse | Description
--- | ---
0 | Année
1 | Mois
2 | Jour
3 | Heure
4 | Minute
5 | Seconde
--- | ---
10 | Heure de démarrage POMPE 1
11 | Minute de démarrage POMPE 1
12 | Durée (en min) d'activation de la POMPE 1
13 | Jours de la semaine pour l'activation de la POMPE 1 (Bit 0: Dimanche, Bit 1: Lundi, ..., Bit 7: Samedi)
14 | Réglage du délai du timer de pompe 1 (en min)
--- | ---
20 | Heure de démarrage POMPE 2
21 | Minute de démarrage POMPE 2
22 | Durée (en min) d'activation de la POMPE 2
23 | Jours de la semaine pour l'activation de la POMPE 2 (Bit 0: Dimanche, Bit 1: Lundi, ..., Bit 7: Samedi)
24 | Réglage du délai du timer de pompe 2 (en min)