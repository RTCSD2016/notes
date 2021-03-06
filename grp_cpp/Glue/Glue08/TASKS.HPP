// Main header file for the glue/manufacturing example
// File: tasks.hpp
// Originally created: May 17, 1997 by D.M. Auslander
// Various versions of this file will be used to build a complete
// control system in steps. Comments will be additive to describe
// which level the file refers to.

// All class definitions for tasks are in this file as are external
// variable definitions, etc.

/*
0. Simulation of the two belt-driven axes only
1. Add simulation of the oven temperature (based on level 0)
2. Add PID control for the belts and heaters
3. Define a motion profile so acceleration can be controlled
    Don't define any command structure yet -- this should just be
    for debugging the profile generation.
    (Study numerics: interaction of dt for Euler simulation,
    controller sample time, controller gains, scheduler "tickrate")
4.  Add a command structure and notification to the motion task
      so it indicates when the profile
      is finsihed. Add tasks to exercise the axes.
5.  Simulate the clamp that holds the assembly onto the belt axis
      so it can be moved into the oven and then to the robot
      unloading station. The clamp simulation is in the sim.cpp
      file along with other simulation modules. One class is used
      to instantiate clamps for both belts. Modify the Transport to
      exercise the clamps.
6.  Add similar minimal simulations for the load robot, glue
      applicator, and unload robots. Change the Transport tasks
      back to a single class, which is more convenient in this
      problem. Again, use Transport to exercise these components.
7.  Do the actual production version of the transport task. This task
		takes the object from the assembly area to the oven for curing, then
		to the unload area.
8.  Putting it all together -- making widgets requires the proper sequence
      in the Assembly task. All of the other pieces are already in place.
*/

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

// Motion
// This class defines point-to-point motion profiles. The terminology
// is based on motion, but it can be used with any system that has
// a command signal

class CMotion : public BaseTask
    {
    public:
        CMotion(char *name,BaseTask *aProfileOut,
            int aProfileOutBoxNo,int StartBox,int StopBox,
            BaseTask *a_taskCurPos,int a_indexCurPos,
            double a_accel,double a_vcruise,
            double dt,int ProcNo);
        int Run(void);

    private:
        double StopPoint(double xc,double vc,double ac);
        double Sign(double x);
        int Straddle(double x0,double x1,double x2);
        double xtarget,xcur,vcur,acur;
        double accel,vcruise;
        int command;
        int StartBox,StopBox;  // Message box for cmds to start
            // and stop the controller
        BaseTask *ProfileOut,*taskCurPos,*Controller;
        int ProfileOutBoxNo,indexCurPos;
        int fromStart,fromStop,fromIdle,fromHold;  // Tasks that sent commands
        int respStart,respStop,respIdle,respHold;  // Response message boxes
        int InitialState;  // Mainly for debug
    };
// Global Data base assignments
#define MOTION_xcur 0
#define MOTION_STATE 1

// Message boxes -- each command has its own message box

// Command format:
// The arrival of the message is the command
// The integer value is the box number to which the results of the
//   command should be sent (-1 for none).
// The double value is the parameter, if any
// The completion message is: 1 for success; -n for failure where n
//   is an error number
#define MOTION_CMD_HOLD_POSITION 0
#define MOTION_CMD_START_PROFILE 1
#define MOTION_CMD_STOP 2
#define MOTION_CMD_IDLE 3
#define MOTION_CMD_NEW_PAR 4
#define MOTION_MSG_ACCEL 5
#define MOTION_MSG_VCRUISE 6

// Transport tasks -- these will ultimately be responsible
//  for sequencing each transport operation, moving the object
//  from the assembly position to the even, etc.
// Right now, they will just be used to exercise the belt axes
//  and other simulated components.

class CTransport : public BaseTask
    {
    public:
        CTransport(char *name,int aArmNo,BaseTask *aMotionTask,
    		BaseTask *aClampTask,BaseTask *aUnloadRobotTask,int ProcNo);
        int Run(void);
    private:
        double Time0,TWait1,TWait2;
        BaseTask *ClampTask;
        BaseTask *UnloadRobotTask;
        BaseTask *MotionTask;
        int ArmNo;   // So internal functions can tell which arm
   	 	// this is if they have to (not currently used)
        double LoadPos,ReadyPos,OvenPos,UnloadPos;
        double AccSlow,AccFast,VcSlow,VcFast;
        double CureTime;
        int cmdTask,cmdBox;
    };
// Message Boxes:
#define TRANSPORT_MSG_GetObject 0  // Command to pick up an object
		// at the assembly station and complete its processing
#define TRANSPORT_MSG_MoveDone 1  // Box for motion to send completion
		// message
#define TRANSPORT_MSG_UnloadClear 2  // For unload robot to signal object
      // is clear of the unloar area
