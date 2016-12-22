//*************************************************************************************
//  OPER_INT.HPP
//      This file contains definitions of structures (or classes) and functions used
//      for the generic character-mode operator interface for DOS or Easy Windows.  
//  
//      The current version has been modified from a regular-C application for Windows 
//      into a basically regular-C application with a C++-style user interface.  
//  
//  Version
//      Original:  copyright 1993, DM Auslander
//      Modified:  1-94    JR  non-C++ version added 
//                 2-94    JR  Screen Manager stuff added 
//                 2-7-94  JR  Went with op_xxxxx names only 
//                 8-94    JR  Ported to Visual C++ DOS/QuickWin
//                 9-94    JR  Version done for Visual C++ running under Windows
//                 12-94   JR  C++ interface added to Windows app. in C version
//                 6-95    JR  Changed from function keys to control keys
//                 10-97   JR  Replaced 'boolean' with 'bool' for compatibility
//*************************************************************************************

#ifndef OPER_INT_HPP
    #define  OPER_INT_HPP           //  Variable to prevent multiple inclusions

#if defined (__WIN32__)             //  If we're compiling a Windows application,
    #include <windows.h>            //  we must include the Windows header file
#endif

#if !defined (BASE_OBJ_HPP)         //  If the base-object header (which contains the
    #include <BASE_OBJ.HPP>         //  definition of a list handling object) hasn't
#endif                              //  yet been included, include it now

#define  TOP_BORDER     3           //  Number of rows in area where window title goes
#define  MAX_ITEMS      20          //  Maximum number of input items or output items
#define  MAX_KEYS       8           //  or keys this program can support

#ifdef OPI_SMALL
    #define  COL_INTITLE    1       //  Define columns in which screen items begin.
    #define  COL_TYPEIN     13      //  These are all character coordinates, not
    #define  COL_INDATA     27      //  pixels.  The "small" version is for use in
    #define  COL_OUTTITLE   38      //  Windows on a VGA screen, so #define the
    #define  COL_OUTDATA    50      //  OPI_SMALL variable if interface is too big 
#else
    #define  COL_INTITLE    1       //  This is the normal version, used for any
    #define  COL_TYPEIN     16      //  screen which can display 25 rows by 80 cols.
    #define  COL_INDATA     33      //  Such as a DOS character-mode screen or a
    #define  COL_OUTTITLE   48      //  super-VGA monitor under most Windows versions
    #define  COL_OUTDATA    63      //
#endif

#define  MAX_INS        20          //  These are the maximum number of input,
#define  MAX_OUTS       20          //  output, and key items allowed
#define  MAX_KEYS       8


//=====================================================================================
//  External Function:  UserMain
//      In a Windows application, there is no main() which can be edited by the user.
//      We want DOS applications to be fully code-compatible with Windows applications
//      here.  So we tell the user to write a function called UserMain() which acts
//      like main() as far as he is concerned; and we write the code which replaces
//      WinMain() or Main() in the program here.  It returns int just like a main().

extern int UserMain (void);


//-------------------------------------------------------------------------------------
//  Structure:  OW_Rect
//      This structure holds 4 numbers describing the boundaries of a rectangle.  It's
//      a replacement for the RECT structure used in the Windows API - because we
//      don't have a definition of that structure in DOS.

#ifndef __WIN32__
typedef struct tagRECT
    {
    int left, top, right, bottom;
    } RECT;
#endif


//=====================================================================================
//  Class:  CDataItem
//      This class represents one input or output item.  Deriving it from CBaseObj
//      makes it easy to keep DataItems in a list.  A DataItem has member functions
//      which allow it to display itself and return its contents and so on.
//=====================================================================================

//  These are the types of data which can be entered and displayed 
enum DataType {DT_INT, DT_LONG, DT_FLOAT, DT_DOUBLE, DT_LONG_DBL, DT_STRING};

//  An item can be of either input or output type.  This enum determines which.  
enum IO_Type {DATA_INPUT, DATA_OUTPUT};

class CDataItem
    {
    private: 
        DataType Type;                  //  Type - int, string, double, whatever
        char* Label;                    //  Label text displayed next to item's value
        void* pData;                    //  Pointer to the data which is being shown
        RECT ScreenRect;                //  Rectangle holds borders of area for item
        IO_Type InOrOut;                //  Is this an input or output item?
        void Data2String (void*,        //  Converts data in numeric format to a string
            char*, DataType);           //  for display
        int NumberInList;               //  Number index of item in the list 

    public:
        CDataItem (DataType, IO_Type,   //  Constructor makes an item of given type
                   const char*, void*,  //  and figures out where it should go
                   int);                //  etc.
        ~CDataItem (void);

        void SetValue (const char*);    //  Set data to equal what user typed
        const char* GetLabel (void);    //  Update display on the screen
        const char* GetData (void);     //  Returns data as a character string
        void Paint (void);              //  Data item displays itself on the screen
    };


//=====================================================================================
//  Class:  CDataItemList 
//      This class is derived from CListObj, which faciliates keeping a list of some
//      objects (in this case, CDataItem's) derived from CBaseObj.  Two data item 
//      lists are kept by an operator window, one for input and one for output items.  
//=====================================================================================

class CDataItemList : public CBasicList
    {
    private: 
        IO_Type InOrOut;                            //  Is this input or output item? 
        int ListIndex;                              //  Counts elements in the list 

    public:
        CDataItemList (IO_Type);
        ~CDataItemList (void);

        void Insert (DataType, const char*, void*); //  Create an item and put in list
        bool PaintOne (void);                       //  Paint one item in the list
        void PaintAll (void);                       //  or paint the whole list full 
    };


