
#include <FastLED.h>
#include <LowPower.h>
#define NUM_LEDS 12

#define DATA_PIN 10   //led data
int EN_PIN = 11;      // mosfet trig
int SHUTDN_PIN = 14;  // from charge to shut off leds
int INT_PIN = 2;      //trembler
int HEARTBEAT_PIN = 13;
volatile bool motionDetected = false;
unsigned long lastMotionTime = 0;
const unsigned long sleepTimeout = 180000;  // 180 secs
bool sleep = 0;

CRGB leds[NUM_LEDS];
uint8_t hue = 0;
int i;
int pattNo = 1;
unsigned long lastPattChange = 0;
const unsigned long PattTo = 30000;
int noPatts = 6;
int oldled;
unsigned long lastheartbeat = 0;
const unsigned long heartbeat = 1000;
bool charging = 0;


void setup() {
  pinMode(INT_PIN, INPUT_PULLUP);  // Assuming trembler switch on pin 2
  pinMode(EN_PIN, OUTPUT);         // MOSFET LED POWER TRIGGER
  pinMode(SHUTDN_PIN, INPUT);      // SHUTDOWN LED WHEN CHARGING
  //pinMode(INT_PIN, INPUT_PULLUP);
  pinMode(8, INPUT);  // RESET PASSES THROUGH PIN 8
  attachInterrupt(digitalPinToInterrupt(2), motionISR, CHANGE);
  lastMotionTime = millis();

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  randomSeed(analogRead(A0));
  Serial.begin(19200);
  Serial.println("LED glass");
  pattNo = 1;
  Serial.print("Pattern ");
  Serial.println(pattNo);
}

void allOut() {
  int ii;
  for (ii = 0; ii < 12; ii++) {
    leds[ii] = CHSV(hue, 255, 0);
  }
  FastLED.show();
}

void swirlRB() { //pattern 1
  for (i = 0; i < 12; i++) {
    leds[i] = CHSV(hue += 20, 255, 255);
    //Serial.println(hue);
  }
  delay(50);

  FastLED.show();
}
void slowSwirlRB() { //pattern 2
  for (i = 0; i < 12; i++) {
    leds[i] = CHSV(hue += 1, 255, 255);
    FastLED.show();
    delay(100);
    //checkAlive();
    checkCharge();
  }
}

void chaseRB() {//pattern 3
  int j;
  int k;
  for (k = 0; k < 3; k++) {
    for (i = 0; i < 12; i++) {
      if (i > 0) j = i - 1;
      else j = 11;
      leds[i] = CHSV(hue, 255, 255);
      leds[j] = CHSV(hue, 255, 0);
      FastLED.show();
      delay(80);
      //checkAlive();
      checkCharge();
    }
    hue += 8;
  }
  for (k = 0; k < 3; k++) {
    for (i = 0; i < 12; i++) {
      if (i > 0) j = i - 1;
      else j = 11;
      leds[11 - i] = CHSV(hue, 255, 255);
      leds[11 - j] = CHSV(hue, 255, 0);
      FastLED.show();
      delay(80);
      //checkAlive();
      checkCharge();
    }
    hue += 8;
  }
}
void colourRoll() {//pattern 4
  for (i = 0; i < 12; i++) {
    leds[i] = CHSV(hue, 255, 255);
  }
  FastLED.show();
  delay(130);
  hue += 1;
}
void randomLED() {//pattern 5
  leds[random(0, 12)] = CHSV(random(0, 255), 255, 255);
  FastLED.show();
  delay(random(30, 250));
}
void randomLED2() {//pattern 6
static uint8_t baseCol =160;
  int rndled = random(0, 12);
  leds[oldled] = CHSV(baseCol+=2, 255, 50);
  leds[rndled] = CHSV(0, 0, 255);
  oldled = rndled;
  FastLED.show();
  delay(random(30, 150));
}

void checkAlive() {
  digitalWrite(HEARTBEAT_PIN, 0);
  if (millis() - lastheartbeat > heartbeat) {
    lastheartbeat = millis();
    digitalWrite(HEARTBEAT_PIN, 1);
  }
}
void checkCharge() {

  if (digitalRead(SHUTDN_PIN) == 1) {
    allOut();
    digitalWrite(EN_PIN, 0);
    if (charging == 0) {
      Serial.println("charging ");
      charging = 1;
    }

  } else {
    digitalWrite(EN_PIN, 1);
    if (charging == 1) {
      Serial.println("off charge");
      charging = 0;
    }
  }
}


void loop() {
  checkCharge();
  //checkAlive();
  if (millis() - lastPattChange > PattTo) {
    pattNo++;
    if (pattNo > noPatts) pattNo = 1;
    Serial.print("Pattern ");
    Serial.println(pattNo);

    lastPattChange = millis();
  }
  //pattNo = 6;
  if (!charging) {
    switch (pattNo) {
      case 1:
        swirlRB();
        break;

      case 2:
        slowSwirlRB();
        break;

      case 3:
        chaseRB();
        break;

      case 4:
        colourRoll();
        break;

      case 5:
        randomLED();
        break;

      case 6:
        randomLED2();
        break;

      default:
        colourRoll();
        break;
    }
  }

  if (motionDetected) {
    motionDetected = false;
    lastMotionTime = millis();
    if (sleep) {
      sleep = 0;
      Serial.println("wake");
      delay(200);
    }
    digitalWrite(EN_PIN, 1);

    // Perform tasks (e.g., light up Neopixel ring, run logic)
  }

  if (millis() - lastMotionTime > sleepTimeout) {
    allOut();
    goToSleep();
  }
}
void motionISR() {
  motionDetected = true;
}

void goToSleep() {
  digitalWrite(EN_PIN, 0);
  digitalWrite(13, 0);
  sleep = 1;
  Serial.println("asleep");
  delay(1000);

  // Turn off peripherals if needed
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}
