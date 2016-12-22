//*************************************************************************************
//  MainFile.cpp
//      This file is a template for the "main" file of a group priority application.
//      This file shows skeletons of all of the component parts of an application. It
//      cannot be compiled because the actual code is missing. It can be used to build
//      a program by filling in the missing pieces and changing names (away from the
//      generic names such as "task1").  There are also template files for a task
//      file (TaskFile.cpp) and for the projecct header file (Tasks.hpp).
//
//  Revisions:
//      12-19-95  DMA/JCJ  Created based on existing examples
//*************************************************************************************

#define MAIN_CPP
#include "tasks.hpp"


//-------------------------------------------------------------------------------------
//  File-scope Global Data
//      Put data items here so they'll be accessible to other functions in this file.

static double EndTime = 1.0e10;             //  Use a big number to run nearly forever
static bool StopControlNow = false;         //  Becomes true when it's time to quit

//  Allocate memory for message to print when exiting program; put in default message
static char StopMessage[256] = "\n\n***Normal Program Termination***\n\n";


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

    // nTask lists are instantiated here; TList pointers are declared in Tasks.hpp
	LowPriority = new TList ("Low Priority");
    Intermittent = new TList ("Intermittent");

    //  These lists are only instantiated if interrupt scheduling is used
    #ifdef ISR_SCHEDULING
        Preemptable = new TList ("Preemptable");
        Interrupt = new TList ("Interrupt");
    #endif  //  ISR_SCHEDULING

    //  Create all of the task objects used in the project
    Task1 = new CTask1 ("Task1");
    Task2 = new CTask2 ("Task2");
    ...    //  Other tasks
    
    //  Define another task object for running the simulation.  This task is scheduled
    //  just as any other task but is only added to the task list if a simulation is
    //  desired.  The actual simulation code in contained in the Run() member function
    //  of SimulationTask.  Note that this use of the #define SIMULATION is optional.
    #ifdef SIMULATION
        SimTask = new CSimulationTask ("Simulation");
    #endif  //  SIMULATION

    //  All tasks must be added to the task lists here.  Some tasks will be placed in
    //  an interrupt task list only if interrupt scheduling is used; otherwise they
    //  go into the appropriate non-interrupt task list.
    #ifdef ISR_SCHEDULING
        Preemptable->Append (Task1);
        Interrupt->Append (Task2);
    #else   //  ISR_SCHEDULING
        Intermittent->Append (Task1);
        LowPriority->Append (Task2);
    #endif  //  ISR_SCHEDULING

    //  This low-priority task doesn't ever need to be scheduled by interrupts
    LowPriority->Append (Task3);

    //  The simulation task only exists when compiling in simulation mode 
    #ifdef SIMULATION
        LowPriority->Append (SimTask);
    #endif  //  SIMULATION

    //  Set up a data output object.  The arguments for the DataOutput constructor are
    //      int nvars, double STime, double LTime, double DTime
    DataOutput *DataOut1 = new DataOutput (3, 0.0, 20.0, 0.1);

    //  You must call this function to initialize the Audit trail array before you
    //  write to it.
    InitAuditTrail ();

    //  Perform any more setup code which your project needs here
    ProjectSetup ();

    //  If using interrupt scheduling in DOS mode, set up the interrupt hardware.
    //  If not, the clock must be set up in whatever mode you're using
    #ifdef ISR_SCHEDULING
        SetupTimerInterrupt ();
    #else
        SetupClock (-1.0);
    #endif  //  ISR_SCHEDULING

    //  This is the scheduler loop.  Inside this loop, you MUST NOT call any console
    //  I/O functions such as printf, cout, getchar, scanf, etc.  They are either too
    //  slow or are blocking.  This includes inside the run functions of the tasks.
    //  These things are OK in SIMULATION ONLY.  Use #ifdef SIMULATION (or something
    //  similar) if you have sections of code in which you want to print to the screen
    //  for debugging purposes.
    while (((TheTime = GetTimeNow()) <= EndTime) && (StopControlNow == false))
        {
        //  This section is used for general output. The DataOutput object stores data
        //  in arrays until requested to write the data to the disk. This could be
        //  done in a separate task, if desired
        if (DataOut1->IsTimeForOutput ())
            {
            double val, mc, set;                //  Get data for output
            Task1->GetData (&val, &mc, &set);   //  Data from the controller
            DataOut1->AddData (Task2->GetSetpoint (), val, mc, END_OF_ARGS);
            IncrementTime ();                   //  Saving the data takes some time
            }

        //  Run low priority and intermittent task lists here in the background.
        //  The TaskDispatcher() function returns a nonzero value in case of errors
        if (TaskDispatcher (LowPriority, Intermittent))
            StopControl ("Problem with background tasks");
        }

    //  The scheduling loop has finished, so turn off interrupt scheduling as well
    #ifdef ISR_SCHEDULING
        RestoreTimerInterrupt ();
    #endif  //  ISR_SCHEDULING

    //  Clear the screen if we're not running in Windows GUI mode
    #if (!defined (__WIN32__) || defined (__CONSOLE__))
        clrscr ();
    #endif  //  GUI mode

    //  Print the exit message so that the user can see it
    cout << StopMessage;

    //  There are two versions of the audit trail, one with task names and one with
    //  task numbers instead (for Matlab).  We print both versions here
    WriteAuditTrail ("AudTrail.txt");
    WriteAuditTrailNum ("TrailNum.txt");

    cout << "\nThe time at termination is " << TheTime << "\n";

    // Print the scan statistics to the screen
    WriteScanStat("scans.txt", TheTime);

    // Save the output data to a file
    DataOut1->WriteOutputFile ("Data_Out.txt");

    delete DataOut1;                        //  Done with this object

    //  If any tasks have created their own output records, the files are written here
    Task2->WriteEventRecord ("Evnt_Rec.txt");

    //  Delete the task lists.  The task lists' destructors automatically delete the
    //  tasks in the task lists for you
    delete (LowPriority);
    delete (Intermittent);
    #ifdef ISR_SCHEDULING
        delete (Preemptable);
        delete (Interrupt);
    #endif  //  ISR_SCHEDULING

    cout << "Hit any key to exit.\n";       //  Let user view screen before it
    while (!kbhit ());                      //  disappears
    getch ();                               //  Empty out the keyboard buffer
    }


