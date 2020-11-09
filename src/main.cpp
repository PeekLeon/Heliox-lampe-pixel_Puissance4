#include <Arduino.h>
#define FASTLED_INTERNAL
#include <FastLED.h>

// Grille
#define P4_NB_LIGNES        6   // Hauteur de la grille (par défaut 6 cases)
#define P4_NB_COLONNES       7   // Largeur de la grille (par défaut 7 cases)
#define P4_TAILLE_GRILLE  (P4_NB_LIGNES * P4_NB_COLONNES)
#define P4_POSITION_GRILLE  5   // La grille commence à partir de la colonne n (permet de centrer la grille sur la lampe)
#define P4_COULEUR_GRILLE         0x0000FF // Couleur de la grille (sans pion)
#define P4_COULEUR_FONT_GRILLE    0xFFFFFF // Couleur autour de la grille
#define P4_VITESSE_DEPLACEMENT_PION   150 // Vitesse pour l'annimation de déplacement du pion en millisecondes

// Lampe
#ifdef ARDUINO_ARCH_ESP32
  #define LED_PIN            16   
#else
  #define LED_PIN             6  
#endif

#define COLOR_ORDER       GRB
#define CHIPSET        WS2811
#define BRIGHTNESS        128
#define LAMP_LARGEUR       16   // Largeur de la lampe
#define LAMP_HAUTEUR        8   // Hauteur de la lampe
#define LAMP_TAILLE    (LAMP_LARGEUR * LAMP_HAUTEUR) // Nombre de cases dans la lampe
CRGB leds[LAMP_TAILLE];

// Boutons
#ifdef ARDUINO_ARCH_ESP32
  #define POT_DEPLACEMENT    36   // Potentiomètre du haut
  #define POT_VALIDATION     34   // Potentiomètre du bas
  #define POT_NB_PAS       4096   // Port analogique sur 12bits - 4096 octets
#else
  #define POT_DEPLACEMENT     1   // Potentiomètre du haut
  #define POT_VALIDATION      0   // Potentiomètre du bas
  #define POT_NB_PAS       1024   // Port analogique sur 10bits - 1024 octets
#endif

uint8_t pot_valeur_derniere_validation;
uint8_t pot_valeur_derniere_position;

// Joueurs
#define JOUEUR_1_COULEUR    0xFFFF00 // couleur du joueur 1 (jaune = 0xFFFF00)
#define JOUEUR_2_COULEUR    0xFF0000 // Couleur du joueur 2 (rouge = 0xFF0000)
uint8_t joueurActif;
bool gagnant;

uint8_t positionX = P4_POSITION_GRILLE; // Position du pion en X
uint8_t positionY = P4_NB_LIGNES; // Position du pion en Y
uint8_t grille[P4_NB_COLONNES][P4_NB_LIGNES];  // La grille, cases vide = 0 - joueur 1 = 1 - joueur 2 = 2
uint8_t grilleGagnant[P4_NB_COLONNES][P4_NB_LIGNES]; // Positions des pions gagnants
bool reste_place_vide;
bool afficherCases; // utilisé pour l'animation

uint8_t position_XY(uint8_t x, uint8_t y); // Retourne la position de la case sur le bandeau de LEDs
void initialisation_grille(); // Initialise la grille et le font de la grille
void affichage_deplacement_pion_X(uint8_t x); // Affichage du déplacement du pion en haut de la lampe
void affichage_deplacement_pion_Y(uint8_t y); // Affichage du déplacement du pion vers le bas
void deplacement_pion_X(); // Déplacement du pion en X
void deplacement_pion_Y(); // Déplacement du pion en Y
bool validation_position(); // Valide la position avec le bouton du bas de la lampe
void animation_gagnant(); // Fait clignoter les cases gagnantes
bool test_gagnant(); // Test si 4 pions sont alignés avec les fonctions ci-dessous
bool test_cases_horizontalement(uint8_t ajout = 1);
bool test_cases_verticalement(uint8_t ajout = 1);
bool test_cases_diagonal_droite(uint8_t ajout = 1);
bool test_cases_diagonal_gauche(uint8_t ajout = 1);

