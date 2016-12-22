// Inherited classes for PWM -- the only substantive
// material here is the PWMOn and PWMOff functions
// File: pwmtask2.cpp
// created 12/25/95, DM Auslander

#include <iostream.h>
#include <string.h>
#include "tasks.hpp"

PWMTask2::PWMTask2(char *aName) : PWMBaseTask (aName)
	{
   // Nothing to initialize here -- its all done in the
   // base class constructor. Any initializations unique
   // to the inherited class would be done here.
	}

PWMTask2::~PWMTask2(void)
	{
	};


// PWMOn() and PWMOff are defined here and are unique to this
// class. Although they are really the same in the case of
// this example, in a real example they would have differen
// code to access different I/O devices.

void PWMTask2::PWMOn(void)
	{
	PWMOutput = 1;
	PWMRecord->AddData(PWMOutput,END_OF_ARGS);
	CriticalSectionBegin();
	xPWMOutput = PWMOutput;
	CriticalSectionEnd();
	}

void PWMTask2::PWMOff(void)
	{
	PWMOutput = 0;
	PWMRecord->AddData(PWMOutput,END_OF_ARGS);
	CriticalSectionBegin();
	xPWMOutput = PWMOutput;
	CriticalSectionEnd();
	}

