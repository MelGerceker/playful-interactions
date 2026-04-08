const int dataPin = 8;    // DS
const int latchPin = 12;  // ST_CP
const int clockPin = 13;  // SH_CP



#define SET_RADAR(led)   (radar |= (1 << led)) // Turn on specific radar LED
#define CLEAR_RADAR(led) (radar &= ~(1 << led)) // Turn off specific radar LED
#define TURN_ALL_RED() (radar = 0x5555)  // Turn all radar LEDs red
#define TURN_ALL_GREEN() (radar = 0xAAAA)  // Turn all radar LEDs green
#define TURN_OFF_RADAR() (radar = 0) // Turn off radar

void sendData(uint16_t data) {
  digitalWrite(latchPin, LOW);

  // Send high byte first → second shift register
  shiftOut(dataPin, clockPin, MSBFIRST, (data >> 8) & 0xFF);

  // Then low byte → first shift register
  shiftOut(dataPin, clockPin, MSBFIRST, data & 0xFF);

  digitalWrite(latchPin, HIGH);
}

void sendDataNEW(uint16_t data) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, (data >> 8) & 0xFF);
  shiftOut(dataPin, clockPin, MSBFIRST, data & 0xFF);
  digitalWrite(latchPin, HIGH);
}

void sendData3(uint16_t data) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, (data >> 8) & 0xFF);
  shiftOut(dataPin, clockPin, LSBFIRST, data & 0xFF);
  digitalWrite(latchPin, HIGH);
}

void setup() {
  Serial.begin(9600);

  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  digitalWrite(dataPin, LOW);
  digitalWrite(clockPin, LOW);
  digitalWrite(latchPin, HIGH);
}

void loop() {
  sendData(0x0000);
  Serial.println("0000");
  delay(2000);

  sendData(0xFFFF);
  Serial.println("FFFF");
  delay(2000);

  sendData(0x5555);
  Serial.println("5555");
  delay(2000);

  sendData(0xAAAA);
  Serial.println("AAAA");
  delay(2000);
}