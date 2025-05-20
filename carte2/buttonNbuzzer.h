#ifndef BUTTONNBUZZER_H
#define BUTTONNBUZZER_H

#include <Arduino.h>

// Définitions des notes (fréquences en Hz)
#define REST      0   // Silence

#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370  // Fa# / Solb
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784

/**
 * @brief Initializes the debounced button. Call this in your main setup().
 * @param pin The Arduino pin the button is connected to.
 * @param samplesForChange The number of consecutive identical readings required to confirm a state change.
 */
void setupButton(int pin, int samplesForChange = 5);

/**
 * @brief Updates the internal state of the debounced button. Call this regularly in your main loop().
 */
void updateButtonState();

/**
 * @brief Checks if the debounced button state has changed since the last call to this function.
 * @param newState (Output parameter) If the state changed, this will be populated with the new debounced button state (HIGH or LOW).
 * @return True if the state has changed, false otherwise.
 */
bool didButtonStateChange(int& newState);

/**
 * @brief Gets the current debounced state of the button without affecting the change flag.
 * @return The current debounced button state (HIGH or LOW).
 */
int getCurrentButtonState();

// Déclaration de la fonction pour jouer la mélodie d'arrivée
void playArrivalMelody(int buzzerPin);

void playErrorLoopSegment(int buzzerPin);

#endif // BUTTONNBUZZER_H