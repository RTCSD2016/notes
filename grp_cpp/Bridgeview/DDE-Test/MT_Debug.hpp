//*************************************************************************************
//  MT_Debug.hpp
//      This is the header file for a very simple system which allows the programmer
//      to record debugging information in a multithreading program.  The method of
//      storing simple text notes is intended to be very simple and fast.  Also, if
//      debugging mode isn't being used the code doesn't get compiled at all.
//
//  Revisions
//       8-19-97  JRR  Frustrated programmer creates this thing
//*************************************************************************************

#define MT_DEBUG_MODE                       //  If not debugging, #undefine it here

#ifdef MT_DEBUG_MODE
    extern char* DebugNotes;                //  Pointer to the main debugging string

    void InitDebugNotes (void);
    void TakeDebugNote (const char* aFormat, ...);
    void PrintDebugNotes (const char* aFileName);

#else                                       //  If not using debugging, make function
    #define  TakeDebugNote(x)               //  which would take notes do nothing
    #define  TakeDebugNote(x,y)
    #define  TakeDebugNote(x,y,z)
    #define  TakeDebugNote(x,y,z,a)
    #define  TakeDebugNote(x,y,z,a,b)
    #define  TakeDebugNote(x,y,z,a,b,c)
    #define  TakeDebugNote(x,y,z,a,b,c,d)
    #define  TakeDebugNote(x,y,z,a,b,c,d,e)
    #define  TakeDebugNote(x,y,z,a,b,c,d,e,f)
    #define  TakeDebugNote(x,y,z,a,b,c,d,e,f,g)

    #define  PrintDebugNotes(x)
#endif

