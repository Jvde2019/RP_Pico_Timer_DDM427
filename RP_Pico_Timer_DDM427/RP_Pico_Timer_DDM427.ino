/**************************************************************************
 * https://github.com/Jvde2019/RP_Pico_Timer_DDM427/
 */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3C

//#define ROTBut 7  // GPIO7   PIN9   rotary Button
// Menuvariables
volatile boolean Menu = false;
//byte page = 1;
int page = 1;
byte menuitem = 1;
byte menuitem4 = 1;
// Clockvariables
uint32_t delay_time = 1000;  // 1000ms
uint32_t old_time;
uint32_t act_time;
uint32_t sece = 0;  // secundeneiner
uint32_t secz = 0;  // secundenzehner
uint32_t mine = 0;  // minuteneiner
uint32_t minz = 0;  // minutenzehner
// Rotaryvariables
volatile boolean irDir = false;
volatile byte ccw = 0;
volatile byte cw = 0;
volatile byte inc = 0;
volatile int freq = 1720;
bool rm = false;
volatile uint8_t a = 0;
volatile uint8_t b = 0;
volatile boolean right = false;
volatile boolean left = false;

bool buttonPress = false;
bool led_state = false;

//Perhaps the following 2 lines are required for the Pico?  Or use 4,5 instead of (6u), (7u)?
//#define PIN_WIRE_SDA   (21u)
//#define PIN_WIRE_SCL   (22u)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// ISR Button
void buttonPressed() {
  buttonPress = true;
}

// ISR Rotary
void rotaryMoved() {
  // ISR for DDM427 See Datasheet https://hopt-schuler.com/sites/default/files/medien/dokumente/2022-11/miniature_2bit_encoder_427_2022.pdf
  rm = true;
  a = digitalRead(6);
  b = digitalRead(7);
  if (a == b) {
    left = true;
  } else {
    right = true;
  }
  if (irDir) {
    attachInterrupt(6, rotaryMoved, RISING);
    irDir = false;
  } else {
    attachInterrupt(6, rotaryMoved, FALLING);
    irDir = true;
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);

  // Rotary Button INPUT_PULLUP GPIO 11 !!
  pinMode(11, INPUT_PULLUP);
  //pinMode(11, INPUT);
  attachInterrupt(11, buttonPressed, FALLING);
  // can be CHANGE or LOW or RISING or FALLING or HIGH
  // Rotary ChanA INPUT_PULLUP GPIO 6 !!
  pinMode(6, INPUT_PULLUP);
  //pinMode(6, INPUT);
  attachInterrupt(6, rotaryMoved, CHANGE);
  // can be CHANGE or LOW or RISING or FALLING or HIGH
  // Rotary ChanB INPUT_PULLUP GPIO 7 !!
  pinMode(7, INPUT_PULLUP);
  //pinMode(7, INPUT);
  pinMode(8, OUTPUT_8MA);


  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
}

void loop() {

  // Clock
  uhr();
  // Rotaryevent ?
  if (rm) {
    Serial.print("Encoder ist cw: ");
    Serial.print(cw);
    Serial.print(" , ccw:   ");
    Serial.print(ccw);
    Serial.print(" , inc:  ");
    Serial.println(inc);
    Serial.println(freq);
    rm = false;
  }
  Eventhandling();
  menu_new();
}

void uhr() {
  act_time = millis();
  if (act_time - old_time >= delay_time) {
    old_time = act_time;
    //digitalWrite(LED_BUILTIN, HIGH);
    tone(8,freq,200);
    sece = sece + 1;  // secundeneiner hochzählen
    if (sece > 9) {
      sece = 0;
      secz = secz + 1;  // secundenzehner hochzählen
      if (secz > 5) {
        secz = 0;
        sece = 0;
        mine = mine + 1;
        if (mine > 9) {
          mine = 0;
          minz = minz + 1;
          if (minz > 5) {
            minz = 0;
          }
        }
      }
    }
  }
}

void display_Clock() {
  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(3);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(10, 0);
  display.print(minz);
  display.print(mine);
  display.print(":");
  display.print(secz);
  display.print(sece);
  display.display();
}

