//*************************************************************************************
//  DDE_Stuf.cpp
//      This file contains the class member functions for DDE handler objects.  These
//      objects encapsulate a DDE communication interface which allows C++ programs to
//      talk to each other (or to other applications) using dynamic data exchange.
//      Note that since DDE is a Windows phenomenon, this file can only be compiled
//      and run under Windows.  Also, it's designed for 32-bit, Windows 95/NT mode.
//
//  Classes
//      C_DDE_Item    - An item of data maintained by one of a server's topics
//      C_DDE_Topic   - A topic which is to be serviced by a DDE server
//      C_DDE_Manager - Base class with common code for client and server objects
//      C_DDE_Server  - A DDE server which services a bunch of topics and items
//      C_DDE_Client  - A client which can talk to a DDE server
//
//  Nonmember Functions
//      DDE_CheckForError          - Check for errors from the DDEML library
//      DDE_ErrorString            - Convert DDE error codes into readable strings        
//      DDE_Server_WndProc         - Processes messages for DDE server window
//      Create_DDE_Server_Window   - Creates the DDE server window
//      DDE_Server_Thread_Function - Function which runs the DDE server's thread
//      DdeServerCallback          - DDEML callback handles incoming DDE transactions
//      DdeClientCallback          - A similar callback, but for DDE client objects
//
//  Version
//       8-16-97  JRR  Original file
//       9-02-97  JRR  Added thread protection for data
//       9-07-97  JRR  A few changes to make it compatible with client object
//       9-14-97  JRR  Merged server and client into one file for convenience
//*************************************************************************************

#ifdef __WIN32__            //  This file should only be compiled for Win32 projects

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <ddeml.h>
#include <DDE_Stuf.hpp>
#include "MT_Debug.hpp"

//  We need global pointers to the DDE objects so the callback functions, which are
//  not member functions, can access member functions and data of the server object
static C_DDE_Server* The_DDE_Server = NULL;
static C_DDE_Client* The_DDE_Client = NULL;

static HWND hDDE_Server_Window = NULL;      //  Handles for the server's and client's
static HWND hDDE_Client_Window = NULL;      //  little GUI windows

static bool DDE_ServerTimeToStop = false;   //  These flags synchronize the exiting
static bool DDE_ServerThreadDone = false;   //  of the DDE threads with the server's
static bool DDE_ClientTimeToStop = false;   //  and the client's destructors
static bool DDE_ClientThreadDone = false;

static int NumConnections = 0;              //  Number of clients currently connected
const int MAX_TIMEOUT = 1000;               //  How long to wait for thread to finish

//------------------------------------------------------------------------------
//  Function:  DDE_NumberConnections
//       Returns the number of connections currently registered
//

int DDE_NumberConnections(void)
   {
   return(NumConnections);
   }
   
//-------------------------------------------------------------------------------------
//  Function:  DDE_CheckForError
//      This function checks to see if there has been a DDE error.  If there has, it
//      displays a complaint.

void DDE_CheckForError (DWORD idInstance)
    {
    UINT ErrorCode;                         //  Save the error (or no error) code here

    if ((ErrorCode = DdeGetLastError (idInstance)) != DMLERR_NO_ERROR)
        MessageBox (NULL, DDE_ErrorString (ErrorCode), "DDE Error!",
                    MB_OK | MB_ICONEXCLAMATION);
    }


//-------------------------------------------------------------------------------------
//  Function:  DDE_ErrorString
//      This function simply finds the correct string for a given DDE error, puts it
//      into the buffer it maintains, and returns a pointer to that buffer.

const char* DDE_ErrorString (UINT TheError)
    {
    static char DDE_Debug_String[64];       //  Buffer holds debugging strings

    //  Macro which copies DDE error strings into a buffer for output from function
    #define  ErrorDecode(x,y)  case (x): strcpy (DDE_Debug_String, (y)); break

    switch (TheError)
        {
        ErrorDecode (DMLERR_NO_ERROR, "NO ERROR: Oops, there's no error");
        ErrorDecode (DMLERR_ADVACKTIMEOUT, "Advise ACK timeout");
        ErrorDecode (DMLERR_BUSY, "Somebody's busy");
        ErrorDecode (DMLERR_DATAACKTIMEOUT, "Data ACK timeout");
        ErrorDecode (DMLERR_DLL_NOT_INITIALIZED, "DLL not initialized");
        ErrorDecode (DMLERR_DLL_USAGE, "DLL usage");
        ErrorDecode (DMLERR_EXECACKTIMEOUT, "Exec ACK timeout");
        ErrorDecode (DMLERR_INVALIDPARAMETER, "Invalid parameter");
        ErrorDecode (DMLERR_LOW_MEMORY, "Low memory");
        ErrorDecode (DMLERR_MEMORY_ERROR, "Memory error");
        ErrorDecode (DMLERR_NOTPROCESSED, "Not processed");
        ErrorDecode (DMLERR_NO_CONV_ESTABLISHED, "No conversation established");
        ErrorDecode (DMLERR_POKEACKTIMEOUT, "Poke ACK timeout");
        ErrorDecode (DMLERR_POSTMSG_FAILED, "Post message failed");
        ErrorDecode (DMLERR_REENTRANCY, "Re-entrancy is a no-no");
        ErrorDecode (DMLERR_SERVER_DIED, "Server died *sniff*");
        ErrorDecode (DMLERR_SYS_ERROR, "System error");
        ErrorDecode (DMLERR_UNADVACKTIMEOUT, "Unadvise ACK timeout");
        ErrorDecode (DMLERR_UNFOUND_QUEUE_ID, "Queue not found");
        default:
            sprintf (DDE_Debug_String, "Unknown error code %d", TheError);
            break;
        }
    return DDE_Debug_String;
    }


//-------------------------------------------------------------------------------------
//  Function:  DDE_WndProc
//      This is the window procedure for the DDE server window.  The DDE server runs
//      in its own thread and has its own message loop, and this is the procedure
//      which services messages for that window.

LRESULT CALLBACK DDE_WndProc (HWND hWnd, WPARAM message, WPARAM wParam, LPARAM lParam)
    {
    static LONG TextLineHeight;             //  Saves height of a line of screen text
    static TEXTMETRIC TextMetrics;          //  Structure for getting GUI text info
    PAINTSTRUCT PaintStruct;                //  Structure for talking to Windows GUI
    HDC hDevContext;                        //  Another structure for talking to GUI
    RECT ClientRect, DrawRect;              //  Rectangle structures, also for GUI
    char TextBuffer[80];                    //  Buffer holds a line of displayed text

    switch (message)
        {
        //  This message appears when the window is first being created
        case WM_CREATE:
            //  Figure out how big the window text is
            hDevContext = GetDC (hWnd);
            GetTextMetrics (hDevContext, &TextMetrics);
            TextLineHeight = TextMetrics.tmHeight + TextMetrics.tmExternalLeading;
            ReleaseDC (hWnd, hDevContext);
            return FALSE;

        //  This message means the window must be repainted.  Display text in the
        //  window showing how many connections to this server are currently active
        case WM_PAINT:
            BeginPaint (hWnd, &PaintStruct);
            SetBkMode (PaintStruct.hdc, TRANSPARENT);
            GetClientRect (hWnd, &ClientRect);
            ClientRect.bottom = ClientRect.top + TextLineHeight;
            wsprintf (TextBuffer, "%d connections active", NumConnections);
            DrawRect = ClientRect;
            if (IntersectRect (&DrawRect, &ClientRect, &(PaintStruct.rcPaint)))
                DrawText (PaintStruct.hdc, TextBuffer, -1, &ClientRect, DT_DDESTYLE);
            EndPaint (hWnd, &PaintStruct);
            break;

        //  If the message isn't anything we know about, let Windows deal with it
        default:
            return DefWindowProc (hWnd, message, wParam, lParam);
        }

    return FALSE;
    }


//-------------------------------------------------------------------------------------
//  Function:  Create_DDE_Window
//      This function creates a window just for the DDE server.  The window is just a
//      small white popup named "DDE Server" which doesn't display anything much.  A
//      handle for the window is returned.

HWND Create_DDE_Window (const char* aTitle)
    {
    WNDCLASS wc;

    wc.style = 0;                                   //  Class styles
    wc.lpfnWndProc = DDE_WndProc;                   //  Name of message loop function
    wc.cbClsExtra = 0;                              //  Not using Class Extra data
    wc.cbWndExtra = 0;                              //  Not using Window Extra data
    wc.hInstance = NULL;                            //  Instance that owns this class
    wc.hIcon = LoadIcon (NULL, IDI_APPLICATION);        //  Use default app. icon
    wc.hCursor = LoadCursor (NULL, IDC_ARROW);          //  Use arrow cursor
    wc.hbrBackground = GetStockObject (WHITE_BRUSH);    //  Use sys. backg. color
    wc.lpszMenuName = NULL;                             //  Resource name for menu
    wc.lpszClassName = "DDE Window Class";              //  Name for this class

    if (RegisterClass (&wc) == 0) return NULL;          //  Register the window class

    //  Create a window in the default X and Y position, size 240x100, no parent
    //  window nor menu, and owned by nobody
    return (CreateWindow ("DDE Window Class", aTitle,
                          WS_POPUP | WS_VISIBLE | WS_CAPTION, CW_USEDEFAULT,
                          CW_USEDEFAULT, 240, 100, 0, 0, NULL, NULL));
    }


//-------------------------------------------------------------------------------------
//  Function:  DDE_ServerThreadFunction
//      Because DDEML applications have to run entirely in one thread, the DDE server
//      object creates a thread and then runs entirely within it.  This is the thread
//      function for that thread.  It is started up by the C_DDE_Server constructor
//      and continues to run until the server object is deleted.

