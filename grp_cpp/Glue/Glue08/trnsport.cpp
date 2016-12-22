/* Transport tasks, classes CTransport1 and CTransport2
File: trnsport.cpp

Created June 12, 1997, DM Auslander

Handles the transport of the assembled object from the assembly area
to the oven for curing of the glue, then on to the unload area where
the unload robot removes the object.

The two classes have been combined back to a single class.
*/

#include <math.h>
#include <tasks.hpp>

// States:
#define Initialize 0
#define Start 1
#define AtReady 2
#define MoveToAssy 3
#define CloseClamp 4
#define MoveToOven 5
#define Cure 6
#define MoveToUnload 7
#define OpenClamp1 8
#define Unload 9
#define MoveToReady 10
#define OpenClamp2 11

static int InitialState = Start;

CTransport::CTransport(char *name,int aArmNo,BaseTask *aMotionTask,
    BaseTask *aClampTask,BaseTask *aUnloadRobotTask,int ProcNo)
        : BaseTask(name,ProcNo)
    {
    ClampTask = aClampTask;
    MotionTask = aMotionTask;
    UnloadRobotTask = aUnloadRobotTask;
    ArmNo = aArmNo;  // So internal functions can tell which arm
        // this is if they have to (not currently used)
    LoadPos = 0.04;  // target positions
    ReadyPos = 0.25;
    OvenPos = 0.96;
    UnloadPos = 1.3;
    AccSlow = 0.006;  // Motion parameters
    AccFast = 0.015;
    VcSlow = 0.06;
    VcFast = 0.1;
    CureTime = 10.0;
    cmdBox = -1;  // Default value for box to respond to command
    CreateGlobal(5);  // Create global data bases
    CreateStateNameArray(20);  // So the audit trail will have
        // state names
    RegisterStateName("Initialize",Initialize);
    RegisterStateName("Start",Start);
    RegisterStateName("AtReady",AtReady);
    RegisterStateName("MoveToAssy",MoveToAssy);
    RegisterStateName("CloseClamp",CloseClamp);
    RegisterStateName("MoveToOven",MoveToOven);
    RegisterStateName("MoveToOven",MoveToOven);
    RegisterStateName("Cure",Cure);
    RegisterStateName("MoveToUnload",MoveToUnload);
    RegisterStateName("OpenClamp1",OpenClamp1);
    RegisterStateName("OpenClamp2",OpenClamp2);
    RegisterStateName("Unload",Unload);
    RegisterStateName("MoveToReady",MoveToReady);
    }

