# Rapport Projet STM32 - Système de Contrôle de Servo-moteur

## Introduction

Ce rapport présente le développement d'un système de contrôle de servo-moteur basé sur un microcontrôleur STM32F407. Le projet a été réalisé par Matthieu Tremblai et Tom Cantin. L'objectif était de créer un système capable de contrôler un servo-moteur selon deux modes distincts : en fonction de la distance mesurée par un capteur ultrasonique HC-SR04, ou via des commandes envoyées par une liaison série UART.

## Cahier des charges

Selon les spécifications du projet, le système devait respecter les exigences suivantes :
- Mesure de distance avec le capteur HC-SR04 toutes les 250 ms
- Deux modes de fonctionnement :
  - Mode 1 (Distance) : LED bleue allumée, position du servo-moteur proportionnelle à la distance mesurée (entre 5 et 25 cm)
  - Mode 2 (UART) : LED verte allumée, position du servo-moteur déterminée par une consigne provenant de la liaison série
- Commutation entre les modes via un bouton-poussoir ou une commande série
- Envoi de la distance mesurée via UART toutes les secondes

## Architecture du système

Le système est composé des modules suivants :
1. Module capteur HC-SR04 (SR04.c)
2. Module servo-moteur (motor.c)
3. Module contrôleur (motor_control.c)
4. Programme principal (main.c)

## Brochage et interconnexions

Le schéma fonctionnel du système comprend :
- STM32F407 comme unité centrale
- HC-SR04 pour la mesure de distance
  - TRIG connecté à une broche GPIO (sortie)
  - ECHO connecté à une broche GPIO (entrée)
- Servo-moteur contrôlé par un signal PWM
- Interface UART pour la communication série
- LEDs pour l'indication visuelle des modes
  - LED bleue (GPIOD, PIN 15) pour le mode Distance
  - LED verte (GPIOD, PIN 14) pour le mode UART
- Bouton-poussoir (GPIOA, PIN 0) pour le changement de mode

### Broches principales utilisées dans le projet
| Broche | Fonction | Description |
|--------|----------|-------------|
| PD13 | BUTTON | Bouton-poussoir pour changer de mode |
| PD15 | LED_BLUE | LED bleue (allumée en mode 1) |
| PD12 | LED_GREEN| LED verte (allumée en mode 2) |
| PA1 | TRIG | Signal TRIG du capteur HC-SR04 |
| PA5 | TIM2_CH2 | Canal 2 du Timer 2 - SR04|
| PA6 | TIM3_CH3 | Canal 3 du Timer 3 - Servo moteur|
| PA2 | USART2_TX| Transmission de données série |
| PA3 | USART2_RX| Réception de données série |

## Configuration matérielle (CubeMX)

### Configuration du Timer 2 (TIM2)
Le Timer 2 est utilisé pour la mesure de temps avec le capteur HC-SR04 :
- Mode : Up counting
- Prescaler (PSC) : 16
- Counter Period (ARR) : 65535
- Clock Source : Internal clock
- Channel 1 : Input Capture direct mode
- Polarity Selection : Rising Edge
- IC Selection : Direct
- Prescaler Division Ratio : No division
- Input Filter : 0

### Configuration du Timer 3 (TIM3)
Le Timer 3 est configuré pour générer le signal PWM qui contrôle le servo-moteur :
- Mode : Up counting
- Prescaler (PSC) : 3231
- Counter Period (ARR) : 99
- Clock Source : Internal clock
- Channel 1 : PWM Generation CH1
- Mode : PWM mode 1
- Pulse : 0 (initialement)
- Output compare preload : Enable

### Configuration de l'UART 2 (USART2)
- Mode : Asynchronous
- Baud Rate : 115200 bits/s
- Word Length : 8 bits
- Parity : None
- Stop Bits : 1
- Oversampling : 16
- Mode : TX and RX enabled
- Interruption activée pour la réception

### Configuration des GPIO
- LEDs d'indication
  - GPIOD, PIN 14 : LED verte (sortie push-pull)
  - GPIOD, PIN 15 : LED bleue (sortie push-pull)