DWORD WINAPI DDE_ServerThreadFunction (LPVOID lpParam)
    {
    //  Set flag which tells other threads this one is still running
    DDE_ServerThreadDone = false;

    //  Create and display a window associated with the DDE system's thread
    if ((hDDE_Server_Window = Create_DDE_Window ("DDE Server")) == NULL)
        MessageBox (NULL, "Unable to create window", "DDE Error", MB_OK | MB_ICONSTOP);
    ShowWindow (hDDE_Server_Window, SW_SHOWNORMAL);
    UpdateWindow (hDDE_Server_Window);

    //  Wait until it's OK to register DDE server, topics, and items.  While waiting,
    //  give up the rest of each timeslice to other processes so they can do stuff
    while (The_DDE_Server->ReadyToRegister == false) Sleep (0);

    //  Initialize the DDE server object
    The_DDE_Server->Initialize ();

    //  Process messages for this window until we're told it's time to exit
    MSG msg;
    while (DDE_ServerTimeToStop == false)
        {
        //  Tell all items in advise loops to send data if it has been updated
        The_DDE_Server->SendAdviseData ();

        //  Process Windows messages in the ordinary way for GUI windows
        if (PeekMessage (&msg, 0, 0, 0, PM_REMOVE))
            {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
            }

        //  If no message was received, give remainder of timeslice to other threads
        else Sleep (0);                    
        }

    //  Uninitialize the server; this must be done within this thread
    The_DDE_Server->Uninitialize ();
    DDE_ServerThreadDone = true;
    return 0;
    }


//-------------------------------------------------------------------------------------
//  Function:  DDE_ClientThreadFunction
//      This is the thread function for the DDE client thread.  It works the same way
//      as the DDE server thread function above works, but it is started up by the
//      C_DDE_Client constructor and it runs until the client object is deleted.

DWORD WINAPI DDE_ClientThreadFunction (LPVOID lpParam)
    {
    //  Set flag which tells other threads this one is still running
    DDE_ClientThreadDone = false;

    //  Create and display a window associated with the DDE system's thread
    if ((hDDE_Client_Window = Create_DDE_Window ("DDE Client")) == NULL)
        MessageBox (NULL, "Unable to create window", "DDE Error", MB_OK | MB_ICONSTOP);
    ShowWindow (hDDE_Client_Window, SW_SHOWNORMAL);
    UpdateWindow (hDDE_Client_Window);

    //  Wait until it's OK to register DDE server, topics, and items; then do so
    while (The_DDE_Client->IsReadyToRegister () == false) Sleep (0);
    The_DDE_Client->Initialize ();

    //  Process messages for this window until we're told it's time to exit
    MSG msg;
    while (DDE_ClientTimeToStop == false)
        {
        //  Update the client by checking if the DDE thread has any jobs to do
        The_DDE_Client->Update ();

        //  Check for Windows messages and process them if any have come in
        if (PeekMessage (&msg, 0, 0, 0, PM_REMOVE))
            {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
            }
        else Sleep (0);                     //  If no message, let other threads run
        }

    //  Uninitialize the client, then shut down this thread
    The_DDE_Client->Uninitialize ();
    DDE_ClientThreadDone = true;
    return 0;
    }


//=====================================================================================
//  Class:  C_DDE_DataItem
//      This class represents one item of data to be maintained by the DDE server.
//      Each item has an item name, a type, and a value.
//=====================================================================================

//-------------------------------------------------------------------------------------
//  Constructor:  C_DDE_DataItem
//      These constructors create new C_DDE_DataItem objects.  One constructor is given
//      for each of the data types supported by the DDE server package.  The construc-
//      tor allocates storage space for the local copy (used by the DDE thread) of the
//      data and calls the common Construct() function to set everything else up.

C_DDE_Item::C_DDE_Item (const char* aName, int& rData)
    {
    pTheData = new int[1];                  //  Allocate space for the data
    *(int*)pTheData = rData;                //  Read the initial value of the data in
    TheDataType = DDT_int;                  //  Save the type of the data
    Construct (aName);                      //  Call the common constructor code
    }

C_DDE_Item::C_DDE_Item (const char* aName, unsigned int& rData)
    {
    pTheData = new unsigned int[1];         //  See comments above
    *(unsigned int*)pTheData = rData;
    TheDataType = DDT_uint;
    Construct (aName);
    }

C_DDE_Item::C_DDE_Item (const char* aName, long& rData)
    {
    pTheData = new long[1];                 //  Etc.
    *(long*)pTheData = rData;
    TheDataType = DDT_long;
    Construct (aName);
    }

C_DDE_Item::C_DDE_Item (const char* aName, unsigned long& rData)
    {
    pTheData = new unsigned long[1];
    *(unsigned long*)pTheData = rData;
    TheDataType = DDT_ulong;
    Construct (aName);
    }

C_DDE_Item::C_DDE_Item (const char* aName, float& rData)
    {
    pTheData = new float[1];
    *(float*)pTheData = rData;
    TheDataType = DDT_float;
    Construct (aName);
    }

C_DDE_Item::C_DDE_Item (const char* aName, double& rData)
    {
    pTheData = new double[1];
    *(double*)pTheData = rData;
    TheDataType = DDT_double;
    Construct (aName);
    }

C_DDE_Item::C_DDE_Item (const char* aName, long double& rData)
    {
    pTheData = new long double[1];
    *(long double*)pTheData = rData;
    TheDataType = DDT_long_double;
    Construct (aName);
    }

C_DDE_Item::C_DDE_Item (const char* aName, char& rData)
    {
    pTheData = new char[1];
    *(char*)pTheData = rData;
    TheDataType = DDT_char;
    Construct (aName);
    }

C_DDE_Item::C_DDE_Item (const char* aName, char* pData, int aSize)
    {
    pTheData = new char[aSize];             //  Allocate lots of space for a string
    strcpy ((char*)pTheData, pData);        //  Copy the string data in
    TheDataType = DDT_string;
    Construct (aName);
    }


//-------------------------------------------------------------------------------------
//  Function:  Construct
//      This function contains the code which is common to all the overloaded versions
//      of the constructor.  It is called by each of them.

void C_DDE_Item::Construct (const char* aName)
    {
    ItemName = new char[strlen (aName) + 1];    //  Allocate space for, and save,
    strcpy (ItemName, aName);                   //  the item's DDE name
    idInstance = 0L;                            //  We have no instance handle yet
    AdviseLinked = false;                       //  No advise links have been set up

    DataChanged = false;                        //  Flag will show when data changes
    DDE_Reading = false;                        //  Set the flags which indicate
    DDE_Writing = false;                        //  that data is being read or written
    User_Reading = false;                       //  by either the DDE or user thread
    User_Writing = false;                       //  to all false, as nobody's doing so
    }


//-------------------------------------------------------------------------------------
//  Destructor:  ~C_DDE_DataItem
//      This destructor just frees the memory which was used by the data item object.

C_DDE_Item::~C_DDE_Item (void)
    {
    delete [] ItemName;
    delete [] pTheData;
    }


//-------------------------------------------------------------------------------------
//  Function:  Register
//      Each DDE item must be registered with the DDEML system, and this registration
//      must take place within the DDE thread, not the user-code thread.  This func-
//      tion is called from within the DDE thread, and it does the registration.

void C_DDE_Item::Register (DWORD aInstance, HSZ aTopic)
    {
    //  Save instance handle and create DDE string handle for the name of this item
    idInstance = aInstance;
    hszItem = DdeCreateStringHandle (idInstance, ItemName, CP_DDESERV);
    if (hszItem == 0L) DDE_CheckForError (idInstance);

    hszTopic = aTopic;                      //  Save handle of this item's topic

    #ifdef MT_DEBUG_MODE
        TakeDebugNote ("Registering item \"%s\", instance %08X, handle %08X\n",
                       ItemName, idInstance, (int)hszItem);
    #endif
    }


//-------------------------------------------------------------------------------------
//  Function:  StringToData
//      This function uses the pointer to the data item to change that data in response
//      to a DDE message with new data.  The data is delivered as a string by the
//      DDE server which owns this data item.  The transfer is protected from being
//      corrupted by simultaneous reads and writes in the user's and DDE threads.

void C_DDE_Item::StringToData (const char* aString)
    {
    //  Check if user code is reading or writing the data; if so, wait until it's done
    while ((User_Writing == true) || (User_Reading == true))
        Sleep (0);

    //  Set a flag indicating that the DDE thread is writing data so the user's code
    //  won't read or change it as it's being written by this function
    DDE_Writing = true;

    switch (TheDataType)
        {
        case DDT_int:
            sscanf (aString, "%d", pTheData);
            break;
        case DDT_uint:
            sscanf (aString, "%d", pTheData);
            break;
        case DDT_long:
            sscanf (aString, "%ld", pTheData);
            break;
        case DDT_ulong:
            sscanf (aString, "%ld", pTheData);
            break;
        case DDT_float:
            sscanf (aString, "%f", pTheData);
            break;
        case DDT_double:
            sscanf (aString, "%lf", pTheData);
            break;
        case DDT_long_double:
            sscanf (aString, "%Lf", pTheData);
            break;
        case DDT_char:
            sscanf (aString, "%c", pTheData);
            break;
        case DDT_string:
            sscanf (aString, "%s", pTheData);
            break;
        };

    //  OK, we're done reading the data; the user's thread may read or write it now
    DDE_Writing = false;
    }


//-------------------------------------------------------------------------------------
//  Function:  DataToString
//      This function returns a pointer to a string which has just been filled with
//      the data in this data item, properly formatted.

const char* C_DDE_Item::DataToString (void)
    {
    static char ReturnString[256];          //  Holds string returned by function
    
    //  Check if the user's code is writing the data.  If so, wait until it's done
    while (User_Writing == true)
        Sleep (0);

    //  Set a flag indicating that the DDE thread is reading data so the user's code
    //  won't change it as it's being read by this function
    DDE_Reading = true;
    switch (TheDataType)
        {
        case DDT_int:
            sprintf (ReturnString, "%d", *(int *)pTheData);
            break;
        case DDT_uint:
            sprintf (ReturnString, "%u", *(unsigned int*)pTheData);
            break;
        case DDT_long:
            sprintf (ReturnString, "%ld", *(long*)pTheData);
            break;
        case DDT_ulong:
            sprintf (ReturnString, "%lu", *(unsigned long*)pTheData);
            break;
        case DDT_float:
            sprintf (ReturnString, "%f", *(float*)pTheData);
            break;
        case DDT_double:
            sprintf (ReturnString, "%lf", *(double*)pTheData);
            break;
        case DDT_long_double:
            sprintf (ReturnString, "%Lf", *(long double*)pTheData);
            break;
        case DDT_char:
            sprintf (ReturnString, "%c", *(char*)pTheData);
            break;
        case DDT_string:
            sprintf (ReturnString, "%s", (char*)pTheData);
            break;
        default:
            sprintf (ReturnString, "DDE Data Error");
            break;
        };
    DDE_Reading = false;

    return ReturnString;                    //  Return a pointer to the string
    }


