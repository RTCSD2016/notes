//*************************************************************************************
//  DATA_LGR.CPP
//      This file contains the implementation for the data logger class.  The logger
//      holds data in a linear, circular, or expanding buffer.
//
//  Version
//       1-21-95  JR   Original version
//       2-02-95  JR   Circular array added
//       2-12-95  JR   Bug fix (already)
//       7-18-95  JR   Uses new version of base_obj.cpp with new list objects
//*************************************************************************************

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <values.h>         //  Use maximum numbers defined here as error indicators
#include "base_obj.hpp"
#include "data_lgr.hpp"


//=====================================================================================
//  Class:  CQueueArray
//      This is an array which is of fixed size and stores data in increasing, linear
//      order as it is received.  There is a read pointer and a write pointer; the
//      read pointer follows the write pointer so that the array acts as a FIFO (First
//      In, First Out) storage element.  Because any kind of data must be able to be
//      stored in this array, access is through data pointers.
//=====================================================================================

//-------------------------------------------------------------------------------------
//  Constructor:  CQueueArray
//      This constructor calls that of CBasicArray, then starts the data pointers at 0.

CQueueArray::CQueueArray (unsigned aDataSize, unsigned aArraySize)
    : CBasicArray (aDataSize, aArraySize)
    {
    WriteIndex = 0;                 //  Set the read and write indices to 0 'cause
    ReadIndex = 0;                  //  there isn't any data yet
    PointsTaken = 0;                //  Note that we haven't taken any data yet
    }


//-------------------------------------------------------------------------------------
//  Destructor:  ~CQueueArray
//      This destructor, uh,...?

CQueueArray::~CQueueArray (void)
    {
//    for (CListNode* pCur = pFirst; pCur != NULL; pCur = pCur->pNextNode)
//        delete (pCur->pNodeData);
    }


//-------------------------------------------------------------------------------------
//  Function:  WritePointer
//      When you're going to write some data to the array, use this function to get a
//      write pointer.  It will automatically increment the index to the next item,
//      so don't use it in vain or you'll leave gaps in your data.

void* CQueueArray::WritePointer (void)
    {
    //  Check if the write index is within array bounds; if not return NULL
    if (WriteIndex >= TotalSize)
        return (NULL);

    //  If the index is within bounds, get a copy of the pointer which can be returned
    void* ThePtr = operator[] (WriteIndex);

    //  Next increment the index so it points to the next item, if there is one
    ++WriteIndex;
    ++PointsTaken;

    //  Finally return the pointer we found before incrementing
    return (ThePtr);
    }


//-------------------------------------------------------------------------------------
//  Function:  ReadPointer
//      The read pointer works like the write pointer except that you can't read data
//      which hasn't been written; so the reader's not allowed to catch the writer.

void* CQueueArray::ReadPointer (void)
    {
    //  Check that the read index is less than the write index; if not, we're trying
    //  to read before writing, so return NULL
    if (ReadIndex >= WriteIndex)
        return (NULL);

    //  If the read index is OK, get a copy of the pointer and return it
    void* ThePtr = operator[] (ReadIndex);
    ++ReadIndex;
    return (ThePtr);
    }


//-------------------------------------------------------------------------------------
//  Function:  Flush
//      The flushing of a queue is simple: just set the read and write indices to 0 so
//      that new data can be written and read.  The data array has been allocated
//      already, so just leave it.

void CQueueArray::Flush (void)
    {
    WriteIndex = 0;
    ReadIndex = 0;
    PointsTaken = 0;
    }


//-------------------------------------------------------------------------------------
//  Function:  Rewind
//      This function resets the read pointer to the beginning of the data, thereby
//      allowing the array to be read again from its 'full' state.

void CQueueArray::Rewind (void)
    {
    ReadIndex = 0;
    }


//=====================================================================================
//  Class:  CExpandingArray
//      This variation on the queue array "expands" - i.e. allocates some new memory
//      for more data - whenever it overflows.  Otherwise, it's used just the same as
//      the fixed-size queue.
//=====================================================================================

//-------------------------------------------------------------------------------------
//  Constructor:  CExpandingArray
//      This constructor starts up the array with one buffer allocated and the read
//      and write pointers set to the 0-th element...well actually the constructor
//      for CQueueArray does all that.

CExpandingArray::CExpandingArray (unsigned aDataSize, unsigned aArraySize)
    : CQueueArray (aDataSize, aArraySize)
    {
    }


