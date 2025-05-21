#include "buttonNbuzzer.h"

// --- Variable interne pour le debounced bouton ---

static int _buttonPin = -1;
static int _samplesRequiredCfg = 5;        // nombre d'état identique d'affiler pour confirmer un changement
static unsigned long _sampleIntervalCfg = 20; // temps entre chaque prs de mesure

static int _officialButtonState = HIGH;    // L'état actuel du bouton
static int _lastReportedState = HIGH;      // L'ancien état du bouton

static const int MAX_HISTORY_SIZE = 10;    // nombre maximum de _samplesRequiredCfg
static int _readingHistory[MAX_HISTORY_SIZE]; // historique des prélèvement d'état
static int _historyIndex = 0;              // indice de l'_historyIndex
static bool _historyFilled = false;        // vrai des que le _readingHistory contient au moins _samplesRequiredCfg

static unsigned long _lastSampleTime = 0;  // dernière mesure d'état

// --- Function Definitions ---

void setupButton(int pin, int samplesForChange) {
    _buttonPin = pin;

    // définit _samplesRequiredCfg
    if (samplesForChange <= 0) {
        _samplesRequiredCfg = 1;
    } else if (samplesForChange > MAX_HISTORY_SIZE) {
        _samplesRequiredCfg = MAX_HISTORY_SIZE;
    } else {
        _samplesRequiredCfg = samplesForChange;
    }

    pinMode(_buttonPin, INPUT);
    _officialButtonState = digitalRead(_buttonPin);


    // initialise le_readingHistory
    int initialReading = digitalRead(_buttonPin);
    for (int i = 0; i < _samplesRequiredCfg; i++) {
        _readingHistory[i] = initialReading;
    }
    _officialButtonState = initialReading;
    _lastReportedState = initialReading;
    _historyIndex = 0;
    _historyFilled = false;
    _lastSampleTime = millis();
}

void updateButtonState() {
    if (_buttonPin == -1) return; // Not initialized

    if (millis() - _lastSampleTime >= _sampleIntervalCfg) {
        _lastSampleTime = millis();

        int currentPhysicalReading = digitalRead(_buttonPin);

        _readingHistory[_historyIndex] = currentPhysicalReading;
        _historyIndex = (_historyIndex + 1) % _samplesRequiredCfg;

        // l'historique est complet
        if (!_historyFilled && _historyIndex == 0) {
            _historyFilled = true;
        }

        if (_historyFilled) {
            bool allSameInHistory = true;
            // Prend une valeur de comparaison
            int consistentPhysicalState = _readingHistory[(_historyIndex + _samplesRequiredCfg) % _samplesRequiredCfg];

            // Regarde s'il y a un différent
            for (int i = 0; i < _samplesRequiredCfg; i++) {
                if (_readingHistory[i] != consistentPhysicalState) {
                    allSameInHistory = false;
                    break;
                }
            }
            
            // Si encore dans la boucle, changemet d'état
            if (allSameInHistory && (consistentPhysicalState != _officialButtonState)) {
                _officialButtonState = consistentPhysicalState;
            }
        }
    }
}

bool didButtonStateChange(int& newState) {
    if (_buttonPin == -1) {
        newState = digitalRead(_buttonPin);
        return false;
    }

    newState = _officialButtonState;

    if (_officialButtonState != _lastReportedState) {
        _lastReportedState = _officialButtonState;
        return true;
    }
    return false;
}

int getCurrentButtonState() {
    if (_buttonPin == -1) {
        return digitalRead(_buttonPin);
    }
    return _officialButtonState;
}


// Fonction pour jouer la mélodie d'arrivée
void playArrivalMelody(int buzzerPin) {
  int melody[][2] = {
    {NOTE_C4, 150},
    {NOTE_E4, 150},
    {NOTE_G4, 150},
    {NOTE_C5, 300},
    {REST,     50},
    {NOTE_G5, 200},
    {NOTE_E5, 200},
    {NOTE_C5, 350},
    {REST,     50},
    {NOTE_F5, 250},
    {NOTE_G5, 500}
  };

  int numNotes = sizeof(melody) / sizeof(melody[0]);
  int pauseBetweenNotes = 30;

  for (int i = 0; i < numNotes; i++) {
    int noteFrequency = melody[i][0];
    int noteDuration = melody[i][1];

    if (noteFrequency == REST) {
      delay(noteDuration);
    } else {
      tone(buzzerPin, noteFrequency, noteDuration);
      delay(noteDuration + pauseBetweenNotes);
    }
  }
  noTone(buzzerPin);
}

// Joue la mélodie d'erreur
void playErrorLoopSegment(int buzzerPin) {
  int error_segment[][2] = {
    {NOTE_G4,  150},
    {NOTE_FS4, 150},
    {NOTE_F4,  150},
    {NOTE_E4,  250},
  };

  int numNotes = sizeof(error_segment) / sizeof(error_segment[0]);
  int pauseBetweenNotes = 20;

  for (int i = 0; i < numNotes; i++) {
    int noteFrequency = error_segment[i][0];
    int noteDuration = error_segment[i][1];

    tone(buzzerPin, noteFrequency, noteDuration);
    delay(noteDuration + pauseBetweenNotes);
  }
  noTone(buzzerPin);
}