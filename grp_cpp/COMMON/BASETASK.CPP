// Base class for task template.
// File:  basetask.cpp
// Created 8/2/95 by Jason Jones
// Modifications:
//      8/23/95 - JJ added TList.Name and cout in
//                                       TList.Append.
//     10/6/95, DM Auslander, added task ID numbers
//      9/11/96, DM Auslander, added task profiling
//     10/1/96, DM Auslander, added message passing
//     10/7/96, DM Auslander, added global data base
//      6/16/97, DM Auslander, added state names for audit trail

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <dos.h>
#include <stdarg.h>
#include "tasks.hpp"


#define NTASKSMAX 50    // For use in ScanStat section
// These arrays are filled in InitScanStat()
static int ntasks=0;
// These arrays use TaskID as the index
static char *tskname[NTASKSMAX];        // This isn't really needed
                                // but it's convenient!
BaseTask *AllTasks[NTASKSMAX];

// This array uses TaskCode as its index
BaseTask *TasksByCode[2 * NTASKSMAX];  // Extra space because TaskCode
        // can be assigned arbitrarily

static task_id = -1;    // Use this to get a running index of tasks

#ifndef PROCESS_INFO  // Default process information for code that
   // does not have process information in tasks.hpp
const int ThisProcess = 0; // Process number
const int NumberOfProcesses = 1;  // Number of independent processes
#endif

void BaseTask::BaseTaskInit(char *aName,int ProcNo)
    {
    Name = new char[strlen(aName)+1];
    strcpy(Name, aName);
    task_id++;      // Index the id #
    TaskID = task_id;
    TaskCode = TaskID;  // Always the same as TaskID
    NumScans = 0;
    State = xState = 0;
    NextState = 0;
    RunEntry = 0;
    InitScanStat(this);
    ProcessNo = ProcNo;
    if(TaskCode >= (NTASKSMAX))
        {
      cout << "<BaseTaskInit> Task code too big (>2*NTASKSMAX)\n";
      exit(1);
      }
    TasksByCode[TaskCode] = this;  // Pointer to this task
    GlobalSize = 0;  // Default is no global data
    StateNames = NULL;  // No state names defined yet
    };

BaseTask::BaseTask(char *aName,int ProcNo)  // Constructor
    {
    profile_on = 0;  // Default is no profiling
    BaseTaskInit(aName,ProcNo);
    }

// The destructor is overloaded in the derived task
BaseTask::~BaseTask(void)
   {
   delete [] Name;
   if(profile_on)
       {
       // Free space used by profiler
       delete [] values;
       delete [] occur;
       }
   if(GlobalSize > 0)
      {
      // Free space used for globals
      delete [] PrivateGlobalData;
      delete [] GlobalData;
      }
   if(StateNames != NULL)delete [] StateNames;
   }

static char *DefName = "aState";

// Functions to name states
void BaseTask::CreateStateNameArray(int n)
    {
    StateNames = new char * [n];
    nStateNames = n;
    for(int i = 0; i < n; i++)StateNames[i] = DefName;
    }

void BaseTask::RegisterStateName(char *pName,int i)
    {
    if(i >= nStateNames)
        {
        cout << "RegisterStateName> Index too large\n";
        exit(1);
        }
    StateNames[i] = pName;
    }

char **BaseTask::GetStateNames(void)
    {
    return(StateNames);
    }

// The default Run() function should never actually
// be used.  It should be overloaded in the actual
// task definition.
int BaseTask::Run(void)
        {
        StopControl("Run function not overloaded\n");
        return (1);
        }

long BaseTask::GetNumScan(void)
        {
        return(NumScans);
        }

