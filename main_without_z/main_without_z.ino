#include "Modulino.h"
#include "Servo.h"

uint16_t radar = 0; // 16 bits for 2 shift registers (8 LEDs × 2 outputs per LED)


#include "SequencePlayer.h"
SequencePlayer sequencePlayer(&radar);

/*
Step introSequence[] = {
  {DIR_UP, 2000},
  {DIR_LEFT, 3000},
  {DIR_RIGHT, 1000},
  {DIR_DOWN, 2000}
};
*/

ModulinoMovement movement;
Servo myservo;

const int b1Pin = 2; // Battery LED 1
const int b2Pin = 3; // Battery LED 2
const int b3Pin = 4; // Battery LED 3

const int v1Pin = 5; // Vmotor 1
const int v2Pin = 6; // Vmotor 2

const int dataPin = 8;   // DS
const int latchPin = 12; // ST_CP
const int clockPin = 13; // SH_CP

int led=10;

#define SET_RADAR(led)   (radar |= (1 << led)) // Turn on specific radar LED
#define CLEAR_RADAR(led) (radar &= ~(1 << led)) // Turn off specific radar LED
#define TURN_ALL_RED() (radar = 0x5555)  // Turn all radar LEDs red
#define TURN_ALL_GREEN() (radar = 0xAAAA)  // Turn all radar LEDs green
#define TURN_OFF_RADAR() (radar = 0) // Turn off radar

int servoPos = 60;
//0: hit, 60: neutral, 120: recharge, 180: win state

float x, y;
const float a = 0.2;
float current_dot_product;

bool Target_is_Hit = false;
int battery_life = 3; // init 1 led on 1 led flickering
bool prevHit = false;
bool Win_State=false;

unsigned long hitStartTime = 0;
bool timingHit = false;

unsigned long lastFlickerTime = 0;
bool flickerState = false;
const int flickerInterval = 500;

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
  {"H3", {0.0, -1.0}},
  {"R4", {0.5, 0.0}}

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

void Update_Flicker() {
  unsigned long currentTime = millis();

  if(currentTime-lastFlickerTime>=flickerInterval){
    flickerState = !flickerState; 
    lastFlickerTime = currentTime;
  }
}

void Update_Battery_Life() {

  switch (battery_life)
  {
  case 0:
    // ALL 3 LEDS OFF
    digitalWrite(b1Pin, LOW);
    digitalWrite(b2Pin, LOW);
    digitalWrite(b3Pin, LOW);
    break;
  
  case 1:
    //1 LED FLICKERING
    digitalWrite(b2Pin, LOW);
    digitalWrite(b3Pin, LOW);
    digitalWrite(b1Pin, flickerState); //FLICKER HERE
    break;

  case 2:
    // 1 LED ON
    digitalWrite(b1Pin, HIGH);
    digitalWrite(b2Pin, LOW);
    digitalWrite(b3Pin, LOW);
    break;
  
  case 3:
    // 1 LED ON 1 FLICKERING
    digitalWrite(b1Pin, HIGH);
    digitalWrite(b3Pin, LOW);

    digitalWrite(b2Pin, flickerState); //FLICKER HERE
    break;

  case 4:
    // 2 LEDS ON
    digitalWrite(b1Pin, HIGH);
    digitalWrite(b2Pin, HIGH);
    digitalWrite(b3Pin, LOW);
    break;
  
  case 5:
    // 2 LEDS ON 1 FLICKERING
    digitalWrite(b1Pin, HIGH);
    digitalWrite(b2Pin, HIGH);

    digitalWrite(b3Pin, flickerState); //FLICKER HERE
    break;
  
  case 6:
    // ALL 3 LEDS ON
    digitalWrite(b1Pin, HIGH);
    digitalWrite(b2Pin, HIGH);
    digitalWrite(b3Pin, HIGH);
    Win_State=true;
    break;
    
  default:
    Serial.println("Error: Invalid battery life state");
    break;
  }

}

