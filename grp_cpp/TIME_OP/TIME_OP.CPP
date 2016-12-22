// Timing and operator interface test program.
// time_op.cpp

// J. Jones and DM Auslander, 1995

// This program produces a histogram indicating timing intervals
// between successive scans of the opertor interface 'update'
// function. This gives a measure of how much latency the operator
// interface will cause in single-thread mode.

// NOTE: the initial version of this program is set up for internal timing
// (TIMEINTERNAL) for easy initial compilation. Internal timing doesn't
// test for much of anything, however, so change it to TIMEFREE as soon 
// as successful compilation has been achieved.

#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include "tasks.hpp"

//-------------------------------------------------------------------------------------
//  Function:  DumpOut
//      Function to exit program when the user presses a given control key

static int DoExitNow = 0;

void DumpOut (void)
    {
    DoExitNow = 1;
    }


//-------------------------------------------------------------------------------------
//  Function:  ResetCounter
//      Here's a file-global counter and a key-mappable function to reset it

static long Counter = 0;

void ResetCounter (void)
    {
    Counter = 0;
	 }


//=====================================================================================

main ()
	 {
	 static int AnInt = 0;
	 static long A_Long = 0L;
	 static float A_Float = 0.0f;
	 static double A_Double = 0.0;
	 static long double A_Long_Double = (long double)0.0;
	 static char A_String[32] = "Blah";
	 // Variables associated with the time-test
	const int nbin = 15;
	double *values = new double [nbin];	// Set up the values for the
			// frequency (histogram) boundaries
	double v1 = 5.e-6, vlast = 30.e-3;	// Beginning and end values
	long *occur = new long [nbin];	// One extra bin for >vlast
	double LastTime,Time=0.0,delt,EndTime = 60.0;
	int i,j;

	double mult = exp(log(vlast / v1) / (nbin - 2));
	// Set up the boundaries for the time histogram
	for(values[0]= v1,i = 1;i < nbin-1; i++)
		{
		values[i] = values[i - 1] * mult;
		}
	values[nbin-1] = 1.e20;	// Big number
	for(i = 0; i < nbin; i++)occur[i] = 0;


	 COperatorWindow* TheOpWin = new COperatorWindow ("Another Test Window");

	 TheOpWin->AddInputItem ("An Integer:", &AnInt);
	 TheOpWin->AddInputItem ("A Long Int:", &A_Long);
	 TheOpWin->AddInputItem ("A Float:", &A_Float);
	 TheOpWin->AddInputItem ("A Double:", &A_Double);
	 TheOpWin->AddInputItem ("Long Double:", &A_Long_Double);
	 TheOpWin->AddInputItem ("A String:", A_String);

	 TheOpWin->AddOutputItem ("A Counter", &Counter);
	 TheOpWin->AddOutputItem ("Time", &Time);

	 TheOpWin->AddKey ('X', "Stop", "Program", DumpOut);
	 TheOpWin->AddKey ('R', "Reset", "Counter", ResetCounter);
	cout << "Maximize window if necessary. ENTER to continue...\n";
	getch();

	 TheOpWin->Display ();

	SetupClock(-1.0);	// Start the clock running
	LastTime = Time = GetTimeNow();

	while((Time < EndTime) && (DoExitNow == 0))
		  {
		  TheOpWin->Update ();
		  Counter++;
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
		  IncrementTime();	// For internal time mode
		  }

	 TheOpWin->Close ();
	clrscr ();	// Clear the operator interface
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
   // Delete objects that were created with 'new'
   delete [] values;
   delete [] occur;
   delete TheOpWin;

   cout << "Hit any key to exit.\n";
   while(!kbhit()) ;  // Wait to exit
	 return (0);
	 }

    // Dummy functions
void EnableInterrupt(void){}
void DisableInterrupt(void){}

