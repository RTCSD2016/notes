//*************************************************************************************
//  OPER_INT.CPP
//      This file contains definitions of classes and functions used the generic
//      character-mode real-time operator interface.
//
//      The current version has been modified from a regular-C application for Windows
//      into a C++-style program.  It is designed to use control keys rather than
//      function keys so that it will be as portable as possible between platforms.
//
//      FILES:  To make an operator window package you need this file and its
//              associated HPP file.  It is designed to fit neatly with the TranRun3
//              or CTL_EXEC scheduler packages; these are necessary for compilation
//              whether or not the scheduler is to be used.  See TranRun4.hpp for a
//              list of all the header files which need to be made available.
//  
//  Version
//      Original:  copyright 1993, DM Auslander
//      Modified:  1-94    JR  non-C++ version added 
//                 2-94    JR  Screen Manager stuff added 
//                 2-7-94  JR  Went with op_xxxxx names only 
//                 8-94    JR  Ported to Visual C++ DOS/QuickWin
//                 9-94    JR  Version done for Visual C++ running under Windows
//                 12-94   JR  C++ interface added to Windows app. in C version
//                 1-95    JR  Class structure modified, file OPER_WIN.CPP split up
//                 6-95    JR  Modified DOS version into EasyWin version; changed
//                             use of function keys (F1-F8) to use of control keys
//                 9-95    JR  Merged contents of OPI_EZWN and OPI_MAIN to this file
//                10-95    JR  Eliminated TranRun3 dependency; now uses BASE_OBJ only
//                 8-96    JR  Changed delete calls for arrays to DELETE_ARRAY macro
//                10-96   DMA  Added SelectedInput line in Display()
//                 8-97    JR  Added code to run in console from WinNT GUI application
//*************************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#if defined (__WIN32__) && !defined (__CONSOLE__)
    #include <windows.h>
    extern HANDLE hStdin, hStdout;          //  Handles to console's standard I/O
    extern HANDLE hSchedulerWindow;         //  Handle of scheduler's GUI window
    DWORD dDummy;                           //  Holds unused information
    COORD conxy;                            //  Coordinates of text on console
    #define  OPI_print(x)  WriteConsole (hStdout, (x), strlen (x), &dDummy, NULL);
    #define  gotoxy(x,y)   conxy.X = (x); conxy.Y = ((y) - 1); \
                           SetConsoleCursorPosition (hStdout, conxy)
    #define  clrscr()      //  There's no convenient replacement for consoles

    //---------------------------------------------------------------------------------
    //  Function:  kbhit
    //      This function must be redefined for use with a console which has been
    //      added to a Windows GUI application.  It checks for the number of console
    //      input events (these may be key events) which are ready to be read.
    
    bool kbhit (void)
        {
        DWORD NumEvents;                    //  Number of console events not yet read

        GetNumberOfConsoleInputEvents (hStdin, &NumEvents);
        if (NumEvents > 0) return true;
        else               return false;
        }
        
    //---------------------------------------------------------------------------------
    //  Function:  getch
    //      This function must also be redefined for use with a console.  It reads a
    //      console input event.  If the event is a key press, it returns the ASCII
    //      code of the key which was pressed; if not it discards the event. 

    int getch (void)
        {
        INPUT_RECORD EventBuffer;           //  Holds a console input event record
        KEY_EVENT_RECORD KeyEvent;          //  Holds a record of what key was pressed

        if (ReadConsoleInput (hStdin, &EventBuffer, 1, &dDummy) == false)
            MessageBox (hSchedulerWindow, "Unable to read console input",
                        "Interface Error", MB_OK | MB_ICONSTOP);

        //  If the event isn't a key event, ignore it
        if (EventBuffer.EventType != KEY_EVENT) return 0;

        //  Get key event record; make sure it's a key down, not a key up
        KeyEvent = (KEY_EVENT_RECORD)EventBuffer.Event.KeyEvent;
        if (KeyEvent.bKeyDown == false) return 0;

        return ((int)(KeyEvent.uChar.AsciiChar));
        }
