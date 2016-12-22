// Motion profiling, implements class CMotion
// File: motion.cpp

// Created June 4, 1997, DM Auslander
// Modified June 11, 1997, DM Auslander to change command structure
//  to include information on where to send a "completion" message.

#include <math.h>
#include <tasks.hpp>

// Define states:
#define INITIALIZE 0
#define IDLE 1
#define HOLD 2
#define DECEL 3
#define ACCEL 4
#define STOP 5

// Command mail boxes:
// Command format:
// The arrival of the message is the command
// The integer value is the box number to which the results of the
//   command should be sent (-1 for none).
// The double value is the parameter, if any
// The completion message is: 1 for success; -n for failure where n
//   is an error number
/*
#define MOTION_CMD_HOLD_POSITION 0
#define MOTION_CMD_START_PROFILE 1
#define MOTION_CMD_STOP 2
#define MOTION_CMD_IDLE 3
#define MOTION_CMD_NEW_PAR 4
#define MOTION_MSG_ACCEL 5
#define MOTION_MSG_VCRUISE 6
*/

// Motion error codes:
#define MOTION_SUCCESS 1
#define MOTION_ERROR -1
#define PROFILE_STOPPED -2

CMotion::CMotion(char *name,BaseTask *aProfileOut,
    int aProfileOutBoxNo,int aStartBox,int aStopBox,
    BaseTask *a_taskCurPos,int a_indexCurPos,
    double a_accel,double a_vcruise,double dt,int ProcNo)
    : BaseTask (name,ProcNo)
    {
    CreateStateNameArray(20);  // So states can be listed by name
            // in audit trail
    RegisterStateName("Initialize",INITIALIZE);
    RegisterStateName("IDLE",IDLE);
    RegisterStateName("HOLD",HOLD);
    RegisterStateName("DECEL",DECEL);
    RegisterStateName("ACCEL",ACCEL);
    RegisterStateName("STOP",STOP);
    ProfileOut = aProfileOut;  // Task to send result to
    ProfileOutBoxNo = aProfileOutBoxNo;
    StartBox = aStartBox;  // Message box for controller start cmd
    StopBox = aStopBox;  // for stop cmd
    taskCurPos = a_taskCurPos;  // Task from which current position
    indexCurPos = a_indexCurPos;  // is obtained
    vcruise = a_vcruise;
    accel = a_accel;
    DeltaTaskTime = dt; // Task period
    NextTaskTime = 0.0;
    State = 0;         // set to initialize
    NextState = 0;
    respIdle = respHold = respStart = respStop = -1;
    CreateGlobal(5);  // Create global data bases
    //ActivateProfiling(5.e-6,80.e-6,15,LOG_PROFILE);
    InitialState = IDLE;
    xcur = vcur = 0.0;
    }

void CMotion::SendCompletionMessage(int toTask,int &respBox,int code)
    {
    if(respBox >= 0)
        {
        SendRTMessage(toTask,respBox,code,0.0,MESSAGE_OVERWRITE);
        respBox = -1;  // Reset so accidental messages won't be sent
        }
    }

