#include "intensity.h"
#include "alarm.h"

float bmi;
float lo_itn;
float hi_itn;
float lo_hr;
float hi_hr;

void computeKarvonen(float rhr, float mhr, float age) {
	lo_hr = ((mhr-rhr) * lo_itn + rhr);
	hi_hr = ((mhr-rhr) * hi_itn + rhr);
}

void computeRecommendedIntensity(float bmi) {
	if (bmi < BMI_NORMAL_LO) {
		lo_itn = ITN_UNDER_LO;
		hi_itn = ITN_UNDER_HI;
	}
	else if (bmi < BMI_NORMAL_HI) {
		lo_itn = ITN_NORMAL_LO;
		hi_itn = ITN_NORMAL_HI;
	}
	else {
		lo_itn = ITN_OVER_LO;
		hi_itn = ITN_OVER_HI;
	}
}

float computeBMI(float height_cm, float weight) {
	float height = height_cm / 100.0;
	return weight / (height*height);
}

uint8_t checkIntensityRange(float hr) {
	if (hr < lo_hr) return 0;
	if (hr < hi_hr) return 1;
	return 2;
}

float getLoHR() {
	return lo_hr;
}

float getHiHR() {
	return hi_hr;
}

void initializeKarvonen() {
	bmi = computeBMI(USER_HEIGHT, USER_WEIGHT);
	computeRecommendedIntensity(bmi);
	computeKarvonen(USER_RHR, USER_MHR, USER_AGE);

	Serial.printf("\n\nBMI:%f, intensitas:%f-%f%, denyut:%f-%fBPM\n\n", round(bmi), round(lo_itn*100), round(hi_itn*100), round(lo_hr), round(hi_hr));
}