#else
    #include <conio.h>                      //  kbhit() and getch() are defined here
    #include <dos.h>
    #define  OPI_print(x)  printf ("%s", (x))
#endif

#include <oper_int.hpp>                     //  This file #includes <base_obj.hpp>


//=====================================================================================
//  Global Variables
//      Those variables which absolutely have to be made global are instantiated here.

COperatorWindow* CurrentOpWin = NULL;   //  Pointer to operator window in use
CDataItem* SelectedInput = NULL;        //  Pointer to selected screen input item
static int NeedToDrawKeys = 0;          //  Tells Update() to write key legend
static int NeedToDrawTitle = 0;         //  Tells Update() it must write title


//=====================================================================================
//  Class:  CDataItem
//      This class represents one input or output item.  Deriving it from CBaseObj 
//      makes it easy to keep DataItems in a list.  A DataItem has member functions 
//      which allow it to update and return its contents and label.  It can also print 
//      itself on the screen; the code for that is in operating environment specific 
//      files, though, not here.  
//=====================================================================================

//-------------------------------------------------------------------------------------
//  Constructor:  CDataItem
//      This constructor creates a CDataItem object.  The parameters we have to give
//      it are:  type of data, input or output item, label text, pointer to data, and
//      the number of this item in the list (which determines Y coordinate on screen).

CDataItem::CDataItem (DataType aType, IO_Type aIO, const char* aLabel, void* aData,
    int aNum)
    {
    Label = new char[128];              //  Allocate string for item label
    strcpy (Label, aLabel);             //  Copy the data's name into this string
    Type = aType;                       //  Set data type
    pData = aData;                      //  Set pointer to data, whatever data it is
    InOrOut = aIO;                      //  Save choice of input or output item
    NumberInList = aNum;                //  Save number in the list of items

    //  Get the coordinates of the rectangle in which to display the item.  The Y
    //  coordinates depend on the number of the item in the list (static member data
    //  called NumberInList holds this).  This is for character, not pixel, mode
    ScreenRect.left = (InOrOut == DATA_INPUT) ? COL_INTITLE : COL_OUTTITLE;
    ScreenRect.top = (NumberInList + TOP_BORDER);
    ScreenRect.right = (InOrOut == DATA_INPUT) ? COL_OUTTITLE : 80;
    ScreenRect.bottom = ScreenRect.top + 1;
    }


//-------------------------------------------------------------------------------------
//  Destructor:  ~CDataItem
//      This destructor is here to free up the memory used by the CDataItem object.

CDataItem::~CDataItem (void)
    {
    DELETE_ARRAY Label;
    }


//-------------------------------------------------------------------------------------
//  Function:  SetValue
//      When the user has typed in a string, the value of the data item must be set
//      to equal whatever the data is in the string.  Here's a function to do that:
//      it's sort of an assignment, pData = (what's in UserString).

void CDataItem::SetValue (const char *UserString)
    {
    switch (Type)
        {
        case DT_INT:
            sscanf (UserString, "%d", (int*)pData);
            break;
        case DT_LONG:
            sscanf (UserString, "%ld", (long*)pData);
            break;
        case DT_FLOAT:
            sscanf (UserString, "%g", (float*)pData);
            break;
        case DT_DOUBLE:
            sscanf (UserString, "%lg", (double*)pData);
            break;
        case DT_LONG_DBL:
            sscanf (UserString, "%Lg", (long double*)pData);
            break; 
        case DT_STRING:
            if (strlen (UserString) > 0)
                strcpy ((char*)pData, UserString);
            break;
        default:
            break;
        }
    }


//-------------------------------------------------------------------------------------
//  Function:  GetLabel 
//      This function returns a pointer to the character string containing the label 
//      of this data item.  

const char* CDataItem::GetLabel (void)
    {
    return (Label);
    }


//-------------------------------------------------------------------------------------
//  Function:  GetData 
//      This function returns the data of the item in the form of a character string, 
//      which makes it easier for the COperatorWindow to display it on the screen.  As 
//      the pointer returned points to a static character array, the data will be 
//      valid until the next time this function is called.  

