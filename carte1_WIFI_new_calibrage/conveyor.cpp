#include "conveyor.h"

const uint8_t STATE_LOW = 255;
const uint8_t STATE_HIGH = 0;
const uint8_t BACKWARD_PWM_THRESHOLD_A1 = 200;


// Arrête complètement le convoyeur via le Master Enable
void stopConveyor() {
  analogWrite(PIN_MASTER_EN, STATE_LOW);
  analogWrite(PIN_CTRL_A1, STATE_HIGH);
  analogWrite(PIN_SELECT_1, STATE_LOW);
  analogWrite(PIN_SELECT_2, STATE_LOW);
  delay(100);
}

// Freine activement le moteur
void brakeConveyor() {
  analogWrite(PIN_SELECT_1, STATE_HIGH);
  analogWrite(PIN_SELECT_2, STATE_HIGH);
  analogWrite(PIN_CTRL_A1, STATE_HIGH);
  analogWrite(PIN_MASTER_EN, STATE_HIGH);
}

// Fait tourner le convoyeur en AVANT
void moveForward(uint8_t speed) {
  if (speed == 0) {
    stopConveyor();
    return;
  }
  uint8_t inverted_pwm_speed = 255 - speed;
  // S'assure qu'on ne va pas à 255 exactement si speed=0 est géré avant
  inverted_pwm_speed = constrain(inverted_pwm_speed, 0, 254);  // 0=Max, 254=Min

  analogWrite(PIN_SELECT_1, STATE_LOW);
  analogWrite(PIN_SELECT_2, STATE_HIGH);
  analogWrite(PIN_CTRL_A1, inverted_pwm_speed);
  analogWrite(PIN_MASTER_EN, STATE_HIGH);
}

// Fait tourner le convoyeur en ARRIÈRE (vitesse variable mais limitée)
void moveBackward(uint8_t speed) {
  if (speed == 0) {
    stopConveyor();
    return;
  }
  uint8_t pwm_val_A1 = map(speed, 1, 255, BACKWARD_PWM_THRESHOLD_A1, 255);
  // S'assure qu'on reste dans la plage valide pour l'arrière
  pwm_val_A1 = constrain(pwm_val_A1, BACKWARD_PWM_THRESHOLD_A1, 255);

  // Config Mode 2 (Arrière)
  analogWrite(PIN_SELECT_1, STATE_HIGH);
  analogWrite(PIN_SELECT_2, STATE_HIGH);
  analogWrite(PIN_CTRL_A1, pwm_val_A1);
  analogWrite(PIN_MASTER_EN, STATE_LOW);
}