int CTransport::Run(void)
    {
    int MsgIndex,FrTask;
    double MsgValue,MsgVal;
    int done = 1;  // Default is don't ask for more scans
    NumScans++; // Increment scan count

    if (State == Initialize)
        {
        State = InitialState;
        NextState = InitialState;
        return (done);
        }
    if (NextState != -1)
        {
        // record audit trail here if desired
        AuditTrail(this, State, NextState);
        State = NextState;
        NextState = -1;
        RunEntry = 1;
        }
    else
        RunEntry = 0;

    switch (State)
        {
        case Start:
                // Entry
            if(RunEntry)
                {
               // Set acceleration and cruise velocity to fast
               SendRTMessage(MotionTask,MOTION_MSG_ACCEL,0,AccFast,
                   MESSAGE_OVERWRITE);
               SendRTMessage(MotionTask,MOTION_MSG_VCRUISE,0,
                   VcFast,MESSAGE_OVERWRITE);
               SendRTMessage(MotionTask,MOTION_CMD_NEW_PAR);
               // Send a move command to Motion task
               SendRTMessage(MotionTask,MOTION_CMD_START_PROFILE,
                   TRANSPORT_MSG_MoveDone,ReadyPos);
               PrivateGlobalData[TRANSPORT_GLOBAL_Status] =
                    TRANSPORT_STATUS_InTransit;
               }
            // Action
            // Test/Exit
            // Check for completion of the move
            if(GetRTMessage(TRANSPORT_MSG_MoveDone,&MsgIndex,&MsgValue))
                {
                // Move is finished
                if(MsgIndex == 1)
                    // Successful Move
                    NextState = AtReady;
                else
                    StopControl("<Transport> Move to Ready failed\n");
                }
            break;
        case AtReady:
                // Entry
                if(RunEntry)
                {
               PrivateGlobalData[TRANSPORT_GLOBAL_Status] =
                    TRANSPORT_STATUS_AtReady;
               }
            // Action
            // Test/Exit
            if(GetRTMessage(TRANSPORT_MSG_GetObject,&MsgIndex,
                    &MsgVal,&FrTask))
                {
                // Command to pick up and move the assembly object
                cmdTask = FrTask;
                cmdBox = MsgIndex;
                NextState = OpenClamp1;
                }

            break;
        case OpenClamp1:
                // Entry
            if(RunEntry)
                {
               SendRTMessage(ClampTask,CLAMP_MSG_Open);  // Command to clamp
               PrivateGlobalData[TRANSPORT_GLOBAL_Status] =
                    TRANSPORT_STATUS_Loading;
               }
            // Action
            // Test/Exit
            if(GetGlobalData(ClampTask,CLAMP_STATUS) == 1)
                {
                // Clamp is open
                NextState = MoveToAssy;
                }
            break;
        case MoveToAssy:
                // Entry
            if(RunEntry)
                {
               // Set acceleration and cruise velocity to fast
               SendRTMessage(MotionTask,MOTION_MSG_ACCEL,0,AccFast,
                   MESSAGE_OVERWRITE);
               SendRTMessage(MotionTask,MOTION_MSG_VCRUISE,0,
                   VcFast,MESSAGE_OVERWRITE);
               SendRTMessage(MotionTask,MOTION_CMD_NEW_PAR);
               // Send a move command to Motion task
               SendRTMessage(MotionTask,MOTION_CMD_START_PROFILE,
                   TRANSPORT_MSG_MoveDone,LoadPos);
               PrivateGlobalData[TRANSPORT_GLOBAL_Status] =
                    TRANSPORT_STATUS_InTransit;
               }
            // Action
            // Test/Exit
            // Check for completion of the move
            if(GetRTMessage(TRANSPORT_MSG_MoveDone,&MsgIndex,&MsgValue))
                {
                // Move is finished
                if(MsgIndex == 1)
                    // Successful Move
                    NextState = CloseClamp;
                else
                    StopControl("<Transport> Move to Ready failed\n");
                }
            break;
        case CloseClamp:
                // Entry
            if(RunEntry)
                {
               SendRTMessage(ClampTask,CLAMP_MSG_Close);  // Command to clamp
               PrivateGlobalData[TRANSPORT_GLOBAL_Status] =
                    TRANSPORT_STATUS_Loading;
               }
            // Action
            // Test/Exit
            if(GetGlobalData(ClampTask,CLAMP_STATUS) == 0)
                {
                // Clamp is closed
                NextState = MoveToOven;
                }
            break;
        case MoveToOven:
                // Entry
            if(RunEntry)
                {
                // Set acceleration and cruise velocity to slow
                SendRTMessage(MotionTask,MOTION_MSG_ACCEL,0,AccSlow,
                    MESSAGE_OVERWRITE);
                SendRTMessage(MotionTask,MOTION_MSG_VCRUISE,0,
                    VcSlow,MESSAGE_OVERWRITE);
                SendRTMessage(MotionTask,MOTION_CMD_NEW_PAR);
                // Send a move command to belt #1 Motion task
                SendRTMessage(MotionTask,MOTION_CMD_START_PROFILE,
                    TRANSPORT_MSG_MoveDone,OvenPos);
               PrivateGlobalData[TRANSPORT_GLOBAL_Status] =
                    TRANSPORT_STATUS_InTransit;
               }
            // Action
            // Test/Exit
            // Check for completion of the move
            if(GetRTMessage(TRANSPORT_MSG_MoveDone,&MsgIndex,&MsgValue))
                {
                // Move is finished
                if(MsgIndex == 1)
                  {
                    // Successful Move
                  // Let requesting task know that the assembly
                  //is clear of the assembly area
                  SendCompletionMessage(cmdTask,cmdBox,1);
                   NextState = Cure;
                  }
                else
                    StopControl("<Transport> Move to Oven failed\n");
                }
            break;
        case Cure:  // Wait for cure time
                // Entry
            if(RunEntry)
                {
               PrivateGlobalData[TRANSPORT_GLOBAL_Status] =
                    TRANSPORT_STATUS_InOven;
               Time0 = GetTimeNow();
               }
            // Action
            // Test Exit
            if(GetTimeNow() >= (Time0 + CureTime))NextState = MoveToUnload;
            break;
        case MoveToUnload:
                // Entry
            if(RunEntry)
                {
               // Set acceleration and cruise velocity to fast
               SendRTMessage(MotionTask,MOTION_MSG_ACCEL,0,AccFast,
                   MESSAGE_OVERWRITE);
               SendRTMessage(MotionTask,MOTION_MSG_VCRUISE,0,
                   VcFast,MESSAGE_OVERWRITE);
               SendRTMessage(MotionTask,MOTION_CMD_NEW_PAR);
               // Send a move command to Motion task
               SendRTMessage(MotionTask,MOTION_CMD_START_PROFILE,
                   TRANSPORT_MSG_MoveDone,UnloadPos);
               PrivateGlobalData[TRANSPORT_GLOBAL_Status] =
                    TRANSPORT_STATUS_InTransit;
               }
            // Action
            // Test/Exit
            // Check for completion of the move
            if(GetRTMessage(TRANSPORT_MSG_MoveDone,&MsgIndex,&MsgValue))
                {
                // Move is finished
                if(MsgIndex == 1)
                    // Successful Move
                    NextState = OpenClamp2;
                else
                    StopControl("<Transport> Move to Unload failed\n");
                }
            break;
        case OpenClamp2:
                // Entry
            if(RunEntry)
                {
               SendRTMessage(ClampTask,CLAMP_MSG_Open);  // Command to clamp
               PrivateGlobalData[TRANSPORT_GLOBAL_Status] =
                    TRANSPORT_STATUS_Loading;
               }
            // Action
            // Test/Exit
            if(GetGlobalData(ClampTask,CLAMP_STATUS) == 1)
                {
                // Clamp is open
                NextState = Unload;
                }
            break;
        case Unload:
            // Entry
            if(RunEntry)
                {
                SendRTMessage(UnloadRobotTask,UNLOAD_ROBOT_CMD_Unload,
                    TRANSPORT_MSG_UnloadClear);
                }
            // Action
            // Test/Exit
            if(GetRTMessage(TRANSPORT_MSG_UnloadClear,&MsgIndex))
                {
                if(MsgIndex == 1)
                  NextState = MoveToReady;  // Wait for robot to get assembly
                  // clear of the unload area
                else StopControl("<Transport> Error in unload robot\n");
                }
            break;
            break;
        case MoveToReady:
                // Entry
            if(RunEntry)
                {
               // Set acceleration and cruise velocity to fast
               SendRTMessage(MotionTask,MOTION_MSG_ACCEL,0,AccFast,
                   MESSAGE_OVERWRITE);
               SendRTMessage(MotionTask,MOTION_MSG_VCRUISE,0,
                   VcFast,MESSAGE_OVERWRITE);
               SendRTMessage(MotionTask,MOTION_CMD_NEW_PAR);
               // Send a move command to Motion task
               SendRTMessage(MotionTask,MOTION_CMD_START_PROFILE,
                   TRANSPORT_MSG_MoveDone,ReadyPos);
               PrivateGlobalData[TRANSPORT_GLOBAL_Status] =
                    TRANSPORT_STATUS_InTransit;
               }
            // Action
            // Test/Exit
            // Check for completion of the move
            if(GetRTMessage(TRANSPORT_MSG_MoveDone,&MsgIndex,&MsgValue))
                {
                // Move is finished
                if(MsgIndex == 1)
                    // Successful Move
                    NextState = AtReady;
                else
                    StopControl("<Transport> Move to Ready failed\n");
                }
            break;

        default:
            StopControl("<Transport> Illegal state");
        }
    PrivateGlobalData[TRANSPORT_GLOBAL_State] = State;
    CopyGlobal();  // Make global data visible
    return(done);
    }

