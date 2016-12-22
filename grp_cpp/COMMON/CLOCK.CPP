// Functions for handling time keeping for Borland C++ 4.5 and greater --
//    tested on v5.0
//  based control programs.  The timing modes are set by defines:
//      #define TIMEFREE  to use the free-running PC timer
//      #define TIMEINTERNAL to use internal calibrated time
//      #define TIME_NT to use WinNT's internal microsec. timer
//      #define ISR_SCHEDULING to use interrupt based scheduling and/or
//         timing.
//  These are usually defined in time_mod.h
// File:  clock.cpp
// Created 8/2/95 by Jason Jones
// Modified 9/25/95 to allow for more timing modes by DM Auslander
// Modified 8/13/96 to add WinNT timer, DM Auslander

#include <conio.h>
#include "time_mod.hpp"

#ifdef TIME_NT
#include <windows.h>   // To get WinNT timing functions


const double SEC_PER_OV = 3599.597124; //  Freq. at which 32-bit clock overflows
const double CK_FREQQ = 1193180.0;  //  Frequency of PC clock in Hz
static double StartingTime;        //  Time at which clock was started
#endif  //  end of TIME_NT


#ifdef ISR_SCHEDULING
#define USE_TIMER0
#else // ISR_SCHEDULING
#define USE_TIMER0
//#define USE_TIMER2
#endif // ISR_SCHEDULING

static double TotalTime = 0.0;
static double TickRate = TICKRATE;
//  Initialize to DOS default
static unsigned short TimerPreset = 0xFFFF;

#ifdef TIMEFREE
static unsigned short Last=0;   //  Time read in previous call
#endif

void StopControl(char *aMessage);

//-------------------------------------------------------
// Function: SetTickRate
//      Use this to change the tickrate
//      The default value (from the define symbol TICKRATE
//      will be used if this is never called

void SetTickRate(double Rate)
	 {
	 TickRate = Rate;
	 }

double GetTickRate(void)
	{
	return(TickRate);
	}

//-------------------------------------------------------
//  Function: SetupClock
//      Must be called once in order to set the
//  proper mode for the timer used to keep track of time
//      in TIMEFREE mode.  If TIMEFREE is not defined, then
//      this function does not need to be called.  Look in
//      clock.hpp for a more detailed desciption of what is
//      going on here with respect to the clock modes.

