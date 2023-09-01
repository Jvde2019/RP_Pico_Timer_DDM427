/**************************************************************************
 * https://github.com/Jvde2019/RP_Pico_Timer_DDM427/
 */

//#include <SPI.h>
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
int menuitem = 1;
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
bool rt_mov = false;
volatile boolean rt_right = false;
volatile boolean rt_left = false;
volatile boolean rt_irdir = false;
volatile byte rt_ccw = 0;
volatile byte rt_cw = 0;
volatile byte rt_inc = 0;
volatile uint8_t rt_a = 0;
volatile uint8_t rt_b = 0;

// Button variables
bool bt_mov = false;
volatile boolean bt_irDir = false;
bool bt_press = false;
bool bt_releas = false;
bool bt_shortpress = false;
bool bt_longpress = false;
volatile uint8_t bt_c = 0;
uint32_t bt_timepressed = 0;
uint32_t bt_timereleased = 0;
uint32_t bt_deltatime = 0;

// Statecontrol variables
uint8_t state = 0;
bool run = false;

bool buttonPress = false;
bool led_state = false;
volatile int freq = 1720;

int line;
int startitem = 0;
int enditem;
char Menu_one[] = "MAIN MENU";
// Constants:-
// char Item_0[15];
// char Item_1[15];
// char Item_2[15];
// char Item_3[15];
// char Item_4[15];
// char Item_5[15];
// char Item_6[15];
// char Item_7[15];
//Mainmenu
const byte numChars = 16;
const char mmArray[][16] = {"> Exit", "> Settings", "> Encodertest", "> LED_state:", "> Clock", "> Item_6", "> Item_7", "> Item_8", "> Item_9"};
int *p;
//Mainmenu
//const byte numChars = 16;
const char smArray[][16] = {"> Exit", "> sSettings", "> sEncodertest", "> sLED_state:", "> sClock", "> sItem_6", "> sItem_7", "> sItem_8", "> sItem_9"};

//p = &mmArray[][];

//strcpy(Item_0[14], "Mainmenu");

//Perhaps the following 2 lines are required for the Pico?  Or use 4,5 instead of (6u), (7u)?
//#define PIN_WIRE_SDA   (21u)
//#define PIN_WIRE_SCL   (22u)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// ISR Button
void buttonPressed() {
  buttonPress = true;
}

// ISR Button
// detection of falling and raising edge of Buttonpin
void buttonEvent(){
  bt_mov = true;
  bt_c = digitalRead(11);
  if (bt_c == false) {
    bt_press = true;
  } else {
    bt_releas = true;
  }  
  if (bt_irDir) {
    attachInterrupt(11, buttonEvent, RISING);
    bt_irDir = false;
  } else {
    attachInterrupt(11, buttonEvent, FALLING);
    bt_irDir = true;
  }
}

// ISR Rotary
void rotaryMoved() {
  // ISR for DDM427 See Datasheet https://hopt-schuler.com/sites/default/files/medien/dokumente/2022-11/miniature_2bit_encoder_427_2022.pdf
  rt_mov = true;
  rt_a = digitalRead(6);
  rt_b = digitalRead(7);
  if (rt_a == rt_b) {
    rt_left = true;
  } else {
    rt_right = true;
  }
  if (rt_irdir) {
    attachInterrupt(6, rotaryMoved, RISING);
    rt_irdir = false;
  } else {
    attachInterrupt(6, rotaryMoved, FALLING);
    rt_irdir = true;
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);

  // Rotary Button INPUT_PULLUP GPIO 11 !!
  pinMode(11, INPUT_PULLUP);
  //pinMode(11, INPUT);
  attachInterrupt(11, buttonEvent, CHANGE);
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
  //statecontrol();
  // Rotaryevent ?
  if (rt_mov) {
    Serial.print("Encoder ist rt_cw: ");
    Serial.print(rt_cw);
    Serial.print(" , rt_ccw:   ");
    Serial.print(rt_ccw);
    Serial.print(" , rt_inc:  ");
    Serial.print(rt_inc);
    Serial.print("Menuitem :");
    Serial.print(menuitem);
    Serial.print("  state :");
    Serial.println(state );
    rt_mov = false;
  }
  //Eventhandling();
  Eventhandling_new();
  //menu_new();
//  statecontrol();
  program_control();
}