//-------------------------------------------------------------------------------------
//  Destructor:  ~CExpandingArray
//      This thing frees up memory.

CExpandingArray::~CExpandingArray (void)
    {
    }


//-------------------------------------------------------------------------------------
//  Function:  WritePointer
//      When you're going to write some data to the array, use this function to get a
//      write pointer.  It will automatically increment the index to the next item,
//      so don't use it in vain or you'll leave gaps in your data.

void* CExpandingArray::WritePointer (void)
    {
    //  If the write index is greater than the current size of the array, we must
    //  allocate another buffer
    if (WriteIndex >= TotalSize)
        {
        if (Expand () == 0)                 //  A zero means we can't allocate more
            return (NULL);
        }

    //  If the index is within bounds, get a copy of the pointer which can be returned
    void* ThePtr = operator[] (WriteIndex);

    //  Next increment the index so it points to the next item, if there is one
    ++WriteIndex;
    ++PointsTaken;

    //  Finally return the pointer we found before incrementing
    return (ThePtr);
    }


//-------------------------------------------------------------------------------------
//  Function:  ReadPointer
//      The read pointer works like the write pointer except that if there's nothing
//      to read, the array isn't expanded - the function just returns NULL.

void* CExpandingArray::ReadPointer (void)
    {
    //  Check that the read index is less than the write index; if not, we're trying
    //  to read before writing, so return NULL
    if (ReadIndex >= WriteIndex)
        return (NULL);

    //  If the read index is OK, get a copy of the pointer and return it
    void* ThePtr = operator[] (ReadIndex);
    ++ReadIndex;
    return (ThePtr);
    }


//-------------------------------------------------------------------------------------
//  Function:  Flush
//      To flushing of an expanding buffer requires that we set the read and write
//      indices to 0 so that new data can be written and read and call the CBasicArray
//      Flush() method to delete the buffers full of data and allocate a new one.

void CExpandingArray::Flush (void)
    {
    WriteIndex = 0;
    ReadIndex = 0;
    PointsTaken = 0;

    CBasicArray::Flush ();
    }


//-------------------------------------------------------------------------------------
//  Function:  Rewind
//      This function resets the read pointer to the beginning of the data, thereby
//      allowing the array to be read again from its 'full' state.

void CExpandingArray::Rewind (void)
    {
    ReadIndex = 0;
    }


//=====================================================================================
//  Class:  CCircularArray
//      When you need a buffer of fixed size which will keep a record of the most
//      recent events, you need a ring buffer.  This class implements a ring buffer as
//      a descendent of the fixed-size basic array with pointers that wrap around and
//      around and write over old data, always keeping the most recent data available.
//=====================================================================================

//-------------------------------------------------------------------------------------
//  Constructor:  CCircularArray
//      This constructor starts up the array with a buffer allocated and the read
//      and write pointers set to the 0-th element...well actually the constructor
//      for CBasicArray does all that.

CCircularArray::CCircularArray (unsigned aDataSize, unsigned aArraySize)
    : CBasicArray (aDataSize, aArraySize)
    {
    WriteIndex = 0;                 //  Set the read and write indices to 0 'cause
    ReadIndex = 0;                  //  there isn't any data yet
    PointsTaken = 0;                //  Same for the points-taken counter
    MaxPointsTaken = 0;             //  Ditto again for maximum points counter
    }


//-------------------------------------------------------------------------------------
//  Destructor:  ~CCircularArray
//      This destructor, as usual, frees up the memory used by the array.

CCircularArray::~CCircularArray (void)
    {
    }


//-------------------------------------------------------------------------------------
//  Function:  WritePointer
//      Whenever there's data to be written, call this function to get a pointer to a
//      place to put it.  It will give you a pointer and increment the write index to
//      the next location, whether there's data there or not.

void* CCircularArray::WritePointer (void)
    {
    //  Get the pointer which we're going to return to the calling function
    void* ThePtr = operator[] (WriteIndex);

    //  If the buffer's full and we're writing over old data, push the read index
    //  along in front of the write index so that it still points to the oldest data
    if (PointsTaken >= TotalSize)
        {
        if (++ReadIndex >= TotalSize)
            ReadIndex = 0;
        }

    //  Now move the write index.  Note that with a full buffer which isn't being read
    //  the read and write indices end up moving together
    if (++WriteIndex >= TotalSize)
        WriteIndex = 0;

    //  If we haven't filled the array, increment the numbers of data points taken
    if (PointsTaken < TotalSize)
        PointsTaken++;
    if (MaxPointsTaken < TotalSize)
        MaxPointsTaken++;

    return (ThePtr);
    }


