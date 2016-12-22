// Base classes for use in group priority tasking system
// File:  basetask.hpp
// Created 8/2/95 by Jason Jones
// Added task profiling, 9/11/96, DM Auslander
// Added message passing, 10/1/96, DM Auslander
// Added Global data base, 10/7/96, DM Auslander
//      6/16/97, DM Auslander, added state names for audit trail

#ifndef BASETASK_HPP
#define BASETASK_HPP

#include <_null.h>

#define END_OF_ARGS -1l
#define LOG_PROFILE 0  // Constants for profiler
#define LINEAR_PROFILE 1

// Definition of a message
class Message_Box
   {
   public:  // These members need to be accessed by Basetask functions
      int FromTask;
      int FromProcess;
      int MessageFlag;
      int MessageAvailable;
      double MessageValue;
      Message_Box(void){MessageAvailable = 0;} // Constructor
   };

#define NUM_MESSAGE_BOX 20  // Number of message boxes in each task
#define MESSAGE_NORMAL 0  // Only deliver message if box is empty
#define MESSAGE_OVERWRITE 1 // Deliver message even if box is not empty

// Base class for task template.  The class must have a public Run() function.
class BaseTask
    {
    public:
        BaseTask(char *aName,int ProcNo = 0);  // Constructor
        void BaseTaskInit(char *aName,int taskcode);  // Initialization
        virtual ~BaseTask();
        void ActivateProfiling(double vbegin,double vend,
               int num_bins,int loglin);
        virtual int Run(void);
        long GetNumScan(void);
        int GetTaskProcess(void){return ProcessNo;}
        int GetTaskCode(void){return TaskCode;}
        void StartProfile(void);  // Do the profile calculations
        void EndProfile(void);
        void WriteProfile(ofstream &OutputFile);
        void SendRTMessage(BaseTask *ToTask,int BoxNo,int Flag = 1,
                double Val = 0.0,int mode = 0,int ReturnReceipt = -1);
        void SendRTMessage(int ToTask,int BoxNo,int Flag = 1,
                double Val = 0.0,int mode = 0,int ReturnReceipt = -1);
        int GetRTMessage(int BoxNo,int *msg_flag = NULL,
            double *val = NULL,int *from_task = NULL,int *from_process = NULL);
        // Delivery modes:
        //  0  Only deliver message if message box is empty (ie, flag=0)
        //  1  Deliver message regardless of message box state
        char *Name;     // Task name
        int TaskID;     // ID number that can be used to track tasks
        int TaskCode;  // Will be set to same as TaskID unless explicitly
               // set
        double GetGlobalData(BaseTask *pb,int k);
        int AcceptMessage(int BoxNo,int msg_flag,double msg_val,int mode,
              int from_task,int from_process);  // This was private,
              // but is has been moved to public so it can be
              // accessed by StoreMessage() and other network
              // interface functions. They could have been declared
              // to be 'friend' functions, but that would make the
              // placement of #define USE_UDP very critical.
        void SendCompletionMessage(int toTask,int &respBox,int code);
        int AcceptGlobal(double *pData,int nData);
        double *GetGlobalArray(BaseTask *pb,int *n);
        void CreateStateNameArray(int n);
        void RegisterStateName(char *pName,int i);
        char **GetStateNames(void);

    private:
        Message_Box mbox[NUM_MESSAGE_BOX]; // This is private so
          // it can't be accessed by derived classes -- this
          // protects the integrity of the message
        void SendMsg(BaseTask *ToTask,int BoxNo,int Flag,
                double Val,int mode,int ReturnReceipt);  // Internal function
        double *GlobalData;  // Global information for this task -- accessible
               // to all other tasks via call to GetGobalData()

    protected:
        int State, xState;      // Keep track of the current State.
        int NextState;  // The State to be run next execution scan.  -1 means no change
        int RunEntry;   // Flag set on State change to cause entry function to run
        double NextTaskTime, xNextTaskTime;// The next time this task should run
        double DeltaTaskTime, xDeltaTaskTime;// Period for this task
        long NumScans;  // For performance tracking
        int ProcessNo;  // Process that this task is located in
        // Data associated with task profiling
        int profile_on;  // Flag to determine if profiling is on
        int nbin;  // number of bins
        double *values;  // Boundary values for histogram bins
        double v1,vlast;  // Low and high boundaries for histogram
        long *occur;  // Record of occurrences by bin
        double Time0;  // Time at start of task scan
        double *PrivateGlobalData;  // Global data array -- must be copied
               // to public version for use by other tasks
        int GlobalSize;  // Number of global variables
        int CopyGlobal(void);  // Copy the private global variables to
          // public view -- for multiprocess systems this copies the
          // private global set to all processes
        int CreateGlobal(int nGlobal);  // Create global arrays
        char **StateNames;  // Array of pointers to store state names
        int nStateNames;
     };

