//*************************************************************************************
//  MT_Debug.cpp
//      This is the source file for a very simple system which allows the programmer
//      to record debugging information in a multithreading program.  The method of
//      storing simple text notes is intended to be very, very simple and fast.
//
//  Revisions
//       8-19-97  JRR  Frustrated programmer creates this thing
//*************************************************************************************

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "MT_Debug.hpp"

#ifdef MT_DEBUG_MODE

static char* DebugNotes = NULL;             //  Pointer to buffer holding debug info
const size_t MT_DEBUG_BUFFER_SIZE = 32768;  //  How big is the buffer
static size_t CharsRecorded = 0;            //  How many characters have been saved
static size_t CharsNotRecorded = 0;         //  How much not saved for lack of space
static bool StringFull = false;             //  Flag indicating that buffer is full
static bool Array_OK = false;               //  Flag says array is OK to save data


//-------------------------------------------------------------------------------------
//  Function:  TakeDebugNote
//      This function appends the given data (which is supplied as a printf() format
//      variable-argument list) into the debugging string.

void TakeDebugNote (const char* aFormat, ...)
    {
    static char AddString[512];

    //  Process the variable-argument list by using vsprintf to print the user's
    //  format string plus data into the AddString buffer
    va_list Arguments;
    va_start (Arguments, aFormat);
    vsprintf (AddString, aFormat, Arguments);
    va_end (Arguments);

    //  If the data buffer isn't full, check to make sure the new string will fit;
    //  if it will fit, save the data.
    if ((StringFull == false) && (Array_OK == true))
        {
        if ((CharsRecorded += strlen (AddString)) >= MT_DEBUG_BUFFER_SIZE)
            StringFull = true;
        else
            strcat (DebugNotes, AddString);
        }
    //  If the data buffer is full, record how many characters we have lost
    else
        CharsNotRecorded += strlen (AddString);
    }


//-------------------------------------------------------------------------------------
//  Function:  InitDebugNotes
//      This function just prepares the debugging string for use.

void InitDebugNotes (void)
    {
    //  If no buffer has been allocated yet, allocate one now
    if (DebugNotes == NULL) DebugNotes = new char[MT_DEBUG_BUFFER_SIZE + 1];

    //  Now if no buffer is available, something is wrong, so don't save any data
    if (DebugNotes != NULL) Array_OK = true;

    //  Put a header on the file
    DebugNotes[0] = '\0';
    TakeDebugNote ("MT-Debug Information:\n");
    }


//-------------------------------------------------------------------------------------
//  Function:  PrintDebugNotes
//      This function prints the debugging string which was set up by InitDebugNotes()
//      and had junk written to it by TakeDebugNote() to a file.

void PrintDebugNotes (const char* aFileName)
    {
    FILE* aFile;
    if ((aFile = fopen (aFileName, "w")) != NULL)
        {
        if (Array_OK == false)
            fprintf (aFile, "ERROR:  Cannot allocate %d byte debugging data array\n",
                     MT_DEBUG_BUFFER_SIZE);
        else
            {
            char* pRead = DebugNotes;
            while (*pRead)
                fputc ((int)*pRead++, aFile);

            fprintf (aFile, "\nEnd of Debugging Information.  Recorded %d bytes.\n",
                     CharsRecorded);
            if (CharsNotRecorded > 0)
                fprintf (aFile, "Warning: %d bytes not recorded!\n",
                         CharsNotRecorded);
            }

        fclose (aFile);
        }
    if (DebugNotes != NULL) delete [] DebugNotes;
    }

#endif