int CMotion::Run(void)
    {
    double Time,xstop,xnext,xold;
    int done = 1;   // Flag, default will suspend task
    int MsgIndex,FromTask;
    int AtTarget = 0,ReverseAccel = 0;
    double MsgVal;

    NumScans++; // Increment scan count

    if (State == INITIALIZE)
        {
        State = InitialState;
        NextState = InitialState;
        return (0);
        }

    Time = GetTimeNow();
    if(Time < NextTaskTime)return(0);  // Not time yet
    NextTaskTime += DeltaTaskTime;

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
        case IDLE:
            // Entry
            if(RunEntry)
                {
                // Turn the controller off
                if(ProfileOut != NULL)
                    {
                    SendRTMessage(ProfileOut,StopBox);
                    }
                SendCompletionMessage(fromIdle,respIdle,1);
                  // Idle completes immediately, send SUCCESS (1)
                }
            // No action
            // Test/Exit:
            // Check for commands
            if(GetRTMessage(MOTION_CMD_HOLD_POSITION,&MsgIndex,
                        &MsgVal,&FromTask))
                {
                xtarget = MsgVal;
                fromHold = FromTask;
                respHold = MsgIndex;
                NextState = HOLD;
                }
            else if(GetRTMessage(MOTION_CMD_START_PROFILE,&MsgIndex,
                        &MsgVal,&FromTask))
                {
                xtarget = MsgVal;  // Start a new point-to-point move
                fromStart = FromTask;
                respStart = MsgIndex;
                NextState = ACCEL;
                // Clear any old commands
                GetRTMessage(MOTION_CMD_HOLD_POSITION,&MsgIndex,&MsgVal);
                GetRTMessage(MOTION_CMD_STOP,&MsgIndex,&MsgVal);
                GetRTMessage(MOTION_CMD_IDLE,&MsgIndex,&MsgVal);
                }
            break;
        case HOLD:
            if(RunEntry)
                {
                // Send target position
                if(ProfileOut != NULL)
                    {
                    SendRTMessage(ProfileOut,ProfileOutBoxNo,0,xtarget,
                        MESSAGE_OVERWRITE);
                    // Turn the controller on
                    SendRTMessage(ProfileOut,StartBox);
                    }
                SendCompletionMessage(fromHold,respHold,1);
                  // Hold completes immediately, send SUCCESS (1)
                }
            // No action
            // Test/Exit
            if(GetRTMessage(MOTION_CMD_HOLD_POSITION,&MsgIndex,
                        &MsgVal,&FromTask))
                {
                // Self transition; keep holding, new target
                xtarget = MsgVal;
                fromHold = FromTask;
                respHold = MsgIndex;
                NextState = HOLD;
                }
            else if(GetRTMessage(MOTION_CMD_START_PROFILE,&MsgIndex,
                        &MsgVal,&FromTask))
                {
                xtarget = MsgVal;  // Start a new point-to-point move
                fromStart = FromTask;
                respStart = MsgIndex;
                NextState = ACCEL;
                // Clear any old commands
                GetRTMessage(MOTION_CMD_HOLD_POSITION,&MsgIndex,&MsgVal);
                GetRTMessage(MOTION_CMD_STOP,&MsgIndex,&MsgVal);
                GetRTMessage(MOTION_CMD_IDLE,&MsgIndex,&MsgVal);
                }
           else if(GetRTMessage(MOTION_CMD_IDLE,&MsgIndex,
                        &MsgVal,&FromTask))
                {
                fromIdle = FromTask;
                respIdle = MsgIndex;
                NextState = IDLE;
                }
            break;
        case ACCEL:
                {
                // Entry
                if(RunEntry)
                    {
                    // Get current initial position
                    if(taskCurPos != NULL)
                        {
                        // If no task is specified use value left from
                        // previous profile
                        xcur = GetGlobalData(taskCurPos,indexCurPos);
                        }
                    vcur = 0.0;  // By default

                    // Check for new accel - vcruise values
                    if(GetRTMessage(MOTION_CMD_NEW_PAR))
                        {
                        // New parameters have been received
                        if(GetRTMessage(MOTION_MSG_ACCEL,&MsgIndex,
                            &MsgVal))accel = MsgVal;
                        if(GetRTMessage(MOTION_MSG_VCRUISE,&MsgIndex,
                            &MsgVal))vcruise = MsgVal;
                        }

                    // Compute proper acceleration
                    xstop = StopPoint(xcur,vcur,accel);
                    if(xstop < xtarget)acur = accel;
                    else acur = -accel;

                    if(ProfileOut != NULL)
                        {
                        // Send initial position to target task
                        SendRTMessage(ProfileOut,ProfileOutBoxNo,
                            0,xcur,MESSAGE_OVERWRITE);
                        // Turn the controller on
                        SendRTMessage(ProfileOut,StartBox);
                        }
                    }
                // Action
                vcur += acur * DeltaTaskTime;
                if(vcur > vcruise)vcur = vcruise;
                if(vcur < (-vcruise))vcur = - vcruise;
                xnext = xcur + vcur * DeltaTaskTime;
                xstop = StopPoint(xnext,vcur,accel);

                // Write result
                xcur = xnext;
                PrivateGlobalData[MOTION_xcur] = xcur;
                if(ProfileOut != NULL)
                    {
                    // Send result to target task
                    SendRTMessage(ProfileOut,ProfileOutBoxNo,
                        0,xcur,MESSAGE_OVERWRITE);
                    }
                // Test/exit
                // Check to see if acceleration should be reversed
                if(acur > 0)
                    {
                    // Only check if velocity and accel are same sign
                    if(xstop >= xtarget)
                        NextState = DECEL;
                    }
                else
                    {
                    if(xstop <= xtarget)
                        NextState = DECEL;
                    }

                // Check for a new command
                if(GetRTMessage(MOTION_CMD_STOP,&MsgIndex,
                        &MsgVal,&FromTask))
                    {
                    // Send completion message since this profile is now stopped
                    SendCompletionMessage(fromStart,respStart,PROFILE_STOPPED);
                    fromStop = FromTask;
                    respStop = MsgIndex;
                    NextState = STOP;
                    }
            break;
            }
        case DECEL:
                {
                // Entry
                if(RunEntry)
                    {
                    // Compute proper acceleration
                    acur = -Sign(vcur)* accel;
                    }
                // Action
                // Adjust acceleration to hit target point as
                //  closely as possible
                double aadj = -(vcur * vcur) / (2.0 * (xtarget - xcur));
                vcur += aadj * DeltaTaskTime;
                if(vcur > vcruise)vcur = vcruise;
                if(vcur < (-vcruise))vcur = - vcruise;
                xnext = xcur + vcur * DeltaTaskTime;
                xstop = StopPoint(xnext,vcur,accel);

                // Write result
                xold = xcur;
                xcur = xnext;
                PrivateGlobalData[MOTION_xcur] = xcur;
                if(ProfileOut != NULL)
                    {
                    // Send result to target task
                    SendRTMessage(ProfileOut,ProfileOutBoxNo,
                        0,xcur,MESSAGE_OVERWRITE);
                    }
                // Test/Exit
                // Check for reaching the target point
                if(((acur * vcur) >= 0)  // Velocity has changed sign
                    || Straddle(xold,xtarget,xnext)  // Position at target
                    || ((fabs(aadj) / accel) < 0.001))
                    {
                    // Profile is complete -- send message
                    SendCompletionMessage(fromStart,respStart,MOTION_SUCCESS);
                    NextState = HOLD;
                    respHold = -1;  // HOLD is being entered from profile
                        // not by command, so don't send a completion message
                    done = 0;  // If this is an intermittent task, run
                        // HOLD immediately
                    }
                break;
                }
        case STOP:
                // Entry
                if(RunEntry)
                    {
                    fromStart = fromStop;  // Completion message must come
                            // from the profile since this state is only
                            // transient
                    respStart = respStop;
                    xtarget = StopPoint(xcur,vcur,accel);
                        // Stop as soon as possible
                    }
                // No action
                // Test/Exit
                NextState = DECEL;  // Unconditional transition
                acur = -Sign(vcur) * accel;
            break;
        default:
            StopControl("<Motion> Illegal state");
        }
    PrivateGlobalData[MOTION_STATE] = State;
    CopyGlobal();  // Make global data visible
    return(done);
    }

double CMotion::StopPoint(double xc,double vc,double ac)
    {
    // Compute the point at which the velocity would be brought
    // to zero given the current velocity and position
    double acc = -Sign(vc) * ac;
    double xstop = xc - 0.5 * vc * vc / acc;
    return(xstop);
    }

double CMotion::Sign(double x)
    {
    if(x >= 0)return(1.0);
    else return(-1.0);
    }

int CMotion::Straddle(double x0,double x1,double x2)
    {
    // Check to see if points x0 and x2 straddle x1
    // Return 1 if so
    if(x2 > x0)
        {
        if((x1 >= x0) && (x1 <= x2))return(1);
        else return(0);
        }
    else
        {
        if((x1 >= x2) && (x1 <= x0))return(1);
        else return(0);
        }
    }
