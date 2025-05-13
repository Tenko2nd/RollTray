#include <Arduino.h>


// --- Configuration des Broches (Logique Finale) ---
extern const int PIN_CTRL_A1;   // Contrôle Vitesse/Trigger
extern const int PIN_SELECT_1;  // Sélecteur Mode/Dir 1
extern const int PIN_SELECT_2;  // Sélecteur Mode/Dir 2
extern const int PIN_MASTER_EN; // Master Enable (HIGH=Run, LOW=Stop)

// --- Constantes ---
extern const uint8_t STATE_LOW;
extern const uint8_t STATE_HIGH;
// Valeur de A1 où le mode arrière semble s'arrêter/basculer (à ajuster si besoin)
extern const uint8_t BACKWARD_PWM_THRESHOLD_A1;

// Pour le tapis
void stopConveyor();
void brakeConveyor();
void moveForward(uint8_t speed);
void moveBackward(uint8_t speed);

