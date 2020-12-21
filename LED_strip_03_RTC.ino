/*   5/8/2020
 *   Program by Gabriel Winkler
 *   
 *   This program uses the DS1307 real time clock to control two sets of LED strips.
 *   One set will fade up and down in brightness while the other will come on instantly.
 *   The two strips are triggered by a pair of PIR sensors, one for each strip.
*/
#include<Wire.h>                // This will be used by the Real Time Clock
#include<TimeLib.h>             // This too...
#include<RTClib.h>              // ...and this.

// Defining the pins for the two strips
#define HALL_LED 9
#define CLOSET_LED 6

int calTime = 20;               // This is the time we allow the sensors to calibrate
long unsigned int lowIn;        // This is the number of milliseconds the sensor must be LOW before
                                //  we assume motion has stopped
long unsigned int pause = 5000; // Our default pause time, 5 seconds.
boolean lockLow = true;         
boolean lockLow2 = true;        // Needed to separate the lockLow variables so the lights will turn off correctly
boolean takeLowTime;
boolean takeLowTime2;
int pirHallPin = 10;            // the pin that the PIR sensor is connected to for the hall light
                                // Pin 11 will be always on but very dim due to a small draw in amps...
int pirClosetPin = 11;          // the pin that the PIR sensor is connected to for the closet light

// Setting the variables for brightness
int bright = 255;
int nightBright = 20;
// Individual strip brightness values
int hallBright = 0;
int closetBright = 0;
int nightHallBright = 0;
int nightClosetBright = 0;

// Fade speed control. Higher numbers mean faster fading.
int fadeSpd = 10;
int nightFadeSpd = 2;

// For real-time clock:
RTC_DS1307 rtc;