//-------------------------------------------------------------------------------------
//  Function:  SetValue
//      This function is called to perform a thread-safe transfer of the user's data
//      (in the user's thread) to the copy kept by the DDE item and accessed by the
//      DDE thread.  Thread safety requires that the user thread wait until the DDE
//      thread has finished before changing the data, so these functions will refuse
//      to set the data if the DDE thread is currently using it.  If they successfully
//      write the data, the return value is true; otherwise it's false.

bool C_DDE_Item::SetValue (int aData)
    {
    //  If DDE thread is using the data or type is wrong, don't allow it to be written
    if ((DDE_Reading == true) || (DDE_Writing == true) || (TheDataType != DDT_int))
        return false;

    User_Writing = true;                    //  DDE isn't using the data so set a flag
    *(int*)pTheData = aData;                //  to indicate that this thread is, then
    User_Writing = false;                   //  change the data and clear the in-use
    DataChanged = true;                     //  flag and set data-changed flag so the
    return true;                            //  DDE thread is allowed to use the data
    }

bool C_DDE_Item::SetValue (unsigned int aData)
    {
    if ((DDE_Reading == true) || (DDE_Writing == true) || (TheDataType != DDT_uint))
        return false;
    User_Writing = true;
    *(unsigned int*)pTheData = aData;
    User_Writing = false;
    DataChanged = true;
    return true;
    }

bool C_DDE_Item::SetValue (long aData)
    {
    if ((DDE_Reading == true) || (DDE_Writing == true) || (TheDataType != DDT_long))
        return false;
    User_Writing = true;
    *(long*)pTheData = aData;
    User_Writing = false;
    DataChanged = true;
    return true;
    }

bool C_DDE_Item::SetValue (unsigned long aData)
    {
    if ((DDE_Reading == true) || (DDE_Writing == true) || (TheDataType != DDT_ulong))
        return false;
    User_Writing = true;
    *(unsigned long*)pTheData = aData;
    User_Writing = false;
    DataChanged = true;
    return true;
    }

bool C_DDE_Item::SetValue (float aData)
    {
    if ((DDE_Reading == true) || (DDE_Writing == true) || (TheDataType != DDT_float))
        return false;
    User_Writing = true;
    *(float*)pTheData = aData;
    User_Writing = false;
    DataChanged = true;
    return true;
    }

bool C_DDE_Item::SetValue (double aData)
    {
    if ((DDE_Reading == true) || (DDE_Writing == true) || (TheDataType != DDT_double))
        return false;
    User_Writing = true;
    *(double*)pTheData = aData;
    User_Writing = false;
    DataChanged = true;
    return true;
    }

bool C_DDE_Item::SetValue (long double aData)
    {
    if ((DDE_Reading == true) || (DDE_Writing == true)
        || (TheDataType != DDT_long_double)) return false;
    User_Writing = true;
    *(long double*)pTheData = aData;
    User_Writing = false;
    DataChanged = true;
    return true;
    }

bool C_DDE_Item::SetValue (char aData)
    {
    if ((DDE_Reading == true) || (DDE_Writing == true) || (TheDataType != DDT_char))
        return false;
    User_Writing = true;
    *(char*)pTheData = aData;
    User_Writing = false;
    DataChanged = true;
    return true;
    }

bool C_DDE_Item::SetValue (const char* pData)
    {
    if ((DDE_Reading == true) || (DDE_Writing == true) || (TheDataType != DDT_string))
        return false;
    User_Writing = true;
    strcpy ((char*)pTheData, pData);
    User_Writing = false;
    DataChanged = true;
    return true;
    }


//-------------------------------------------------------------------------------------
//  Function:  GetValue
//      This function performs a thread-safe transfer of the item data from the copy
//      kept by the DDE item and accessed by the DDE thread, to the copy kept by the
//      user's program.  If the DDE thread is currently reading the data, there is no
//      problem because both threads can simultaneously read the data; but if the DDE
//      thread is currently writing the data, this function refuses to read it and
//      returns false.

bool C_DDE_Item::GetValue (int& rData)
    {
    //  If DDE thread is currently writing the data we can't read it yet
    if ((DDE_Writing == true) || (TheDataType != DDT_int)) return false;
    User_Reading = true;
    rData = *(int*)pTheData;
    User_Reading = false;
    return true;
    }

bool C_DDE_Item::GetValue (unsigned int& rData)
    {
    if ((DDE_Writing == true) || (TheDataType != DDT_uint)) return false;
    User_Reading = true;
    rData = *(unsigned int*)pTheData;
    User_Reading = false;
    return true;
    }

bool C_DDE_Item::GetValue (long& rData)
    {
    if ((DDE_Writing == true) || (TheDataType != DDT_long)) return false;
    User_Reading = true;
    rData = *(long*)pTheData;
    User_Reading = false;
    return true;
    }

bool C_DDE_Item::GetValue (unsigned long& rData)
    {
    if ((DDE_Writing == true) || (TheDataType != DDT_ulong)) return false;
    User_Reading = true;
    rData = *(unsigned long*)pTheData;
    User_Reading = false;
    return true;
    }

bool C_DDE_Item::GetValue (float& rData)
    {
    if ((DDE_Writing == true) || (TheDataType != DDT_float)) return false;
    User_Reading = true;
    rData = *(float*)pTheData;
    User_Reading = false;
    return true;
    }

bool C_DDE_Item::GetValue (double& rData)
    {
    if ((DDE_Writing == true) || (TheDataType != DDT_double)) return false;
    User_Reading = true;
    rData = *(double*)pTheData;
    User_Reading = false;
    return true;
    }

bool C_DDE_Item::GetValue (long double& rData)
    {
    if ((DDE_Writing == true) || (TheDataType != DDT_long_double)) return false;
    User_Reading = true;
    rData = *(long double*)pTheData;
    User_Reading = false;
    return true;
    }

bool C_DDE_Item::GetValue (char& rData)
    {
    if ((DDE_Writing == true) || (TheDataType != DDT_char)) return false;
    User_Reading = true;
    rData = *(char*)pTheData;
    User_Reading = false;
    return true;
    }

bool C_DDE_Item::GetValue (char* pData)
    {
    if ((DDE_Writing == true) || (TheDataType != DDT_string)) return false;
    User_Reading = true;
    strcpy (pData, (char*)pTheData);
    User_Reading = false;
    return true;
    }


//-------------------------------------------------------------------------------------
//  Function:  SendAdviseData
//      This function runs within the DDE thread.  It checks to see if this data item
//      is advise linked and has been changed by a call to SetValue().  If so, the
//      function sends a message to the DDEML indicating that this data has changed.
//      Clients which are advise linked to this data will be advised of the change
//      and they'll send advise-request messages asking for the new value.

bool C_DDE_Item::SendAdviseData (void)
    {
    //  If this item isn't advise linked or hasn't changed, don't send it to anybody
    if ((AdviseLinked == false) || (DataChanged == false)) return false;

    #ifdef MT_DEBUG_MODE
        TakeDebugNote ("Post advise notice, item \"%s\"\n", ItemName);
    #endif

    //  OK, there's something to send, so send it and clear the data-changed flag
    DdePostAdvise (idInstance, hszTopic, hszItem);
    DataChanged = false;
    return true;
    }


//-------------------------------------------------------------------------------------
//  Function:  PokeChangedData
//      Running within the DDE thread, this function checks to see if data has been
//      changed by the user's thread.  If so, it sends the changed data to a server.

bool C_DDE_Item::PokeChangedData (HCONV hConversation)
    {
    HDDEDATA DidItWork;                     //  Whether the transaction worked right
    char DataString[256];                   //  Buffer to hold data in string format

    //  If DDEML isn't ready or the data hasn't changed, don't do anything
    if ((DataChanged == false) || (hConversation == NULL) || (idInstance == 0L))
        return false;

    //  Create a data handle for the data, then send data to the server synchronously
    //  (synchronously until I figure out how asynchronous transfers work, anyway)
    strcpy (DataString, DataToString ());
    HDDEDATA hData = DdeCreateDataHandle (idInstance, (LPBYTE)DataString,
                                       lstrlen (DataString), 0L, hszItem, CF_TEXT, 0);
    DidItWork = DdeClientTransaction ((LPBYTE)hData, -1, hConversation, hszItem,
                                      CF_TEXT, XTYP_POKE, DDE_SYNCH_TIMEOUT, NULL);

    #ifdef MT_DEBUG_MODE
        TakeDebugNote ("Poke data item \"%s\", value \"%s\"\n", ItemName, DataString);
    #endif

    //  If the data was sent successfully, return true to indicate that it was
    if (DidItWork)
        {
        DataChanged = false;
        return true;
        }
    else
        return false;
    }


//-------------------------------------------------------------------------------------
//  Function:  StartAdviseLink
//      This function initiates and advise link.  When this data item has been advise
//      linked, the server will tell the client when the data's value changes, and
//      the client (which owns this item) can then update the item's value.

void C_DDE_Item::StartAdviseLink (HCONV aConversation)
    {
    //  If no conversation is currently taking place, we can't link any items
    if ((idInstance == 0L) || (aConversation == NULL) || (AdviseLinked == false))
        return;

    //  If there is a conversation active, try starting an advise link
    DdeClientTransaction (NULL, NULL, aConversation, GetStringHandle (), CF_TEXT,
                          XTYP_ADVSTART, DDE_SYNCH_TIMEOUT, NULL);
    }


//-------------------------------------------------------------------------------------
//  Function:  EndAdviseLink
//      This function terminates an advise link which was set up the the function
//      StartAdviseLink().

