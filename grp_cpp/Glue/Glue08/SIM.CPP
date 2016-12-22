// Glue/manufacturing example -- this file contains all of the
//  simulation related material
// File: sim.cpp
// Created May 27, 1997, DM Auslander

// 1. Added thermal simulation of the ovens to version 0
// 5. Added clamp simulation
// 6. Added robot simulations
// 7. Modified clamps to account for redundant commands (already open, etc)

#include <math.h>
#include "tasks.hpp"

CBeltAxis::CBeltAxis(char *aName,double JJ,double bb,double Ktt,int ProcNo)
          :BaseTask(aName,ProcNo)
    {
    DeltaTaskTime = 0.01; // Nominal task period, sec.
    NextTaskTime = 0.0;
    State = 0;         // set to initialize
    previous_time = 0.0;
    CreateGlobal(5);  // Create a global data base with 5 slots
    // ActivateProfiling(5.e-6,80.e-6,15,LOG_PROFILE);  // Not needed yet
    // Set parameters
    J = JJ;
    b = bb;
    Kt = Ktt;
    // Set initial conditions
      // Away from the assembly area so there is no
      // conflict
    x = 0.3;  // Position and velocity
    v = 0.0;
    cur = 0.0;
    }

CBeltAxis::~CBeltAxis(void)
    {
    }

int CBeltAxis::Run(void)
    {
    double Time;
    NumScans++; // Increment scan count

    if (State == 0)
        {
        // Initialization code.  If there is anything
        // that the task needs to do only once at
        // start-up, put it here.
        State = 1;  // To prevent this from running again
        previous_time = NextTaskTime = GetTimeNow();
        return (1);
        }

    Time = GetTimeNow();
    // Check for time to do the next simulation computations
    if(Time >= NextTaskTime)
        {
        //NextTaskTime += DeltaTaskTime;
        NextTaskTime = Time + DeltaTaskTime;

        double delt;

        Time = GetTimeNow();
        delt = Time - previous_time;  // Compute actual time
                // increment for Euler integration
        previous_time = Time;

        // Check to see if a new value for motor current has arrived
        int MsgFlag;
        double val;
        if(GetRTMessage(BELTAXISMSG_Cur,&MsgFlag,&val))cur = val;

        // Start belt axis simulation
        double torque = Kt * cur - b * v;
        v += (torque / J) * delt;
        x += v * delt;

        // Copy data to local arrays
        PrivateGlobalData[BELTAXIS_x] = x;
        PrivateGlobalData[BELTAXIS_v] = v;
        CopyGlobal();  // Make data available to other tasks
        }

    return (1);
    }

CThOven::CThOven(char *aName,double aK,double aThAmb,
                double aR,double aHCap,int ProcNo)
          :BaseTask(aName,ProcNo)
    {
    DeltaTaskTime = 0.1; // Nominal task period, sec.
    NextTaskTime = 0.0;
    State = 0;         // set to initialize
    previous_time = 0.0;
    CreateGlobal(5);  // Create a global data base with 5 slots
    // ActivateProfiling(5.e-6,80.e-6,15,LOG_PROFILE);  // Not needed yet
    // Set parameters
    K = aK;
    ThAmb = aThAmb;
    R = aR;
    Cur = 0.0;
    HCap = aHCap;
    // Set initial conditions
    Th = 0;  // Initial condition
    }

CThOven::~CThOven(void){};

int CThOven::Run(void)
    {
    double Time;
    NumScans++; // Increment scan count

    if (State == 0)
        {
        // Initialization code.  If there is anything
        // that the task needs to do only once at
        // start-up, put it here.
        State = 1;  // To prevent this from running again
        previous_time = NextTaskTime = GetTimeNow();
        return (1);
        }

    Time = GetTimeNow();
    // Check for time to do the next simulation computations
    if(Time >= NextTaskTime)
        {
        NextTaskTime += DeltaTaskTime;

        double delt;

        Time = GetTimeNow();
        delt = Time - previous_time;  // Compute actual time
                // increment for Euler integration
        previous_time = Time;

        // Start belt axis simulation
       // Check to see if a new value for heater current has arrived
        int MsgFlag;
        double val;
        if(GetRTMessage(THOVEN_MSG_CUR,&MsgFlag,&val))Cur = val;

        double HeatIn = Cur * Cur * R;
        double HeatOut = K * (Th - ThAmb);
        Th += ((HeatIn - HeatOut) / HCap)* delt;

        // Copy data to local arrays
        PrivateGlobalData[THOVEN_Th] = Th;
        CopyGlobal();  // Make data available to other tasks
        }
    return (1);
}

