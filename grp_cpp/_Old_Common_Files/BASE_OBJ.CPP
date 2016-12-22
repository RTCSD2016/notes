//*************************************************************************************
//  BASE_OBJ.CPP
//      This file contains the implementation (member functions and stuff) for the
//      basic general-purpose classes developed for the CTL_EXEC project.
//
//  Copyright (c) 1994, D.M.Auslander and J.R.Ridgely
//      May be used and distributed for any non-commercial purposes as long as this
//      copyright notice is included.
//
//  Version
//      11-01-94  DMA  Original program
//       5-26-95   JR  Updated for use with TranRun3
//       7-18-95   JR  Class base_obj eliminated and list objects revised 
//*************************************************************************************
    
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <base_obj.hpp>


//=====================================================================================
//  Class:  CListNode
//      This is a template for the nodes in a linked list.  These nodes contain poin-
//      ters so that they can point to one another (the list is doubly linked) and
//      data pointers so that each node can hold its own data object.
//=====================================================================================

//-------------------------------------------------------------------------------------
//  Constructor:  CListNode
//      Here we create a node which can be inserted into the generic linked list which
//      is implemented in class CBasicList below.

CListNode::CListNode (CListNode* aPrev, CListNode* aNext, void* aData, int aKey,
                      CBasicList* aParent)
    {
    pPrevNode = aPrev;
    pNextNode = aNext;
    pNodeData = aData;
    SortKey = aKey;
    pParent = aParent;
    }


//-------------------------------------------------------------------------------------
//  Destructor:  CListNode
//      Here we create a node which can be inserted into the generic linked list which
//      is implemented in class CBasicList below.

CListNode::~CListNode (void)
    {
    }


//=====================================================================================
//  Class:  CBasicList
//      This class implements a linked list, using CListNode objects to represent the
//      actual nodes in the list.
//=====================================================================================

//-------------------------------------------------------------------------------------
//  Constructor:  CBasicList
//      This constructor creates an empty list object which is ready to hold pointers
//      to items.

CBasicList::CBasicList (void)
    {
    pFirst = NULL;                          //  Set all the object pointers to NULL
    pLast = NULL;
    pCurrent = NULL;
    NumEntries = 0;                         //  Number of entries is obviously 0 now
    CurrentIndex = 0;                       //  And index of current item is zero too
    }


//-------------------------------------------------------------------------------------
//  Destructor:  CBasicList
//      This destructor goes through the list and removes the items in it one by one.

CBasicList::~CBasicList (void)
    {
    RemoveNodes ();
    }


//-------------------------------------------------------------------------------------
//  Function:  RemoveNodes
//      This function goes through the list and removes the items in it one by one.

void CBasicList::RemoveNodes (void)
    {
    CListNode* pTemp;               //  Temporarily store pointer to node being deleted


    //  Begin at the head of the list.  Delete every node, then go get the next one
    //  Note that this deletes the node objects but NOT the data stored by each node
    pCurrent = pFirst;
    while (pCurrent != NULL)
        {
        pTemp = pCurrent;                   //  Save a pointer to the current object,
        pCurrent = pCurrent->pNextNode;     //  then get a new pointer to the next one
        delete (pTemp);                     //  in the list and zap the current one
        }

    pFirst = NULL;                  //  There's no more data, so null the pointers
    pLast = NULL;
    pCurrent = NULL;
    NumEntries = 0;
    CurrentIndex = 0;
    }


//-------------------------------------------------------------------------------------
//  Function:  Insert (unsorted)
//      This function inserts an object into the list, placing it at the tail without
//      sorting.  The sort index of this item is set to a default sort of 0.

void CBasicList::Insert (void* pNewData)
    {
    CListNode* pNewNode;                    //  Pointer to newly created node object


    //  If the list is empty, put the new object at the head...tail...whatever
    if (pFirst == NULL)
        {
        pNewNode = new CListNode (NULL, NULL, pNewData, 0, this);
        pFirst = pNewNode;
        pLast = pNewNode;
        pCurrent = pNewNode;
        CurrentIndex = 0;
        }
    //  The list already has entries, so put this new thing at the end
    else
        {
        pNewNode = new CListNode (pLast, NULL, pNewData, 0, this);
        pLast->pNextNode = pNewNode;
        pLast = pNewNode;
        }

    //  Regardless of whether there were objects before or not, there's one more entry
    NumEntries++;
    }


