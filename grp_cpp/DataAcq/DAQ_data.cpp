//*************************************************************************************
//  DAQ_Data.cpp
//      This file contains source code for an object which records real or simulated
//      data for the ME230 Lab 1 Data Acquisition program.  
//
//  Revisions:
//      12-19-95  DMA/JCJ  Created based on existing examples
//       8-19-97  JRR      Cleaned up, comments modified
//*************************************************************************************

#include "tasks.hpp"


//-------------------------------------------------------------------------------------
//  Constructor:  C_DAQ_DataTask
//      This constructor creates the C_DAQ_DataTask object.

C_DAQ_DataTask::C_DAQ_DataTask (char *aName) : BaseTask (aName)
    {
    //  The following variables belong to class BaseTask.  We set their values here
    //  in order to control the timing parameters and transition-logic state of our
    //  task.
    DeltaTaskTime = 0.01;       //  This is the task's "sample time", the period of
                                //  time between successive runs of the Run function
    NextTaskTime = 0.0;         //  The next time this task will be ready to run
    State = 0;                  //  This is the initial state for the task
    NextState = 0;              //  Use this elsewhere to cause state transitions

    //  Create a data logger object which will store our data until test is finished
    TheLogger = new DataOutput (NUM_CHANNELS, 0.0, 20.0, 0.02);

    #ifdef REAL_IO                  //  If we're using a real I/O interface card, we
        io_init ();                 //  must initialize it before using it
        d2a (0, 0.0);               //  Turn off that motor before testing
    #endif
    }


//-------------------------------------------------------------------------------------
//  Destructor:  ~C_DAQ_DataTask
//      This destructor closes down the data acquisition system by deleting the data
//      logger object and closing the I/O driver if needed.

C_DAQ_DataTask::~C_DAQ_DataTask (void)
    {
    //  Ask the data logger to write its data, then delete it
    TheLogger->WriteOutputFile ("Data_Acq.txt");
    delete TheLogger;

    #ifdef REAL_IO              //  If we're using a real I/O interface card, we
        io_close ();            //  should close the driver when we're done with it
    #endif
    }


//-------------------------------------------------------------------------------------
//  Function:  Run
//      This is the function which the task dispatcher runs once every sample time (or
//      for low priority tasks, whenever there's some free time).

int C_DAQ_DataTask::Run (void)
    {
    int Index;                  //  Just an array index counter
    double TheTimeNow;          //  Local variable used for temporary storage of time
    int done = 1;               //  Default return value indicating done for this scan
    NumScans++;                 //  Increment scan count

    //  This code decides if the task should be run at the current time
    if ((TheTimeNow = GetTimeNow()) < NextTaskTime)
        return (done);
    NextTaskTime += DeltaTaskTime;

    //  Initialization code:  This code runs once when first starting the scheduler
    if (State == 0)
        {
        StartTime = TheTimeNow;             //  Record the time things started up

        State = 1;
        NextState = 1;
        return 0;
        }

    //  This section checks if there has been a state transition
    if (NextState != -1)
        {
        AuditTrail (this, State, NextState);    //  Record the state transition
        State = NextState;
        NextState = -1;
        RunEntry = 1;
        }
    else
        RunEntry = 0;

    //  Do whatever's appropriate for the given state
    switch (State)
        {
        //  State 1:  Running the test.  The output voltage is set in the Entry
        //            section of code, and it remains there until the test is over
        case 1:
            if (RunEntry)
                {
                CriticalSectionBegin ();
                StepHeight = xc_StepHeight;
                CriticalSectionEnd ();
                SetActuationVoltage (StepHeight); 
                }
            //  Action Section:  Test is running.  Get the current voltages from the
            //  plant output and save them in the data log.  If you change the number
            //  of channels measured, change the number of items in AddData()
            for (Index = 0; Index < NUM_CHANNELS; Index++)
                Output[Index] = MeasureVoltage (Index);
            TheLogger->AddData (Output[0], Output[1], Output[2], END_OF_ARGS);

            //  Do a protected transfer of the output data
            CriticalSectionBegin ();
            for (Index = 0; Index < NUM_CHANNELS; Index++)
                xc_Output[Index] = Output[Index];
            CriticalSectionEnd ();

            //  Test/Exit section:  If the time has come to quit the test, tell the
            //  other tasks it's time to quit, then go into the last state
            if (TheTimeNow > EndingTime)
                {
                DAQ_InterfaceTask->EndTheTest ();
                
                #ifdef REAL_IO                      //  Oh yeah, turn the actuation
                    SetActuationVoltage (0.0);      //  off as the test ends
                #endif
                NextState = 2;                      //  Go into the final state
                done = 0;                           //  Run next state immediately
                }
            break;

        //  State2: This state is entered when the test is ending; it does nothing
        case 2:
            //  Entry Section:  Empty
            //  Action Section:  It's empty too
            //  Test/Exit Section:  What were you expecting?
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


//-------------------------------------------------------------------------------------
//  Function:  SetActuationVoltage
//      This function sends an actuation voltage to the real or simulated plant.

void C_DAQ_DataTask::SetActuationVoltage (double aVoltage)
    {
    #ifdef SIMULATION
        //  For a simulated plant, just tell the simulator what its input voltage is
        SimulationTask->SetInput (aVoltage);
    #endif
    #ifdef REAL_IO
        //  For a real plant, we have to send out the real voltage
        d2a (OUTPUT_CHANNEL, aVoltage);
    #endif
    }


//-------------------------------------------------------------------------------------
//  Function:  MeasureVoltage
//      This function gets the voltage at the output of the plant from the I/O card.

double C_DAQ_DataTask::MeasureVoltage (int aChannel)
    {
    #ifdef SIMULATION
        //  For a simulated plant, the outputs come from the simulation task
        return (SimulationTask->GetOutput (aChannel));
    #endif
    #ifdef REAL_IO
        //  For a real plant, we use the I/O card to measure the voltages
        return (a2d (aChannel));
    #endif
    }

