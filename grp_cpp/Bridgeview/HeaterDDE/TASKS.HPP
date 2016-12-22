// Main header file for listing tasks for use in other task
// definitions.  Also set the timing and any other global
// #define statements here.
// File:  tasks.hpp
// Created 8/2/95 by Jason Jones
// Modified 10/5/95, from PWM example for
//   heater control example, DMA
// 10/18/95 DM Auslander, added operator interface
// 10/4/96, DM Auslander, modified for message passing, added
//   data logging task
// Modified 10/14/97, DM Auslander, add interface to Bridgeview (DDE)

#ifndef TASKS_HPP
#define TASKS_HPP

#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include "basetask.hpp"
#include "audit.hpp" // Transition audit file functions
#include "time_mod.hpp"  // Set the time keeping mode

// Determine whether the character-based operator interface will be
//  used or a DDE connection (initially to Bridgeview)

#define USE_DDE  // for DDE-based communication with an operator
      // interface program (such as Bridgeview)
// #define USE_CHAR_OP  // To use the native, character-based interface

#ifdef USE_DDE
  #include <windows.h>
  #include "DDE_Stuf.hpp"
  #include "MT_Debug.hpp"
#endif

#ifdef USE_CHAR_OP
  #include "oper_int.hpp"
#endif

#ifdef ISR_SCHEDULING
void interrupt TimerISR(...);
#endif

#define PI 3.14159

#define PROCESS_INFO // So that basetask.cpp does not put in defaults
const int ThisProcess = 0; // Process number
const int NumberOfProcesses = 1;  // Number of independent processes
 
// Put all task class definitions here.  Each class MUST
// be inherited from BaseTask and include a Run() function

class PWMTask : public BaseTask
	{
	public:
	  PWMTask(char *aName);      // Constuctor for the class.
	  ~PWMTask();     // Destructor for the class.
	  int Run(void);  // Run() is the function containing the actual task code
							//  that the scheduler loop calls.  The return value can
							//  be used as desired.
     void WritePWMRecord(char *filename);	// Write a data file
	private:
	  double DutyCycle;  // The 'x' indicates an exchange variable
	  double PWMOutput;  // Variable used for simulating output
	  double EndOnTime;
	  double EndOffTime;
	  double Period;
	  void PWMOn(void);
	  void PWMOff(void);
	  DataOutput *PWMRecord;	// Output object to record output changes
	};
// Define global variable indexes
#define PWM_OUTPUT 0

class HeatSupTask : public BaseTask
	{
	public:
		HeatSupTask(char *aName);// Constuctor for the class.
		~HeatSupTask();		// Destructor for the class.
		int Run(void); // Run() is the function containing
							//  the actual task code that the
							//  scheduler loop calls.  The return
							//  value can be used as desired.
	private:
		double temp_soak,temp_init,temp_final,soak_time,soak_end;
	   double setpt,temp;
	};
// Message boxes for HeatSup:
#define HEATSUP_PIDCONFIRM 2

// Global Data indexes
#define HEATSUP_SETPOINT 0

//      The class for the control object, derived from PIDControl

#include "pid_ctr2.hpp"  // Get generic PID definitions
class HeaterControl : public PIDControl
	 {
	 public:
		  HeaterControl(char *name);     // Constructor
		  void SetActVal(double val);      // Set the actuation value
		  double GetProcVal(void);     // Get the process value --
        double GetSetpoint(void);
        void GetGains(double *kp,double *ki,double *kd,
        		double *min,double *max);
  	 };
// Message boxes (be careful that these don't overlap with the
//   message boxes used by the generic portion of this class)

// Global data index definitions

