// Main file for the glueing/manufacturing example
// File: GlueMain.cpp

// Originally created: May 17, 1997 by D.M. Auslander
// Various versions of this file will be used to build a complete
// control system in steps. Comments will be additive to describe
// which level the file refers to.

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
*/

#define MAIN_CPP

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include <dos.h>
#include "tasks.hpp" // Task and environment info

static StopControlNow = 0;
char *StopMessage = NULL;
void StopControl(char *aMessage)
    {
    StopControlNow = 1;
    StopMessage = new char[strlen(aMessage)+1];
    strcpy(StopMessage, aMessage);
    }

static double EndTime = 200.0;  // make this value visible to other functions
          // in this file (use big number to run forever)

void main(void)
    {
    // These variables are used to keep track of when the
    // scheduler must exit.
    double TheTime;

    SetTickRate(0.00005);  // Set the basic sample time
    // Declaration of TList pointers are in tasks.hpp
    LowPriority = new TList("Low Priority");
    Intermittent = new TList("Intermittent");
    #ifdef ISR_SCHEDULING
    Preemptable = new TList("Preemptable");
    Interrupt = new TList("Interrupt");
    #endif // ISR_SCHEDULING

    // Create all of the task objects used in the project
    DataLogger = new CDataLogger("Data Logger",PROCESS_A);

    // Define task objects for running the simulation.
    // These tasks are scheduled just as any other tasks, and are
    // only added to the task list if a simulation is desired.
    // The actual simulation code in contained in the Run()
    // member functions.
    #ifdef SIMULATION
    // char *aName,double JJ,double bb,double Ktt, int ProcNo
    BeltAxis1 = new CBeltAxis("BeltAxis1",10.0,1.2,4.0,PROCESS_A);
    BeltAxis2 = new CBeltAxis("BeltAxis2",10.0,1.2,4.0,PROCESS_A);
    //char *aName,double aK,double aThAmb,
    // double aR,double aHCap,int ProcNo
    ThOven1 = new CThOven("ThOven1",10.0,20.0,20.0,
        2.0e3,PROCESS_A);
    ThOven2 = new CThOven("ThOven2",10.0,20.0,20.0,
        2.0e3,PROCESS_A);
    Clamp1 = new CClamp("Clamp1",0.5,0.3,PROCESS_A);
    Clamp2 = new CClamp("Clamp2",0.5,0.3,PROCESS_A);
    #endif //SIMULATION

    // Declare the control tasks after the simulation tasks
    // because they reference the simulation task pointers.

    // NOTE: When changing from simulation to REAL operaration,
    // the pointers used in the construction should either be
    // changed, or the same names should be used as in the simulations

    /*  Arguments for PID control constructor:
		PIDControl(char *name,double dtt,
            BaseTask *ActTask,int ActBox,
            BaseTask *ProcTask, int PrIndex,
            double aKp,double aKi,double aKd,double aMin,double aMax,
            double aSetpoint,int aControllerOn,
            int ProcNo=0); // Constructor
    */
    BeltControl1 = new PIDControl("BeltControl1",0.2,  // sample time
            BeltAxis1,BELTAXISMSG_Cur,  // where to send actuation
            BeltAxis1,BELTAXIS_x,  // where to get process value
            1.5,0.0,5.0,-5.0,5.0,  // gains
            0.0,0,  // Setpoint, start with controller OFF
            PROCESS_A);

    BeltControl2 = new PIDControl("BeltControl2",0.2,  // sample time
            BeltAxis2,BELTAXISMSG_Cur,  // where to send actuation
            BeltAxis2,BELTAXIS_x,  // where to get process value
            1.5,0.0,5.0,-5.0,5.0,  // gains
            0.0,0,  // Setpoint, start with controller OFF
            PROCESS_A);

    HeatControl1 = new PIDControl("HeatControl1",1.0,  // sample time
            ThOven1,THOVEN_MSG_CUR,  // where to send actuation
            ThOven1,THOVEN_Th,  // where to get process value
            10.0,2.0,0.0,0.0,14.5,  // gains
            120.0,1,  // Setpoint, start with controller ON
            PROCESS_A);

    HeatControl2 = new PIDControl("HeatControl2",1.0,  // sample time
            ThOven2,THOVEN_MSG_CUR,  // where to send actuation
            ThOven2,THOVEN_Th,  // where to get process value
            10.0,2.0,0.0,0.0,14.5,  // gains
            140.0,1,  // Setpoint, start with controller ON
            PROCESS_A);

    // Motion tasks to generate motion profiles for the belt axes
    /*
    CMotion::CMotion(char *name,BaseTask *aProfileOut,
        int aProfileOutBoxNo,int StartBox,int StopBox,
        BaseTask *a_taskCurPos,int a_indexCurPos,
        double a_accel,double a_vcruise,double dt,int ProcNo)
     */
    BeltMotion1 = new CMotion("BeltMotion1",BeltControl1,PID_SETPOINT,
        PID_START,PID_STOP,
        BeltAxis1,BELTAXIS_x,0.006,0.04,0.2,PROCESS_A);
    BeltMotion2 = new CMotion("BeltMotion2",BeltControl2,PID_SETPOINT,
        PID_START,PID_STOP,
        BeltAxis2,BELTAXIS_x,0.006,0.04,0.2,PROCESS_A);

    // Transport tasks
    Transport1 = new CTransport1("Transport1",PROCESS_A);
    Transport2 = new CTransport2("Transport2",PROCESS_A);

    // All tasks must be added to the task list here.
    // Only add tasks that are in this process
    #ifdef ISR_SCHEDULING
    #else // ISR_SCHEDULING
    #endif // ISR_SCHEDULING

    // The check for process is used for multi-process implementation
    //   to schedule active tasks in the right process. It is a
    //   dummy here.
    if(ThisProcess == PROCESS_A)LowPriority->Append(DataLogger);
    if(ThisProcess == PROCESS_A)LowPriority->Append(BeltMotion1);
    if(ThisProcess == PROCESS_A)LowPriority->Append(BeltMotion2);
    if(ThisProcess == PROCESS_A)LowPriority->Append(BeltControl1);
    if(ThisProcess == PROCESS_A)LowPriority->Append(BeltControl2);
    if(ThisProcess == PROCESS_A)LowPriority->Append(HeatControl1);
    if(ThisProcess == PROCESS_A)LowPriority->Append(HeatControl2);
    if(ThisProcess == PROCESS_A)LowPriority->Append(Transport1);
    if(ThisProcess == PROCESS_A)LowPriority->Append(Transport2);

    #ifdef SIMULATION
    if(ThisProcess == PROCESS_A)LowPriority->Append(BeltAxis1);
    if(ThisProcess == PROCESS_A)LowPriority->Append(BeltAxis2);
    if(ThisProcess == PROCESS_A)LowPriority->Append(ThOven1);
    if(ThisProcess == PROCESS_A)LowPriority->Append(ThOven2);
    if(ThisProcess == PROCESS_A)LowPriority->Append(Clamp1);
    if(ThisProcess == PROCESS_A)LowPriority->Append(Clamp2);
    #endif //SIMULATION

    // You must call this function to initialize the Audit
    // trail array before you write to it.
    InitAuditTrail();

    cout << "Maximize window if necessary. ENTER to continue...\n";
    getchar();

    #ifdef ISR_SCHEDULING
    SetupTimerInterrupt();
    #else // ISR_SCHEDULING
    // Set the proper mode for the timer
    SetupClock(-1.0);
    #endif // ISR_SCHEDULING

    #ifdef USE_UDP
        InitUDP(NumberOfProcesses,ThisProcess,ProcessAddress);
    #endif

    // Scheduler loop.  Inside this loop, you MUST NOT call
    // any terminal I/O functions such as printf, cout,
    // getchar, scanf, etc.  This includes inside the run
    // functions of the tasks.  These things are OK in
    // SIMULATION ONLY.  Use #ifdef SIMULATION (or something
    // similar) if you have sections of code in which you
    // want to print to the screen (for debugging).
    while (((TheTime = GetTimeNow()) <= EndTime)
                 && (TheTime >= 0.0) && (!StopControlNow))
      {
      // Run LowPriority and Intermittent list
      if (TaskDispatcher(LowPriority,Intermittent))
          StopControl("\n***Problem with background tasks***\n");

      #ifdef USE_UDP  // For multiprocess only, will not be compiled otherwise
        if(TheTime > NextNetTime)
            {
            CheckNet();  // This is the lowest priority position --
            // Higher priority positions may be more appropriate
            if(!SendAll())StopControl("\n***Problem sending network data***\n");
            NextNetTime += DeltaNetTime;
            }
      #endif
      }

    // End-of-run clean-up activities...

    #ifdef ISR_SCHEDULING
    RestoreTimerInterrupt();
    #endif // ISR_SCHEDULING

    #ifdef USE_UDP
        NetCleanUp();
    #endif

    // clrscr ();  // Clear the operator interface

   if (StopMessage == NULL)
      cout << "\n\n***Normal Program Termination***\n\n";
   else
      {
      cout << StopMessage;
      delete [] StopMessage;
      }

    // There are two versions of the audit trail, one with task
    //  names and one with task numbers instead (for matlab)
    WriteAuditTrail("trail.txt");
    WriteAuditTrailNum("trl_num.txt");
    // open a file to save the output data
    cout << "\nThe time at termination is " << TheTime << "\n";
    // Print the scan statistics to the screen and to a file
    WriteScanStat("scans.txt", TheTime);
    // Save the output data to a file
    WriteProfileStat("profiles.txt");  // Write profiles for any tasks
    // that have them
    // De-allocate space given to tasks
    delete (LowPriority);
    delete (Intermittent);
    #ifdef ISR_SCHEDULING
    delete (Preemptable);
    delete (Interrupt);
    #endif // ISR_SCHEDULING
    cout << "Hit any key to exit.\n";
    while(!kbhit()) ;  // Wait to exit
    }
    