void C_DDE_Item::EndAdviseLink (HCONV aConversation)
    {
    //  If no conversation is currently taking place, we can't link any items
    if ((idInstance == 0L) || (aConversation == NULL) || (AdviseLinked == false))
        return;

    //  If there is a conversation active, try starting an advise link
    DdeClientTransaction (NULL, NULL, aConversation, GetStringHandle (), CF_TEXT,
                          XTYP_ADVSTOP, DDE_SYNCH_TIMEOUT, NULL);
    }


//=====================================================================================
//  Class:  C_DDE_Topic
//      This class represents a topic to be served by the DDE server.  Each topic has
//      a name and a list of data items associated with it.
//=====================================================================================

//-------------------------------------------------------------------------------------
//  Constructor:  C_DDE_Topic
//      This constructor creates a new topic to be added to its parent DDE server's
//      topic list.

C_DDE_Topic::C_DDE_Topic (const char* aName) : CBasicList ()
    {
    Name = new char[strlen (aName) + 1];    //  Allocate space for topic name
    strcpy (Name, aName);                   //  Save the topic name
    idInstance = 0L;                        //  No handle to DDEML instance yet
    }


//-------------------------------------------------------------------------------------
//  Destructor:  ~C_DDE_Topic
//      This destructor just frees memory used by the topic object.

C_DDE_Topic::~C_DDE_Topic (void)
    {
    delete [] Name;

    //  Go through the data item list, deleting every data item we find
    C_DDE_Item* pTemp;
    C_DDE_Item* pCurItem = (C_DDE_Item*) GetHead ();
    while (pCurItem != NULL)
        {
        pTemp = pCurItem;
        pCurItem = (C_DDE_Item*) GetNext ();
        delete (pTemp);
        }
    }


//-------------------------------------------------------------------------------------
//  Function:  Register
//      This function registers the topic with DDEML.  It's made separate from the
//      constructor because the constructor is called from the user's thread, while
//      this function must be called from within the DDE thread.  When this function
//      is called to register the topic, it also calls the Register() methods of all
//      the items included in this topic.

void C_DDE_Topic::Register (DWORD aInstance)
    {
    //  Save instance handle, create DDE string handle for the name of this topic
    idInstance = aInstance;
    hszTopic = DdeCreateStringHandle (idInstance, Name, CP_DDESERV);
    if (hszTopic == 0L) DDE_CheckForError (idInstance);

    #ifdef MT_DEBUG_MODE
        TakeDebugNote ("Registering topic \"%s\", handle %08X\n", Name,
                       (int)hszTopic);
    #endif

    //  Go through the data item list, registering every item we find
    C_DDE_Item* pCurItem = (C_DDE_Item*) GetHead ();
    while (pCurItem != NULL)
        {
        pCurItem->Register (idInstance, hszTopic);
        pCurItem = (C_DDE_Item*) GetNext ();
        }
    }


//-------------------------------------------------------------------------------------
//  Function:  AddItem
//      This function creates a new DDE item and adds it to this topic's item list.
//      It's overloaded like crazy so that items can be created to contain data of
//      each of the many types supported by the DDE server.

C_DDE_Item* C_DDE_Topic::AddItem (const char* aName, int& rData)
    {
    //  Create a new data item and put a pointer to it in the list
    C_DDE_Item* pNew = new C_DDE_Item (aName, rData);
    CBasicList::Insert ((void*)pNew);
    return pNew;
    }

C_DDE_Item* C_DDE_Topic::AddItem (const char* aName, unsigned int& rData)
    {
    C_DDE_Item* pNew = new C_DDE_Item (aName, rData);
    CBasicList::Insert ((void*)pNew);
    return pNew;
    }

C_DDE_Item* C_DDE_Topic::AddItem (const char* aName, long& rData)
    {
    C_DDE_Item* pNew = new C_DDE_Item (aName, rData);
    CBasicList::Insert ((void*)pNew);
    return pNew;
    }

C_DDE_Item* C_DDE_Topic::AddItem (const char* aName, unsigned long& rData)
    {
    C_DDE_Item* pNew = new C_DDE_Item (aName, rData);
    CBasicList::Insert ((void*)pNew);
    return pNew;
    }

C_DDE_Item* C_DDE_Topic::AddItem (const char* aName, float& rData)
    {
    //  Create a new data item and put a pointer to it in the list
    C_DDE_Item* pNew = new C_DDE_Item (aName, rData);
    CBasicList::Insert ((void*)pNew);
    return pNew;
    }

C_DDE_Item* C_DDE_Topic::AddItem (const char* aName, double& rData)
    {
    //  Create a new data item and put a pointer to it in the list
    C_DDE_Item* pNew = new C_DDE_Item (aName, rData);
    CBasicList::Insert ((void*)pNew);
    return pNew;
    }

C_DDE_Item* C_DDE_Topic::AddItem (const char* aName, long double& rData)
    {
    //  Create a new data item and put a pointer to it in the list
    C_DDE_Item* pNew = new C_DDE_Item (aName, rData);
    CBasicList::Insert ((void*)pNew);
    return pNew;
    }

C_DDE_Item* C_DDE_Topic::AddItem (const char* aName, char& rData)
    {
    //  Create a new data item and put a pointer to it in the list
    C_DDE_Item* pNew = new C_DDE_Item (aName, rData);
    CBasicList::Insert ((void*)pNew);
    return pNew;
    }

C_DDE_Item* C_DDE_Topic::AddItem (const char* aName, char* rData, int aSize)
    {
    //  Create a new data item and put a pointer to it in the list
    C_DDE_Item* pNew = new C_DDE_Item (aName, rData, aSize);
    CBasicList::Insert ((void*)pNew);
    return pNew;
    }


//-------------------------------------------------------------------------------------
//  Function:  GetItemWithName
//      This function returns a pointer to an item in this topic's item list whose
//      name matches the given name.  If no item matches, it returns NULL.

C_DDE_Item* C_DDE_Topic::GetItemWithName (const char* aName)
    {
    //  Go through the data item list, until we find the one we want
    C_DDE_Item* pCurItem = (C_DDE_Item*)GetHead ();
    while (pCurItem != NULL)
        {
        if (strcmp (aName, pCurItem->GetName ()) == 0) return pCurItem;
        pCurItem = (C_DDE_Item*)GetNext ();
        }
    return NULL;
    }


//-------------------------------------------------------------------------------------
//  Function:  GetItemWithHandle
//      This function returns a pointer to an item in this topic's item list whose
//      name string handle matches the given name, or NULL if no items match.

C_DDE_Item* C_DDE_Topic::GetItemWithHandle (HSZ hszName)
    {
    #ifdef MT_DEBUG_MODE
        char str[32];
        DdeQueryString (idInstance, hszName, str, 30, CP_DDESERV);
        TakeDebugNote ("Looking for item with handle %08X named \"%s\"\n",
                       (int)hszName, str);
    #endif

    //  Go through the data item list, until we find the one we want
    C_DDE_Item* pCurItem = (C_DDE_Item*)GetHead ();
    while (pCurItem != NULL)
        {
        if (DdeCmpStringHandles (hszName, pCurItem->GetStringHandle ()) == 0)
            return pCurItem;
        pCurItem = (C_DDE_Item*)GetNext ();
        }
    #ifdef MT_DEBUG_MODE
        TakeDebugNote ("Topic not found.\n");
    #endif

    return NULL;
    }


//-------------------------------------------------------------------------------------
//  Function:  SendAdviseData
//      This function calls the SendAdviseData() methods for all the items in the
//      topic's item list to give them a chance to check if their data has been
//      changed and if it has, to send an advise message to DDEML saying it changed.

bool C_DDE_Topic::SendAdviseData (void)
    {
    //  Go through the data item list, asking every item we find to check for changes
    C_DDE_Item* pCurItem = (C_DDE_Item*) GetHead ();
    while (pCurItem != NULL)
        {
        pCurItem->SendAdviseData ();
        pCurItem = (C_DDE_Item*) GetNext ();
        }
    return true;
    }


//-------------------------------------------------------------------------------------
//  Function:  PokeChangedData
//      This function calls the PokeChangedData() methods for all the items in the
//      topic's item list to give them a chance to check if their data has been
//      changed and if it has, to send the changed data to servers which need it.

bool C_DDE_Topic::PokeChangedData (HCONV aConversation)
    {
    //  Go through the data item list, asking every item we find to check for changes
    C_DDE_Item* pCurItem = (C_DDE_Item*) GetHead ();
    while (pCurItem != NULL)
        {
        pCurItem->PokeChangedData (aConversation);
        pCurItem = (C_DDE_Item*) GetNext ();
        }
    return true;
    }


//-------------------------------------------------------------------------------------
//  Function:  StartAdviseLinks
//      This function tells all the items which belong to this topic to initiate
//      advise links to a server so that the server can automatically update the
//      data in these items if the server's copy of the data changes.

void C_DDE_Topic::StartAdviseLinks (HCONV aConversation)
    {
    C_DDE_Item* pCurItem = (C_DDE_Item*) GetHead ();
    while (pCurItem != NULL)
        {
        pCurItem->StartAdviseLink (aConversation);
        pCurItem = (C_DDE_Item*) GetNext ();
        }
    }


//-------------------------------------------------------------------------------------
//  Function:  EndAdviseLinks
//      This function tells each item in the list to break its advise link, if one
//      has been set up.

void C_DDE_Topic::EndAdviseLinks (HCONV aConversation)
    {
    C_DDE_Item* pCurItem = (C_DDE_Item*) GetHead ();
    while (pCurItem != NULL)
        {
        pCurItem->EndAdviseLink (aConversation);
        pCurItem = (C_DDE_Item*) GetNext ();
        }
    }


//=====================================================================================
//  Class:  C_DDE_Manager
//      This class encapsulates that part of the DDE interface code which is common to
//      both the DDE server and DDE client objects.  This includes holding basic DDE
//      registration information and managing lists of topics and items.
//=====================================================================================

//-------------------------------------------------------------------------------------
//  Constructor:  C_DDE_Manager
//      This constructor just initializes a few variables.

C_DDE_Manager::C_DDE_Manager (void) : CBasicList ()
    {
    ReadyToRegister = false;                //  Don't register with DDEML until ready
    NumTopics = 0;
    NumConnections = 0;
    }


