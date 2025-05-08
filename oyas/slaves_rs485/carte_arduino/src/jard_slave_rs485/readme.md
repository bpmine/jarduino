# Logiciel Esclave RS485 Oyas/Pompe

Ce logiciel pilote un noeud RS485 du système Oyas.

## Protocole

### Principe

Le maître envoi une requête à un esclave et attend une réponse de celui-ci.

Il existe une adresse de broadcast permettant de maintenir la communication réseau vivante pour tous les esclaves qui peuvent considérer alors le réseau comme sain et maintenir les commandes en cours.
Aucun esclave n'est autorisé de répondre à cette adresse de broadcast.

Il utilise des fonctions pour agir sur un esclave. A chaque fonction correspond un format de réponse associé.

### Format de la trame

#### Requête

Seul le maître peut envoyer une requête.

[SOH] {Adresse esclave} {Fonction} {datas} {Checksum} [STX]

Elément | Description | exemple(s)
--- | --- | ---
[SOH] | Valeur 1 (Voir table ASCII) | 
{Adresse esclave} | Caractère de 'A' à 'Y' identifiant un noeud/esclave (1 caractère) | 'A': Pompe, 'B': Oya 1, ...
{Fonction} | Fonction à executer sur le noeud (1 caractère) | '1': Commander la pompe, '2': Commander l'électrovanne, 'p': Ping, ...
{datas} | Donnée pour la commande (1 octet codé en chaîne hexadécimale) |  '12' = 18 , 'AE' = 174, '01' = 1, ...
{Checksum} | Somme de contrôle (1 octet codé en chaîne hxadécimale) | 'AE' = 174, ...
[STX] | Valeur 2 (Voir table ASCII) |

Nota: L'adresse 'Z' est une adresse spéciale qui ne doit jamais être utilisée sur un réseau. C'est l'adresse initiale du noeud après une programmation du micro avec le #define FORCE_INIT. Toujours revenir à un programme sans ce #define après réinit.

Le checksum correspond à la somme modulo 256 de tous les octets envoyée en commençant après le SOH jusqu'aux datas incluses.

### Liste des fonctions

Fonction | Description | Paramètre | Retour | exemple
--- | --- | --- | --- | ---
'1' | Actionner une pompe | 1= ON, 0= OFF | [SOH]{addr}1{Status}{Temp}{Hum>]{Cs>][STX] | [SOH]A100[<Cs>][STX] => OFF, [SOH]A101[<Cs>][STX] => ON
'2' | Actionner une électrovanne | 1= ON, 0= OFF | [SOH]{addr}1{Status}{Temp}{Hum}{Cs}[STX] | [SOH]B200{Cs}[STX] => OFF, [SOH]B201{Cs}[STX] => ON
'@' | Changer l'adresse du noeud | code ascii de la nouvelle adresse (codé en chaîne hexa) | [SOH]{nouvelle addr}@{Status}{Nouvelle addr}{Cs}[STX] | [SOH]Z@59{Cs}[STX] => Le noeud 'Z' devient 'Y' (Ascii 89 / 59H)
'f' | Changer la fonction principale du noeud | code ascii de la nouvelle fonction (codé en chaîne hexa) | | [SOH]Zf31{Cs}[STX] => Le noeud 'A' devient une pompe '1'
'e' | Activer ou non le noeud (un noeud désactivé ne commandera pas sa pompe ou son électrovanne) | "01" pour activer ou "00" pour désactiver | | [SOH]Ae01{Cs}[STX] => Le noeud 'A' est activé
's' | Retourne le temps en s d'actionnement de la pompe ou de l'électrovanne | Laisser à "00"  | | [SOH]As00{Cs}[STX]
'S' | Remet à zéro le compteur de temps | Laisser à "00" | | [SOH]AS00{Cs}[STX]
'r' | Retourne les compteurs d'erreur réseau du noeud| Laisser à "00" | | [SOH]Ar00{Cs}[STX]
'R' | Remet à zéro les compteurs d'erreurs | Laisser à "00" | | [SOH]AR00{Cs}[STX]
'p' | Ping | Valeur quelconque entre '00' et 'FF' | Echo de la valeur passée en paramètre | [SOH]Ap09{Cs}[STX] -> Réponse: [SOH]Ap09{Cs}[STX]

## Taille du programme

| Description | Taille |
| --- | --- |
| Mémoire Flash | 11,154 kOctets |
| RAM | 485 Octets|