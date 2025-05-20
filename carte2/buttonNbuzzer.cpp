#include "buttonNbuzzer.h"

// --- Internal variables for the debouncer module ---

static int _buttonPin = -1;
static int _samplesRequiredCfg = 5;
static unsigned long _sampleIntervalCfg = 20;

static int _officialButtonState = HIGH;    // The current confirmed, debounced state
static int _lastReportedState = HIGH;      // The state that was last reported by didDebouncedButtonStateChange()

static const int MAX_HISTORY_SIZE = 10;    // Max samples we can store (to size the array)
static int _readingHistory[MAX_HISTORY_SIZE]; // Stores recent raw physical readings
static int _historyIndex = 0;              // Current index in the readingHistory array
static bool _historyFilled = false;        // True once the history buffer has _samplesRequiredCfg readings

static unsigned long _lastSampleTime = 0;  // Timestamp of the last physical read

// --- Function Definitions ---

void setupButton(int pin, int samplesForChange) {
    _buttonPin = pin;

    if (samplesForChange <= 0) {
        _samplesRequiredCfg = 1; // Need at least 1 sample
    } else if (samplesForChange > MAX_HISTORY_SIZE) {
        _samplesRequiredCfg = MAX_HISTORY_SIZE; // Cap at our array's max
    } else {
        _samplesRequiredCfg = samplesForChange;
    }

    pinMode(_buttonPin, INPUT);
    _officialButtonState = digitalRead(_buttonPin);


    // Initialize reading history with the current (assumed or read) state
    int initialReading = digitalRead(_buttonPin); // Read the actual pin state for init
    for (int i = 0; i < _samplesRequiredCfg; i++) {
        _readingHistory[i] = initialReading;
    }
    _officialButtonState = initialReading;
    _lastReportedState = initialReading; // Ensure no "false change" on first check
    _historyIndex = 0;
    _historyFilled = false; // Will become true after enough samples
    _lastSampleTime = millis(); // Allow first sample to happen soon
}

void updateButtonState() {
    if (_buttonPin == -1) return; // Not initialized

    // Only sample at the configured interval
    if (millis() - _lastSampleTime >= _sampleIntervalCfg) {
        _lastSampleTime = millis();

        int currentPhysicalReading = digitalRead(_buttonPin);

        // Store the current reading in our history array (circular buffer)
        _readingHistory[_historyIndex] = currentPhysicalReading;
        _historyIndex = (_historyIndex + 1) % _samplesRequiredCfg;

        // Mark history as filled once we've gone through the array enough times
        if (!_historyFilled && _historyIndex == 0) {
            _historyFilled = true;
        }

        // Only proceed to check for consistency if the history buffer is considered full
        if (_historyFilled) {
            bool allSameInHistory = true;
            // Assume the first element in current history window is the one to check against
            // (The oldest reading in the current circular window)
            int consistentPhysicalState = _readingHistory[(_historyIndex + _samplesRequiredCfg) % _samplesRequiredCfg];


            for (int i = 0; i < _samplesRequiredCfg; i++) {
                 // Check all elements currently in the history window
                if (_readingHistory[i] != consistentPhysicalState) {
                    allSameInHistory = false;
                    break; // No need to check further
                }
            }
            
            // Change of state
            if (allSameInHistory && (consistentPhysicalState != _officialButtonState)) {
                _officialButtonState = consistentPhysicalState;
            }
        }
    }
}

bool didButtonStateChange(int& newState) {
    if (_buttonPin == -1) { // Not initialized
        newState = digitalRead(_buttonPin);
        return false;
    }

    newState = _officialButtonState; // Always provide the current official state

    if (_officialButtonState != _lastReportedState) {
        _lastReportedState = _officialButtonState;
        return true;
    }
    return false;
}

int getCurrentButtonState() {
    if (_buttonPin == -1) { // Not initialized
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