//-------------------------------------------------------------------------------------
//  Function:  ReadPointer
//      The read pointer always "follows" the write pointer around the ring; it must
//      never overtake the write pointer or you might read unwritten data.

void* CCircularArray::ReadPointer (void)
    {
    //  Check if there's data available (that is, more data has been written than
    //  read) by looking at PointsTaken.  If there's no new data, return NULL
    if (PointsTaken == 0)
        return (NULL);

    //  If we get here, there is new data, so get a pointer to it
    void* ThePtr = operator[] (ReadIndex);

    //  Now advance the read index and decrement the points-taken counter.  Next time
    //  we want to read data, we'll check if this counter is 0 again
    if (++ReadIndex >= TotalSize)
        ReadIndex = 0;
    PointsTaken--;

    return (ThePtr);
    }


//-------------------------------------------------------------------------------------
//  Function:  Flush
//      In order to flush a circular array, just set the indices to 0 so you can over-
//      write the old data.

void CCircularArray::Flush (void)
    {
    WriteIndex = 0;
    ReadIndex = 0;
    PointsTaken = 0;
    }


//-------------------------------------------------------------------------------------
//  Function:  Rewind
//      This function resets the read pointer to the beginning of the data, thereby
//      allowing the array to be read again from its 'full' state.  For a circular
//      array, the beginning of the data is the oldest data.  If the buffer's not yet
//      full, the oldest data is at location 0; else it's pointed to by the write
//      index, because the write index points to where the next new data will go.

void CCircularArray::Rewind (void)
    {
    if (MaxPointsTaken < TotalSize)
        ReadIndex = 0;
    else
        ReadIndex = WriteIndex;

    PointsTaken = MaxPointsTaken;
    }


//=====================================================================================
//  Class:  CLogArray
//      This class implements a somewhat automatic array which stores data to be
//      logged in a file.  The log array keeps a pointer to one of the CBasicArray
//      descendents, using the one appropriate for the user's choice of array type:
//      linear, expanding, or circular.
//=====================================================================================

//-------------------------------------------------------------------------------------
//  Constructor:  CLogArray
//      This constructor sets up a logging array of the given type with a starting
//      buffer of the given size.

CLogArray::CLogArray (LogDataType aDaType, LogArrayType aArType, unsigned aBufferSize)
    {
    int DataSize;                                   //  How many bytes per data item?

    ErrorString = new CString (128);                //  String to hold error messages
    HeaderLine = new CString (64);                  //  String holds column's header
    DataType = aDaType;                             //  Save the data type information
    ArrayType = aArType;                            //  Save array type info. also

    //  Depending on the type of data the user wants to store, set the size of each
    //  data item in bytes
    switch (aDaType)
        {
        case LOG_INT:
            DataSize = sizeof (int);
            break;
        case LOG_LONG:
            DataSize = sizeof (long);
            break;
        case LOG_FLOAT:
            DataSize = sizeof (float);
            break;
        case LOG_DOUBLE:
            DataSize = sizeof (double);
            break;
        case LOG_POINTER:
            DataSize = sizeof (void*);
            break;
        }

    //  Create a new array to hold the data, of the type specified by the caller
    switch (aArType)
        {
        case LOG_FINITE:
            TheArray = new CQueueArray (DataSize, aBufferSize);
            break;
        case LOG_EXPANDING:
            TheArray = new CExpandingArray (DataSize, aBufferSize);
            break;
        case LOG_CIRCULAR:
            TheArray = new CCircularArray (DataSize, aBufferSize);
            break;
        }

    //  If there's no room for data, write complaint to error string
    if (TheArray == NULL)
        *ErrorString << "ERROR:  Unable to allocate memory for data array";
    }


//-------------------------------------------------------------------------------------
//  Destructor:  ~CLogArray
//      This destructor just frees the memory which has been used by the log array.

CLogArray::~CLogArray (void)
    {
    delete HeaderLine;
    delete ErrorString;

    //  Delete the data array, specifying the type of the array for proper deletion
    switch (ArrayType)
        {
        case LOG_FINITE:
            delete ((CQueueArray*)TheArray);
            break;
        case LOG_EXPANDING:
            delete ((CExpandingArray*)TheArray);
            break;
        case LOG_CIRCULAR:
            delete ((CCircularArray*)TheArray);
            break;
        }

    }


