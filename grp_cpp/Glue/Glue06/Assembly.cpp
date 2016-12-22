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
    }

int CAssembly::Run(void)
    {
    int MsgIndex;
    double MsgValue;
    int done = 1;  // Default is don't ask for more scans
    NumScans++; // Increment scan count

    if (State == Initialize)
        {
        State = InitialState;
        NextState = InitialState;
        return (0);
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
            // Doesn't do anything in this version
            // Entry
            // Action
            // Test/Exit
            NextState = PlaceBase;
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
