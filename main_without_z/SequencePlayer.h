#ifndef SEQUENCEPLAYER_H
#define SEQUENCEPLAYER_H

#include <Arduino.h>

enum Direction {
  DIR_UP,
  DIR_LEFT,
  DIR_RIGHT,
  DIR_DOWN,
  DIR_CENTER
};

struct Step {
  Direction dir;
  unsigned long duration;
};

class SequencePlayer {
public:
  SequencePlayer(uint16_t* radarPtr);

  void start(const Step* steps, int length);
  void update();
  void stop();
  bool isRunning() const;

private:
  const Step* sequence;
  int sequenceLength;
  int currentStep;
  unsigned long stepStartTime;
  bool running;

  uint16_t* radar;

  void setDirectionLED(Direction dir);
};

#endif