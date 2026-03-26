#include "Modulino.h"

const int b1Pin = 2; // Battery LED 1
const int b2Pin = 3; // Battery LED 2
const int b3Pin = 4; // Battery LED 3

const int dataPin = 8;   // DS
const int latchPin = 12; // ST_CP
const int clockPin = 13; // SH_CP

uint16_t radar = 0; // 16 bits for 2 shift registers (8 LEDs × 2 outputs per LED)

ModulinoMovement movement;

float x, y, z;
float roll, pitch, yaw;
const float a = 0.2;
float current_dot_product;

bool Target_is_Hit = false;
int battery_life = 1; //0: empty, 1: 1/3, 2: 2/3, 3: full
//bool prevHit = false;

struct Vec3 {
  float x;
  float y;
  float z;
};

struct TargetPoint {
  const char* name;
  Vec3 dir;
};

Vec3 current;

// H=HIT, R=RECHARGE
TargetPoint points[] = {
  {"H1", {0.0, 0.0, 1.0}},
  {"R2", {1.0, 0.0, 0.0}},
  {"H3", {0.0, 0.0, 0.7}}

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


//today:
//hit vs. recharge point

//how should hit affect battery
//2 seconds

//how should recharge affect battery
//3 seconds

//LED light guidance
//point to closest point for now
//assume cross shape?


bool Hit_Calculator(int closest_index) {
  // the dot product being near equal to 1 means we are pointing at the point

  Target_is_Hit = (current_dot_product > 0.95);

  if(Target_is_Hit){
  delay(2000);
  }

  // check for damage vs recharge point here
  if (Target_is_Hit && battery_life > 0) {

    if(points[closest_index].name[0]=='H'){
      battery_life--;

    }else if(points[closest_index].name[0]=='R'){
      battery_life++;
    }

    // test that this doesnt cause the battery life to decrease by more than 1 per hit bc of the loop?
    //if so just check that a bool switches
  }

  Update_Battery_Life();

  return Target_is_Hit;
}

void setup() {
  Serial.begin(9600);
  Modulino.begin();
  movement.begin();

  // Initialize the first reading
  movement.update();
  x=movement.getX();
  y=movement.getY();
  z=movement.getZ();

  current = {x, y, z};

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
  Vec3 NewVec = {movement.getX(), movement.getY(), movement.getZ()};
  //smoothed values
  if (DotProduct(current, NewVec) < 0.95){
  x = a*movement.getX() + (1-a)*x;
  y = a*movement.getY() + (1-a)*y;
  z = a*movement.getZ() + (1-a)*z;}
  current=NewVec;


  int closest_index = Closest_Target_Finder(current);
  bool is_hit = Hit_Calculator(closest_index); //already calls Update_Battery_Life() if hit is true


  // Print acceleration values
  Serial.print("A: ");
  Serial.print(x, 3);
  Serial.print(", ");
  Serial.print(y, 3);
  Serial.print(", ");
  Serial.print(z, 3);
  

  if (closest_index != -1) {
    Serial.println(" | Closest point: ");
    Serial.print(points[closest_index].name);
  }

  if(is_hit) {
    Serial.println(" | Target Hit!");
  }

  delay(200);
}