//-------------------------------------------------------------------------------------
//  Function:  Insert (sorted)
//      This function inserts an object into the list at a position determined by its
//      sort index.  

void CBasicList::Insert (void* pNewData, int aSortKey)
    {
    CListNode* pNewNode;                    //  Pointer to newly created node object
    CListNode* pCur;                        //  Pointer for stepping through the list


    //  If the list is empty, put the new object in the only possible place
    if (pFirst == NULL)
        {
        pNewNode = new CListNode (NULL, NULL, pNewData, aSortKey, this);
        pFirst = pNewNode;
        pLast = pNewNode;
        pCurrent = pNewNode;
        CurrentIndex = 0;
        }
    //  The list already has entries.  See if this item has the highest sort key; in
    //  this case it goes at the head 
    else
        {
        pCur = pFirst;
        if (aSortKey > pCur->SortKey)
            {
            pNewNode = new CListNode (NULL, pCur, pNewData, aSortKey, this);
            pCur->pPrevNode = pNewNode;
            pFirst = pNewNode;
            }
        //  OK, this isn't going to be the first item; go find where it can be placed
        else
            {
            while ((pCur->pNextNode != NULL) && (aSortKey < pCur->pNextNode->SortKey))
                pCur = pCur->pNextNode;

            //  pCurrent points to the last item whose sort key is less than new one
            pNewNode = new CListNode (pCur, pCur->pNextNode, pNewData, aSortKey, this);

            //  If there's an item following this in the list, set its previous node
            //  pointer to point to this item; also set next node pointer of current
            if (pCur->pNextNode != NULL)
                (pCur->pNextNode)->pPrevNode = pNewNode;
            pCur->pNextNode = pNewNode;
            }
        }

    //  Update the counter to reflext one more entry in the list 
    NumEntries++;
    }


//-------------------------------------------------------------------------------------
//  Function:  Remove (given a data pointer)
//      This function removes an object from the linked list and resets the list
//      pointers so that the list's integrity is maintained.  If the current-object
//      pointer pointed to this object, it is set to point to the next one.

void CBasicList::Remove (void* KillData)
    {
    CListNode* pCur;                        //  Pointer for stepping through the list


    //  Find the object in the list which has this pointer - just like GoToObjWith()
    pCur = pFirst;
    while (pCur->pNodeData != KillData)
        {
        pCur = pCur->pNextNode;
        if (pCur == NULL)                   //  If we can't find it, there's nothing
            return;                         //  to delete so just return 
        }

    //  Now that we have a pointer to the node containing the data to be removed,
    //  call the function which does the actual deleting
    Remove (pCur);
    }


//-------------------------------------------------------------------------------------
//  Function:  Remove (given a node pointer)
//      This function removes the object whose node is pointed to by the given pointer.
//      The user wouldn't usually call this because users only deal with pointers to
//      data, not pointers to node objects.

void CBasicList::Remove (CListNode* pKillNode)
    {
    //  If this is the furst or last object, fix the pointer which points to it
    if (pKillNode == pFirst)
        pFirst = pKillNode->pNextNode;
    if (pKillNode == pLast)
        pLast = pKillNode->pPrevNode;

    //  Reset the pointers of the previous and next objects so that they can find each
    //  other without going through this object
    if (pKillNode->pPrevNode != NULL)
        (pKillNode->pPrevNode)->pNextNode = pKillNode->pNextNode;
    if (pKillNode->pNextNode != NULL)
        (pKillNode->pNextNode)->pPrevNode = pKillNode->pPrevNode;

    //  If the current object pointer points to this thing, set it to the next one or,
    //  if there is no next one, the previous one; if there's no other node it's NULL
    if (pCurrent == pKillNode)
        if (pKillNode->pNextNode == NULL)
            pCurrent = pKillNode->pPrevNode;
        else
            pCurrent = pKillNode->pNextNode;

    //  Delete the node object
    delete pKillNode;
    }


