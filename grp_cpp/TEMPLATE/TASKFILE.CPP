//*************************************************************************************
//  TaskFile.cpp
//      This file is a template for each task file of a group priority application.
//      This file contains skeletons of the component parts of a task file, but it
//      cannot be compiled because the actual code is missing. It can be used to build
//      a program by filling in the missing pieces and changing names away from the
//      generic names such as "task1".
//
//  Revisions:
//      12-19-95  DMA/JCJ  Template created based on existing examples
//       8-19-97  JRR      Cleaned up, comments modified
//*************************************************************************************

#include "tasks.hpp"


//-------------------------------------------------------------------------------------
//  Constructor:  CTask1
//      This is a template for the class CTask1 constructor.  The user will have
//      derived this task class from the BaseTask class.  Note that this constructor
//      calls the BaseTask constructor as it begins.

CTask1::CTask1 (char *aName) : BaseTask (aName)
    {
    //  The following variables belong to class BaseTask.  We set their values here
    //  in order to control the timing parameters and transition-logic state of our
    //  task.  
    DeltaTaskTime = 0.1;        //  This is the task's "sample time", the period of
                                //  time between successive runs of the Run function
    NextTaskTime = 0.0;         //  The next time this task will be ready to run
    State = 0;                  //  This is the initial state for the task
    NextState = 0;              //  Use this elsewhere to cause state transitions

    //  Initialize your task-specific data here.  If more than one instantiation
    //  (i.e., more than one task object) will be created from this class, data that
    //  is different between instantiations can be given as arguments to the con-
    //  structor - for example "CTask2::CTask2 (char* aName, int TaskNum, etc.)"
    Example = 0.0;
    pMyNewObject = new C_MyObject (42);     //  Create some sort of object here
    pMyName = new char[128];                //  Create a string buffer for big names
    ...etc...
    }

    
//-------------------------------------------------------------------------------------
//  Destructor:  ~CTask1
//      The destructor for the task deletes any dynamically allocated memory items.
//      If you created any arrays or objects with the 'new' operator in the construc-
//      tor, delete them in the destructor to prevent 'resource leak' errors.  

CTask1::~CTask1 (void)
    {
    delete pMyNewObject;                    //  Delete objects which were created
    delete [] pMyName;                      //  with the 'new' operator
    }

    
//-------------------------------------------------------------------------------------
//  Function:  Run
//      This is the function which the task dispatcher runs once every sample time (or
//      for low priority tasks, whenever there's some free time).  Most of the user's
//      task code will go in here.  You must always supply a Run() function for every
//      one of your tasks.

int CTask1::Run (void)
    {
    double Time;                //  Local variable used for temporary storage of time
    int done = 1;               //  Default return value indicating done for this scan
    NumScans++;                 //  Increment scan count

    //  Initialization code:  If there is anything which the task needs to do just
    //  once when it first runs, put it below.  (Anything which should be done before
    //  the scheduler even starts can be put into the constructor.)
    if (State == 0)
        {
        //  Your startup code goes here
        State = 1;
        NextState = 1;
        return 0;
        }

    //  Task timing code:  This is the code which decides if the task should be run
    //  at the given time.  Typical code (shown below) will compare the current
    //  measured or simulated time to the time the task should next run, which is the
    //  previous time the task was supposed to run plus the delta time.  
    //  Note:  It is not required that there be some time based criterion for when
    //  the task runs.  This section could be taken out, resulting in a task that
    //  runs every scan.  That is a design decision based on the nature of the task.
    if ((Time = GetTimeNow()) < NextTaskTime)
        return (done);
    NextTaskTime += DeltaTaskTime;

    //  This section checks if there has been a state transition.  If you want to
    //  cause a transition between states, you set NextState to the new state; when
    //  there is not going to be a transition just leave NextState as -1.
    if (NextState != -1)
        {
        AuditTrail (this, State, NextState);    //  Record transition for audit trail
        State = NextState;
        NextState = -1;
        RunEntry = 1;
        }
    else
        RunEntry = 0;

    //  This is the state transition section of the code, containing algorithms
    //  which are specified to run in the entry, action, and test sections for each
    //  state in the state transition diagram.  Note:  The variable 'State' is
    //  defined in class BaseTask, so it is inherited by your task class
    switch (State)
        {
        //  State 1 -- It's good to put the state name and description here 
        case 1:
            if (RunEntry)
                {
                //  Entry code goes here.  Entry code only runs when entering a state
                ...
                //  Often you need to copy information from exchange variables to
                //  local ones during the entry into a state.  The code looks like:
                CriticalSectionBegin ();
                Something = xc_Something;
                CriticalSectionEnd ();
                ...
                }
            //  Action Section:  This part is run on every scan.  It can be empty if
            //  your state doesn't do anything but wait to transition to next state
            ...

            //  Sometimes you need to copy data out of a task so that it can be used
            //  by other tasks.  The code looks like this:
            CriticalSectionBegin ();
            xc_SomethingElse = SomethingElse;
            CriticalSectionEnd ();


            //  Test/Exit section:  This part is also run on every scan.  Here you
            //  place code which checks for the condition(s) in which a transition
            //  to another task will occur
            if (/* test something */)
                {
                NextState = 2;              //  Go into another state
                done = 0;                   //  Run next state immediately
                }

            break;

        //  State2 -- This state does <whatever>
        case 2:
            //  Entry Section
            if (RunEntry)
                {
                ...
                }
            //  Action Section
            ...

            //  Test/Exit Section
            if (/* test something */)
                {
                NextState = 3;
                done = 0;    
                }

            //  This exit section sets done = 1, which causes the scheduler to go to
            //  the next state's entry code only after other tasks have been serviced
            else if (-- test for something else --)
                {
                NextState = 6;              //  Another transition
                done = 1;
                }

            break;

        ...     //  Add more additional states here...

        //  The default case will only be entered if there has been a transition to a
        //  nonexistent state.  This is an error condition, so complain about it
        default:
            StopControl ("ERROR: Transition to illegal state %d in task \"%s\",
                         State, Name);   
            return (-1);
        }
    return (done);
    }