const char* CDataItem::GetData (void)
    {
    static char aString[48];        //  Allocate ample space for the returned string 

    //  Call function to convert data from its native form into a string 
    Data2String (pData, aString, Type); 

    return (aString);
    }


//-------------------------------------------------------------------------------------
//  Function:  Data2String
//      In order to facilitate the display of numbers and such on the screen, this 
//      function converts numbers or whatever into character strings.  

void CDataItem::Data2String (void* data, char* string, DataType aType)
    {
    switch (aType)
        {
        case DT_INT:
            sprintf (string, "%d", *(int*)data);
            break;
        case DT_LONG:
            sprintf (string, "%ld", *(long*)data);
            break;
        case DT_FLOAT:
            sprintf (string, "%g", (double)(*(float*)data));
            break;
        case DT_DOUBLE:
            sprintf (string, "%lg", *(double*)data);
            break;
        case DT_LONG_DBL:
            sprintf (string, "%Lg", *(long double*)data);
            break;
        case DT_STRING:
            sprintf (string, "%s", (char*)data);
            break;
        default:
            sprintf (string, "**Data Error**");
            break;
        }
    }


//-------------------------------------------------------------------------------------
//  Function:  Paint
//      This function displays the data item on the screen.  It's to be called by some
//      function such as Paint() or Update().

void CDataItem::Paint (void)
    {
    int Twid;                                       //  Width of text area on screen
    int Length;                                     //  Length of a text string
    char OutString[128];                            //  Stores text to be shown
    int X;                                          //  Stores X coord., other stuff


    //  Make sure there's a current operator window active in which to paint
    if (CurrentOpWin == NULL)
        return;

    //  If this is the input item which is selected, write label in white on gray
    if (InOrOut == DATA_INPUT)
        {
        //  Begin by filling the OutString string with the data label, plus extra
        //  spaces so that it erases anything which might be in the typein area
        strcpy (OutString, GetLabel ());
        Twid = COL_INDATA - COL_INTITLE;
        Length = strlen (OutString);
        while (Length++ < Twid)
          strcat (OutString, " ");

        if (this == SelectedInput)                      //  If this is the input
            {                                           //  which accepts type-ins,
            gotoxy (COL_INTITLE, ScreenRect.top);       //  signify it with a little
            OPI_print (OutString);                      //  arrow "->" to the left of
            gotoxy (COL_TYPEIN, ScreenRect.top);        //  the type-in area; it
            OPI_print ("->");                           //  looks kinda like a prompt
            OPI_print (CurrentOpWin->InputBuffer);
            }
        else                                            //  Else it's not highlighted,
            {                                           //  so write the label alone
            gotoxy (COL_INTITLE, ScreenRect.top);
            OPI_print (OutString);
            }
        }
    else                                                //  This else means it's
        {                                               //  not an input item but
        gotoxy (COL_OUTTITLE, ScreenRect.top);          //  and output item; the
        OPI_print (GetLabel ());                        //  label goes over here
        }
    //  Now display the data for the item in the appropriate column
    strcpy (OutString, GetData ());

    //  Pad length: 'Twid' is length of string which just fills the data area, and
    //  'Length' is length of the output string.  Pad output string until it's just
    //  long enough to fill the data area
    Twid = (InOrOut == DATA_INPUT) ? (COL_OUTTITLE - COL_INDATA) : (80 - COL_OUTDATA);
    Length = strlen (OutString);
    while (Length++ < Twid)
        strcat (OutString, " ");
    X = (InOrOut == DATA_INPUT) ? (COL_INDATA) : (COL_OUTDATA);
    gotoxy (X, ScreenRect.top);
    OPI_print (OutString);
    }


//=====================================================================================
//  Class:  CDataItemList
//      This class is derived from CListObj, which faciliates keeping a list of some
//      objects (in this case, CDataItem's) derived from CBaseObj.  Two data item
//      lists are kept by an operator window, one for input and one for output items.
//=====================================================================================