//-------------------------------------------------------------------------------------
//  Destructor:  C_DDE_Manager
//      The DDE manager contains a list of topics, and the topics contain items; all
//      these things use dynamically allocated memory.  We free up their memory now
//      by deleting the topics; the topics' destructors delete all the items.

C_DDE_Manager::~C_DDE_Manager (void)
    {
    C_DDE_Topic* pTemp;
    C_DDE_Topic* pCurTopic = (C_DDE_Topic*) GetHead ();
    while (pCurTopic != NULL)
        {
        pTemp = pCurTopic;
        pCurTopic = (C_DDE_Topic*) GetNext ();
        delete (pTemp);
        }
    }


//-------------------------------------------------------------------------------------
//  Function:  Register_DDE
//      The registration of the client or server's items and topics must be done in
//      the DDEML thread, not the user thread; but it must be done after the user has
//      set up the topics and items.  This function sets a flag telling the DDE thread
//      that it's time to register the topic and item names and stuff with DDEML now.

void C_DDE_Manager::Register_DDE (void)
    {
    ReadyToRegister = true;
    }


//-------------------------------------------------------------------------------------
//  Function:  GetTopicWithName
//      This function returns a pointer to a topic in the topic list whose name
//      matches the given name.  If no item matches, it returns NULL.

C_DDE_Topic* C_DDE_Manager::GetTopicWithName (const char* aName)
    {
    #ifdef MT_DEBUG_MODE
        TakeDebugNote ("Looking for topic \"%s\"\n", aName);
    #endif

    //  Go through the topic list until we find the one we want or run out of topics
    C_DDE_Topic* pCurTopic = (C_DDE_Topic*)GetHead ();
    while (pCurTopic != NULL)
        {
        if (strcmp (aName, pCurTopic->GetName ()) == 0) return pCurTopic;
        pCurTopic = (C_DDE_Topic*)GetNext ();
        }

    //  If we get here, we didn't find any matching topics
    #ifdef MT_DEBUG_MODE
        TakeDebugNote ("Topic not found.\n");
    #endif
    return NULL;
    }


//-------------------------------------------------------------------------------------
//  Function:  GetTopicWithHandle
//      If there is a topic in the list with the given DDE name string handle, this
//      function returns a pointer to it.  If not, this function returns NULL.

C_DDE_Topic* C_DDE_Manager::GetTopicWithHandle (HSZ hszName)
    {
    #ifdef MT_DEBUG_MODE
        char str[32];
        DdeQueryString (idInstance, hszName, str, 30, CP_DDESERV);
        TakeDebugNote ("Looking for topic with handle %08X named \"%s\"\n",
                       (int)hszName, str);
    #endif

    //  Go through the data item list, until we find the one we want
    C_DDE_Topic* pCurTopic = (C_DDE_Topic*)GetHead ();
    while (pCurTopic != NULL)
        {
        //  If we've found the right one, return a pointer to it; if not, keep looking
        if (DdeCmpStringHandles (hszName, pCurTopic->GetStringHandle ()) == 0)
            return pCurTopic;
        pCurTopic = (C_DDE_Topic*)GetNext ();
        }
    #ifdef MT_DEBUG_MODE
        TakeDebugNote ("Topic not found.\n");
    #endif

    return NULL;
    }


//-------------------------------------------------------------------------------------
//  Function:  GetItemWithName
//      This function returns a pointer to an item whose name matches the given name.
//      The items are kept in the topics' item lists, so this function has to ask each
//      of the topics if it has the item.  If no item matches, it returns NULL.

C_DDE_Item* C_DDE_Manager::GetItemWithName (const char* aName)
    {
    #ifdef MT_DEBUG_MODE
        TakeDebugNote ("Looking for item \"%s\"\n", aName);
    #endif

    //  Go through the data item list, until we find the one we want
    C_DDE_Item* pItem;
    C_DDE_Topic* pCurTopic = (C_DDE_Topic*)GetHead ();
    while (pCurTopic != NULL)
        {
        if ((pItem = pCurTopic->GetItemWithName (aName)) != NULL)
            return pItem;
        pCurTopic = (C_DDE_Topic*)GetNext ();
        }
    #ifdef MT_DEBUG_MODE
        TakeDebugNote ("Item not found.\n");
    #endif

    return NULL;
    }


//-------------------------------------------------------------------------------------
//  Function:  GetItemWithHandle
//      If there is an item belonging to a topic in the topic list with the given DDE
//      name string handle, this function returns a pointer to it.  If not, this
//      function returns NULL.

C_DDE_Item* C_DDE_Manager::GetItemWithHandle (HSZ hszName)
    {
    #ifdef MT_DEBUG_MODE
        char str[32];
        DdeQueryString (idInstance, hszName, str, 30, CP_DDESERV);
        TakeDebugNote ("Looking for item with handle %08X named \"%s\"\n",
                       (int)hszName, str);
    #endif

    //  Go through the data item list, until we find the one we want
    C_DDE_Item* pItem;
    C_DDE_Topic* pCurTopic = (C_DDE_Topic*)GetHead ();
    while (pCurTopic != NULL)
        {
        //  If we've found the right one, return a pointer to it; if not, keep looking
        if ((pItem = pCurTopic->GetItemWithHandle (hszName)) != NULL)
            return pItem;
        pCurTopic = (C_DDE_Topic*)GetNext ();
        }
    #ifdef MT_DEBUG_MODE
        TakeDebugNote ("Item not found.\n");
    #endif

    return NULL;
    }


//-------------------------------------------------------------------------------------
//  Function:  AddTopic
//      This function adds a new topic to the DDE manager's topic list.  The topic is
//      just put on the end of the list because there's no particular order.

C_DDE_Topic* C_DDE_Manager::AddTopic (const char* aName)
    {
    C_DDE_Topic* pNew = new C_DDE_Topic (aName);
    CBasicList::Insert ((void*)pNew);
    NumTopics++;

    return (pNew);
    }


//=====================================================================================
//  Class:  C_DDE_Server
//      This class encapsulates a DDE server.  When set up by the user's program, it
//      will maintain a database of variables whose values can be queried by a client
//      program.  DDE clients will be able to read and set the values of the data by
//      making REQUEST and POKE type DDE transactions.
//=====================================================================================

//-------------------------------------------------------------------------------------
//  Constructor:  C_DDE_Server
//      This constructor is called to create a DDE server object.  It actually just
//      creates a new thread within which the DDEML system can run, and the thread's
//      main function calls Initialize() to set up the DDEML so that the server will
//      be ready to begin communicating with clients.

C_DDE_Server::C_DDE_Server (const char* aName) : C_DDE_Manager ()
    {
    //  Set the pointer to this object so that the DDEML callback can find it
    The_DDE_Server = this;

    //  Set the flag so thread won't stop, then create a new thread for DDEML
    DDE_ServerTimeToStop = false;
    HANDLE hThread = CreateThread (NULL, 0, DDE_ServerThreadFunction, NULL, 0,
                                   &dwThreadId);
    if (hThread == NULL)
        MessageBox (NULL, "Cannot create thread", "DDE Error", MB_OK | MB_ICONSTOP);

    //  Save the service name in a character string and initialize other variables
    ServiceName = new char[strlen (aName) + 1];
    strcpy (ServiceName, aName);
    phszPair = NULL;
    }


//-------------------------------------------------------------------------------------
//  Destructor:  ~C_DDE_Server
//      This destructor frees the memory used by the DDE server and any item objects
//      which it might have created.

C_DDE_Server::~C_DDE_Server (void)
    {
    int Timeout;                            //  Counts "time" for thread to finish

    //  Tell the DDE thread that it's time to exit now and close its window
    DDE_ServerTimeToStop = true;

    //  Delete the server/topic pair array and service name
    if (phszPair != NULL) delete [] phszPair;
    delete [] ServiceName;

    //  We should wait until the DDE thread has exited before going any further
    for (Timeout = 0L; Timeout < MAX_TIMEOUT; Timeout++)
        {
        Sleep (20);
        if (DDE_ServerThreadDone == true) break;
        }

    //  If the wait loop timed out, the wait for the DDE thread to exit was too long
    if (Timeout >= MAX_TIMEOUT)
        MessageBox (NULL, "DDE server thread not exiting on time", "DDE Warning",
                    MB_OK | MB_ICONEXCLAMATION);
    }


//-------------------------------------------------------------------------------------
//  Function:  Initialize
//      The DDE server's constructor starts up a thread, and the thread function calls
//      this function to initialize the DDEML system from within the newly started
//      thread.

void C_DDE_Server::Initialize (void)
    {
    //  Try to initialize DDE and get an instance handle
    idInstance = 0L;
    DdeInitialize ((LPDWORD)&idInstance, (PFNCALLBACK)DdeServerCallback,
                   APPCMD_FILTERINITS, 0L);

    //  Check for DDE errors and complain if any are found
    DDE_CheckForError (idInstance);

    //  Create string handle for service name; call user's function to create topic
    //  and item string handles
    hszService = DdeCreateStringHandle (idInstance, ServiceName, CP_DDESERV);
    if (hszService == 0L) DDE_CheckForError (idInstance);

    #ifdef MT_DEBUG_MODE
        TakeDebugNote ("Registering service \"%s\", handle %08X\n",
                       ServiceName, (int)hszService);
    #endif

    //  Register the service name.  This allows DDEML messages to come to this server
    DdeNameService (idInstance, hszService, NULL, DNS_REGISTER);
    DDE_CheckForError (idInstance);

    //  Register each of the topic names; the topic's registration function calls
    //  the registration functions of each of the items too
    C_DDE_Topic* pCurTopic = (C_DDE_Topic*) GetHead ();
    while (pCurTopic != NULL)
        {
        pCurTopic->Register (idInstance);
        pCurTopic = (C_DDE_Topic*) GetNext ();
        }
    }


//-------------------------------------------------------------------------------------
//  Function:  Uninitialize
//      The DDE server's destructor tells the DDE thread to call this function.  This
//      function then uninitializes the DDEML system.

void C_DDE_Server::Uninitialize (void)
    {
    #ifdef MT_DEBUG_MODE
        TakeDebugNote ("Uninitializing DDEML system\n");
    #endif

    if (idInstance != 0L)
        DdeUninitialize (idInstance);       //  Shut down the DDE server if it's up

    The_DDE_Server = NULL;                  //  Un-set the callback's pointer
    }


