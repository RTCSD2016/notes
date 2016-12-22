// PWMSup Sets the PWM duty cycle
// File:  pwm_sup.cpp
// Created 12/24/95, DM Auslander (from other examples)

#include <iostream.h>
#include <math.h>
#include "tasks.hpp"

PWMSupTask::PWMSupTask(char *aName,double DutyCyclePer)	// Constructor
		  :BaseTask(aName)
	{
	DeltaTaskTime = 0.001; // Task period
	NextTaskTime = 0.0;
	State = 0;	       // set to initialize
	NextState = 0;
	dc_period = xdc_period = DutyCyclePer;
	dutycycle = xdutycycle = 0.0;
   ActivateProfiling(5.e-6,30.e-3,15,LOG_PROFILE);
	}

PWMSupTask::~PWMSupTask(void){};  // Nothing to delete

double PWMSupTask::GetDutyCycle(void)
   {
   double rv;

   CriticalSectionBegin();
   rv = xdutycycle;
   CriticalSectionEnd();
   return(rv);
   }

int PWMSupTask::Run(void)
	{
	double Time;
	int done = 1;	// Default return value indicating
			// done for this event
	NumScans++;	// Increment scan count

	if (State == 0)
		{
		State = 1;
		NextState = 1;
		return (0);
		}

	// Only run the task if it is proper time.
	// Note:  It is not required that there be some time
	//		based criteria for when the task runs.  This
	//		section could be taken out, resulting in a task
	//		that runs every scan.
	if ((Time = GetTimeNow()) < NextTaskTime)
		return (done);		//  Done for now

	// Calculate the next run time for the task
	NextTaskTime += DeltaTaskTime;

	if (NextState != -1)
		{
		// record audit trail here if desired
		AuditTrail(this, State, NextState);
		State = NextState;
		NextState = -1;
		RunEntry = 1;
		}
	else
		RunEntry = 0;

	switch (State)
		{
		case 1: // This task has only one state, to compute the
            // duty cycle
			if (RunEntry)
				{
				// only run this on entry to the state
				// entry code goes here
				}
			// Action Section
         dutycycle = 0.5 * sin(2.0 * Time * PI/ dc_period) + 0.5;
         CriticalSectionBegin();
         xdutycycle = dutycycle; // Protect the transfer variable
         CriticalSectionEnd();
         PWM->SetDutyCycle(xdutycycle); // Send value to PWM task
			// Test/Exit section
         // This is the only state, so no transitions
			break;

		default:  // check for an illegal state.
			cout << "\nIllegal State assignment, Task1\n";
			return (-1);
		}
	return (done);
}

