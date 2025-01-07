#ifndef HELPER_H
#define HELPER_H

#include <Arduino.h>
#include "config.h"

// Timer update function (us)
inline void putTimer(uint32_t &t) { t = millis(); }
// Get time elapsed since timer var was updated (us)
inline uint32_t getTimeElapsed(uint32_t &t) { return millis() - t; }
#if ENABLE_DEBUG
// Get time elapsed and print to serial
uint32_t showTimer(uint32_t &t);
void printDebug(const char *s);
void printDebug(const String &s);
void printDebug(float f);
#else
inline uint32_t showTimer(uint32_t &t) { return 0; }
inline void printDebug(const char *s) { return; }
inline void printDebug(const String &s) { return; }
inline void printDebug(float f) { return; }
#endif

// Macro for circular buffer looping. Ensure `0 <= a, b < n`.
#define forc(n, a, b, code_block)		\
	do {								\
		uint8_t __idx_forc = (a);		\
		while (__idx_forc != (b)) {		\
			{code_block}				\
			__idx_forc=__idx_forc+1;	\
			if(__idx_forc==(n))			\
				 __idx_forc=0;			\
		}								\
	} while (0)
// Inline function for circular buffer incrementing
inline void incc(uint8_t &i, uint8_t n) { if (n!=0) {i++; if(i>=n) i=0;} }
// Inline function for circular buffer decrementing
inline void decc(uint8_t &i, uint8_t n) { if (n!=0) {if(i==0) i=n; i--;} }
inline uint8_t decconst(uint8_t i, uint8_t n) { if (n!=0) {if(i==0) i=n; return i-1; } }

#endif