#include "Modulino.h"
#include <map>

// Create a ModulinoMovement
ModulinoMovement movement;

float x, y, z;
float roll, pitch, yaw;
int a - 0.2; //smoothing factor

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

  //initialize the map and points
  map<string, array> points;
  map["P1"] = {0, 0, 0};
  map["P2"] = {1, 0, 0};

  
}

void loop() {
  // Read new movement data from the sensor
  movement.update();

  //smoothed values
  x = a*movement.getX() + (1-a)*x;
  y = a*movement.getY() + (1-a)*y;
  z = a*movement.getZ() + (1-a)*z;

  //create current dir vector!!!

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
  
  delay(200);
}

int Compare(array current, map points) {
  //compares the current direction vector to the points in the map and returns how close it is to the closest one
  // the dot product being near equal to 1 means we are pointing at the point
  // so this method could be used to say if more than 0.95 point hit

  //assuming the current is just current[x][y][z] for now same for points

  int closest_distance = -2;

  //for (int i =0; i<point.size();i++) {//loop through the points in the map and get the dot product with current dir

    //result = dot product of current and point
    // result is between -1 and 1

    //ex: D_current​⋅D_point​=dx1​dx2​+dy1​dy2​+dz1​dz2​

  for (auto point: points){

    result = current[0]*point.second[0]+current[1]*point.second[1]+current[2]*point.second[2];

    if (closest_distance < result) {
      closest_distance = result;
    }


  }


  return closest_distance;
}