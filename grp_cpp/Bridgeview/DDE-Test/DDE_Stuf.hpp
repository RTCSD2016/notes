//*************************************************************************************
//  DDE_Stuf.hpp
//      This file contains the class definition for DDE handler objects.  These objects
//      encapsulate a DDE communication interface which allows C++ programs to talk
//      to each other or to applications such as Excel.
//
//  Classes
//      C_DDE_Item   - An item of data maintained by one of a server's topics
//      C_DDE_Topic  - A topic which is to be serviced by a DDE server
//      C_DDE_Server - A DDE server which services a bunch of topics and items
//      C_DDE_Client - A DDE client object with topics and items and such
//
//  Revisions
//       8-16-97  JRR  Original file
//       9-12-97  JRR  Merged client and server files into one big mess
//*************************************************************************************

#ifdef __WIN32__                            //  Only compile this for Win95/NT apps.
#ifndef BASE_OBJ_HPP                        //  Someone must #include the header for
    #include <base_obj.hpp>                 //  base objects before this file
#endif
#ifndef DDE_STUFF                           //  This section protects the header from
    #define DDE_STUFF                       //  being included twice in a source file

//  Define a code-page variable so that someone could change the code page if needed
#define CP_DDESERV  CP_WINANSI

//  Define a text style to be used for drawing text in the DDE server's GUI window
#define DT_DDESTYLE  \
        (DT_LEFT | DT_EXTERNALLEADING | DT_SINGLELINE | DT_NOCLIP | DT_NOPREFIX)

//  How many milliseconds can a DDE transaction take before it's considered too slow
const int DDE_SYNCH_TIMEOUT = 1000;

//  This enumeration defines the types of data which can be kept by the DDE server
enum DDE_Data_Type {DDT_int, DDT_uint, DDT_long, DDT_ulong, DDT_float, DDT_double,
                    DDT_long_double, DDT_char, DDT_string};

//  Prototypes for the nonmember functions
HWND Create_DDE_Window (const char*);       //  Function creates a GUI window for DDE
void DDE_CheckForError (DWORD);             //  Function checks for any DDE errors
const char* DDE_ErrorString (UINT);         //  Function converts error codes to text


//=====================================================================================
//  Class:  C_DDE_Item
//      This class represents one item of data to be maintained by the DDE server.
//      Each item has an item name, a type, and a value.  The data is still held by
//      the program module which created it; this object holds a pointer to the data
//      and can change the data's value in response to a POKE message from DDE or
//      send the data to DDE in response to a REQUEST message (through C_DDE_Server).
//=====================================================================================

class C_DDE_Item
    {
    private:
        void Construct (const char*);       //  Common bits for all constructors

        char* ItemName;                     //  String holds the DDE name of the item
        void* pTheData;                     //  Pointer to the data item
        DDE_Data_Type TheDataType;          //  What kind of data is this, anyway?
        DWORD idInstance;                   //  Instance handle for DDE system
        HSZ hszItem;                        //  String handle assigned by DDEML
        HSZ hszTopic;                       //  Handle for topic owning this item
        bool AdviseLinked;                  //  Is an advise link set up for this item
        bool DataChanged;                   //  Has data been changed by SetValue()
        bool DDE_Reading;                   //  Flags indicate when DDE is reading
        bool DDE_Writing;                   //  or writing data
        bool User_Reading;                  //  GetValue() is reading the data
        bool User_Writing;                  //  SetValue() is writing the data

    public:
        C_DDE_Item (const char*, int&);             //  There is one constructor
        C_DDE_Item (const char*, unsigned int&);    //  for each type of data to
        C_DDE_Item (const char*, long&);            //  be used.  This way the
        C_DDE_Item (const char*, unsigned long&);   //  user code doesn't have to
        C_DDE_Item (const char*, float&);           //  specify the data type; the
        C_DDE_Item (const char*, double&);          //  compiler will figure it
        C_DDE_Item (const char*, long double&);     //  out and call the correct
        C_DDE_Item (const char*, char&);            //  constructor which then
        C_DDE_Item (const char*, char*, int);       //  saves the data type

        ~C_DDE_Item (void);                 //  The one destructor frees up memory
        void Register (DWORD, HSZ);         //  Function registers strings with DDEML

        bool SetValue (int);                //  These functions all copy the value
        bool SetValue (unsigned int);       //  given in their argument into the
        bool SetValue (long);               //  data buffer.  The copy is performed in
        bool SetValue (unsigned long);      //  a thread-protected fashion using a
        bool SetValue (float);              //  bunch of flags to prevent conflicts
        bool SetValue (double);             //  between the DDE thread and the calling
        bool SetValue (long double);        //  thread.  If the setting was blocked
        bool SetValue (char);               //  because DDE is writing the data, the
        bool SetValue (const char*);        //  return value is false

        bool GetValue (int&);               //  These functions all copy the value
        bool GetValue (unsigned int&);      //  given in their argument into the
        bool GetValue (long&);              //  data buffer for this item.  The copy
        bool GetValue (unsigned long&);     //  must be performed in a thread-safe
        bool GetValue (float&);             //  fashion because the DDE communication
        bool GetValue (double&);            //  is taking place in a different thread
        bool GetValue (long double&);       //  from the user code which is calling
        bool GetValue (char&);              //  these functions.  If the reading was
        bool GetValue (char*);              //  blocked, the function returns false

        void StringToData (const char*);    //  Convert a string into data item format
        const char* DataToString (void);    //  and convert data to a string
        const char* GetName (void)          //  Function returns the name of this
            { return ItemName; }            //  data item
        HSZ GetStringHandle (void)          //  This function returns a handle to the
            { return hszItem; }             //  DDE string with the data item's name
        void AdviseLink (void)              //  Function tells the item it will be
            { AdviseLinked = true; }        //  advise-linked to a server or client
        void AdviseUnlink (void)            //  Function which tells this item it
            { AdviseLinked = false; }       //  should cut off its advise link
        const bool IsAdviseLinked (void)    //  Function tells the caller if the item
            { return (AdviseLinked); }      //  is currently advise linked to a client
        bool SendAdviseData (void);         //  Send advise-linked data to DDE clients
        void StartAdviseLink (HCONV);       //  Set up an advise link to a server
        void EndAdviseLink (HCONV);         //  Break the aforementioned advise link
        bool PokeChangedData (HCONV);       //  Send client's data to a DDE server
    };


