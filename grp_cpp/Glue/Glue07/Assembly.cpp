/* Assembly Task: CAssembly
File: Assembly.cpp

Created July 15, 1997, DM Auslander

This task handles the sequencing of the basic assembly process. It
places a base, applies glue, then places the cyclinder. At that point,
it finds a transport mechanism that is open and sends the part through
the rest of the process
*/

#include <math.h>
#include <tasks.hpp>

// States:
#define Initialize 0
#define Start 1
#define TestTran1 6
#define TestTran2 7
#define WaitTran 8

// Leave these states for the next version. They will not be used here
#define PlaceBase 2
#define ApplyGlue 3
#define PlaceCylinder 4
#define Wait1 5

static int InitialState = Start;

CAssembly::CAssembly(char *aName,int ProcNo)
        : BaseTask(aName,ProcNo)
    {
    InitialState = Start;
    CreateStateNameArray(20);  // So the audit trail will have
        // state names
    RegisterStateName("Initialize",Initialize);
    RegisterStateName("Start",Start);
    RegisterStateName("PlaceBase",PlaceBase);
    RegisterStateName("ApplyGlue",ApplyGlue);
    RegisterStateName("PlaceCylinder",PlaceCylinder);
    RegisterStateName("Wait1",Wait1);
    RegisterStateName("TestTran1",TestTran1);
    RegisterStateName("TestTran2",TestTran2);
    RegisterStateName("WaitTran",WaitTran);
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
               NextState = TestTran1;
               }
            break;
        case TestTran1:  // Test transport on belt1
            // Entry
            if(RunEntry)
               {
               // Send transport1 through its paces
               SendRTMessage(Transport1,TRANSPORT_MSG_GetObject,
                  ASSEMBLY_MSG_ObjectClear);
               }
            //Action
            // Test/Exit
            // Wait for belt to move out of assembly area
            if(GetRTMessage(ASSEMBLY_MSG_ObjectClear,&MsgIndex))
                {
                if(MsgIndex == 1)
                  NextState = TestTran2;
                else StopControl("<Assembly> Error placing base\n");
                }
            break;
        case TestTran2:
            // Entry
            if(RunEntry)
               {
               // Send transport2 through its paces
               SendRTMessage(Transport2,TRANSPORT_MSG_GetObject,
                  ASSEMBLY_MSG_ObjectClear);
               }
            //Action
            // Test/Exit
            // Wait for belt to move out of assembly area
            if(GetRTMessage(ASSEMBLY_MSG_ObjectClear,&MsgIndex))
                {
                if(MsgIndex == 1)
                  NextState = WaitTran;
                else StopControl("<Assembly> Error placing base\n");
                }
            break;
        case WaitTran:  // Wait for Transport2 to return to its ready pos.
            // Entry
            // Action
            // Test/Exit
            t2 = GetGlobalData(Transport2,TRANSPORT_GLOBAL_Status);
            if(t2 == TRANSPORT_STATUS_AtReady)
               {
               StopControl("Test Complete");
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
                  NextState = Wait1;  // Wait for robot to finish
                else StopControl("<Assembly> Error placing base\n");
                }
            break;
        case Wait1:
            // Wait indefinitely (Quit is in Transport)
            // Entry
            // Action
            // Test Exit
            break;
        default:
            StopControl("<Assembly> Unknown command\n");
        }
    return(done);
    }