//-------------------------------------------------------------------------------------
//  Function:  GoToHead
//      In this function we just set the current list pointer to point to the first
//      item in the list and return that pointer.

void* CBasicList::GetHead (void)
    {
    pCurrent = pFirst;
    CurrentIndex = 0;

    if (pCurrent != NULL)
        return (pCurrent->pNodeData);
    else
        return (NULL);
    }


//-------------------------------------------------------------------------------------
//  Function:  GoToTail
//      Here we move the current-object pointer to the tail of the list and return it.

void* CBasicList::GetTail (void)
    {
    pCurrent = pLast;
    CurrentIndex = NumEntries - 1;

    if (pCurrent != NULL)
        return (pCurrent->pNodeData);
    else
        return (NULL);
    }


//-------------------------------------------------------------------------------------
//  Function:  GoToNext
//      This function moves the current-object pointer to the next item in the list
//      and returns it, if there *is* a next item.  If there isn't, it leaves the
//      pointer where it was and returns NULL.

void* CBasicList::GetNext (void)
    {
    //  If the current pointer points to an object and its next-object pointer does
    //  also, move the pointer to the next object and return the new pointer
    if ((pCurrent != NULL) && (pCurrent->pNextNode != NULL))
        {
        pCurrent = pCurrent->pNextNode;
        CurrentIndex++;
        return (pCurrent->pNodeData);
        }

    //  If either of those pointers was NULL, we can't go to any next object
    return (NULL);
    }


//-------------------------------------------------------------------------------------
//  Function:  GoToPrevious
//      This function moves to the previous object in the list if it can.

void* CBasicList::GetPrevious (void)
    {
    //  Return a pointer to the previous object if one is availabubble
    if ((pCurrent != NULL) && (pCurrent->pPrevNode != NULL))
        {
        pCurrent = pCurrent->pPrevNode;
        CurrentIndex--;
        return (pCurrent->pNodeData);
        }

    //  OK, we don't have a previous item, so return NULL
    return (NULL);
    }


//-------------------------------------------------------------------------------------
//  Function:  GoToCurrent
//      This function is used to get a pointer to the list object which is currently
//      being pointed to, unless the list is empty in which case it returns NULL.

void* CBasicList::GetCurrent ()
    {
    if (pCurrent != NULL)
        return (pCurrent->pNodeData);
    else
        return (NULL);
    }


//-------------------------------------------------------------------------------------
//  Function:  GoToObjWith
//      Here we hunt down an object in the list with the given pointer, unless it's
//      not in there, in which case we return NULL.

void* CBasicList::GetObjWith (void* aPointer)
    {
    void* pSomeData;                    //  Pointer to any old node being looked at


    //  Begin at the head of the list.  Check every node.  If its data pointer matches
    //  the pointer given in this function's argument, quit searching and return it
    pSomeData = GetHead ();
    while (pSomeData != NULL)
        {
        if (pSomeData == aPointer)
            return (aPointer);
        pSomeData = GetNext ();
        }

    //  If we get here, we never found the item in question, so give up
    return (NULL);
    }


//-------------------------------------------------------------------------------------
//  Function:  NumberOfObjWith
//      Here we hunt down an object in the list with the given pointer and return the
//      sequence number in the list of that item.  It the item's not there return -1.

int CBasicList::NumberOfObjWith (void* aPointer)
    {
    void* pSomeData;                    //  Pointer to any old node being looked at


    //  Begin at the head of the list.  Check every node.  If its data pointer matches
    //  the pointer given in this function's argument, quit searching and return it
    pSomeData = GetHead ();
    int Counter = 0;
    while (pSomeData != NULL)
        {
        if (pSomeData == aPointer)
            return (Counter);
        pSomeData = GetNext ();
        Counter++;
        }

    //  If we get here, we never found the item in question, so give up
    return (-1);
    }


//-------------------------------------------------------------------------------------
//  Function:  GoToNumber
//      This function goes and finds the item at the given number, i.e. the n-th item
//      in the list (if that n-th item exists).  It returns a pointer to that item's
//      data, unless the item doesn't exist in which case it returns NULL.

