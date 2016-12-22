//*************************************************************************************
//  Console_DDE.cpp
//      This is insane!  Why on Earth can't I use DDE in a console application?  Maybe
//      if I give it its own thread, its own window, and its own house in the hills
//      with a three car garage containing a Rolls, a Ferrari and a Mercedes,...am I
//      going completely insane or what?  (Answer: Yes, no doubt about it.)
//
//  Revisions
//       8-19-97  JRR  Programmer goes completely looney and writes this file
//*************************************************************************************

#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <DDE_Stuf.hpp>
#include <MT_Debug.hpp>
#include <oper_int.hpp>


long Counter = 0;                           //  Data to make sure we're still running
int IntFromServer = 0;                      //  Sample data to be sent to clients
long LongFromClient = -999;                 //  Data to be received from clients
float FloatFromServer = 1.23F;              //  Floating point to be sent out
double DoubleFromClient = -999.9;           //  Double data to be received by server
char StringFromServer[32] = "Test Server";  //  String can be read by DDE clients
 

//-------------------------------------------------------------------------------------
//  Key Function:  ExitProgram
//      This function is called when the user presses the ^X key in the operator
//      interface.  It changes a variable to allow the program to finish running.

static bool TimeToExitProgram = false;

void ExitProgram (void)
    {
    TimeToExitProgram = true;
    }


//=====================================================================================

main ()
    {
    #ifdef MT_DEBUG_MODE
        InitDebugNotes ();
    #endif

    int Last_IFS = -1;
    float Last_FFS = -1.111;                //  These hold previous values of data
    char Last_SFS[32] = "+++ No String +++";

    //  Create a DDE server object
    C_DDE_Server* MyServer = new C_DDE_Server ("Testing");

    //  Add a topic called "Server_Topic", then add each of the variables under that
    //  topic.  Each of the items is data meant to be sent or received by the server
    C_DDE_Topic* Server_Topic = MyServer->AddTopic ("Server Data");
    C_DDE_Item* LFC = Server_Topic->AddItem ("LongFromClient", LongFromClient);
    C_DDE_Item* DFC = Server_Topic->AddItem ("DoubleFromClient", DoubleFromClient);
    C_DDE_Item* CNT = Server_Topic->AddItem ("Counter", Counter);
    C_DDE_Item* IFS = Server_Topic->AddItem ("IntFromServer", IntFromServer);
    C_DDE_Item* FFS = Server_Topic->AddItem ("FloatFromServer", FloatFromServer);
    C_DDE_Item* SFS = Server_Topic->AddItem ("StringFromServer", StringFromServer,
                                              32);

    //  Call function to start DDE server running
    MyServer->Register_DDE ();

    //  Create an operator window in which to display and edit the server's data 
    COperatorWindow* MyWindow = new COperatorWindow ("DDE Server Test Program");
    MyWindow->AddInputItem ("Integer:", &IntFromServer);
    MyWindow->AddInputItem ("Float:", &FloatFromServer);
    MyWindow->AddInputItem ("String:", StringFromServer);
    MyWindow->AddOutputItem ("Counter:", &Counter);
    MyWindow->AddOutputItem ("Long:", &LongFromClient);
    MyWindow->AddOutputItem ("Double:", &DoubleFromClient);
    MyWindow->AddKey ('X', "Exit", "Program", ExitProgram);
    MyWindow->Display ();

    //  Run everything in a loop instead of bothering with a scheduler
    while (TimeToExitProgram == false)
        {
        LFC->GetValue (LongFromClient);     //  Get data changed by DDE
        DFC->GetValue (DoubleFromClient);

        MyWindow->Update ();

        Counter++;
        if (Counter % 1000 == 0)            //  Don't update the counter too often
            CNT->SetValue (Counter);
        if (IntFromServer != Last_IFS)      //  If data has changed, send update
            {
            while (!IFS->SetValue (IntFromServer));
            Last_IFS = IntFromServer;
            }
        if (FloatFromServer != Last_FFS)
            {
            while (!FFS->SetValue (FloatFromServer));
            Last_FFS = FloatFromServer;
            }
        if (strcmp (StringFromServer, Last_SFS))
            {
            while (!SFS->SetValue (StringFromServer));
            strcpy (Last_SFS, StringFromServer);
            }
        }

    //  Delete the operator interface and the DDE server object.  When the server is
    //  deleted, it automatically closes the server GUI window and stops its thread
    delete MyWindow;
    delete MyServer;

    #ifdef MT_DEBUG_MODE
        PrintDebugNotes ("Server_Debug.txt");
    #endif

    printf ("\nEnd of DDE server test program.  Press RETURN to exit.\n");
    getchar ();
    }

