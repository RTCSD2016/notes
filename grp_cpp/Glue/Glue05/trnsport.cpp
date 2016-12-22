/* Transport tasks, classes CTransport1 and CTransport2
File: trnsport.cpp

Created June 12, 1997, DM Auslander

This task handles the transport of objects using the #1 and #2 sides of
the system. Two independent classes are used to define these to
show how such a system would be built if the two sides differed
significantly. In this case, the code will be nearly the same for
both.

This version is just for testing the model of the clamps (#5)
*/

#include <math.h>
#include <tasks.hpp>

// States:
#define Initialize 0
#define Start 1
#define Close1 2
#define Quit 3
#define Wait1 4
#define Open1 5
#define Wait2 6
#define Close2 7

static int InitialState = Start;

CTransport1::CTransport1(char *name,int ProcNo)
        : BaseTask(name,ProcNo)
    {
    TWait1 = 2.5; // Wait time, seconds
    TWait2 = 1.6;
    CreateStateNameArray(20);  // So the audit trail will have
        // state names
    RegisterStateName("Initialize",Initialize);
    RegisterStateName("Start",Start);
    RegisterStateName("Close1",Close1);
    RegisterStateName("Quit",Quit);
    RegisterStateName("Wait1",Wait1);
    RegisterStateName("Open1",Open1);
    RegisterStateName("Wait2",Wait2);
    RegisterStateName("Close2",Close2);
    }

int CTransport1::Run(void)
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
            NextState = Close1;
            break;
        case Close1:
            // Entry
            if(RunEntry)
                {
                SendRTMessage(Clamp1,CLAMP_MSG_Close);  // Command to clamp
                }
            // Action
            // Test/Exit
            // Check for completion of the clamp move
            if(GetGlobalData(Clamp1,CLAMP_STATUS) == 0)
                {
                // Clamp is closed
                NextState = Wait1;
                }
            break;
        case Wait1:
            // Wait for specified time
            // Entry
            if(RunEntry)
                {
                Time0 = GetTimeNow();
                }
            // Action
            // Test Exit
            if(GetTimeNow() >= (Time0 + TWait1))NextState = Open1;
            break;
        case Open1:
            // Entry
            if(RunEntry)
                {
                SendRTMessage(Clamp1,CLAMP_MSG_Open);  // Command to clamp
                }
            // Action
            // Test/Exit
            // Check for completion of the clamp move
            if(GetGlobalData(Clamp1,CLAMP_STATUS) == 1)
                {
                // Clamp is open
                NextState = Wait2;
                }
            break;
        case Wait2:
            // Wait for specified time
            // Entry
            if(RunEntry)
                {
                Time0 = GetTimeNow();
                }
            // Action
            // Test Exit
            if(GetTimeNow() >= (Time0 + TWait2))NextState = Quit;
            break;
        case Quit:
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
    TWait1 = 1.5; // Wait time, seconds
    TWait2 = 0.8;
    CreateStateNameArray(20);  // So the audit trail will have
        // state names
    RegisterStateName("Initialize",Initialize);
    RegisterStateName("Start",Start);
    RegisterStateName("Close1",Close1);
    RegisterStateName("Quit",Quit);
    RegisterStateName("Wait1",Wait1);
    RegisterStateName("Open1",Open1);
    RegisterStateName("Wait2",Wait2);
    RegisterStateName("Close2",Close2);
    }

int CTransport2::Run(void)
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
            NextState = Close1;
            break;
        case Close1:
            // Entry
            if(RunEntry)
                {
                SendRTMessage(Clamp2,CLAMP_MSG_Close);  // Command to clamp
                }
            // Action
            // Test/Exit
            // Check for completion of the clamp move
            if(GetGlobalData(Clamp2,CLAMP_STATUS) == 0)
                {
                // Clamp is closed
                NextState = Wait1;
                }
            break;
        case Wait1:
            // Wait for specified time
            // Entry
            if(RunEntry)
                {
                Time0 = GetTimeNow();
                }
            // Action
            // Test Exit
            if(GetTimeNow() >= (Time0 + TWait1))NextState = Open1;
            break;
        case Open1:
            // Entry
            if(RunEntry)
                {
                SendRTMessage(Clamp2,CLAMP_MSG_Open);  // Command to clamp
                }
            // Action
            // Test/Exit
            // Check for completion of the clamp move
            if(GetGlobalData(Clamp2,CLAMP_STATUS) == 1)
                {
                // Clamp is open
                NextState = Wait2;
                }
            break;
        case Wait2:
            // Wait for specified time
            // Entry
            if(RunEntry)
                {
                Time0 = GetTimeNow();
                }
            // Action
            // Test Exit
            if(GetTimeNow() >= (Time0 + TWait2))NextState = Quit;
            break;
        case Quit:
            // Turn control off and exit
            //StopControl("Exit from state QUIT of Transport2\n");
            // Wait in this state; Clamp1 will Quit
            break;
        default:
            StopControl("<Transport2> Illegal state");
        }
    return(done);
    }

