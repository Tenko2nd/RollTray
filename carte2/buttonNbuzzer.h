#ifndef BUTTONNBUZZER_H
#define BUTTONNBUZZER_H

#include <Arduino.h>

// Définitions des notes (fréquences en Hz)
#define REST      0   // Silence

#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784

/**
 * @brief Initialise Le bouton poussoir debounced. call in setup
 * @param pin Le pin du bouton poussoir
 * @param samplesForChange nombre de lecture de valeur consécutive pour définir un nouvel état. Defaut 5.
 */
void setupButton(int pin, int samplesForChange = 5);

/**
 * @brief met a jour l'état du bouton. call in loop
 */
void updateButtonState();

/**
 * @brief Regarde si la valeur du bouton à changer
 * @param newState Le nouvel état du bouton
 * @return True si il a changer, false sinon 
 */
bool didButtonStateChange(int& newState);

/**
 * @brief Récupère l'état actuel du bouton
 * @return L'état du bouton actuel
 */
int getCurrentButtonState();

/**
 * @brief Mélodie d'arriver du convoyeur à la carte
 */
void playArrivalMelody(int buzzerPin);

/**
 * @brief Mélodie d'erreur de la commande
 */
void playErrorLoopSegment(int buzzerPin);

#endif // BUTTONNBUZZER_H