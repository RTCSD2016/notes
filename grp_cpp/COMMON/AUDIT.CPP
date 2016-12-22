// Functions for saving the task transition information
// to a file. Written for Borland C++ 4.5 based control programs
// File:  audit.cpp
// Created 8/15/95 by Jason Jones
#include "tasks.hpp"

// Arrays for Transition Audit Trail
static int AuditIndex = 0;
static char *TaskName[AUDITLENGTH];
static BaseTask *TaskPointer[AUDITLENGTH];
static int TaskID[AUDITLENGTH];
static int FromState[AUDITLENGTH];
static int ToState[AUDITLENGTH];
static double TransTime[AUDITLENGTH];

//-------------------------------------------------------
//  Function: InitAuditTrail
//		This function must be called to initialize the name
//		fields in the audit trail to NULL.  Call it before
//		any calls to AuditTrail
void InitAuditTrail(void)
	{
	for (int i=0; i<AUDITLENGTH; i++)
		TaskName[i] = NULL;
	}

//-------------------------------------------------------
//  Function: AuditTrail
//		This function writes to an array the time, name,
//		from state and to state for a transition.  The
//		arrays will roll over if more entries are written
//		than space was allocated for.
void AuditTrail(BaseTask *Task, int aFromState, int aToState)
	{
    TaskPointer[AuditIndex] = Task;
	TaskName[AuditIndex] = Task->Name;
	TaskID[AuditIndex] = Task->TaskID;
	FromState[AuditIndex] = aFromState;
	ToState[AuditIndex] = aToState;
	TransTime[AuditIndex] = GetTimeNow();

	AuditIndex ++;

	if (AuditIndex >= AUDITLENGTH)
		AuditIndex = 0;

	return;
	}

//-------------------------------------------------------
//  Function: WriteAuditTrail
//		This function writes the contents of the audit
//		trail arrays to a file.  The file name is the
//		only argument.  The data is written starting with
//		the oldest transition.
void WriteAuditTrail(char *FileName)
	{
	ofstream OutputFile(FileName);	// Opens for 'write'
	int i;
    BaseTask *pTask;
    char **sname;

	if (!OutputFile)
		{
		cout << "Problem opening file " << FileName << "\n";
		return;
		}

	if (TaskName[AuditIndex] != NULL)
		{
		for (i = AuditIndex; i < AUDITLENGTH; i++)
			{
            pTask = TaskPointer[i];
            sname = pTask->GetStateNames();
            if(sname == NULL)
                {
    			OutputFile << " " << TransTime[i] << " " <<
    	  			TaskName[i] << " " <<  FromState[i] <<
	    			" " <<  ToState[i] << "\n";
                }
            else
                {
    			OutputFile << " " << TransTime[i] << " " <<
    	  			TaskName[i] << " " <<  sname[FromState[i]] <<
	    			" " <<  sname[ToState[i]] << "\n";
                }
			}
		}
	for (i = 0; i < AuditIndex; i++)
    		{
            pTask = TaskPointer[i];
            sname = pTask->GetStateNames();
            if(sname == NULL)
                {
    			OutputFile << " " << TransTime[i] << " " <<
    	  			TaskName[i] << " " <<  FromState[i] <<
	    			" " <<  ToState[i] << "\n";
                }
            else
                {
    			OutputFile << " " << TransTime[i] << " " <<
    	  			TaskName[i] << " " <<  sname[FromState[i]] <<
	    			" " <<  sname[ToState[i]] << "\n";
                }
	    	}
	OutputFile.close(); // close the output file.
	}

//-------------------------------------------------------
//  Function: WriteAuditTrailNum
//		This function writes the contents of the audit
//		trail arrays to a file using task number instead of name.
//    The file name is the only argument.  The data is written
//		starting with the oldest transition.
void WriteAuditTrailNum(char *FileName)
	{
	ofstream OutputFile(FileName);	// Opens for 'write'
	int i;
	if (!OutputFile)
		{
		cout << "Problem opening file " << FileName << "\n";
		return;
		}

	if (TaskName[AuditIndex] != NULL)
		{
		for (i = AuditIndex; i < AUDITLENGTH; i++)
			{
			OutputFile << " " << TransTime[i] << " " <<
				TaskID[i] << " " <<  FromState[i] <<
				" " <<  ToState[i] << "\n";
			}
		}
	for (i = 0; i < AuditIndex; i++)
		{
		OutputFile << " " << TransTime[i] << " " <<
				TaskID[i] << " " <<  FromState[i] <<
				" " <<  ToState[i] << "\n";
		}
	OutputFile.close(); // close the output file.
	}

