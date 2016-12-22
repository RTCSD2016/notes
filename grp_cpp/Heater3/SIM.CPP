// Simulation task for heater control example
// File: sim.cpp
// Created 10/5/95, DM Auslander

#include <math.h>
#include "tasks.hpp"

SimulationTask::SimulationTask(char *aName)
		  :BaseTask(aName)
	{
	DeltaTaskTime = 0.01; // Task period
	NextTaskTime = 0.0;
	State = 0;	       // set to initialize
	NextState = 0;
	Kin = 2;	// Conversion from pwm_out
	Kout = 1.5;	// Heat loss coeffiecient
	Csys = 10.0;		// Target system total heat capacity
	temp_amb = 0;	// Ambient temperature
	temp = 0.3;	// Initial temperature
   pwm_out = 0.0;
	previous_time = 0.0;
   CreateGlobal(2);  // Create a global data base
   ActivateProfiling(5.e-6,80.e-6,15,LOG_PROFILE);
	}

SimulationTask::~SimulationTask(void){};

int SimulationTask::Run(void)
	{
	double Time;
	NumScans++;	// Increment scan count

	if (State == 0)
		{
		// Initialization code.  If there is anything
		// that the task needs to do only once at
		// start-up, put it here.
		State = 1;
		NextState = 1;
		previous_time = GetTimeNow();
		return (0);
		}

	if (NextState != -1)
		{
		// record audit trail here if desired
		// AuditTrail(this, State, NextState);
		State = NextState;
		NextState = -1;
		RunEntry = 1;
		}
	else
		RunEntry = 0;

	switch (State)
		{
		case 1: // State 1
			// Entry Section
			if (RunEntry)
				{
				// only run this on entry to the state
				// Entry code goes here:
				// The actual simulation calculation goes in the
				// entry section since a self-transition is used
				// to do the timing for this task
				double qin,qout,xdot,delt;

				Time = GetTimeNow();
				delt = Time - previous_time;
				previous_time = Time;
				// Get data from other tasks
            pwm_out = GetGlobalData(PWM,PWM_OUTPUT);

				qin = Kin * pwm_out;	// Heat input rate from heater
				qout = Kout * (temp - temp_amb);	// Heat loss
				xdot = (qin - qout) / Csys;
				temp += xdot * delt;
            PrivateGlobalData[SIM_TEMP] = temp;
            CopyGlobal();  // Make data available
				}
			// Action Section
			// No action section for this task

			// Test/Exit section
			// transition tests go here.
			Time = GetTimeNow();
			// Check for time to do the next simulation computations
			if(Time >= NextTaskTime)
				{
				NextState = 1;	// Self transition
				NextTaskTime += DeltaTaskTime;
				}
			break;
		default:  // check for an illegal state.
			cout << "\nIllegal State assignment, Task1\n";
			return (-1);
		}
	return (0);
}