// Create Menu Pages handle Rotary & Buttonpres Events
void menu_new() {
  switch(page){
    case 1:
    // Page 1 Mainmenu
    // Title
    display.setTextSize(1);
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(15, 0);
    display.print("MAIN MENU");
    display.drawLine(10, 10, 73, 10, SSD1306_WHITE);
    display.setCursor(0, 15);
    if (menuitem == 1) {
      display.setTextColor(BLACK, WHITE);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    display.print("> Settings");
    display.setCursor(0, 25);
    if (menuitem == 2) {
      display.setTextColor(BLACK, WHITE);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    display.print("> Test Encoder");
    if (menuitem == 3) {
      display.setTextColor(BLACK, WHITE);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    display.setCursor(0, 35);
    display.print("> LED_state:");
    if (led_state) {
      display.print("ON");
    } else {
      display.print("OFF");
    }
    if (menuitem == 4) {
      display.setTextColor(BLACK, WHITE);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    display.setCursor(0, 45);
    display.print("> Clock");
    break;

    case 2:
    // Page 2 Encodertest
    display.setTextSize(1);
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(15, 0);
    display.print("ENC. TEST");
    display.drawLine(10, 10, 73, 10, SSD1306_WHITE);
    display.setCursor(5, 15);
    display.print("LEFT      RIGHT");
    display.setTextSize(2);
    display.setCursor(10, 25);
    display.print(cw);
    display.setCursor(65, 25);
    display.print(ccw);
    display.setTextSize(2);
    display.display();
    break;

    case 3:
    // Page 3 Clock
    display_Clock();
    break;

    case 4:
    // Page 4 Settings
    display.setTextSize(1);
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(15, 0);
    display.print("SETTINGS");
    display.drawLine(10, 10, 73, 10, SSD1306_WHITE);
    if (menuitem4 == 1) {
      display.setTextColor(BLACK, WHITE);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    display.setCursor(0, 15);
    display.print("> Frequenz ");
    display.setCursor(70, 15);
    display.print(freq);
    display.setCursor(95, 15);
    display.print("Hz");
    display.setCursor(0, 25);
    if (menuitem4 == 2) {
      display.setTextColor(BLACK, WHITE);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    display.print("> Duration");
    display.setCursor(0, 35);
       if (menuitem4 == 3) {
      display.setTextColor(BLACK, WHITE);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    display.print("> Exit");
    break;
  }
  display.display(); 
}

void Eventhandling(){
  // Take action if a new command received from the encoder
  if (left) {
    left = false;
    ccw++;
    inc++;
    switch(page){
      case 1:
      menuitem--;
      if (menuitem == 0) { menuitem = 4; }
      break;

      case 4:
      menuitem4--;
      if (menuitem4 == 0) { menuitem4 = 3; }
      if (menuitem4 == 1) { freq = freq+10;}
      if (menuitem4 == 2) { freq = freq+10;}
     } 
   }

  if (right) {
    right = false;
    cw++;
    inc--;
    switch(page){
      case 1:
      menuitem++;
      if (menuitem == 5) { menuitem = 1; }
      break;

      case 4:
      menuitem4++;
      if (menuitem4 == 4) { menuitem4 = 1; }      
      if (menuitem4 == 1) { freq = freq-10; }
      if (menuitem4 == 1) { freq = freq-10; }
    }
  }

  if (buttonPress) {
    buttonPress = false;
    switch(page){
      case 1:
      switch(menuitem){
        case 1:
        page = 4;
        break;

        case 2:
        page = 2;
        cw = 0;
        ccw = 0;
        inc = 0;
        break;

        case 3:
        led_state = !led_state;
        digitalWrite(LED_BUILTIN, led_state);
        break;

        case 4:
        page = 3;
        break;
      } 

      break;

      case 2:
      page = 1;
      break;

      case 3:
      page = 1;
      break;

      case 4:
      if (menuitem4 == 3){page = 1;}
      break;
    }
  }
}
