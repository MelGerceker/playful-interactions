#include "Modulino.h"

ModulinoMovement movement;

const int b1Pin = 2; // Battery LED 1
const int b2Pin = 3; // Battery LED 2
const int b3Pin = 4; // Battery LED 3

const int v1Pin = 5; // Vmotor 1
const int v2Pin = 6; // Vmotor 2

const int dataPin = 8;   // DS
const int latchPin = 12; // ST_CP
const int clockPin = 13; // SH_CP

// Defining the variable and functions to operate the radar
uint16_t radar = 0; // 16 bits for 2 shift registers (8 LEDs × 2 outputs per LED)

#define SET_RADAR(led)   (radar |= (1 << led)) // Turn on specific radar LED
#define CLEAR_RADAR(led) (radar &= ~(1 << led)) // Turn off specific radar LED
#define TURN_ALL_RED() (radar = 0x5555)  // Turn all radar LEDs red
#define TURN_ALL_GREEN() (radar = 0xAAAA)  // Turn all radar LEDs green
#define TURN_OFF_RADAR() (radar = 0) // Turn off radar

float x, y;
const float a = 0.2;
float current_dot_product;

bool Target_is_Hit = false;
int battery_life = 3; //0: empty, 1: 1/3, 2: 2/3, 3: full
bool prevHit = false;
unsigned long hitStartTime = 0;
bool timingHit = false;

struct Vec3 {
  float x;
  float y;
};

struct TargetPoint {
  const char* name;
  Vec3 dir;
};

Vec3 current;

// H=HIT, R=RECHARGE
TargetPoint points[] = {
  {"H1", {0.0, 1.0}},
  {"R2", {1.0, 0.0}},
  {"H3", {0.0, -1.0}}

};

float DotProduct(Vec3 a, Vec3 b) {
  return a.x * b.x + a.y * b.y;
}

Vec3 Normalize(Vec3 v){
  float magnitude = sqrt(v.x *v.x + v.y*v.y);

  if (magnitude < 0.0001){
    return v; //avoid division by zero
  }

  Vec3 normalized = {v.x/magnitude, v.y/magnitude};
  return normalized;
}

int Closest_Target_Finder(Vec3 current) {

  float closest_distance = -2;
  int closest_index = -1;

  int pointCount = sizeof(points) / sizeof(points[0]);

  for (int i =0; i<pointCount;i++) {
    //float result = DotProduct(current, points[i].dir);
    Vec3 target = Normalize(points[i].dir);
    float result = DotProduct(current, target);

    if (closest_distance < result) {
      closest_distance = result;
      closest_index = i;
      current_dot_product = result;
    }
  }

  return closest_index;
}

// Sending radar data to microcontrollers
void sendData(uint16_t data) {
  digitalWrite(latchPin, LOW);

  // Send high byte first → second shift register
  shiftOut(dataPin, clockPin, MSBFIRST, (data >> 8) & 0xFF);

  // Then low byte → first shift register
  shiftOut(dataPin, clockPin, MSBFIRST, data & 0xFF);

  digitalWrite(latchPin, HIGH);
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

bool Hit_Calculator(int closest_index) {
  // the dot product being near equal to 1 means we are pointing at the point

  Target_is_Hit = (current_dot_product > 0.97);

  //just started hitting
  if (Target_is_Hit && !timingHit) {
    hitStartTime = millis();
    timingHit = true;
  }

  //stopped hitting so reset timer
  if (!Target_is_Hit) {
    timingHit = false;
    prevHit = false;
    TURN_OFF_RADAR();
    return false;
  }

  // Check how long we've been holding target
  unsigned long elapsed = millis() - hitStartTime;

  int requiredTime = 0;

  if (points[closest_index].name[0] == 'H') {
    requiredTime = 2000; // 2 seconds for hit
    TURN_ALL_RED();
  } else if (points[closest_index].name[0] == 'R') {
    requiredTime = 3000; // 3 seconds for recharge
    TURN_ALL_GREEN();
  }

  Serial.print("Hold time: ");
  Serial.print(elapsed);
  Serial.println(" ms");

  //trigger when held for long enough
  if (elapsed>=requiredTime && !prevHit) {

    if(points[closest_index].name[0]=='H' && battery_life > 0){
      battery_life--;

    }else if(points[closest_index].name[0]=='R' && battery_life<3){
      battery_life++;
    }

    Update_Battery_Life();
    prevHit=true;

  }

  return Target_is_Hit;
}

void UpdateCompass(int closest_index){
  if(closest_index== -1){
    return;
  }

  Vec3 target = Normalize(points[closest_index].dir);
  Vec3 cur = Normalize(current);

  float currX = cur.x;
  float currY = cur.y;
  float tarX = target.x;
  float tarY = target.y;

  float dot=DotProduct(cur,target); //can be set as param if needed
  float cross=currX*tarY - currY*tarX;

  if(dot>0.95){
    return;
  }

  if(fabs(cross)>fabs(dot)){
    if (cross > 0) {
      Serial.println("LEFT");
    } else {
      Serial.println("RIGHT");
    }
  } else {
    if (dot > 0) {
      Serial.println("UP");
    } else {
      Serial.println("DOWN");
    }
  }

}

void setup() {
  Serial.begin(9600);
  Modulino.begin();
  movement.begin();

  // Initialize the first reading
  movement.update();
  x=movement.getX();
  y=movement.getY();

  current = Normalize({x, y});

  // Set battery LED pins as outputs
  pinMode(b1Pin, OUTPUT);
  pinMode(b2Pin, OUTPUT);
  pinMode(b3Pin, OUTPUT);

  // Set Vmotor pins as outputs
  pinMode(v1Pin, OUTPUT);
  pinMode(v2Pin, OUTPUT);

  // Set radar pins as outputs
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  Update_Battery_Life();
  
  TURN_OFF_RADAR();
}

void loop() {
  // Read new movement data from the sensor
  movement.update();

  Vec3 newVec = {movement.getX(), movement.getY()};
  newVec = Normalize(newVec);

  //smoothed values
  x = a * newVec.x + (1 - a) * x;
  y = a * newVec.y + (1 - a) * y;
  current = Normalize({x, y});

  int closest_index = Closest_Target_Finder(current);
  bool is_hit = Hit_Calculator(closest_index);
  UpdateCompass(closest_index);

  sendData(radar);

  Serial.print(" | current: ");
  Serial.print(current.x, 3);
  Serial.print(", ");
  Serial.print(current.y, 3);
  Serial.print(" | Battery life: ");
  Serial.print(battery_life);
  Serial.print(" | Radar: ");
  Serial.print(radar);
  
  if (closest_index != -1) {
    Serial.print(" | Closest point: ");
    Serial.print(points[closest_index].name);
  }

  if(is_hit) {
    Serial.print(" | Target Hit!");
    digitalWrite(v1Pin, HIGH);
    digitalWrite(v2Pin, HIGH);
  } else {
    digitalWrite(v1Pin, LOW);
    digitalWrite(v2Pin, LOW);
  }

  Serial.println();

  if (battery_life == 0) {
    digitalWrite(v1Pin, LOW);
    digitalWrite(v2Pin, LOW);
    TURN_ALL_RED();
    sendData(radar);
    delay(10000);
    TURN_ALL_GREEN();
    sendData(radar);
    delay(10000);
  }
}

