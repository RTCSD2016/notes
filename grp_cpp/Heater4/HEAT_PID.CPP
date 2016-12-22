// Function definitions for the heater PID controller
// File: heat_pid.cpp
// The control class is derived from the generic PID controller
// Only the virtual functions are defined here
// Created 10/5/95 by DM Auslander
#include <iostream.h>
#include <math.h>
#include "tasks.hpp"

HeaterControl::HeaterControl(char *name,int ProcNo)
         : PIDControl(name,0.1,ProcNo)
                  // Call base class constructor also -- 2nd argument
                  // is the sample time for the controller
         {
         // Give legal values to the gains. They will be set from
         // outside this class (often by operator interface)
         kp = 0.0;
         ki = 0.0;               // Make sure to set transfer (x) values also!
         kd = 0.0;
         set = 0.0;
         min = 0.0;
         max = 0.0;
         ActivateProfiling(5.e-6,80.e-6,15,LOG_PROFILE);
         }

// These are all virtual functions in the PID class definition         
void HeaterControl::SetActVal(double val)     // Set the actuation value
         {
         // Nothing to do in this simulation version
         }

double HeaterControl::GetProcVal(void)
         {
         double temp;

         temp = GetGlobalData(SimTask,SIM_TEMP);  // Get temperature
           // from the simulation task

         return(temp);
         }

double HeaterControl::GetSetpoint(void)
   {
   double setpoint;

   setpoint = GetGlobalData(HeatSup,HEATSUP_SETPOINT);
   return(setpoint);
   }

void HeaterControl::GetGains(double *kp,double *ki,double *kd,
        		double *min,double *max)
	{
	if(GetGlobalData(OpInt,OPINT_NEWGAINS) == 0.0)return;  // No new values
	*kp = GetGlobalData(OpInt,OPINT_KP);
	*ki = GetGlobalData(OpInt,OPINT_KI);
	*kd = GetGlobalData(OpInt,OPINT_KD);
	*min = GetGlobalData(OpInt,OPINT_MIN);
	*max = GetGlobalData(OpInt,OPINT_MAX);
   }

