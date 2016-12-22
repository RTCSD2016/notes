#include <conio.h>
#include <stdio.h>
#include <stdlib.h>

#define DTEZ_CPP

#include "dtez.hpp"

int d_output = 0;

extern "C"
{
void io_init()
{
int dummy,i,LockupFlag;

//while(!((OUT_READY & (dummy = inp(STATUS))) || (READY & dummy)));
LockupFlag = 1;
for (i=0; i<1000; i++)
	{
	if ((OUT_READY & (dummy = inp(STATUS))) || (READY & dummy))
		{
		LockupFlag = 0;
		break;
		}
	}
if (LockupFlag)
	{
	}

dummy = outp(CONTROL, RESET);

//while(!((OUT_READY & (dummy = inp(STATUS))) || (READY & dummy)));
LockupFlag = 1;
for (i=0; i<1000; i++)
	{
	if ((OUT_READY & (dummy = inp(STATUS))) || (READY & dummy))
		{
		LockupFlag = 0;
		break;
		}
	}
if (LockupFlag)
	{
//	printf("\n\nError initializing Data Translation Card\n\n");
//	exit(0);
	}

dummy = outp(CONTROL, STOP);

//while(!((OUT_READY & (dummy = inp(STATUS))) || (READY & dummy)));
LockupFlag = 1;
for (i=0; i<1000; i++)
	{
	if ((OUT_READY & (dummy = inp(STATUS))) || (READY & dummy))
		{
		LockupFlag = 0;
		break;
		}
	}
if (LockupFlag)
	{
//	printf("\n\nError initializing Data Translation Card\n\n");
//	exit(0);
	}

dummy = inp(DATA_IN);

//while(!((OUT_READY & (dummy = inp(STATUS))) || (READY & dummy)));
LockupFlag = 1;
for (i=0; i<1000; i++)
	{
	if ((OUT_READY & (dummy = inp(STATUS))) || (READY & dummy))
		{
		LockupFlag = 0;
		break;
		}
	}
if (LockupFlag)
	{
	printf("\n\nError initializing Data Translation Card\n\n");
	exit(0);
	}

dummy = inp(DATA_IN);

//while(!(READY & (dummy = inp(STATUS))));
LockupFlag = 1;
for (i=0; i<1000; i++)
	{
	if ((OUT_READY & (dummy = inp(STATUS))) || (READY & dummy))
		{
		LockupFlag = 0;
		break;
		}
	}
if (LockupFlag)
	{
	printf("\n\nError initializing Data Translation Card\n\n");
	exit(0);
	}

d_output = 0;
d_out(d_output);
}



int d_in(void)
{
int data, dummy, i, LockupFlag;

//while(!(READY & (dummy = inp(STATUS))));
LockupFlag = 1;
for (i=0; i<1000; i++)
	{
	if ((READY & (dummy = inp(STATUS))))
		{
		LockupFlag = 0;
		break;
		}
	}
if (LockupFlag)
	{
	printf("\n\nError with digital input\n\n");
	exit(0);
	}

dummy = outp(CONTROL, SET_D_IN);

//while(DATA_IN_FULL & (dummy = inp(STATUS)));
LockupFlag = 1;
for (i=0; i<1000; i++)
	{
	if (!(DATA_IN_FULL & (dummy = inp(STATUS))))
		{
		LockupFlag = 0;
		break;
		}
	}
if (LockupFlag)
	{
	printf("\n\nError with digital input\n\n");
	exit(0);
	}

dummy = outp(DATA_IN, 1);

//while(!(READY & (dummy = inp(STATUS))));
LockupFlag = 1;
for (i=0; i<1000; i++)
	{
	if (READY & (dummy = inp(STATUS)))
		{
		LockupFlag = 0;
		break;
		}
	}
if (LockupFlag)
	{
	printf("\n\nError with digital input\n\n");
	exit(0);
	}


dummy = outp(CONTROL, READ_D_IN);

//while(DATA_IN_FULL & (dummy = inp(STATUS)));
LockupFlag = 1;
for (i=0; i<1000; i++)
	{
	if (!(DATA_IN_FULL & (dummy = inp(STATUS))))
		{
		LockupFlag = 0;
		break;
		}
	}
if (LockupFlag)
	{
	printf("\n\nError with digital input\n\n");
	exit(0);
	}

dummy = outp(DATA_IN, 1);

//while(!((OUT_READY & (dummy = inp(STATUS))) || (READY & dummy)));
LockupFlag = 1;
for (i=0; i<1000; i++)
	{
	if ((OUT_READY & (dummy = inp(STATUS))) || (READY & dummy))
		{
		LockupFlag = 0;
		break;
		}
	}
if (LockupFlag)
	{
	printf("\n\nError with digital input\n\n");
	exit(0);
	}

data = inp(DATA_OUT);

return(data);
}



void d_out(int data)
{
int dummy, i, LockupFlag;

//while(!(READY & (dummy = inp(STATUS))));
LockupFlag = 1;
for (i=0; i<1000; i++)
	{
	if ((READY & (dummy = inp(STATUS))))
		{
		LockupFlag = 0;
		break;
		}
	}
if (LockupFlag)
	{
	printf("\n\nError with digital input\n\n");
	exit(0);
	}

dummy = outp(CONTROL, SET_D_OUT);

//while(DATA_IN_FULL & (dummy = inp(STATUS)));
LockupFlag = 1;
for (i=0; i<1000; i++)
	{
	if (!(DATA_IN_FULL & (dummy = inp(STATUS))))
		{
		LockupFlag = 0;
		break;
		}
	}
if (LockupFlag)
	{
//	printf("\n\nError with digital output\n\n");
//	exit(0);
	}

dummy = outp(DATA_IN, 0);

//while(!(READY & (dummy = inp(STATUS))));
LockupFlag = 1;
for (i=0; i<1000; i++)
	{
	if ((READY & (dummy = inp(STATUS))))
		{
		LockupFlag = 0;
		break;
		}
	}
if (LockupFlag)
	{
//	printf("\n\nError with digital input\n\n");
//	exit(0);
	}

dummy = outp(CONTROL, WRITE_D_OUT);

//while(DATA_IN_FULL & (dummy = inp(STATUS)));
LockupFlag = 1;
for (i=0; i<1000; i++)
	{
	if (!(DATA_IN_FULL & (dummy = inp(STATUS))))
		{
		LockupFlag = 0;
		break;
		}
	}
if (LockupFlag)
	{
//	printf("\n\nError with digital output\n\n");
//	exit(0);
	}

dummy = outp(DATA_IN, 0);

//while(DATA_IN_FULL & (dummy = inp(STATUS)));
LockupFlag = 1;
for (i=0; i<1000; i++)
	{
	if (!(DATA_IN_FULL & (dummy = inp(STATUS))))
		{
		LockupFlag = 0;
		break;
		}
	}
if (LockupFlag)
	{
//	printf("\n\nError with digital output\n\n");
//	exit(0);
	}

dummy = outp(DATA_IN, data);

d_output = data;
}

double a2d(int ch_no)
{
int data, data_low, data_high, dummy;
double volts;

while(!(READY & (dummy = inp(STATUS)))); // wait until board is ready

dummy = outp(CONTROL, READ_A2D); // Set for read a/d immediate command

while(DATA_IN_FULL & (dummy = inp(STATUS))); // wait until board is ready for data

dummy = outp(DATA_IN, 0);  // set the gain to 1 (+- 10 Volts or 0 to 10 Volts)

while(DATA_IN_FULL & (dummy = inp(STATUS))); // wait until board is ready for data

dummy = outp(DATA_IN, ch_no); // select a/d channel

// wait until board is ready for data
while(!((OUT_READY & (dummy = inp(STATUS))) || (READY & dummy)));

data_low = inp(DATA_OUT); // read low byte

// wait until board is ready for data
while(!((OUT_READY & (dummy = inp(STATUS))) || (READY & dummy)));

data_high = inp(DATA_OUT);  // read high byte

// concatenate data and convert to volts
data = data_low | (data_high << 8);
volts = 20.0*((double)data)/4095.0 - 10.0;
//volts = 10.0*((double)data)/4095.0 - 5.0;

return(volts);
}



void d2a(int ch_no, double volts)
{
int dummy, data_low=0, data_high=0;
long data;

data = (long)((volts + 10.0)*2048/10.0);
if (data >= 4095) data = 4095;
if (data <= 0) data = 0;
data_low = (int)(data & 0xff);
data_high = (int)((data & 0xf00) >> 8);

while(!(READY & (dummy = inp(STATUS))));

dummy = outp(CONTROL, WRITE_D2A);

while(DATA_IN_FULL & (dummy = inp(STATUS)));

dummy = outp(DATA_IN, ch_no);                                   

while(DATA_IN_FULL & (dummy = inp(STATUS)));

dummy = outp(DATA_IN, data_low);

while(DATA_IN_FULL & (dummy = inp(STATUS)));
                                   
dummy = outp(DATA_IN, data_high);
}        

void bit_out(int bit_no, int data)
{

if(data==0)     d_output = d_output & (~(0x01 << bit_no));

else            d_output = d_output | (0x01 << bit_no);

d_out(d_output);
}

int bit_in(int bit_no)
{
if (d_in() & (0x01 << bit_no)) return(1);
else return(0);
//return(d_in() & (0x01 << bit_no));
}
}