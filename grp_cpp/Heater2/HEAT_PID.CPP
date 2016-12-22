// Function definitions for the heater PID controller
// File: heat_pid.cpp
// The control class is derived from the generic PID controller
// Only the virtual functions are defined here
// Created 10/5/95 by DM Auslander
#include <iostream.h>
#include <math.h>
#include "tasks.hpp"

HeaterControl::HeaterControl(char *name) : PIDControl(name,0.1)
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
         }

void HeaterControl::SetActVal(double val)     // Set the actuation value
         {
         // Send value to tasks that need it
         SendRTMessage(PWM,PWM_DUTYCYCLE,1,val,MESSAGE_OVERWRITE);
         SendRTMessage(OpInt,OPINT_MC,1,val,MESSAGE_OVERWRITE);
         SendRTMessage(DataLogger,DATALOGGER_MC,1,val,MESSAGE_OVERWRITE);
         }

double HeaterControl::GetProcVal(void)
         {
         double temp=0.0,val;
         int flag;

         if(GetRTMessage(HEATCTRL_TEMPERATURE,&flag,&val))temp = val;

         // Send temperature to any other tasks that need it
         SendRTMessage(HeatSup,HEATSUP_TEMPERATURE,1,temp,MESSAGE_OVERWRITE);
         SendRTMessage(OpInt,OPINT_TEMPERATURE,1,temp,MESSAGE_OVERWRITE);
         SendRTMessage(DataLogger,DATALOGGER_VAL,1,temp,MESSAGE_OVERWRITE);
         return(temp);
         }