//-------------------------------------------------------------------------------------
//  Function:  SendAdviseData
//      This function tells all the topics owned by this server to tell all the items
//      which they own to update their advise-linked data.  If any of the items' data
//      has changed, the items will inform the client of the change of data.

bool C_DDE_Server::SendAdviseData (void)
    {
    //  Go through the topic list, asking every topic we find to do an update
    C_DDE_Topic* pTopic = (C_DDE_Topic*) GetHead ();
    while (pTopic != NULL)
        {
        pTopic->SendAdviseData ();
        pTopic = (C_DDE_Topic*) GetNext ();
        }

    return true;
    }


//-------------------------------------------------------------------------------------
//  Function:  SetServiceName
//      This function allows the user to specify the service name.

void C_DDE_Server::SetServiceName (const char* aName)
    {
    //  Un-register the previously registered service name
    DdeNameService (idInstance, hszService, NULL, DNS_UNREGISTER);

    //  Create a new string handle for the service name and register it
    hszService = DdeCreateStringHandle (idInstance, aName, CP_DDESERV);
    if (hszService == 0L) DDE_CheckForError (idInstance);

    DdeNameService (idInstance, hszService, NULL, DNS_REGISTER);
    DDE_CheckForError (idInstance);
    }


//-------------------------------------------------------------------------------------
//  Function:  ConnectClient
//      A client somewhere has requested a connection with a server, and the client
//      has specified a service name and a topic name in which it's interested.  If
//      the service and topic names match names supported by this server, hook on up.

HDDEDATA C_DDE_Server::ConnectClient (HSZ hsz1, HSZ hsz2)
    {
    //  If the service name matches, look for a match in each of the topic names
    if (hszService == hsz2)
        {
        C_DDE_Topic* pCurTopic = (C_DDE_Topic*)GetHead ();
        while (pCurTopic != NULL)
            {
            if (hsz1 == pCurTopic->GetStringHandle ()) return (HDDEDATA)TRUE;
            pCurTopic = (C_DDE_Topic*)GetNext ();
            }
        }

    //  Oh well, nothing matched
    return (HDDEDATA)FALSE;
    }


//-------------------------------------------------------------------------------------
//  Function:  WildConnect
//      If a client object is sniffing around trying to find all the service and/or
//      topic names supported by applications running on the system, the DDEML
//      callback will call this function.  It returns an array of service name/topic
//      name pairs which are supported by this server and match any given names.

HDDEDATA C_DDE_Server::WildConnect (HSZ hsz1, HSZ hsz2)
    {
    //  If the client's looking for this service name or any service name, fill the
    //  array of pairs of service and topic names and add a NULL/NULL pair at the end
    if ((hsz2 == hszService) || (hsz2 == NULL))
        {
        int nTopic = 0;                     //  Index for array of service/topic pairs

        //  Create or re-create array of handle pairs to hold service and topic names
        if (phszPair != NULL) delete [] phszPair;
        phszPair = new HSZPAIR[NumTopics + 1];

        for (C_DDE_Topic* pCurTopic = (C_DDE_Topic*)GetHead (); pCurTopic != NULL;
            pCurTopic = (C_DDE_Topic*)GetNext ())
            {
            //  Only create a pair of names for the list if the topic name matches
            if ((hsz1 == NULL) || (hsz1 == pCurTopic->GetStringHandle ()))
                {
                phszPair[nTopic].hszSvc = hszService;
                phszPair[nTopic].hszTopic = pCurTopic->GetStringHandle ();
                nTopic++;
                }
            }
        //  Add a pair of NULL service and topic names to mark the end of the list
        phszPair[nTopic].hszSvc = (HSZ)NULL;
        phszPair[nTopic].hszTopic = (HSZ)NULL;
        return (HDDEDATA)DdeCreateDataHandle (idInstance, (LPBYTE)phszPair,
                (NumTopics + 1) * sizeof (HSZPAIR), 0L, 0, CF_TEXT, 0);
        }

    //  If the service name is wrong, don't send back any information about topics
    return (HDDEDATA)FALSE;
    }


//-------------------------------------------------------------------------------------
//  Function:  RequestData
//      This function responds to a client application which has sent an XTYP_REQUEST
//      message through DDE to the server.  It looks for a data item whose topic and
//      item names match those given.  If it finds one, it formats the data in text
//      clipboard format and returns a handle to it.  If no topic and data names match
//      then this function returns the not-processed code.

HDDEDATA C_DDE_Server::RequestData (WORD wFormat, HSZ hsz1, HSZ hsz2)
    {
    static char DataString[256];            //  Place to store string sent to DDEML

    //  This server only supports the CF_TEXT format
    if (wFormat != CF_TEXT) return (HDDEDATA)DDE_FNOTPROCESSED;

    //  Look for a topic whose name is the same as that the client's looking for
    C_DDE_Topic* TheTopic;
    if ((TheTopic = GetTopicWithHandle (hsz1)) != NULL)
        {
        //  Look for an item belonging to that topic with the correct item name
        C_DDE_Item* TheItem;
        if ((TheItem = TheTopic->GetItemWithHandle (hsz2)) != NULL)
            {
            //  We found the right item - convert its data to a string and send it
            strcpy (DataString, TheItem->DataToString ());
            #ifdef MT_DEBUG_MODE
                TakeDebugNote ("Sending data \"%s\"\n", DataString);
            #endif
            return (HDDEDATA)DdeCreateDataHandle (idInstance, (LPBYTE)DataString,
                                       strlen (DataString) + 1, 0L, hsz2, CF_TEXT, 0);
            }
        }

    //  If the names don't match, return the code for "I can't do it"
    return (HDDEDATA)DDE_FNOTPROCESSED;
    }


//-------------------------------------------------------------------------------------
//  Function:  PokeData
//      This function responds to a client application which has sent an XTYP_POKE
//      message through DDE to the server.  It looks for a data item whose topic and
//      item names match those given.  If it finds one, it gets the data which was
//      sent, saves that data in the local data item, and returns an acknowledgement.
//      If the topic and data names don't match, then this function returns the
//      not-processed code and doesn't mess with the local copy of the data.

HDDEDATA C_DDE_Server::PokeData (WORD wFormat, HSZ hsz1, HSZ hsz2, HDDEDATA hData)
    {
    static char DataBuffer[256];            //  Place to keep text strings with data

    //  This server only supports the CF_TEXT format
    if (wFormat != CF_TEXT) return (HDDEDATA)DDE_FNOTPROCESSED;

    //  Look for a topic whose name is the same as that the client's looking for
    C_DDE_Topic* TheTopic;
    if ((TheTopic = GetTopicWithHandle (hsz1)) != NULL)
        {
        //  Look for an item belonging to that topic with the correct item name
        C_DDE_Item* TheItem;
        if ((TheItem = TheTopic->GetItemWithHandle (hsz2)) != NULL)
            {
            //  We found the right item; read the data which it gave us and save it
            DdeGetData (hData, (LPBYTE)DataBuffer, 254L, 0L);
            TheItem->StringToData (DataBuffer);
            #ifdef MT_DEBUG_MODE
                TakeDebugNote ("Receiving poked data \"%s\"\n", DataBuffer);
            #endif
            return (HDDEDATA)DDE_FACK;
            }
        }

    //  If the names don't match, return the code for "I can't do it"
    return (HDDEDATA)DDE_FNOTPROCESSED;
    }


//-------------------------------------------------------------------------------------
//  Function:  AdviseStart
//      This function responds to a client's XTYP_ADVSTART message.  It looks for an
//      item with the correct item and topic names which can be linked to the client
//      and if it finds one, it sends back a message to indicate the link is working.

HDDEDATA C_DDE_Server::AdviseStart (WORD wFormat, HSZ hsz1, HSZ hsz2)
    {
    //  This server only supports the CF_TEXT format
    if (wFormat != CF_TEXT) return (HDDEDATA)FALSE;

    //  Look for a topic whose name is the same as that the client's looking for
    C_DDE_Topic* TheTopic;
    if ((TheTopic = GetTopicWithHandle (hsz1)) != NULL)
        {
        //  Look for an item belonging to that topic with the correct item name
        C_DDE_Item* TheItem;
        if ((TheItem = TheTopic->GetItemWithHandle (hsz2)) != NULL)
            {
            //  We found the right item; tell the client we can support it unless it
            //  has already been advise-linked
            if (TheItem->IsAdviseLinked () == false)
                {
                TheItem->AdviseLink ();
                #ifdef MT_DEBUG_MODE
                    TakeDebugNote ("Setting up advise link on item \"%s\"\n",
                                   TheItem->GetName ());
                #endif
                return (HDDEDATA)TRUE;
                }
            }
        }
    //  If we get here, the item the client wants isn't supported
    #ifdef MT_DEBUG_MODE
        TakeDebugNote ("Cannot do advise link on item %08X\n", hsz2);
    #endif
    return (HDDEDATA)FALSE;
    }


//-------------------------------------------------------------------------------------
//  Function:  AdviseRequest
//      This function responds to a client's XTYP_ADVREQ message.  The client is
//      asking for some data; if the request is acceptable (format, topic and item)
//      then send the data to the client.  If not, send it a virtual raspberry.

HDDEDATA C_DDE_Server::AdviseRequest (WORD wFormat, HSZ hsz1, HSZ hsz2)
    {
    static char DataString[256];            //  Place to store string sent to DDEML

    //  This server only supports the CF_TEXT format
    if (wFormat != CF_TEXT) return (HDDEDATA)FALSE;

    //  Look for a topic whose name is the same as that the client's looking for
    C_DDE_Topic* TheTopic;
    if ((TheTopic = GetTopicWithHandle (hsz1)) != NULL)
        {
        //  Look for an item belonging to that topic with the correct item name
        C_DDE_Item* TheItem;
        if ((TheItem = TheTopic->GetItemWithHandle (hsz2)) != NULL)
            {
            //  We found the right item; if the item is advise-linked, send it to the
            //  client and if it isn't, well, don't
            if (TheItem->IsAdviseLinked () == true)
                {
                #ifdef MT_DEBUG_MODE
                    TakeDebugNote ("Advise request on \"%s\"\n", TheItem->GetName ());
                #endif
                //  We found the right item - convert its data to a string and send it
                strcpy (DataString, TheItem->DataToString ());
                return (HDDEDATA)DdeCreateDataHandle (idInstance, (LPBYTE)DataString,
                                       strlen (DataString) + 1, 0L, hsz2, CF_TEXT, 0);
                }
            }
        }
    //  If we get here, the item the client wants isn't supported
    #ifdef MT_DEBUG_MODE
        TakeDebugNote ("Can't do advise request on item %08X\n", hsz2);
    #endif
    return (HDDEDATA)FALSE;
    }