// Functions associated with task profiling
void BaseTask::ActivateProfiling(double vbegin,double vend,int num_bins,int loglin)
    {
    if(num_bins <= 0)
        {
        // No bins -> No profiling
        profile_on = 0;
        return;
        }
    else
        {
        nbin = num_bins;
        v1 = vbegin;
        vlast = vend;
        nbin = num_bins;
        values = new double [nbin]; // Set up the values for the
            // frequency (histogram) boundaries
        occur = new long [nbin];  // One extra bin for >vlast

        int i;

        if(loglin)
            // 0 -> log; 1 -> linear intervals
            {
            // Linear intervals
            double add_amnt = (vlast - v1) / (nbin -2);
            for(values[0]= v1,i = 1;i < nbin-1; i++)
                {
                values[i] = values[i - 1] + add_amnt;
                }
            }
        else
            {
            // Log intervals
            double mult = exp(log(vlast / v1) / (nbin - 2));
            // Set up the boundaries for the time histogram
            for(values[0]= v1,i = 1;i < nbin-1; i++)
                {
                values[i] = values[i - 1] * mult;
                }
            }
        values[nbin-1] = 1.e20; // Big number
        for(i = 0; i < nbin; i++)occur[i] = 0;
        profile_on = 1;
        }
    }

void BaseTask::StartProfile(void)
    {
   if(!profile_on)return;
   Time0 = GetTimeNow();
   return;
   }

void BaseTask::EndProfile(void)
    {
   int j;
   double delt;

   if(!profile_on)return;
   delt = GetTimeNow() - Time0;

   for(j = 0; j < nbin; j++)
    {
      if(delt <= values[j])
        {
         occur[j]++;
         break;
         }
        }
   return;
   }

void BaseTask::WriteProfile(ofstream &OutputFile)
    {
   int i;

   if(!profile_on)
        {
        // Nothing to print
        OutputFile << "No profile statistics for this task\n";
        return;
        }
   for(i = 0; i < nbin - 1; i++)
        {
        OutputFile << "<" << values[i] << "\t\t" << occur[i] << "\n";
        }
    OutputFile << ">" << values[i-1] << "\t\t" << occur[i] << "\n";
   }

// BaseTask functions for message passing
// Delivery modes:
//  0  Only deliver message if message box is empty
//  1  Deliver message regardless of message box state

void BaseTask::SendMsg(BaseTask *ToTask,int BoxNo,int Flag,
                double Val,int mode,int ReturnReceipt)
    {
   int ReturnValue;
   int to_process = ToTask->GetTaskProcess();
   int this_task_code = this->GetTaskCode();

   if(to_process == ThisProcess)
    {
      // Message is local -- deliver it immediately
      ReturnValue = ToTask->AcceptMessage(BoxNo,Flag,Val,mode,
        TaskCode,ThisProcess);
      if(ReturnReceipt != -1)
        {
         // Send a return receipt
         SendRTMessage(this,ReturnReceipt,ReturnValue);  // The value
            // returned by Accept is sent as the flag to the return
            // mailbox, which is specified by the value of ReturnReceipt
         }
      }
   else
      {
      #ifdef USE_UDP
      int RetVal;
      int to_task_code = ToTask->GetTaskCode();

      RetVal = InsertMessage(to_process,to_task_code,BoxNo,ThisProcess,
        this_task_code,Flag,Val,ReturnReceipt,mode);
      if(RetVal)
        {
        // Error inserting message
        StopControl("<SendMsg> Error inserting message\n");
        return;
        }
      #else
      StopControl("<SendMsg> MultiProcess control not defined\n");
      return;
      #endif
      }
   }

// Two versions of SendRTMessage so to-task can be specified either by
//    pointer or by code
void BaseTask::SendRTMessage(BaseTask *ToTask,int BoxNo,int Flag,
                double Val,int mode,int ReturnReceipt)
    {
   SendMsg(ToTask,BoxNo,Flag,Val,mode,ReturnReceipt);
   }

void BaseTask::SendRTMessage(int ToTask,int BoxNo,int Flag,
                double Val,int mode,int ReturnReceipt)
    {
   SendMsg(TasksByCode[ToTask],BoxNo,Flag,Val,mode,ReturnReceipt);
   }


int BaseTask::GetRTMessage(int BoxNo,int *msg_flag,
         double *val, int *from_task,int *from_process)
   {
   /* Returns:
     -1 for error
      0 for no message available
      1 for successful retrieval
   */
   if((BoxNo >= NUM_MESSAGE_BOX) || (BoxNo < 0))return(-1); // Error
   CriticalSectionBegin();
   if(mbox[BoxNo].MessageAvailable == 0)
      {
      CriticalSectionEnd();
      return(0);  // No message available
      }
   // Get information
   if(msg_flag != NULL)*msg_flag = mbox[BoxNo].MessageFlag;
   if(val != NULL)*val = mbox[BoxNo].MessageValue;
   if(from_task != NULL)*from_task = mbox[BoxNo].FromTask;
   if(from_process != NULL)*from_process = mbox[BoxNo].FromProcess;
   mbox[BoxNo].MessageAvailable = 0;  // Clear message box
   CriticalSectionEnd();
   return(1);  // Success
   }

