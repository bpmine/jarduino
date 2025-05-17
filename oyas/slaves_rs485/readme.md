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

## Protocole

Ce protocole est commun à toutes les cartes esclaves.

### Principe

Seul le maître peut initier un échange en envoyant une trame.
Chaque trame d'induit pas nécessairement de réponse de la part des esclaves. Si une réponse est attendue, alors la trame contient l'adresse de l'esclave concerné.

Lors d'un cycle bus, le maître envoi successivement une trame de commande à l'ensemble des noeuds. 
Elle contient:
- l'état souhaité de la pompe et des électrovannes;
- une adresse d'esclave.

Tous les noeuds reçoivent cette trame et obéissent à la consigne les concernant.

Seul le noeud concerné répond avec une trame de pompe ou d'oya contenant l'ensemble des informations concernant son état actuel.

Lorsque le maître a interrogé tous les esclaves, il doit envoyer une trame de synchronisation de fin de cycle. Cette trame permet aux esclaves de travailler tranquilement pendant le temps d'inter-cycle.
Ils peuvent effectuer des traitements pendant lesquels ils pourraient se permettre de perdre des trames du bus. Durant cette période, le traitement des commandes n'est donc pas garanti.
Cette technique permet d'éviter qu'une éventuelle surcharge de cpu ne vienne perturber le cycle réseau.

D'autres commandes, à l'initiative du maître, permettent d'effectuer d'autres opérations de façon asynchrone.

### Format d'une trame

Toute trame doit avoir le format suivant:

[SOH] | {Length} | {Id} | {Datas} | {Checksum} | [STX]

Elément | Taille | Description | exemple(s)
--- | --- | ---
SOH | 1 | Valeur 1 (Voir table ASCII) | 
Length | 2 | Taille de la trame comprenant Length + ID msg + datas + Checksum (1 octet codé en chaîne hexadécimale) | '09' = 9 octets
Id | 1 | Identifiant (ou type) de trame(1 caractère) | 'm' = Trame de commande du maître, 'o' = Réponse d'un oya...
Datas | n=2p | Données propres au type de trame (n octets codés en chaîne hexadécimale) |  Voir chaque type de trame
Checksum | 2 | Somme de contrôle (1 octet codé en chaîne hexadécimale) | 'AE' = 174
[STX] | 1 | Valeur 2 (Voir table ASCII) |

### Liste des types de trames

ID | Maître | Esclave | Description
--- | --- | --- | ---
m | X | | Trame de commande générale envoyée par le maître
p | | X | Trame de réponse envoyée par un esclave de type pompe lorsque son adresse est contenue dans la trame de commande générale
o | | X | Trame de réponse pour un oya (même principe que pour la trame p)
S | X | | Trame SYNC de synchronisation fin de cycle
i | X | | Trame PING envoyé par le maître pour tester un esclave en particulier
y | | X | Trame PONG de réponse d'un esclave à un PING
t | X | | Trame de RAZ du temps destinée à un esclave
z | X | | Trame de RAZ du nombre d'erreurs destinée à un esclave

### Description détaillée des types de trames

#### Principe communs à toutes les trames



#### Trame de commande générale (m)

[SOH]09m{Commandes}{Adresse}{Checksum}[STX]

Elément | Description | exemple(s)
--- | --- | ---
Commandes | 16 bits B15..B0 codés en chaîne hexadécimale | '0802' = Pompe (@1) et oya (@11) actifs
Adresse| Adresse du noeud concerné par la réponse | '0E' = esclave 14, '01' = pompe, ...

B1 correspond à la pompe (activée si à 1)
B2..B14 correspondent aux Oyas @2 à @14 (activés si à 1)

NB: B0 et B15 sont réservés et aucun esclave ne doit interpréter ces commandes.

Les adresses sont comprises entre 1 et 14. Aucun esclave ne doit répondre à des sollicitations pour les adresses en dehors de cet intervalle.

Lorsque le maître envoi cette trame, il attend une réponse de la part de l'esclave concerné. En cas d'absence de réponse de l'esclave, celui-ci est considéré comme non connecté.

#### Trames de réponse d'un esclave (p,o)

Les trames de réponse d'un Oya et d'une pompe sont très similaires.
La différence réside dans l'interprétation de l'octet de statut et par la présence du débit pour la pompe.

[SOH]XXp{Statut}{tick}{temp}{hum}{volt}{total time}{total errs}{flow}{Checksum}[STX]

Elément | Description | exemple(s)
--- | --- | ---
Statut | 1 octet de statut B7..B0 codés en chaîne hexadécimale | '02' = 
tick |  | 


### Description détaillée des échanges

#### Echange Commande/Réponse

#### Echange Ping/Pong

### Description d'un cycle


#define STATUS_CMD     		(0x01)
#define STATUS_ON     		(0x02)
#define STATUS_LVL_LOW    	(0x04)
#define STATUS_LVL_HIGH   	(0x08)


#define ADDR_SYNC		'S'		///< Synchro fin de cycle (pour laisser du temps aux esclave de faire leur traitement)