//-------------------------------------------------------------------------------------
//  Function:  AdviseStop
//      This function responds to a client's XTYP_ADVSTOP message.  The client is
//      cutting off the advise link on this item.

HDDEDATA C_DDE_Server::AdviseStop (HSZ hsz1, HSZ hsz2)
    {
    //  Look for a topic whose name is the same as that the client's looking for
    C_DDE_Topic* TheTopic;
    if ((TheTopic = GetTopicWithHandle (hsz1)) != NULL)
        {
        //  Look for an item belonging to that topic with the correct item name
        C_DDE_Item* TheItem;
        if ((TheItem = TheTopic->GetItemWithHandle (hsz2)) != NULL)
            {
            //  We found the right item; if the item is advise-linked, unlink it
            if (TheItem->IsAdviseLinked () == true)
                {
                #ifdef MT_DEBUG_MODE
                    TakeDebugNote ("Stopping advise link for item \"%s\"\n",
                                   TheItem->GetName ());
                #endif
                TheItem->AdviseUnlink ();
                return (HDDEDATA)TRUE;
                }
            }
        }
    //  If we get here, the item the client wants isn't supported
    #ifdef MT_DEBUG_MODE
        TakeDebugNote ("Can't stop advise link for item %08X\n", hsz2);
    #endif
    return (HDDEDATA)FALSE;
    }


//-------------------------------------------------------------------------------------
//  Function:  DdeServerCallback
//      This function handles callbacks from the DDEML.  Whenever a client application
//      wants to connect to this server, or disconnect, or communicate data, a call to
//      this callback function is how the server is informed.
//      Parameters: WORD wType     - transaction type
//                  WORD wFmt      - clipboard data format
//                  HCONV hConv    - handle of the conversation
//                  HSZ hsz1       - handle of a string
//                  HSZ hsz2       - handle of another string
//                  HDDEDATA hData - handle of a global memory object
//                  DWORD dwData1  - transaction-specific data
//                  DWORD dwData2  - transaction-specific data
//      Returns various results depending on the transaction type.

HDDEDATA EXPENTRY DdeServerCallback (WORD wType, WORD wFmt, HCONV hConv, HSZ hsz1,
                                 HSZ hsz2, HDDEDATA hData, DWORD lData1,DWORD lData2)
    {
    #ifdef MT_DEBUG_MODE
        TakeDebugNote ("Server Callback: ");
    #endif

    switch (wType)
        {
        //  This advisory message tells us that a new connection was made.  Increment
        //  the count of connections and make sure the GUI window shows the count
        case XTYP_CONNECT_CONFIRM:
            #ifdef MT_DEBUG_MODE
                TakeDebugNote ("XTYP_CONNECT_CONFIRM\n");
            #endif
            NumConnections++;
            InvalidateRect (hDDE_Server_Window, NULL, TRUE);
            return (HDDEDATA)FALSE;

        //  Another advisory message to tell us a connection has been dropped
        case XTYP_DISCONNECT:
            #ifdef MT_DEBUG_MODE
                TakeDebugNote ("XTYP_DISCONNECT\n");
            #endif
            NumConnections--;
            InvalidateRect (hDDE_Server_Window, NULL, TRUE);
            return (HDDEDATA)FALSE;

        //  Somebody wants to set up an advise link.  Call the function which will
        //  do so if the topic name and item name are supported by this server
        case XTYP_ADVSTART:
            #ifdef MT_DEBUG_MODE
                TakeDebugNote ("XTYP_ADVSTART\n");
            #endif
            return (The_DDE_Server->AdviseStart (wFmt, hsz1, hsz2));

        case XTYP_ADVREQ:
            #ifdef MT_DEBUG_MODE
                TakeDebugNote ("XTYP_ADVREQ\n");
            #endif
            return (The_DDE_Server->AdviseRequest (wFmt, hsz1, hsz2));

        case XTYP_ADVSTOP:
            #ifdef MT_DEBUG_MODE
                TakeDebugNote ("XTYP_ADVSTOP\n");
            #endif
            return (The_DDE_Server->AdviseStop (hsz1, hsz2));

        //  A client wants to connect to this server.  If the service and topic
        //  names match this server's names, allow the connection; if not, don't
        case XTYP_CONNECT:
            #ifdef MT_DEBUG_MODE
                char s1[32], s2[32];
                DdeQueryString (The_DDE_Server->idInstance, hsz1, s1, 30, CP_DDESERV);
                DdeQueryString (The_DDE_Server->idInstance, hsz2, s2, 30, CP_DDESERV);
                TakeDebugNote ("XTYP_CONNECT  Server: \"%s\" Topic: \"%s\"\n", s2, s1);
            #endif
            return (The_DDE_Server->ConnectClient (hsz1, hsz2));

        //  A client is asking around to find all the available servers and/or topics.
        //  Return an array of pairs of service and topic names this server supports
        case XTYP_WILDCONNECT:
            #ifdef MT_DEBUG_MODE
                TakeDebugNote ("XTYP_WILDCONNECT\n");
            #endif
            return (The_DDE_Server->WildConnect (hsz1, hsz2));

        //  A client is asking for some data
        case XTYP_REQUEST:
            #ifdef MT_DEBUG_MODE
                DdeQueryString (The_DDE_Server->idInstance, hsz1, s1, 30, CP_DDESERV);
                DdeQueryString (The_DDE_Server->idInstance, hsz2, s2, 30, CP_DDESERV);
                TakeDebugNote ("XTYP_REQUEST  Topic: \"%s\" Item: \"%s\"\n", s1, s2);
            #endif
            return (The_DDE_Server->RequestData (wFmt, hsz1, hsz2));

        //  Here, a client is sending some data
        case XTYP_POKE:
            #ifdef MT_DEBUG_MODE
                DdeQueryString (The_DDE_Server->idInstance, hsz1, s1, 30, CP_DDESERV);
                DdeQueryString (The_DDE_Server->idInstance, hsz2, s2, 30, CP_DDESERV);
                TakeDebugNote ("XTYP_POKE  Topic: \"%s\" Item: \"%s\"\n", s1, s2);
            #endif
            return (The_DDE_Server->PokeData (wFmt, hsz1, hsz2, hData));

        //  This server doesn't support execute transactions (yet), so reject it
        case XTYP_EXECUTE:
            #ifdef MT_DEBUG_MODE
                TakeDebugNote ("XTYP_EXECUTE\n");
            #endif
            return (HDDEDATA)DDE_FNOTPROCESSED;

        //  Someone's registering a server someplace.  We don't really care
        case XTYP_REGISTER:
            #ifdef MT_DEBUG_MODE
                DdeQueryString (The_DDE_Server->idInstance, hsz1, s1, 30, CP_DDESERV);
                DdeQueryString (The_DDE_Server->idInstance, hsz2, s2, 30, CP_DDESERV);
                TakeDebugNote ("XTYP_REGISTER  hsz1: \"%s\" hsz2: \"%s\"\n", s1, s2);
            #endif
            return (HDDEDATA)FALSE;

        //  The following just take notes about transactions we don't support
        case XTYP_ADVDATA:
            #ifdef MT_DEBUG_MODE
                TakeDebugNote ("XTYP_ADVDATA\n");
            #endif
            return (HDDEDATA)FALSE;

        case XTYP_UNREGISTER:
            #ifdef MT_DEBUG_MODE
                TakeDebugNote ("XTYP_UNREGISTER\n");
            #endif
            return (HDDEDATA)FALSE;

        case XTYP_XACT_COMPLETE:
            #ifdef MT_DEBUG_MODE
                TakeDebugNote ("XTYP_XACT_COMPLETE\n");
            #endif
            return (HDDEDATA)FALSE;

        case XTYP_ERROR:
            #ifdef MT_DEBUG_MODE
                TakeDebugNote ("XTYP_ERROR\n");
            #endif
            DDE_CheckForError (The_DDE_Server->idInstance);
            return (HDDEDATA)FALSE;

        //  If the transaction is something we don't recognize, just return FALSE
        default:
            #ifdef MT_DEBUG_MODE
                TakeDebugNote ("Unknown Message\n");
            #endif
            return (HDDEDATA)FALSE;
        }
    }


//=====================================================================================
//  Class:  C_DDE_Client
//      This class encapsulates DDE client communications.  It allows the user to send
//      and receive data to and from a server application.  In order to keep things
//      simple, the client can only carry on one conversation at a time, and it can
//      only use the text clipboard format.
//=====================================================================================

//-------------------------------------------------------------------------------------
//  Constructor:  C_DDE_Client
//      This constructor creates a DDE client object.  The client won't try to connect
//      to any servers until the Connect() method is called.

C_DDE_Client::C_DDE_Client (void)
    {
    //  Set the pointer used by the callback function to point to this object
    The_DDE_Client = this;

    //  Initialize all the local variables
    idInstance = 0L;
    hConversation = NULL;
    DoConnect = false;
    DoDisconnect = false;
    ServerName = NULL;
    TopicName = NULL;

    //  Set the flag so thread won't stop, then create a new thread for DDEML
    DDE_ClientTimeToStop = false;
    HANDLE hThread = CreateThread (NULL, 0, DDE_ClientThreadFunction, NULL, 0,
                                   &dwThreadId);
    if (hThread == NULL)
        MessageBox (NULL, "Cannot create thread", "DDE Client Error",
                    MB_OK | MB_ICONSTOP);
    }


