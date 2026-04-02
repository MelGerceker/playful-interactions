#include <Servo.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position

const int dataPin = 8;   // DS
const int latchPin = 12; // ST_CP
const int clockPin = 13; // SH_CP

uint16_t leds = 0; // 16 bits for 2 shift registers (8 LEDs × 2 outputs per LED)

void setup() {
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  myservo.attach(10);  // attaches the servo on pin 9 to the servo object
}

void sendData(uint16_t data) {
  digitalWrite(latchPin, LOW);

  // Send high byte first → second shift register
  shiftOut(dataPin, clockPin, MSBFIRST, (data >> 8) & 0xFF);

  // Then low byte → first shift register
  shiftOut(dataPin, clockPin, MSBFIRST, data & 0xFF);

  digitalWrite(latchPin, HIGH);
}

void loop() {
  myservo.write(90);

  // Red from left to right
  for (int i = 0; i < 8; i++) {
    leds = 0;                  // Clear all LEDs
    leds |= (1 << (i * 2));    // Turn on RED for LED i
    sendData(leds);
    delay(1000);
  }

  // All OFF
  sendData(0);
  myservo.write(180);
  delay(1000);

  // Radar effect on final LED
  uint16_t finalRed = (1 << (7 * 2)); // bit 14
  uint16_t blink = 0;
  int steps = 10;
  float x0 = 1000;
  float x_end = 50;  // almost zero
  float r = pow(x_end / x0, 1.0 / steps);

  for (int i = steps; i >= 0; i--) {
    float x = x0 * pow(r, steps - i);
    sendData(finalRed);
    delay(x);
    sendData(blink);
    delay(x);
  }

  // All red ON
  leds = 0;
  for (int i = 0; i < 8; i++) {
    leds |= (1 << (i * 2)); // red bits
  }
  sendData(leds);
  myservo.write(90);
  delay(5000);

  // All OFF
  sendData(0);
  myservo.write(0);
  delay(5000);

  // Green from left to right
  for (int i = 0; i < 8; i++) {
    leds = 0;
    leds |= (1 << (i * 2 + 1)); // green bits
    sendData(leds);
    delay(1000);
  }
}