void* CBasicList::GetObjNumber (int aNumber)
    {
    CListNode* pCur;                    //  Pointer to current item in the list
    int Counter;                        //  Integer counter for getting to item #N


    //  Begin at head of list.  Count through aNumber items, unless we hit the end
    pCur = pFirst;
    for (Counter = 0; Counter < aNumber; Counter++)
        {
        if (pCur == NULL)
            return (NULL);
        pCur = pCur->pNextNode;
        }

    //  If we get here, there probably is an N-th node, so return its data pointer
    if (pCur != NULL)
        return (pCur->pNodeData);
    else
        return (NULL);
    }


//-------------------------------------------------------------------------------------
//  Function:  HowMany
//      This function returns the number of items in the list.

int CBasicList::HowMany (void)
    {
    return (NumEntries);
    }


//-------------------------------------------------------------------------------------
//  Function:  GetCurrentIndex
//      Here we return the index number of the currently accessed list item.

int CBasicList::GetCurrentIndex (void)
    {
    return (CurrentIndex);
    }


//=====================================================================================
//  Class:  CBasicArray
//      This class implements an array which stores data of any type (as long as the
//      size of a data element can be fixed).  It's intended to be used as a base
//      class for arrays with more civilized interfaces.
//=====================================================================================

//-------------------------------------------------------------------------------------
//  Constructor:  CBasicArray
//      This constructor sets up a logging array of the given type with a starting
//      buffer of the given size.

CBasicArray::CBasicArray (unsigned aDataSize, unsigned aArraySize)
    {
    BufferSize = aArraySize;      //  Save number of elements
    TotalSize = 0;                //  Begin with no buffer ready
    DataSize = aDataSize;         //  Remember how big the data is
    Expand ();                    //  This allocates the first block of memory
    }


//-------------------------------------------------------------------------------------
//  Destructor:  ~CBaseArray
//      This destructor frees memory which has been used by the arrays in the list.

CBasicArray::~CBasicArray (void)
    {
    //  Delete all the buffers
    for (char* pBuf = (char*)GetHead (); pBuf != NULL; pBuf = (char*)GetNext())
        DELETE_ARRAY pBuf;
    }


//-------------------------------------------------------------------------------------
//  Function:  Flush
//      This function resets the pointers to the beginning of the data, effectively
//      restoring the array to its 'empty' state, ready to accept some data.

void CBasicArray::Flush (void)
    {
    //  Delete all the buffers and zap the list nodes
    for (char* pBuf = (char*)GetHead (); pBuf != NULL; pBuf = (char*)GetNext())
        DELETE_ARRAY pBuf;
    CBasicList::RemoveNodes ();

    //  Create the first buffer and save a pointer to it in the buffer pointer list
    char* pNewBuf = new char[BufferSize * DataSize];
    if (pNewBuf != NULL)
        {
        CBasicList::Insert ((void*)(pNewBuf));
        TotalSize = BufferSize;
        }
    }


//-------------------------------------------------------------------------------------
//  Function:  Expand
//      This function is used to add another buffer onto the array.  It returns the
//      new total size of the array if everything went OK and 0 if there are problems.

unsigned CBasicArray::Expand (void)
    {
    char* pNewBuf = new char[BufferSize * DataSize];
    if (pNewBuf != NULL)
        {
        CBasicList::Insert ((void*)(pNewBuf));
        TotalSize += BufferSize;
        return (TotalSize);
        }
    else
        return (0);
    }


//-------------------------------------------------------------------------------------
//  Operator:  []
//      This operator is somewhat analogous to the array indexing operator [] used for
//      normal arrays, except that it returns a *pointer* to the item at the given
//      index.  (It must, because the array might be storing any kind of data.)  If
//      you ask for an element which isn't in the range of the array, [] returns NULL.
//      Ditto if the data you're asking for hasn't been initialized yet. 

void* CBasicArray::operator[] (int aIndex)
    {
    if (aIndex >= 0)
        return (operator[] ((unsigned)(aIndex)));
    else
        return (NULL);
    }


void* CBasicArray::operator[] (unsigned aIndex)
    {
    if (aIndex >= TotalSize)
        return (NULL);

    //  Find which buffer the item is in, and get a pointer to that buffer 
    void* WhichBuffer = GetObjNumber (aIndex / BufferSize);
    if (WhichBuffer == NULL)
        return (NULL);

    //  Go to the element (within the buffer) which we want; return a pointer to it
    return ((void*)(((char*)WhichBuffer) + ((aIndex % BufferSize) * DataSize)));
    }


