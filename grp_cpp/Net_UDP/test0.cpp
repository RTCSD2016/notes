// Test program for Internet/UDP data exchange
// Created 1/1/97, DM Auslander
// Files: test0.cpp, test1.cpp
// The files differ only in that one simulates process #0,
// the other #1

#include "net_udp.hpp"
#include <iostream.h>
#include <stdlib.h>
#include <conio.h>

static int ThisProcess = 0;  // Change this for other file
static int ToProc=1;
static char *ProcessAddress[2] =
    {
    // The same address for both processes is for testing
    //   with both processes running on the same computer
    "10.0.2.15", //"128.32.142.37",  "10.0.2.15",
    "10.0.2.15" //"128.32.142.37"  "10.0.2.15"
    };

main(void)
    {
    int RetVal;

    InitUDP(2,ThisProcess,ProcessAddress);

    // Send data to the other process
    int ToTask=2, nValues = 3;
    double Val[] = {1.2,4.3e-4,5.4e2};
    RetVal = InsertSharedArray(ToProc,ToTask,nValues,Val);
    cout << "Insert Shared RetVal " << RetVal << "\n";

    ToTask=2;
    int BoxNo=3,FromProc=ThisProcess,FromTask=2;
    int MsgFlag=7,RetRecpt=0,DelMode=1;
    double MsgValue=9.87;
    RetVal = InsertMessage(ToProc,ToTask,BoxNo,FromProc,
    	FromTask,MsgFlag,MsgValue,RetRecpt,DelMode);
    cout << "Insert Message RetVal " << RetVal << "\n";
    RetVal = SendData(); // Put the data out on the network
    cout << "Send Data RetVal " << RetVal << "\n";

    cout << "Hit any key to exit.\n";
    while(1)  // Infinite loop - user causes exit
        {
        CheckNet();  // Check for incoming data
        if(kbhit())break ;
        }
    NetCleanUp();
    }

// Dummy routines for the control program interface routines
void StoreMessage(int ToProc,int ToTask,int BoxNo,int FromProc,
     int FromTask,int MsgFlag,double MsgValue,int RetRecpt,int DelMode)
	{
	cout << "***Message received, Process #" << ThisProcess << "\n";
    cout << "ToProc " << ToProc << " ToTask " << ToTask << " BoxNo "
    	<< BoxNo << "\n";
    cout << "FromProc " << FromProc << " FromTask " << FromTask << "\n";
    cout << "MsgFlag " << MsgFlag << " MsgValue " << MsgValue
   	 << " RetRecpt " << RetRecpt << " DelMode " << DelMode << "\n";
    }

void StoreSharedData(int ToProc,int ToTask,int n,double *v)
	{
	cout << "***Shared Data received, Process #" << ThisProcess << "\n";
    cout << "ToProc " << ToProc << " ToTask " << ToTask << "\n";
    for(int i = 0; i < n; i++)cout << v[i] << " ";
    cout << "\n";
    }

