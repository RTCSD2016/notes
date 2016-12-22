//*************************************************************************************
//  DAQ_Simu.cpp
//      This file contains the code for a class which simulates the behavior of a
//      simple linear second-order dynamic system.  It's meant to be used to simulate
//      the behavior of some of the ME230 lab devices such as the DC motors.  
//
//  Revisions:
//      12-19-95  DMA/JCJ  Created based on existing examples
//       8-19-97  JRR      Cleaned up, comments modified
//*************************************************************************************

#include "tasks.hpp"

#ifdef SIMULATION  //  We only use this class if in simulation mode

//-------------------------------------------------------------------------------------
//  Constructor:  CSimulationTask
//      This constructor sets up the simulation task object.

CSimulationTask::CSimulationTask (char *aName) : BaseTask (aName)
    {
    DeltaTaskTime = 0.001;      //  This is the task's "sample time", the period of
                                //  time between successive runs of the Run function
    NextTaskTime = 0.0;         //  The next time this task will be ready to run
    State = 0;                  //  This is the initial state for the task
    NextState = 0;              //  Use this elsewhere to cause state transitions

    Position = 0.0;             //  Set the state and inputs to default conditions
    Velocity = 0.0;             //  of all zeros
    Input = 0.0;
    xc_Input = 0.0;

    SetUpCorrectly = 0x00;      //  No setup functions have been called yet
    }


//-------------------------------------------------------------------------------------
//  Destructor:  ~CSimulationTask
//      The destructor for the task deletes any dynamically allocated memory items.

CSimulationTask::~CSimulationTask (void)
    {
    }


//-------------------------------------------------------------------------------------
//  Function:  SetInitialConditions
//      This function is used to set the initial conditions of the simulation.  It is
//      meant to be called from the user's main program.

void CSimulationTask::SetInitialConditions (double aPos, double aVel)
    {
    Position = aPos;
    Velocity = aVel;

    SetUpCorrectly |= 0x01;                 //  Flag records this function was called
    }


//-------------------------------------------------------------------------------------
//  Function:  SetSimulationParameters
//      This function is used to set the initial conditions of the simulation.  It is
//      meant to be called from the user's main program.

void CSimulationTask::SetSimulationParameters (double aGain, double aCon,
                                               double aFric, double aMass)
    {
    InputGain = aGain;
    SpringConstant = aCon;
    FrictionRate = aFric;
    Mass = aMass;

    SetUpCorrectly |= 0x02;                 //  Flag records this function was called
    }


//-------------------------------------------------------------------------------------
//  Function:  GetOutput
//      This function returns the current value of one of the system's simulated out-
//      puts.  The outputs (note, not outputs in the control-theory sense, just things
//      which we might want to measure) are:
//          - Channel 0: position
//          - Channel 1: velocity
//          - Channel 2: 0.0 (not used) 

double CSimulationTask::GetOutput (int aChannel)
    {
    switch (aChannel)
        {
        case 0:
            return (xc_Position);
            break;
        case 1:
            return (xc_Velocity);
            break;
        case 2:
            return (0.0);
            break;
        default:
            StopControl ("Somebody asked for invalid simulation output, channel %d",
                         aChannel);
        }
    }


//-------------------------------------------------------------------------------------
//  Function:  Run
//      This is the function which the task dispatcher runs once every sample time (or
//      for low priority tasks, whenever there's some free time).  Most of the user's
//      task code will go in here.  You must always supply a Run() function for every
//      one of your tasks.

int CSimulationTask::Run (void)
    {
    double TheTimeRightNow;         //  Local variable for temporary storage of time
    int done = 1;                   //  Return value indicating done for this scan
    NumScans++;                     //  Increment scan count

    static double PreviousTime;     //  Used to calculate duration of a scan
    double DeltaTime;               //  Time between previous scan and this one
    double ddt_Velocity;            //  Rate of change of velocity
    double ddt_Position;            //  and rate of change of position

    //  Task timing code:  This code decides if the task should be run now
    if ((TheTimeRightNow = GetTimeNow ()) < NextTaskTime)
        return (done);
    NextTaskTime += DeltaTaskTime;

    //  Initialization code:  If there is anything which the task needs to do just
    //  once when it first runs, put it below.  (Anything which should be done before
    //  the scheduler even starts can be put into the constructor.)
    if (State == 0)
        {
        //  Make sure the simulation parameters were actually set up by the user
        if (SetUpCorrectly != 0x03)
            StopControl ("Attempt to run simulation without setting it up first\n");

        //  Get an initial time to use for starting up the Euler integrator
        PreviousTime = TheTimeRightNow;

        State = 1;
        NextState = 1;
        return 0;
        }

    //  This section checks if there has been a state transition
    if (NextState != -1)
        {
        AuditTrail (this, State, NextState);    //  Make a record of state transition
        State = NextState;
        NextState = -1;
        RunEntry = 1;
        }
    else
        RunEntry = 0;

    //  Run the code for whichever state the transition logic system is in now
    switch (State)
        {
        //  State 1 -- Simulate the system.  This is the only state other than 0
        case 1:
            //  Entry code goes here, but there isn't any

            //  Action Section
            //  Run a simulation step.  First get input exchange variables
            CriticalSectionBegin ();
            Input = xc_Input;
            CriticalSectionEnd ();

            //  Figure out how long the integration time step has been
            DeltaTime = TheTimeRightNow - PreviousTime;
            PreviousTime = TheTimeRightNow;

            //  Calculate current values of state derivatives from dynamic equations
            ddt_Velocity = (1.0 / Mass) * ((Input * InputGain)
                           - (FrictionRate * Velocity) - (SpringConstant * Position));
            ddt_Position = Velocity;

            //  Now that we have the state derivatives, integrate them
            Position += ddt_Position * DeltaTime;
            Velocity += ddt_Velocity * DeltaTime;

            //  Make protected output copies of these variables so that other tasks
            //  can safely grab them when needed
            CriticalSectionBegin ();
            xc_Position = Position;
            xc_Velocity = Velocity;
            CriticalSectionEnd ();

            //  Test/Exit section:  There will be no transitions out of state 1, ever
            break;

        //  The default case will only be entered if there has been a transition to a
        //  nonexistent state.  This is an error condition, so complain about it
        default:
            StopControl ("ERROR: Transition to illegal state %d in task \"%s\"",
                         State, Name);
            return (-1);
        }
    return (done);
    }

#endif  //  SIMULATION
    
