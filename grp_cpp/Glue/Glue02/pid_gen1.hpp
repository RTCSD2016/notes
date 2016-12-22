// Header definitions for generic PID control
// Created 10/3/96, DM Auslander
// Modified 5/29/97, DM Auslander
// Uses a combination of global data and messages to avoid
//  the need for virtual functions.
// Does not use any state structure (control is either on or off)

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
        int first;  // Determines if PID is just starting
        BaseTask *ActuationTask;  // Task that will get the
            //  Actuation value
        int ActuationBox;  // Box number to put actuation value into
        BaseTask *ProcessTask;  // Task from which to get proc. value
        int ProcValIndex;  // Global data index for process value
        int ControllerOn;
	 public:
		PIDControl(char *name,double dtt,
            BaseTask *ActTask,int ActBox,
            BaseTask *ProcTask, int PrIndex,
            double aKp,double aKi,double aKd,double aMin,double aMax,
            double aSetpoint,int aControllerOn,
            int ProcNo=0); // Constructor
		int Run(void);    // Run method
		double PIDCalc(void);    // Do the PID calculation
	 };
// Define Global variable indexes
#define PID_PROCESS 0
#define PID_ACTUATION 1
#define PID_ERROR 2
#define PID_SET 3

// Define message boxes for data input to PID Control
// The derived task should not have any message boxes since it has
// no run function but, just in case, these are defined at the top
// of the message box set!
#define PID_START NUM_MESSAGE_BOX-1
#define PID_SETPOINT NUM_MESSAGE_BOX-2
#define PID_KP NUM_MESSAGE_BOX-3
#define PID_KI NUM_MESSAGE_BOX-4
#define PID_KD NUM_MESSAGE_BOX-5
#define PID_MINV NUM_MESSAGE_BOX-6
#define PID_MAXV NUM_MESSAGE_BOX-7
#define PID_NEWGAINS NUM_MESSAGE_BOX-8
#define PID_STOP NUM_MESSAGE_BOX-9

