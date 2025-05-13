#ifndef BUTTONNBUZZER_H
#define BUTTONNBUZZER_H

#include <Arduino.h>

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

#endif // BUTTONNBUZZER_H