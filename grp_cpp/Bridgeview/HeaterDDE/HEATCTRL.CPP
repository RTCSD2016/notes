// Heater control example system                     
// File:  heatctrl.cpp
// Created 8/2/95 by Jason Jones
// Modified 10/6/95 by DM Auslander to become heater control
// 10/18/95, DM Auslander, added operator interface

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

	// Declaration of TList pointers are in tasks.hpp
	LowPriority = new TList("Low Priority");
	Intermittent = new TList("Intermittent");
	#ifdef ISR_SCHEDULING
	Preemptable = new TList("Preemptable");
	Interrupt = new TList("Interrupt");
	#endif // ISR_SCHEDULING

	// Create all of the task objects used in the project
	PWM = new PWMTask("PWM");
	HeatSup = new HeatSupTask("HeatSup");
	HeatCtrl = new HeaterControl("HeaterPID");
	OpInt = new OperatorInterface("Oper Int");
   DataLogger = new _DataLogger("Data Logger");
	// Define another task object for running the simulation.
	// This task is scheduled just as any other tasks, and is
	// only added to the task list if a simulation is desired.
	// The actual simulation code in contained in the Run()
	// member function of SimulationTask.
	#ifdef SIMULATION
	SimTask = new SimulationTask("Simulation");
	#endif //SIMULATION

	// All tasks must be added to the task list here.
	#ifdef ISR_SCHEDULING
	Preemptable->Append(PWM);
	Interrupt->Append(HeatCtrl);
	#else // ISR_SCHEDULING
	Intermittent->Append(PWM);
	LowPriority->Append(HeatCtrl);
	#endif // ISR_SCHEDULING
	LowPriority->Append(HeatSup);
	LowPriority->Append(OpInt);
   LowPriority->Append(DataLogger);
	#ifdef SIMULATION
	LowPriority->Append(SimTask);
	#endif //SIMULATION

	// You must call this function to initialize the Audit
	// trail array before you write to it.
	InitAuditTrail();

	cout << "Maximize window if necessary. ENTER to continue...\n";
	getch();

   #ifdef USE_DDE
    // Wait for the DDE connection to be made before continuing
    // This assures that information won't be sent into a void!
    cout << "Waiting for a DDE connection\n";
    while(DDE_NumberConnections() <= 0) ;   // Wait for a connection
    cout << "DDE connection established\n";
   #endif

	OpInt->HeatSetup();	// Get initial parameter values

	#ifdef ISR_SCHEDULING
	SetupTimerInterrupt();
	#else // ISR_SCHEDULING
	// Set the proper mode for the timer
	SetupClock(-1.0);
	#endif // ISR_SCHEDULING

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
      // (Data logging has been moved to its own task)
		}
	#ifdef ISR_SCHEDULING
	RestoreTimerInterrupt();
	#endif // ISR_SCHEDULING

	clrscr ();	// Clear the operator interface

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
	// Print the scan statistics to the screen
	WriteScanStat("scans.txt", TheTime);
	// Save the output data to a file
	DataLogger->WriteFile("heat_out.txt");
	PWM->WritePWMRecord("pwm_rec.txt");
	WriteProfileStat("profiles.txt");  // Write profiles for any tasks
   	// that have them
	// De-allocate space give to tasks
	delete (LowPriority);
	delete (Intermittent);
	#ifdef ISR_SCHEDULING
	delete (Preemptable);
	delete (Interrupt);
	#endif // ISR_SCHEDULING
   cout << "Hit any key to exit.\n";
   while(!kbhit()) ;  // Wait to exit
	}


