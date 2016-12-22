//*************************************************************************************
//  CDDE_Client1.CPP
//      This is a test file with which I'm trying to set up a DDE client.  It's going
//      to talk to the Console DDE server test application, or so one hopes.
//
//  Version
//       6-11-97  JRR  Original file
//*************************************************************************************

#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <windows.h>
#include <MT_Debug.hpp>
#include <DDE_Stuf.hpp>
#include <oper_int.hpp>


long Counter = 5;                           //  Data to make sure we're still running
int IntFromServer = -5;                     //  Sample data to be sent to clients
long LongFromClient = -555;                 //  Data to be received from clients
float FloatFromServer = 5.55F;              //  Floating point to be sent out
double DoubleFromClient = -555.5;           //  Double data to be received by server
char StringFromServer[32] = "Empty B.S.";   //  String read by DDE client

C_DDE_Client* MyClient;


//-------------------------------------------------------------------------------------
//  Key Function:  ExitProgram
//      This function is called when the user presses the ^X key in the operator
//      interface.  It changes a variable to allow the program to finish running.

static bool TimeToExitProgram = false;

void ExitProgram (void)
    {
    TimeToExitProgram = true;
    }


//-------------------------------------------------------------------------------------
/*
void ConnectIt (void)
    {
    MyClient->ConnectToServer ("Excel", "Sheet1");
    }


void DisconnectIt (void)
    {
    MyClient->DisconnectFromServer ();
    }
*/

//=====================================================================================

main ()
    {
    long Last_LFC = -12345;                 //  Save previous values to detect changes
    double Last_DFC = -12345.678;

    #ifdef MT_DEBUG_MODE
        InitDebugNotes ();
    #endif

    //  Create a DDE client object
    MyClient = new C_DDE_Client ();

    //  Give the client a set of topics and items...
    //  Add a topic called "Server Data", then add each of the variables under that
    //  topic.  Each of these items is data meant to be received by the server
    C_DDE_Topic* Inputs_Topic = MyClient->AddTopic ("Server Data");
    C_DDE_Item* LFC = Inputs_Topic->AddItem ("LongFromClient", LongFromClient);
    C_DDE_Item* DFC = Inputs_Topic->AddItem ("DoubleFromClient", DoubleFromClient);

    //  These are the items which will be server outputs and client inputs
    C_DDE_Item* CNT = Inputs_Topic->AddItem ("Counter", Counter);
    C_DDE_Item* IFS = Inputs_Topic->AddItem ("IntFromServer", IntFromServer);
    C_DDE_Item* FFS = Inputs_Topic->AddItem ("FloatFromServer", FloatFromServer);
    C_DDE_Item* SFS = Inputs_Topic->AddItem ("StringFromServer", StringFromServer, 32);

    //  Specify that the preceding 3 items are to be advise-linked to the server
    IFS->AdviseLink ();
    FFS->AdviseLink ();
    SFS->AdviseLink ();
    CNT->AdviseLink ();

    //  Call function to start DDE client running
    MyClient->Register_DDE ();

    //  Connect to server and start advise links to update changing data
    MyClient->ConnectToServer ("Testing", "Server Data");

    COperatorWindow* MyWindow = new COperatorWindow ("DDE Client Test Program");
    MyWindow->AddOutputItem ("Integer:", &IntFromServer);
    MyWindow->AddOutputItem ("Float:", &FloatFromServer);
    MyWindow->AddOutputItem ("String:", StringFromServer);
    MyWindow->AddOutputItem ("Counter:", &Counter);
    MyWindow->AddInputItem ("Long:", &LongFromClient);
    MyWindow->AddInputItem ("Double:", &DoubleFromClient);

/*    MyWindow->AddOutputItem ("R1C1", &IntFromServer);     //  For tests with Excel
    MyWindow->AddOutputItem ("R2C1", &FloatFromServer);
    MyWindow->AddOutputItem ("R3C1", StringFromServer);
    MyWindow->AddOutputItem ("R4C1", &Counter);
    MyWindow->AddInputItem ("R5C1", &LongFromClient);
    MyWindow->AddInputItem ("R6C1", &DoubleFromClient);
*/
    MyWindow->AddKey ('X', "Exit", "Program", ExitProgram);
//    MyWindow->AddKey ('E', "Connect", "to Server", ConnectIt);
//    MyWindow->AddKey ('R', "Dis-", "connect", DisconnectIt);
    MyWindow->Display ();

    //  Run everything in a loop instead of bothering with a scheduler
    while (TimeToExitProgram == false)
        {
        while (!CNT->GetValue (Counter));         //  Get data changed by the server
        while (!IFS->GetValue (IntFromServer));
        while (!FFS->GetValue (FloatFromServer));
        while (!SFS->GetValue (StringFromServer));

        MyWindow->Update ();

        //  If the operator changed these values, send the updated values by DDE
        if (LongFromClient != Last_LFC)
            {
            while (!LFC->SetValue (LongFromClient));
            Last_LFC = LongFromClient;
            }
        if (fabs (DoubleFromClient - Last_DFC) > 1E-6)
            {
            while (!DFC->SetValue (DoubleFromClient));
            Last_DFC = DoubleFromClient;
            }
        }

    //  Close the link to the server
    MyClient->DisconnectFromServer ();

    //  Delete the operator interface and the DDE client object.  When the client is
    //  deleted, it automatically closes the client GUI window and stops its thread
    delete MyClient;
    delete MyWindow;

    #ifdef MT_DEBUG_MODE
        PrintDebugNotes ("Client_Debug.txt");
    #endif

    printf ("\nEnd of DDE client test program.  Press RETURN to exit.\n");
    getchar ();
    }