//-------------------------------------------------------------------------------------
//  Destructor:  ~C_DDE_Client
//      This destructor frees the memory which was used by the DDE client and tells
//      the DDE client thread that it's time to exit.  The DDE thread will call the
//      un-initializing function which actually shuts the DDEML interface down.

C_DDE_Client::~C_DDE_Client (void)
    {
    int Timeout;                            //  Counts "time" for thread to finish

    //  Tell the DDE thread that it's time to exit now and close its window
    DDE_ClientTimeToStop = true;

    //  While we're waiting, delete some character strings
    if (ServerName != NULL) delete [] ServerName;
    if (TopicName != NULL) delete [] TopicName;

    //  We should wait until the DDE thread has exited before going any further
    for (Timeout = 0L; Timeout < MAX_TIMEOUT; Timeout++)
        {
        Sleep (20);                         //  Let other threads take over timeslice
        if (DDE_ClientThreadDone == true) break;
        }

    //  If the wait loop timed out, the wait for the DDE thread to exit was too long
    if (Timeout >= MAX_TIMEOUT)
        MessageBox (NULL, "DDE client thread not exiting on time", "DDE Warning",
                    MB_OK | MB_ICONEXCLAMATION);
    }


//-------------------------------------------------------------------------------------
//  Function:  Initialize
//      This function runs within the DDE thread and prepares the client object.

void C_DDE_Client::Initialize (void)
    {
    //  Try to initialize DDE and get an instance handle
    idInstance = 0L;
    DdeInitialize ((LPDWORD)&idInstance, (PFNCALLBACK)DdeClientCallback,
                   APPCMD_CLIENTONLY, 0L);

    #ifdef MT_DEBUG_MODE
        TakeDebugNote ("Initializing DDE client\n");
    #endif

    //  Check for DDE errors and complain if any are found
    DDE_CheckForError (idInstance);

    //  Register each of the topic names; the topic's registration function calls
    //  the registration functions of each of the items too
    C_DDE_Topic* pCurTopic = (C_DDE_Topic*) GetHead ();
    while (pCurTopic != NULL)
        {
        pCurTopic->Register (idInstance);
        pCurTopic = (C_DDE_Topic*) GetNext ();
        }
    }


//-------------------------------------------------------------------------------------
//  Function:  Uninitialize

void C_DDE_Client::Uninitialize (void)
    {
    if (idInstance != 0L)
        DdeUninitialize (idInstance);       //  Shut down the DDE client if it's up
    The_DDE_Client = NULL;

    #ifdef MT_DEBUG_MODE
        TakeDebugNote ("Un-initializing DDE client\n");
    #endif
    }


//-------------------------------------------------------------------------------------
//  Function:  PokeChangedData
//      This function calls the PokeChangedData() methods for all the items in the
//      topic's item list to give them a chance to check if their data has been
//      changed and if it has, to send the changed data to servers which need it.

bool C_DDE_Client::PokeChangedData (void)
    {
    //  If there's no DDE conversation active, don't try to send any data
    if (hConversation == NULL) return false;

    //  Go through the data item list, asking every item we find to check for changes
    C_DDE_Topic* pCurTopic = (C_DDE_Topic*) GetHead ();
    while (pCurTopic != NULL)
        {
        pCurTopic->PokeChangedData (hConversation);
        pCurTopic = (C_DDE_Topic*) GetNext ();
        }

    return true;
    }


//-------------------------------------------------------------------------------------
//  Function:  ConnectToServer
//      This function is called by the user's thread to connect our client to a DDE
//      server with the given service and topic names.  It sets flags so that the
//      DDE_ConnectToServer() function, running in the DDE thread, will connect to the
//      server when it gets a chance.
//      Return value:  True if the connection can be made, false otherwise

bool C_DDE_Client::ConnectToServer (const char* aServerName, const char* aTopicName)
    {
    //  If there's a conversation handle in use, we're connected already
    if (hConversation != NULL) return false;

    //  Allocate some space and save the server and topic names
    if (ServerName != NULL) delete [] ServerName;
    ServerName = new char[strlen (aServerName) + 1];
    strcpy (ServerName, aServerName);
    if (TopicName != NULL) delete [] TopicName;
    TopicName = new char[strlen (aTopicName) + 1];
    strcpy (TopicName, aTopicName);

    //  Signal the DDE thread that it should connect and return
    DoConnect = true;
    return true;
    }


//-------------------------------------------------------------------------------------
//  Function:  DisconnectFromServer
//      This function disconnects the DDE client from the server.
//      Return value:  true if it had been connected and was successfully disconnected
//      and false otherwise.

bool C_DDE_Client::DisconnectFromServer (void)
    {
    if (hConversation == NULL) return false;    //  If not connected, can't disconnect

    DoDisconnect = true;                        //  If we are, we can, so do it
    return true;
    }


//-------------------------------------------------------------------------------------
//  Function:  Update
//      This function is called periodically within the DDE thread.  It checks to see
//      if the user thread has ordered the DDE thread to do anything such as connect
//      our client to a DDE server with the given service and topic names, disconnect
//      from a server, or update changed data items.
//      Return value:  none

void C_DDE_Client::Update (void)
    {
    //  If DDEML didn't initialize correctly, we can't do anything
    if (idInstance == 0L) return;

    //  If it's time to connect to a server and we're not connected already, hook up
    if ((DoConnect == true) && (hConversation == NULL))
        {
        DoConnect = false;

        //  Create string handles, make sure they're OK, and then connect
        HSZ hszService, hszTopic;
        hszService = DdeCreateStringHandle (idInstance, ServerName, 0);
        hszTopic = DdeCreateStringHandle (idInstance, TopicName, 0);
        if ((hszService == 0L) || (hszTopic == 0L)) return;

        //  Here's where we actually start the conversation, hopefully
        hConversation = DdeConnect (idInstance, hszService, hszTopic, (LPVOID)NULL);
        if (hConversation == NULL)
            MessageBox (NULL, "Cannot connect to server", "DDE Client Error", MB_OK);

        #ifdef MT_DEBUG_MODE
            TakeDebugNote ("Connecting to server \"%s\", topic \"%s\", ", ServerName,
                           TopicName);
            TakeDebugNote ("conversation handle %08X\n", (int)hConversation);
        #endif

        //  If we connected OK, record that information and return it to the caller
        DdeFreeStringHandle (idInstance, hszService);
        DdeFreeStringHandle (idInstance, hszTopic);

        //  Get each of the topics to tell each of its items to connect advise links
        C_DDE_Topic* pCurTopic = (C_DDE_Topic*) GetHead ();
        while (pCurTopic != NULL)
            {
            pCurTopic->StartAdviseLinks (hConversation);
            pCurTopic = (C_DDE_Topic*) GetNext ();
            }
        }

    //  Scan all the items to check if any of them needs to be updated
    PokeChangedData ();

    //  Check to see if we're supposed to disconnect from a server; if so, do so
    if ((DoDisconnect == true) && (hConversation != NULL))
        {
        DoDisconnect = false;

        //  Get each topic to tell each of its items to break its advise link
        C_DDE_Topic* pCurTopic = (C_DDE_Topic*) GetHead ();
        while (pCurTopic != NULL)
            {
            pCurTopic->EndAdviseLinks (hConversation);
            pCurTopic = (C_DDE_Topic*) GetNext ();
            }

        #ifdef MT_DEBUG_MODE
            TakeDebugNote ("Disconnecting conversation %08X\n", (int)hConversation);
        #endif

        DdeDisconnect (hConversation);
        hConversation = NULL;
        }
    }


//-------------------------------------------------------------------------------------
//  Function:  AdviseData
//      This function processes an advise-loop data message.  If the message has data
//      which updates the value in a data item which is connected in an advise loop,
//      this function updates that item with the data and returns an acknowledgement;
//      if not, the function returns a null value.

HDDEDATA C_DDE_Client::AdviseData (WORD wFormat, HSZ hsz1, HSZ hsz2, HDDEDATA hData)
    {
    static char DataBuffer[256];            //  Place to keep text strings with data

    //  This client only supports the CF_TEXT format
    if (wFormat != CF_TEXT) return (HDDEDATA)DDE_FNOTPROCESSED;

    //  Look for a topic whose name is the same as that the client's looking for
    C_DDE_Topic* TheTopic;
    if ((TheTopic = GetTopicWithHandle (hsz1)) != NULL)
        {
        //  Look for an item belonging to that topic with the correct item name
        C_DDE_Item* TheItem;
        if ((TheItem = TheTopic->GetItemWithHandle (hsz2)) != NULL)
            {
            //  We found the right item; read the data which it gave us and save it
            DdeGetData (hData, (LPBYTE)DataBuffer, 254L, 0L);
            TheItem->StringToData (DataBuffer);
            #ifdef MT_DEBUG_MODE
                TakeDebugNote ("Got advise-loop data \"%s\"\n", DataBuffer);
            #endif
            return (HDDEDATA)DDE_FACK;
            }
        }
    return (HDDEDATA)DDE_FNOTPROCESSED;
    }


//-------------------------------------------------------------------------------------
//  Function:  DdeClientCallback
//      This function handles callbacks from the DDEML.  Because we're dealing with a
//      DDE client, the callback doesn't do anything very interesting.
//      Parameters:  See the comments for DdeServerCallback() in this file.
//      Return value:  It depends on what the callback was doing.

HDDEDATA EXPENTRY DdeClientCallback (WORD wType, WORD wFmt, HCONV hConv, HSZ hsz1,
                                 HSZ hsz2, HDDEDATA hData, DWORD lData1, DWORD lData2)
    {
    #ifdef MT_DEBUG_MODE
        TakeDebugNote ("Client Callback: ");
    #endif

    switch (wType)
        {
        //  This message brings us advise-loop data which we must process
        case XTYP_ADVDATA:
            #ifdef MT_DEBUG_MODE
                TakeDebugNote ("XTYP_ADVDATA\n");
            #endif
            return The_DDE_Client->AdviseData (wFmt, hsz1, hsz2, hData);

        default:
            #ifdef MT_DEBUG_MODE
                TakeDebugNote ("Unknown message\n");
            #endif
            return (HDDEDATA)FALSE;
        }

    return (HDDEDATA)NULL;
    }

#endif  //  End Win32 mode code (pretty much the whole file)

