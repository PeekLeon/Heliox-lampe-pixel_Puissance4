# Heliox lampe pixel : Puissance 4

Jouez au célèbre jeu du puissance 4 sur la lampe pixel d'Heliox.  
Le programme a été fait pour ne pas avoir à modifier la lampe. Nous utilisons donc les 2 potentiomètres de la lampe pour déplacer les pions et pour valider.

<img src="https://raw.githubusercontent.com/PeekLeon/Heliox-lampe-pixel_Puissance4/master/HelioxPuissance4.jpg" height=300px/>

Le jeu a été testé sur l'Arduino Mega mais devrait fonctionner sur un ESP32 (pas testé).

## Installation

Le code a été développé avec **Visual Studio Code** et l'extension **PlateformIO**.
Vous pouvez donc le déployer via ces derniers.  

Si vous utilisez l'**ide d'Arduino** il faut créer un nouveau projet et copier/coller le code contenu dans `src/main.cpp` (tout le code est dans ce fichier).

## Configurer

Par défaut le code est fait pour fonctionner sur la carte Arduino Mega sur les ports utilisés par Heliox.

### Pins

- Potentiomètre du haut : `POT_DEPLACEMENT` par défaut port `A1`
- Potentiomètre du bas : `POT_VALIDATION` par défaut port `A0`
- Gestion du bandeau de LED : `LED_PIN` par défaut port `6`

### Couleur

- Joueur 1 : `JOUEUR_1_COULEUR` par défaut `0xFFFF00` couleur jaune
- Joueur 2 : `JOUEUR_2_COULEUR` par défaut `0xFF0000` couleur rouge
- Grille du puissance 4 : `P4_COULEUR_GRILLE` par défaut `0x0000FF` couleur bleu
- Contour de la grille : `P4_COULEUR_FONT_GRILLE` par défaut `0xFFFFFF` couleur blanc

### Grille

- Largeur : `P4_NB_COLONNES` par défaut `7`
- Hauteur : `P4_NB_LIGNES` par défaut `6`
- Position du début de la grille : `P4_POSITION_GRILLE` par défaut `5`

### Animation

- Vitesse de déplacement des pions (en millisecondes) : `P4_VITESSE_DEPLACEMENT_PION` par défaut `150`