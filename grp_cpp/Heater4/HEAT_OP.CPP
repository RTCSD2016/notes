// Operator Interface for heater control system
// File: heat_op.cpp
// Created 10/18/95, DM Auslander

#include <iostream.h>
#include <math.h>
#include "tasks.hpp"

static int StopFlag=0;  // Used to signal system to shut down

void Stop(void)
   {
   StopFlag = 1;
   }

OperatorInterface::OperatorInterface(char *aName,int ProcNo)
          :BaseTask(aName,ProcNo)
    {
    // Create screens
    OpWin1 = new COperatorWindow ("Heater Control Operation");
    OpWin1->AddInputItem("Final Setpoint",&temp_final);
    OpWin1->AddOutputItem ("Time", &Time);
    OpWin1->AddOutputItem ("Setpoint", &setpoint);
    OpWin1->AddOutputItem ("Temperature", &temperature);
    OpWin1->AddOutputItem ("M-Cntrl", &mcntrl);

    OpWin1->AddKey ('X', "Stop", "Program", Stop);
    DeltaTaskTime = 0.05;   // Set screen refresh interval
    NextTaskTime = 0.0;
    State = 0;
    temp_final = 0.3;   // Initial value -- operator sets this
    CreateGlobal(8);  // Create global data bases
    ActivateProfiling(5.e-6,2.e-3,15,LOG_PROFILE);
    }

OperatorInterface::~OperatorInterface()     // Destructor for the class.
    {
    delete OpWin1;
    }

int OperatorInterface::Run(void)
    {
    int done = 1;   // Default return value indicating
            // done for this event

    NumScans++; // Increment scan count
    if (State == 0)
        {
        // Initialization code.  If there is anything
        // that the task needs to do only once at
        // start-up, put it here.
        State = 1;
        NextState = 1;
        OpWin1->Display();  // Initial screen display
        return (0);
        }

    // Only run the task if it is proper time.
    // This can be used to keep the screen from refreshing too fast
    // to be readable
    // Note:  It is not required that there be some time
    //      based criteria for when the task runs.  This
    //      section could be taken out, resulting in a task
    //      that runs every scan.
    if ((Time = GetTimeNow()) < NextTaskTime)
        return (done);      //  Done for now

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
                }
            // Action Section
            Time = GetTimeNow();

         temperature = GetGlobalData(HeatCtrl,PID_PROCESS);
         mcntrl = GetGlobalData(HeatCtrl,PID_ACTUATION);
         setpoint = GetGlobalData(HeatCtrl,PID_SET);

         OpWin1->Update();   // Check for user input and refresh outputs
         PrivateGlobalData[OPINT_FINALSETPOINT] = temp_final;
         PrivateGlobalData[OPINT_STOP] = (double)StopFlag;
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
    OpWinSetup->Display();  // Initial display
    while(!setup_done)OpWinSetup->Update();
    SetTickRate(tickrate);
   PrivateGlobalData[OPINT_KP] = kp;
   PrivateGlobalData[OPINT_KI] = ki;
   PrivateGlobalData[OPINT_KD] = kd;
   PrivateGlobalData[OPINT_MIN] = mn;
   PrivateGlobalData[OPINT_MAX] = mx;
   PrivateGlobalData[OPINT_NEWGAINS] = 1.0;
   CopyGlobal();  // Make data visible
    OpWinSetup->Close();    // Shutdown the window
   delete OpWinSetup;
    }


