// Pin setup
const int b1Pin = 2;  // Battery LED 1 output pin
const int b2Pin = 3;  // Battery LED 2 output pin
const int b3Pin = 4;  // Battery LED 3 output pin

const int v1Pin = 5;  // Vibration motor 1 output pin
const int v2Pin = 6;  // Vibration motor 2 output pin

int Vlevels[] = {255, 191, 127, 63, 0}; // max, 3/4, 1/2, 1/4, 0

void setup() {
  // Set battery LED pins as output
  pinMode(b1Pin, OUTPUT);
  pinMode(b2Pin, OUTPUT);
  pinMode(b3Pin, OUTPUT);

  // Set Vmotor pins as output
  pinMode(v1Pin, OUTPUT);
  pinMode(v2Pin, OUTPUT);
}

void loop() {
  // Vibration levels (0-255)
  int stepTime = 2000; // 2 seconds per level (milliseconds)
  
  for (int i = 0; i < 5; i++) {
    // Set battery LEDs
    if (i == 0) {
      digitalWrite(b1Pin, LOW);
      digitalWrite(b2Pin, LOW);
      digitalWrite(b3Pin, LOW);
    } else if (i == 1) {
      digitalWrite(b1Pin, HIGH);
      digitalWrite(b2Pin, LOW);
      digitalWrite(b3Pin, LOW);
    } else if (i == 2) {
      digitalWrite(b1Pin, LOW);
      digitalWrite(b2Pin, HIGH);
      digitalWrite(b3Pin, LOW);
    } else if (i == 3) {
      digitalWrite(b1Pin, LOW);
      digitalWrite(b2Pin, LOW);
      digitalWrite(b3Pin, HIGH);
    } else {
      digitalWrite(b1Pin, HIGH);
      digitalWrite(b2Pin, HIGH);
      digitalWrite(b3Pin, HIGH);
    }

    // Set Vmotors
    analogWrite(v1Pin, Vlevels[i]);
    analogWrite(v2Pin, Vlevels[i]);
    delay(stepTime);                // Wait 2 seconds
  }
}