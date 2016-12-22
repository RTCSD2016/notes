// main header file for gloabal function prototypes
// File:  clock.hpp
// Created 8/2/95 by Jason Jones
#ifndef CLOCK_HPP
#define CLOCK_HPP

// Function Prototypes
void SetupClock(double);
void SetTickRate(double tr);
double GetTickRate(void);
double GetTimeNow(void);
void IncrementTime(void);
void TimerEOI(void);
//----------------------------------------------------
//  INFORMATION ABOUT FREE-RUNNING TIMER IN IBM-PC,
//  used by TIMEFREE mode. The following are the
//  port addresses for the timers and the control
//  register. The port number selects the proper
//  address (a1 and a0 on 8253 chip) and read and write
//  flags depending on if written or read, i.e.
//  COMMAND               RD'     WR'     A1      A0
//  inp(TIMER0)            0       1       0       0
//  inp(TIMER1)            0       1       0       1
//  inp(TIMER2)            0       1       1       0
//  outp(TIMER0)           1       0       0       0
//  outp(TIMER1)           1       0       0       1
//  outp(TIMER2)           1       0       1       0
//  outp(TIMER_CTL)        1       0       1       1
//
//  To set up each timer you must write to the control
//  word register:
//  D7    D6    D5    D4    D3    D2    D1    D0
//  SC1   SC0   RL1   RL0   M2    M1    M0    BCD
//
//  Where:   SC1  SC0                          RL1  RL0
//  Timer0   0    0    Counter Latch for Read  0    0
//  Timer1   0    1    Read/Load MSB only      1    0
//  Timer2   1    0    Read/Load LSB only      0    1
//                     Read/Load LSB, THEN MSB 1    1
//  M2,M1,M0 mode:  Mode 0: Interrupt on terminal count
//                  Mode 1: Programmable one-shot
//                  Mode 2: Rate generator
//                  Mode 3: Square wave (Counts by twos)
//                  Mode 4: Software triggered strobe
//                  Mode 5: Hardware triggered strobe
//  BCD: 0 - Binary counter (16 bits);
//  	1 - Binary coded decimal counter (4 decades)
//
//  Notes: - In order to set the gate high for timer 2
//				 (sound timer), which enables counting, use
//				 the command: _outp (0x61, _inp(0x61) | (0x01));
//         - All counters count down to zero.
//         - More info is in Intel's Microprocessor and
//				 Peripheral Handbook Vol. 2

#define  CK_FREQ     1193180.0 //  Frequency of PC clock in Hz
#define  MAX_PER     0.0549      //  Maximum timer period
#define  TIMER0      0x40      //  Timer 0 port
#define  TIMER1      0x41      //  Timer 1 port
#define  TIMER2      0x42      //  Timer 2 port
#define  TIMER_CTL   0x43      //  8253 Timer control port
#define  DELTA_LIMIT 0xB000    //  Clock overflow check limit
#define  TMREOI      0x60      //  Timer-specific EOI signal
#define  INTCTL0     0x20      //  Port 0 of 8259
#define  CK_VECTOR   8         //  Interrupt vector of PC timer

#endif // CLOCK_HPP