//=====================================================================================
//  Class: CString
//      A simple class to implement a buffer for character strings is implemented
//      here.  Actually it's mostly just to ease the process of dealing with strings
//      of dynamically variable length.
//=====================================================================================

//-------------------------------------------------------------------------------------
//  Constructor: CString
//      If someone calls the constructor without specifying a size, call the real
//      CString constructor to create a CString object with a 256 byte buffer.

CString::CString (void)
    {
    CString ((size_t)256);            //  one to start off with a 1KB buffer
    }


//-------------------------------------------------------------------------------------
//  Constructor: CString
//      This constructor creates a CString object with a buffer of the given size.

CString::CString (size_t aBufSize)
    {
    //  Allocate a starting buffer, and any 'next' buffer does not exist yet
    TheBuffer = new char[aBufSize];
    NextString = NULL;
    OutString = NULL;

    if (TheBuffer == NULL)              //  Buffer's not allocated!  Don't use it 
        BufferSize = 0;
    else                                //  Buffer's OK; make it contain empty string
        {
        BufferSize = aBufSize - 1;      //  Leave a byte of space for terminating '\0'
        strcpy (TheBuffer, "\0");
        }
    SizeLeft = BufferSize;              //  This doesn't include the '\0' at the end 
    }


//-------------------------------------------------------------------------------------
//  Destructor: ~CString 
//      This one clears out the memory used by the CString object. 

CString::~CString (void)
    {
    Flush ();                   //  Free all memory used by this and overflow CStrings 

    DELETE_ARRAY TheBuffer;     //  Delete the entire array
    }


//-------------------------------------------------------------------------------------
//  Function: Flush 
//      This function clears out the memory used by the CString object and leaves us 
//      with an empty string.  In doing so it deletes any overflow objects which may 
//      have been created to hold the extra parts of this string.

void CString::Flush (void)
    {
    if (OutString != NULL)      //  Frees string used to send out text if it exists 
        {
        DELETE_ARRAY OutString;
        OutString = NULL;
        }

    if (NextString != NULL)     //  Deletes overflow object, if there is one 
        {
        delete NextString;
        NextString = NULL;
        }

    strcpy (TheBuffer, "\0");   //  Return member data to starting values 
    SizeLeft = BufferSize;
    }


//-------------------------------------------------------------------------------------
//  Function: GetString (public version, returns character pointer) 
//      This function returns a pointer to a newly allocated character string which
//      contains all the stuff which was written to this string, including the over-
//      flow which went to any other CString objects we've created to hold it.  

const char *CString::GetString (void)
    {
    size_t TotalSize;                   //  How many bytes in the entire string


    //  If a string has been sent out previously, free its memory now 
    if (OutString != NULL)
        delete (OutString);

    //  Find out how big the output string must be, then allocate for it 
    TotalSize = GetSize ();
    OutString = new char[TotalSize];
    if (OutString == 0)
        return ("*** ERROR: Can't allocate memory for string ***\n");

    //  Now fill the output string with the string in this (and overflow) buffer(s) 
    strcpy (OutString, TheBuffer);
    if (NextString != NULL)
        NextString->GetString (OutString);

    return (OutString);
    }


//-------------------------------------------------------------------------------------
//  Function: GetString (overloaded to take character pointer parameter)
//      This function concatentates the contents of the buffer in this object instance 
//      into the character buffer which was passed to it.  There had better be room.  

void CString::GetString (char *OutString)
    {
    //  Put the contents of this object's buffer into the output 
    strcat (OutString, TheBuffer);

    //  Check to see if there's another object with more stuff; if so, get it 
    if (NextString != NULL)
        NextString->GetString (OutString);
    }


//-------------------------------------------------------------------------------------
//  Function: GetSize
//      This function returns the size of this string in bytes.  

size_t CString::GetSize (void)
    {
    if (NextString == NULL)
        return (BufferSize - SizeLeft + 1);
    else
        return (BufferSize - SizeLeft + NextString->GetSize ());
    }


