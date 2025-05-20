#include <Arduino.h>
#include "conveyor.h"
#include "iot.h"

extern bool calibrageFait;
extern bool calibrageEnCours;
extern bool bascule_etat_0;

extern bool carte1_verifiee;
extern bool carte2_verifiee;
extern bool carte3_verifiee;
extern bool carte4_verifiee;

extern unsigned long t0;
extern unsigned long t1;
extern unsigned long t2;
extern unsigned long t3;
extern unsigned long t4;

extern unsigned long t01;
extern unsigned long t12;
extern unsigned long t23;
extern unsigned long t34;

extern unsigned long tempsDepart;



extern bool attenteNouvelleCarte0;

extern const unsigned long TEMPS_MAX_CALIBRAGE;
extern unsigned long temps_debut_calibrage;

void reinitialize_variables();

void setup_conveyor();

bool toutesCartesVerifiees();

// Check Timers
bool checkTimerCard1();
bool checkTimerCard2();
bool checkTimerCard3();
bool checkTimerCard4();