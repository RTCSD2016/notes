/* Assembly Task: CAssembly
File: Assembly.cpp

Created July 15, 1997, DM Auslander

This task handles the sequencing of the basic assembly process. It
places a base, applies glue, then places the cyclinder. At that point,
it finds a transport mechanism that is open and sends the part through
the rest of the process

This version implements the full manufacturing sequence
*/

#include <math.h>
#include <tasks.hpp>

// States:
#define Initialize 0
#define Start 1
#define PlaceBase 2
#define ApplyGlue 3
#define PlaceCylinder 4
#define WaitForTransport 5
#define CureUnload 6
#define BeginNewObject 7
#define Shutdown 8

static int InitialState = Start;

CAssembly::CAssembly(char *aName,int ProcNo)
        : BaseTask(aName,ProcNo)
    {
    InitialState = Start;
    NumberToMake = 4;
    ObjectsStarted = 0;
    CreateStateNameArray(20);  // So the audit trail will have
        // state names
    RegisterStateName("Initialize",Initialize);
    RegisterStateName("Start",Start);
    RegisterStateName("PlaceBase",PlaceBase);
    RegisterStateName("ApplyGlue",ApplyGlue);
    RegisterStateName("PlaceCylinder",PlaceCylinder);
    RegisterStateName("WaitForTransport",WaitForTransport);
    RegisterStateName("CureUnload",CureUnload);
    RegisterStateName("BeginNewObject",BeginNewObject);
    RegisterStateName("Shutdown",Shutdown);
    }

int CAssembly::Run(void)
    {
    int MsgIndex;
    double MsgValue;
    int done = 1;  // Default is don't ask for more scans
    int t1,t2;
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
            // Action
            // Test/Exit
            // Wait for belts to get to their AtReady position
            t1 = GetGlobalData(Transport1,TRANSPORT_GLOBAL_Status);
            t2 = GetGlobalData(Transport2,TRANSPORT_GLOBAL_Status);
            if((t1 == TRANSPORT_STATUS_AtReady) &&
               (t2 == TRANSPORT_STATUS_AtReady))
               {
               NextState = BeginNewObject;
               }
            break;
        case CureUnload:  // Send object thorugh the oven and then to unload
            // Entry
            if(RunEntry)
               {
               // Send transport through its paces
               SendRTMessage(TranNext,TRANSPORT_MSG_GetObject,
                  ASSEMBLY_MSG_ObjectClear);
               }
            //Action
            // Test/Exit
            // Wait for belt to move out of assembly area
            if(GetRTMessage(ASSEMBLY_MSG_ObjectClear,&MsgIndex))
                {
                if(MsgIndex == 1)
                  NextState = BeginNewObject;
                else StopControl("<Assembly> Error sending object to Transport\n");
                }
            break;
        case BeginNewObject:
            // Entry
            // Action
            // Test/Exit
            if(ObjectsStarted >= NumberToMake)
               {
               NextState = Shutdown;
               }
            else if(GetGlobalData(LoadRobot,LOAD_ROBOT_Status) == 0)
               {
               // Robot is ready
               ObjectsStarted++;  // Record an object started
               NextState = PlaceBase;
               }
            break;
        case PlaceBase:
            // Entry
            if(RunEntry)
                {
                // Send command to the robot to put a base in position
                SendRTMessage(LoadRobot,LOAD_ROBOT_CMD_LoadBase,
                  ASSEMBLY_MSG_BaseDone);
                }
            // Action
            // Test/Exit
            if(GetRTMessage(ASSEMBLY_MSG_BaseDone,&MsgIndex))
                {
                if(MsgIndex == 1)
                  NextState = ApplyGlue;  // Wait for robot to finish
                else StopControl("<Assembly> Error placing base\n");
                }
            break;
        case ApplyGlue:
            // Entry
            if(RunEntry)
                {
                // Send command to the robot to put a base in position
                SendRTMessage(GlueApplicator,GLUE_APPLICATOR_CMD_Apply,
                  ASSEMBLY_MSG_GlueDone);
                }
            // Action
            // Test/Exit
            if(GetRTMessage(ASSEMBLY_MSG_GlueDone,&MsgIndex))
                {
                if(MsgIndex == 1)
                  NextState = PlaceCylinder;  // Wait for robot to finish
                else StopControl("<Assembly> Error applying glue\n");
                }
            break;
        case PlaceCylinder:
            // Entry
            if(RunEntry)
                {
                // Send command to the robot to put a base in position
                SendRTMessage(LoadRobot,LOAD_ROBOT_CMD_LoadCylinder,
                  ASSEMBLY_MSG_CylinderDone);
                }
            // Action
            // Test/Exit
            if(GetRTMessage(ASSEMBLY_MSG_CylinderDone,&MsgIndex))
                {
                if(MsgIndex == 1)
                  NextState = WaitForTransport;  // Wait for robot to finish
                else StopControl("<Assembly> Error placing base\n");
                }
            break;
        case WaitForTransport:
            // Entry
            // Action
            // Test Exit
            // Decide which transport mechanism to use. This state can
            //be used to change preferences, for example, if one mechanism
            //is not working
            t1 = GetGlobalData(Transport1,TRANSPORT_GLOBAL_Status);
            t2 = GetGlobalData(Transport2,TRANSPORT_GLOBAL_Status);
            if(t1 == TRANSPORT_STATUS_AtReady)
               {
               TranNext = Transport1;
               NextState = CureUnload;  // Send the object off!
               }
            else if(t2 == TRANSPORT_STATUS_AtReady)
               {
               TranNext = Transport2;
               NextState = CureUnload;  // Send the object off!
               }
            break;
        case Shutdown:
            // Entry
            // Action
            // Test/Exit
            // Wait for both transport mechanisms to get to their ready states
            t1 = GetGlobalData(Transport1,TRANSPORT_GLOBAL_Status);
            t2 = GetGlobalData(Transport2,TRANSPORT_GLOBAL_Status);
            if((t1 == TRANSPORT_STATUS_AtReady) &&
               (t2 == TRANSPORT_STATUS_AtReady))
               {
               StopControl("<Assembly> All objects made.\n");
               }
            break;
        default:
            StopControl("<Assembly> Unknown command\n");
        }
    return(done);
    }