// Clamps
//States:
#define CLAMP_INITIALIZE 0
#define CLAMP_OPEN 1
#define CLAMP_CLOSED 2
#define CLAMP_MOVINGTO_OPEN 3
#define CLAMP_MOVINGTO_CLOSED 4

CClamp::CClamp(char *aName,double aCloseTime,double aOpenTime,
                int ProcNo) :BaseTask(aName,ProcNo)
    {
    CloseTime = aCloseTime;
    OpenTime = aOpenTime;
    InitialState = CLAMP_OPEN;

    CreateStateNameArray(10);  // So the audit trail will have
        // state names
    CreateGlobal(5);  // Create global data bases
    //ActivateProfiling(5.e-6,80.e-6,15,LOG_PROFILE);
    RegisterStateName("CLAMP_INITIALIZE",CLAMP_INITIALIZE);
    RegisterStateName("CLAMP_OPEN",CLAMP_OPEN);
    RegisterStateName("CLAMP_CLOSED",CLAMP_CLOSED);
    RegisterStateName("CLAMP_MOVINGTO_OPEN",CLAMP_MOVINGTO_OPEN);
    RegisterStateName("CLAMP_MOVINGTO_CLOSED",CLAMP_MOVINGTO_CLOSED);
    }

int CClamp::Run(void)
    {
    double Time;
    NumScans++; // Increment scan count

    if (State == CLAMP_INITIALIZE)
        {
        // Initialization code.  If there is anything
        // that the task needs to do only once at
        // start-up, put it here.
        State = NextState = InitialState;  // To prevent this from running again
        return (1);
        }
    // Don't bother checking time here -- none of the states of this
    //  task do anything more than check time.
    if (NextState != -1)
        {
        // record audit trail here if desired
        AuditTrail(this, State, NextState);
        State = NextState;
        PrivateGlobalData[CLAMP_STATE] = State;
        NextState = -1;
        RunEntry = 1;
        }
    else
        RunEntry = 0;

    switch(State)
        {
        case CLAMP_OPEN:
            // Entry
            if(RunEntry)
                {
                PrivateGlobalData[CLAMP_STATUS] = 1;  // Open
                }
            // Action
            // Test/Exit
            if(GetRTMessage(CLAMP_MSG_Close))
                {
                // Close the clamp
                NextState = CLAMP_MOVINGTO_CLOSED;
                }
            if(GetRTMessage(CLAMP_MSG_Open))
                {
                // Clamp is already open -- do self-transition so an
                //action will show up on the audit trail
                NextState = CLAMP_OPEN;
                }
            break;
        case CLAMP_CLOSED:
            // Entry
            if(RunEntry)
                {
                PrivateGlobalData[CLAMP_STATUS] = 0;  // Closed
                }
            // Action
            // Test/Exit
            if(GetRTMessage(CLAMP_MSG_Open))
                {
                // Open the clamp
                NextState = CLAMP_MOVINGTO_OPEN;
                }
            if(GetRTMessage(CLAMP_MSG_Close))
                {
                // Clamp is already closed -- do self-transition so an
                //action will show up on the audit trail
                NextState = CLAMP_CLOSED;
                }
            break;
        case CLAMP_MOVINGTO_OPEN:
            // Entry
            if(RunEntry)
                {
                PrivateGlobalData[CLAMP_STATUS] = -1;  // Moving
                StartTime = GetTimeNow();
                }
            // Action
            // Test/Exit
            if(GetTimeNow() >= (StartTime + CloseTime))
                {
                NextState = CLAMP_OPEN;
                }
            break;
        case CLAMP_MOVINGTO_CLOSED:
            // Entry
            if(RunEntry)
                {
                PrivateGlobalData[CLAMP_STATUS] = -1;  // Moving
                StartTime = GetTimeNow();
                }
            // Action
            // Test/Exit
            if(GetTimeNow() >= (StartTime + CloseTime))
                {
                NextState = CLAMP_CLOSED;
                }
            break;
        default:
            StopControl("<Clamp> Illegal state");
        }
    PrivateGlobalData[CLAMP_STATE] = State;
    CopyGlobal();  // Make global data visible
    return(1);
    }

// Simulated Robot Controllers
// The robot operation is simulated with time delays
// An actual robot controller would either communicate with
//the robot or with another task that actually controlled a robot

