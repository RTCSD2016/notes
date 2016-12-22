/* Transport tasks, classes CTransport1 and CTransport2
File: trnsport.cpp

Created June 12, 1997, DM Auslander

This task handles the transport of objects using the #1 and #2 sides of
the system. Two independent classes are used to define these to
show how such a system would be built if the two sides differed
significantly. In this case, the code will be nearly the same for
both.
*/

#include <math.h>
#include <tasks.hpp>

// States:
#define INITIALIZE 0
#define START 1
#define MOVE1 2
#define QUIT 3
#define WAIT1 4
#define MOVE2 5
#define WAIT2 6
static int InitialState = START;

CTransport1::CTransport1(char *name,int ProcNo)
        : BaseTask(name,ProcNo)
    {
    TWait1 = 2.5; // Wait time, seconds
    AccSlow = 0.006;
    AccFast = 0.015;
    VcSlow = 0.06;
    VcFast = 0.1;
    CreateStateNameArray(20);  // So the audit trail will have
        // state names
    RegisterStateName("Initialize",0);
    RegisterStateName("Start",1);
    RegisterStateName("Move1",2);
    RegisterStateName("Quit",3);
    RegisterStateName("Wait1",4);
    RegisterStateName("Move2",5);
    RegisterStateName("Wait2",6);
    }

int CTransport1::Run(void)
    {
    int MsgIndex;
    double MsgValue;
    int done = 1;  // Default is don't ask for more scans
    NumScans++; // Increment scan count

    if (State == INITIALIZE)
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
        case START:
            // Doesn't do anything in this version
            // Entry
            // Action
            // Test/Exit
            NextState = MOVE1;
            break;
        case MOVE1:
            // Entry
            if(RunEntry)
                {
                // Set acceleration and cruise velocity
                SendRTMessage(BeltMotion1,MOTION_MSG_ACCEL,0,AccSlow,
                    MESSAGE_OVERWRITE);
                SendRTMessage(BeltMotion1,MOTION_MSG_VCRUISE,0,
                    VcSlow,MESSAGE_OVERWRITE);
                SendRTMessage(BeltMotion1,MOTION_CMD_NEW_PAR);
                // Send a move command to belt #1 Motion task
                SendRTMessage(BeltMotion1,MOTION_CMD_START_PROFILE,
                    TRANSPORT1_MOVE_DONE,1.0);
                }
            // Action
            // Test/Exit
            // Check for completion of the move
            if(GetRTMessage(TRANSPORT1_MOVE_DONE,&MsgIndex,&MsgValue))
                {
                // Move is finished, wait for axis to settle
                NextState = WAIT1;
                }
            break;
        case WAIT1:
            // Wait for a few seconds to allow the motion to settle
            // Entry
            if(RunEntry)
                {
                Time0 = GetTimeNow();
                }
            // Action
            // Test Exit
            if(GetTimeNow() >= (Time0 + TWait1))NextState = MOVE2;
            break;
        case MOVE2:
            // Entry
            if(RunEntry)
                {
                // Set acceleration and cruise velocity
                SendRTMessage(BeltMotion1,MOTION_MSG_ACCEL,0,AccFast,
                    MESSAGE_OVERWRITE);
                SendRTMessage(BeltMotion1,MOTION_MSG_VCRUISE,0,
                    VcFast,MESSAGE_OVERWRITE);
                SendRTMessage(BeltMotion1,MOTION_CMD_NEW_PAR);
                // Send a move command to belt #1 Motion task
                SendRTMessage(BeltMotion1,MOTION_CMD_START_PROFILE,
                    TRANSPORT1_MOVE_DONE,0.1);
                }
            // Action
            // Test/Exit
            // Check for completion of the move
            if(GetRTMessage(TRANSPORT1_MOVE_DONE,&MsgIndex,&MsgValue))
                {
                // Move is finished, wait for axis to settle
                NextState = WAIT2;
                }
            break;
        case WAIT2:
            // Wait for a few seconds to allow the motion to settle
            // Entry
            if(RunEntry)
                {
                Time0 = GetTimeNow();
                }
            // Action
            // Test Exit
            if(GetTimeNow() >= (Time0 + TWait1))NextState = QUIT;
            break;
        case QUIT:
            // Turn control off and exit
            StopControl("Exit from state QUIT of Transport1\n");
            break;
        default:
            StopControl("<Transport1> Illegal state");
        }
    return(done);
    }

