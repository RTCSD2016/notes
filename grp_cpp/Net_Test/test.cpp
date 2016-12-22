// What size are common types in 32-bit console mode applications?
#include <iostream.h>
#include <stdlib.h>
#include <conio.h>

void main(void)
	{
   cout << "int " << sizeof(int) << " long " << sizeof(long) <<
   	" short " << sizeof(short) << " float " << sizeof(float) <<
      " double " << sizeof(double) << "\n";
   cout << "Hit any key to exit.\n";
   while(!kbhit()) ;  // Wait to exit
   }
   