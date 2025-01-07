#ifndef POX_H
#define POX_H

#include "MAX30105.h"

#define PULSE_AMPLITUDE 0x24	

extern uint32_t batchTimer;
extern uint32_t sampleTimer;			// Max current to sensor LEDs

void readIRToBuffer(uint32_t *irbuff, uint32_t *redbuff, uint8_t idx);
void updateMinMaxBufferValue(uint32_t &mn, uint32_t &mx, uint32_t *buff, uint8_t idx);

void initializeSensor();

void showBatchTimer();
void showSampleTimer();

#endif