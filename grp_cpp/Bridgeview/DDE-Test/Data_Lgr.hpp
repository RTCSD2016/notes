//*************************************************************************************
//  DATA_LGR.HPP 
//      In this file, we define the interface for data logging objects which can be 
//      used to save data in real time and write it to a disk file after the real time 
//      work has been done.  
//
//  Version
//       1-21-94  JR   Original version
//       7-19-95  JR   User calls as function with variable arguments added
//*************************************************************************************

#ifndef  DATA_LGR_HPP
    #define  DATA_LGR_HPP                   //  Variable to prevent multiple inclusions

//  If nobody has already defined real_time, define it as double here
#ifndef real_time
    #define real_time double 
    #define REAL_TIME_FMT "%lg"
#endif

//  This enum represents all data types which can be saved in these data arrays 
enum LogDataType {LOG_INT, LOG_LONG, LOG_FLOAT, LOG_DOUBLE, LOG_POINTER};

//  Here's the enum which describes the type of buffer which we're using 
enum LogArrayType {LOG_FINITE, LOG_CIRCULAR, LOG_EXPANDING};


//=====================================================================================
//  Class:  CQueueArray
//      This is an array which is of fixed size and stores data in increasing, linear
//      order as it is received.  There is a read pointer and a write pointer; the
//      read pointer follows the write pointer so that the array acts as a FIFO (First
//      In, First Out) storage element.  Because any kind of data must be able to be
//      stored in this array, access is through data pointers.  Every time you ask for
//      a write or read pointer, the array index is incremented, so don't ask for
//      pointers which you're not going to use!
//=====================================================================================

class CQueueArray : public CBasicArray
    {
    public:
        CQueueArray (unsigned, unsigned);   //  Constructor sets data and array size
        ~CQueueArray (void);                //  Destructor frees up memory

        virtual void* WritePointer (void);  //  Get write pointer for writing data in
        virtual void* ReadPointer (void);   //  and read pointer for taking it out
        virtual void Flush (void);          //  Flush (empty) the array and start over
        virtual void Rewind (void);         //  Allow re-reading of data already taken
    };


//=====================================================================================
//  Class:  CExpandingArray
//      This variation on the queue array "expands" - i.e. allocates some new memory
//      for more data - whenever it overflows.  Otherwise, it's used just the same as
//      the fixed-size queue.
//=====================================================================================

class CExpandingArray : public CQueueArray
    {
    protected:

    public:
        CExpandingArray (unsigned, unsigned);   //  Constructor sets up with 1 buffer
        ~CExpandingArray (void);                //  Destructor frees memory

        virtual void* WritePointer (void);      //  Get write pointer
        virtual void* ReadPointer (void);       //  Get read pointer
        virtual void Flush (void);              //  Flush the array and start over
        virtual void Rewind (void);             //  Allow re-reading of the data
        };


//=====================================================================================
//  Class:  CCircularArray
//      When you need a buffer of fixed size which will keep a record of the most
//      recent events, you need a ring buffer.  This class implements a ring buffer as
//      a descendent of the fixed-size basic array with pointers that wrap around and
//      around and write over old data, always keeping the most recent data available.
//=====================================================================================

class CCircularArray : public CBasicArray
    {
    protected:
        unsigned WriteIndex;                    //  Index used for writing data in
        unsigned ReadIndex;                     //  Index used for reading it out
        unsigned MaxPointsTaken;                //  Max. number taken (for re-reading)

    public:
        CCircularArray (unsigned, unsigned);    //  Constructor sets data, array size
        ~CCircularArray (void);                 //  Destructor frees up memory

        virtual void* WritePointer (void);      //  Get write pointer for writing data
        virtual void* ReadPointer (void);       //  and read pointer for taking it out
        virtual void Flush (void);              //  Empty the array and start over
        virtual void Rewind (void);             //  Allow re-reading of the data
        void SetReadIndex (unsigned aNum) { }   //  Set read index makes no sense here
    };


//=====================================================================================
//  Class:  CLogArray
//      This class implements a somewhat automatic array which stores data to be
//      logged in a file.  A list of CLogArray objects is kept by a CDataLogger
//      object, one array for each column of data logged.
//=====================================================================================

