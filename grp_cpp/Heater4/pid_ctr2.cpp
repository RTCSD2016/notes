// Function definitions for the generic PID controller
// File: pid_ctr2.cpp
// Created 10/5/95 by DM Auslander
// Modified from original PID, 10/3/96, DM Auslander to use message
//  passing instead of functions calls for data exchange

#include <iostream.h>
#include <math.h>
#include "tasks.hpp"

PIDControl::PIDControl(char *name,double dtt,int ProcNo)   // Constructor
	 : BaseTask (name,ProcNo)
	 {
	 dt = dtt;
    NextTaskTime = 0.0;
	 set = val = 0.0;    // Initial values
	 integ = prev_set = prev_val = 0.0;
	 kp = 1.0;
	 ki =  kd = 0.0;
	 min = -1.0;
	 max = 1.0;
	 State = 0;      // Set the initial task state
	 NextState = 0;
	 start_flag = 0;
    CreateGlobal(10);  // Create global data bases
	 }

int PIDControl::Run (void) // Task function
	 {
	 // This task has three states - a "waiting" state when it first turns on and
	 // two states to do the control -- the first (state 2) does the control
    // calculation in its ENTRY section, the other (state 3) waits for the
    // next sample instant to run the control.

    // The variable 'State' is inherited from BaseTask
	double Time;

	NumScans++;	// Increment scan count

	if (State == 0)
		{
		State = 1;
		NextState = 1;
		return (0);
		}

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

	 switch(State)
		  {
		  case 1:     // Waiting for 'start' signal
				// No Entry section

				// Test/Exit section
				if(GetRTMessage(PID_START))  // Wait for "start" message
					 {
					 NextState = 2;
					 return(0);     // Set new state and return. If in min-latency
							// continue in this task
					 }
				else
					 {
					 // Stay in 'wait' state
					 return(1); // Indicate that the task can inactivate until next sample time
					 }

		  case 2:     // Run the control algorithm
				{
				// Entry Section -- that is where all the work is for this state
				if(RunEntry)
					{
					GetGains(&kp,&ki,&kd,&min,&max);
               set = GetSetpoint();

					val = GetProcVal(); // Get the process output
					mc = PIDCalc(); // Do the PID calculation
					SetActVal(mc);      // Send out the actuation value
               CopyGlobal();  // Make global information visible
					}
				// Action Section (empty)

				// Test/Exit Section
               NextState = 3;  // Wait for next control sample instant
					return(1);     // Let task dispatcher go on to next task
					}
				break;  // Not really needed here but left in case code
            		// is added.
		  case 3:     // Wait for the next time to run, then go
               // back to state 2
            // No ENTRY or ACTION sections
				{
				// Test/Exit Section
				Time = GetTimeNow();
				// Check for time to do the next simulation computations
				if(Time >= NextTaskTime)
               {
				   NextState = 2;	// Self transition
				   NextTaskTime += dt;
               return(0);  // Continue in this task (if min-latency)
               }
			   return(1);     // Let task dispatcher go on to next task
				break;  // Not really needed here but left in case code
            		// is added.
			default:  // check for an illegal state.
				cout << "\nIllegal State assignment, Task1\n";
				return (-1);
				}
		  }
	 return(1);	// default return
	 }

double PIDControl::PIDCalc(void) // Calculate the output of a PID controller
	 // 'val' is the process variable value; 'set' is the setpoint
    {
    double m;    // Controller output
    double mpd;      // Portion of output due to P and D terms
    double itrial;       // Trial value for intergrator
    double err,prev_err;     // Errors

    err = set - val;
    prev_err = prev_set - prev_val;     // Previous error
    mpd = kp * err + kd * (err - prev_err) / dt;
    if(mpd > max)mpd = max; // Keep partial result within limits
    if(mpd < min)mpd = min;

    itrial = integ + ki * err * dt; // Trial value for integrator
	 m = mpd + itrial;

	 if(itrial >= 0)
        {
        // Check the 'max' limit
        if(m > max)
            {
            integ = max - mpd;
            m = max;
            }
        else
            {
            // Not in limit
            integ = itrial;
            }
        }
    else
        {
        // itrial < 0 -- check the 'min' limit
		  if(m < min)
            {
				integ = min - mpd;
            m = min;
            }
        else
            {
            integ = itrial;
				}
        }
    prev_set = set;
    prev_val = val;
    PrivateGlobalData[PID_ACTUATION] = m;
    PrivateGlobalData[PID_PROCESS] = val;
    PrivateGlobalData[PID_ERROR] = err;
    PrivateGlobalData[PID_SET] = set;
	 return(m);
    }

