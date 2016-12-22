//*************************************************************************************
//  TR3_COMP.HPP or COMPCONV.HPP
//      In this file are macros and such to permit the scheduler, which was written
//      for Visual C++ version 1.5, to be compiled for other (better) compilers.
//      We give it different names for the TranRun3 and JJ Group Priority schedulers.
//
//  Note about DisableInterrupts() and EnableInterrupts()
//       If we're compiling in a mode that uses interrupts, define the ...Interrupts()
//       functions to be compatible with the compiler.  If not using interrupts, define
//       them as inline null functions which won't do anything.  The user can thus put
//       ...Interrupts() in his code and it will have no effect in non-interrupt modes
//
//  Version
//       3-16-95  JR   Original file
//       5-20-95  JR   Changed from CTL_COMP to TR3_COMP for use with Tranlog 3
//       5-20-95  JR   Primary compiler for Tranlog3 is Borland C++ 4.5
//*************************************************************************************


#ifndef COMPCONV_HPP
    #define  COMPCONV_HPP                   //  Variable to prevent multiple inclusions
#ifndef TR3_COMP_HPP
    #define  TR3_COMP_HPP


//-------------------------------------------------------------------------------------
//  Some Common Stuff
//      The macros which control mutual exclusion protection for real-time tasks (used
//      only when interrupt-driven multithreading is active) are the same for all of
//      the compilers, except

#if defined (TR_THREAD_MULTI) || defined (TR_THREAD_TIMER) || defined (TR_TIME_INT)
    #define USES_INTERRUPTS
#endif

#if !defined (USES_INTERRUPTS)
    #define  EnableInterrupts()
    #define  DisableInterrupts()
#endif


//-------------------------------------------------------------------------------------
//  Default - Borland C++ Version 4.5x and 5.0 
//      These macros allow the program to be compiled under Borland C++ Version 4 - 5,
//      which is currently our compiler of choice.

#if (__BORLANDC__ >= 0x460)
    #define  DELETE_ARRAY           delete []
#endif
#if (__BORLANDC__ >= 0x460) && defined (USES_INTERRUPTS)
    #define  ISR_POINTER(X)         void interrupt (*X)(...)
    #define  ISR_FUNCTIONDEF(X)     void interrupt X(...)
#endif
#if (__BORLANDC__ >= 0x460) && !defined (USES_INTERRUPTS)
    #define  ISR_POINTER(X)         void (*X)(...)
    #define  ISR_FUNCTIONDEF(X)     void X(...)
#endif


//-------------------------------------------------------------------------------------
//  For backward compatibility - Borland C++ Version 1.0
//      These macros are for Borland C++ Version 1.0.  This older version can be a bit
//	    more convenient to use for writing DOS applications

#if __BORLANDC__ == 0x200
    #define  ISR_POINTER(X)         void interrupt (*X)(...)
    #define  ISR_FUNCTIONDEF(X)     void interrupt X(...)
    #define  _dos_getvect	        getvect
    #define  _dos_setvect           setvect
    #define  DELETE_ARRAY           delete
#endif


//-------------------------------------------------------------------------------------
//  Stuff common to all the Borland versions

#if defined (__BORLANDC__) && defined (USES_INTERRUPTS)
    #define  EnableInterrupts()     enable()
    #define  DisableInterrupts()    disable()
#endif


//-------------------------------------------------------------------------------------
//  Useable - Microsoft Visual C++ Version 1.5
//      This compiler was used during original development of the program but we
//      changed to Borland C++ (which we prefer) later on.

#if defined (__MSVC__)
    #define  ISR_POINTER(X)         void (__interrupt *X)(void)
    #define  ISR_FUNCTIONDEF(X)     void __cdecl __interrupt __far X (void)

    #define  _NOCURSOR              0x2000
    #define  _NORMALCURSOR          0x0607
#endif
#if defined (__MSVC__) && defined (USES_INTERRUPTS)
    #define  EnableInterrupts()   _enable()
    #define  DisableInterrupts()  _disable()
#endif

#endif                                      //  End multiple-inclusion protection
#endif