void uhr() {
  act_time = millis();
  if (act_time - old_time >= delay_time) {
    old_time = act_time;
    //digitalWrite(LED_BUILTIN, HIGH);
    tone(8,freq,200);
    sece ++;  // secundeneiner hochzählen
    if (sece > 9) {
      sece = 0;
      secz ++;  // secundenzehner hochzählen
      if (secz > 5) {
        secz = 0;
        sece = 0;
        mine ++;
        if (mine > 9) {
          mine = 0;
          minz ++;
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

void program_control() {
  switch(state){
    case 0:
    // display clock shortpress calls Mainmenu 
    display_Clock(); 
    if (bt_shortpress){
      state = 1;
      bt_shortpress = false;
    }
    break;
    case 1:
    menu_new();
    // display Mainmenu  
    if (bt_shortpress){
      state = menuitem;
      bt_shortpress = false;
      //digit = 1;
      Serial.println(state);
    } 
    if (bt_longpress){
      state = 3;
      page = 4;
      bt_longpress = false;
      //digit = 1;
      Serial.println(state);
    } 
    if (rt_right){
      rt_right = false;
      menuitem ++;
      // Check if we have Display to shift 
      if (menuitem > 4) {
        startitem = menuitem - 4;
        }
      else{
        startitem = 0;
      } 
      // Check if last item 
      if (menuitem == 9) {
        menuitem = 0; 
        startitem = 0;
        }
    }
    if (rt_left){
      rt_left = false;
      menuitem --;
      if (menuitem > 4) {
        startitem = menuitem - 4;
        }
      else {
        startitem = 0;
      }  
      if (menuitem == -1) { 
        menuitem = 8; 
        startitem = menuitem -4;
        }
    }    
    break;

    case 2:
    if (bt_longpress){
      bt_longpress = false;
      state = 0;
    }
    break;

    case 3:
    if (bt_longpress){
      bt_longpress = false;
      state = 0;
    }
    break;

    case 4:
    if (bt_longpress){
      bt_longpress = false;
      state = 0;
    }
    break;  
  }
}

//void makemenu(int &startitem, int &menuitem, char c[]){
void makemenu(int &a, int &b, const char (&c)[][numChars]){    
  // Items Display can display 5 Items [0..4] + Title
    line = 15;
    //startitem = 0;
    enditem = a + 4;

    for (int item = a; item <= enditem ; item++){
      display.setCursor(0, line);
      if (b == item) {
        display.setTextColor(BLACK, WHITE);
        }
      else {
      display.setTextColor(SSD1306_WHITE);  
      }  
      //display.print(mmArray[item]);
      display.print(c[item]);
      line = line + 10;
    } 

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
    
    makemenu(startitem, menuitem, smArray);
    // // Items Display can display 5 Items [0..4] + Title
    // line = 15;
    // //startitem = 0;
    // enditem = startitem + 4;

    // for (int item = startitem; item <= enditem ; item++){
    //   display.setCursor(0, line);
    //   if (menuitem == item) {
    //     display.setTextColor(BLACK, WHITE);
    //     }
    //   else {
    //   display.setTextColor(SSD1306_WHITE);  
    //   }  
    //   display.print(mmArray[item]);
    //   line = line + 10;
    // } 
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
    display.print(rt_cw);
    display.setCursor(65, 25);
    display.print(rt_ccw);
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
  if (rt_left) {
    rt_left = false;
    rt_ccw++;
    rt_inc++;
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

  if (rt_right) {
    rt_right = false;
    rt_cw++;
    rt_inc--;
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

  if (bt_shortpress) {
    bt_shortpress = false;
    switch(page){
      case 1:
      switch(menuitem){
        case 1:
        page = 4;
        break;

        case 2:
        page = 2;
        rt_cw = 0;
        rt_ccw = 0;
        rt_inc = 0;
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

void Eventhandling_new(){
  // looks for actions detected by ISR
  // in case of btpress time saved
  if (bt_press) {
    bt_timepressed = millis();
    bt_press = false;
    tone(8,1000,50);
  }
  
  // in case of bt_releas delta T is calculated 
  // short- and longpress are determinated
  if (bt_releas) {
    bt_timereleased = millis();
    bt_releas = false;
    bt_deltatime = bt_timereleased - bt_timepressed;
    if (bt_deltatime < 200) {
      bt_shortpress = true;
      bt_longpress = false;
    }
    else if(bt_deltatime > 250) {
      bt_shortpress = false;
      bt_longpress = true;
    }
  }
}

void statecontrol(){
  switch(state){
    // display clock shortpress calls Mainmenu  
    case 1: 
    // display clock shortpress calls Mainmenu  
    if (bt_shortpress){
      state = 2;
      bt_shortpress = false;
      run = false;
    }
    break;

    case 2:
    // display Mainmenu  
    if (bt_shortpress){
      state = 1;
      bt_shortpress = false;
      //digit = 1;
      Serial.println(state);
    } 
    if (bt_longpress){
      state = 3;
      page = 4;
      bt_longpress = false;
      //digit = 1;
      Serial.println(state);
    } 
    if (rt_right){
      rt_right = false;
      menuitem ++;
      if (menuitem == 5) { menuitem = 0; }
    }
    if (rt_left){
      rt_left = false;
      menuitem --;
      if (menuitem == -1) { menuitem = 4; }
    }    
    break;

    case 3:  // Clock stopped Setting cl_secz digit 1
    if (rt_right) {
      rt_right = false;
      //cl_secz --;  // secundenzehner runterzählen
      //if (cl_secz < 0) {cl_secz = 5; }
    }
    if (rt_left) {
      rt_left = false;
      //cl_secz ++;  // secundenzehner hochzählen
      //if (cl_secz == 6) {cl_secz = 0;} 
    }     
    if (bt_shortpress){
      state = 4;
      bt_shortpress = false;
      //digit = 2;
      Serial.println(state);      
    }  
    break;

    case 4:  // Clock stopped Setting cl_mine digit 2
    if (rt_right) {
      rt_right = false;
      //cl_mine --;
      //if (cl_mine < 0) {cl_mine = 9;}  
    } 
    if (rt_left) {
      rt_left = false;
      //cl_mine ++;  // secundeneiner hochzählen
      //if (cl_mine == 10) {cl_mine = 0;} 
    }             
    if (bt_shortpress){
      state = 5;
      bt_shortpress = false;
      //digit = 3;
      Serial.println(state);      
    }    
    break;

    case 5:  // Clock stopped Setting cl_minz digit 3
    if (rt_right) {
      rt_right = false;
      //cl_minz --;
      //if (cl_minz < 0) {cl_minz = 5;} 
    }   
    if (rt_left) {
      rt_left = false;
      //cl_minz ++;  // secundeneiner hochzählen
      //if (cl_minz == 6) {cl_minz = 0;} 
    }          
    if (bt_shortpress){
      state = 6;
      bt_shortpress = false;
      //digit = 4;
      Serial.println(state);      
    }    
    break;

    case 6:  // Clock running wait for Alarm reached
    run = true;
    if (bt_longpress){
      state = 1;
      run = false;
    }
    //if (cl_sece == 0 && cl_secz == 0  && cl_mine == 0 && cl_minz == 0){
      run = false;
      //bz_state = true;
      if (bt_shortpress){
        state = 1;
        //bz_state = false;
        //bz_activ = false;
      }
    }
  if (bt_shortpress){
    bt_shortpress = false;
    Serial.println(state); 
  }  
  }   

