// Main header file for listing tasks for use in other task
// definitions. This version has nothing in it but the operator
// interface.
// D.M. Auslander, 8/25/98

#ifndef TASKS_HPP
#define TASKS_HPP
#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include "oper_int.hpp"  // Only include this if an operator
		// interface is being used

#ifdef ISR_SCHEDULING
void interrupt TimerISR(...);
#endif

void DisableInterrupt(void);	// Prototypes
void EnableInterrupt(void);
#endif // TASKS_HPP