class OperatorInterface : public BaseTask
	{
	public:
		OperatorInterface(char *aName);// Constuctor for the class.
		~OperatorInterface();		// Destructor for the class.
		int Run(void); // Run() function for this task
      void HeatSetup(void); // Does pre-real-time set up
	private:
		double setpoint,mcntrl,temperature,Time;
      #ifdef USE_CHAR_OP
		  COperatorWindow *OpWin1;	// Pointers for screens
      #endif

      #ifdef USE_DDE
         C_DDE_Server* DDEHeater;
         C_DDE_Topic* HeaterData;
         C_DDE_Item *dde_setpoint,*dde_mcntrl,*dde_temperature,*dde_Time;
         C_DDE_Item *dde_QuitCommand,*dde_kp,*dde_ki,*dde_kd,*dde_min;
         C_DDE_Item *dde_max,*dde_start_op,*dde_temp_final,*dde_ScreenNo;
         C_DDE_Item *dde_tickrate,*dde_stop_prog;
      #endif
      int stop_prog,start_op,ScreenNo;
      double kp,ki,kd,min,max,temp_final,tickrate;
	};
// Define global data indexes
#define OPINT_FINALSETPOINT 0
#define OPINT_KP 1
#define OPINT_KI 2
#define OPINT_KD 3
#define OPINT_MIN 4
#define OPINT_MAX 5
#define OPINT_NEWGAINS 6



// Data logging task (move data logging from the main() function
// to a task where it can be better controlled

class _DataLogger : public BaseTask
   {
   public:
      _DataLogger(char *name);  // Constructor
      ~_DataLogger(void); // Destructor
      int Run(void);
      void WriteFile(char *filename);

   private:
	   DataOutput *DataOut1;
      double set,val,mc;  // Data to be logged
   };

// Leave this line in to add a simulation task to be scheduled
// with the rest of the tasks in the background list.  This
// define is optional;  it is not used by any of the program
// code not written by the user.
#define SIMULATION

#ifdef SIMULATION
class SimulationTask : public BaseTask
	{
	public:
		SimulationTask(char *aName);// Constuctor for the class.
		~SimulationTask();		// Destructor for the class.
		int Run(void);
	private:
		double Kin,Kout,Csys;	// System parameters
		double temp_amb;	// Ambient temperature
		double temp;		// Heater temperature
		double previous_time;
      double pwm_out;
		};
// Define global variable indexes
#define SIM_TEMP 0

#endif // SIMULATION

// Function Prototypes:
void DisableInterrupt(void);
void EnableInterrupt(void);
void InitRecPWM(double period);
void RecPWM(int val);
void WritePWMRec(char *fname);
int TaskDispatcher (TList *List1, TList *List2);
void StopControl(char *aMessage);
void SetTickRate(double tr);
double GetTickRate(void);
#ifdef ISR_SCHEDULING
void SetupTimerInterrupt(void);
void RestoreTimerInterrupt(void);
#endif

// All tasks defined in the project must be listed here.  The
// extern definitions are so that other tasks can refer to each
// other.  Understand that no space is allocated for the tasks
// here.  The space is allocated with the 'new' operator in the
// main function.
#ifdef MAIN_CPP
	PWMTask *PWM;
	HeatSupTask *HeatSup;
	HeaterControl *HeatCtrl;
	OperatorInterface *OpInt;
   _DataLogger *DataLogger;
	#ifdef SIMULATION
	SimulationTask *SimTask;
	#endif
	// The tasks lists defined only as pointers here to allow
	// the interrupt routine to access them.  Space should be
	// allocated for them using new() in the main() function.
	TList *LowPriority;
	TList *Intermittent;
	#ifdef ISR_SCHEDULING
	TList *Preemptable;
	TList *Interrupt;
	#endif // ISR_SCHEDULING
#else // MAIN_CPP
	extern PWMTask *PWM;
	extern HeatSupTask *HeatSup;
	extern HeaterControl *HeatCtrl;
	extern OperatorInterface *OpInt;
   extern _DataLogger *DataLogger;
	#ifdef SIMULATION
	extern SimulationTask *SimTask;
	#endif
	#ifdef ISR_SCHEDULING
	extern TList *Preemptable;
	extern TList *Interrupt;
	#endif // ISR_SCHEDULING
#endif // MAIN_CPP

#endif // TASKS_HPP

