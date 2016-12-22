//*************************************************************************************
//  BASE_OBJ.HPP
//      This file contains definitions for some basic general-purpose classes which
//      were developed for use with the CTL_EXEC scheduler.  They are also used with
//      the buffered data logger, Tranrun 3, and so on.
//
//  Copyright (c) 1994, D.M.Auslander and J.R.Ridgely
//      May be used and distributed for any non-commercial purposes as long as this
//      copyright notice is included.
//      Original program 11-94 by DMA
//*************************************************************************************

#define  BASE_OBJ_HPP                       //  Variable to prevent multiple inclusions

#include <stdio.h>

#include <compconv.hpp>         //  File which contains compiler compatibility stuff

//  Define a boolean data type with TRUE and FALSE - unless somebody else has
//  12-26-94  JR  Changed to #definitions for compatibility with Windows
#define  boolean  int
#if !defined (TRUE) && !defined (FALSE)
    #define  TRUE     1
    #define  FALSE    0
#endif

//  Handy macro to define the minimum of two numbers
#define  minimum(x,y)  ((x)>(y))?(y):(x)

//  Forward declaration of CBasicList - List node needs it to a keep parent pointer
class CBasicList;


//=====================================================================================
//  Class:  CListNode
//      This is a template for the nodes in a linked list.  These nodes contain poin-
//      ters so that they can point to one another (the list is doubly linked) and
//      data pointers so that each node can hold its own data object.
//=====================================================================================

class CListNode
    {
    public:
        CListNode* pPrevNode;                   //  Pointers to previous and next
        CListNode* pNextNode;                   //  nodes in the linked list
        void* pNodeData;                        //  Pointer to actual saved data
        int SortKey;                            //  Key used for sorting items
        CBasicList* pParent;                    //  Pointer to parent list object

        CListNode (CListNode*, CListNode*,      //  Constructor creates a new node
                   void*, int, CBasicList*);    //  with the given pointers
        ~CListNode (void);                      //  Destructor deletes node & data
    };


//=====================================================================================
//  Class:  CBasicList
//      This class implements a linked list, using CListNode objects to represent the
//      actual nodes in the list.
//=====================================================================================

class CBasicList
    {
    private:
        CListNode* pFirst;                  //  Points to first node in list
        CListNode* pLast;                   //  Pointer to the last node
        CListNode* pCurrent;                //  Node most currently accessed
        int NumEntries;                     //  How many items are in the list
        int CurrentIndex;                   //  Index of currently accessed
        void Remove (CListNode*);           //  This removes a given node by pointer

    public:
        CBasicList (void);                  //  Construct an empty list
        ~CBasicList (void);                 //  Destructor cleans out the list
        void RemoveNodes (void);            //  Delete all the nodes (not their data)
        void Insert (void*);                //  Dump item at tail of list
        void Insert (void*, int);           //  Insert item into sorted list
        void Remove (void*);                //  Remove data item from the list
        void* GetHead (void);               //  Go to first item in list
        void* GetTail (void);               //  Go to the last item
        void* GetNext (void);               //  Yeah, go to the next one
        void* GetPrevious (void);           //  and go to the previous one
        void* GetCurrent (void);            //  Pointer to current item
        void* GetObjWith (void*);           //  Get object with this ptr.
        void* GetObjNumber (int);           //  Go to item at this index
        int NumberOfObjWith (void*);        //  Get serial number of item in the list
        int HowMany (void);                 //  How many items in list
        int GetCurrentIndex (void);         //  Index of current item
        void DeleteData (void);             //  Calls delete for all data items
    };


//=====================================================================================
//  Class:  CBasicArray 
//      This class represents a most basic array object.  The array can be set up to 
//      hold any of several types of data.  It does so by providing a flexible indexing
//      method: just tell it the size of a data item in bytes, and it will allocate 
//      that many bytes for each data item and increment its indices by that many bytes
//      for each item in the array.  This object can configure itself in the following
//      ways, depending on the LogArrayType choice given by the user:
//        - Circular buffer:  When the end of the array is reached writing continues
//                            at the beginning.  The most recent data is kept.
//        - Expanding buffer: When the array is full, a new one is allocated and data
//                            continues to be taken.
//        - Finite buffer:    When the array becomes full, recording stops.
//      The access functions are deliberately made a little primitive - this is a base
//      class, and user functions should talk to classes derived from this one.
//=====================================================================================