//-------------------------------------------------------------------------------------
//  Constructor:  CDataItemList
//      Here we save the type of list, input or output; also zero initialize index

CDataItemList::CDataItemList (IO_Type aIO_Type)
    {
    InOrOut = aIO_Type;
    ListIndex = 1;                          //  This index determines Y coord. of item
    }


//-------------------------------------------------------------------------------------
//  Destructor:  ~CDataItemList
//      This destructor is here to free up the memory used by the CDataItemList.  

CDataItemList::~CDataItemList (void)
    {
    CDataItem* pCurItem;                    //  These pointers are to step through
    CDataItem* pTemp;                       //  the list, deleting the objects we find


    pCurItem = (CDataItem*) GetHead ();     //  Start at first one and keep deleting
    while (pCurItem != NULL)
        {
        pTemp = pCurItem;
        pCurItem = (CDataItem*) GetNext (); 
        delete (pTemp);
        }
    }

        
//-------------------------------------------------------------------------------------
//  Function:  Insert 
//      This function creates a new object of type CDataItem and puts that item into 
//      the CDataItem list.  

void CDataItemList::Insert (DataType aType, const char* aName, void* apData)
    {
    CDataItem* pNew;                        //  Pointer to give our new creation life

    //  Before bothering with calculations, make sure there's room in the list.  If 
    //  there is, create the object and put a pointer to it in the list 
    if (HowMany () < MAX_ITEMS)
        {
        pNew = new CDataItem (aType, InOrOut, aName, apData, ListIndex++);
        CBasicList::Insert ((void*)pNew);
        }
    }


//-------------------------------------------------------------------------------------
//  Function:  PaintOne 
//      This function repaints a single item in the list.  It uses the current-item 
//      pointer to choose an item (beware of other functions moving the pointer).  If 
//      we have come to the end of the list, it returns FALSE so the calling program 
//      knows when to change over display to the other list.  

bool CDataItemList::PaintOne (void)
    {
    static CDataItem* pShowIt = NULL;       //  Pointer to item being shown now


    //  Current-item pointer points to the item to be displayed.  If we're at the end,
    //  the pointer is NULL, so move to the beginning of the list and don't draw any-
    //  thing, just return FALSE so calling program will draw from the other list.
    pShowIt = (CDataItem*) GetCurrent ();

    //  Here's where we call the function to actually draw the thing
    if (pShowIt != NULL)
        pShowIt->Paint ();

    //  Move the item-to-draw pointer to the next item in the list
    if ((pShowIt = (CDataItem*) GetNext ()) == NULL)
        {
        GetHead ();
        return (FALSE);
        }

    return (TRUE);
    }


//=====================================================================================
//  Class:  CKeyItem
//      This class represents a key which can be pressed by the user.  It contains a 
//      label to be displayed at the bottom of the screen which shows what the key 
//      does and a pointer to the function which will run when the key is pressed.  
//=====================================================================================

//-------------------------------------------------------------------------------------
//  Constructor:  CKeyItem 
//      This constructor creates a key item with labels and a key function it can run.  

CKeyItem::CKeyItem (int aKeyChar, const char* aLab1, const char* aLab2,
                    void (*aFunc)(), int aSerialNumber)
    {
    KeyChar = aKeyChar;
    strcpy (line1, aLab1);
    strcpy (line2, aLab2);
    KeyFunction = aFunc;
    SerialNumber = aSerialNumber;
    }


//-------------------------------------------------------------------------------------
//  Destructor:  ~CKeyItem 
//      This destructor is here to free up the memory used by the CDataItem object.  

CKeyItem::~CKeyItem (void)
    {
    }


//-------------------------------------------------------------------------------------
//  Function:  RunKeyFunction 
//      This function simply runs the function whose address has been provided.  

void CKeyItem::RunKeyFunction (void)
    {
    KeyFunction ();
    }


