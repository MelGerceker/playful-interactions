#include "Modulino.h"

ModulinoMovement movement;

// Motion detection thresholds
const float MOTION_THRESHOLD = 0.15;  // g units for acceleration
const float ROTATION_THRESHOLD = 20.0;  // degrees per second

// Calibration variables
float baseX = 0, baseY = 0, baseZ = 1.0;  // Expect 1g on Z when flat
float baseRoll = 0, basePitch = 0, baseYaw = 0;
bool isCalibrated = false;

// Motion state tracking
bool inMotion = false;
unsigned long motionStartTime = 0;
unsigned long stillStartTime = 0;
const unsigned long STILL_TIMEOUT = 2000;  // 2 seconds to detect stillness

void setup() {
  Serial.begin(9600);
  Modulino.begin();
  movement.begin();
  
  Serial.println("Motion Detection System");
  Serial.println("======================");
  Serial.println("Keep device still for calibration...");
  
  delay(2000);
  calibrateSensor();
}

void calibrateSensor() {
  const int samples = 50;
  float sumX = 0, sumY = 0, sumZ = 0;
  float sumRoll = 0, sumPitch = 0, sumYaw = 0;
  
  for (int i = 0; i < samples; i++) {
    movement.update();
    sumX += movement.getX();
    sumY += movement.getY();
    sumZ += movement.getZ();
    sumRoll += movement.getRoll();
    sumPitch += movement.getPitch();
    sumYaw += movement.getYaw();
    delay(20);
  }
  
  baseX = sumX / samples;
  baseY = sumY / samples;
  baseZ = sumZ / samples;
  baseRoll = sumRoll / samples;
  basePitch = sumPitch / samples;
  baseYaw = sumYaw / samples;
  
  isCalibrated = true;
  Serial.println("✓ Calibration complete!");
  Serial.println();
}

void detectMotion() {
  movement.update();
  
  // Calculate deltas from baseline
  float deltaX = abs(movement.getX() - baseX);
  float deltaY = abs(movement.getY() - baseY);
  float deltaZ = abs(movement.getZ() - baseZ);
  float deltaRoll = abs(movement.getRoll() - baseRoll);
  float deltaPitch = abs(movement.getPitch() - basePitch);
  float deltaYaw = abs(movement.getYaw() - baseYaw);
  
  // Check if any threshold is exceeded
  bool motionDetected = (deltaX > MOTION_THRESHOLD || 
                         deltaY > MOTION_THRESHOLD || 
                         deltaZ > MOTION_THRESHOLD ||
                         deltaRoll > ROTATION_THRESHOLD ||
                         deltaPitch > ROTATION_THRESHOLD ||
                         deltaYaw > ROTATION_THRESHOLD);
  
  // State machine for motion detection
  if (motionDetected && !inMotion) {
    // Motion just started
    inMotion = true;
    motionStartTime = millis();
    Serial.println("🏃 MOTION DETECTED!");
    
    // Identify type of motion
    if (deltaRoll > ROTATION_THRESHOLD || deltaPitch > ROTATION_THRESHOLD) {
      Serial.println("   Type: Rotation");
    }
    if (deltaX > MOTION_THRESHOLD || deltaY > MOTION_THRESHOLD) {
      Serial.println("   Type: Linear movement");
    }
    if (deltaZ > MOTION_THRESHOLD * 2) {
      Serial.println("   Type: Vertical movement");
    }
  }
  else if (!motionDetected && inMotion) {
    // Motion might have stopped
    if (stillStartTime == 0) {
      stillStartTime = millis();
    }
    else if (millis() - stillStartTime > STILL_TIMEOUT) {
      // Device has been still for timeout period
      inMotion = false;
      unsigned long duration = (millis() - motionStartTime) / 1000;
      Serial.print("✋ Motion stopped. Duration: ");
      Serial.print(duration);
      Serial.println(" seconds");
      stillStartTime = 0;
    }
  }
  else if (motionDetected && inMotion) {
    // Still in motion, reset still timer
    stillStartTime = 0;
  }
}

void loop() {
  if (isCalibrated) {
    detectMotion();
    delay(50);
  }
}