bool Hit_Calculator(int closest_index) {
  // the dot product being near equal to 1 means we are pointing at the point

  //safety check for the setup/no points found
  if (closest_index == -1) {
    Target_is_Hit = false;
    timingHit = false;
    prevHit = false;
    servoPos=60;
    myservo.write(servoPos);
    return false;
  }

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
    //TURN_OFF_RADAR();
    //servo reset
    servoPos=60;
    myservo.write(servoPos); //NEUTRAL ANGLE 60
    return false;
  }

  // Check how long we've been holding target
  unsigned long elapsed = millis() - hitStartTime;

  int requiredTime = 0;

  if (points[closest_index].name[0] == 'H') {
    requiredTime = 2000; // 2 seconds for hit
    //TURN_ALL_RED();
    servoPos=0;
    myservo.write(servoPos); //HIT ANGLE 0

  } else if (points[closest_index].name[0] == 'R') {
    requiredTime = 3000; // 3 seconds for recharge
    //TURN_ALL_GREEN();
    servoPos=120;
    myservo.write(servoPos); //RECHARGE ANGLE 120
  }

  Serial.print("Hold time: ");
  Serial.print(elapsed);
  Serial.println(" ms");

  //trigger when held for long enough
  if (elapsed>=requiredTime && !prevHit) {

    if(points[closest_index].name[0]=='H' && battery_life > 0){
      battery_life--;

    }else if(points[closest_index].name[0]=='R' && battery_life<6){
      battery_life++;
    }

    prevHit=true;
  }

  return Target_is_Hit;
}

String getDirAtanMethod(Vec3 target, Vec3 current) {
  float angleCurrent = atan2(current.y, current.x) * 180.0 / PI;
  float angleTarget  = atan2(target.y, target.x) * 180.0 / PI;
  float angleDiff = angleTarget - angleCurrent;

  // Normalize to [-180, 180]
  while (angleDiff > 180) angleDiff -= 360;
  while (angleDiff < -180) angleDiff += 360;

  if (angleDiff >= -45 && angleDiff < 45) {
    return "UP"; // target is in front
  } 
  else if (angleDiff >= 45 && angleDiff < 135) {
    return "LEFT";
  } 
  else if (angleDiff >= -135 && angleDiff < -45) {
    return "RIGHT";
  } 
  else {
    return "DOWN";
  }
}

String getDirCrossMethod(Vec3 target, Vec3 current) {
  float cross = current.x * target.y - current.y * target.x;
  float dot = DotProduct(current, target);

  if (fabs(cross) > fabs(dot)) {
    if(cross > 0){
      //TURN_OFF_RADAR();
      //TURN_ALL_RED();
      //SET_RADAR(1); //LEFT
      Serial.println("AAAAAAAAAA");
      return "LEFT";

    } else {
      //TURN_OFF_RADAR();
      //SET_RADAR(2); //RIGHT
      return "RIGHT";

    }
  } else {
    if(dot > 0){
      //TURN_OFF_RADAR();
      //SET_RADAR(3); //UP
      return "UP";
    } else {
      //TURN_OFF_RADAR();
      //SET_RADAR(5); //DOWN
      return "DOWN";
    }
  }
}

void UpdateCompass(int closest_index){

  if(closest_index== -1){
    return;
  }

  Vec3 target = Normalize(points[closest_index].dir);
  Vec3 cur = Normalize(current);

  Serial.println(getDirCrossMethod(target, cur));
  //Serial.println(getDirAtanMethod(target, cur));

}

void setup() {
  Serial.begin(9600);
  Modulino.begin();
  movement.begin();

  // Initialize the first reading
  movement.update();

  x = movement.getX();
  y = movement.getY();

  current = Normalize({x, y});
  x = current.x;
  y = current.y;

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

  myservo.attach(9); // Attach servo to pin 9
  myservo.write(servoPos); // Set initial servo position

  Update_Battery_Life();
  //TURN_OFF_RADAR();

  //sequencePlayer.start(introSequence, sizeof(introSequence) / sizeof(introSequence[0]));


}

void loop() {
  movement.update();


  for (int i = 0; i < 16; i++) {
    radar = (1 << i);

    Serial.print("bit ");
    Serial.print(i);
    Serial.print(" -> ");
    Serial.println(radar, BIN);

    sendData(radar);
    delay(1500);
  }

  //sequencePlayer.update();
  
  Vec3 newVec = {movement.getX(), movement.getY()};
  newVec = Normalize(newVec);

  //smoothed values
  x = a * newVec.x + (1 - a) * x;
  y = a * newVec.y + (1 - a) * y;
  current = Normalize({x, y});


  if(!Win_State){
    int closest_index = Closest_Target_Finder(current);
    bool is_hit = Hit_Calculator(closest_index);
    Update_Flicker();
    Update_Battery_Life();
    UpdateCompass(closest_index);
    sendData(radar);  

    Serial.print(" | Current: ");
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
      //TURN_ALL_RED();
      sendData(radar);
      delay(10000);
      //TURN_ALL_GREEN();
      sendData(radar);
      delay(10000);
    }


  } else{
    Serial.println("YOU WIN");
    servoPos=180;
    myservo.write(servoPos); //WIN ANGLE 180
  }


  delay(200);
  
}

