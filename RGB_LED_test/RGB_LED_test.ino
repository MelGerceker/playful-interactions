int redPin1 = 8;
int greenPin1 = 10;
int redPin2 = 9;
int greenPin2 = 11;
int redPin3 = 13;
int greenPin3 = 12;

void setup() {
  pinMode(redPin1, OUTPUT);
  pinMode(greenPin1, OUTPUT);
  pinMode(redPin2, OUTPUT);
  pinMode(greenPin2, OUTPUT);
  pinMode(redPin3, OUTPUT);
  pinMode(greenPin3, OUTPUT);

  digitalWrite(redPin1, LOW);
  digitalWrite(greenPin1, LOW);
  digitalWrite(redPin2, LOW);
  digitalWrite(greenPin2, LOW);
  digitalWrite(redPin3, LOW);
  digitalWrite(greenPin3, LOW);
}

void loop() {
  // PINGING
  int i = 0;
  while (i > 0) {
    digitalWrite(redPin3, HIGH);
    delay(1000);
    digitalWrite(redPin3, LOW);
    delay(1000);
    i -= 1;
  }

  // DISTANCE DECREASING
  int steps = 30;
  float x0 = 1000;
  float x_end = 10;  // almost zero
  float r = pow(x_end / x0, 1.0 / steps);  // compute decay factor

  for (i = steps; i >= 0; i--) {
    float x = x0 * pow(r, steps - i);
    digitalWrite(redPin3, HIGH);
    delay(x);
    digitalWrite(redPin3, LOW);
    delay(x);
  }

  // CAUGHT
  digitalWrite(redPin1, HIGH);
  digitalWrite(redPin2, HIGH);
  digitalWrite(redPin3, HIGH);
  delay(5000);

  digitalWrite(redPin1, LOW);
  digitalWrite(redPin2, LOW);
  digitalWrite(redPin3, LOW);
  delay(2000);

  // GOOD PINGING
  i = 10;
  while (i > 0) {
    digitalWrite(greenPin2, HIGH);
    delay(1000);
    digitalWrite(greenPin2, LOW);
    delay(1000);
    i -= 1;
  }
}