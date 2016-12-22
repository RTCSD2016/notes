// Header definitions for generic PID control
// Created 10/3/96, DM Auslander
// This version uses message passing for all of its data exchange

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
	 public:
		PIDControl(char *name,double dtt); // Constructor
		int Run(void);    // Run method
		double PIDCalc(void);    // Do the PID calculation
		virtual void SetActVal(double val){}     // Set the actuation value --
				// The real version of this must be supplied in the derived class
		virtual double GetProcVal(void){return 0.0;}     // Get the process value --
				//  The real version of this must be supplied in the derived class
      virtual double GetSetpoint(void){return 0.0;}
      virtual void GetGains(double *kp,double *ki,double *kd,
        		double *min,double *max){}
	 };

// Define message boxes for data input to PID Control
// The derived task should not have any message boxes since it has
// no run function but, just in case, these are defined at the top
// of the message box set!
#define PID_START NUM_MESSAGE_BOX-1

// Define Global variable indexes
#define PID_PROCESS 0
#define PID_ACTUATION 1
#define PID_ERROR 2
#define PID_SET 3