class CBasicArray : public CBasicList
    {
    protected:
        unsigned DataSize;                  //  Size of data item, in bytes
        unsigned PointsTaken;               //  How many data have been taken so far?
        unsigned BufferSize;                //  Number of items in each buffer
        unsigned TotalSize;                 //  Items in whole array (all buffers)
        unsigned WriteIndex;                //  Index used for writing data in
        unsigned ReadIndex;                 //  Index used for reading it back out

    public:
        //  Constructor allocates memory for buffer; destructor frees it
        CBasicArray (unsigned, unsigned);
        ~CBasicArray (void);

        virtual void Flush (void);          //  Empty out the array and start over
        virtual void Rewind (void) { }      //  Base function for data logger array
        unsigned Expand (void);             //  Add another buffer to the array
        void* operator[] (unsigned);        //  Returns a pointer to a given item 
        void* operator[] (int);             //  Ditto, in case the user uses int index

        //  Virtual base functions: these will be used in the linear, circular, and
        //  expanding buffers which are derived from this base class
        virtual void* ReadPointer (void) { return (NULL); }
        virtual void* WritePointer (void) { return (NULL); }
        virtual void SetReadIndex (unsigned aNewIndex) { ReadIndex = 0; }
    };


//=====================================================================================
//  Class: CString 
//      A simple class to implement a buffer for character strings is implemented
//      here.  Actually it's mostly just to ease the process of dealing with strings 
//      of dynamically variable length.  Heck, I always thought it would be cool to 
//      write a CString anyway.  
//      The method of making the size dynamically adjustable is to make another object
//      of this same type if the current object overflows and keep a pointer to it.  
//=====================================================================================

class CString
    {
    private: 
        CString *NextString;            //  Next object, if one's needed for overflow 
        char *TheBuffer;                //  Buffer currently being written into 
        char *OutString;                //  Buffer holds string to be sent out 
        size_t BufferSize;              //  Size of this string's buffer in bytes 
        size_t SizeLeft;                //  How many bytes are stored in this instance 

        void GetString (char *);        //  Function adds buffer contents to output 
        void Flush (void);              //  Flushes buffers, thus emptying the string 

    public:
        CString (void);                 //  Default constructor just calls another
        CString (size_t);               //  Give it a size, it allocates a buffer
        ~CString (void);                //  Destructor frees up that buffer's memory 
        const char *GetString (void);   //  Returns string's contents in char. buffer 
        size_t GetSize (void);          //  Size of string, including overflow objects
        void WriteToFile (FILE *);      //  Write buffer's whole contents to a file 

        CString& operator= (const char *);      //  Operator replaces buffer w/new text
        CString& operator<< (const char *);     //  Insertion operators:  These oper- 
        CString& operator<< (char);             //  ators all append the given data 
        CString& operator<< (unsigned char);    //  to the string.  All except the 
        CString& operator<< (int);              //  (char *) version first convert 
        CString& operator<< (unsigned);         //  the data from whatever its native 
        CString& operator<< (short);            //  format might be into a character 
        CString& operator<< (unsigned short);   //  array, then append the array to 
        CString& operator<< (long);             //  the buffer of the string object.  
        CString& operator<< (unsigned long);
        CString& operator<< (float);
        CString& operator<< (double);
        CString& operator<< (long double);
    };


//=====================================================================================
//  Class: CFileObj
//      In order to make dealing with data files more streamlined, we here implement
//      a class which opens and closes a file and can give whoever needs it a pointer
//      to that file.  The file pointer can be used for normal C reading and writing.  
//      The object also contains a buffer so that writing can be done to the buffer, 
//      and the buffer will be flushed when the file is closed or when so requested.  
//      This allows information to be written to the file from within ISR's.  
//=====================================================================================

class CFileObj
    {
    private: 
        FILE *FileHandle;               //  File handle, in standard C format 
        CString *Buffer;                //  Buffer for storing data to be written 
        boolean NeedToClose;            //  TRUE if we need to close during delete()

    public:
        //  There's a default constructor, one which opens a file from a given file
        //  handle, and two which open files from file names - with and without buffers
        CFileObj (void) { }
        CFileObj (FILE *);
        CFileObj (const char *, const char *);
        CFileObj (const char *, const char *, size_t);
        ~CFileObj (void);

        void Flush (void);                      //  Method to flush buffer to file

        CFileObj& operator<< (const char *);    //  Insertion operators:  These oper- 
        CFileObj& operator<< (char);            //  ators all append the given data 
        CFileObj& operator<< (unsigned char);   //  to the text file.  All except the 
        CFileObj& operator<< (int);             //  (char *) version first convert 
        CFileObj& operator<< (unsigned);        //  the data from whatever its native 
        CFileObj& operator<< (short);           //  format might be into a character 
        CFileObj& operator<< (unsigned short);  //  string, then append the string to 
        CFileObj& operator<< (long);            //  the file buffer or file.  
        CFileObj& operator<< (unsigned long);
        CFileObj& operator<< (float);
        CFileObj& operator<< (double);
        CFileObj& operator<< (long double);
    };

