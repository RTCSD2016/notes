// Operator Interface for heater control system
// File: heat_op.cpp
// Created 10/18/95, DM Auslander
// Modified 10/14/97, DM Auslander, add interface to Bridgeview (DDE)

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
   // Initial values
	temp_final = 0.3;
   setpoint = mcntrl = temperature = Time = 0.0;
	kp=10.0;
   ki=6.0;
   kd=0.0;
   min=0.0;
   max=1.0;
   ScreenNo = 0;

   #ifdef USE_CHAR_OP
	// Create screens
	OpWin1 = new COperatorWindow ("Heater Control Operation");
	OpWin1->AddInputItem("Final Setpoint",&temp_final);
	OpWin1->AddOutputItem ("Time", &Time);
	OpWin1->AddOutputItem ("Setpoint", &setpoint);
	OpWin1->AddOutputItem ("Temperature", &temperature);
	OpWin1->AddOutputItem ("M-Cntrl", &mcntrl);

	OpWin1->AddKey ('X', "Stop", "Program", Stop);
   #endif

   #ifdef USE_DDE
     // Set up DDE Server and items
     //  Create a DDE server object
     DDEHeater = new C_DDE_Server ("DDEHeater");

    //  Add a topic called "HeaterData", then add each of the variables
    // under that
    //  topic.  Each of the items is data meant to
    //  be sent or received by the server
    HeaterData = DDEHeater->AddTopic ("HeaterData");
    dde_setpoint = HeaterData->AddItem("dde_setpoint", setpoint);
    dde_mcntrl = HeaterData->AddItem("dde_mcntrl",mcntrl);
    dde_temperature = HeaterData->AddItem("dde_temperature",temperature);
    dde_Time = HeaterData->AddItem("dde_Time",Time);
    dde_kp = HeaterData->AddItem("dde_kp",kp);
    dde_ki = HeaterData->AddItem("dde_ki",ki);
    dde_kd = HeaterData->AddItem("dde_kd",kd);
    dde_min = HeaterData->AddItem("dde_min",min);
    dde_max = HeaterData->AddItem("dde_max",max);
    dde_tickrate = HeaterData->AddItem("dde_tickrate",tickrate);
    dde_start_op = HeaterData->AddItem("dde_start_op",start_op);
    dde_stop_prog = HeaterData->AddItem("dde_stop_prog",stop_prog);
    dde_temp_final = HeaterData->AddItem("dde_temp_final",temp_final);
    dde_ScreenNo = HeaterData->AddItem("dde_ScreenNo",ScreenNo);

    //  Call function to start DDE server running
    DDEHeater->Register_DDE ();

   #endif

	DeltaTaskTime = 0.05;	// Set screen refresh interval
	NextTaskTime = 0.0;
	State = 0;
   CreateGlobal(7);  // Create global data bases
   ActivateProfiling(5.e-6,2.e-3,15,LOG_PROFILE);
	}

OperatorInterface::~OperatorInterface()		// Destructor for the class.
	{
   #ifdef USE_CHAR_OP
	  delete OpWin1;
   #endif

   #ifdef USE_DDE
     delete DDEHeater;
   #endif
	}

int OperatorInterface::Run(void)
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

      #ifdef USE_CHAR_OP
		  OpWin1->Display();	// Initial screen display
      #endif
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
            #ifdef USE_DDE
               ScreenNo = 1;
              // Set the display screen number
              while(!dde_ScreenNo->SetValue(ScreenNo));  // Repeated calls are necessary
               // to maintain a "thread-safe" transfer

            #endif
				}
			// Action Section
			Time = GetTimeNow();

         temperature = GetGlobalData(HeatCtrl,PID_PROCESS);
         mcntrl = GetGlobalData(HeatCtrl,PID_ACTUATION);
         setpoint = GetGlobalData(HeatCtrl,PID_SET);

         #ifdef USE_CHAR_OP
			  OpWin1->Update();	// Check for user input and refresh outputs
         #endif

         #ifdef USE_DDE
            while(!dde_Time->SetValue(Time)) ; // Make sure outputs are sent
            while(!dde_setpoint->SetValue(setpoint)) ;
            while(!dde_temperature->SetValue(temperature)) ;
            while(!dde_mcntrl->SetValue(mcntrl)) ;

            dde_temp_final->GetValue(temp_final);
            dde_stop_prog->GetValue(stop_prog);
            if(stop_prog)Stop();
         #endif
         PrivateGlobalData[OPINT_FINALSETPOINT] = temp_final;
         CopyGlobal();  // Make global data visible

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
   #ifdef USE_CHAR_OP
	  COperatorWindow *OpWinSetup = new COperatorWindow("Setup Initial Data");
   #endif

	tickrate = GetTickRate();
	setup_done = 0;

   #ifdef USE_CHAR_OP
 	  OpWinSetup->AddInputItem("Tick Rate",&tickrate);
	  OpWinSetup->AddInputItem("Kp",&kp);
	  OpWinSetup->AddInputItem("Ki",&ki);
	  OpWinSetup->AddInputItem("Kd",&kd);
	  OpWinSetup->AddKey ('X', "Start", "Control", SetupSetDone);
	  OpWinSetup->Display();	// Initial display
	  while(!setup_done)OpWinSetup->Update();
   #endif

   #ifdef USE_DDE
     ScreenNo = 0;  // Set Up Screen
   //// debug
   cout << "kp ki kd min max start_op tickrate\n";
     start_op = 0;  // default value

     // Set the display screen number
     while(!dde_ScreenNo->SetValue(ScreenNo));  // Repeated calls are necessary
            // to maintain a "thread-safe" transfer

     while(!setup_done)
      {
      // Keep this screen alive until the user closes it
      dde_kp->GetValue(kp);
      dde_ki->GetValue(ki);
      dde_kd->GetValue(kd);
      dde_min->GetValue(min);
      dde_max->GetValue(max);
      dde_tickrate->GetValue(tickrate);
      dde_start_op->GetValue(start_op);
      if(start_op == 1)SetupSetDone();;
   ///////// Debug ///////////////
   cout << kp << " " << ki << " " << kd << " " << min << " " << max
     << " " << start_op << " " << tickrate << "            \r";
      }
   #endif

	SetTickRate(tickrate);
   PrivateGlobalData[OPINT_KP] = kp;
   PrivateGlobalData[OPINT_KI] = ki;
   PrivateGlobalData[OPINT_KD] = kd;
   PrivateGlobalData[OPINT_MIN] = min;
   PrivateGlobalData[OPINT_MAX] = max;
   PrivateGlobalData[OPINT_NEWGAINS] = 1.0;
   CopyGlobal();  // Make data visible

   #ifdef USE_CHAR_OP
	  OpWinSetup->Close();	// Shutdown the window
     delete OpWinSetup;
   #endif
	}