int BaseTask::AcceptMessage(int BoxNo,int msg_flag,double msg_val,int mode,
    int from_task,int from_process)
   {
   // Processes an incoming message
   // Returns 1 if message successfully delivered, 0 otherwise
   if((BoxNo >= NUM_MESSAGE_BOX) || (BoxNo < 0))return(0);
   if((mode < 0) || (mode > 1))
      {
      StopControl("Unknown message mode\n");
      return(0);
      }
   CriticalSectionBegin();
   if(mode == 0)
      {
      // Check to see if message box is empty
      if(mbox[BoxNo].MessageAvailable != 0)
         {
         CriticalSectionEnd();
         return(0);  // Can't deliver
         }
      }

   // Copy message information
   mbox[BoxNo].FromTask = from_task;
   mbox[BoxNo].FromProcess = from_process;
   mbox[BoxNo].MessageFlag = msg_flag;
   mbox[BoxNo].MessageValue = msg_val;
   mbox[BoxNo].MessageAvailable = 1;
   CriticalSectionEnd();
   return(1); // Successful delivery
   }

void BaseTask::SendCompletionMessage(int toTask,int &respBox,int code)
    {
    if(respBox >= 0)
        {
        SendRTMessage(toTask,respBox,code,0.0,MESSAGE_OVERWRITE);
        respBox = -1;  // Reset so accidental messages won't be sent
        }
    }

#ifdef USE_UDP  // Only compile for network applications
void StoreMessage(int ToProc,int ToTask,int BoxNo,int FromProc,
     int FromTask,int MsgFlag,double MsgValue,int RetRecpt,int DelMode)
    {
    // Store an incoming message into the task's message box
    int RetVal;

    if(ToProc != ThisProcess)
        {
        StopControl("<StoreMessage> Wrong process #.\n");
        return;
        }
    BaseTask *pTask = TasksByCode[ToTask];
    RetVal = pTask->AcceptMessage(BoxNo,MsgFlag,MsgValue,DelMode,
        FromTask,FromProc);
    if(RetRecpt != -1)
      {
       // Send a return receipt
       pTask->SendRTMessage(FromTask,RetRecpt,RetVal);  // The value
          // returned by Accept is sent as the flag to the return
          // mailbox, which is specified by the value of ReturnReceipt
      }
    }
#endif  // USE_UDP

// Create the arrays needed for global storage
// Returns 1 for successful creation; 0 for error
int BaseTask::CreateGlobal(int nGlobal)
   {
   PrivateGlobalData = new double [nGlobal];
   GlobalData = new double [nGlobal];

   if((GlobalData == NULL) || (PrivateGlobalData == NULL))return(0);
   for(int i = 0; i < nGlobal; i++)
      {
      // Default values
      PrivateGlobalData[i] = GlobalData[i] = 0.0;
      }
   GlobalSize = nGlobal;
   return(1);  // Success
   }

// Copy global information from private to public view
// Returns 1 for successful; 0 for error
int BaseTask::CopyGlobal(void)
   {
   CriticalSectionBegin();
   for(int i = 0; i < GlobalSize; i++)
      {
      GlobalData[i] = PrivateGlobalData[i];
      }
   CriticalSectionEnd();
   return(1);
   }

int BaseTask::AcceptGlobal(double *pData,int nData)
    {
    // Accept global data from an external source
    if(nData != GlobalSize)
        {
        StopControl("<AcceptGlobal>Share array sizes don't match.\n");
        exit(1);
        }
    CriticalSectionBegin();
    for(int i = 0; i < GlobalSize; i++)
       {
       GlobalData[i] = pData[i];
       }
    CriticalSectionEnd();
    return(1);
    }

