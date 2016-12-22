//*************************************************************************************
//  DAQ_main.cpp
//      This is the main file for the ME230 Lab 1 data acquisition program.
//
//  Revisions:
//      12-19-95  DMA/JCJ  Created based on existing examples
//*************************************************************************************

#include <stdarg.h>                         //  Header for variable-argument lists
#define MAIN_CPP
#include "tasks.hpp"


//-------------------------------------------------------------------------------------
//  File-scope Global Data
//      Put data items here so they'll be accessible to other functions in this file.

static double EndTime = 1.0e10;             //  Use a big number to run nearly forever
static bool StopControlNow = false;         //  Becomes true when it's time to quit
static char* StopMessage = NULL;            //  Message to print when exiting program


//-------------------------------------------------------------------------------------
//  Functions:  StopControl
//      This function stops the scheduler cleanly.  One should not stop the scheduler
//      by pressing Control-C or calling a function such as _exit(), because exiting
//      the program abruptly like that will fail to save data (bummer) or in interrupt
//      mode, crash the computer (total bummer).  Call this function instead.  There
//      are two versions of this function provided; this is a patch to provide compat-
//      ibility with an earlier version of the scheduler.

void StopControl (const char* aFormat, ...) //  This version takes variable arguments
    {
    char TBuf[256];                         //  Buffer holds text string temporarily
    va_list Args;                           //  Variable argument list

    va_start (Args, aFormat);               //  Set up variable arguments
    vsprintf (TBuf, aFormat, Args);         //  Build up the string to be displayed
    StopMessage = new char[strlen(TBuf)];   //  Allocate memory for output string
    strcpy (StopMessage, TBuf);             //  and copy the output message into it
    va_end (Args);                          //  Clean up variable argument processing
	StopControlNow = true;                  //  Set flag to exit scheduler loop
    }

void StopControl (char* aStr)               //  This version takes one argument only
    {
    StopMessage = new char[strlen(aStr)];   //  Allocate memory for output string
    strcpy (StopMessage, aStr);             //  and copy the output message into it
	StopControlNow = true;                  //  Set flag to exit scheduler loop
    }


//-------------------------------------------------------------------------------------
//  Function:  main
//      This is the entry point for the program.  It first sets up all the task lists
//      and other items used by your program.  Then it runs the scheduler loop; the
//      program remains in that loop for as long as your tasks are being executed.
//      After exiting the scheduler loop the program writes data and diagnostic
//      information to files.

int main ()
    {
    double TheTime;                         //  Current measured or simulated time
    double TickRate = DEF_TICK_RATE;        //  Tick rate at which scheduler runsf

    // nTask lists are instantiated here; TList pointers are declared in Tasks.hpp
	LowPriority = new TList ("Low Priority");
    Intermittent = new TList ("Intermittent");

    //  These lists are only instantiated if interrupt scheduling is used
    #ifdef ISR_SCHEDULING
        Preemptable = new TList ("Preemptable");
        Interrupt = new TList ("Interrupt");
    #endif  // ISR_SCHEDULING

    //  Create all of the task objects used in the project
    DAQ_InterfaceTask = new C_DAQ_InterfaceTask ("Interface");
    DAQ_DataTask = new C_DAQ_DataTask ("Data Control");
    #ifdef SIMULATION
        SimulationTask = new CSimulationTask ("Simulator");
    #endif

    //  All tasks are added to the task lists here
    #ifdef ISR_SCHEDULING
        #ifdef SIMULATION
            Preemptable->Append (SimulationTask);
        #endif
        Interrupt->Append (DAQ_DataTask);
    #else   //  ISR_SCHEDULING
        #ifdef SIMULATION
            Intermittent->Append (SimulationTask);
        #endif
        Intermittent->Append (DAQ_DataTask);
    #endif  //  ISR_SCHEDULING

    //  This low-priority task doesn't ever need to be scheduled by interrupts
    LowPriority->Append (DAQ_InterfaceTask);

    //  Call this function to initialize the Audit trail array before you write to it
    InitAuditTrail ();

    //  Pop up an operator window in which we get the parameters from the user
    printf ("\nAbout to run setup window; press RETURN to continue.");
    getchar ();
    GetSetupParameters (TickRate);
    SetTickRate (TickRate);

    //  If using interrupt scheduling in DOS mode, set up the interrupt hardware.
    //  If not, the clock must be set up in whatever mode you're using
    #ifdef ISR_SCHEDULING
        SetupTimerInterrupt ();
    #else
        SetupClock (-1.0);
    #endif  //  ISR_SCHEDULING

    //  This is the scheduler loop in which all the interesting stuff happens
    while (((TheTime = GetTimeNow ()) <= EndTime) && (StopControlNow == false))
        {
        //  Run low priority and intermittent task lists here in the background.
        //  The TaskDispatcher() function returns a nonzero value in case of errors
        if (TaskDispatcher (LowPriority, Intermittent))
            StopControl ("Problem with background tasks");
        }

    //  The scheduling loop has finished, so turn off interrupt scheduling as well
    #ifdef ISR_SCHEDULING
        RestoreTimerInterrupt ();
    #endif  //  ISR_SCHEDULING

    //  Clear the screen
	getchar ();  //////////////////////
    clrscr ();

    //  Print the exit message so that the user can see it
    if (StopMessage == NULL)
        cout << "\n\n***Normal Program Termination***\n\n";
    else
        cout << StopMessage;

    // There are two versions of the audit trail, one with task
    //  names and one with task numbers instead (for matlab)
    WriteAuditTrail ("trail.txt");
    WriteAuditTrailNum ("trl_num.txt");

    cout << "\nThe time at termination is " << TheTime << "\n";

    // Print the scan statistics to the screen
    WriteScanStat ("scans.txt", TheTime);

    // De-allocate space given to tasks
    delete (LowPriority);
    delete (Intermittent);
    #ifdef ISR_SCHEDULING
        delete (Preemptable);
        delete (Interrupt);
    #endif // ISR_SCHEDULING

    if (StopMessage != NULL)                //  If a scheduler-stopping message
        delete [] StopMessage;              //  has been created, delete it now

    cout << "Hit any key to exit.\n";       //  Print the press-a-key message and
    while (!kbhit ());                      //  wait until the user does so, then
        getch ();                           //  remove keystroke from the buffer
    }