//-------------------------------------------------------------------------------------
//  Function:  GetData
//      This function returns one item of data from the array in the form of a char-
//      acter string.  The intended use of this GetData() function is where the data
//      taken during a real-time run nust be printed on the screen or saved in a file.
//      The buffer pTemp stores the data in its native binary format - int, double, 
//      whatever; aBuf is where the character string goes. 

const char *CLogArray::GetData (void) 
    {
    static char aBuf[36];           //  Keep an array around which holds output strings 


    //  Look up and convert the data item indexed by current read pointer 
    if (DataType == LOG_INT)
        {
        int Data;
        (*this) >> &Data;
        sprintf (aBuf, "%d", Data);
        }
    else if (DataType == LOG_LONG)
        {
        long Data;
        (*this) >> &Data;
        sprintf (aBuf, "%ld", Data);
        }
    else if (DataType == LOG_FLOAT)
        {
        float Data;
        (*this) >> &Data;
        sprintf (aBuf, "%g", Data);
        }
    else if (DataType == LOG_DOUBLE)
        {
        double Data;
        (*this) >> &Data;
        sprintf (aBuf, "%lg", Data);
        }
    else if (DataType == LOG_POINTER)
        {
        void* Data;
        (*this) >> &Data;
        sprintf (aBuf, "%p", Data);
        }

    return (aBuf);                  //  Return pointer to the array holding the output
    }


//-------------------------------------------------------------------------------------
//  Function:  AddHeader
//      This function adds the string pointed to by the pointer given in the argument
//      to the header line which will be printed above this data in the output file.

void CLogArray::AddHeader (char *aString) 
    {
    *HeaderLine << aString;
    }


//-------------------------------------------------------------------------------------
//  Function:  GetErrorString 
//      This function returns the error message(s) which this array has generated.  It 
//      should be called *once* when the data is being read out by the logger object.  

const char *CLogArray::GetErrorString (void)
    {
    //  Call the error string's method and return the pointer it gives to us 
    return (ErrorString->GetString());
    }


//-------------------------------------------------------------------------------------
//  Operators:  << 
//      These overloaded arrays allow a user to write data into the array by using the 
//      "<<" operator, much as he would use when writing output to cout.  They only 
//      work when the argument is one of the types in the list given in DATA_LGR.HPP.  

CLogArray& CLogArray::operator<< (int aData) 
    {
    int* here;                                      //  Store pointer to the data here 

    if (DataType == LOG_INT)
        {
        here = (int*)TheArray->WritePointer ();
        if (here != NULL)
            *here = aData;
        }
    return (*this);
    }

CLogArray& CLogArray::operator<< (long aData)
    {
    long* here;

    if (DataType == LOG_LONG)
        {
        here = (long*)TheArray->WritePointer ();
        if (here != NULL)
            *here = aData;
        }
    return (*this);
    }

CLogArray& CLogArray::operator<< (float aData)
    {
    float* here;

    if (DataType == LOG_FLOAT)
        {
        here = (float*)TheArray->WritePointer ();
        if (here != NULL)
            *here = aData;
        }
    return (*this);
    }

CLogArray& CLogArray::operator<< (double aData)
    {
    double* here;

    if (DataType == LOG_DOUBLE)
        {
        here = (double*)TheArray->WritePointer ();
        if (here != NULL)
            *here = aData;
        }
    return (*this);
    }

CLogArray& CLogArray::operator<< (void *aData)
    {
    void** here;

    if (DataType == LOG_POINTER)
        {
        here = (void**)TheArray->WritePointer ();
        if (here != NULL)
            *here = aData;
        }
    return (*this);
    }


//-------------------------------------------------------------------------------------
//  Operators:  >>
//      These overloaded arrays allow a user to read data from the array by using the
//      ">>" operator in the same way as one uses cin.  Note that the only indication
//      you have that the number is invalid is a big number which is returned, and in
//      many cases that's valid data.  The moral: the user program should keep track
//      of how many data items it saved and only ask for data which is really there.  

CLogArray& CLogArray::operator>> (int &aData)
    {
    int* here = (int*)TheArray->ReadPointer ();     //  Get a pointer to the data
    if ((DataType == LOG_INT) && (here != NULL))    //  and use it to fill the refer-
        aData = *here;                              //  ence given by the user
    else
        aData = -MAXINT;
    return (*this);                                 //  Return reference to this obj.
    }

