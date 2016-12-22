// Data logging task for glue/manufacturing example
// datalog.cpp
// Created May 27, 1997, DM Auslander

// Changed for level 2, PID control, June 3, 1997, DM Auslander

#include <iostream.h>
#include <math.h>
#include "tasks.hpp"

CDataLogger::CDataLogger(char *name,int ProcNo)  // Constructor
      :BaseTask(name,ProcNo)
    {
    DataOutBelt = new DataOutput(4,0.0,10.0,0.1);
    DataOutOven = new DataOutput(6,0.0,200.0,2.0);

    State = 0;  // For initialization on the first run
    }

CDataLogger::~CDataLogger()// Destructor
    {
    if(ThisProcess == PROCESS_A)WriteFile();
    delete DataOutBelt;  // Recover memory used
    delete DataOutOven;
    }

void CDataLogger::WriteFile(void)
   {
   DataOutBelt->WriteOutputFile("gluedata.txt");
   DataOutOven->WriteOutputFile("ovendata.txt");
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

     if(DataOutBelt->IsTimeForOutput())
        {
        double x1set,x1,x2set,x2;

        // Get new values
        CriticalSectionBegin();  // To make sure values are consistent even
                // if preemptive scheduling is in use
        x1set = GetGlobalData(BeltControl1,PID_SET);
        x1 = GetGlobalData(BeltAxis1,BELTAXIS_x);
        x2set = GetGlobalData(BeltControl2,PID_SET);
        x2 = GetGlobalData(BeltAxis2,BELTAXIS_x);
        CriticalSectionEnd();
        DataOutBelt->AddData(x1set,x1,x2set,x2,END_OF_ARGS);
        IncrementTime(); // Account for time spend in output recording
        }

     if(DataOutOven->IsTimeForOutput())
        {
        double Th1set,Th1,Th2set,Th2,m1,m2;

        // Get new values
        CriticalSectionBegin();  // To make sure values are consistent even
                // if preemptive scheduling is in use
        Th1set = GetGlobalData(HeatControl1,PID_SET);
        Th2set = GetGlobalData(HeatControl2,PID_SET);
        Th1 = GetGlobalData(ThOven1,THOVEN_Th);
        Th2 = GetGlobalData(ThOven2,THOVEN_Th);
        m1 = GetGlobalData(HeatControl1,PID_ACTUATION);
        m2 = GetGlobalData(HeatControl2,PID_ACTUATION);
        CriticalSectionEnd();
        DataOutOven->AddData(Th1set,Th1,m1,Th2set,Th2,m2,END_OF_ARGS);
        IncrementTime(); // Account for time spend in output recording
        }

   return(done);
   }
