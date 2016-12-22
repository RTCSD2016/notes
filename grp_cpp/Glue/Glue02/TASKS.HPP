// Main header file for the glue/manufacturing example
// File: tasks.hpp
// Originally created: May 17, 1997 by D.M. Auslander
// Various versions of this file will be used to build a complete
// control system in steps. Comments will be additive to describe
// which level the file refers to.

// All class definitions for tasks are in this file as are external
// variable definitions, etc.

// 0. Simulation of the two belt-driven axes only
// 1. Add simulation of the oven temperature (based on level 0)
// 2. Add PID control for the belts and heaters

#ifndef TASKS_HPP
#define TASKS_HPP

#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include "basetask.hpp"
#include "audit.hpp" // Transition audit file functions
// #include "oper_int.hpp"  // Only include this if an operator
#include "time_mod.hpp"  // Set the time keeping mode

#ifdef ISR_SCHEDULING
void interrupt TimerISR(...);
#endif

// Define rudimentary process information here so that conversion
//  to multi-processor implementation will be easier

#define PROCESS_INFO // So that basetask.cpp does not put in defaults

#ifdef PROCESS_INFO  // Bring in process information; otherwise allow defaults
    const int NumberOfProcesses = 1;  // Number of independent processes
    // Process map:
    #define PROCESS_A 0  // Define as 0 for single process system
    static int ThisProcess = 0; //  This value must move to another include
            // file if a multi-process implementation is used
#endif

// Put all task class definitions here.  Each class MUST
// be inherited from BaseTask and include a Run() function


// Data logging task

class CDataLogger : public BaseTask
   {
   public:
      CDataLogger(char *name,int ProcNo);  // Constructor
      ~CDataLogger(void); // Destructor
      int Run(void);
      void WriteFile(void);

   private:
      DataOutput *DataOutBelt,*DataOutOven;
   };

// Controller classes - these are all inherited from the base
// PID controller
#include "pid_gen1.hpp"

// Leave this line in to add a simulation task(s) to be scheduled
// with the rest of the tasks in the background list.  This
// define is optional;  it is not used by any of the program
// code not written by the user.

// In this program several tasks will be used -- one for each of the major
// system components. This allows the use of inheritance to efficiently
// model the duplicate components on each leg of the system

#define SIMULATION

#ifdef SIMULATION

// This is the generic class for the belt-driven axis that carries
// the part to be assembled. There are two of these in the system.
// The initial version uses only internal data for actuation (values
// set in the constructor). It uses global data arrays to make its
// internal variables visible to other tasks (data logging, in this
// case).
// Tasks BeltAxis1 and BeltAxis2 will be instantiated directly from
// this class

class CBeltAxis : public BaseTask
    {
    public:
        CBeltAxis(char *aName,double JJ,double Ktt,int ProcNo);
                // Constuctor for the class. Put in parameter
                        // values here so the various instances can be
                        // distinguished
        ~CBeltAxis();      // Destructor for the class.
        int Run(void);
    private:
        double x,v;  // Position and velocity. Position is 0 when the clamp
                // is at the part pick-up point.  m, m/s
        double cur;  // Current to the motor, amps
        double J;  // System inertia, as viewed from the motor, N m s^2
        double Kt; // Conversion from motor current to torque, N m /amp
        double previous_time;  // Used to compute the time increment for
                // the Euler integration
        };
// Define global variable indexes
#define BELTAXIS_x 0
#define BELTAXIS_v 1

// Define Message Boxes
#define BELTAXISMSG_Cur 0

class CThOven : public BaseTask
    {
    public:
        CThOven(char *aName,double aK,double aThAmb,
                double aR,double aHCap,int ProcNo);
        ~CThOven();
        int Run(void);
    private:
        double Th;  // Temperature (C)
        double ThAmb;  // Ambient temperature (C)
        double K;  // Heat transfer coefficient
        double Cur;  // Electrical current to heater (amp)
        double R;  // Electrical resistance of the heater
        double HCap;  // Heat capacity of oven
        double previous_time;  // Use to compute time increment
                // for the Euler intergration
        };
// Global variable indexes
#define THOVEN_Th 0

// Message boxes
#define THOVEN_MSG_CUR 0

#endif // SIMULATION

// Function Prototypes (list the common function prototypes first, then
//  those specific to this project):
void DisableInterrupt(void);
void EnableInterrupt(void);
int TaskDispatcher (TList *List1, TList *List2);
void StopControl(char *aMessage);
void SetTickRate(double tr);
double GetTickRate(void);
#ifdef ISR_SCHEDULING
void SetupTimerInterrupt(void);
void RestoreTimerInterrupt(void);
#endif

// Pointers to all tasks defined in the project must be listed here.
// The extern definitions are so that other tasks refer to each other
// across files.  Understand that no space is allocated for the tasks
// here.  The space is allocated with the 'new' operator in the
// main function.

#ifdef MAIN_CPP
    #define EXTERNAL  // So externs get declared properly
#else
    #define EXTERNAL extern
#endif

// Tasks
EXTERNAL CDataLogger *DataLogger;
EXTERNAL PIDControl *BeltControl1,*BeltControl2;
EXTERNAL PIDControl *HeatControl1,*HeatControl2;

#ifdef SIMULATION
    EXTERNAL CBeltAxis *BeltAxis1,*BeltAxis2;
    EXTERNAL CThOven *ThOven1,*ThOven2;
#endif

// Lists
    // The tasks lists defined only as pointers here to allow
    // the interrupt routine to access them.  Space should be
    // allocated for them using new() in the main() function.
EXTERNAL TList *LowPriority;
EXTERNAL TList *Intermittent;
#ifdef ISR_SCHEDULING
    EXTERNAL TList *Preemptable;
    EXTERNAL TList *Interrupt;
#endif // ISR_SCHEDULING
#endif // TASKS_HPP