//-------------------------------------------------------------------------------------
//  Function:  Paint
//      This function displays the key item information in the proper space near the
//      bottom of the screen.  In order to most correctly imitate the DOS version, it
//      puts the data on the same lines - numbers 23, 24, and 25.

void CKeyItem::Paint (void)
    {
    int X;                          //  Coordinates for displaying key item on screen
    static char OutString[32];      //  Temporary storage for text to be displayed


    X = (SerialNumber * 10) + 1;
    sprintf (OutString, "Ctrl-%c", KeyChar);
    gotoxy (X, 23);
    OPI_print (OutString);
    gotoxy (X, 24);
    OPI_print (line1);
    gotoxy (X, 25);
    OPI_print (line2);
    }


//=====================================================================================
//  Class:  CKeyItemList 
//      This class just implements a list of key redefinitions.  
//=====================================================================================

//-------------------------------------------------------------------------------------
//  Constructor:  CKeyItemList 
//      This no-parameters constructor does very little, if anything, at all.  

CKeyItemList::CKeyItemList (void)
    {
    }


//-------------------------------------------------------------------------------------
//  Destructor:  ~CKeyItemList 
//      This destructor is here to free up the memory used by the CDataItem object.  

CKeyItemList::~CKeyItemList (void)
    {
    CKeyItem* pCurItem;                     //  These pointers are to step through
    CKeyItem* pTemp;                        //  the list, deleting the objects we find


    pCurItem = (CKeyItem*) GetHead ();      //  Start at first one and keep deleting
    while (pCurItem != NULL)
        {
        pTemp = pCurItem;
        pCurItem = (CKeyItem*) GetNext (); 
        delete (pTemp);
        }
    }


//-------------------------------------------------------------------------------------
//  Function:  Insert 
//      This function adds a key item to the key item list.  The item gets added at 
//      the end of the list.  

void CKeyItemList::Insert (int aKeyNum, const char* aLab1, const char* aLab2,
                           void (*aFunc)(), int aSerialNumber)
    {
    CKeyItem* pNew;


    //  Call constructor to create a new key mapping item 
    pNew = new CKeyItem (aKeyNum, aLab1, aLab2, aFunc, aSerialNumber++);
    CBasicList::Insert ((void*)pNew);
    }


//=====================================================================================
//  Class:  OperatorWindow 
//      This class holds together all the stuff needed to run an operator window.  
//=====================================================================================

//-------------------------------------------------------------------------------------
//  Constructor:  COperatorWindow
//      Here we create a new operator window, complete with an input buffer but no 
//      input or output items.  Those items are added by the user later.  

COperatorWindow::COperatorWindow (const char* aTitle)
    {
    Title = new char[128];                  //  Make some space for the title
    strncpy (Title, aTitle, 126);           //  and put the given text into it

    //  Call the constructors to create lists of input items, output items, and keys
    InputItems = new CDataItemList (DATA_INPUT);
    OutputItems = new CDataItemList (DATA_OUTPUT);
    KeyItems = new CKeyItemList ();

    InputBuffer = new char[64];             //  Allocate buffer for user input 
    InputBuffer[0] = '\0';                  //  Set it to the null string 
    InputIndex = 0;                         //  Set character-accessed index to 0
    SelectedInput = NULL;                   //  There's no input item to point to yet 
    KeyItemSerialNumber = 0;                //  Used to give each key its own number
    }


//-------------------------------------------------------------------------------------
//  Destructor:  ~COperatorWindow 
//      This destructor is here to free up the memory used by the COperatorWindow.  It 
//      also closes the operator window, setting the current-op-win pointer to NULL and
//      clearing the screen (in DOS mode).  

COperatorWindow::~COperatorWindow (void)
    {
    Close ();

    DELETE_ARRAY Title;
    delete (InputItems);
    delete (OutputItems);
    delete (KeyItems);
    DELETE_ARRAY InputBuffer;
    }