void SetupClock(double aPeriod)
	 {
	 #ifdef TIMEFREE
	 unsigned char  msb, lsb; //  Hold bytes from hardware
	 #endif // TIMEFREE
	 TotalTime = 0.0;    // Initialize running time
		  // Usual units are milliseconds
	 //  For free-running timer, set up the timer hardware
	 #ifdef TIMEFREE
	 #ifndef ISR_SCHEDULING
	 // For Timer0, there is no reason to use anything other than
	 //  the maximum clock period if we are not using interrupts
	 aPeriod = -1.0;
	 #endif // ISR_SCHEDULING
	 #ifdef USE_TIMER2
	 // For Timer2, there is no reason to use anything other than
	 //  the maximum clock period, so forget about the arguement
	 TimerPreset = 0xFFFF;
	 //  Set mode 0 and binary counter for timer 0 and signal
	 //	read/load cycle to start
	 DisableInterrupt ();
	 outp (TIMER_CTL, 0xB0);
	 //  Send LSB then MSB of counter value...
	 //  0xFFFF sets the 8253 to the maximum rollover of
	 //  65535 counts.  This corresponds to 54.9 ms
	 outp (TIMER0, 0xFF);
	 outp (TIMER0, 0xFF);
	 //  Enable counting by bringing gate to timer2 high
	 //  (on 8253 chip)
	 // turns off speaker ?
	 outp (0x61, inp (0x61) | 0x01);
	 // Measure current time to set 'Last' value
	 //  Latch current timer value
	 outp (TIMER_CTL, 0x84);
	 //  Read LSB of word in timer first
	 lsb = (unsigned char) inp (TIMER2);
	 //  and then MSB
	 msb = (unsigned char) inp (TIMER2);
	 #endif // USE_TIMER2
	 #ifdef USE_TIMER0
	 //  If argument's a valid time
	 if ((aPeriod > 0.0) && (aPeriod < (MAX_PER)))
		  {
		  //  calculate rate divisor
		  TimerPreset = (unsigned)((CK_FREQ * aPeriod) + 0.5);
		  //  Set mode 2 and binary counter for timer 0 and signal
		  //	read/load cycle to start
		  DisableInterrupt ();
		  outp (TIMER_CTL, 0x34);
		  //  Send LSB then MSB of counter value...
		  //  0xFFFF sets the 8253 to the maximum rollover of
		  //  65535 counts.  This corresponds to 54.9 ms
		  outp (TIMER0, (int)(TimerPreset & 0xFF));
		  outp (TIMER0, (int)((TimerPreset >> 8) & 0xFF));
		  }
	 else
		  {
		  TimerPreset = 0xFFFF;
		  //  Reset period an mode to supposed DOS default
		  //  Set mode 2 and binary counter for timer 0 and signal
		  //	read/load cycle to start
		  DisableInterrupt ();
		  outp (TIMER_CTL, 0x34);
		  //  Send LSB then MSB of counter value...
		  //  0xFFFF sets the 8253 to the maximum rollover of
		  //  65535 counts.  This corresponds to 54.9 ms
		  outp (TIMER0, 0xFF);
		  outp (TIMER0, 0xFF);
		  }
	 //  Latch current timer value
	 outp (TIMER_CTL, 0x04);
	 //  Read LSB of word in timer first
	 lsb = (unsigned char) inp (TIMER0);
	 //  and then MSB
	 msb = (unsigned char) inp (TIMER0);
	 #endif // USE_TIMER0
	 EnableInterrupt ();
	 //  Concatenate bytes together
	 Last = (short)lsb | ((short)msb  << 8);
	 #endif // TIMEFREE

    #ifdef TIME_NT  // To use the Win-NT timer
    unsigned long Ticks[2];    //  Holds large integers with tick count

    QueryPerformanceCounter ((LARGE_INTEGER *)Ticks);
    StartingTime = ((double)Ticks[1] * SEC_PER_OV) + ((double)Ticks[0] / CK_FREQQ);
    #endif  // End of Win-NT timer setup
	 }

//-------------------------------------------------------
//  Function: GetTimeNow
//      This function returns the current time as kept by
//      the 'real-time' clock (TIMEFREE mode) or the current
//      time as incremented by the user program (not TIMFREE
//      mode)