void setup() {

  // Initialise la lampe
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, LAMP_TAILLE).setCorrection(TypicalSMD5050);
  FastLED.setBrightness( BRIGHTNESS );

  // Initialisation des variables
  gagnant = false;
  joueurActif = 1;

  // Initialise la grille
  initialisation_grille();
  
  // Initialise le bouton de validation pour ne pas valider dés le début de la partie
  validation_position();
}

void loop() {
  Serial.begin(115200);
  // Test si la grille est pleine
  reste_place_vide = false;
  for(uint8_t i = 0;  i < P4_NB_COLONNES; i++) {
    if(grille[i][P4_NB_LIGNES -1] == 0){
      reste_place_vide = true;
    }
  }
  
  // Réinitialise la grille si elle pleine
  if(reste_place_vide == false){
    delay(1000);
    setup();
  }

  // Si il reste de la place dans la grille et qu'il n'y à pas de gagnant
  if(reste_place_vide && gagnant == false){
    affichage_deplacement_pion_X(positionX + P4_POSITION_GRILLE);
    deplacement_pion_X();
    // Si validation et si il reste de la place dans la colonne
    if (validation_position() && grille[positionX][P4_NB_LIGNES - 1] == 0){
      deplacement_pion_Y();
      validation_position();
      
      // Si il y à un gagnant le dernier joueur reste actif
      if(test_gagnant()){
        gagnant = true;
      }else{
          if(joueurActif == 1){
            joueurActif = 2;
          }else{
            joueurActif = 1;
          }
      }
    
    }
  }

  delay(300);

  // Si il y a un gagnant affiche l'animation et attend une actions sur le bouton du bas pour rejouer 
  if(gagnant){
    if (validation_position()){
      setup();
    }    
    animation_gagnant();
  }

}

// Fonctions
bool test_gagnant(){

  bool test = false;
  if(test_cases_horizontalement(1)){
    test = true;
  }

  if(test_cases_verticalement(1)){
    test = true;
  }

  if(test_cases_diagonal_droite(1)){
    test = true;
  }

  if(test_cases_diagonal_gauche(1)){
    test = true;
  }

  if(test){
    grilleGagnant[positionX][positionY] = 1;
  }

  return test;
}

bool test_cases_verticalement(uint8_t ajout){

  uint8_t testJoueurActif = joueurActif;
  int i = 1;
  uint8_t nb_pion = 1;
  grilleGagnant[positionX][positionY] = ajout;

  while (testJoueurActif == joueurActif && positionY+i < P4_NB_LIGNES){
    testJoueurActif = grille[positionX][positionY+i];
    if(testJoueurActif == joueurActif){
      nb_pion = nb_pion + 1;

      grilleGagnant[positionX][positionY+i] = ajout;
      i++;
    }
  }

  i = 1;
  testJoueurActif = joueurActif;
  
  while (testJoueurActif == joueurActif && positionY-i >= 0){
    testJoueurActif = grille[positionX][positionY-i];
    if(testJoueurActif == joueurActif){
      nb_pion = nb_pion + 1;
      grilleGagnant[positionX][positionY-i] = ajout;
      i++;
    }
  }

  if(nb_pion < 4 && ajout > 0){
    test_cases_verticalement(0);
    return false;
  }
 
  if(ajout == 0){
    return false;
  }
  else{
    return true;
  }
}

