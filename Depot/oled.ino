//pin definitions
#define SCK  D12 //serial clock
#define MOSI D11 //master-out slave-in
#define CS   D10 //chip select
#define DC   D7  //data/command control
#define RES  D5  //power reset


#include <Adafruit_SSD1331.h> 


//define colors
#define black   0x0000
#define blue    0x001F
#define red     0xF800
#define green   0x07E0
#define cyan    0x07FF
#define magenta 0xF81F
#define yellow  0xFFE0
#define white   0xFFFF


Adafruit_SSD1331 display = Adafruit_SSD1331(CS, DC, MOSI, SCK, RES); 


void setup() {
  display.begin();  
}


void loop() {
  display.fillScreen(black);  //set background and clear everything


  //display text
  display.setTextColor(white); //set text color
  display.setCursor(25, 0); //set cursor position (x, y)
  display.setTextSize(1); //set the size of text
  display.print("Testing:"); // display text


  //display one letter at a time
  char test[] = {"PMODOLED"}; //store characters
  display.setTextColor(cyan); //set text color
  display.setCursor(25, 15); //set cursor position (x, y)
  display.setTextSize(2); //set the size of text
  for (int i = 0; i < 4; i++) {
    display.print(test[i]);
    delay(200);
  }


  //continue in a new line
  display.setTextColor(yellow); //set text color
  display.setCursor(5, 40); //set cursor position (x, y)
  for (int i = 4; i < 8; i++) {
    display.print(test[i]);
    delay(200);
  }


  //letters with different colors
  display.setTextColor(red); //set text color
  display.print("r");
  delay(200);
  display.setTextColor(green); //set text color
  display.print("g");
  delay(200);
  display.setTextColor(blue); //set text color
  display.print("b");
  delay(200);


  delay(5000);
}