#ifdef USE_UDP
void StoreSharedData(int ToProc,int ToTask,int n,double *v)
    {
    // Interface from network functions to local task functions
    BaseTask *pTask = TasksByCode[ToTask];
    pTask->AcceptGlobal(v,n);
    }
#endif //  USE_UDP

double BaseTask::GetGlobalData(BaseTask *pb,int k)
    {
   CriticalSectionBegin();
   double v = pb->GlobalData[k];
   CriticalSectionEnd();
   return(v);
   }

double *BaseTask::GetGlobalArray(BaseTask *pb,int *n)
    {
    double *pv = pb->GlobalData;  // Address of global array
    *n = pb->GlobalSize;
    return(pv);
    }

#ifdef USE_UDP
int SendAll(void)
    {
    // Send out messages and shared data to other processes.
    // Messages are already copied into the outgoing packets
    // so nothing special has to be done with them.
    // Shared data must be copied from each task for transmission
    // to all other processes

    // Returns success -- 1 for OK, 0 for error

    BaseTask *pTask;
    int ProcNo,nData;
    #define BUFFER_SIZE 100
    double Buffer[BUFFER_SIZE];

    for(int i = 0; i < ntasks; i++)
        {
        // Scan all tasks
        pTask = AllTasks[i];
        ProcNo = pTask->GetTaskProcess();
        if(ProcNo != ThisProcess)continue; // Not in this process
                // therefore, don't send data
        double *pData = pTask->GetGlobalArray(pTask,&nData);
        if(nData <= 0)continue;  // No data for this task
        if(nData > BUFFER_SIZE)
            {
            StopControl("<SendAll> Global array too big.\n");
            exit(1);
            }
        CriticalSectionBegin();
        // Copy data to a local buffer
        for(int j = 0; j < nData; j++)Buffer[j] = pData[j];
        CriticalSectionEnd();

        for(int j = 0; j < NumberOfProcesses; j++)
            {
            // Send the data to all processes except this one
            if(j == ThisProcess)continue; // Don't send here
            if(InsertSharedArray(j,pTask->TaskCode,nData,Buffer))
                {
                StopControl("<SendAll> Error inserting shared data.\n");
                exit(1);
                }
            }  // Processes
        }  // Tasks
    if(SendData())  // Send out all available data
        {
        StopControl("<SendAll> Error sending data.\n");
        exit(1);
        }
    return(1);  // Normal return
    }
#else  // define a dummy
int SendAll(void){}
#endif  // USE_UDP

// Routines for handling task lists.
//  TListElement is an object to keep track of one
//    BaseTask on the list.  It includes a pointer to
//    the next item on the list.


TListElement::TListElement(BaseTask *ItemPointer)
        {
        Item = ItemPointer;
        Next = NULL;
        }

TListElement::~TListElement(void)
        {
        delete (Item);
        }

//  TList is an object to keep track of one list of
//    BaseTasks.  It includes a pointer to the firts,
//    last and current items on the list, as well as
//    methods for finding the next item on the list,
//    how many objects are on the list, and if the
//    list is empty.

TList::TList(char *aName)
        {
        First = Last = Current = NULL;
        Length = 0;
        Name = new char[strlen(aName) + 1];
        strcpy(Name, aName);
        }

TList::~TList(void)
        {
        TListElement *Element = First;
        TListElement *NextElement;

        delete [] Name;
        while (Element != NULL)
                {
                NextElement = Element->Next;
                delete (Element);
                Element = NextElement;
                }
        }

//-------------------------------------------------------
//  Method: Append
//              Add a task to the end of the list

void TList::Append(BaseTask *Item)
        {
        TListElement *Element = new TListElement(Item);

        if(IsEmpty())
                {
                First = Element;
                Last = Element;
                Current = Element;
                }
        else
                {
                Last->Next = Element;
                Last = Element;
                }
        Length ++;
        if (Item->Name != NULL)
                cout << "Added \"" << Item->Name << "\" as task: "
                  << Length << " to task list \"" << Name << "\"\n";
        else cout << "The task you ar trying to add as the "
                  << Length << "th item on the list has no name.\n";
        }

//-------------------------------------------------------
// Method: NextItem
//              Return a pointer to the next time on
//      the list and then set that item to the
//              current item.