CLogArray& CLogArray::operator>> (long &aData)
    {
    long* here = (long*)TheArray->ReadPointer ();
    if ((DataType == LOG_LONG) && (here != NULL))
        aData = *here;
    else
        aData = -MAXLONG;
    return (*this);
    }

CLogArray& CLogArray::operator>> (float &aData)
    {
    float* here = (float*)TheArray->ReadPointer ();
    if ((DataType == LOG_FLOAT) && (here != NULL))
        aData = *here;
    else
        aData = -MAXFLOAT;
    return (*this);
    }

CLogArray& CLogArray::operator>> (double &aData)
    {
    double* here = (double*)TheArray->ReadPointer ();
    if ((DataType == LOG_DOUBLE) && (here != NULL))
        aData = *here;
    else
        aData = -MAXDOUBLE;
    return (*this);
    }

CLogArray& CLogArray::operator>> (void* &aData)
    {
    void** here = (void**)TheArray->ReadPointer ();
    if ((DataType == LOG_POINTER) && (here != NULL))
        aData = *here;
    else
        aData = NULL;
    return (*this);
    }

//------------------------------  Versions which take pointers  -----------------------

CLogArray& CLogArray::operator>> (int* aData)
    {
    int* here = (int*)TheArray->ReadPointer ();     //  Get a pointer to the data
    if ((DataType == LOG_INT) && (here != NULL))    //  and use it to fill the refer-
        *aData = *here;                              //  ence given by the user
    else
        *aData = -MAXINT;
    return (*this);                                 //  Return reference to this obj.
    }

CLogArray& CLogArray::operator>> (long* aData)
    {
    long* here = (long*)TheArray->ReadPointer ();
    if ((DataType == LOG_LONG) && (here != NULL))
        *aData = *here;
    else
        *aData = -MAXLONG;
    return (*this);
    }

CLogArray& CLogArray::operator>> (float* aData)
    {
    float* here = (float*)TheArray->ReadPointer ();
    if ((DataType == LOG_FLOAT) && (here != NULL))
        *aData = *here;
    else
        *aData = -MAXFLOAT;
    return (*this);
    }

CLogArray& CLogArray::operator>> (double* aData)
    {
    double* here = (double*)TheArray->ReadPointer ();
    if ((DataType == LOG_DOUBLE) && (here != NULL))
        *aData = *here;
    else
        *aData = -MAXDOUBLE;
    return (*this);
    }

CLogArray& CLogArray::operator>> (void** aData)
    {
    void** here = (void**)TheArray->ReadPointer ();
    if ((DataType == LOG_POINTER) && (here != NULL))
        *aData = *here;
    else
        *aData = NULL;
    return (*this);
    }


//=====================================================================================
//  Class:  CDataLogger
//      This class allows the user to simply construct and use a data logging object.
//      The logger keeps one or more arrays into which the user places data with a
//      call to LogDataLine().  After logging is done, the data can be read back
//      out and written into a file.  The goal is to allow a user program to take data
//      in real time and write it to a file when the real-time work is done.
//=====================================================================================

//-------------------------------------------------------------------------------------
//  Constructor:  CDataLogger (version which doesn't use a file) 
//      This constructor creates a data logger object with no arrays and no file name.
//      The user program will get data from the logger and do something with it other 
//      than having this logger object write the data to a file.  

CDataLogger::CDataLogger (LogArrayType aType, unsigned aSize) : CBasicList ()
    {
    Initialize (aType, aSize);          //  First call the initialization function

    FileHandle = NULL;                  //  We don't use a file, so handle is null
    }


//-------------------------------------------------------------------------------------
//  Constructor:  CDataLogger (version which takes a file name)
//      This constructor creates a data logger object with no arrays, but it saves the
//      given size for creating data arrays when the user program calls AddColumn().
//      It does open the data file with the given name for writing.

CDataLogger::CDataLogger (LogArrayType aType, unsigned aSize, char *aFileName)
    : CBasicList ()
    {
    //  First call the initialization function
    Initialize (aType, aSize);

    //  The open the file, replacing the null file buffer with a valid one 
    FileHandle = fopen (aFileName, "w");
    if (FileHandle == NULL)
        *ErrorString << "ERROR:  Unable to open logger file " << aFileName;
    }


