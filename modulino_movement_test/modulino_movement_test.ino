#include "Modulino.h"

const int b1Pin = 2; // Battery LED 1
const int b2Pin = 3; // Battery LED 2
const int b3Pin = 4; // Battery LED 3

const int dataPin = 8;   // DS
const int latchPin = 12; // ST_CP
const int clockPin = 13; // SH_CP

uint16_t radar = 0; // 16 bits for 2 shift registers (8 LEDs × 2 outputs per LED)

// Create a ModulinoMovement
ModulinoMovement movement;

float x, y, z;
float roll, pitch, yaw;
const float a = 0.2; //smoothing factor
bool Target_is_Hit = false;
float current_dot_product;

int battery_life = 1; //0: empty, 1: 1/3, 2: 2/3, 3: full

struct Vec3 {
  float x;
  float y;
  float z;
};

struct TargetPoint {
  const char* name;
  Vec3 dir;
};

TargetPoint points[] = {
  {"P1", {0.0, 0.0, 1.0}},
  {"P2", {1.0, 0.0, 0.0}}
};

float DotProduct(Vec3 a, Vec3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

int Closest_Target_Finder(Vec3 current) {

  float closest_distance = -2;
  int closest_index = -1;

  int pointCount = sizeof(points) / sizeof(points[0]);

  for (int i =0; i<pointCount;i++) {
    float result = DotProduct(current, points[i].dir);

    if (closest_distance < result) {
      closest_distance = result;
      closest_index = i;
      current_dot_product = result;
    }
  }

  return closest_index;
}

void Update_Battery_Life() {
  // for now the target points are all identified as damage points
  //later on battery should behave differently if the target point is a recharge point

  switch (battery_life)
  {
  case 0:
    // ALL 3 LEDS OFF
    digitalWrite(b1Pin, LOW);
    digitalWrite(b2Pin, LOW);
    digitalWrite(b3Pin, LOW);
    break;

  case 1:
    // 1 LED ON
    digitalWrite(b1Pin, HIGH);
    digitalWrite(b2Pin, LOW);
    digitalWrite(b3Pin, LOW);
    break;

  case 2:
    // 2 LEDS ON
    digitalWrite(b1Pin, HIGH);
    digitalWrite(b2Pin, HIGH);
    digitalWrite(b3Pin, LOW);
    break;
  
  case 3:
    // ALL 3 LEDS ON
    digitalWrite(b1Pin, HIGH);
    digitalWrite(b2Pin, HIGH);
    digitalWrite(b3Pin, HIGH);
    break;
    
  default:
    Serial.println("Error: Invalid battery life state");
    break;
  }

}

bool Hit_Calculator() {
  // the dot product being near equal to 1 means we are pointing at the point
  // so this method could be used to say if more than 0.95 point is hit

  Target_is_Hit = (current_dot_product > 0.95);

  // implement check for damage vs recharge point here
  // for now we assume all points are damage points
  if (Target_is_Hit && battery_life > 0) {
    battery_life--;
    Update_Battery_Life();
    // test that this doesnt cause the battery life to decrease by more than 1 per hit bc of the loop?
  }

  return Target_is_Hit;
}

void setup() {
  Serial.begin(9600);
  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to movement sensor module
  movement.begin();

  // Initialize the first reading
  movement.update();
  x=movement.getX();
  y=movement.getY();
  z=movement.getZ();

  // Set battery LED pins as outputs
  pinMode(b1Pin, OUTPUT);
  pinMode(b2Pin, OUTPUT);
  pinMode(b3Pin, OUTPUT);

  // Set 
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
}

void loop() {
  // Read new movement data from the sensor
  movement.update();

  //smoothed values
  x = a*movement.getX() + (1-a)*x;
  y = a*movement.getY() + (1-a)*y;
  z = a*movement.getZ() + (1-a)*z;

  // Temporary current vector
  Vec3 current = {x, y, z};

  int closest_index = Closest_Target_Finder(current);
  bool is_hit = Hit_Calculator(); //already calls Update_Battery_Life() if hit is true

/*
  // Get acceleration and gyroscope values
  x = movement.getX();
  y = movement.getY();
  z = movement.getZ();
  roll = movement.getRoll();
  pitch = movement.getPitch();
  yaw = movement.getYaw();
  */

  // Print acceleration values
  Serial.print("A: ");
  Serial.print(x, 3);
  Serial.print(", ");
  Serial.print(y, 3);
  Serial.print(", ");
  Serial.print(z, 3);
  
  // Print divider between acceleration and gyroscope
  Serial.print(" | G: ");
  
  /*
  // Print gyroscope values
  Serial.print(roll, 1);
  Serial.print(", ");
  Serial.print(pitch, 1);
  Serial.print(", ");
  Serial.println(yaw, 1);
  */

  if (closest_index != -1) {
    Serial.println(" | Closest point: ");
    Serial.println(points[closest_index].name);
  }

    if(is_hit) {
    Serial.println(" | Target Hit!");
  }

  
  delay(200);
}

