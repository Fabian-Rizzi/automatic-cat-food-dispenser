// Time settings, an array of arrays containing 2 variables corresponding to H, MM
// {5, 03} will feed 3 minutes after 5 AM
// {17, 58} will feed 2 minutes before 6 PM   

byte feedHourMin[][2] = {
  {5, 01},      
  {9, 02},
  {13, 03},
  {17, 04},
  {22, 05},
};


// Motor Settings

#define FEED_SPEED 4000
#define STEPS_FRW 2
#define STEPS_BKW 1 

// Feeding Settings

int feedAmount = 10;
int newAmount = 0;
int feedTime = 4;
int feedingSetTime = 4; // feedTime/2 seems to be work 

// Libraries

#include "EncButton.h"
#include <EEPROM.h>
#include <Wire.h>
#include <iarduino_RTC.h>   

// Input/Output Assignations 

#define EE_RESET 12
#define BTN 7
#define STEP 3
#define DIR 4
#define ENABLE 5
#define TGL 6
#define BUZZER 13


// Select RTC Clock model

iarduino_RTC watch(RTC_DS3231);    

// Buttons Assignation (for debouncing)

EncButton<EB_TICK, BTN> btn;
EncButton<EB_TICK, TGL> tgl;


void setup() {  
    delay(300);   
    Serial.begin(9600);
    Wire.begin();
    delay(4000);
    watch.begin(); 
    if (EEPROM.read(0) != EE_RESET) {
        EEPROM.write(0, EE_RESET);
        EEPROM.put(1, feedAmount);
    }
    EEPROM.get(1, feedAmount);
    pinMode(STEP, OUTPUT);  
    pinMode(DIR, OUTPUT); 
    pinMode(ENABLE, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    beep(1);
}

// Used for debugging

void printTime() {

  watch.gettime();
  

// Concatenated output
  Serial.print(watch.hours); 
  Serial.print(":");
  Serial.print(watch.minutes);
  Serial.print(":"); 
  Serial.print(watch.seconds);
  Serial.print(" "); 
  Serial.print(watch.weekday);
  Serial.print(" "); 
  Serial.print(watch.day);
  Serial.print("/"); 
  Serial.print(watch.month); 
  Serial.print("/"); 
  Serial.print(watch.year);
  Serial.println("."); 


//  Serial.print(":");
//  Serial.print(now.minute);
//  Serial.print(":");
//  Serial.print(now.second);
//  Serial.print(" ");
//  Serial.print(now.day);
//  Serial.print(" ");
//  Serial.print(now.date);
//  Serial.print("/");
//  Serial.print(now.month);
//  Serial.print("/");
//  Serial.println(now.year);

}

// Receives X as parameter which is the amount of beeps.
// Default time between beeps is 50ms

// Might set it as variable to allow customization.

void beep(int x){
  digitalWrite(BUZZER, HIGH);
  delay(100);
  for(int i = 0; i < x; i++){     
    digitalWrite(BUZZER, HIGH);
    delay(50);
    digitalWrite(BUZZER, LOW);
    delay(50);
  } 
}

// Feeding function with anti-clog system
// Goes forward-forward-backwards, forward-forward-backwards, in a loop
// Portion size is set in the EEPROM by changing feedAmount either by code or by holding BTN.
// Release BTN once the dispensed food matches the desired portion size.

void fwd(){
  digitalWrite(DIR, HIGH);    // Clockwise rotation  
  digitalWrite(STEP, HIGH);       // Sends pulse to spin a step forward
  delay(feedTime);          
  digitalWrite(STEP, LOW);        
  delay(feedTime);
}

void bwd(){
  digitalWrite(DIR, LOW); // Counter clockwise rotation
  delay(feedTime);  
  digitalWrite(STEP, HIGH); // Sends pulse to spin a step back
  delay(feedTime);
  digitalWrite(STEP, LOW);
  delay(feedTime);  
}

void feed(){  
  Serial.print("feeding");

  // Activates motor
  digitalWrite(ENABLE, LOW);
  //digitalWrite(DIR, HIGH); 

  for(int i = 0; i < feedAmount; i++){
  fwd();
  fwd();
  fwd();
  fwd();
  fwd();
  bwd();
  }
  delay(feedTime);
  printTime();
  }

void loop() {
  static uint32_t tmr = 0;
  watch.gettime();
  if (millis() - tmr > 500) {           // 
    static byte prevMin = 0;
    tmr = millis();
//  Serial.println(watch.minutes);
    if (prevMin != watch.minutes) {
//    Serial.println("inside if");
      prevMin = watch.minutes;
      for (byte i = 0; i < sizeof(feedHourMin) / 2; i++) {  // take out {
        if (feedHourMin[i][0] == watch.Hours && feedHourMin[i][1] == watch.minutes) { 
          beep(5);
          delay(100);
          beep(7);
          feed();
        }} // take out { if it does not run and also above one
    }
  }
  btn.tick();
  tgl.tick();
  if (btn.click()) {
    beep(4);
    feed();
  }
  if (tgl.hold()) {
    feed();
  }
  if (btn.hold()) {
    newAmount = 0;
  //Serial.println("Starting hold");
    while (btn.isHold()){
      //Serial.println("HODLING");
      btn.tick();
      oneRev();
      newAmount++;
      Serial.print(newAmount);
}
  // Stores portion size into EEPROM, location 1, and prints it in serial 
  feedAmount = newAmount;
  EEPROM.put(1, feedAmount);
  Serial.println(feedAmount);
} 
  // Turns off motor to save energy 
  // VERY IMPORTANT !
  digitalWrite(ENABLE, HIGH);
  
  // For debugging
  // delay(2000);
}


// Sets the portion size depending on the time spent holding BTN
// Slower than feed() in order to increase portion size resolution and accuracy when setting

void oneRev() {
  digitalWrite(ENABLE, LOW);    // Turns on motor
  digitalWrite(DIR, HIGH);    // Clockwise rotation
  // 400 steps for a 0.9 degree rotation angle to make a full spin.

  for(int i = 0; i < 2; i++){ // 2 Steps forward
    digitalWrite(STEP, HIGH);
    delay(feedingSetTime);    
    digitalWrite(STEP, LOW); 
    delay(feedingSetTime);    
  }

  digitalWrite(DIR, LOW);
  for(int i = 0; i < 1; i++){     // One step back
    digitalWrite(STEP, HIGH);
    delay(feedingSetTime);    
    digitalWrite(STEP, LOW); 
    delay(feedingSetTime);    
  }
}