//-------------------------------------------------------------------------------------
//  Function:  AddInputItem
//      This function places an input item into the operator window's list.  There are
//      several overloaded versions for different types of input pointers:  DT_INT,
//      DT_LONG, DT_FLOAT, DT_DOUBLE, DT_LONG_DBL, and DT_STRING

void COperatorWindow::AddInputItem (const char* aLabel, int* aData)
    {
    InputItems->Insert (DT_INT, aLabel, (void*)aData);

    //  If there's no input item selected, this one's the first; set pointer to it
    SelectedInput = (CDataItem*) InputItems->GetHead ();
    }

void COperatorWindow::AddInputItem (const char* aLabel, long* aData)
    {
    InputItems->Insert (DT_LONG, aLabel, (void*)aData);
    SelectedInput = (CDataItem*) InputItems->GetHead ();
    }

void COperatorWindow::AddInputItem (const char* aLabel, float* aData)
    {
    InputItems->Insert (DT_FLOAT, aLabel, (void*)aData);
    SelectedInput = (CDataItem*) InputItems->GetHead ();
    }

void COperatorWindow::AddInputItem (const char* aLabel, double* aData)
    {
    InputItems->Insert (DT_DOUBLE, aLabel, (void*)aData);
    SelectedInput = (CDataItem*) InputItems->GetHead ();
    }

void COperatorWindow::AddInputItem (const char* aLabel, long double* aData)
    {
    InputItems->Insert (DT_LONG_DBL, aLabel, (void*)aData);
    SelectedInput = (CDataItem*) InputItems->GetHead ();
    }

void COperatorWindow::AddInputItem (const char* aLabel, char* aData)
    {
    InputItems->Insert (DT_STRING, aLabel, (void*)aData);
    SelectedInput = (CDataItem*) InputItems->GetHead ();
    }


//-------------------------------------------------------------------------------------
//  Function:  AddOutputItem 
//      This function puts an output item into the operator window's list.  It's over-
//      loaded just like the AddInputItem() methods for the same reason.  

void COperatorWindow::AddOutputItem (const char* aLabel, int* aData)
    {
    OutputItems->Insert (DT_INT, aLabel, (void*)aData);
    }

void COperatorWindow::AddOutputItem (const char* aLabel, long* aData)
    {
    OutputItems->Insert (DT_LONG, aLabel, (void*)aData);
    }

void COperatorWindow::AddOutputItem (const char* aLabel, float* aData)
    {
    OutputItems->Insert (DT_FLOAT, aLabel, (void*)aData);
    }

void COperatorWindow::AddOutputItem (const char* aLabel, double* aData)
    {
    OutputItems->Insert (DT_DOUBLE, aLabel, (void*)aData);
    }

void COperatorWindow::AddOutputItem (const char* aLabel, long double* aData)
    {
    OutputItems->Insert (DT_LONG_DBL, aLabel, (void*)aData);
    }

void COperatorWindow::AddOutputItem (const char* aLabel, char* aData)
    {
    OutputItems->Insert (DT_STRING, aLabel, (void*)aData);
    }


//-------------------------------------------------------------------------------------
//  Function:  AddKey
//      This function places the given key item into the operator window's list of key
//      maps.

void COperatorWindow::AddKey (int aChar, const char* aLn1, const char* aLn2,
                              void (*aFunc)(void))
    {
    //  Check the key item to make sure it's not an invalid item - some characters,
    //  such as ^C, ^H, etc. have special meaning on many operating systems.  If your
    //  OS defines other special characters, you can add them to this list.
    switch (aChar)
        {
        case 'C':                       //  Control-C is kill current process
        case 'H':                       //  Control-H is a backspace
        case 'I':                       //  Control-I is a tab character
        case 'J':                       //  Control-J is a linefeed character
        case 'M':                       //  Control-M is a carriage return
            gotoxy (2, 2);
            char Err[3];
            sprintf (Err, "ERROR:  Cannot use key Ctrl-%c in user interface", aChar);
            OPI_print (Err);
            exit (2);
        }

    //  OK, the key is legitimate so insert it in the list
    KeyItems->Insert (aChar, aLn1, aLn2, aFunc, KeyItemSerialNumber++);
    }


