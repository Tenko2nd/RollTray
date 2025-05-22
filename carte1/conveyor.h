#include <Arduino.h>

#define PIN_CTRL_A1 A2
#define PIN_SELECT_1 A3
#define PIN_SELECT_2 A4
#define PIN_MASTER_EN A5

extern const uint8_t STATE_LOW;
extern const uint8_t STATE_HIGH;
// Valeur de A2 où le mode arrière semble s'arrêter/basculer
extern const uint8_t BACKWARD_PWM_THRESHOLD_A1;


void stopConveyor();
void brakeConveyor();
void moveForward(uint8_t speed);
void moveBackward(uint8_t speed);