BaseTask * TList::NextItem(void)
        {
        if (IsEmpty())
                return NULL;

        if (Current == Last)
                {
                Current = First;
                }
        else
                {
                Current = Current->Next;
                }
                return Current->Item;
        }

//-------------------------------------------------------
//  Method: IsFirst
//              True for empty list, false otherwise

int TList::IsEmpty(void)
        {
        if (First == NULL)
                return 1;
        else
                return 0;
        }

//-------------------------------------------------------
//  Method: HowLong
//              Returns how many items are on the list

int TList::HowLong(void)
        {
        return Length;
        }

// TaskDispatcher
// List1 will be run as a sequential (low priority) list
// List2 will be run as a minimum latency list
// Sequential tasks each get 1 scan per call to TaskDispatcher
// Minimum latency tasks each get as many scans as they need per call
// to TaskDispatcher
// Either list or both can be empty

int TaskDispatcher (TList *List1, TList *List2)
        {
        BaseTask *TempTask;
        int Suspend;
        int List1Num,List2Num,jmax;
        // Check to see which lists have entries to be run
        if (List2 == NULL)
                List2Num = 0;
        else
                List2Num = List2->HowLong();
        if (List1 == NULL)
                List1Num = 0;
        else
                List1Num = List1->HowLong();

        // If both lists are empty, there is nothing to do!
        if(!List1Num && !List2Num)
                return (0);
        // If both lists have entries, run them with List2 having
        // higher priority.

        if(List1Num == 0)jmax = 1;      // Run outer loop once even if List1 is empty
        else jmax = List1Num;

        for (int j=0; j < jmax; j++)
                {
                for (int i=0; i < List2Num; i++)
                        {
                        if ((TempTask = List2->NextItem()) == NULL)
                                {// Get the next Task on the list
                                cout << "\nTried to access empty task list\n";
                                return(-1);
                                }
                         Suspend = 0;
                         TempTask->StartProfile();
                         while (Suspend != 1)
                                  {
                                  Suspend = TempTask->Run();
                                  IncrementTime();
                                  if (Suspend == -1)
                                          {
                                          cout << "\nProblem running Task: " <<
                                                           TempTask->Name << "\n";
                                          TempTask->EndProfile();
                                          return(-1);
                                          }
                                  }
                        TempTask->EndProfile();
                        }
                if(List1Num == 0)break; // No items in List1
                if ((TempTask = List1->NextItem()) == NULL)
                        {// Get the next Task on the list
                        cout << "\nTried to access empty task list\n";
                        return(-1);
                        }
                TempTask->StartProfile();
                Suspend = TempTask->Run(); // Run the Task
                IncrementTime();
                TempTask->EndProfile();
                if (Suspend == -1)
                        {
                        cout << "\nProblem running Task: " <<
                                TempTask->Name << "\n";
                        return(-1);
                        }
                }
        return (0);
        }

// System dependent call to disable all maskable interrupts.
void DisableInterrupt (void)
         {
         #ifdef ISR_SCHEDULING
         _disable();
         #endif // ISR_SCHEDULING
         }

// System dependent call to enable all maskable interrupts.
void EnableInterrupt (void)
         {
         #ifdef ISR_SCHEDULING
         _enable();
         #endif // ISR_SCHEDULING
         }

// Sections of code which needs to be protected against
//  preemption should be bracketed by calls to CriticalSectionBegin()
//  and CriticalSectionEnd().  This method is preferred to diabling
//  interrupts because the interrupt helps to keep track of time
//  when the combination of interrupts and Timer0 are used.
// If the CriticalSection flag is set to 1, the scheduling in the
//  interrupt is diabled
static int CriticalSection = 0;
static int CriticalCounter = 0;  // Used to indicate when to actually end a
// critical section -- this is done so nested critical sections are handled
// properly.

// void CriticalSectionBegin (void)
//  Do not allow interrupt scheduling to occur after a call to
//  this function until CriticalSectionEnd() is subsequently called.
void CriticalSectionBegin (void)
         {
         DisableInterrupt();
         CriticalCounter++;  // Increment for each entry
         CriticalSection = 1;
         EnableInterrupt();
         }

