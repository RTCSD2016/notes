//*************************************************************************************
//  CompConv.hpp / TranRun4.hpp
//      In this file are macros and such to permit the scheduler, which was written
//      for Visual C++ version 1.5, to be compiled for other (i.e. better) compilers.
//
//  Note about DisableInterrupts() and EnableInterrupts()
//      If we're compiling in a mode that uses interrupts, define the ...Interrupts()
//      functions to be compatible with the compiler.  If not using interrupts, define
//      them as inline null functions which won't do anything.  The user can thus put
//      ...Interrupts() in his code and it will have no effect in non-interrupt modes
//      (including Windows 95/NT multithreading where our code doesn't control the
//      interrupt hardware because the operating system takes care of it).
//
//  Copyright (c) 1994-1997 by D.M.Auslander and J.R.Ridgely
//      May be used and distributed for any non-commercial purposes as long as this
//      copyright notice is included.
//       3-16-95  JR   Original file
//       5-20-95  JR   Changed from CTL_COMP to TR3_COMP for use with Tranlog 3
//       5-20-95  JR   Primary compiler for Tranlog3 is Borland C++ 4.5
//      12-01-96  JR   Added stuff for compatibility with BC++ 5.0
//       8-17-97  JR   Improved compatibility between TR4 and GP schedulers
//*************************************************************************************

#ifndef COMPCONV_HPP
    #define COMPCONV_HPP
#ifndef TR4_COMP_HPP
    #define  TR4_COMP_HPP                   //  Variable to prevent multiple inclusions


//-------------------------------------------------------------------------------------
//  Some Common Stuff
//      Disabling and enabling interrupts is only used in multithreading DOS modes.
//      We turn it off in other modes by defining interrupt control macros to expand
//      to nothing.  We do not use these functions for Windows 95/NT multithreading. 

#if defined (TR_THREAD_MULTI) || defined (TR_TIME_INT)
    #define USES_INTERRUPTS
#endif

#if !defined (USES_INTERRUPTS)
    #define  EnableInterrupts()
    #define  DisableInterrupts()
    #define  EnableInterrupt()
    #define  DisableInterrupt()
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
    #define  EnableInterrupt()     enable()
    #define  DisableInterrupt()    disable()
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
    #define  EnableInterrupt()   _enable()
    #define  DisableInterrupt()  _disable()
#endif

#endif
#endif                                      //  End multiple-inclusion protection

