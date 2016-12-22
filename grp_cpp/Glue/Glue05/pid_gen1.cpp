// Function definitions for the generic PID controller
// File: pid_gen1.cpp
// Created 10/5/95 by DM Auslander
// Modified 5/29/97, DM Auslander
// Uses a combination of global data and messages to avoid
//  the need for virtual functions.
// Does not use any state structure (control is either on or off)

#include <iostream.h>
#include <math.h>
#include "tasks.hpp"

PIDControl::PIDControl(char *name,double dtt,
            BaseTask *ActTask,int ActBox,
            BaseTask *ProcTask, int PrIndex,
            double aKp,double aKi,double aKd,double aMin,double aMax,
            double aSetpoint,int aControllerOn,
            int ProcNo)
                 : BaseTask (name,ProcNo)
     {
     dt = dtt;
     NextTaskTime = 0.0;
     set = aSetpoint;
     val = 0.0;    // Initial values
     integ = prev_set = prev_val = 0.0;
     kp = aKp;
     ki = aKi;
     kd = aKd;
     min = aMin;
     max = aMax;
     first = 1;
     ControllerOn = aControllerOn;
     ActuationTask = ActTask;
     ActuationBox = ActBox;
     ProcessTask = ProcTask;
     ProcValIndex = PrIndex;
     CreateGlobal(10);  // Create global data bases
     }
PIDControl::~PIDControl()
    {
    }

int PIDControl::Run (void) // Task function
    {
    double Time;
    double value;
    int flag;

    NumScans++; // Increment scan count
    if(!ControllerOn)
        {
        // No actuation is sent if controller is off --
        //   some other task is assumed to be handling that

        // Check for new gains. New gains can only be recognized
        //   when the controller is stopped. This could be changed
        //   by building a "bumpless" transfer into the controller.
        //   Otherwise, a gain change would put a large transient
        //   into the system.
        if(GetRTMessage(PID_START))
            {
            GetRTMessage(PID_STOP);  // Clear old STOP messages
            if(GetRTMessage(PID_NEWGAINS))
                {
                // Read new gains
                if(GetRTMessage(PID_KP,&flag,&value))kp = value;
                if(GetRTMessage(PID_KI,&flag,&value))ki = value;
                if(GetRTMessage(PID_KD,&flag,&value))kd = value;
                if(GetRTMessage(PID_MINV,&flag,&value))min = value;
                if(GetRTMessage(PID_MAXV,&flag,&value))max = value;
                }
            first = 1;  // Make sure PID behaves properly
                    // for its first run
            ControllerOn = 1;
            return(0);  // If an intermittent task,
                    // run again immediately
            }
        return(1);
        }

    if(GetRTMessage(PID_STOP))
        {
        ControllerOn = 0;  // Stop control
        return(1);
        }
    Time = GetTimeNow();
    if((Time >= NextTaskTime) || first)
        {
        //if(first)NextTaskTime = Time + dt;
        //else    NextTaskTime += dt;
        NextTaskTime = Time + dt;

        if(GetRTMessage(PID_SETPOINT,&flag,&value))set = value;
        val = GetGlobalData(ProcessTask,ProcValIndex);
        mc = PIDCalc(); // Do the PID calculation
        CopyGlobal();  // Make global information visible
        // Send out actuation value if this service has been
        //  requested.Otherwise, value can be obtained from the
        //  controller's global data
        if(ActuationTask != NULL)
            {
            // Send out the actuation value
            SendRTMessage(ActuationTask,ActuationBox,0,mc,MESSAGE_OVERWRITE);
            }
        first = 0;  // Not the first time anymore
        }
    return(1);     // Let task dispatcher go on to next task
    }

double PIDControl::PIDCalc(void) // Calculate the output of a PID controller
     // 'val' is the process variable value; 'set' is the setpoint
    {
    double m;    // Controller output
    double mpd;      // Portion of output due to P and D terms
    double itrial;       // Trial value for intergrator
    double err,prev_err;     // Errors
    double deriv;

    err = set - val;
    prev_err = prev_set - prev_val;     // Previous error
    if(first)deriv = 0.0;
    else deriv = kd * (err - prev_err) / dt;
    //else deriv = kd * (prev_val - val) / dt;

    mpd = kp * err + deriv;  // PD result only

    if(first)itrial = integ;  // Don't compute on first time
    else itrial = integ + ki * err * dt; // Trial value for integrator

    m = mpd + itrial;  // Trial controller result
    if((m <= max) && (m >= min))
        {
        // Controller is within limits
        integ = itrial;
        }
    else
        {
        // Controller is saturated
        // Leave integrator at its old value and bring output
        // within limits
        if(m > max)m = max;
        else m = min;
        }
    prev_set = set;
    prev_val = val;
    PrivateGlobalData[PID_ACTUATION] = m;
    PrivateGlobalData[PID_PROCESS] = val;
    PrivateGlobalData[PID_ERROR] = err;
    PrivateGlobalData[PID_SET] = set;
    return(m);
    }