//=====================================================================================
//  Class:  C_DDE_Topic
//      This class represents a topic to be served by the DDE server.  Each topic has
//      a name and a list of data items associated with it.
//=====================================================================================

class C_DDE_Topic : public CBasicList
    {
    private:
        char* Name;                         //  String holds the name of this topic    
        HSZ hszTopic;                       //  Handle to string with topic name
        DWORD idInstance;                   //  Instance handle for DDE system

    public:
        //  Constructor gets topic name and DDEML instance handle
        C_DDE_Topic (const char*);
        ~C_DDE_Topic (void);

        void Register (DWORD);              //  Register topic & its items with DDEML

        //  Functions to add a new data item to this topic's list
        C_DDE_Item* AddItem (const char*, int&);
        C_DDE_Item* AddItem (const char*, unsigned int&);
        C_DDE_Item* AddItem (const char*, long&);
        C_DDE_Item* AddItem (const char*, unsigned long&);
        C_DDE_Item* AddItem (const char*, float&);
        C_DDE_Item* AddItem (const char*, double&);
        C_DDE_Item* AddItem (const char*, long double&);
        C_DDE_Item* AddItem (const char*, char&);
        C_DDE_Item* AddItem (const char*, char*, int);

        //  Function returns pointer to the item with the given name
        C_DDE_Item* GetItemWithName (const char*);
        C_DDE_Item* GetItemWithHandle (HSZ);

        //  Functions return the name of this topic and a handle to its DDE string
        const char* GetName (void) { return Name; }
        HSZ GetStringHandle (void) { return hszTopic; }

        bool SendAdviseData (void);         //  Server items send data to clients
        bool PokeChangedData (HCONV);       //  Client items send data to server
        void StartAdviseLinks (HCONV);      //  Tell items to connect advise links
        void EndAdviseLinks (HCONV);        //  and tell them to break advise links
    };


//=====================================================================================
//  Class:  C_DDE_Manager
//      This class encapsulates that part of the DDE interface code which is common to
//      both the DDE server and DDE client objects.  This includes holding basic DDE
//      registration information and managing lists of topics and items.
//=====================================================================================

