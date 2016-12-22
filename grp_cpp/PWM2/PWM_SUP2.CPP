// PWMSup Sets the PWM duty cycle
// File:  pwm_sup.cpp
// Created 12/24/95, DM Auslander (from other examples)
// Modified to handle 2 PWM signals

#include <iostream.h>
#include <math.h>
#include "tasks.hpp"

PWMSupTask::PWMSupTask(char *aName,double DutyCyclePer1,
   double DutyCyclePer2)	// Constructor
		  :BaseTask(aName)
	{
	DeltaTaskTime = 0.001; // Task period
	NextTaskTime = 0.0;
	State = 0;	       // set to initialize
	NextState = 0;
   dc_period1 = xdc_period1 = DutyCyclePer1;
   dc_period2 = xdc_period2 = DutyCyclePer2;
   dutycycle1 = xdutycycle1 = dutycycle2 = xdutycycle2 = 0.0;
	}

PWMSupTask::~PWMSupTask(void){};  // Nothing to delete

double PWMSupTask::GetDutyCycle(int i)
   {
   double rv;

   CriticalSectionBegin();
   if(i == 1)rv = xdutycycle1;
   else if(i == 2)rv = xdutycycle2;
   // Ignore illegal requests
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
         dutycycle1 = 0.5 * sin(2.0 * Time * PI/ dc_period1) + 0.5;
         dutycycle2 = 0.5 * sin(2.0 * Time * PI/ dc_period2) + 0.5;
         CriticalSectionBegin();
         xdutycycle1 = dutycycle1; // Protect the transfer variable
         xdutycycle2 = dutycycle2;
         CriticalSectionEnd();
         PWM1->SetDutyCycle(xdutycycle1); // Send value to PWM tasks
         PWM2->SetDutyCycle(xdutycycle2);
			// Test/Exit section
         // This is the only state, so no transitions
			break;

		default:  // check for an illegal state.
			cout << "\nIllegal State assignment, Task1\n";
			return (-1);
		}
	return (done);
}

