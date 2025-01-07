#ifndef INTENSITY_H
#define INTENSITY_H
#include <Arduino.h>

#define USER_RHR 70.0
#define USER_MHR 198.0
#define USER_AGE 22.0
#define USER_SEX 1      // pria
#define USER_HEIGHT 170 // cm
#define USER_WEIGHT 80  // kg

// batas BMI normal (WHO & ACSM)
#define BMI_NORMAL_LO 18.5
#define BMI_NORMAL_HI 25

// intensitas latihan berdasarkan BMI (ACSM)
#define ITN_NORMAL_LO 0.5
#define ITN_NORMAL_HI 0.7
#define ITN_OVER_LO 0.4
#define ITN_OVER_HI 0.6
#define ITN_UNDER_LO 0.4
#define ITN_UNDER_HI 0.6

float computeBMI(float height_cm, float weight);
float computeKarvonen(float rhr, float mhr, float age, float itn);
uint8_t checkIntensityRange(float hr);
float getLoHR();
float getHiHR();
void initializeKarvonen();

#endif