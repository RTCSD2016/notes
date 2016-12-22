// Main header file for listing tasks for use in other task
// definitions.  Also set the timing and ony other global
// #define statements here.
// File:  tasks.hpp
// Created 8/2/95 by Jason Jones
// Modified 10/5/95, from PWM example for
// PWM example, 12/25/95, DM Auslander
// This version has two PWM tasks, each inherited from
// a PWM base task

#ifndef TASKS_HPP
#define TASKS_HPP

#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include "basetask.hpp"
#include "audit.hpp" // Transition audit file functions
//#include "oper_int.hpp"  // Only include this if an operator
		// interface is being used
#include "time_mod.hpp"  // Set the time keeping mode

#ifdef ISR_SCHEDULING
void interrupt TimerISR(...);
#endif


#define PI 3.14159

// Put all task class definitions here.  Each class MUST
// be inherited from BaseTask and include a Run() function

class PWMBaseTask : public BaseTask
	{
	public:
	  PWMBaseTask(char *aName);      // Constuctor for the class.
	  ~PWMBaseTask();     // Destructor for the class.
	  int Run(void);  // Run() is the function containing the actual task code
							//  that the scheduler loop calls.  The return value can
							//  be used as desired.
	  void SetPeriod(double);
	  void SetDutyCycle(double);
	  double GetPWMOutput(void);
     void WritePWMRecord(char *filename);	// Write a data file
	protected:
	  double DutyCycle,xDutyCycle;  // The 'x' indicates an exchange variable
	  double PWMOutput,xPWMOutput;  // Variable used for simulating output
	  double EndOnTime;
	  double EndOffTime;
	  double Period,xPeriod;
	  virtual void PWMOn(void);  // These functions will be defined in
	  virtual void PWMOff(void);  // the classes that inherit from this
	  DataOutput *PWMRecord;	// Output object to record output changes
	};

// These classes are inherited from the PWMBase class so that
// PWM tasks with independent versions of PWMOn() and PWMOff()
// can be created.
class PWMTask1 : public PWMBaseTask
   {
   public:
      PWMTask1(char *aName);      // Constuctor for the class.
	   ~PWMTask1();     // Destructor for the class.
   private:
	  virtual void PWMOn(void);  // These functions are actually defined
	  virtual void PWMOff(void);  // here (the 'virtual' keyword is not
            // needed but is is useful for readability)
   };

class PWMTask2 : public PWMBaseTask
   {
   public:
      PWMTask2(char *aName);      // Constuctor for the class.
	   ~PWMTask2();     // Destructor for the class.
   private:
	  virtual void PWMOn(void);  // These functions are actually defined
	  virtual void PWMOff(void);  // here (the 'virtual' keyword is not
            // needed but is is useful for readability)
   };

class PWMSupTask : public BaseTask
	{
	public:
		PWMSupTask(char *aName,double DutyCyclePer1,
         double DutyCyclePer2);// Constructor
		~PWMSupTask();		// Destructor for the class.
      double GetDutyCycle(int i);
		int Run(void); // Run() is the function containing
							//  the actual task code that the
							//  scheduler loop calls.  The return
							//  value can be used as desired.
	private:
      double dc_period1,xdc_period1,dc_period2,xdc_period2;
      double dutycycle1,xdutycycle1,dutycycle2,xdutycycle2;
	};

// Leave this line in to add a simulation task to be scheduled
// with the rest of the tasks in the background list.  This
// define is optional;  it is not used by any of the program
// code not written by the user.
// No simulation in this (PWM) project
//#define SIMULATION

#ifdef SIMULATION
class SimulationTask : public BaseTask
	{
	public:
		SimulationTask(char *aName);// Constuctor for the class.
		~SimulationTask();		// Destructor for the class.
		int Run(void);
	private:
		};
#endif // SIMULATION

#ifdef ISR_SCHEDULING
// Function prototypes for timer interrupt scheduling
void SetupTimerInterrupt(void);
void RestoreTimerInterrupt(void);
#endif

// Function Prototypes:
void InitRecPWM(double period);
void RecPWM(int val);
void WritePWMRec(char *fname);
void StopControl(char *aMessage);
void SetTickRate(double tr);
double GetTickRate(void);

// All tasks defined in the project must be listed here.  The
// extern definitions are so that tasks can refer to each
// other.  Understand that no space is allocated for the tasks
// here.  The space is allocated with the new operator in the
// main function.
#ifdef MAIN_CPP
	PWMTask1 *PWM1;
	PWMTask2 *PWM2;
	PWMSupTask *PWMSup;
	#ifdef SIMULATION
	SimulationTask *SimTask;
	#endif
	// The tasks lists defined only as pointers here to allow
	// the interrupt routine to access them.  Space should be
	// allocated for them using 'new' in the main() function.
	TList *LowPriority;
	TList *Intermittent;
	#ifdef ISR_SCHEDULING
	TList *Preemptable;
	TList *Interrupt;
	#endif // ISR_SCHEDULING
#else // MAIN_CPP
	extern PWMTask1 *PWM1;
	extern PWMTask2 *PWM2;
	extern PWMSupTask *PWMSup;
	#ifdef SIMULATION
	extern SimulationTask *SimTask;
	#endif
	#ifdef ISR_SCHEDULING
	extern TList *Preemptable;
	extern TList *Interrupt;
	#endif // ISR_SCHEDULING
#endif // MAIN_CPP

#endif // TASKS_HPP

