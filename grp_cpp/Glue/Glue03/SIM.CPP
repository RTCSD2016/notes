// Glue/manufacturing example -- this file contains all of the
//  simulation related material
// File: sim.cpp
// Created May 27, 1997, DM Auslander

// 1. Added thermal simulation of the ovens to version 0

#include <math.h>
#include "tasks.hpp"

CBeltAxis::CBeltAxis(char *aName,double JJ,double bb,double Ktt,int ProcNo)
          :BaseTask(aName,ProcNo)
    {
    DeltaTaskTime = 0.02; // Nominal task period, sec.
    NextTaskTime = 0.0;
    State = 0;         // set to initialize
    previous_time = 0.0;
    CreateGlobal(5);  // Create a global data base with 5 slots
    // ActivateProfiling(5.e-6,80.e-6,15,LOG_PROFILE);  // Not needed yet
    // Set parameters
    J = JJ;  // intertia
    b = bb;  // damping
    Kt = Ktt;
    // Set initial conditions
    x = 0.0;  // Position and velocity
    v = 0.0;
    cur = 0.0;
    }

CBeltAxis::~CBeltAxis(void){};

int CBeltAxis::Run(void)
    {
    double Time;
    NumScans++; // Increment scan count

    if (State == 0)
        {
        // Initialization code.  If there is anything
        // that the task needs to do only once at
        // start-up, put it here.
        State = 1;  // To prevent this from running again
        previous_time = NextTaskTime = GetTimeNow();
        return (1);
        }

    Time = GetTimeNow();
    // Check for time to do the next simulation computations
    if(Time >= NextTaskTime)
        {
        NextTaskTime += DeltaTaskTime;

        double delt;

        Time = GetTimeNow();
        delt = Time - previous_time;  // Compute actual time
                // increment for Euler integration
        previous_time = Time;

        // Check to see if a new value for motor current has arrived
        int MsgFlag;
        double val;
        if(GetRTMessage(BELTAXISMSG_Cur,&MsgFlag,&val))cur = val;

        // Start belt axis simulation
        double torque = Kt * cur - b * v;
        v += (torque / J) * delt;
        x += v * delt;

        // Copy data to local arrays
        PrivateGlobalData[BELTAXIS_x] = x;
        PrivateGlobalData[BELTAXIS_v] = v;
        CopyGlobal();  // Make data available to other tasks
        }

    return (1);
}

CThOven::CThOven(char *aName,double aK,double aThAmb,
                double aR,double aHCap,int ProcNo)
          :BaseTask(aName,ProcNo)
    {
    DeltaTaskTime = 0.1; // Nominal task period, sec.
    NextTaskTime = 0.0;
    State = 0;         // set to initialize
    previous_time = 0.0;
    CreateGlobal(5);  // Create a global data base with 5 slots
    // ActivateProfiling(5.e-6,80.e-6,15,LOG_PROFILE);  // Not needed yet
    // Set parameters
    K = aK;
    ThAmb = aThAmb;
    R = aR;
    Cur = 0.0;
    HCap = aHCap;
    // Set initial conditions
    Th = 0;  // Initial condition
    }

CThOven::~CThOven(void){};

int CThOven::Run(void)
    {
    double Time;
    NumScans++; // Increment scan count

    if (State == 0)
        {
        // Initialization code.  If there is anything
        // that the task needs to do only once at
        // start-up, put it here.
        State = 1;  // To prevent this from running again
        previous_time = NextTaskTime = GetTimeNow();
        return (1);
        }

    Time = GetTimeNow();
    // Check for time to do the next simulation computations
    if(Time >= NextTaskTime)
        {
        NextTaskTime += DeltaTaskTime;

        double delt;

        Time = GetTimeNow();
        delt = Time - previous_time;  // Compute actual time
                // increment for Euler integration
        previous_time = Time;

        // Start belt axis simulation
       // Check to see if a new value for heater current has arrived
        int MsgFlag;
        double val;
        if(GetRTMessage(THOVEN_MSG_CUR,&MsgFlag,&val))Cur = val;


        double HeatIn = Cur * Cur * R;
        double HeatOut = K * (Th - ThAmb);
        Th += ((HeatIn - HeatOut) / HCap)* delt;

        // Copy data to local arrays
        PrivateGlobalData[THOVEN_Th] = Th;
        CopyGlobal();  // Make data available to other tasks
        }
    return (1);
}