// TListElement - data associated with any element on a list
class TListElement
    {
    public:
        TListElement(BaseTask *ItemPointer);
        ~TListElement(void);
        TListElement *Next;
        BaseTask *Item;
    };

// TList - a list of BaseTasks.  The list allocates space
// for itself as elements are added via Append.
class TList
    {
    public:
        TList(char *aName);
        ~TList(void);

        void Append(BaseTask *Item);// Add Item to end of list

        BaseTask *NextItem(void);   // Return pointer to next
                                                // item on list.

        int IsEmpty(void);          // Is the list empty
        int HowLong(void);          // How many items on list
        char *Name;                      // List name for debugging
    private:
        TListElement *First;      // First element on list
        TListElement *Last;       // Last element on list
        TListElement *Current;    // Last element on list
        int Length;               // Number of elements on list
    };

/* Class: DataOutput
This class sets up arrays to be used for accumulating data during a control run.
Each instance sets up a 1-D array to track the time and an N-D array for N doubles.
Two constructors are provided: one bases the array size on beginning and end
times and a delta-time; the other takes an array size directly.
Data entry functions can be either time based or direct.
Will check for array end and ignore input beyond that.
*/

class DataOutput
    {
    private:
        double NextTime;  // time of next output write
        double StartTime;   // Time at which output starts
        double LastTime;        // Time at which output ends
        double DeltaTime;    // period for output writes
      double BaseTime;  // Used to bias all time values (defaults to 0)
        long MaxOutputs;
        long OutCount;      // Running count of rows used
        double **OutRecord;
            // Array of pointers for output values
        double *TimeRecord; // Array for time values
        int NVars;
        int SetUpArrays(int nvars,long nrows);
    public:
        // Constructor for time-based array sizing
        DataOutput(int nvars,double STime,double LTime,double DTime);
        // Constructor for direct array sizing
        DataOutput(int nvars,long nrows);
        ~DataOutput();  // Destructor
        int AddData(double val, ...);  // Variable argument list
        int IsTimeForOutput(void);  // Query to find out if output
                // should be sent

        // Re-initialize -- deletes old arrays and starts new ones
        // NOTE: this might take significant computing time because of
        // calls to 'delete' and 'new'
        // These functions are called by the constructors
        int InitDataOutput(int nvars,double STime,double LTime,double DTime);
        int InitDataOutput(int nvars,long nrows);
        int WriteOutputFile(char *FileName);
      void SetBaseTime(double bt){BaseTime = bt;};
    };

void InitScanStat(BaseTask *task);
void ScanStat(BaseTask *task);
void WriteScanStat(char *FileName, double FinalTime);
void CriticalSectionBegin(void);
void CriticalSectionEnd(void);
void DisableInterrupt(void);
void EnableInterrupt(void);
int TaskDispatcher (TList *List1, TList *List2);
void StopControl(char *aMessage);
void SetTickRate(double tr);
double GetTickRate(void);
void WriteProfileStat(char *FileName);
int SendAll(void);
#endif // BASETASK_HPP

