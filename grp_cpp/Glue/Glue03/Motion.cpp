// Motion profiling, implements class CMotion
// File: motion.cpp

// Created June 4, 1997, DM Auslander
// Has IDLE and STOP states, but no command structure is
// built yet, so they can't be reached!

#include <math.h>
#include <tasks.hpp>

// Define states:
#define INITIALIZE 0
#define IDLE 1
#define HOLD 2
#define DECEL 3
#define ACCEL 4
#define STOP 5

CMotion::CMotion(char *name,BaseTask *aProfileOut,
    int aProfileOutBoxNo,double dt,int ProcNo)
    : BaseTask (name,ProcNo)
    {
    ProfileOut = aProfileOut;  // Task to send result to
    ProfileOutBoxNo = aProfileOutBoxNo;
    DeltaTaskTime = dt; // Task period
    NextTaskTime = 0.0;
    State = 0;         // set to initialize
    NextState = 0;
    CreateGlobal(5);  // Create global data bases
    ActivateProfiling(5.e-6,80.e-6,15,LOG_PROFILE);
    // Code for debugging only -- remove it for the next stage!
    InitialState = ACCEL;
    xcur = vcur = 0.0;
    xtarget = 0.6;
    vcruise = 0.05;
    accel = 0.015;
    // End of debugging code
    }

int CMotion::Run(void)
    {
    double Time,xstop,xnext,xold;
    int done = 1;   // Flag, default will suspend task
    int MsgIndex;
    int AtTarget = 0;
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
            // This state will be implemented at the next
            //  stage when a command structure is added
            // Entry
            // No action
            // Test/Exit:
            break;
        case HOLD:
            if(RunEntry)
                {
                // Send target position
                SendRTMessage(ProfileOut,ProfileOutBoxNo,0,xtarget,
                    MESSAGE_OVERWRITE);
                }
            // No action
            // Test/Exit
            // This task is terminal for teh debugging version
            break;
        case ACCEL:
                {
                // Entry
                if(RunEntry)
                    {
                    // Compute proper acceleration
                    xstop = StopPoint(xcur,vcur,accel);
                    if(xstop < xtarget)acur = accel;
                    else acur = -accel;
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
                vcur += acur * DeltaTaskTime;
                // vcur += aadj * DeltaTaskTime;
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
                    || Straddle(xold,xtarget,xnext))  // Position at target
                    {
                    // Profile is complete -- send message
                    NextState = HOLD;
                    done = 0;  // If this is an intermittent task, run
                        // HOLD immediately
                    }
                break;
                }
        case STOP:
                // This state will need a command structure for
                //  it to be reached
                // Entry
                if(RunEntry)
                    {
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
