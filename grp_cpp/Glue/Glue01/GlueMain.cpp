// Main file for the glueing/manufacturing example
// File: GlueMain.cpp

// Originally created: May 17, 1997 by D.M. Auslander
// Various versions of this file will be used to build a complete
// control system in steps. Comments will be additive to describe
// which level the file refers to.

// 1. Added thermal simulation of the ovens

#define MAIN_CPP

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include <dos.h>
#include "tasks.hpp" // Task and environment info

static StopControlNow = 0;
char *StopMessage = NULL;
void StopControl(char *aMessage)
    {
    StopControlNow = 1;
    StopMessage = new char[strlen(aMessage)+1];
    strcpy(StopMessage, aMessage);
    }

double EndTime = 2.0;  // make this value visible to other functions
          // in this file (use big number to run forever)

void main(void)
    {
    // These variables are used to keep track of when the
    // scheduler must exit.
    double TheTime;

    // Declaration of TList pointers are in tasks.hpp
    LowPriority = new TList("Low Priority");
    Intermittent = new TList("Intermittent");
    #ifdef ISR_SCHEDULING
    Preemptable = new TList("Preemptable");
    Interrupt = new TList("Interrupt");
    #endif // ISR_SCHEDULING

    // Create all of the task objects used in the project
    DataLogger = new CDataLogger("Data Logger",PROCESS_A);

    // Define task objects for running the simulation.
    // These tasks are scheduled just as any other tasks, and are
    // only added to the task list if a simulation is desired.
    // The actual simulation code in contained in the Run()
    // member functions.
    #ifdef SIMULATION
    // char *aName,double JJ,double Ktt, double ccur,int ProcNo
    BeltAxis1 = new CBeltAxis("BeltAxis1",25.0,4.0,3.0,PROCESS_A);
    BeltAxis2 = new CBeltAxis("BeltAxis2",25.0,4.0,2.0,PROCESS_A);
    //char *aName,double aK,double aThAmb, double aCur,
    // double aR,double aHCap,int ProcNo
    ThOven1 = new CThOven("ThOven1",0.0,20.0,10.0,20.0,
        2.0e5,PROCESS_A);
    ThOven2 = new CThOven("ThOven2",0.0,20.0,7.0,20.0,
        2.0e5,PROCESS_A);
    #endif //SIMULATION

    // All tasks must be added to the task list here.
    // Only add tasks that are in this process
    #ifdef ISR_SCHEDULING
    #else // ISR_SCHEDULING
    #endif // ISR_SCHEDULING

    // The check for process is used for multi-process implementation
    //   to schedule active tasks in the right process. It is a
    //   dummy here.
    if(ThisProcess == PROCESS_A)LowPriority->Append(DataLogger);

    #ifdef SIMULATION
    if(ThisProcess == PROCESS_A)LowPriority->Append(BeltAxis1);
    if(ThisProcess == PROCESS_A)LowPriority->Append(BeltAxis2);
    if(ThisProcess == PROCESS_A)LowPriority->Append(ThOven1);
    if(ThisProcess == PROCESS_A)LowPriority->Append(ThOven2);
    #endif //SIMULATION


    // You must call this function to initialize the Audit
    // trail array before you write to it.
    InitAuditTrail();

    cout << "Maximize window if necessary. ENTER to continue...\n";
    getchar();

    #ifdef ISR_SCHEDULING
    SetupTimerInterrupt();
    #else // ISR_SCHEDULING
    // Set the proper mode for the timer
    SetupClock(-1.0);
    #endif // ISR_SCHEDULING

    #ifdef USE_UDP
        InitUDP(NumberOfProcesses,ThisProcess,ProcessAddress);
    #endif

    // Scheduler loop.  Inside this loop, you MUST NOT call
    // any terminal I/O functions such as printf, cout,
    // getchar, scanf, etc.  This includes inside the run
    // functions of the tasks.  These things are OK in
    // SIMULATION ONLY.  Use #ifdef SIMULATION (or something
    // similar) if you have sections of code in which you
    // want to print to the screen (for debugging).
    while (((TheTime = GetTimeNow()) <= EndTime)
                 && (TheTime >= 0.0) && (!StopControlNow))
      {
      // Run LowPriority and Intermittent list
      if (TaskDispatcher(LowPriority,Intermittent))
          StopControl("\n***Problem with background tasks***\n");

      #ifdef USE_UDP  // For multiprocess only, will not be compiled otherwise
        if(TheTime > NextNetTime)
            {
            CheckNet();  // This is the lowest priority position --
            // Higher priority positions may be more appropriate
            if(!SendAll())StopControl("\n***Problem sending network data***\n");
            NextNetTime += DeltaNetTime;
            }
      #endif
      }

    // End-of-run clean-up activities...

    #ifdef ISR_SCHEDULING
    RestoreTimerInterrupt();
    #endif // ISR_SCHEDULING

    #ifdef USE_UDP
        NetCleanUp();
    #endif

    // clrscr ();  // Clear the operator interface

   if (StopMessage == NULL)
      cout << "\n\n***Normal Program Termination***\n\n";
   else
      {
      cout << StopMessage;
      delete [] StopMessage;
      }

    // There are two versions of the audit trail, one with task
    //  names and one with task numbers instead (for matlab)
    WriteAuditTrail("trail.txt");
    WriteAuditTrailNum("trl_num.txt");
    // open a file to save the output data
    cout << "\nThe time at termination is " << TheTime << "\n";
    // Print the scan statistics to the screen and to a file
    WriteScanStat("scans.txt", TheTime);
    // Save the output data to a file
    if(ThisProcess == PROCESS_A)DataLogger->WriteFile("gluedata.txt");
    WriteProfileStat("profiles.txt");  // Write profiles for any tasks
    // that have them
    // De-allocate space given to tasks
    delete (LowPriority);
    delete (Intermittent);
    #ifdef ISR_SCHEDULING
    delete (Preemptable);
    delete (Interrupt);
    #endif // ISR_SCHEDULING
    cout << "Hit any key to exit.\n";
    while(!kbhit()) ;  // Wait to exit
    }
    