class C_DDE_Manager : public CBasicList
    {
    protected:
        DWORD idInstance;                   //  Instance handle for DDE system
        DWORD dwThreadId;                   //  ID of thread in which DDE system runs
        HSZ hszService;                     //  Handle of string with service name
        int NumTopics;                      //  Number of topics supported by server
        char* ServiceName;                  //  Name of service provided by server
        bool ReadyToRegister;               //  Flag indicates time to set up DDEML

        //  Functions to find topics within the topic list and items in the item list
        C_DDE_Topic* GetTopicWithName (const char*);
        C_DDE_Topic* GetTopicWithHandle (HSZ);
        C_DDE_Item* GetItemWithName (const char*);
        C_DDE_Item* GetItemWithHandle (HSZ);

    public:
        C_DDE_Manager (void);               //  Constructor initializes variables
        ~C_DDE_Manager (void);              //  Destructor deletes all topics in list

        //  Functions to add a topic to the list
        C_DDE_Topic* AddTopic (const char*);

        //  Tell the DDEML thread it's time to register topic and item names
        void Register_DDE (void);

        //  Function allows caller to see if the DDE thread is ready to register names
        bool IsReadyToRegister (void)
            { return ReadyToRegister; }
    };


//=====================================================================================
//  Class:  C_DDE_Server
//      This class encapsulates a DDE server.  When set up by the user's program, it
//      will maintain a database of variables whose values can be queried by a client
//      program.  The CBasicList ancestor type adds basic linked list handling.
//      Limitations:  There are many; this helps keep the program simple.  Only the
//      text clipboard format is supported.  Only one server may be used in one
//      program at any given time (because the DDEML callback function can find only
//      the one server with which it's associated).
//=====================================================================================

class C_DDE_Server : public C_DDE_Manager
    {
    private:
        HSZPAIR* phszPair;                  //  Points to array of service/topic pairs

        //  These functions respond to messages from a client; all are called by the
        //  DDEML callback function
        HDDEDATA ConnectClient (HSZ, HSZ);
        HDDEDATA WildConnect (HSZ, HSZ);
        HDDEDATA RequestData (WORD, HSZ, HSZ);
        HDDEDATA PokeData (WORD, HSZ, HSZ, HDDEDATA);
        HDDEDATA AdviseStart (WORD, HSZ, HSZ);
        HDDEDATA AdviseRequest (WORD, HSZ, HSZ);
        HDDEDATA AdviseStop (HSZ, HSZ);

        void Initialize (void);             //  Set up the server object
        void Uninitialize (void);           //  Un-setup the same server object
        bool SendAdviseData (void);         //  Send data from advise-linked items

    public:
        C_DDE_Server (const char*);         //  Constrctor starts new thread for DDE
        ~C_DDE_Server (void);               //  Destructor shuts the thread down

        void SetServiceName (const char*);  //  Specify the DDE service name

    //  The DDE server callback function has access to class private data
    friend HDDEDATA EXPENTRY DdeServerCallback (WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA,
                                                DWORD, DWORD);
    friend DWORD WINAPI DDE_ServerThreadFunction (LPVOID lpParam);
    };


//=====================================================================================
//  Class:  C_DDE_Client
//      This class encapsulates DDE client communications.  It allows the user to send
//      and receive data to and from a server application.
//=====================================================================================

class C_DDE_Client : public C_DDE_Manager
    {
    private:
        void Update (void);                 //  Update client's status periodically
        bool PokeChangedData (void);        //  Tell all items to update server's data
        void Initialize (void);             //  Set up the client object
        void Uninitialize (void);           //  Un-setup the same client object
//        bool StartAdviseLink (C_DDE_Item*); //  Start an advise link to a server
//        bool EndAdviseLink (C_DDE_Item*);   //  Break the advise link

        //  Function called by DDE callback processes advise-loop data from a server
        HDDEDATA AdviseData (WORD, HSZ, HSZ, HDDEDATA);

        DWORD dwThreadId;                   //  ID of thread in which DDE system runs
        DWORD idInstance;                   //  Instance handle for DDE system
        bool ReadyToRegister;               //  Flag indicates time to set up DDEML
        HCONV hConversation;                //  Handle to a DDE conversation
        char* ServerName;                   //  String holds name of server to talk to
        char* TopicName;                    //  Name of topic about which to talk
        bool DoConnect;                     //  Signals from user thread to connect
        bool DoDisconnect;                  //  to or disconnect from a server

    public:
        C_DDE_Client (void);                //  Constructor creates a thread for DDE
        ~C_DDE_Client (void);               //  Destructor shuts down the thread

        //  Functions to connect to, and disconnect from, a DDE server
        bool ConnectToServer (const char*, const char*);
        bool DisconnectFromServer (void);

    //  The DDE server callback function has access to class private data
    friend HDDEDATA EXPENTRY DdeClientCallback (WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA,
                                                DWORD, DWORD);
    friend DWORD WINAPI DDE_ClientThreadFunction (LPVOID lpParam);
    };

#endif  //  DDE_STUFF
#endif  //  __WIN32__

