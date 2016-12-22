//*************************************************************************************
//  Tasks.hpp
//      This is the main header file for the ME230 Lab 1 Data Acquisition program.
//
//  Revisions:
//       8/02/95 - Created by Jason Jones
//       7/07/97 - Modified by JR #7 Ridgely, cleaned up
//*************************************************************************************

#ifndef TASKS_HPP                   //  This macro prevents the contents of this file
    #define TASKS_HPP               //  from being included twice in source files

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>                   //  Include a bunch of standard C++ headers;
#include <dos.h>                    //  people usually wind up needing them
#include <iostream.h>
#include <fstream.h>
#include <string.h>

#include "Time_Mod.hpp"             //  Set the time keeping mode in this file
#include <CompConv.hpp>             //  For compatibility between compilers
#include <Base_Obj.hpp>
#include <BaseTask.hpp>             //  Header for base task classes
#include <Audit.hpp>
#include <Oper_Int.hpp>             //  Include only when operator interface is used

//#define  REAL_IO                    //  Define this if using real I/O, not simulation

#if defined (__WIN32__) && defined (REAL_IO)    //  Header file for I/O card: If we're
    #include <NT-DTEZ.hpp>                      //  compiling for NT, use the NT
#endif                                          //  compatible driver header; if we
#if !defined (__WIN32__) && defined (REAL_IO)   //  are compiling for DOS, use DOS one
    #include <DTEZ.hpp>                         //  but if we aren't using real I/O,
#endif                                          //  don't use any I/O card header file

#define  OUTPUT_CHANNEL  0          //  Which I/O card channel is used for output
#define  NUM_CHANNELS    3          //  How many input channels are being used
#define  DEF_TICK_RATE   0.001      //  The default tick rate for the scheduler


//-------------------------------------------------------------------------------------
//  Class:  C_DAQ_InterfaceTask
//      This class handles the operator interface for the Data Acquisition program.

class C_DAQ_InterfaceTask : public BaseTask
    {
    private:
        double StartTime;                   //  Real-time at which program started
        bool TestHasEnded;                  //  Becomes true when the test is over

    public:
        C_DAQ_InterfaceTask (char *aName);  //  Constuctor creates new task object
        ~C_DAQ_InterfaceTask (void);        //  Destructor frees up memory it used

        //  Run() is the function containing the task code that the scheduler loop
        //  calls when the task is scanned.  Return value can be used as desired.
        int Run (void);

        void EndTheTest (void)              //  Function to set a flag which causes
            { TestHasEnded = true; }        //  the test mode window to be exited
    };


//-------------------------------------------------------------------------------------
//  Class:  C_DAQ_DataTask
//      This class controls the data for the data acquisition program's step response
//      test.

class C_DAQ_DataTask : public BaseTask
    {
    private:
        double StartTime;                   //  Real-time at which program started
        double EndingTime;                  //  Time at which the test will end
        double StepHeight;                  //  Height of step response input
        double xc_StepHeight;               //  Exchange copy of the step height
        DataOutput* TheLogger;              //  Points to a data logger object
        bool TimeToEndTest;                 //  Becomes true when test is to be ended
        double Output[NUM_CHANNELS];        //  Output from the experimental system
        double xc_Output[NUM_CHANNELS];     //  and an exchange copy of that too'
        char FileName[32];                  //  Name of file in which data is saved

    public:
        C_DAQ_DataTask (char *aName);       //  Constructor creates new task object
        ~C_DAQ_DataTask (void);             //  Destructor frees up memory it used

        //  Run() is the function containing the task code that the scheduler loop
        //  calls when the task is scanned.  Return value can be used as desired.
        int Run (void);

        double GetStepVoltage (void)        //  Function returns step input height
            { return StepHeight; }          //  which the user gave to this object
        void SetStepVoltage (double aVolt)  //  Set step height used by this task
            { xc_StepHeight = aVolt; }      //  to control the response test
        double GetCurrentOutput (int Chan)  //  Returns current measured output
            { return xc_Output[Chan]; }     //  of the real or simulated plant
        void SetEndTime (double aTime)      //  Function which sets the time when
            { EndingTime = aTime; }         //  the test is to be ended
        void SetSamplingTime (double aTm)   //  Function to set sampling time of task
            { DeltaTaskTime = aTm; }        //  is called only before scheduler runs

        //  These functions interface with a real or simulated I/O card and plant
        void SetActuationVoltage (double aVolts);
        double MeasureVoltage (int);
    };