// Global Variables
#define TRANSPORT_GLOBAL_Status 0
#define TRANSPORT_GLOBAL_State 1

// Status values: -1: error; 0: in transit
//  1: at ready; 2: in oven; 3: unloading
#define TRANSPORT_STATUS_Error -1
#define TRANSPORT_STATUS_InTransit 0
#define TRANSPORT_STATUS_AtReady 1
#define TRANSPORT_STATUS_InOven 2
#define TRANSPORT_STATUS_Unloading 3
#define TRANSPORT_STATUS_Loading 4

// [CTransport2 has been removed]

// Assembly task -- controls the assembly of base & cylinder and
//then tells a transport task to take the part through the rest
//of the process.

// This version (#7) just exercises the transport sequence
class CAssembly : public BaseTask
    {
    public:
      CAssembly(char *aName,int ProcNo);
      int Run(void);
    private:
      int ObjectsStarted,NumberToMake;
      BaseTask *TranNext;  // The axis that will be used for the next object
    };
// Message Boxes
#define ASSEMBLY_MSG_BaseDone 0
#define ASSEMBLY_MSG_GlueDone 1
#define ASSEMBLY_MSG_CylinderDone 2
#define ASSEMBLY_MSG_ObjectClear 3

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
        CBeltAxis(char *aName,double JJ,double bb,double Ktt,int ProcNo);
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
        double b;  // Damping
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

// Clamps
class CClamp : public BaseTask
    {
    public:
        CClamp(char *aName,double aCloseTime,double aOpenTime,
                int ProcNo);
        ~CClamp(){};
        int Run(void);
    private:
        double StartTime;
        double CloseTime,OpenTime;
        int InitialState;
    };
// Global Variable indexes
#define CLAMP_STATUS 0  // Code: 0 - closed; 1 - open; -1 - moving
#define CLAMP_STATE 1  // Current state

// Message boxes
#define CLAMP_MSG_Open 0  // Commands to open or close clamp
#define CLAMP_MSG_Close 1

// Load Robot (there is only  one of these)
class CLoadRobot : public BaseTask
    {
    public:
        CLoadRobot(char *aName,double aLoadBaseTime,
            double aLoadCylinderTime,double aToReadyTime,int ProcNo);
        int Run(void);
    private:
        double LoadBaseTime,LoadCylinderTime,ToReadyTime;
        double StartTime;
        int InitialState;
        int cmdFromTask,cmdFromBox;
    };
// Global data base
#define LOAD_ROBOT_State 0
#define LOAD_ROBOT_Status 1  // Codes:
    // 0 - Ready; 1 - Executing LoadBase; 2 - Executing LoadCylinder
// Messsage Boxes
#define LOAD_ROBOT_CMD_LoadCylinder 0
#define LOAD_ROBOT_CMD_LoadBase 1

// Glue applicator (only one of these also)
class CGlueApplicator : public BaseTask
    {
    public:
        CGlueApplicator(char *aName,double aGlueTime,
        double aToReadyTime,int ProcNo);
        int Run(void);
    private:
        int InitialState;
        double StartTime,GlueTime,ToReadyTime;
        int cmdFromTask,cmdFromBox;
    };
// Global data base
#define GLUE_APPLICATOR_State 0
#define GLUE_APPLICATOR_Status 1  // Codes:
    // 0 - Ready; 1 - Glueing; 2- Moving
// Message Boxes
#define GLUE_APPLICATOR_CMD_Apply 0

// Unload Robots (there are two of these)
class CUnloadRobot : public BaseTask
    {
    public:
        CUnloadRobot::CUnloadRobot(char *aName,double aUnloadTime,
            double aToReadyTime,int ProcNo);
        int Run(void);
    private:
        int InitialState;
        double StartTime,UnloadTime,ToReadyTime;
        int cmdFromTask,cmdFromBox;
    };
// Global Variables
#define UNLOAD_ROBOT_State 0
#define UNLOAD_ROBOT_Status 1

// Message boxes
#define UNLOAD_ROBOT_CMD_Unload 0

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
EXTERNAL CMotion *BeltMotion1,*BeltMotion2;
EXTERNAL PIDControl *HeatControl1,*HeatControl2;
EXTERNAL CTransport *Transport1,*Transport2;
EXTERNAL CAssembly *Assembly;

#ifdef SIMULATION
    EXTERNAL CBeltAxis *BeltAxis1,*BeltAxis2;
    EXTERNAL CThOven *ThOven1,*ThOven2;
    EXTERNAL CClamp *Clamp1,*Clamp2;
    EXTERNAL CLoadRobot *LoadRobot;
    EXTERNAL CGlueApplicator *GlueApplicator;
    EXTERNAL CUnloadRobot *UnloadRobot1,*UnloadRobot2;
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

