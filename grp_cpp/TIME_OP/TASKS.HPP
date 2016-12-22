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
#include "oper_int.hpp"  // Only include this if an operator 
		// interface is being used
#include "time_mod.hpp"  // Set the time keeping mode

#ifdef ISR_SCHEDULING
void interrupt TimerISR(...);
#endif

void DisableInterrupt(void);	// Prototypes
void EnableInterrupt(void);
#endif // TASKS_HPP

