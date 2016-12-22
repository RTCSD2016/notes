// Heater control example system
// File:  heatctrl.cpp
// Created 8/2/95 by Jason Jones
// Modified 10/6/95 by DM Auslander to become heater control
// 10/18/95, DM Auslander, added operator interface
// 3/1/97, DM Auslander, modified for multi-process using UDP

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

double EndTime = 1.0e10;  // make this value visible to other function
          // in this file (use big number to run forever)

void main(void)
    {
    // These variables are used to keep track of when the
    // scheduler must exit.
    double TheTime;
    double NextNetTime = 0.0,DeltaNetTime = 0.1;  // Interval for network data
    
    // Declaration of TList pointers are in tasks.hpp
    LowPriority = new TList("Low Priority");
    Intermittent = new TList("Intermittent");
    #ifdef ISR_SCHEDULING
    Preemptable = new TList("Preemptable");
    Interrupt = new TList("Interrupt");
    #endif // ISR_SCHEDULING

    // Create all of the task objects used in the project
    PWM = new PWMTask("PWM",PROCESS_A);
    HeatSup = new HeatSupTask("HeatSup",PROCESS_A);
    HeatCtrl = new HeaterControl("HeaterPID",PROCESS_A);
    OpInt = new OperatorInterface("Oper Int",PROCESS_B);
    DataLogger = new _DataLogger("Data Logger",PROCESS_A);
    // Define another task object for running the simulation.
    // This task is scheduled just as any other tasks, and is
    // only added to the task list if a simulation is desired.
    // The actual simulation code in contained in the Run()
    // member function of SimulationTask.
    #ifdef SIMULATION
    SimTask = new SimulationTask("Simulation",PROCESS_A);
    #endif //SIMULATION

    // All tasks must be added to the task list here.
    // Only add tasks that are in this process
    #ifdef ISR_SCHEDULING
    if(ThisProcess == PROCESS_A)Preemptable->Append(PWM);
    if(ThisProcess == PROCESS_A)Interrupt->Append(HeatCtrl);
    #else // ISR_SCHEDULING
    if(ThisProcess == PROCESS_A)Intermittent->Append(PWM);
    if(ThisProcess == PROCESS_A)LowPriority->Append(HeatCtrl);
    #endif // ISR_SCHEDULING
    if(ThisProcess == PROCESS_A)LowPriority->Append(HeatSup);
    if(ThisProcess == PROCESS_B)LowPriority->Append(OpInt);
    if(ThisProcess == PROCESS_A)LowPriority->Append(DataLogger);
    #ifdef SIMULATION
    if(ThisProcess == PROCESS_A)LowPriority->Append(SimTask);
    #endif //SIMULATION


    // You must call this function to initialize the Audit
    // trail array before you write to it.
    InitAuditTrail();

    cout << "Maximize window if necessary. ENTER to continue...\n";
    getch();

    #ifdef ISR_SCHEDULING
    SetupTimerInterrupt();
    #else // ISR_SCHEDULING
    // Set the proper mode for the timer
    SetupClock(-1.0);
    #endif // ISR_SCHEDULING

    #ifdef USE_UDP
        InitUDP(NumberOfProcesses,ThisProcess,ProcessAddress);
    #endif

    if(ThisProcess == PROCESS_B)OpInt->HeatSetup();
         // Get initial parameter values

    // Scheduler loop.  Inside this loop, you MUST NOT call
    // any terminal I/O functions such as printf, cout,
    // getchar, scanf, etc.  This include inside the run
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
      #ifdef USE_UDP
        if(TheTime > NextNetTime)
            {
            CheckNet();  // This is the lowest priority position --
            // Higher priority positions may be more appropriate
            if(!SendAll())StopControl("\n***Problem sending network data***\n");
            NextNetTime += DeltaNetTime;
            }
      #endif
      if(OpInt->GetGlobalData(OpInt,OPINT_STOP) > 0.0)
      	{
         SendAll();  // Make sure final data is sent (it has the
            // STOP signal!)
         StopControl("\n\n***User terminated program with ctrl-x***\n\n");
            // Do it this way so all processes will stop, not just the one with
            // the operator interface
         }
      }

    #ifdef ISR_SCHEDULING
    RestoreTimerInterrupt();
    #endif // ISR_SCHEDULING

    #ifdef USE_UDP
        NetCleanUp();
    #endif

    clrscr ();  // Clear the operator interface

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
    if(ThisProcess == PROCESS_A)DataLogger->WriteFile("heat_out.txt");
    if(ThisProcess == PROCESS_A)PWM->WritePWMRecord("pwm_rec.txt");
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