class CLogArray
    {
    private:
        CBasicArray* TheArray;              //  The array which actually holds data  
        LogDataType DataType;               //  What kind of data does this array hold
        LogArrayType ArrayType;             //  What type of array is this anyway?
        CString *HeaderLine;                //  Line of text to go above this column
        CString *ErrorString;               //  String holds error messages

    public:
        CLogArray (LogDataType,             //  Constructor allocates some memory for
                   LogArrayType, unsigned); //    an array of the given type
        ~CLogArray (void);                  //  Destructor frees memory back up
        const char *GetData (void);         //  Gets one item of data as text
        void AddHeader (char *);            //  Sets text of this column's header line
        const char *GetErrorString (void);  //  Returns the string with error messages
        void Flush (void)                   //  Flush contents of the basic array in
            { TheArray->Flush (); }         //    which the data had been saved
        void Rewind (void)                  //  Rewind buffers by setting pointers so
            { TheArray->Rewind (); }        //    that the data can be read again
        LogDataType GetDataType (void)      //  Return the type of data which is to be
            { return (DataType); }          //    stored in this array
        void ResetRead (void)               //  Function to set the read index, used
            { TheArray->SetReadIndex (0); } //    for example to reset for re-reading

        //  Function to write the header line into the given file
        void WriteHeader (FILE *aFile) { HeaderLine->WriteToFile (aFile); }

        CLogArray& operator<< (int);        //  Overloaded << operators allow user
        CLogArray& operator<< (long);       //  program to write data to the array
        CLogArray& operator<< (float);      //  in the same syntax cout uses
        CLogArray& operator<< (double);
        CLogArray& operator<< (void *);

        CLogArray& operator>> (int &);      //  These >> operators allow output from
        CLogArray& operator>> (long &);     //  array to calling program, 'cin' style
        CLogArray& operator>> (float &);
        CLogArray& operator>> (double &);
        CLogArray& operator>> (void* &);
        CLogArray& operator>> (int*);       //  These >> operators are versions which
        CLogArray& operator>> (long*);      //  take pointers, not references 
        CLogArray& operator>> (float*);
        CLogArray& operator>> (double*);
        CLogArray& operator>> (void**);
    };


//=====================================================================================
//  Class:  CDataLogger
//      This class allows the user to simply construct and use a data logging object.  
//      The logger keeps one or more arrays into which the user places data with a
//      call similar to that of printf().  After logging is done, the data can be read
//      out and written into a file.  The goal is to allow a user program to take data 
//      in real time and write it to a file when the real-time work is done.  
//      The 'Separator' string holds the text which goes between columns.  The default 
//      is a comma for a .CSV file (easily read by spreadsheets).  You can use the 
//      SetSeparator() method to change the separator to tabs ("\t") or whatever.  
//=====================================================================================

class CDataLogger : public CBasicList 
    {
    private:
        FILE *FileHandle;                       //  Handle of file to which we write 
        CString *ErrorString;                   //  String holds error messages 
        CString *TitleString;                   //  String for title written atop file 
        CString *Separator;                     //  Text which comes between columns 
        LogArrayType ArrayType;                 //  Type of array - linear, circular 
        unsigned ArraySize;                     //  Number of elements in data arrays 
        unsigned LinesSaved;                    //  How many lines saved so far?
        unsigned MaxLinesSaved;                 //  Maximum number we have ever saved  
        boolean WriteHeaders;                   //  TRUE if we'll write column headers 
        boolean WriteLineNumbers;               //  TRUE if we will write line numbers 
        void Initialize (LogArrayType,          //  Function holds common code used by 
                         unsigned);             //    both overloaded constructors 

    public:
        CDataLogger (LogArrayType, unsigned);   //  Constructor which opens no file 
        CDataLogger (LogArrayType, unsigned,    //  Constructor with size, file name 
                     char *);                   //    makes it write to file on delete 
        ~CDataLogger (void);                    //  Destructor writes to file 

        void DefineData (int, ...);             //  Define what data goes where 
        void AddTitle (char *);                 //  Add some text to the header
        void AddLineNumbers (void);             //  Turn on line numbers in column 1 
        void SetSeparator (char *);             //  Change the text between columns 
        void Flush (void);                      //  Flush array contents to file
        void Rewind (void);                     //  Allow data to be read over again
        void DiscardData (void);                //  Throw out data and restart logging 
        unsigned GetNumLines (void);            //  Returns how many data taken so far

        //  These functions are called to save a line of data into the logger and to
        //  read a line from it with formats like those of printf() and scanf()
        friend int SaveLoggerData (CDataLogger*, ...);
        friend int GetLoggerData (CDataLogger*, ...);
        friend void DefineLoggerData (CDataLogger*, int, ...);
    };

#endif      //  End of multiple inclusion protection

