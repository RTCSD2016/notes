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
         kp = x_kp = 10.0;   // Initial controller gains
         ki = x_ki = 6.0;               // Make sure to set transfer (x) values also!
         kd = x_kd = 0.0;
         set = x_set = 0.0;  // Initial set point
         min = x_min = 0.0;             // Duty cycle actuation, positive only
         max = x_max = 1.0;
         }

void HeaterControl::SetActVal(double val)     // Set the actuation value
         {
         PWM->SetDutyCycle(val);
         }

double HeaterControl::GetProcVal(void)
         {
         double temp;

         temp = SimTask->GetTemperature();
         return(temp);
         }