double GetTimeNow (void)
	 {
	 #ifdef TIMEFREE
	 unsigned short Current=0;//  Current ticks in 8253 chip
	 unsigned char  msb, lsb; //  Hold bytes from hardware
	 unsigned short Delta;    //  Time change since last read

	 //  Prevent interrupt while reading
	 DisableInterrupt();
	 #ifdef USE_TIMER2
	 //  Latch current timer value
	 outp (TIMER_CTL, 0x84);
	 //  Read LSB of word in timer first
	 lsb = (unsigned char) inp (TIMER2);
	 //  and then MSB
	 msb = (unsigned char) inp (TIMER2);
	 //  Concatenate bytes together
	 Current = (short)lsb | ((short)msb  << 8);
	 //  Calculate ticks since last read
	 Delta = Last - Current;
	 #endif // USE_TIMER2
	 #ifdef USE_TIMER0
	 //  Latch current timer value
	 outp (TIMER_CTL, 0x04);
	 //  Read LSB of word in timer first
	 lsb = (unsigned char) inp (TIMER0);
	 //  and then MSB
	 msb = (unsigned char) inp (TIMER0);
	 //  Concatenate bytes together
	 Current = (short)lsb | ((short)msb  << 8);
	 //  Calculate ticks since last read
	 if(Current < Last)
		{
		Delta = Last - Current;
		}
	 // In case of overflow, make correction.  No data has been
	 //  lost, however, if there is only one overflow.
	 else Delta = (TimerPreset - Current) + Last;
	 #endif // USE_TIMER0
	 //  Save current count for later
	 Last = Current;
	 //  Calculate and save the time from the number of
	 //  ticks since last measurement
	 TotalTime += ((double)Delta) / (double)CK_FREQ;

	 EnableInterrupt();  //  Done reading hardware, reEnableInterrupt interrupts
	 #ifndef ISR_SCHEDULING
	 //  If the number of ticks is greater than DELTA_LIMIT,
	 //  consider this an over-flow.  This is a statistical
	 //  error, since an overflow of certain values will not
	 //  be detected.  The max value can be changed if desired.
	 if (Delta > DELTA_LIMIT)
		  {
		  StopControl("Error: Free-running timer not read every 30ms\n");
		  return(-1.0);
		  }
	 #endif // ISR_SCHEDULING

	 return (TotalTime);
	 #endif  // TIMEFREE

	 #ifdef TIMEINTERNAL
	 return (TotalTime);
	 #endif

    #ifdef TIME_NT  // Compute time using the NT microsec timer
    unsigned long Ticks[2];  //  Holds large integers with tick count

    QueryPerformanceCounter ((LARGE_INTEGER *)Ticks);
    TotalTime = ((double)Ticks[1] * SEC_PER_OV) +
        ((double)Ticks[0] / CK_FREQQ) - StartingTime;
    return(TotalTime);
    #endif  // end of TIME_NT section

	 }

//-------------------------------------------------------
//  Function: IncrementTime
//      This function increments the "TotalTime".  It is for
//      cases where TIMEFREE is not defined.  If this function
//      is never called, time will change.  The function should
//      be called periodically, either in the background
//      scheduler loop (main function) or in tasks.  TICKRATE
//      should be defined in "tasks.hpp".

void IncrementTime (void)
	 {
	 #ifdef TIMEINTERNAL
	 TotalTime += TickRate;
	 #endif
	 }

//--------------------------------------------------------------
//  Function: SetAlarm
//      Function to set the rate at which the time of day clock
//      in the PC causes timer interrupts.  This clock runs at
//      about 1.19MHz and can count down from any 16-bit unsigned
//      int to zero.  When it reaches zero, it causes a timer
//      interrupt.  Call SetAlarm ('time') where 'time' is an
//      interval in seconds.  If 'time' is outside the allowable
//      range of intervals, the clock will be reset to the DOS
//    default value of ~54.9 ms, so call SetAlarm with a time
//      less than zero at the end of the program.  SetAlarm()
//      returns 0 if a non-DOS default time is set and FALSE
//      if the default is set (so that if the user wants to set
//      a non-default time, FALSE signifies an error).
//  Note:
//    For a maximum count of 0xFFFF, the time interval is about
//      0.0549 sec.
//  Version:
//    Original - from the lab of D.M.Auslander
//    12-13-94  JJ  Removed compiler specific macro and inout.h; changed all I/O bus
//                  functions to outp().  Changed scope of 'count' to local.
//    12-18-94  JR  Changed to C++; integrated into CTL_EXEC scheduler
//     2-02-95  JR  Moved into CTL_INTR.CPP but not changed

//-------------------------------------------------------
//  Function: TimerEOI
//      Must be called in the Interrupt Service Routine to signal
//      that another interrupt may be scheduled.  Until this
//      function is called, no other interrupts will occur on
//      this channel.
#ifdef ISR_SCHEDULING  // Only define this if needed
void TimerEOI(void)
	 {
	 outp(INTCTL0, TMREOI);
	 }
#endif