//-------------------------------------------------------------------------------------
//  Function: WriteToFile
//      This function writes the contents of the string to the file whose handle is 
//      given.  

void CString::WriteToFile (FILE *OutFile)
    {
    char *OutChar;                      //  Character currently being written to file 


    //  Write the contents of this object to the output file 
    for (OutChar = TheBuffer; *OutChar != '\0'; OutChar++)
        fputc (*OutChar, OutFile);

    //  If there is an overflow object, write its contents to the output file too 
    if (NextString != NULL)
        NextString->WriteToFile (OutFile);
    }


//-------------------------------------------------------------------------------------
//  Operator: =
//      Here we overload the assignment operator so that character strings may be 
//      copied into the CString object very easily.  Also the << operator may follow
//      an assignment, as "=" returns an object reference.  

CString& CString::operator= (const char *NewString)
    {
    Flush ();               //  Empty this string of all contents 
    (*this) << NewString;   //  Add the new character string into this object 

    return (*this);
    }


//-------------------------------------------------------------------------------------
//  Operator: << (for character pointer) 
//      Overloaded left-shift operator to add character arrays into a CString object 
//      in much the same way as the << operator works in cout.  

CString& CString::operator<< (const char *StrToAdd)
    {
    size_t BytesCopied;                 //  How many bytes copied into buffer this time 
    size_t AddLength;                   //  Length of the string given in 'StrToAdd'


    //  If this object is already full and we have created another object to hold the 
    //  overflow, just send the string along to that next object 
    if (NextString != NULL)
        (*NextString) << (char *)StrToAdd;

    //  If this string object's buffer couldn't be allocated, we've filled memory, so 
    //  give up on trying to store further items - just dump them (it's safer this way!) 
    //  If there's room, copy as much as we can into buffer of this CString object.  If
    //  there's still more data to be held, make a new object to hold it.  
    else if (BufferSize > 0)
        {
        AddLength = strlen (StrToAdd);                  //  How big is string to store 
        BytesCopied = minimum (AddLength, SizeLeft);    //  How many bytes we'll copy 
        strncat (TheBuffer, StrToAdd, SizeLeft);        //  Copy as much as we can 
        SizeLeft -= BytesCopied;                        //  How much space is left 

        //  If there's uncopied text left over, make a new CString to hold it and add 
        //  in the remaining part of the string 
        if (SizeLeft <= 0)
            {
            NextString = new CString ((size_t)(BufferSize + 1));
            if (NextString != NULL)
                (*NextString) << (StrToAdd + BytesCopied);
            }
        }

    return (*this);
    }


//-------------------------------------------------------------------------------------
//  Operators: << (for everything except character pointers) 
//      These overloaded add-accumulate operators convert the given data into a char-
//      acter string and then call the character string version to appends it to the 
//      file object.  COMPILER DEPENDENCY NOTE:  Visual C++ doesn't support the normal 
//      itoa() function, so you'll have to make changes if you port these functions.  

CString& CString::operator<< (char aChar)
    {
    char aBuf[4];                   //  Room for any possible character as a string 


    itoa ((int)aChar, aBuf, 10);   //  Convert the character to a signed integer,
    *this << aBuf;                  //  then add it to the string 
    return (*this);                 //  Return a reference to the file object 
    }

CString& CString::operator<< (unsigned char aChar)
    {
    char aBuf[4];                               //  Room for any character as a string 

    itoa ((unsigned int)aChar, aBuf, 10);       //  Convert this one to *unsigned* int
    *this << aBuf; 
    return (*this);
    }

CString& CString::operator<< (int aNumber)
    {
    char aBuf[18];                  //  Give ample room for any possible int as string 

    itoa (aNumber, aBuf, 10);      //  Use itoa() to convert integer to string
    *this << aBuf;                  //  Call << to add string to 'this' file object
    return (*this);
    }

CString& CString::operator<< (unsigned int aNumber)
    {
    char aBuf[36];

    //  An odd way to get an unsigned int converted into a character string without
    //  allowing the value to represent a negative number, even if the first bit is 1
    *this << ultoa (((unsigned long)((unsigned)aNumber)), aBuf, 10);
    return (*this);
    }

