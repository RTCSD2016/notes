//*************************************************************************************
//  DAQ_Oper.cpp
//      This is the source file for the operator window task of the ME230 Lab 1 Data
//      Acquisition program.
//
//  Revisions:
//      12-19-95  DMA/JCJ  Created based on existing examples
//       8-19-97  JRR      Cleaned up, comments modified
//       8-20-97  JRR      Created DAQ_Oper.cpp
//*************************************************************************************

#include "tasks.hpp"


bool ExitSetupNow = false;                  //  Flag to exit setup phase of program

//  The following constants are defined here for convenience.  They will be copied
//  into the variables which are used in the setup window to allow the user to adjust
//  the parameters of the experiment before the experiment begins to run
#define DEF_TICK_RATE    0.001              //  Default tick rate in seconds
#define DEF_SAMP_TIME    0.01               //  Sampling time for taking data 
#define DEF_RUN_TIME    10.0                //  How long the test will take
#define DEF_STEP_SIZE    2.5                //  Height of step response voltage
#define DEF_INPUT_GAIN   1.0                //  Gain from input to actuation
#define DEF_MASS         1.0                //  Mass of moving item
#define DEF_FRICTION     1.0                //  Friction coefficient
#define DEF_SPR_CON      1.0                //  Spring constant


//-------------------------------------------------------------------------------------
//  Function:  ExitSetup
//      This function exits the setup window so that the main function can go on to
//      actually running the experiment.

void ExitSetup (void)
    {
    ExitSetupNow = true;
    }


//-------------------------------------------------------------------------------------
//  Function:  GetSetupParameters
//      This function is called before the data acquisition program's scheduler runs.
//      It prompts the user for the most important scheduling parameters such as
//      scheduler tick time, data sampling time, and so on.  The parameters to the
//      function are those items which are being passed back to main().

void GetSetupParameters (double& TickRate)
    {
//    char FileName[32] = "Data_Acq.txt";     //  Name for the data output file
    double SamplingTime = DEF_SAMP_TIME;    //  Sampling time for taking data
    double RunTime = DEF_RUN_TIME;          //  How long the test will take
    double StepSize = DEF_STEP_SIZE;        //  Height of the step response step
    double InputGain = DEF_INPUT_GAIN;      //  Gain from input to actuation
    double Mass = DEF_MASS;                 //  Mass of moving item
    double Friction = DEF_FRICTION;         //  Friction coefficient
    double SpringConst = DEF_SPR_CON;       //  Spring constant

    TickRate = DEF_TICK_RATE;               //  Set default scheduler tick rate

    //  Create an operator window object and set up its input, output, and key items
    COperatorWindow* pOpWin = new COperatorWindow ("Data Acquisition Setup");

    pOpWin->AddInputItem ("Tick Rate", &TickRate);
    pOpWin->AddInputItem ("Run Time", &RunTime);
    pOpWin->AddInputItem ("Sample Time", &SamplingTime);
    pOpWin->AddInputItem ("Step (0-10 V)", &StepSize);
    #ifdef SIMULATION  //  Only ask for simulation parameters if simulating
        pOpWin->AddInputItem ("Input Gain", &InputGain);
        pOpWin->AddInputItem ("Mass", &Mass);
        pOpWin->AddInputItem ("Friction", &Friction);
        pOpWin->AddInputItem ("Spring K", &SpringConst);
    #endif
    pOpWin->AddKey ('T', "Start", "Test", ExitSetup);

    //  Display the operator interface for the first time
    pOpWin->Display();

    //  Go into a loop whose only purpose is to run the setup window.  The user will
    //  edit the setup values, then press ^T or ^X when it's time to stop setting up
    while (ExitSetupNow == false)
        pOpWin->Update ();

    //  Close and delete the operator interface object
    pOpWin->Close ();
    delete pOpWin;

    //  Now that we have all these numbers, send them to the tasks which need them
    DAQ_DataTask->SetEndTime (RunTime);
    DAQ_DataTask->SetSamplingTime (SamplingTime);
    DAQ_DataTask->SetStepVoltage (StepSize);
    #ifdef SIMULATION
        SimulationTask->SetSimulationParameters (InputGain, SpringConst, Friction,
                                                 Mass);
        SimulationTask->SetInitialConditions (0.0, 0.0);
    #endif

    //  Clear the screen so we can go on to the next bit 
    clrscr ();
    }


//-------------------------------------------------------------------------------------
//  Constructor:  C_DAQ_InterfaceTask
//      This constructor creates a task object for this class.

C_DAQ_InterfaceTask::C_DAQ_InterfaceTask (char *aName) : BaseTask (aName)
    {
    DeltaTaskTime = 0.05;       //  This is the task's "sample time", the period of
                                //  time between successive runs of the Run function
    NextTaskTime = 0.0;         //  The first time this task will be ready to run
    State = 0;                  //  This is the initial state for the task
    NextState = 0;              //  Use this elsewhere to cause state transitions
    TestHasEnded = false;		//  Make sure test isn't over prematurely
    }


//-------------------------------------------------------------------------------------
//  Destructor:  ~C_DAQ_InterfaceTask
//      This destructor deletes dynamically allocated memory items.

C_DAQ_InterfaceTask::~C_DAQ_InterfaceTask (void)
    {
    //  But there are no dynamically allocated items
    }


//-------------------------------------------------------------------------------------
//  Function:  Run
//      This run function writes information to the screen to let the user know what
//      is happening as the data acquisition program runs.

int C_DAQ_InterfaceTask::Run (void)
    {
    double Time;                //  Local variable used for temporary storage of time
    int done = 1;               //  Default return value indicating done for this scan
    NumScans++;                 //  Increment scan count

    //  Task timing code decides if the task should be run at a given time
    Time = GetTimeNow();

    //  Calculate the next run time for the task
    NextTaskTime += DeltaTaskTime;

    //  This section checks if there has been a state transition.  When the task is
    //  going to transition between states, you set NextState to the new state; when
    //  there is not going to be a transition just leave NextState as -1.
    if (NextState != -1)
        {
        AuditTrail (this, State, NextState);    //  Record a state transition here
        State = NextState;
        NextState = -1;
        RunEntry = 1;
        }
    else
        RunEntry = 0;

    //  Depending on which state we're in, run the correct state code
    switch (State)
        {
        //  State 0:  Running a test.  The test begins as soon as the scheduler is
        //            started up and ends when the time reaches 'EndTime'
        case 0:
            if (RunEntry)                       //  This entry code runs just once
                {
                StartTime = GetTimeNow ();      //  Save time when task starts up

                printf ("Beginning data acquisition\n");
                printf ("Step voltage: %6.2lf volts\n",
                        DAQ_DataTask->GetStepVoltage ());
                printf ("Time:   Position:  Velocity:\n");
                }

            //  Code in the Action Section runs repeatedly until state is exited
            printf ("%7.4lf  %5.3lf     %5.3lf\r", Time,
                    DAQ_DataTask->GetCurrentOutput (0),
                    DAQ_DataTask->GetCurrentOutput (1));

            //  The Test/Exit section runs repeatedly until exit also
            if (TestHasEnded == true)
                {
                NextState = 1;              //  Go into the next state
                done = 1;                   //  Run next state on next scan
                }
            break;

        //  State 1:  The test has finished; print a message to indicate this and
        //            then just remain in this state until the program ends
        case 1:
            //  Entry Section: Upon entering this state, cause the test to end
            if (RunEntry)                   
                {
                printf ("\nTest finished.\n");
                StopControl ("");
                }
            //  Action Section: Empty because this state just sits idly
            //  Test/Exit Section: Empty also; this state is never exited
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