// Load Robot
//States:
#define Initialize 0
#define Start 1
#define PickupCylinder 2
#define PlaceCylinder 3
#define GoingToReady 4
#define AtReady 5
#define PickupBase 6
#define PlaceBase 7

CLoadRobot::CLoadRobot(char *aName,double aLoadBaseTime,
    double aLoadCylinderTime,
    double aToReadyTime,int ProcNo) :BaseTask(aName,ProcNo)
    {
    LoadBaseTime = aLoadBaseTime;
    LoadCylinderTime = aLoadCylinderTime;
    ToReadyTime = aToReadyTime;
    InitialState = Start;
    cmdFromBox = -1;  // Initial value

    CreateStateNameArray(10);  // So the audit trail will have
        // state names
    CreateGlobal(5);  // Create global data bases
    //ActivateProfiling(5.e-6,80.e-6,15,LOG_PROFILE);
    RegisterStateName("Initialize",Initialize);
    RegisterStateName("Start",Start);
    RegisterStateName("PickupCylinder",PickupCylinder);
    RegisterStateName("PlaceCylinder",PlaceCylinder);
    RegisterStateName("GoingToReady",GoingToReady);
    RegisterStateName("AtReady",AtReady);
    RegisterStateName("PickupBase",PickupBase);
    RegisterStateName("PlaceBase",PlaceBase);
    }

int CLoadRobot::Run(void)
    {
    double Time;
    NumScans++; // Increment scan count
    int MsgIndex,FromTask;
    double MsgVal;

    if (State == Initialize)
        {
        // Initialization code.  If there is anything
        // that the task needs to do only once at
        // start-up, put it here.
        State = NextState = InitialState;  // To prevent this from running again
        return (1);
        }
    // Don't bother checking time here -- none of the states of this
    //  task do anything more than check time.
    if (NextState != -1)
        {
        // record audit trail here if desired
        AuditTrail(this, State, NextState);
        State = NextState;
        PrivateGlobalData[LOAD_ROBOT_State] = State;
        NextState = -1;
        RunEntry = 1;
        }
    else
        RunEntry = 0;

    switch(State)
        {
        case Start:
            // Entry
            if(RunEntry)
                {
                PrivateGlobalData[LOAD_ROBOT_Status] = 0; // Ready
                }
            // Action
            // Test/Exit
            NextState = AtReady;  // Unconditional
            break;
        case AtReady:
            // Entry
            if(RunEntry)
                {
                PrivateGlobalData[LOAD_ROBOT_Status] = 0; // Ready
                SendCompletionMessage(cmdFromTask,cmdFromBox,1);
                }
            // Action
            // Test/Exit
            if(GetRTMessage(LOAD_ROBOT_CMD_LoadCylinder,&MsgIndex,
                        &MsgVal,&FromTask))
                {
                // Load the cylinder
                NextState = PickupCylinder;
                cmdFromTask = FromTask;
                cmdFromBox = MsgIndex;
                }
            if(GetRTMessage(LOAD_ROBOT_CMD_LoadBase,&MsgIndex,
                        &MsgVal,&FromTask))
                {
                // Load the base
                NextState = PickupBase;
                cmdFromTask = FromTask;
                cmdFromBox = MsgIndex;
                }
            break;
        case PickupCylinder:
            // Entry
            if(RunEntry)
                {
                PrivateGlobalData[LOAD_ROBOT_Status] = 2;  // Moving
                StartTime = GetTimeNow();
                }
            // Action
            // Test/Exit
            if(GetTimeNow() >= (StartTime + LoadCylinderTime/2.0))
                {
                NextState = PlaceCylinder;
                }
            break;
        case PlaceCylinder:
            // Entry
            if(RunEntry)
                {
                StartTime = GetTimeNow();
                }
            // Action
            // Test/Exit
            if(GetTimeNow() >= (StartTime + LoadCylinderTime/2.0))
                {
                NextState = GoingToReady;
                }
            break;
        case PickupBase:
            // Entry
            if(RunEntry)
                {
                PrivateGlobalData[LOAD_ROBOT_Status] = 1;  // Moving
                StartTime = GetTimeNow();
                }
            // Action
            // Test/Exit
            if(GetTimeNow() >= (StartTime + LoadBaseTime/2.0))
                {
                NextState = PlaceBase;
                }
            break;
        case PlaceBase:
            // Entry
            if(RunEntry)
                {
                StartTime = GetTimeNow();
                }
            // Action
            // Test/Exit
            if(GetTimeNow() >= (StartTime + LoadBaseTime/2.0))
                {
                NextState = GoingToReady;
                }
            break;
        case GoingToReady:
            // Entry
            if(RunEntry)
                {
                StartTime = GetTimeNow();
                }
            // Action
            // Test/Exit
            if(GetTimeNow() >= (StartTime + ToReadyTime))
                {
                NextState = AtReady;
                }
            break;
        default:
            StopControl("<LoadRobot> Illegal state");
        }
    PrivateGlobalData[LOAD_ROBOT_State] = State;
    CopyGlobal();  // Make global data visible
    return(1);
    }

