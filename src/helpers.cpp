#include "helpers.h"
#include "Config.h"
void collectSamples(uint32_t* redBuf, uint32_t* irBuf, int count) {
  for (int i = 0; i < count; i++) {
    while (particleSensor.available() == 0)
      particleSensor.check();

    redBuf[i] = particleSensor.getRed();
    irBuf[i]  = particleSensor.getIR();
    particleSensor.nextSample();
  }
}