//  Simulation switch:  Leave the following line in to add a simulation task to be
//  scheduled with the rest of the tasks in the background list.  This define is
//  optional; it is not used by any of the program code not written by the user.
#define SIMULATION

//-------------------------------------------------------------------------------------
//  Class:  CSimulationTask
//      This class has been set up to simulate a simple second-order dynamic system.
//      An example would be a mass-spring-damper system.  It performs a simple (crude)
//      Euler integration at each time step to determine the state variables' values
//      for the next time step.  The state variables are called Position and Velocity,
//      but the mathematics would work for some other systems (thermal, electronic,
//      and so on).

#ifdef SIMULATION
    class CSimulationTask : public BaseTask
        {
        private:
            double Position;                //  System state variable
            double xc_Position;             //  Exchange copy of first state variable
            double Velocity;                //  Another bleeping state variable
            double xc_Velocity;             //  Exchange copy of this one
            double Input;                   //  Here's the input to the system
            double xc_Input;                //  and an exchange copy of it too
            double Output[NUM_CHANNELS];    //  These variables store the system's
            double xc_Output[NUM_CHANNELS]; //  outputs and an exchange copy thereof

            double SpringConstant;          //  These are the parameters for the
            double FrictionRate;            //  formula dv/dt = (1/m)(F - bv - kx)
            double Mass;                    //  where m = mass, b = FrictionRate,
            double InputGain;               //  k = SpringConstant, F = u * InputGain

            int SetUpCorrectly;             //  Used to ensure setup functions called 

        public:
            CSimulationTask (char *aName);  //  Constructor for the class
            ~CSimulationTask (void);        //  Destructor for the class

            int Run (void);                 //  You must supply a Run() method
            double GetOutput (int Ch);      //  Function to get the plant outputs
            void SetInput (double aIn)      //  Actuation function sets exchange
                { xc_Input = aIn; }         //  copy of the plant input variable

            //  These next functions set initial conditions and system parameters
            void SetInitialConditions (double, double);
            void SetSimulationParameters (double, double, double, double);
        };

#endif  //  SIMULATION


//  Function prototypes which are used only for timer interrupt scheduling
#ifdef ISR_SCHEDULING
    void SetupTimerInterrupt (void);
    void RestoreTimerInterrupt (void);
    void interrupt TimerISR (...);
#endif

//  Function prototypes for globally accessible functions in this project
void SetTickRate (double);
double GetTickRate (void);
void StopControl (const char*, ...);
void StopControl (char*);

//  This function allows an operator interface to be used to set up the experiment
void GetSetupParameters (double&);


//-------------------------------------------------------------------------------------
//  Task lists, task pointers, and global data
//      Pointers to all task objects defined in the project must be listed here.  The
//      extern definitions are used so that tasks can refer to each other.  Note that
//      no space is allocated for the task objects themselves here -- only pointers to
//      the objects.  Space is allocated with the 'new' operator in the main function.
//
//      You must enter each task pointer twice, once without the keyword 'extern' and
//      once with 'extern' (see the CTask1 example below).  Thus the pointer will be
//      declared in your main file, and each other file in your project will be able
//      to use that pointer to access the task to which it points.

#ifdef MAIN_CPP             //  The following is only compiled into the main .CPP file
    C_DAQ_InterfaceTask* DAQ_InterfaceTask;
    C_DAQ_DataTask* DAQ_DataTask;

    #ifdef SIMULATION                       //  If using simulation mode, declare a
        CSimulationTask* SimulationTask;    //  pointer to the simulation task here
    #endif

    //  Define the task list pointers here; first come the background task lists
	TList* LowPriority;
    TList* Intermittent;

    //  These lists are only instantiated if interrupt scheduling is used
    #ifdef ISR_SCHEDULING
        TList* Preemptable;
        TList* Interrupt;
    #endif  // ISR_SCHEDULING

#else  //  MAIN_CPP
    //  These pointers are only used if interrupt scheduling is used
    #ifdef ISR_SCHEDULING
        extern TList* Preemptable;
        extern TList* Interrupt;
    #endif  // ISR_SCHEDULING
#endif  //  MAIN_CPP

//  These extern declarations of the pointers to task objects allow each task's task
//  object to find each other task's task object
extern C_DAQ_InterfaceTask* DAQ_InterfaceTask;
extern C_DAQ_DataTask* DAQ_DataTask;
#ifdef SIMULATION
    extern CSimulationTask* SimulationTask;
#endif

#endif  //  TASKS_HPP