// UnLoad Robot
//States:
#define Initialize 0
#define Start 1
#define PickupAssembly 2
#define PlaceAssembly 3
#define GoingToReady 4
#define AtReady 5

CUnloadRobot::CUnloadRobot(char *aName,double aUnloadTime,
            double aToReadyTime,int ProcNo) :BaseTask(aName,ProcNo)
    {
    UnloadTime = aUnloadTime;
    ToReadyTime = aToReadyTime;
    InitialState = Start;
    cmdFromBox = -1;  // Initial value

    CreateStateNameArray(10);  // So the audit trail will have
        // state names
    CreateGlobal(5);  // Create global data bases
    //ActivateProfiling(5.e-6,80.e-6,15,LOG_PROFILE);
    RegisterStateName("Initialize",Initialize);
    RegisterStateName("Start",Start);
    RegisterStateName("PickupAssembly",PickupAssembly);
    RegisterStateName("PlaceAssembly",PlaceAssembly);
    RegisterStateName("GoingToReady",GoingToReady);
    RegisterStateName("AtReady",AtReady);
    }

int CUnloadRobot::Run(void)
    {
    double Time;
    NumScans++; // Increment scan count
    int MsgIndex,FromTask;
    double MsgVal;

    if (State == Initialize)
        {
        // Initialization code.  If there is anything
        // that the task needs to do only once at
        // start-up, put it here.
        State = NextState = InitialState;  // To prevent this from running again
        return (1);
        }
    // Don't bother checking time here -- none of the states of this
    //  task do anything more than check time.
    if (NextState != -1)
        {
        // record audit trail here if desired
        AuditTrail(this, State, NextState);
        State = NextState;
        PrivateGlobalData[UNLOAD_ROBOT_State] = State;
        NextState = -1;
        RunEntry = 1;
        }
    else
        RunEntry = 0;

    switch(State)
        {
        case Start:
            // Entry
            if(RunEntry)
                {
                PrivateGlobalData[UNLOAD_ROBOT_Status] = 0; // Ready
                }
            // Action
            // Test/Exit
            NextState = AtReady;  // Unconditional
            break;
        case AtReady:
            // Entry
            if(RunEntry)
                {
                PrivateGlobalData[UNLOAD_ROBOT_Status] = 0; // Ready
                }
            // Action
            // Test/Exit
            if(GetRTMessage(UNLOAD_ROBOT_CMD_Unload,&MsgIndex,
                        &MsgVal,&FromTask))
                {
                // Unload the assembly from the conveyor
                NextState = PickupAssembly;
                cmdFromTask = FromTask;
                cmdFromBox = MsgIndex;
                }
            break;
        case PickupAssembly:
            // Entry
            if(RunEntry)
                {
                PrivateGlobalData[UNLOAD_ROBOT_Status] = 2;  // Moving
                StartTime = GetTimeNow();
                }
            // Action
            // Test/Exit
            if(GetTimeNow() >= (StartTime + UnloadTime/2.0))
                {
                NextState = PlaceAssembly;
                }
            break;
        case PlaceAssembly:
            // Entry
            if(RunEntry)
                {
                StartTime = GetTimeNow();
                SendCompletionMessage(cmdFromTask,cmdFromBox,1);
                  // Send completion message when robot has grasped
                  //the assembly. This allows overlapped operation.
                }
            // Action
            // Test/Exit
            if(GetTimeNow() >= (StartTime + UnloadTime/2.0))
                {
                NextState = GoingToReady;
                }
            break;
        case GoingToReady:
            // Entry
            if(RunEntry)
                {
                StartTime = GetTimeNow();
                }
            // Action
            // Test/Exit
            if(GetTimeNow() >= (StartTime + ToReadyTime))
                {
                NextState = AtReady;
                }
            break;
        default:
            StopControl("<UnloadRobot> Illegal state");
        }
    PrivateGlobalData[UNLOAD_ROBOT_State] = State;
    CopyGlobal();  // Make global data visible
    return(1);
    }

