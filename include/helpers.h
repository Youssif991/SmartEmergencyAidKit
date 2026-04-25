#include  "MAX30105.h"

void collectSamples(uint32_t* redBuf, uint32_t* irBuf, int count);
float ema(float current, float next);