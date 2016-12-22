//*************************************************************************************
//  TIME_TST.CPP
//  A program to test time lost to operations performed in the underlying
// operating system.
//*************************************************************************************

// NOTE: the initial version of this program is set up for internal timing
// (TIMEINTERNAL) for easy initial compilation. Internal timing doesn't
// test for much of anything, however, so change it to TIMEFREE as soon 
// as successful compilation has been achieved.

#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include "time_mod.hpp"



//=====================================================================================

main ()
     {
     // Variables associated with the time-test
    const int nbin = 15;
    double *values = new double [nbin]; // Set up the values for the
            // frequency (histogram) boundaries
    double v1 = 5.e-6, vlast = 30.e-3;  // Beginning and end values
    long *occur = new long [nbin];  // One extra bin for >vlast
    double LastTime,Time=0.0,delt,EndTime = 10.0;  // EndTime is in seconds
    int i,j;

    double mult = exp(log(vlast / v1) / (nbin - 2));
    // Set up the boundaries for the time histogram
    for(values[0]= v1,i = 1;i < nbin-1; i++)
        {
        values[i] = values[i - 1] * mult;
        }
    values[nbin-1] = 1.e20; // Big number
    for(i = 0; i < nbin; i++)occur[i] = 0;

    SetupClock(-1.0);   // Start the clock running
    LastTime = Time = GetTimeNow();

    while(Time < EndTime)
          {
          Time = GetTimeNow();
          delt = Time - LastTime;
          LastTime = Time;
          for(j = 0; j < nbin; j++)
            {
            if(delt <= values[j])
                {
                occur[j]++;
                break;
                }
            }
          IncrementTime();  // For internal time mode
          }

    cout << "Time: " << GetTimeNow() << "\n";

    FILE *out_file = fopen("timevals.txt","w");
    if(out_file == NULL)
        {
        printf("Can't open output file\n");
        exit(1);
        }
    for(i = 0; i < nbin - 1; i++)
        {
        printf("  %9.6f %ld\n",values[i],occur[i]);
        fprintf(out_file,"  %9.6f %ld\n",values[i],occur[i]);
        }
    printf(" >%9.6f %ld\n",values[i-1],occur[i]);
    fprintf(out_file," >%9.6f %ld\n",values[i-1],occur[i]);
    fclose(out_file);
    delete [] values;  // Free memory allocated with 'new'
    delete [] occur;
   printf("Press any key to quit.\n");
   while (!kbhit ()) ;  // Wait for key press
     return (0);
     }

     // Dummy functions
void EnableInterrupt(void){}
void DisableInterrupt(void){}

