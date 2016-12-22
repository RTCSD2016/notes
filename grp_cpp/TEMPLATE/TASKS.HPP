//*************************************************************************************
//  Tasks.hpp
//      This is the main header file for a Group Priority Scheduler project.  You
//      should place your class definitions and global variable declarations in this
//      file.  Then #include "Tasks.hpp" in each of your task source files (*.cpp)
//      as well as your main source file.
//
//      NOTE:  It is recommended that this file be named Tasks.hpp for all projects.
//
//  Revisions:
//       8/02/95 - Created by Jason Jones
//       7/07/97 - Modified by JR #7 Ridgely, cleaned up
//*************************************************************************************

#ifndef TASKS_HPP                   //  This macro prevents the contents of this file
    #define TASKS_HPP               //  from being included twice in source files

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <conio.h>
#include <math.h>                   //  Include a bunch of standard C++ headers;
#include <dos.h>                    //  people usually wind up needing them
#include <iostream.h>
#include <fstream.h>
#include <string.h>

#include "Time_Mod.hpp"             //  Set the time keeping mode in this file
#include <BaseTask.hpp>             //  Header for base task classes
#include <Oper_Int.hpp>             //  Include only when operator interface is used


//=====================================================================================
//  Class Definitions
//      Put all task class definitions here.  Each class MUST be inherited from class
//      BaseTask and include a Run() function.
//=====================================================================================

//-------------------------------------------------------------------------------------
//  Sample class:  CTask1
//      This class definition is to be used as a template.  Edit it to add all the
//      functionality you need in each of your task classes.  

class CTask1 : public BaseTask
    {
    private:
        //  This section is for variables that are private to this class. The local
        //  variables and the "exchange" variables are placed here (the convention is
        //  to use "xc_" as the first letters of the exchange variable names).
        double A_variable, xc_A_variable;
        double Another_variable, xc_Another_variable;
        ...

        // Functions that are private to this class can't be called by other classes
        void DoSomethingPrivate (double);
        ...

        // This object is used to keep track of data internal to this class
        DataOutput* EventRecord;

    public:
        Task1 (char* aName);                //  Constuctor for the class.
        ~Task1 (void);                      //  Destructor for the class.

        //  Run() is the function containing the task code that the scheduler loop
        //  calls when the task is scanned.  
        int Run (void);

        //  These are the functions that allow other tasks to send data to or receive
        //  data from this task.  If the functions only copy exchange variables, they
        //  are often defined inline, as GetSomething() is below
        void SetSomething (double);
        ...
        double GetSomething (void) { return (xc_Something); }
        ...

        //  You may add other functions which enable your task object to do whatever
        //  it needs to do.  This one for example is a "cleanup" function that is used
        //  to write a data file after the real time operation is done.
        void WriteDataRecord (char* filename);
    };

//  Additional class definitions follow the same form as that for Task1 above.
//-------------------------------------------------------------------------------------


//  Simulation switch:  Leave the following line in to add a simulation task to be
//  scheduled with the rest of the tasks in the background list.  This define is
//  optional; it is not used by any of the program code not written by the user.
#define SIMULATION

//-------------------------------------------------------------------------------------
//  Class:  SimulationTask
//      If the SIMULATION define method is being used, edit the class for the simula-
//      tion task below.

#ifdef SIMULATION
    class SimulationTask : public BaseTask
        {
        public:
            SimulationTask (char *aName);   //  Constuctor for the class
            ~SimulationTask (void);         //  Destructor for the class

            int Run (void);                 //  You must supply a Run() method
            
            double GetSomething (void);     //  Add your own data transfer functions

        private:
            // Private variables for the simulation
            double parameter1, parameter2;  //  System parameters
            ...
            };
#endif  //  SIMULATION


//-------------------------------------------------------------------------------------

//  Function prototypes which are used only for timer interrupt scheduling
#ifdef ISR_SCHEDULING
    void SetupTimerInterrupt (void);
    void RestoreTimerInterrupt (void);
    void interrupt TimerISR (...);
#endif  //  ISR_SCHEDULING

//  Prototypes for other functions.  When you write functions inside one file in the
//  project, add your prototype here if other files need to use that function
void InitRecPWM (double period);
void RecPWM (int val);
void WritePWMRec (char *fname);
void SetTickRate (double tr);
double GetTickRate (void);
void StopControl (const char *, ...);
void StopControl (char*);
...


//-------------------------------------------------------------------------------------
//  Task lists, task pointers, and global data
//      Pointers to all task objects defined in the project must be listed here.  The
//      extern definitions are used so that tasks can refer to each other.  Note that
//      no space is allocated for the task objects themselves here -- only pointers to
//      the objects.  Space is allocated with the 'new' operator in the main function.
//
//      You must enter each task pointer twice, once without the keyword 'extern' and
//      once with 'extern' (see the CTask1 example below).  Thus the pointer will be
//      created in your main file, and each other file in your project will be able
//      to use that pointer as an 'extern' to access the task to which it points.

#ifdef MAIN_CPP             //  The following is only compiled into the main .CPP file

    #ifdef SIMULATION                       //  If using simulation mode, declare a
        SimulationTask *SimTask;            //  pointer to the simulation task here
    #endif                                  //  If not, just comment it out

    CTask1 *Task1;                          //  Replace this with pointers to your
    ...                                     //  task objects

#else
    //  External declarations of task pointers go here; they are compiled into every
    //  file in your project
    #ifdef SIMULATION
        extern SimulationTask *SimTask;
    #endif

    extern CTask1 *Task1;                   //  Replace this with extern declarations
    ...                                     //  of all your task object pointers

#endif  //  MAIN_CPP

#endif  //  TASKS_HPP

