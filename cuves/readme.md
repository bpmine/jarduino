# Cuves

Nous avons 4 cuves pour une capacité totale de 1600 L.

- Cuve principale de 1000 L
- cuve  "Paul" de 100 L (pour goutteurs petits bacs)
- cuve "Réduit" de 250 L pour les OYAs
- cuve "Barbec" de 250 L pour les OYAs

Deux cuves de 250 alimentent nos deux systèmes de remplissage automatique des OYAs.

Les cuves sont communicantes et un système permet de pomper l'eau de la cuve principale vers les autres. 

## Description des cuves

### Cuve principale (Main)

Cette cuve recueille l'eau du toit de la maison. D'une capacité de 1000 L, elle constitue la réserve principale.
Elle sert à alimenter la cuve "Paul" et aussi la cuve "Réduit" en cas de besoin.

![Cuve principale 1000L](./images/cuve_main.png)

Comme on peut le voir sur la troisième photo, une colonne PVC de 32mm est utilisée comme jauge de niveau.
Cette solution permet d'éviter de percer des trous dans la paroi de la cuve (comme pour les autres).

Trois capteurs à flotteur sont fixés sur cette colonne qui ensuite est remplie de résine pour rendre le tout étanche et durable:

![Mât de mesure](./images/mat.png)

A la base de la cuve, un système de 2 pompes (précédées de filtres) permet de déplacer l'eau vers les autres cuves (Paul et Réduit):

![Système de pompes](./images/pompes_main.png)

Le modèle de pompe utilisé est le AW500S 12V 5m 800L/H:

![Pompe AW500S 12V 5m 800L/H](./images/pompe_AW500S.png)

Pour la petite histoire, nous disposions avant 2023 à cet emplacement d'une cuve de moindre contenance.
Mal positionnée sur son support, un jour de pluie elle est tombée de tout son poids et BOOM !

Depuis, elle marche moins bien...

![Cuve cassée](./images/boom.png)

### Cuve Paul

La cuve "Paul" (de Paul) de 100 L permet d'alimenter un système de goutteurs pour des bacs de jardinage (5 bacs en 2024).

La détection de niveau est réalisée à l'aide de 3 capteurs avec flotteur montés sur la paroi de la cuve.

![Cuve Réduit](./images/cuve_paul.png)

### Cuve Réduit

Cette cuve de 250 L alimente un système de remplisage d'OYAs et permet également d'alimenter la cuve Barbecue si besoin.

La détection de niveau est réalisée à l'aide de 3 capteurs avec flotteur montés sur la paroi de la cuve. 

![Cuve Réduit](./images/cuve_reduit.png)

Le système de capteurs a montré ses limites car des fuites sont apparus au bout d'un an d'utilisation au niveau de ces capteurs.
Pour pallier au problème, de la résine a été utilisée pour recouvrir la zone sensible autour des capteurs.

![Résine sur les capteurs](./images/resine_capteur.png)

### Cuve Barbecue

Cette cuve de 250 L alimente un système de remplisage d'OYAs et un système de goutteurs.
La détection de niveau est réalisée à l'aide de 3 capteurs avec flotteur montés sur la paroi de la cuve. 

![Cuve Barbecue](./images/cuve_barbec.png)

## Filtres imprimés en 3D

Les électrovannes utilisées pour les OYAs et le transfert de l'eau inter-cuves, les pompes et les goutteurs requièrent une eau parfaitement filtrée.
Si pour les goutteurs d'entrée 8mm, cela paraît évident, il faut savoir que c'est également nécessaire pour les électrovannes. En effet, de petits morceaux d'algues peuvent bloquer une électrovanne en position ouverte.

C'est la raison pour laquelle des filtres ont été ajoutés à plusieurs endroits sur le tuyau (Cuves Paul, Réduit et Barbecue).

### Filtre sortie vers tuyau 4mm/8mm

Une pièce a été conçue et imprimée pour filtrer directement sur le tuyau PVC de sortie des cuves.
Elle a été utilisée avant 2023 lorsque tous les systèmes d'arrosages étaient en version goutteurs.

Ce filtre permet de brancher un tuyau 4 mm vers le système de gouteurs. 
Trop fragile, une version similaire avec une sortie 8 mm a ensuite été imprimée.

### Pré-filtre intérieur

Pour filtrer de plus gros morceaux avant le tuyau de sortie, la pièce suivante est vissée à l'intérieur des cuves (Réduit et Barbecue).

## Modules Wifi pour commander les pompes

### Description

Le module de pompage comporte:
- 1 alimentation 12V;
- 1 sortie pompe de 12V/5A;
- 3 entrées de type interrupteur (actif à l'état bas) pour la détection du niveau de cuve.

### Carte Wifi IO

### Protocole d'échange avec MQTT

## Serveur de contrôle

Le serveur comporte un broker MQTT pour centraliser les connexions des différents modules du système.

Un serveur reddis stocke tous les états, les réglages et les commandes des différentes applications.

Les applications sont les suivantes:
- Interraction avec les modules de pompage
- Publication des états sur les écrans
- Webservice pour exposer les services à des clients
- Automatisme de contrôle
- Logger

## Module d'affichage

Le module d'affichage comporte un écran, OLED.

Il permet de visualiser l'état des cuves et l'activité de pompage en temps réel.