// Glue Applicator
//States:
#define Initialize 0
#define Start 1
#define MoveToGlueSpot 2
#define GlueApplication 3
#define GoingToReady 4
#define AtReady 5

CGlueApplicator::CGlueApplicator(char *aName,double aGlueTime,
        double aToReadyTime,int ProcNo) :BaseTask(aName,ProcNo)
    {
    GlueTime = aGlueTime;
    ToReadyTime = aToReadyTime;
    InitialState = Start;
    cmdFromBox = -1;  // Initial value

    CreateStateNameArray(10);  // So the audit trail will have
        // state names
    CreateGlobal(5);  // Create global data bases
    //ActivateProfiling(5.e-6,80.e-6,15,LOG_PROFILE);
    RegisterStateName("Initialize",Initialize);
    RegisterStateName("Start",Start);
    RegisterStateName("MoveToGlueSpot",MoveToGlueSpot);
    RegisterStateName("GlueApplication",PlaceAssembly);
    RegisterStateName("GoingToReady",GoingToReady);
    RegisterStateName("AtReady",AtReady);
    }

int CGlueApplicator::Run(void)
    {
    double Time;
    NumScans++; // Increment scan count
    int MsgIndex,FromTask;
    double MsgVal;

    if (State == Initialize)
        {
        // Initialization code.  If there is anything
        // that the task needs to do only once at
        // start-up, put it here.
        State = NextState = InitialState;  // To prevent this from running again
        return (1);
        }
    // Don't bother checking time here -- none of the states of this
    //  task do anything more than check time.
    if (NextState != -1)
        {
        // record audit trail here if desired
        AuditTrail(this, State, NextState);
        State = NextState;
        PrivateGlobalData[GLUE_APPLICATOR_State] = State;
        NextState = -1;
        RunEntry = 1;
        }
    else
        RunEntry = 0;

    switch(State)
        {
        case Start:
            // Entry
            if(RunEntry)
                {
                PrivateGlobalData[GLUE_APPLICATOR_Status] = 0; // Ready
                }
            // Action
            // Test/Exit
            NextState = AtReady;  // Unconditional
            break;
        case AtReady:
            // Entry
            if(RunEntry)
                {
                PrivateGlobalData[GLUE_APPLICATOR_Status] = 0; // Ready
                SendCompletionMessage(cmdFromTask,cmdFromBox,1);
                }
            // Action
            // Test/Exit
            if(GetRTMessage(GLUE_APPLICATOR_CMD_Apply,&MsgIndex,
                        &MsgVal,&FromTask))
                {
                // Unload the assembly from the conveyor
                NextState = MoveToGlueSpot;
                cmdFromTask = FromTask;
                cmdFromBox = MsgIndex;
                }
            break;
        case MoveToGlueSpot:
            // Entry
            if(RunEntry)
                {
                PrivateGlobalData[GLUE_APPLICATOR_Status] = 2;  // Moving
                StartTime = GetTimeNow();
                }
            // Action
            // Test/Exit
            if(GetTimeNow() >= (StartTime + ToReadyTime))
                // Time getting from READY is the same as to READY
                {
                NextState = GlueApplication;
                }
            break;
        case GlueApplication:
            // Entry
            if(RunEntry)
                {
                StartTime = GetTimeNow();
                PrivateGlobalData[GLUE_APPLICATOR_Status] = 1;  // Glueing
                }
            // Action
            // Test/Exit
            if(GetTimeNow() >= (StartTime + GlueTime))
                {
                NextState = GoingToReady;
                }
            break;
        case GoingToReady:
            // Entry
            if(RunEntry)
                {
                StartTime = GetTimeNow();
                PrivateGlobalData[GLUE_APPLICATOR_Status] = 2;  // Moving
                }
            // Action
            // Test/Exit
            if(GetTimeNow() >= (StartTime + ToReadyTime))
                {
                NextState = AtReady;
                }
            break;
        default:
            StopControl("<GlueApplicator> Illegal state");
        }
    PrivateGlobalData[GLUE_APPLICATOR_State] = State;
    CopyGlobal();  // Make global data visible
    return(1);
    }