// void CriticalSectionEnd (void)
//  Allow the interrupt to resume scheduling tasks.
void CriticalSectionEnd (void)
         {
         DisableInterrupt();
         CriticalCounter--;  // decrement for each exit
         if(CriticalCounter <= 0)
            {
            // Only remove critical status when leaving the last nesting
            CriticalSection = 0;
            CriticalCounter = 0;  // Make sure counter is at 0 (ignore
                // extra ENDs)
            }
         EnableInterrupt();
         }

#ifdef ISR_SCHEDULING
// File scope global variables for interrupt service routine
// Keep track of preemption (reentry) with this flag
static int ReentryInterrupt = 0;
static int ReentryPreemptable = 0;
int ReentryLevel = 0;
// Allocate static memory to save the floating point processor
//  state apon reentry.  The compiler (Borland C++ 4.5) does not
//  take care of this even for a void interrupt type function.
static far char FPU_Buffer[94 * 2];

// Function to be installed as interrupt service routine
void interrupt TimerISR(...)
        {
        //  Save the state of the floating point unit to the FPU.
        //  Calling fsave [DI] causes all the FPU's internal
        //  registers to be saved to a memory location pointed to
        //  by DI and referenced by the segment register DS.
        //  Therefore, before calling fsave we must set DS to the
        //  data segment of the FPU "stack" space, since defining
        //  FPU_Buffer as a file-scope global has put it in a data
        //  segment which may be different from the one we're in
        //  when this ISR runs.  DI gets the offset into that data
        //  segment at which the FPU state will go.
        // --------------------------------------------------------

        //      Assembly code to save the state of the floating
        //      point processor.
        DisableInterrupt ();
        unsigned Save_DI = FP_OFF (FPU_Buffer) + (94*ReentryLevel);
        unsigned Save_DS = FP_SEG (FPU_Buffer);
        asm     push    DS    //  Push the data segment in which the ISR is
        _DI = Save_DI;        //  intended by the compiler to run; then get
        _DS = Save_DS;        //  DI and DS from the local, automatic vars.
        asm     fsave   [DI]  //  in which they were saved and use them to
        asm     fwait         //  save the FPU state.  Then pop DS because
        asm     pop     DS    //  it's needed for dealing with other data
        EnableInterrupt ();

        // Read the real time clock so that we avoid an overflow and
        //  don't lose track of time.
        GetTimeNow();
        // IF the program is in a critical section - signified by a
        //  a call to CriticalSectionBegin() - don't allow any scheduling.
        if (CriticalSection||ReentryInterrupt)
                {
                // Clean up and return from interrupt
                DisableInterrupt ();
                asm     push    DS
                _DI = Save_DI;      //  Get the data segment and offset at which
                _DS = Save_DS;      //  we saved the FPU state; use to restore
                asm     frstor  [DI]//  the FPU state.  Wait until FPU is fully
                asm     fwait       //  restored before going back to background
                asm     pop     DS
                EnableInterrupt ();
                TimerEOI ();         //  Re-enable hardware timer interrupt
                return;
                }
        ReentryLevel ++;
        ReentryInterrupt = 1;
        //  Re-enable hardware timer interrupt.  From this point on,
        //   this function may reenter, possibly allowing a third thread.
        TimerEOI ();

        // Run Interrupt list
        // Sequentially run all of the Interrupt tasks once.  If
        //  there is some sort of problem, call StopControl and let
        //  the scheduling loop in the main() function (the parent thread)
        //  handle exiting the program cleanly.
        if (TaskDispatcher(NULL, Interrupt))
                StopControl("\n***Problem with Interrupt list***\n");

        DisableInterrupt ();
        ReentryInterrupt = 0;
        EnableInterrupt ();
        // If this call to the ISR was reentrant, don't allow
        // preemptable list to be processed.
        if(ReentryPreemptable)
                {
                // Clean up and return from interrupt
                DisableInterrupt ();
                ReentryLevel --;
                asm     push    DS
                _DI = Save_DI;      //  Get the data segment and offset at which
                _DS = Save_DS;      //  we saved the FPU state; use to restore
                asm     frstor  [DI]//  the FPU state.  Wait until FPU is fully
                asm     fwait       //  restored before going back to background
                asm     pop     DS
                EnableInterrupt ();
                return;
                }
        DisableInterrupt ();
        ReentryPreemptable = 1;
        EnableInterrupt ();

        // Run Preemptable list
        // Sequentially run all of the Preemptable tasks once.  If
        //  there is some sort of problem, call StopControl and let
        //  the scheduling loop in the main() function (the parent thread)
        //  handle exiting the program cleanly.
        if (TaskDispatcher(NULL, Preemptable))
                StopControl("\n***Problem with Preemptable list***\n");

        // Clean up and return from interrupt
        DisableInterrupt ();
        ReentryPreemptable = 0;
        ReentryLevel --;
        asm     push    DS
        _DI = Save_DI;      //  Get the data segment and offset at which
        _DS = Save_DS;      //  we saved the FPU state; use 'em to restore
        asm     frstor  [DI]//  the FPU state.  Wait until FPU is fully
        asm     fwait       //  restored before going back to background
        asm     pop     DS
        EnableInterrupt ();
        return;
        }