void setup() {
  // This section is for the PIR sensor
  Serial.begin(9600); // Don't forget to set the serial monitor to this value...
  if(!rtc.begin()){
    Serial.println("RTC not working.");
    while(true);
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  pinMode(pirHallPin, INPUT);
  pinMode(pirClosetPin, INPUT);
  digitalWrite(pirHallPin, LOW);
  digitalWrite(pirClosetPin, LOW);
  //  Give the sensor time to calibrate...
  Serial.print("Calibrating sensor... ");
  for(int i = 0; i < calTime; i++){
    Serial.print("-");
    delay(1000);
  }
  Serial.println(" done.");
  Serial.println("*** SENSOR ACTIVE ***");
  delay(50);

  
  // Setting pins for output
  pinMode(HALL_LED, OUTPUT);
  pinMode(CLOSET_LED, OUTPUT);
//  pinMode(BUTTON, INPUT);
}

void TurnOnHall(){
  for (int i=0; i < 256; i++){
    analogWrite(HALL_LED, hallBright);
    hallBright += 1;
    delay(fadeSpd);
  }
  Serial.println("Hall Light on.");
}

void TurnOnCloset(){
  for (int i=0; i < 256; i++){
    analogWrite(CLOSET_LED, closetBright);
    closetBright += 1;
    delay(fadeSpd);
  }
  Serial.println("Closet light on.");
}

void TurnOffHall(){
  for (int i=0; i < 256; i++){
    analogWrite(HALL_LED, bright);
    bright -= 1;
    delay(fadeSpd);
  }
  Serial.println("Hall light off.");
}

void TurnOffCloset(){
  for (int i=0; i < 256; i++){
    analogWrite(CLOSET_LED, bright);
    bright -= 1;
    delay(fadeSpd);
  }
  Serial.println("Closet light off.");
}

void TurnOnHallNight() {
  for (int i=0; i < 20; i++){
    analogWrite(HALL_LED, nightHallBright);
    nightHallBright += 1;
    delay(nightFadeSpd);
  }
  nightHallBright = 0;
}

void TurnOnClosetNight(){
  for (int i=0; i < 20; i++){
    analogWrite(CLOSET_LED, nightClosetBright);
    nightClosetBright += 1;
    delay(nightFadeSpd);
  }
  nightClosetBright = 0;
}

void TurnOffHallNight() {
  for (int i=0; i < 20; i++){
    analogWrite(HALL_LED, nightBright);
    nightBright -= 1;
    delay(nightFadeSpd);
  }
  nightBright = 20;
  analogWrite(HALL_LED, 0);
}

void TurnOffClosetNight(){
  for (int i=0; i < 20; i++){
    analogWrite(CLOSET_LED, nightBright);
    nightBright -= 1;
    delay(nightFadeSpd);
  }
  nightBright = 20;
  analogWrite(CLOSET_LED, 0);
}

void TestFlash(){ // This is to help find where in the loop the program is.
  for (int i=0;i < 3;i++){
    analogWrite(HALL_LED, nightBright);
    delay(300);
    analogWrite(HALL_LED, 0);
    analogWrite(CLOSET_LED, nightBright);
    delay(300);
    analogWrite(CLOSET_LED, 0);
    delay(100);
  }
}

void loop() {
  // This will get the hour of the day from the RTC module
  DateTime now = rtc.now();           // Sets the "now" variable to the current time
//  Serial.print("Hour: ");           
//  Serial.println(now.hour(), DEC);  // This lets us see what the hour is reading

// This if statement will use the hour of the day to decide day or night ops
// Changing the high number will adjust the hour when the night settings take over.
// Changing the low number will adjust the hour when the night settings stop.
  if(now.hour() >= 20 || now.hour() <= 9){
      // TurnOnHallNight will be for the hall LED strip
    if(digitalRead(pirHallPin) == HIGH){
      if(lockLow){
        lockLow = false;
        Serial.println("---");
        Serial.print("Night motion detected in the hall at ");
        Serial.print(now.hour(), DEC);
        Serial.print(":");
        Serial.println(now.minute(), DEC);
        delay(50);
        TurnOnHallNight();
      }
      takeLowTime = true;
    }
    if(digitalRead(pirHallPin) == LOW){
      if(takeLowTime){
        lowIn = millis();
        takeLowTime = false;
      }
      if(!lockLow && millis() - lowIn > pause){
        lockLow = true;
        Serial.print("Night motion ended in the hall at ");
        Serial.print(now.hour(), DEC);
        Serial.print(":");
        Serial.println(now.minute(), DEC);
        
        TurnOffHallNight();
      }
    }
  
   // TurnOnClosetNight will be for the closet LED strip.
    if(digitalRead(pirClosetPin) == HIGH){
      if(lockLow2){
        lockLow2 = false;
        Serial.println("---");
        Serial.print("Night motion detected in the closet at ");
        Serial.print(now.hour(), DEC);
        Serial.print(":");
        Serial.println(now.minute(), DEC);
        delay(50);
        TurnOnClosetNight();
      }
      takeLowTime2 = true;
    }
    if(digitalRead(pirClosetPin) == LOW){
      if(takeLowTime2){
        lowIn = millis();
        takeLowTime2 = false;
      }
      if(!lockLow2 && millis() - lowIn > pause){
        lockLow2 = true;
        Serial.print("Night motion ended in the closet at ");
        Serial.print(now.hour(), DEC);
        Serial.print(":");
        Serial.println(now.minute(), DEC);
        TurnOffClosetNight();
      }
    }
  }
// Below this should all be for daytime running...
  else{ 
      // TurnHallOn will be for the hall, and should be called in the first two "if" statements.
    if(digitalRead(pirHallPin) == HIGH){
      if(lockLow){
        lockLow = false;
        Serial.println("---");
        Serial.print("Motion detected in the hall at ");
        Serial.print(now.hour(), DEC);
        Serial.print(":");
        Serial.println(now.minute(), DEC);
        delay(50);
        TurnOnHall();
      }
      takeLowTime = true;
    }
    if(digitalRead(pirHallPin) == LOW){
      if(takeLowTime){
        lowIn = millis();
        takeLowTime = false;
      }
      if(!lockLow && millis() - lowIn > pause){
        lockLow = true;
        Serial.print("Motion ended in the hall at ");
        Serial.print(now.hour(), DEC);
        Serial.print(":");
        Serial.println(now.minute(), DEC);
        TurnOffHall();
      }
    }
  
   // TurnClosetOn will be for the closet, and should be called in the second two "if" statements.
    if(digitalRead(pirClosetPin) == HIGH){
      if(lockLow2){
        lockLow2 = false;
        Serial.println("---");
        Serial.print(now.hour(), DEC);
        Serial.print(":");
        Serial.println(now.minute(), DEC);
        delay(50);
        TurnOnCloset();
      }
      takeLowTime2 = true;
    }
    if(digitalRead(pirClosetPin) == LOW){
      if(takeLowTime2){
        lowIn = millis();
        takeLowTime2 = false;
      }
      if(!lockLow2 && millis() - lowIn > pause){
        lockLow2 = true;
        Serial.print("Motion ended in the closet at ");
        Serial.print(now.hour(), DEC);
        Serial.print(":");
        Serial.println(now.minute(), DEC);
        TurnOffCloset();
      }
    }
  }
}
