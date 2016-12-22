// Main header file for listing tasks for use in other task
// definitions.  Also set the timing and ony other global
// #define statements here.
// File:  tasks.hpp
// Created 8/2/95 by Jason Jones
// Modified 10/5/95, from PWM example for
//   heater control example, DMA
// 10/18/95 DM Auslander, added operator interface

#ifndef TASKS_HPP
#define TASKS_HPP

#include <windows.h>  //////////////////////////////////////////////////////////////

#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include "basetask.hpp"
#include "audit.hpp" // Transition audit file functions
#include "oper_int.hpp"  // Only include this if an operator
#include "time_mod.hpp"  // Set the time keeping mode

#ifdef ISR_SCHEDULING
void interrupt TimerISR(...);
#endif

#define PI 3.14159

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
	  void SetPeriod(double);
	  void SetDutyCycle(double);
	  double GetPWMOutput(void);
     void WritePWMRecord(char *filename);	// Write a data file
	private:
	  double DutyCycle,xDutyCycle;  // The 'x' indicates an exchange variable
	  double PWMOutput,xPWMOutput;  // Variable used for simulating output
	  double EndOnTime;
	  double EndOffTime;
	  double Period,xPeriod;
	  void PWMOn(void);
	  void PWMOff(void);
	  DataOutput *PWMRecord;	// Output object to record output changes
	};

class HeatSupTask : public BaseTask
	{
	public:
		HeatSupTask(char *aName);// Constuctor for the class.
		~HeatSupTask();		// Destructor for the class.
		double GetSetpoint(void);
		void SetTempFinal(double);
      double GetTempFinal(void);
		int Run(void); // Run() is the function containing
							//  the actual task code that the
							//  scheduler loop calls.  The return
							//  value can be used as desired.
	private:
		double temp_soak,temp_init,temp_final,soak_time,soak_end;
	      double setpt,xsetpt,xtemp_final;
	};

// The generic definition for a PID controller. The actual PID
// task is inherited from this one by defining real rather than
// virtual functions to get data and output actuation.
//=====================================================================================
//  Class PIDControl
// This is a generic class that can be used for implementing single-loop
// PID controllers. It requires a custom class to be derived from it. That
// class MUST contain the functions GetProcVal() and SetActVal() which
// are virtualized here and not expected to be used. Other than these
// functions and some local scaling factors, etc., there is nothing else
// required in the derived class.
// This class itself is derived from CTask, the class defined internally
// to handle most of the control task functionality.

class PIDControl : public BaseTask
	 {
	 protected:  // This is just the generic part of a control task
				// so all variables are made accessible to the derived class
		double integ;
		double set,val;      // Setpoint and output (position) value
		double prev_set,prev_val;
		double kp,ki,kd,min,max; // Controller gains, limits
		double dt;	// Controller sample interval
		double mc;       // Controller output
		int start_flag;     // Used for transition from initial state

		  // Exchange variables, set in this class
		double x_val,x_mc;
		  // Exchange variables obtained from other tasks
		double x_set,x_kp,x_ki,x_kd,x_min,x_max;
		int x_newgains;      // Use this as a flag to indicate that new gains
					 // have been set
		int x_start_flag;

	 public:
		PIDControl(char *name,double dtt); // Constructor
		int Run(void);    // Run method
		double PIDCalc(void);    // Do the PID calculation
		void SetGains(double kpv,double kiv,double kdv,double minv,double maxv);
		void GetGains(double *kpv,double *kiv,double *kdv,
				double *minv,double *maxv);
		void SetStart(void);    // Set the start flag to 1
		virtual void SetActVal(double val){}     // Set the actuation value --
				// The real version of this must be supplied in the derived class
		void SetSetpoint(double sp);
		void GetData(double *pval,double *pmc,double *pset);
		virtual double GetProcVal(void){return 0.0;}     // Get the process value --
				//  The real version of this must be supplied in the derived class
	 };

//      The class for the control object, derived from PIDControl

class HeaterControl : public PIDControl
	 {
	 public:
		  HeaterControl(char *name);     // Constructor
		  void SetActVal(double val);      // Set the actuation value
		  double GetProcVal(void);     // Get the process value --
	 };

class OperatorInterface : public BaseTask
	{
	public:
		OperatorInterface(char *aName);// Constuctor for the class.
		~OperatorInterface();		// Destructor for the class.
		int Run(void); // Run() function for this task
	private:
		double setpoint,mcntrl,temperature,Time;
		COperatorWindow *OpWin1;	// Pointers for screens
      double temp_final;
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
		double GetTemperature(void);
	private:
		double Kin,Kout,Csys;	// System parameters
		double temp_amb;	// Ambient temperature
		double temp,xtemp;		// Heater temperature
		double previous_time;
		};
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
void HeatSetup(void);
#ifdef ISR_SCHEDULING
void SetupTimerInterrupt(void);
void RestoreTimerInterrupt(void);
#endif

// All tasks defined in the project must be listed here.  The
// extern definitions are so that other tasks can refer to each
// other.  Understand that no space is allocated for the tasks
// here.  The space is allocated with the new operator in the
// main function.
#ifdef MAIN_CPP
	PWMTask *PWM;
	HeatSupTask *HeatSup;
	HeaterControl *HeatCtrl;
	OperatorInterface *OpInt;
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
	#ifdef SIMULATION
	extern SimulationTask *SimTask;
	#endif
	#ifdef ISR_SCHEDULING
	extern TList *Preemptable;
	extern TList *Interrupt;
	#endif // ISR_SCHEDULING
#endif // MAIN_CPP

#endif // TASKS_HPP

