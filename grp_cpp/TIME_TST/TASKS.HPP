// Main header file for listing tasks for use in other task
// definitions.   This version is for the time-test program 
// and just has clock definitions in it.
// 10/15/95, DM Auslander

#ifndef TASKS_HPP
#define TASKS_HPP
#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include "clock.hpp" // Time-keeping functions
//#include "audit.hpp" // Transition audit file functions
//#include "oper_int.hpp"  // Only include this if an operator 
		// interface is being used


// Define the timing mode  -- One (and only one) of the
// following should be active
// #define TIMEFREE	// Use the PC's free-running timer
// #define TIMEINTERNAL	// Use internal time calculation only
		// (i.e., calibrated time). TICKRATE sets the internal
		// time increment.
    #define TIME_NT   // Use WinNT's internal microsecond timer
    
// This is the time granularity if the free running timer
// is not used.  Each scheduler scan, the time is incremented
// by TICKRATE.  You can adjust this value to make the time
// a decent approximation of the real time.
#define TICKRATE 0.001

void DisableInterrupt(void);	// Prototypes
void EnableInterrupt(void);
#endif // TASKS_HPP