//-------------------------------------------------------------------------------------
//  Function:  ShowAll
//      This function tells all the input, output, and key items in the lists to show 
//      themselves.  It's called by Display().  

void COperatorWindow::ShowAll (void)
    {
    CKeyItem* pKeyItem;             //  Points to a key item which must be displayed 


    //  Make sure that this operator window will be displayed, not some other one
    CurrentOpWin = this;

    //  Display all input items with titles in columns on the left half of the screen 
    InputItems->GetHead ();
    while (InputItems->PaintOne () == TRUE);

    //  Display the output items and titles in columns on the right half of the screen
    OutputItems->GetHead ();
    while (OutputItems->PaintOne () == TRUE);

    //  Display all the key items in the list
    pKeyItem = (CKeyItem*) KeyItems->GetHead ();
    while (pKeyItem != NULL)
        {
        pKeyItem->Paint ();
        pKeyItem = (CKeyItem*) KeyItems->GetNext();
        }
    }


//-------------------------------------------------------------------------------------
//  Function:  Display
//      This function is the one which the user calls when he wants to display the
//      operator window for the first time after he's constructed it.  It clears the
//      whole screen and paints on the new operator window.

void COperatorWindow::Display (void)
    {
    NeedToDrawKeys = 1;                     //  Set these flags so that key functions
    NeedToDrawTitle = 1;                    //  and title are drawn first by Update()

    //  Set the chosen-input-item pointer to the first element in the list
    //  Changed 10/96 by DMA
    SelectedInput = (CDataItem*)InputItems->GetHead ();

    CurrentOpWin = this;                    //  Set current-window pointer to this one
    clrscr ();                              //  Clear anything which was there off

#ifndef _Windows
    _setcursortype (_NOCURSOR);             //  turn the cursor off
#endif
    }


//-------------------------------------------------------------------------------------
//  Function:  Close
//      This function causes the operator window to stop displaying itself, but it
//      still remains in memory, ready to show itself if the user needs it.

void COperatorWindow::Close (void)
    {
    //  Set pointer-to-current-OpWin to NULL, meaning don't show this one
    CurrentOpWin = NULL;
//    clrscr ();

#ifndef _Windows                            //  If not running EasyWin,
    _setcursortype (_NORMALCURSOR);         //  turn the cursor back on again
#endif
    }


//-------------------------------------------------------------------------------------
//  Function:  CheckKeyboard
//      When the Update() function (see below) runs, it repaints part of the screen
//      and then checks the keyboard for input.  The keyboard checking stuff is here
//      - just to make the program easier to read.