//=====================================================================================
//  Class:  CKeyItem
//      This class represents a key which can be pressed by the user.  It contains a
//      label to be displayed at the bottom of the screen which shows what the key 
//      does and a pointer to the function which will run when the key is pressed.  
//=====================================================================================

class CKeyItem 
    {
    private:
        int KeyChar;                    //  Character which activates key's function 
        void (*KeyFunction)();          //  Function which runs when key's pressed
        int SerialNumber;               //  Unique number for each key item

    public:
        char line1[16];                 //  First line of text to be displayed
        char line2[16];                 //  Second line of key label text

        //  Constructor takes key number, 2 label lines, and pointer to key function
        CKeyItem (int, const char*, const char*, void (*)(), int);
        ~CKeyItem (void);

        void RunKeyFunction (void);     //  Run the function in response to key press
        int GetKeyChar (void)           //  Function to return the number of the key
            { return KeyChar; }         //    associated with this item
        void Paint (void);              //  Function draws key definition on screen 
    };


//=====================================================================================
//  Class:  CKeyItemList 
//      This class keeps a list of the above defined key mapping items.  
//=====================================================================================

class CKeyItemList : public CBasicList 
    {
    public:
        CKeyItemList (void);
        ~CKeyItemList (void);

        void Insert (int, const char*, const char*, void (*)(void), int);
    };


//=====================================================================================
//  Class:  COperatorWindow 
//      This class holds together all the stuff needed to run an operator window.  
//      etc. 
//=====================================================================================

class COperatorWindow
    {
    private:
        CDataItemList* InputItems;      //  List object holding input data items
        CDataItemList* OutputItems;     //  List of output-only items
        CKeyItemList* KeyItems;         //  List of key mappings
        char* Title;                    //  Title of window 
        int InputIndex;                 //  Currently selected character in buffer 

        void SaveCurrentInput (void);   //  Save user input into highlighted data field
        void ShowAll (void);            //  Tells items in window to show themselves
        int KeyItemSerialNumber;        //  Gives unique numbers to key items  
        #if defined (_WINDOWS)          //  These functions are only needed in Windows:
            void ReadChar (MSG*);       //  Read a character the user typed into buffer
            void KeyPress (MSG*);       //  Check what kind of key the user pressed
        #endif

    public:
        char* InputBuffer;              //  Buffer for storing user's input 

        COperatorWindow (const char*);  //  Constructor assigns a window title 
        ~COperatorWindow (void);        //  Destructor, uh...  destructs stuff 

        //  A whole bunch of overloaded methods allow user to add input and output
        //  items whose data is of various types.  
        void AddInputItem (const char*, int*);
        void AddInputItem (const char*, long*);
        void AddInputItem (const char*, float*);
        void AddInputItem (const char*, double*);
        void AddInputItem (const char*, long double*);
        void AddInputItem (const char*, char*);
        void AddOutputItem (const char*, int*);
        void AddOutputItem (const char*, long*);
        void AddOutputItem (const char*, float*);
        void AddOutputItem (const char*, double*);
        void AddOutputItem (const char*, long double*);
        void AddOutputItem (const char*, char*);

        //  Here's the function we call to add a user key to the list
        void AddKey (int, const char*, const char*, void (*)(void));

        void Paint (void);              //  Function to repaint window when uncovered
        void Display (void);            //  Call this to repaint entire window
        void CheckKeyboard (void);      //  The part of Update() which processes keys
        void Update (void);             //  Incremental, low-latency repainting
        void Close (void);              //  Stop displaying it, but keep it around

        friend class CWindowState;      //  This state needs access to item lists
    };

#if defined (TRANRUN4_HPP)

//=====================================================================================
//  Class:  CWindowState
//      This class encapsulates a transition logic state which displays an operator
//      window.  In a given state, a given operator window interface is displayed.
//      The design strategy is to use one window state of this type for each operator
//      window which needs to be displayed.  This class only has meaning in a project
//      which has both operator windows and the TranRun scheduler, so only include it
//      if the flag _TRANRUN_ has been defined.

class CWindowState : public CState
    {
    private:
        COperatorWindow* pOpWin;

    public:
        CWindowState (char*);               //  Give window title to the constructor
        void Entry (void);                  //  Get the window painted at first
        void Action (void);                 //  Update the window periodically
        void Close (void);                  //  Tell operator window to close itself

        //  A bunch of overloaded methods to add input and output items whose data is
        //  of various types; these methods follow those in COperatorWindow
        void AddInputItem (const char*, int*);
        void AddInputItem (const char*, long*);
        void AddInputItem (const char*, float*);
        void AddInputItem (const char*, double*);
        void AddInputItem (const char*, long double*);
        void AddInputItem (const char*, char*);
        void AddOutputItem (const char*, int*);
        void AddOutputItem (const char*, long*);
        void AddOutputItem (const char*, float*);
        void AddOutputItem (const char*, double*);
        void AddOutputItem (const char*, long double*);
        void AddOutputItem (const char*, char*);

        //  Here's the function we call to add a user key to the list
        void AddKey (int, const char*, const char*, void (*)(void));
    };

#endif     //  End of stuff which is only included if TRANRUN4_HPP is defined

//-------------------------------------------------------------------------------------
//  Global variables
//      CurrentOpWin:   At any given time, there is one currently running operator
//                      window.  This pointer is to point to that window object.  This
//                      allows the object to be given messages by Windows when some-
//                      thing happens like a key press, etc.  If this variable is NULL
//                      no operator window has been set up, or it's turned off.
//      SelectedInput:  Just one item in the input item list may be selected at any
//                      given time.  This pointer points to the selected one.

extern COperatorWindow* CurrentOpWin;
extern CDataItem* SelectedInput;

#endif  //  OPER_INT_HPP - Multiple inclusion protection