bool test_cases_horizontalement(uint8_t ajout){

  uint8_t testJoueurActif = joueurActif;
  int i = 1;
  uint8_t nb_pion = 1;
  grilleGagnant[positionX][positionY] = ajout;

  while (testJoueurActif == joueurActif && positionX+i < P4_NB_COLONNES){
    testJoueurActif = grille[positionX+i][positionY];
    if(testJoueurActif == joueurActif){
      nb_pion = nb_pion + 1;
      grilleGagnant[positionX+i][positionY] = ajout;
      i++;
    }
  }

  i = 1;
  testJoueurActif = joueurActif;

  while (testJoueurActif == joueurActif && positionX-i >= 0){
    testJoueurActif = grille[positionX-i][positionY];
    if(testJoueurActif == joueurActif){
      nb_pion = nb_pion + 1;
      grilleGagnant[positionX-i][positionY] = ajout;
      i++;
    }
  }

  if(nb_pion < 4 && ajout > 0){
    test_cases_horizontalement(0);
    return false;
  }
 
  if(ajout == 0){
    return false;
  }
  else{
    return true;
  }
}

bool test_cases_diagonal_droite(uint8_t ajout){

  uint8_t testJoueurActif = joueurActif;
  int i = 1;
  uint8_t nb_pion = 1;
  grilleGagnant[positionX][positionY] = ajout;

  while (testJoueurActif == joueurActif && positionY+i < P4_NB_LIGNES && positionX+i < P4_NB_COLONNES){
    testJoueurActif = grille[positionX+i][positionY+i];
    if(testJoueurActif == joueurActif){
      nb_pion = nb_pion + 1;
      grilleGagnant[positionX+i][positionY+i] = ajout;
      i++;
    }
  }

  i = 1;
  testJoueurActif = joueurActif;

  while (testJoueurActif == joueurActif && positionY-i >= 0 && positionX-i >= 0){
    testJoueurActif = grille[positionX-i][positionY-i];
    if(testJoueurActif == joueurActif){
      nb_pion = nb_pion + 1;
      grilleGagnant[positionX-i][positionY-i] = ajout;
      i++;
    }
  }

  if(nb_pion < 4 && ajout > 0){
    test_cases_diagonal_droite(0);
    return false;
  }
 
  if(ajout == 0){
    return false;
  }
  else{
    return true;
  }
}

bool test_cases_diagonal_gauche(uint8_t ajout){

  uint8_t testJoueurActif = joueurActif;
  int i = 1;
  uint8_t nb_pion = 1;
  grilleGagnant[positionX][positionY] = ajout;

  while (testJoueurActif == joueurActif && positionY+i < P4_NB_LIGNES && positionX-i >= 0){
    testJoueurActif = grille[positionX-i][positionY+i];
    if(testJoueurActif == joueurActif){
      nb_pion = nb_pion + 1;
      grilleGagnant[positionX-i][positionY+i] = ajout;
      i++;
    }
  }

  i = 1;
  testJoueurActif = joueurActif;

  while (testJoueurActif == joueurActif && positionY-i >= 0 && positionX+i < P4_NB_COLONNES){
    testJoueurActif = grille[positionX+i][positionY-i];
    if(testJoueurActif == joueurActif){
      nb_pion = nb_pion + 1;
      grilleGagnant[positionX+i][positionY-i] = ajout;
      i++;
    }
  }

  if(nb_pion < 4 && ajout > 0){
    test_cases_diagonal_gauche(0);
    return false;
  }
 
  if(ajout == 0){
    return false;
  }
  else{
    return true;
  }
}

void animation_gagnant(){
  long couleurGagnant;

  if(afficherCases){
    afficherCases = false;
  }else{
    afficherCases = true;
  }

  if(joueurActif == 1){
    couleurGagnant = JOUEUR_1_COULEUR;
  }else{
    couleurGagnant = JOUEUR_2_COULEUR;
  }

  for (uint8_t x = 0; x < P4_NB_COLONNES; x++) {
    for (uint8_t y = 0; y < P4_NB_LIGNES; y++){
      
      if(grilleGagnant[x][y] == 1){
        if(afficherCases){
          leds[position_XY(P4_POSITION_GRILLE + x,y)] = couleurGagnant;
        }else{
          leds[position_XY(P4_POSITION_GRILLE + x,y)] = P4_COULEUR_GRILLE;
        }
      } 

    }
  }
  FastLED.show();
}

