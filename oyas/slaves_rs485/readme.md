# Esclaves RS485

Ce sont des cartes esclaves permettant de piloter pompes et oyas.

## Types de carte

### Carte_arduino

Design initial à base de modules arduino nano.
Gestion des pompes et des oyas avec la même carte.
Les premières cartes comportaient un module RS485 indépendant. Il a ensuite été intégré sur la carte mère.
Ces cartes sont alimentées par le régulateur du module nano.
Il y'a eu plusieurs versions de cette "première" carte. 

Cette carte existe depuis 2023. Elle a donc déjà un long historique. Elle fonctionnait avec un vieux protocole qui a été remplacé dans la dernière version de logiciel.

### Carte AT328

Seule l'alimentation à découpage provient d'un module extérieur. Tout le reste a été intégré en composant CMS ou tranversants (connecteurs).
Il n'y a plus de capteur de température DHT22.
Il n'y a qu'un seul gros bornier pour accueillir les fils du bus.
Elle peut gérer soit une pompe, soir un oya.
Le logiciel utilise le arduinocore (compatible arduino).

### Carte MSP430

Même principe que la carte à base de AT328 mais ne peut gérer qu'un oya.
Il était prévu de pouvoir à terme gérer également la pompe avec cette carte.

Mais 2 MSP430 sur 3 achetés sur aliexpress s'avèrent impossible à programmer! Il semblent qu'il aient été bloqués par logiciels. Peut-être le vendeur a-t-il mélangé des MSP430 corrects aau milieu d'une série bloquée? 

Le choix s'expliquait par ma connaissance de ce micro suite à son utilisation dans l'un de mes projets profesionnels dans les années 2000. 

Même si j'adore ce micro, ce n'était pas une bonne idée:
- L'environnement IAR gratuit n'est plus dispo en téléchargement pour <32k
- Le MSP430F1232 est devenu obsolète

### Carte M0 (avec ATSAMD21G18A-A)

Suite aux problèmes rencontrés par les MSP430 achetés sur aliexpress, je décide d'abandonner définitivement le MSP430.

Je change également de critère pour choisir le cpu:
- Avoir une clock interne performante et assez précise pour ne pas nécessiter de quartz
- Compatible avec l'un des environnements que je connais
- Suffisament de périphériques et de broches pour pouvoir l'utiliser sur d'autres projets
- ...

Tant pis si je sors de l'Arduino... retour aux sources !

Le choix est le ATSAMD21G18A. La soudure à la main est assez délicate mais j'y arrive.