CString& CString::operator<< (short aNumber)
    {
    char aBuf[36];

    //  This conversion should work on any system where shorts are shorter than longs
    //  (though you may need to hunt for a replacement for the _ltoa() function)
    *this << ltoa ((long)aNumber, aBuf, 10);
    return (*this);
    }

CString& CString::operator<< (unsigned short aNumber)
    {
    char aBuf[36];

    *this << ultoa (((unsigned long)aNumber), aBuf, 10);
    return (*this);
    }

CString& CString::operator<< (long aNumber)
    {
    char aBuf[36];

    *this << ltoa (aNumber, aBuf, 10);
    return (*this);
    }

CString& CString::operator<< (unsigned long aNumber)
    {
    char aBuf[36];

    *this << ultoa (aNumber, aBuf, 10);
    return (*this);
    }

CString& CString::operator<< (float aNumber)
    {
    char aBuf[36];

    sprintf (aBuf, "%g", aNumber);
    *this << aBuf;
    return (*this);
    }

CString& CString::operator<< (double aNumber)
    {
    char aBuf[36];

    sprintf (aBuf, "%lg", aNumber);
    *this << aBuf;
    return (*this);
    }

CString& CString::operator<< (long double aNumber)
    {
    char aBuf[36];

    sprintf (aBuf, "%Lg", aNumber);
    *this << aBuf;
    return (*this);
    }


//=====================================================================================
//  Class: CFileObj
//      In order to make dealing with data files more streamlined, we here implement
//      a class which opens and closes a file and can give whoever needs it a pointer
//      to that file.  The file pointer can be used for normal C reading and writing.
//      Usually, the file buffer is used so writing can be done quickly in real time 
//      and the file I/O can be done later.  For the user, writing to the file object 
//      is done with << operators in a manner similar to the way you use cout.  
//=====================================================================================

//-------------------------------------------------------------------------------------
//  Constructor: CFileObj (version for file with buffer) 
//      This constructor opens up a data file to which we can write, allocates space 
//      for a buffer in which to store stuff, and sets the allow-writing flag.  If the
//      buffer size is zero, it sets the use-no-buffer flag.  

CFileObj::CFileObj (const char *aName, const char *aMode, size_t BufferSize)
    {
    NeedToClose = FALSE;                        //  Unless we open a file, why close it

    FileHandle = fopen (aName, aMode);          //  Try opening that file
    if (FileHandle != NULL)
        NeedToClose = TRUE;

    Buffer = NULL;                              //  Default: no buffer is used
    if (BufferSize > 0)                         //  If we are using a buffer
        Buffer = new CString (BufferSize);
    }


//-------------------------------------------------------------------------------------
//  Constructor: CFileObj (no-buffer version)
//      This constructor opens up a data file to which we can write and sets the allow
//      writing flag.  It also sets the use-no-buffer flag.

CFileObj::CFileObj (const char *aName, const char *aMode)
    {
    NeedToClose = FALSE;                        //  Unless we open a file, why close it

    Buffer = NULL;                              //  Don't use a buffer at all 
    FileHandle = fopen (aName, aMode);
    if (FileHandle != NULL)                     //  But still make sure the file's open
        NeedToClose = TRUE;
    }


//-------------------------------------------------------------------------------------
//  Constructor: CFileObj (given a file pointer) 
//      This constructor creates a CFileObj object which just writes to the file whose
//      handle is given in the argument without using a buffer.  The file should 
//      already be open.  

CFileObj::CFileObj (FILE *aFile)
    {
    NeedToClose = FALSE;                    //  Someone else's file; they'll close it
    Buffer = NULL;
    FileHandle = aFile;
    }


//-------------------------------------------------------------------------------------
//  Destructor: ~CFileObj
//      This destructor closes the file (flushing the file buffer automatically to 
//      disk) and frees the CString (memory buffer) memory, if one is used.  

CFileObj::~CFileObj (void)
    {
    Flush ();                   //  Get buffer contents into file 

    if (NeedToClose == TRUE)    //  Then close the file 
        fclose (FileHandle);

    if (Buffer != NULL)         //  And delete the buffer
        delete (Buffer);
    }