//-------------------------------------------------------------------------------------
//  Function:  Initialize 
//      This function contains code that all versions of the overloaded constructor 
//      have in common.  It's just here for convenience really.  

void CDataLogger::Initialize (LogArrayType aType, unsigned aSize) 
    {
    ErrorString = new CString (256);    //  Create a string in which to store errors 
    Separator = new CString (16);       //  Create string for column separator text 
    TitleString = new CString (128);    //  Also create string for the log's title 
    ArraySize = aSize;                  //  Save starting number of items in arrays 
    ArrayType = aType;                  //  Save the array overflow handling method 
    LinesSaved = 0;                     //  Haven't saved any data yet
    MaxLinesSaved = 0;                  //  Same thing for the maximum saved so far 
    WriteLineNumbers = FALSE;           //  Don't write line numbers unless asked to 
    WriteHeaders = FALSE;               //  Don't write headers unless asked to
    *Separator = " ";                   //  Default separator is space for Matlab text
    }


//-------------------------------------------------------------------------------------
//  Destructor:  ~CDataLogger
//      This destructor closes the data file (which forces writing to the disk from
//      the operating system's file buffer on PC's and such) and frees up memory.

CDataLogger::~CDataLogger (void)
    {
    if (FileHandle != NULL)
        {
        Flush ();                   //  This writes data that's been taken to a file
        fclose (FileHandle);        //  This line flushes the data file to disk
		}
    delete ErrorString;
    delete Separator;
	delete TitleString;

    //  Delete all the array objects which are held in the list of log arrays
    for (CLogArray* pCur = (CLogArray*)GetHead (); pCur != NULL;
         pCur = (CLogArray*)GetNext ())
        delete (pCur);

    this->CBasicList::~CBasicList ();     //  Call the basic list object destructor
    }


//-------------------------------------------------------------------------------------
//  Function:  DefineData
//      The user program calls this function to define the data items which go into
//      the data columns in the list of data to be saved by the logger.

void CDataLogger::DefineData (int NumColumns, ...)
    {
    va_list Parameters;                 //  List of the function's parameters
    int Index;                          //  Index number counts through the columns
    LogDataType ColumnType;             //  Type of data to be stored in each column

    //  Use va_arg macros to get each column's data type; then create column arrays
    va_start (Parameters, NumColumns);
    for (Index = 0; Index < NumColumns; Index++)
        {
        //  Get the column type out of the argument list
        ColumnType = va_arg (Parameters, LogDataType);

        //  Call constructor to create the new log array object for this column
        CLogArray *pNewArray = new CLogArray (ColumnType, ArrayType, ArraySize);

        //  Now call the Insert() method this object inherited from CListObj.  Insert()
        //  will place this column at the end of the list of columns.
        CBasicList::Insert ((void*)pNewArray);
        }

    va_end (Parameters);
    }


//-------------------------------------------------------------------------------------
//  Friend function:  DefineLoggerData
//      This is a version of DefineData() which is implemented as a friend function
//      instead of a member function so that it can be called with a C style call
//      rather than requiring the user to use a C++ style member function call. 

void DefineLoggerData (CDataLogger* apLogger, int NumColumns, ...)
    {
    va_list Parameters;                 //  List of the function's parameters 
    int Index;                          //  Index number counts through the columns
    LogDataType ColumnType;             //  Type of data to be stored in each column

    //  Use va_arg macros to get each column's data type; then create column arrays
    va_start (Parameters, NumColumns);
    for (Index = 0; Index < NumColumns; Index++)
        {
        //  Get the column type out of the argument list
        ColumnType = va_arg (Parameters, LogDataType);

        //  Call constructor to create the new log array object for this column
        CLogArray *pNewArray = new CLogArray (ColumnType, apLogger->ArrayType,
                                                          apLogger->ArraySize);

        //  Now call the Insert() method this object inherited from CListObj.  Insert()
        //  will place this column at the end of the list of columns.
        apLogger->CBasicList::Insert ((void*)pNewArray);
        }

    va_end (Parameters);
    }


//-------------------------------------------------------------------------------------
//  Function:  AddTitle
//      This function creates a title for the log which will be written at the top of
//      the output file.

void CDataLogger::AddTitle (char *aString)
    {
    *TitleString << aString;                    //  Add text to the title string 
    }


