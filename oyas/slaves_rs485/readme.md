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

[SOH] {Length} {ID} {datas} {Checksum} [STX]

Elément | Description | exemple(s)
--- | --- | ---
SOH | Valeur 1 (Voir table ASCII) | 
Length | Taille de la trame comprenant Length + ID msg + datas + Checksum (1 octet codé en chaîne hexadécimale) | '09' = 9 octets
ID | Identifiant (ou type) de trame(1 caractère) | 'm' = Trame de commande du maître, 'o' = Réponse d'un oya...
{datas} | Données propres au type de trame (n octets codés en chaîne hexadécimale) |  Voir chaque type de message
{Checksum} | Somme de contrôle (1 octet codé en chaîne hexadécimale) | 'AE' = 174
[STX] | Valeur 2 (Voir table ASCII) |

### Liste des types de trame

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



#define STATUS_CMD     		(0x01)
#define STATUS_ON     		(0x02)
#define STATUS_LVL_LOW    	(0x04)
#define STATUS_LVL_HIGH   	(0x08)


#define ADDR_SYNC		'S'		///< Synchro fin de cycle (pour laisser du temps aux esclave de faire leur traitement)


