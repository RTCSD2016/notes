// Operator Interface for heater control system
// File: heat_op.cpp
// Created 10/18/95, DM Auslander

#include <iostream.h>
#include <math.h>
#include "tasks.hpp"

void Stop(void)
   {
   StopControl ("\n\n***User terminated program with ctrl-x***\n\n");
   }

OperatorInterface::OperatorInterface(char *aName)// Constuctor for the class.
		  :BaseTask(aName)
	{
	// Create screens
	OpWin1 = new COperatorWindow ("Heater Control Operation");
	OpWin1->AddInputItem("Final Setpoint",&temp_final);
	OpWin1->AddOutputItem ("Time", &Time);
	OpWin1->AddOutputItem ("Setpoint", &setpoint);
	OpWin1->AddOutputItem ("Temperature", &temperature);
	OpWin1->AddOutputItem ("M-Cntrl", &mcntrl);

	OpWin1->AddKey ('X', "Stop", "Program", Stop);
	DeltaTaskTime = 0.05;	// Set screen refresh interval
	NextTaskTime = 0.0;
	State = 0;
	temp_final = 0;	// Make sure it has a legal value
	}

OperatorInterface::~OperatorInterface()		// Destructor for the class.
	{
	delete OpWin1;
	}

int OperatorInterface::Run(void)
	{
   double tf_old;
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
		OpWin1->Display();	// Initial screen display
		return (0);
		}

	// Only run the task if it is proper time.
	// This can be used to keep the screen from refreshing too fast
	// to be readable
	// Note:  It is not required that there be some time
	//		based criteria for when the task runs.  This
	//		section could be taken out, resulting in a task
	//		that runs every scan.
	if ((Time = GetTimeNow()) < NextTaskTime)
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
		case 1: // Display main operation screen
			// Entry Section
			HeatCtrl->GetData(&temperature,&mcntrl,&setpoint);   // Data from the controller

			if (RunEntry)
				{
				// only run this on entry to the state
				// entry code goes here
				}
			// Action Section
			Time = GetTimeNow();
			temp_final = HeatSup->GetTempFinal();
			tf_old = temp_final;
			OpWin1->Update();	// Check for user input and refresh outputs
			if(tf_old != temp_final)
         	HeatSup->SetTempFinal(temp_final);	// Transmit the new value
			// Test/Exit section
				// No other states yet!
         NextState = 1;
			break;
		default:  // check for an illegal state.
			cout << "\nIllegal State assignment, Operator Interface Task\n";
			return (-1);
		}
	return (done);
}

// Setup screen - this screen is run before real time starts
// so can be used to set up initial values for gains, etc.

static int setup_done;

void SetupSetDone(void)
	{
	setup_done = 1;
	}

void HeatSetup(void)
	{
	double kp,ki,kd,mn,mx,tickrate;

	COperatorWindow *OpWinSetup = new COperatorWindow("Setup Initial Data");

	HeatCtrl->GetGains(&kp,&ki,&kd,&mn,&mx);	// Get current values
	tickrate = GetTickRate();
	OpWinSetup->AddInputItem("Tick Rate",&tickrate);
	OpWinSetup->AddInputItem("Kp",&kp);
	OpWinSetup->AddInputItem("Ki",&ki);
	OpWinSetup->AddInputItem("Kd",&kd);
	OpWinSetup->AddKey ('X', "Start", "Control", SetupSetDone);
	setup_done = 0;
	OpWinSetup->Display();	// Initial display
	while(!setup_done)OpWinSetup->Update();
	SetTickRate(tickrate);
	HeatCtrl->SetGains(kp,ki,kd,mn,mx);
	OpWinSetup->Close();	// Shutdown the window
   delete OpWinSetup;
	}