//-------------------------------------------------------------------------------------
//  Function: Flush 
//      This function flushes the contents of the file buffer to the disk file.  
//      Having done so it also clears out the buffer.  

void CFileObj::Flush (void)
    {
    if ((Buffer != NULL) && (FileHandle != NULL))   //  Make sure file & buffer are OK
        {
        Buffer->WriteToFile (FileHandle);
        *Buffer = "";
        }
    }


//-------------------------------------------------------------------------------------
//  Operator: << (for character pointer) 
//      Overloaded left-shift operator to add character strings into a file object in 
//      the same way as the << operator works in cout.  

CFileObj& CFileObj::operator<< (const char *NewString)
    {
    if (Buffer != NULL)             //  If we're using the file in buffered mode, 
        *Buffer << NewString;       //  "add" the text into the CString buffer object 

    else if (FileHandle != NULL)
        while (*NewString != '\0')              //  If not in buffered mode, write 
            fputc (*NewString++, FileHandle);   //  directly to the file 

    return (*this);                 //  Return a reference to the file object 
    }


//-------------------------------------------------------------------------------------
//  Operators: << (for everything except character pointers) 
//      These overloaded add-accumulate operators convert the given data into a char- 
//      acter string and then call the character string version to appends it to the 
//      file object.  COMPILER DEPENDENCY NOTE:  Visual C++ doesn't support the normal 
//      itoa() function, so you'll have to make changes if you port these functions.  

CFileObj& CFileObj::operator<< (char aChar)
    {
    char aBuf[4];                   //  Room for any possible character as a string 


    itoa ((int)aChar, aBuf, 10);    //  Convert the character to a signed integer,
    *this << aBuf;                  //  then add it to the string
    return (*this);                 //  Return a reference to the file object
    }

CFileObj& CFileObj::operator<< (unsigned char aChar)
    {
    char aBuf[4];                               //  Room for any character as a string

    itoa ((unsigned int)aChar, aBuf, 10);       //  Convert this one to *unsigned* int
    *this << aBuf;
    return (*this);
    }

CFileObj& CFileObj::operator<< (int aNumber)
    {
    char aBuf[18];                  //  Give ample room for any possible int as string 

    itoa (aNumber, aBuf, 10);       //  Use itoa() to convert integer to string
    *this << aBuf;                  //  Call << to add string to 'this' file object
    return (*this);
    }

CFileObj& CFileObj::operator<< (unsigned int aNumber)
    {
    char aBuf[36];

    //  An odd way to get an unsigned int converted into a character string without
    //  allowing the value to represent a negative number, even if the first bit is 1
    *this << ultoa (((unsigned long)((unsigned)aNumber)), aBuf, 10);
    return (*this);
    }

CFileObj& CFileObj::operator<< (short aNumber)
    {
    char aBuf[36];

    //  This conversion should work on any system where shorts are shorter than longs
    //  (though you may need to hunt for a replacement for the _ltoa() function)
    *this << ltoa ((long)aNumber, aBuf, 10);
    return (*this);
    }

CFileObj& CFileObj::operator<< (unsigned short aNumber)
    {
    char aBuf[36];

    *this << ultoa (((unsigned long)aNumber), aBuf, 10);
    return (*this);
    }

CFileObj& CFileObj::operator<< (long aNumber)
    {
    char aBuf[36];

    *this << ltoa (aNumber, aBuf, 10);
    return (*this);
    }

CFileObj& CFileObj::operator<< (unsigned long aNumber)
    {
    char aBuf[36];

    *this << ultoa (aNumber, aBuf, 10);
    return (*this);
    }

CFileObj& CFileObj::operator<< (float aNumber)
    {
    char aBuf[36];

    sprintf (aBuf, "%g", aNumber);
    *this << aBuf;
    return (*this);
    }

CFileObj& CFileObj::operator<< (double aNumber)
    {
    char aBuf[36];

    sprintf (aBuf, "%lg", aNumber);
    *this << aBuf;
    return (*this);
    }

CFileObj& CFileObj::operator<< (long double aNumber)
    {
    char aBuf[36]; 

    sprintf (aBuf, "%Lg", aNumber);
    *this << aBuf; 
    return (*this);
    }
    
    
