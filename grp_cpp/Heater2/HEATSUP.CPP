// HeatSup Sets the setpoint for the heater control
// File:  HeatSup.cpp
// Created 8/2/95 by Jason Jones
// Modified 10/5/95 by DM Auslander, changed to setpoint generator

#include <iostream.h>
#include <math.h>
#include "tasks.hpp"

HeatSupTask::HeatSupTask(char *aName)	// Constructor
		  :BaseTask(aName)
	{
	DeltaTaskTime = 0.1; // Task period
	NextTaskTime = 0.0;
	State = 0;	       // set to initialize
	NextState = 0;
	temp_init = temp = 0.3;
	temp_final = 0.0; // Legal value -- operator sets this
	temp_soak = 0.7;
	soak_time = 3.0;
	setpt = temp_init;
	}

HeatSupTask::~HeatSupTask(void){};

int HeatSupTask::Run(void)
	{
	// Initialization code.  If there is anything
	// that the task needs to do only once at
	// start-up, put it here.
	//double Time;
	int done = 1;	// Default return value indicating
			// done for this event
   int msg_flag;
   double msg_val;

	NumScans++;	// Increment scan count

	if (State == 0)
		{
		State = 1;
		NextState = 1;
		return (0);
		}

	// Only run the task if it is proper time.
	// Note:  It is not required that there be some time
	//		based criteria for when the task runs.  This
	//		section could be taken out, resulting in a task
	//		that runs every scan.
	if ((GetTimeNow()) < NextTaskTime)
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
		case 1:  // Start controller
			// Entry Section
			if (RunEntry)
				{
				// only run this on entry to the state
				// entry code goes here
				setpt = temp_soak;
            SendRTMessage(HeatCtrl,PID_SETPOINT,1,setpt,MESSAGE_OVERWRITE);
            SendRTMessage(OpInt,OPINT_SETPOINT,1,setpt,MESSAGE_OVERWRITE);
            SendRTMessage(DataLogger,DATALOGGER_SET,1,setpt,MESSAGE_OVERWRITE);

            SendRTMessage(HeatCtrl,PID_START,1,0.0,
                       MESSAGE_NORMAL,HEATSUP_PIDCONFIRM);
             // Send message to start control -- return receipt will go
             // to indicated message box
				}
         // No action section

         // Test/Exit Section
         int rv;
         if(rv = GetRTMessage(HEATSUP_PIDCONFIRM,&msg_flag))
         	{
            // Make sure message was delivered
            if((msg_flag != 1) || (rv == -1))
            	{
               StopControl("Control start messsage not delivered\n");
               return(1);
               }
            // Yes, it was delivered
            NextState = 2;
            done = 0;
            break;
            }
      	// No confirmation yet, stay in this state
         done = 1;
         break;

   	case 2: // Bring temperature up to soaking temperature
			// Action Section
         if(GetRTMessage(HEATSUP_TEMPERATURE,&msg_flag,&msg_val))temp = msg_val;
			// Test/Exit section
				if(temp > (temp_soak - 0.03))
					{
					NextState = 3;		// Go into the Soak state
					done = 0;		// Run next state immediately
					}
			break;

		case 3: // Soak State -- hold for specified time
			// Entry Section
			if(RunEntry)
				{
				soak_end = GetTimeNow() + soak_time;	// Record time at start
				}
			// Action Section

			// Test/Exit Section
			if(GetTimeNow() >= soak_end)
				{
				NextState = 4;		// Bring temperature to final value
				done = 0;	// Go to next state immediately
				}
			break;

		case 4:	// Final state -- bring temperature to temp_final
			// Entry Section
			if(RunEntry)
				{
   			setpt = temp_final;
            SendRTMessage(HeatCtrl,PID_SETPOINT,1,setpt,MESSAGE_OVERWRITE);
            SendRTMessage(OpInt,OPINT_SETPOINT,1,setpt,MESSAGE_OVERWRITE);
            SendRTMessage(DataLogger,DATALOGGER_SET,1,setpt,MESSAGE_OVERWRITE);
				}

			// Action Section
         if(GetRTMessage(HEATSUP_TEMPFINAL,&msg_flag,&msg_val))
            {
            // Update if new value received
            temp_final = msg_val;
   			setpt = temp_final;
            SendRTMessage(HeatCtrl,PID_SETPOINT,1,setpt,MESSAGE_OVERWRITE);
            SendRTMessage(OpInt,OPINT_SETPOINT,1,setpt,MESSAGE_OVERWRITE);
            SendRTMessage(DataLogger,DATALOGGER_SET,1,setpt,MESSAGE_OVERWRITE);
            }

			// Test/Exit section
			break;

		default:  // check for an illegal state.
			cout << "\nIllegal State assignment, Task1\n";
			return (-1);
		}
	return (done);
}