static void  interrupt (*oldvec)(...);

// Set up the timer interrupt
void SetupTimerInterrupt(void)
    {
    // Declaration for a pointer to an interrupt service function
    // in which the previous interrupt vector (probably DOS clock)
    // will be stored and later reinstalled from.
    // Set the desired period for the timer ISR (in sec)
    SetupClock(GetTickRate());
    cout << "setting interrupt tickrate to :" << GetTickRate() << " sec\n";
    // Disable interrupts while setting up a new ISR
    DisableInterrupt ();
    // Get existing vector
    oldvec = _dos_getvect(8);
    // Replace it with the new timer interrupt function
    _dos_setvect(8,TimerISR);
    // Enable interrupts when done
    EnableInterrupt ();
    }

void RestoreTimerInterrupt(void)
    {
    DisableInterrupt ();
    // Restore the timer interrupt to what it was
    _dos_setvect(8,oldvec);
    // Reset the default mode for the timer (DOS)
    SetupClock(-1.0);
    EnableInterrupt ();
    }
#endif // ISR_SCHEDULING

// Functions associated with the class DataOutput

// Constructor for time-based array sizing
DataOutput::DataOutput(int nvars,double STime,double LTime,double DTime)
        {
        MaxOutputs = 0; // Make sure that Init... doesn't try an illegal delete!
        NVars = 0;
        BaseTime = 0.0;  // Default value
        if(InitDataOutput(nvars,STime,LTime,DTime) == -1)
                {
                StopControl("Can't initialize for output data\n");
                }
        }

                // Constructor for direct array sizing
DataOutput::DataOutput(int nvars,long nrows)
        {
        MaxOutputs = 0; // Make sure that Init... doesn't try an illegal delete!
        NVars = 0;
        StartTime = 0.0;        // Not used in this mode - make sure they have
        LastTime = 0.0; // legal values
        DeltaTime = 0.0;
        NextTime = 1.e20;
        BaseTime = 0.0;  // Default value
        if(InitDataOutput(nvars,nrows) == -1)
                {
                StopControl("Can't initialize for output data\n");
                }
        }


DataOutput::~DataOutput()       // Destructor
        {
        // If output arrays exist, delete them
        if(MaxOutputs != 0)delete [] TimeRecord;
        if(NVars != 0)
            {
            for(int i = 0; i < NVars; i++)delete [] OutRecord[i];
            delete [] OutRecord;
            }
        }

int DataOutput::SetUpArrays(int nvars,long nrows)
        {
        if((TimeRecord = new double[nrows]) == NULL)
                {
                return(-1);
                }
        for(int i = 0; i < nvars; i++)
                {
                if((OutRecord[i] = new double[nrows]) == NULL)
                        {
                        return(-1);
                        }
                }
        MaxOutputs = nrows;
        OutCount = 0;
        NVars = nvars;
        return(0);
        }

int DataOutput::InitDataOutput(int nvars,long nrows)
        {
        // Returns -1 for error, 0 for OK
        // If output arrays already exist, delete them
        if(MaxOutputs != 0)
      delete TimeRecord;
   if(NVars != 0)
      {
      delete OutRecord;
      }
   OutRecord = new double *[nvars];
        if(SetUpArrays(nvars,nrows) == -1)return(-1);
        return(0);
        }

