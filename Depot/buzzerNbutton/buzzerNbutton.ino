/************************************************************************

Buzzer controlled by button

************************************************************************/

const int buzzer = D10;
const int buttonPin = D11;

int buttonState = 0;

void setup(){
  pinMode(buzzer, OUTPUT); 
  pinMode(buttonPin, INPUT);
}

void loop(){
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    tone(buzzer, 1000);
    delay(100);
  } else {
    noTone(buzzer); 
    delay(100);
  }
}