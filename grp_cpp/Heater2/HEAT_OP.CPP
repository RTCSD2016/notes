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
	temp_final = 0.3;	// Initial value -- operator sets this
	}

OperatorInterface::~OperatorInterface()		// Destructor for the class.
	{
	delete OpWin1;
	}

int OperatorInterface::Run(void)
	{
   double tf_old,msg_val;
   int msg_flag;
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

			if (RunEntry)
				{
				// only run this on entry to the state
				// entry code goes here
            // Make sure HeatSup has the initial value
            SendRTMessage(HeatSup,HEATSUP_TEMPFINAL,1,temp_final,MESSAGE_OVERWRITE);
				}
			// Action Section
			Time = GetTimeNow();
         if(GetRTMessage(OPINT_TEMPERATURE,&msg_flag,&msg_val))temperature = msg_val;
         if(GetRTMessage(OPINT_MC,&msg_flag,&msg_val))mcntrl = msg_val;
         if(GetRTMessage(OPINT_SETPOINT,&msg_flag,&msg_val))setpoint = msg_val;
			tf_old = temp_final;
			OpWin1->Update();	// Check for user input and refresh outputs
			if(tf_old != temp_final)
            // Only send if value changes
            SendRTMessage(HeatSup,HEATSUP_TEMPFINAL,1,temp_final,MESSAGE_OVERWRITE);
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

// This function runs before real time operation starts so it can
//   use a blocking structure for screen update
void OperatorInterface::HeatSetup(void)
	{
	double kp=10.0,ki=6.0,kd=0.0,mn=0.0,mx=1.0,tickrate; // Gain value defaults
      // are set here since these variables are operator set

	COperatorWindow *OpWinSetup = new COperatorWindow("Setup Initial Data");

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
   // Send gains to the controller
   SendRTMessage(HeatCtrl,PID_KP,1,kp);
   SendRTMessage(HeatCtrl,PID_KI,1,ki);
   SendRTMessage(HeatCtrl,PID_KD,1,kd);
   SendRTMessage(HeatCtrl,PID_MINV,1,mn);
   SendRTMessage(HeatCtrl,PID_MAXV,1,mx);
   SendRTMessage(HeatCtrl,PID_NEWGAINS);  // Signal that new gains are available
	OpWinSetup->Close();	// Shutdown the window
   delete OpWinSetup;
	}