bool validation_position(){
  uint8_t valeur_validation_pot = analogRead(POT_VALIDATION) / (32*(POT_NB_PAS/1024)); // Divisé par 32 pour avoir 32 crans sur le potentionmétre (1024/32 = 32)
  bool validation = false;

  // Si valeur précédente est différente de la valeur actuel 
  // Si la valeur est paire pour éviter les erreurs entre deux positions voisines. Il y a donc 8 position pour valider.
  if(pot_valeur_derniere_validation != valeur_validation_pot && valeur_validation_pot % 2 == 0){
    pot_valeur_derniere_validation = valeur_validation_pot;
    validation = true;
  }

  return validation;
}

void deplacement_pion_Y(){
  for(uint8_t i = 0;  i < P4_NB_LIGNES; i++) {
    if(grille[positionX][i] == 0){
      grille[positionX][i] = joueurActif;
      positionY = i;
      affichage_deplacement_pion_Y(i);
      i = 200;
    }
  }
  
}

void affichage_deplacement_pion_Y(uint8_t y){
  uint8_t casesGrille = 0;
  for(uint8_t i = LAMP_HAUTEUR;  i > y; i--) {
    casesGrille = position_XY(positionX + P4_POSITION_GRILLE, i -1);
    
    if(i >= P4_NB_LIGNES){
      leds[position_XY(positionX + P4_POSITION_GRILLE, i)] = P4_COULEUR_FONT_GRILLE;
    }else{
      leds[position_XY(positionX + P4_POSITION_GRILLE, i)] = P4_COULEUR_GRILLE;
    }

    if(joueurActif == 1){
      leds[casesGrille] = JOUEUR_1_COULEUR;
    }else{
      leds[casesGrille] = JOUEUR_2_COULEUR;
    }
    FastLED.show();
    delay(P4_VITESSE_DEPLACEMENT_PION);
  }
}

void deplacement_pion_X(){
  uint8_t valeur_position_pot = analogRead(POT_DEPLACEMENT) / (POT_NB_PAS/P4_NB_COLONNES);
  valeur_position_pot = (P4_NB_COLONNES-1) - valeur_position_pot;
  if(valeur_position_pot >= P4_NB_COLONNES){
    valeur_position_pot = P4_NB_COLONNES - 1;
  }
  
  if(positionX != valeur_position_pot){
    positionX = valeur_position_pot;
    affichage_deplacement_pion_X(positionX + P4_POSITION_GRILLE);
  }
}

void affichage_deplacement_pion_X(uint8_t x){
  uint8_t casesGrille = 0;
  for(uint8_t i = 0;  i < P4_NB_COLONNES; i++) {
    casesGrille = position_XY(P4_POSITION_GRILLE + i, LAMP_HAUTEUR - 1);
    leds[casesGrille] = P4_COULEUR_FONT_GRILLE;
  }
  
  if(joueurActif == 1){
    leds[position_XY(x, LAMP_HAUTEUR - 1)] = JOUEUR_1_COULEUR;
  }else{
    leds[position_XY(x, LAMP_HAUTEUR - 1)] = JOUEUR_2_COULEUR;
  }

  FastLED.show();
}

void initialisation_grille(){
    
  // couleur de font
  for (uint8_t i = 0; i < LAMP_TAILLE; i++) {
    leds[i] = P4_COULEUR_FONT_GRILLE;
  }

  // Grille
  uint8_t casesGrille = 0;
  for (uint8_t i = 0; i < P4_NB_LIGNES; i++) {
    for(uint8_t j = 0;  j < P4_NB_COLONNES; j++) {
      casesGrille = position_XY(P4_POSITION_GRILLE + j, P4_NB_LIGNES - (i+1));
      leds[casesGrille] = P4_COULEUR_GRILLE;
      grille[j][i] = 0;
      grilleGagnant[j][i] = 0;
    }
  }

  FastLED.show();
}

uint8_t position_XY(uint8_t x, uint8_t y)
{
  if( x & 0x01) {
    y = (LAMP_HAUTEUR - 1) - y;
  }
    
  return (x * LAMP_HAUTEUR) + y;
}