CTransport2::CTransport2(char *name,int ProcNo)
        : BaseTask(name,ProcNo)
    {
    TWait1 = 2.5; // Wait time, seconds
    AccSlow = 0.006;
    AccFast = 0.015;
    VcSlow = 0.06;
    VcFast = 0.1;
    CreateStateNameArray(20);  // So the audit trail will have
        // state names
    RegisterStateName("Initialize",0);
    RegisterStateName("Start",1);
    RegisterStateName("Move1",2);
    RegisterStateName("Quit",3);
    RegisterStateName("Wait1",4);
    RegisterStateName("Move2",5);
    RegisterStateName("Wait2",6);
    }

int CTransport2::Run(void)
    {
    int MsgIndex;
    double MsgValue;
    int done = 1;  // Default is don't ask for more scans
    NumScans++; // Increment scan count

    if (State == INITIALIZE)
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
        case START:
            // Doesn't do anything in this version
            // Entry
            // Action
            // Test/Exit
            NextState = MOVE1;
            break;
        case MOVE1:
            // Entry
            if(RunEntry)
                {
                // Set acceleration and cruise velocity
                SendRTMessage(BeltMotion2,MOTION_MSG_ACCEL,0,AccSlow,
                    MESSAGE_OVERWRITE);
                SendRTMessage(BeltMotion2,MOTION_MSG_VCRUISE,0,
                    VcSlow,MESSAGE_OVERWRITE);
                SendRTMessage(BeltMotion2,MOTION_CMD_NEW_PAR);
                // Send a move command to belt #2 Motion task
                SendRTMessage(BeltMotion2,MOTION_CMD_START_PROFILE,
                    TRANSPORT2_MOVE_DONE,0.1);
                }
            // Action
            // Test/Exit
            // Check for completion of the move
            if(GetRTMessage(TRANSPORT2_MOVE_DONE,&MsgIndex,&MsgValue))
                {
                // Move is finished, wait for axis to settle
                NextState = WAIT1;
                }
            break;
        case WAIT1:
            // Wait for a few seconds to allow the motion to settle
            // Entry
            if(RunEntry)
                {
                Time0 = GetTimeNow();
                }
            // Action
            // Test Exit
            if(GetTimeNow() >= (Time0 + TWait1))NextState = MOVE2;
            break;
        case MOVE2:
            // Entry
            if(RunEntry)
                {
                // Set acceleration and cruise velocity
                SendRTMessage(BeltMotion2,MOTION_MSG_ACCEL,0,AccFast,
                    MESSAGE_OVERWRITE);
                SendRTMessage(BeltMotion2,MOTION_MSG_VCRUISE,0,
                    VcFast,MESSAGE_OVERWRITE);
                SendRTMessage(BeltMotion2,MOTION_CMD_NEW_PAR);
                // Send a move command to belt #2 Motion task
                SendRTMessage(BeltMotion2,MOTION_CMD_START_PROFILE,
                    TRANSPORT2_MOVE_DONE,0.6);
                }
            // Action
            // Test/Exit
            // Check for completion of the move
            if(GetRTMessage(TRANSPORT2_MOVE_DONE,&MsgIndex,&MsgValue))
                {
                // Move is finished, wait for axis to settle
                NextState = WAIT2;
                }
            break;
        case WAIT2:
            // Wait for a few seconds to allow the motion to settle
            // Entry
            if(RunEntry)
                {
                Time0 = GetTimeNow();
                }
            // Action
            // Test Exit
            if(GetTimeNow() >= (Time0 + TWait1))NextState = QUIT;
            break;
        case QUIT:
            // Turn control off and exit
            // Do nothing here -- exit is from Transport1
            break;
        default:
            StopControl("<Transport2> Illegal state");
        }
    return(done);
    }