- Bouton-poussoir
  - GPIOA, PIN 0 : Entrée
- HC-SR04
  - TRIGGER_PIN : Sortie push-pull
  - ECHO_PIN : Entrée

## Développement des modules

### Module SR04 (Capteur ultrasonique)
Le module SR04.c implémente l'interface avec le capteur ultrasonique HC-SR04 :

1. Initialisation :
   - Configuration des broches GPIO pour TRIGGER (sortie) et ECHO (entrée)
   - Démarrage du Timer 2 pour mesurer le temps d'écho

2. Mesure de distance :
   - Génération d'une impulsion de 10μs sur la broche TRIGGER
   - Mesure du temps entre le front montant et descendant du signal ECHO
   - Calcul de la distance en utilisant la vitesse du son (343 m/s)
   - Gestion des timeouts pour éviter les blocages

3. Principe de fonctionnement : Le capteur HC-SR04 utilise des ultrasons pour mesurer la distance. Il émet une impulsion ultrasonique et mesure le temps nécessaire pour que l'écho revienne après avoir rebondi sur un objet. La distance est calculée en utilisant la formule :
   - Distance = (Temps × Vitesse du son) / 2

La division par 2 est nécessaire car le temps mesuré correspond au trajet aller-retour.

### Module Motor (Servo-moteur)
Le module motor.c gère le servo-moteur :

1. Initialisation :
   - Démarrage du PWM sur le Timer 3, Channel 1

2. Positionnement :
   - Fonction Motor_SetPosition pour définir l'angle du servo (0-180°)
   - Calcul de la largeur d'impulsion en fonction de l'angle
   - Le calcul pulse_width = 5 + (angle * 2) / 45 correspond à une conversion adaptée aux caractéristiques du servo-moteur utilisé

3. Test (Sweep) :
   - Fonction Motor_Sweep pour tester le servo-moteur en le faisant osciller entre 0° et 180°

4. Principe du PWM pour servo-moteur : Les servo-moteurs standard sont contrôlés par des impulsions PWM d'une période de 20ms (50Hz). La largeur de l'impulsion détermine l'angle de rotation :
   - Impulsion de 1ms (~5% duty cycle) : 0°
   - Impulsion de 1.5ms (~7.5% duty cycle) : 90°
   - Impulsion de 2ms (~10% duty cycle) : 180°

### Module Motor Control (Machine à états)
Le module motor_control.c implémente une machine à états pour gérer les modes de fonctionnement :

1. États :
   - S_INIT : État initial
   - S_MODE_DISTANCE : Mode contrôle par distance
   - S_MODE_UART : Mode contrôle par UART
   - S_DEATH : État d'arrêt

2. Événements :
   - E_BUTTON_PRESS : Appui sur le bouton
   - E_UART_COMMAND : Réception d'une commande UART
   - E_STOP : Événement d'arrêt

3. Actions :
   - A_SET_DISTANCE_MODE : Activer le mode distance
   - A_SET_UART_MODE : Activer le mode UART
   - A_UPDATE_MOTOR : Mettre à jour la position du moteur
   - A_STOP : Arrêter le système

4. Matrice de transition : Définit les transitions entre états en fonction des événements

5. Gestion des modes :
   - Mode Distance : Calcul de l'angle en fonction de la distance (0-25cm → 180-0°)
   - Mode UART : Positionnement direct en fonction de la valeur reçue (0-180°)

### Programme principal (Main)
Le programme principal (main.c) coordonne les différents modules :

1. Initialisation :
   - Configuration de l'horloge système
   - Initialisation des périphériques (GPIO, UART, Timers)
   - Initialisation des modules (SR04, Motor, MotorControl)
   - Configuration de la réception UART par interruption

2. Boucle principale :
   - Vérification du bouton-poussoir
   - Traitement des commandes UART
   - Mise à jour du contrôleur moteur

3. Traitement des commandes UART :
   - Réception des caractères dans un buffer
   - Traitement lorsqu'un retour chariot est reçu
   - Conversion en angle (0-180°)
   - Mise à jour du mode et de la position du servo

