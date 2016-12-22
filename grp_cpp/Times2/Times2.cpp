// Operator interface test program.
// times2.cpp

// D. M. Auslander  8/25/98

// This program does the "Xs 2" test -- it has a single data
// entry item, multiplies that value by two, and displays
// the result

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


main ()
	 {
	 double A_Double = 0.0;
    double x2;

	 COperatorWindow* TheOpWin = new COperatorWindow ("Times-2 Test Window");

	 TheOpWin->AddInputItem ("A Double:", &A_Double);
	 TheOpWin->AddOutputItem ("Times 2", &x2);
	 TheOpWin->AddKey ('X', "Stop", "Program", DumpOut);

	 cout << "Maximize window if necessary. ENTER to continue...\n";
	 getch();

	 TheOpWin->Display ();


	 while(DoExitNow == 0)
		  {
		  TheOpWin->Update ();
        x2 = 2.0 * A_Double;
		  }

   TheOpWin->Close ();
	clrscr ();	// Clear the operator interface
   // Delete objects that were created with 'new'
   delete TheOpWin;

   cout << "Hit ENTER to exit.\n";
	getch();
	 return (0);
	 }

    // Dummy functions
//void EnableInterrupt(void){}
//void DisableInterrupt(void){}

