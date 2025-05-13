#include "conveyor.h"

// --- Configuration des Broches (Logique Finale) ---
const int PIN_CTRL_A1 = A2;   // Contrôle Vitesse/Trigger
const int PIN_SELECT_1 = A3;  // Sélecteur Mode/Dir 1
const int PIN_SELECT_2 = A4;  // Sélecteur Mode/Dir 2
const int PIN_MASTER_EN = A5; // Master Enable (HIGH=Run, LOW=Stop)

// --- Constantes ---
const uint8_t STATE_LOW = 255;
const uint8_t STATE_HIGH = 0;
// Valeur de A1 où le mode arrière semble s'arrêter/basculer (à ajuster si besoin)
const uint8_t BACKWARD_PWM_THRESHOLD_A1 = 200;


/** @brief Arrête complètement le convoyeur via le Master Enable. */
void stopConveyor() {
  Serial.println(">>> HARD STOP (A4=LOW)");
  analogWrite(PIN_MASTER_EN, STATE_LOW);
  analogWrite(PIN_CTRL_A1, STATE_HIGH);
  analogWrite(PIN_SELECT_1, STATE_LOW);
  analogWrite(PIN_SELECT_2, STATE_LOW);
  delay(100); // Courte pause
}

/** @brief Freine activement le moteur. */
void brakeConveyor() {
  Serial.println(">>> BRAKE (A1=H, A2=H, A3=H, A4=H)");
  analogWrite(PIN_SELECT_1, STATE_HIGH);
  analogWrite(PIN_SELECT_2, STATE_HIGH);
  analogWrite(PIN_CTRL_A1, STATE_HIGH);
  analogWrite(PIN_MASTER_EN, STATE_HIGH);
}

/**
 * @brief Fait tourner le convoyeur en AVANT.
 * @param speed Vitesse souhaitée (0=Stop Complet, 1=Vitesse MIN, 255=Vitesse MAX).
 */
void moveForward(uint8_t speed) {
  if (speed == 0) {
    stopConveyor();
    return;
  }
  uint8_t inverted_pwm_speed = 255 - speed;
  // S'assurer qu'on ne va pas à 255 exactement si speed=0 est géré avant
  inverted_pwm_speed = constrain(inverted_pwm_speed, 0, 254); // 0=Max, 254=Min

  Serial.print("FORWARD | Vitesse: "); Serial.print(speed);
  Serial.print(" -> A1(Inv PWM): "); Serial.println(inverted_pwm_speed);

  // Config Mode 1 (Avant)
  analogWrite(PIN_SELECT_1, STATE_LOW);
  analogWrite(PIN_SELECT_2, STATE_HIGH);
  analogWrite(PIN_CTRL_A1, inverted_pwm_speed);
  analogWrite(PIN_MASTER_EN, STATE_HIGH);
}

/**
 * @brief Fait tourner le convoyeur en ARRIÈRE (vitesse variable mais limitée).
 * @param speed Vitesse souhaitée (0=Stop Complet, 1=Vitesse MIN Arrière, 255=Vitesse MAX Arrière observée).
 */
void moveBackward(uint8_t speed) {
  if (speed == 0) {
    stopConveyor();
    return;
  }

  // Mapper la vitesse désirée (1-255) sur la plage PWM A1 pour l'arrière (BACKWARD_PWM_THRESHOLD_A1 à 255)
  // speed=1 (Min) -> A1=BACKWARD_PWM_THRESHOLD_A1
  // speed=255 (Max) -> A1=255
  uint8_t pwm_val_A1 = map(speed, 1, 255, BACKWARD_PWM_THRESHOLD_A1, 255);
  // S'assurer qu'on reste dans la plage valide pour l'arrière
  pwm_val_A1 = constrain(pwm_val_A1, BACKWARD_PWM_THRESHOLD_A1, 255);

  Serial.print("BACKWARD | Vitesse: "); Serial.print(speed);
  Serial.print(" -> A1(PWM): "); Serial.println(pwm_val_A1);

  // Config Mode 2 (Arrière)
  analogWrite(PIN_SELECT_1, STATE_HIGH);
  analogWrite(PIN_SELECT_2, STATE_HIGH);
  analogWrite(PIN_CTRL_A1, pwm_val_A1);
  analogWrite(PIN_MASTER_EN, STATE_LOW);
}