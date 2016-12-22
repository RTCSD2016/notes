// Pulse Width Modulation (PWM) Generator
// Sample program
// File: pwm1test.cpp
// Created  12/24/95, DM Auslander (from other examples)
// This sample has no operator interface

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
	StopMessage = new char[strlen(aMessage) +1];
	strcpy(StopMessage, aMessage);
	}

double EndTime = 0.4;  // make this value visible to other function
		  // in this file (use big number to run forever)

void main(void)
	{
	// These variables are used to keep track of when the
	// scheduler must exit.
	double TheTime;

	// Declaration of TList pointers are in tasks.hpp
	LowPriority = new TList("Low Priority");
	Intermittent = new TList("Intermittent"); // No min-latency
         // list for this project so this list will remain empty
	#ifdef ISR_SCHEDULING
	Preemptable = new TList("Preemptable"); // When using timer
      // interrupt no preemptable tasks are used, only 'interrupt'
	Interrupt = new TList("Interrupt");
	#endif // ISR_SCHEDULING

	// Create all of the task objects used in the project
	PWM = new PWMTask("PWM");
	PWMSup = new PWMSupTask("PWMSup",0.4);   // 2nd argument is the
         // period of the duty cycle command signal
	// Define another task object for running the simulation.
	// This task is scheduled just as any other tasks, and is
	// only added to the task list if a simulation is desired.
	// The actual simulation code in contained in the Run()
	// member function of SimulationTask.
   // (There is no simulation in this example)
	#ifdef SIMULATION
	SimTask = new SimulationTask("Simulation");
	#endif //SIMULATION

	// All tasks must be added to the task list here.
	#ifdef ISR_SCHEDULING
	//Preemptable->Append();
	Interrupt->Append(PWM);
	#else // ISR_SCHEDULING
	//Intermittent->Append(PWM);
	LowPriority->Append(PWM);
	#endif // ISR_SCHEDULING

	LowPriority->Append(PWMSup);
	#ifdef SIMULATION
	LowPriority->Append(SimTask);
	#endif //SIMULATION

	// Set up a data output object
   // Arguments for DataOutput:
   // int nvars,double STime,double LTime,double DTime
   // 'nvars' is the number of dependent variables -- time
   // will be placed in the first column automatically
	DataOutput *DataOut1 = new DataOutput(2,0.0,EndTime,0.001);

	// You must call this function to initialize the Audit
	// trail array before you write to it.
	InitAuditTrail();

   // There is no operator interface, so it the next two statements
   // aren't neede
	//cout << "Maximize window if necessary. ENTER to continue...\n";
	//getch();

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
	  if(DataOut1->IsTimeForOutput())
			{
			double val,dc;
         val = PWM->GetPWMOutput();
         dc = PWMSup->GetDutyCycle();
			DataOut1->AddData(dc,val, END_OF_ARGS);
			IncrementTime();	// Account for time spend in output recording
			}
		// Run LowPriority and Intermittent list
		if (TaskDispatcher(LowPriority,Intermittent))
			StopControl("\n***Problem with background tasks***\n");
		}
	#ifdef ISR_SCHEDULING
	RestoreTimerInterrupt();
	#endif // ISR_SCHEDULING

	//clrscr ();	// Clear the operator interface

   if (StopMessage == NULL)
      cout << "\n\n***Normal Program Termination***\n\n";
   else
      cout << StopMessage;

	// There are two versions of the audit trail, one with task
	//  names and one with task numbers instead (for matlab)
	WriteAuditTrail("trail.txt");
	WriteAuditTrailNum("trl_num.txt");
	// open a file to save the output data
	cout << "\nThe time at termination is " << TheTime << "\n";
	// Print the scan statistics to the screen
	WriteScanStat("scans.txt", TheTime);
	// Save the output data to a file
	DataOut1->WriteOutputFile("pwm_out.txt");
   delete DataOut1;  // Done with this object
	PWM->WritePWMRecord("pwm_rec.txt");  // PWM event log
	// De-allocate space give to tasks
	WriteProfileStat("profiles.txt");  // Write profiles for any tasks
   	// that have them
	delete (LowPriority);
	delete (Intermittent);
	#ifdef ISR_SCHEDULING
	delete (Preemptable);
	delete (Interrupt);
	#endif // ISR_SCHEDULING
   cout << "Hit any key to exit.\n";
   while(!kbhit()) ;  // Wait to exit
   }

