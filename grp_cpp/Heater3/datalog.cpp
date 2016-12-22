// Data logging task for heater control problem
// datalog.cpp
// Created 10/4/96, DM Auslander

#include <iostream.h>
#include <math.h>
#include "tasks.hpp"

_DataLogger::_DataLogger(char *name)  // Constructor
      :BaseTask(name)
   {
   DataOut1 = new DataOutput(3,0.0,20.0,0.1);
   set = 0.0;  // Legal values
   val = 0.0;
   mc = 0.0;
	DeltaTaskTime = 0.2;	// Set data logging time interval
	NextTaskTime = 0.0;
	State = 0;
   }

_DataLogger::~_DataLogger()// Destructor
   {
   delete DataOut1;
   }

void _DataLogger::WriteFile(char *filename)
   {
   DataOut1->WriteOutputFile(filename);
   }

int _DataLogger::Run(void)
   {
	int done = 1;	// Default return value indicating
			// done for this event
	NumScans++;	// Increment scan count

	if (State == 0)
		{
		// Initialization code.  If there is anything
		// that the task needs to do only once at
		// start-up, put it here.
		State = 1;
		NextState = 1;
		return (0);
		}

   // Let task scan all of the time -- the data object knows when
   // to store data

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
		case 1:
			// No Entry Section for this state

         // Action section -- log data
         if(DataOut1->IsTimeForOutput())
			   {
            // Get new values
            CriticalSectionBegin();
            val = GetGlobalData(HeatCtrl,PID_PROCESS);
            mc = GetGlobalData(HeatCtrl,PID_ACTUATION);
            set = GetGlobalData(HeatCtrl,PID_SET);
            CriticalSectionEnd();
		   	DataOut1->AddData(set, val, mc, END_OF_ARGS);
			   IncrementTime();	// Account for time spend in output recording
   			}
         // No transitions -- stay in this state all of the time
      }
   return(done);
   }

