// HeatSup Sets the setpoint for the heater control
// File:  HeatSup.cpp
// Created 8/2/95 by Jason Jones
// Modified 10/5/95 by DM Auslander, changed to setpoint generator

#include <iostream.h>
#include <math.h>
#include "tasks.hpp"

HeatSupTask::HeatSupTask(char *aName)	// Constructor
		  :BaseTask(aName)
	{
	DeltaTaskTime = 0.1; // Task period
	NextTaskTime = 0.0;
	State = 0;	       // set to initialize
	NextState = 0;
	temp_init = 0.3;
	temp_final = xtemp_final = 0.3;
	temp_soak = 0.7;
	soak_time = 3.0;
	setpt = xsetpt = temp_init;
	}

HeatSupTask::~HeatSupTask(void){};

double HeatSupTask::GetSetpoint(void)
	{
	double rv;
	CriticalSectionBegin();
	rv = xsetpt;
	CriticalSectionEnd();
	return(rv);
	}

void HeatSupTask::SetTempFinal(double new_tf)
	{
	CriticalSectionBegin();
	xtemp_final = new_tf;
	CriticalSectionEnd();
	}

double HeatSupTask::GetTempFinal(void)
	{
	double rv;
	CriticalSectionBegin();
	rv = xtemp_final;
	CriticalSectionEnd();
	return(rv);
	}

int HeatSupTask::Run(void)
	{
	// Initialization code.  If there is anything
	// that the task needs to do only once at
	// start-up, put it here.
	//double Time;
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
	if ((/*Time = */GetTimeNow()) < NextTaskTime)
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
		case 1: // Bring temperature up to soaking temperature
			// Entry Section
			if (RunEntry)
				{
				// only run this on entry to the state
				// entry code goes here
				setpt = temp_soak;
				CriticalSectionBegin();
				xsetpt = setpt;
				CriticalSectionEnd();
				HeatCtrl->SetSetpoint(setpt);
				HeatCtrl->SetStart();
				}
			// Action Section
			double temp,mc,tset;
			HeatCtrl->GetData(&temp,&mc,&tset);	// Data from the controller

			// Test/Exit section
				if(temp > (temp_soak - 0.03))
					{
					NextState = 2;		// Go into the Soak state
					done = 0;		// Run next state immediately
					}
			break;

		case 2: // Soak State -- hold for specified time
			// Entry Section
			if(RunEntry)
				{
				soak_end = GetTimeNow() + soak_time;	// Record time at start
				}
			// Action Section

			// Test/Exit Section
			if(GetTimeNow() >= soak_end)
				{
				NextState = 3;		// Bring temperature to final value
				done = 0;	// Go to next state immediately
				}
			break;

		case 3:	// Final state -- bring temperature to temp_final
			// Entry Section

			// Action Section
			CriticalSectionBegin();
			temp_final = xtemp_final;
			setpt = temp_final;
			xsetpt = setpt;
			CriticalSectionEnd();
			HeatCtrl->SetSetpoint(setpt);

			// Test/Exit section
			break;

		default:  // check for an illegal state.
			cout << "\nIllegal State assignment, Task1\n";
			return (-1);
		}
	return (done);
}

