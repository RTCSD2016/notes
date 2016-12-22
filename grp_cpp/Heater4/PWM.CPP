// Simple PWM task.
// File:  pwm.cpp
// Created 8/2/95 by Jason Jones
// Modified 10/4/96, DM Auslander, for message passing

#include <iostream.h>
#include <string.h>
#include "tasks.hpp"

PWMTask::PWMTask(char *aName,int ProcNo) : BaseTask (aName,ProcNo)
	{
	DeltaTaskTime = 0.001; // Task period
	NextTaskTime = 0.0;
	State = 0;	       // set to initialize
	NextState = 0;
	Period = 0.10;
	DutyCycle = 0.25;
	PWMOutput = 0.0;
	EndOffTime = 0.0;	// Initial value to calculate first cycle
	PWMRecord = new DataOutput(1,1000);	// Create and output record
	CreateGlobal(1);  // Create global data bases
   ActivateProfiling(5.e-6,80.e-6,15,LOG_PROFILE);
	}

PWMTask::~PWMTask(void)
	{
	delete PWMRecord;
	};

int PWMTask::Run(void)
	{
	// Initialization code.  If there is anything
	// that the task needs to do only once at
	// start-up, put it here.
	double Time;
	int done = 1;	// Flag, default will suspend task

	NumScans++;	// Increment scan count

	if (State == 0)
		{
		State = 1;
		NextState = 1;
		return (0);
		}

	Time = GetTimeNow();

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
		case 1: // State 1: "Compute_Times"
			// Entry Section
			if (RunEntry)
				{
				// only run this on entry to the state
				// entry code goes here
				// Copy the latest value for duty cycle and period
				// Period is set from default value -- it could be
            // obtained from another task at this point
            DutyCycle = GetGlobalData(HeatCtrl,PID_ACTUATION);
				// limit the value of DutyCycle to [0,1]
				if(DutyCycle < 0.0)
					DutyCycle = 0.0;
				else if(DutyCycle > 1.0)
					DutyCycle = 1.0;
				EndOnTime = DutyCycle*Period+EndOffTime;
				EndOffTime += Period;
				}
			// Action Section
				// no action code for this state
			// Test/Exit section
			// transition tests go here.
			if(DutyCycle > 0.0)
				NextState = 2;
				// exit code goes here
					// No exit section for this transition
			else if (DutyCycle <= 0.0)
				NextState = 3;
				// exit code goes here
					// No exit section for this transition
			done = 0;	// Continue in this task
			break;
		case 2: // State 2: "PWM_On"
			// Entry Section
			if (RunEntry)
				{
				// only run this on entry to the state
				// entry code goes here
				// turn the PWM output on
				PWMOn();
				}
			// Action Section
				// no action code for this state
			// Test/Exit section
			// transition tests go here.
			if((Time >= EndOnTime) && (DutyCycle < 1.0))
				NextState = 3;
				// exit code goes here
					// No exit section for this transition
			else if((Time >= EndOnTime) && (DutyCycle >= 1.0))
				NextState = 1;
				// exit code goes here
					// No exit section for this transition
			break;
		case 3: // State 3: "PWM_Off"
			// Entry Section
			if (RunEntry)
				{
				// only run this on entry to the state
				// entry code goes here
				// turn the PWM output on
				PWMOff();
				}
			// Action Section
				// no action code for this state
			// Test/Exit section
			// transition tests go here.
			if((Time >= EndOffTime))
				NextState = 1;
				// exit code goes here
					// No exit section for this transition
			break;
		default:  // check for an illegal state.
			cout << "\nIllegal State assignment, Task1\n";
			return (-1);
		}
   CopyGlobal();  // make global data visible
	return (done);	// Return flag indicating whether this task
   	// is done for the current event
}

void PWMTask::PWMOn(void)
	{
	PWMOutput = 1.0;
   //*SendRTMessage(SimTask,SIM_ACTUATION,1,PWMOutput,MESSAGE_OVERWRITE);
   PrivateGlobalData[PWM_OUTPUT] = PWMOutput;
	PWMRecord->AddData(PWMOutput,END_OF_ARGS);
	}

void PWMTask::PWMOff(void)
	{
	PWMOutput = 0.0;
   //*SendRTMessage(SimTask,SIM_ACTUATION,1,PWMOutput,MESSAGE_OVERWRITE);
   PrivateGlobalData[PWM_OUTPUT] = PWMOutput;
	PWMRecord->AddData(PWMOutput,END_OF_ARGS);
	}

void PWMTask::WritePWMRecord(char *filename)
	{
	PWMRecord->WriteOutputFile(filename);
	}