## Protocole de communication UART
Le protocole UART implémenté est simple mais efficace :

1. Configuration :
   - Vitesse : 115200 bauds
   - Format : 8 bits de données, pas de parité, 1 bit de stop (8N1)

2. Messages envoyés par le système :
   - Message d'accueil au démarrage
   - Rappport de distance périodique (toutes les secondes)
   - Confirmation de changement de mode
   - Confirmation de position du servo-moteur

3. Commandes reçues :
   - Valeur numérique entre 0 et 180 suivie d'un retour chariot (\r ou \n)
   - Cette valeur est interprétée comme l'angle souhaité pour le servo-moteur

4. Principe de l'UART : L'UART (Universal Asynchronous Receiver-Transmitter) est un protocole de communication série asynchrone. Les caractéristiques principales sont :
   - Transmission asynchrone (sans horloge commune)
   - Communication full-duplex (envoi et réception simultanés)
   - Utilisation de bits de start/stop pour la synchronisation
   - Dans ce projet, la réception est gérée par interruption pour ne pas bloquer le système

## Implémentation des exigences

### 1. Mesure de distance
La mesure de distance est réalisée dans la fonction SR04_GetDistance(). Cette fonction :
- Génère une impulsion de 10μs sur la broche TRIGGER
- Mesure le temps de vol avec le Timer 2
- Calcule la distance en centimètres
- Est appelée périodiquement par le contrôleur

### 2. Modes de fonctionnement

#### Mode Distance
Dans ce mode :
- La LED bleue est allumée
- La position du servo est calculée selon la formule :
  - angle = 180 - ((distance - 5) * 180) / 20
Ce qui donne :
  - 5cm → 180°
  - 25cm → 0°
  - Variation linéaire entre ces extrêmes

#### Mode UART
Dans ce mode :
- La LED verte est allumée
- La position du servo est directement déterminée par la valeur reçue via UART
- L'angle est limité entre 0° et 180°

### 3. Changement de mode
Le changement de mode peut être effectué de deux façons :
- En appuyant sur le bouton-poussoir
- En envoyant une commande valide par UART (ce qui active automatiquement le mode UART)

### 4. Reporting périodique
Le système envoie des informations sur UART :
- En mode Distance : distance mesurée et angle correspondant (toutes les secondes)
- En mode UART : position actuelle du servo (toutes les secondes)

## Tests et validation
Pour valider le fonctionnement du système, plusieurs fonctions de test ont été implémentées :

1. Test du servo-moteur :
   - Fonction Motor_Sweep() pour faire balayer le servo de 0° à 180°

2. Test du capteur SR04 :
   - Affichage de la distance mesurée sur UART

3. Test de l'UART :
   - Validation de la réception/transmission avec un terminal série

Ces fonctions de test sont commentées dans la boucle principale du programme mais peuvent être activées pour vérifier le bon fonctionnement de chaque module indépendamment.

## Architecture logicielle
L'architecture logicielle du projet suit le modèle singleton pour chaque module :

1. Modularité :
   - Chaque composant matériel est encapsulé dans un module distinct
   - Interface claire et définie pour chaque module

2. Machine à états :
   - Contrôle du système par une machine à états explicite
   - Transitions clairement définies
   - Actions associées à chaque transition

3. Gestion des événements :
   - Utilisation d'interruptions pour les événements asynchrones (UART)
   - Polling avec debouncing pour le bouton-poussoir

## Conclusion
Le projet implémente avec succès un système de contrôle de servo-moteur à deux modes, répondant à toutes les exigences du cahier des charges. L'architecture modulaire et la machine à états permettent une gestion claire des différents modes de fonctionnement et des transitions entre eux. Le système est robuste, avec une gestion appropriée des événements et des protections contre les erreurs (timeouts, debounce, validation des entrées).

Ce projet démontre l'utilisation efficace des périphériques du STM32 (timers, PWM, UART, GPIO) pour créer un système complet de contrôle, depuis l'acquisition de données (capteur) jusqu'à l'actionnement (servo-moteur), en passant par l'interface utilisateur (LEDs, bouton-poussoir, communication série).