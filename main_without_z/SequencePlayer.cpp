#include "SequencePlayer.h"

#define SET_RADAR_BIT(radar, led)   ((*radar) |= (1 << (led)))
#define CLEAR_ALL_RADAR(radar)      ((*radar) = 0)

SequencePlayer::SequencePlayer(uint16_t* radarPtr)
  : sequence(nullptr), sequenceLength(0), currentStep(0),
    stepStartTime(0), running(false), radar(radarPtr) {}

void SequencePlayer::start(const Step* steps, int length) {
  sequence = steps;
  sequenceLength = length;
  currentStep = 0;
  stepStartTime = millis();
  running = true;

  if (sequenceLength > 0) {
    setDirectionLED(sequence[0].dir);
  }
}

void SequencePlayer::update() {
  if (!running || sequence == nullptr || sequenceLength == 0) return;

  unsigned long now = millis();

  if (now - stepStartTime >= sequence[currentStep].duration) {
    currentStep++;

    if (currentStep >= sequenceLength) {
      stop();
      return;
    }

    stepStartTime = now;
    setDirectionLED(sequence[currentStep].dir);
  }
}

void SequencePlayer::stop() {
  running = false;
  CLEAR_ALL_RADAR(radar);
}

bool SequencePlayer::isRunning() const {
  return running;
}

void SequencePlayer::setDirectionLED(Direction dir) {
  CLEAR_ALL_RADAR(radar);

  switch (dir) {
    case DIR_UP:
      SET_RADAR_BIT(radar, 0);
      break;
    case DIR_LEFT:
      SET_RADAR_BIT(radar, 1);
      break;
    case DIR_RIGHT:
      SET_RADAR_BIT(radar, 2);
      break;
    case DIR_DOWN:
      SET_RADAR_BIT(radar, 3);
      break;
    case DIR_CENTER:
      SET_RADAR_BIT(radar, 4);
      break;
  }
}