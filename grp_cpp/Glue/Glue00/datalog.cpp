// Data logging task for glue/manufacturing example
// datalog.cpp
// Created May 27, 1997, DM Auslander

#include <iostream.h>
#include <math.h>
#include "tasks.hpp"

CDataLogger::CDataLogger(char *name,int ProcNo)  // Constructor
      :BaseTask(name,ProcNo)
    {
    DataOut1 = new DataOutput(4,0.0,20.0,0.1);

    // Initial values for output variables
    x1 = x2 = v1 = v2 = 0.0;
    State = 0;  // For initialization on the first run
    }

CDataLogger::~CDataLogger()// Destructor
    {
    delete DataOut1;  // Recover memory used
    }

void CDataLogger::WriteFile(char *filename)
   {
   DataOut1->WriteOutputFile(filename);
   }

int CDataLogger::Run(void)
   {
    int done = 1;   // Default return value indicating
            // done for this event
    NumScans++; // Increment scan count

    if (State == 0)
        {
        // Initialization code.  If there is anything
        // that the task needs to do only once at
        // start-up, put it here.
        State = 1;  // So this section doesn't run again
        return (0);
        }

   // Let task scan all of the time -- the data object knows when
   // to store data

     if(DataOut1->IsTimeForOutput())
        {
        // Get new values
        CriticalSectionBegin();  // To make sure values are consistent even
                // if preemptive scheduling is in use
        x1 = GetGlobalData(BeltAxis1,BELTAXIS_x);
        v1 = GetGlobalData(BeltAxis1,BELTAXIS_v);
        x2 = GetGlobalData(BeltAxis2,BELTAXIS_x);
        v2 = GetGlobalData(BeltAxis2,BELTAXIS_v);
        CriticalSectionEnd();
        DataOut1->AddData(x1,v1,x2,v2, END_OF_ARGS);
        IncrementTime(); // Account for time spend in output recording
        }
   return(done);
   }
