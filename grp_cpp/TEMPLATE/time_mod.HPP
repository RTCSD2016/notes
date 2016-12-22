//*************************************************************************************
//  Time_Mod.hpp
//      This file allows the user to define the scheduling mode used by the scheduler.
//      By leaving in and commenting out the #define directives below, you choose a
//      scheduling method, timekeeping method, and tick rate.
//
//  Revisions
//       8/19/96  DMA  Created original file
//       ?-??-??  who  Has been separated from Tasks.hpp because of definition
//                     conflicts with the operator interface code.
//*************************************************************************************

#ifndef TIME_MOD_HPP
    #define TIME_MOD_HPP

#include "clock.hpp"                        //  Time-keeping functions


//  Define interrupt mode here.  Choosing to #define ISR_SCHEDULING activates the
//  interrupt scheduler, which only works in a DOS application (not Windows).  If you
//  use interrupt scheduling, the rate at which timer interrupts occur will be
//  determined by the value of TICKRATE.

#undef ISR_SCHEDULING
//#define ISR_SCHEDULING

//  Define the timing mode  -- One (and only one) of the following should be active.
//  TIMEFREE uses the PC's internal hardware timer; this only works for a DOS
//  application (not Windows).  TIMEINTERNAL is 'calibrated time' in which the
//  scheduler assumes a fixed time interval between task scans - there is no time
//  measurement.

// #define TIMEFREE
#define TIMEINTERNAL
// #define TIME_NT

//  Tick Rate:  This is the time granularity if the free running timer is not used.
//  Each scheduler scan, the time is incremented by TICKRATE.  You can adjust this
//  value to make the time a decent approximation of the real time ('calibration').
//  A value of ~0.0002s works OK with most simulations; for interrupt scheduling in
//  DOS mode you should use a value between 1 and 50 ms.

#define TICKRATE 0.0002

//  These are prototypes of functions used by the scheduler to control interrupts
void DisableInterrupt(void);
void EnableInterrupt(void);

#endif // TIME_MOD_HPP