int DataOutput::InitDataOutput(int nvars,double STime,double LTime,double DTime)
        {
        StartTime = STime;
        LastTime = LTime;
        DeltaTime = DTime;
        NextTime = StartTime;
        // Compute the number of rows and then call the other InitDataOutput
        long nrows = ceil((LTime - STime) / DTime);
        return(InitDataOutput(nvars,nrows));
        }

int DataOutput::AddData(double val, ...)        // Add a data row
   {
   va_list ArgPtr;
   va_start(ArgPtr, val);

   if(OutCount >= MaxOutputs)return(-1);
   double time = GetTimeNow() - BaseTime;
   TimeRecord[OutCount] = time;
   *(OutRecord[0] + OutCount) = val;
   for(int jj=1; jj < NVars; jj++)
      {
        *(OutRecord[jj] + OutCount) = va_arg(ArgPtr, double);
      }
   if (va_arg(ArgPtr, long int) != END_OF_ARGS)
     StopControl("\n\n***Incorrect Arg List for DataOutput::AddData***\n\n");
   OutCount++;
   NextTime += DeltaTime;
   va_end(ArgPtr);
   return(0);
   }

int DataOutput::IsTimeForOutput(void)   // Query to find out if output
        {
        // Returns 1 for YES, 0 for NO
        double time = GetTimeNow() - BaseTime;
        if(time >= NextTime)return(1);
        return(0);
        }

int DataOutput::WriteOutputFile(char *FileName)
        {
        ofstream OutputFile(FileName);  // Opens for 'write'

        if (!OutputFile)
                {
                cout << "Problem opening file " << FileName << "\n";
                return(-1);
                }
        // Returns -1 for error, 0 for OK

        for (int j=0; j < OutCount; j++)
                  {
                  OutputFile << TimeRecord[j] << " ";
                  for(int kk = 0; kk < NVars; kk++)
                                OutputFile << (*(OutRecord[kk] + j)) << " ";
                  OutputFile << "\n";
                  }

        OutputFile.close(); // close the output file.
        return(0);
        }
// Accumulate Scan Statistics
// not used? static long nscans[NTASKSMAX];  // This must be big enough for all tasks!

void InitScanStat(BaseTask *task)
         {
         ntasks++;
         if(ntasks > NTASKSMAX)
                  {
                  StopControl("Too many tasks (fix NTASKSMAX in basetask.cpp)\n");
                  }
         AllTasks[task->TaskID] = task;
         tskname[task->TaskID] = task->Name;
         // not used? nscans[task->TaskID] = 0;
         }

void WriteScanStat(char *FileName, double FinalTime)
         {
         long nsc,tot = 0;
         ofstream OutputFile(FileName); // Opens for 'write'

         if (!OutputFile)
                {
                cout << "Problem opening file " << FileName << "\n";
                return;
                }

         cout << "Task # \t Task Name \t\t #Scans\n\n";
         OutputFile << "Task # \t Task Name \t\t #Scans\n\n";
         for(int i = 0; i < ntasks; i++)
                  {
                  nsc = (AllTasks[i])->GetNumScan();
                  int IDno = (AllTasks[i])->TaskID;
                  tot += nsc;
                  cout << IDno << "\t" << tskname[i] << "\t\t" <<nsc << "\n";
                  OutputFile << IDno << "\t" << tskname[i] << "\t\t" <<nsc << "\n";
                  }
         cout << "\nTotal scans, all tasks: " << tot << "\n";
         OutputFile << "\nTotal scans, all tasks: " << tot << "\n";
         cout << "\nTotal running time: " << FinalTime << "\n";
         OutputFile << "\nTotal running time: " << FinalTime << "\n";
         OutputFile.close();
         }

void WriteProfileStat(char *FileName)
         {
         ofstream OutputFile(FileName); // Opens for 'write'

         if (!OutputFile)
                {
                cout << "<ProfileStat>Problem opening file " << FileName << "\n";
                return;
                }

         OutputFile << "Task Profile Statistics\n";
         for(int i = 0; i < ntasks; i++)
                  {
                  OutputFile  << "\n"<< tskname[i] << "\n";
                  (AllTasks[i])->WriteProfile(OutputFile);
                  }
         OutputFile.close();
         }


