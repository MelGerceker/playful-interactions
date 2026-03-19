#include "Modulino.h"

// Create a ModulinoMovement
ModulinoMovement movement;

float x, y, z;
float roll, pitch, yaw;
const float a = 0.2; //smoothing factor

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

String Compare(Vec3 current) {
  // the dot product being near equal to 1 means we are pointing at the point
  // so this method could be used to say if more than 0.95 point is hit

  float closest_distance = -2;
  String closest_point = "";

  int pointCount = sizeof(points) / sizeof(points[0]);

  for (int i =0; i<pointCount;i++) {
    float result = DotProduct(current, points[i].dir);

    if (closest_distance < result) {
      closest_distance = result;
      closest_point = points[i].name;
    }

  }

  return closest_point;
}

void setup() {
  Serial.begin(9600);
  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to movement sensor module
  movement.begin();

  //initialize the first reading
  movement.update();
  x=movement.getX();
  y=movement.getY();
  z=movement.getZ();

  
}

void loop() {
  // Read new movement data from the sensor
  movement.update();

  //smoothed values
  x = a*movement.getX() + (1-a)*x;
  y = a*movement.getY() + (1-a)*y;
  z = a*movement.getZ() + (1-a)*z;

  //create current dir vector!!!
  // Temporary current vector
  Vec3 current = {x, y, z};

  String bestMatch = Compare(current);

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

  if (bestMatch != "") {
    Serial.print(" | Best Match: ");
    Serial.print(bestMatch);
  }
  
  delay(200);
}