//-------------------------------------------------------------------------------------
//  Function:  AddLineNumbers 
//      If the user calls this function, line numbers will be turned on - this means
//      that when the data is printed to a file, integer line numbers will be printed 
//      in the leftmost column.  

void CDataLogger::AddLineNumbers (void) 
    {
    WriteLineNumbers = TRUE;
    }


//-------------------------------------------------------------------------------------
//  Function:  SetSeparator 
//      This function allows the user to change the text which comes between the 
//      columns - the default is a comma for a comma-separated-variable file.  

void CDataLogger::SetSeparator (char *aString)
    {
    *Separator = aString; 
    }


//-------------------------------------------------------------------------------------
//  Function:  DiscardData
//      This function resets the arrays so that they are in the empty state.  The
//      effect is to throw away any data which we've

void CDataLogger::DiscardData (void)
    {
    CLogArray *pCol;                            //  Pointer to data column in list


    //  The data has been retreived, so set pointers to first line to start over
    for (pCol = (CLogArray *)GetHead (); pCol != NULL; pCol = (CLogArray *)GetNext ())
        pCol->Flush ();
    LinesSaved = 0;
    }


//-------------------------------------------------------------------------------------
//  Function:  Flush
//      This function writes the data in the arrays to the data file.

void CDataLogger::Flush (void)
    {
    unsigned LineCounter;                       //  Counts lines in arrays
    CLogArray *pCol;                            //  Pointer to data column in list


    //  Write stuff to file only if there's a file associated with this logger
    if (FileHandle != NULL)
        {
        //  If titles have been turned on, write the file title on the first line.
        //  Then clear the title so it doesn't get written again
        TitleString->WriteToFile (FileHandle);
        *TitleString = "";
        fprintf (FileHandle, "\n");

        //  If there are any errors, write them next, then a linefeed
        ErrorString->WriteToFile (FileHandle);
        fprintf (FileHandle, "\n");

        //  Check the columns for errors (such as wrong type of data) and reset them
        pCol = (CLogArray *)GetHead ();
        while (pCol != NULL)
            {
            fprintf (FileHandle, "%s", pCol->GetErrorString ());
            pCol->ResetRead ();
            pCol = (CLogArray *) GetNext ();
            }

        //  If column headers have been activated, write the headers on the next line.
        //  If line number column is turned on, its header will be "Line"
        if (WriteHeaders == TRUE)
            {
            if (WriteLineNumbers == TRUE)               //  Write line number header
                {                                       //  above the first column
                fprintf (FileHandle, "Line");
                Separator->WriteToFile (FileHandle);
                }

            for (pCol = (CLogArray *)GetHead (); pCol != NULL;
                pCol = (CLogArray *)GetNext ())
                {
                pCol->WriteHeader (FileHandle);         //  Write each header string,
                Separator->WriteToFile (FileHandle);    //  then write separator text,
                }
            fprintf (FileHandle, "\n");                 //  Go down to first data line
            WriteHeaders = FALSE;                       //  Only write headers once.
            }

        //  For each line in the arrays, get the data and write it to the file
        for (LineCounter = 0; LineCounter < LinesSaved; LineCounter++)
            {
            //  If we're supposed to write line numbers, put them first on this line
            if (WriteLineNumbers == TRUE)
                {
                fprintf (FileHandle, "%u", LineCounter);
                Separator->WriteToFile (FileHandle);
                }

            //  Now step through all the columns, writing the data from each
            pCol = (CLogArray *) GetHead ();
            while (pCol != NULL)
                {
                fprintf (FileHandle, "%s", pCol->GetData ());
                Separator->WriteToFile (FileHandle);
                pCol = (CLogArray *) GetNext ();
                }
            fprintf (FileHandle, "\n");
            }
        }                                           //  End of the file-writing "if"

    //  Call function to empty out the data arrays
    DiscardData ();
    }


//-------------------------------------------------------------------------------------
//  Function:  Rewind
//      This function resets the pointers in the data arrays so that the data which
//      has already been taken can be read all over again.

void CDataLogger::Rewind (void)
    {
    CLogArray *pCol;                            //  Pointer to data column in list

    //  The data has been retreived, so set pointers to first line to start over
    for (pCol = (CLogArray *)GetHead (); pCol != NULL; pCol = (CLogArray *)GetNext ())
        {
        pCol->Rewind ();
        }

    //  Set the number of lines saved back to the maximum number available
    LinesSaved = MaxLinesSaved;
    }