void COperatorWindow::CheckKeyboard (void)
    {
    int InputChar;                          //  Character read from the keyboard
    CKeyItem* pKey;                         //  Pointer to key mapping object


    //  Check for a keyboard hit.  If there's been one, see what kind of key it was,
    //  and act appropriately
    if (kbhit ())
        {
        InputChar = getch ();

        //  Somebody hit the TAB key.  Save data and move the selection down, and if
        //  it's at the last item, wrap back to the first one
        if (InputChar == '\t')
            {
            if (SelectedInput != NULL)
                {
                SelectedInput->SetValue (InputBuffer);
                InputItems->GetObjWith (SelectedInput);
                if (InputItems->GetNext () != NULL)
                    SelectedInput = (CDataItem*) InputItems->GetCurrent ();
                else
                    {
                    if (InputItems->GetHead () != NULL);
                        SelectedInput = (CDataItem*) InputItems->GetHead ();
                    }
                InputIndex = 0;
                InputBuffer[0] = '\0';
                }
            }

        //  This stuff runs if someone presses the accent (`) key.  It moves the
        //  selection up after saving data - like a reverse-tab.  By the way, the
        //  ASCII number for this thing is 96.
        if (InputChar == '`')
            {
            if (SelectedInput != NULL)
                {
                SelectedInput->SetValue (InputBuffer);
                InputItems->GetObjWith (SelectedInput);
                if (InputItems->GetPrevious () != NULL)
                    SelectedInput = (CDataItem*) InputItems->GetCurrent ();
                else
                    {
                    if (InputItems->GetTail () != NULL);
                        SelectedInput = (CDataItem*) InputItems->GetTail ();
                    }
                InputIndex = 0;
                InputBuffer[0] = '\0';
                }
            }

        //  A RETURN character causes the input buffer to be processed
        else if (InputChar == '\n' || InputChar == '\r')
            {
            if (SelectedInput != NULL)
                {
                SelectedInput->SetValue (InputBuffer);
                InputIndex = 0;
                InputBuffer[0] = '\0';
                }
            }

        //  A backspace key causes the last character in buffer to be deleted
        else if (InputChar == 8)
            {
            InputIndex--;
            InputBuffer[InputIndex] = '\0';
            }

        //  If someone pressed a function key Ctrl-A to Ctrl-Z, scan through function
        //  key mappings until we find one corresponding to the key just pressed
        else if (InputChar < 27)
            {
            pKey = (CKeyItem *) KeyItems->GetHead ();
            while (pKey != NULL)
                {
                int WhichChar = pKey->GetKeyChar ();
                if (WhichChar == (InputChar + 64))
                    {
                    pKey->RunKeyFunction ();
                    break;
                    }
                pKey = (CKeyItem*) KeyItems->GetNext ();
                }
            }

        //  Default means it's just an ordinary alphanumeric key; save it
        else
            {
            if (SelectedInput != NULL)
                {
                InputBuffer[InputIndex++] = (char)InputChar;
                InputBuffer[InputIndex] = '\0';
                }
            }
        }
    }


//-------------------------------------------------------------------------------------
//  Function:  Update
//      This function performs a real-time partial update of the operator window.  One
//      screen item (and only one) is scanned and, if need be, redrawn.  We scan just
//      one item at a time to reduce the latency associated with screen stuff.  The
//      versions of this function for DOS and Windows are different.  The DOS one here
//      checks for user input at the keyboard.

void COperatorWindow::Update (void)
    {
    static bool ToggleList = TRUE;          //  True if showing input items now
    CKeyItem* pKeyItem;                     //  Points to one key item to be displayed


    //  Call the function above to take care of any keys the user might have pressed
    CheckKeyboard ();

    //  If the key item descriptions or the window title need to be drawn, do so now
    //  and return having done so.  This reduces the latency of calling Display()
    if (NeedToDrawKeys != 0)
        {
        //  Display all the key item legends
        pKeyItem = (CKeyItem*) KeyItems->GetHead ();
        while (pKeyItem != NULL)
            {
            pKeyItem->Paint ();
            pKeyItem = (CKeyItem*) KeyItems->GetNext ();
            }

        NeedToDrawKeys = 0;
        return;
        }

    //  If the title-drawing flag is set, display the operator window title in its
    //  position near top of the screen
    if (NeedToDrawTitle != 0)
        {
        gotoxy (2, 2);
        OPI_print (Title);

        NeedToDrawTitle = 0;
        return;
        }

    //  Repaint one input item.  If we're currently working from the input list, check
    //  to see if we're that the end of that list (The PaintOne() method will return
    //  FALSE at end of list); if at the end, set the toggle so next time we'll begin
    //  going through the output items list.  And vice versa of course.
    if (ToggleList == TRUE)
        {
        if (InputItems->PaintOne () == FALSE)
            ToggleList = FALSE;
        }
    else
        {
        if (OutputItems->PaintOne () == FALSE)
            ToggleList = TRUE;
        }

    }


//-------------------------------------------------------------------------------------
//  Function:  Paint
//      This function isn't needed in the DOS version.  It gets called in the Windows
//      version whenever the window needs repainting - for example, when the function
//      InvalidateRect() has sent a WM_PAINT message to the application - and that
//      doesn't happen in a DOS app.  So, this is just a dummy function.

void COperatorWindow::Paint (void)
    {
    }

