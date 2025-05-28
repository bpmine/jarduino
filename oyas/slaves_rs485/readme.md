# Esclaves RS485

Ce sont des cartes esclaves permettant de piloter pompes et oyas.

## Types de carte

### Carte_arduino

Design initial à base de modules arduino nano.
Gestion des pompes et des oyas avec la même carte.
Les premières cartes comportaient un module RS485 indépendant. Il a ensuite été intégré sur la carte mère.
Ces cartes sont alimentées par le régulateur du module nano.
Il y'a eu plusieurs versions de cette "première" carte. 

Cette carte existe depuis 2023. Elle a donc déjà un plus long historique. Elle fonctionnait avec un vieux protocole qui a été remplacé dans la dernière version de logiciel.

### Carte AT328

Seule l'alimentation à découpage provient d'un module extérieur. Tout le reste a été intégré en composant CMS ou tranversants (connecteurs).
Il n'y a plus de capteur de température DHT22.
Il n'y a qu'un seul gros bornier pour accueillir les fils du bus.
Elle peut gérer soit une pompe, soir un oya.
Le logiciel utilise le arduinocore (compatible arduino).

### Carte MSP430

Même principe que la carte à base de AT328 mais ne peut gérer qu'un oya.