//-------------------------------------------------------------------------------------
//  Function:  GetNumLines
//      This function returns the number of lines of data which can be read from the
//      buffer.

unsigned CDataLogger::GetNumLines (void)
    {
    return (LinesSaved);
    }


//-------------------------------------------------------------------------------------
//  Function:  LogDataLine
//      This is the standard function for saving data to the logger.  The user calls
//      LogDataLine (pLogger, data1, data2, ...) where the 'dataX' parameters are the
//      data to be saved.  The number of data items in the function call MUST equal
//      the number of columns the logger stores.  This function returns the number of
//      data items saved or a negative number if there were problems saving the data.

int SaveLoggerData (CDataLogger* pLogger, ...)
    {
    va_list Parameters;                         //  Parameters given to this function
    CLogArray* pCol;                            //  Pointer to a log column object


    va_start (Parameters, pLogger);             //  Begin variable-argument processing

    //  For each column to which a point is saved, get the data from an argument in
    //  the argument list and then move to the next argument
    pCol = (CLogArray*)(pLogger->GetHead ());
    while (pCol != NULL)
        {
        //  A different call of va_arg is required for each type of parameter
        switch (pCol->GetDataType ())
            {
            case LOG_INT:
                *pCol << va_arg (Parameters, int);
                break;
            case LOG_LONG:
                *pCol << va_arg (Parameters, long);
                break;
            //  Floats get promoted to doubles when they're passed as parameters, so
            //  we have to convert them back to floats before saving
            case LOG_FLOAT:
                *pCol << (float)(va_arg (Parameters, double));
                break;
            case LOG_DOUBLE:
                *pCol << va_arg (Parameters, double);
                break;
            case LOG_POINTER:
                *pCol << va_arg (Parameters, void*);
                break;
            default:
                va_end (Parameters);
                return (-1);
            }
        pCol = (CLogArray*)(pLogger->GetNext ());
        }

    //  Increment the number of items saved, unless we've filled the whole buffer 
    if ((pLogger->ArrayType == LOG_FINITE) || (pLogger->ArrayType == LOG_CIRCULAR))
        {
        if (pLogger->LinesSaved < pLogger->ArraySize)
            {
            pLogger->LinesSaved++;
            pLogger->MaxLinesSaved++;
            }
        }
    else if (pLogger->ArrayType == LOG_EXPANDING)
        {
        pLogger->LinesSaved++;
        pLogger->MaxLinesSaved++;
        }

    //  If everything worked OK, close the parameter processor and return
    va_end (Parameters);
    return (pLogger->HowMany ());
    }


//-------------------------------------------------------------------------------------
//  Function:  GetDataLine
//      This is the standard function for retreiving data from the logger.  The user
//      calls GetDataLine (pLogger, &data1, &data2, ...) where the '&dataX' parameters
//      are *pointers* to places to which the retreived data goes.  The number of data
//      items in the function call MUST equal the number of columns the logger stores.
//      This function returns the number of data items retreived or a negative number
//      if there were problems getting the data out.

int GetLoggerData (CDataLogger* pLogger, ...)
    {
    va_list Parameters;                         //  Parameters given to this function
    CLogArray* pCol;                            //  Pointer to a log column object
    void* pData;                                //  Data pointer from argument list


    va_start (Parameters, pLogger);             //  Begin variable-argument processing

    //  For each column, get the location from an argument in the argument list
    pCol = (CLogArray*)(pLogger->GetHead ());
    while (pCol != NULL)
        {
        //  Get a pointer to the data from the command line
        pData = va_arg (Parameters, void*);

        //  A different call of va_arg is required for each type of parameter
        LogDataType aType = pCol->GetDataType ();

        if (aType == LOG_INT)
            *pCol >> (int*)pData;
        else if (aType == LOG_LONG)
            *pCol >> (long*)pData;
        else if (aType == LOG_FLOAT)
            *pCol >> (float*)pData;
        else if (aType == LOG_DOUBLE)
            *pCol >> (double*)pData;
        else if (aType == LOG_POINTER)
            *pCol >> (void**)pData;
        else
            {
            va_end (Parameters);
            return (-1);
            }
        pCol = (CLogArray*)(pLogger->GetNext ());
        }

    //  If everything worked OK, close the parameter processor and return
    va_end (Parameters);
    pLogger->LinesSaved--;
    return (pLogger->HowMany ());
    }

