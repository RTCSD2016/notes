// Include definitions of time mode. Has been separated from
// tasks.hpp because of definition conflicts with the operator
// interface code.

// time_mod.hpp
// Created 8/19/96, DM Auslander

#ifndef TIME_MOD_HPP
#define TIME_MOD_HPP

//#include <iostream.h>
//#include <fstream.h>
//#include <string.h>
#include "clock.hpp" // Time-keeping functions


// Define the time keeping and scheduling modes
#undef ISR_SCHEDULING
//#define ISR_SCHEDULING

// Define the timing mode  -- One (and only one) of the
// following should be active
// #define TIMEFREE	// Use the PC's free-running timer
 #define TIMEINTERNAL	// Use internal time calculation only
		// (i.e., calibrated time). TICKRATE sets the internal
		// time increment.
// #define TIME_NT   // NT's microsecond timer
// This is the time granularity if the free running timer
// is not used.  Each scheduler scan, the time is incremented
// by TICKRATE.  You can adjust this value to make the time
// a decent approximation of the real time.
#define TICKRATE 0.01

void DisableInterrupt(void);	// Prototypes
void EnableInterrupt(void);

#endif // TIME_MOD_HPP

