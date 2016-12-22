1 Building Programs from Sample Files

NOTE: Some changes have been made in these files (August, 96
versions) from the original, 1995 versions. The only differences that
will have any affect on user-written files is that the timing and
interrupt mode specifications have been moved from tasks.hpp to a
separate include file, time_mod.hpp and a new timing mode (TIME_NT)
has been added for use with Windows NT 32-bit applications.  The
reason for moving the timing mode specification is because of some
incompatibilities associated with definition of the NT material. The
most recent version of the Borland compiler (v5.0) is now specified
-- this entailed no significant change from version 4.51.

1.1 Group Priority Dispatcher, C++
The files for assembling programs based on the group priority method
and C++ are found in an archive file named grp_cpp.zip.  It is
archived using the PKZIP 2.0 format. A set of directories contains
files for sample programs and for utility files. The directory common
contains files that are used by all of the sample programs. Other
directories contain files specific to each sample program (there is
some file duplication among the sample directories). The template
directory contains files that outline the construction of a control
program using the group priority method. These files cannot be
compiled but can be used as the base to build a working  program.

As of this writing, the files have only been tested on MSDOS/Windows
and Windows NT systems, mostly using the Borland C++ (v5.0) compiler.
They are written using generic C++, however, so should be portable to
any environment with a standard C++ compiler. The operator interface
will need to be modified for whatever character-oriented screen and
cursor controls are needed in the target environment.

The directories in grp_cpp.zip are:

	Common - files that are used by most sample and user-written
	programs. These include the base classes needed to build tasks
	and the operator interface classes and functions.

	Template - skeleton files that show all of the component parts of
	a group priority program. They cannot be compiled as is.
	
	DataAcq - a simple data aquisition program.

	Pwm1 - a program that generates a pulse-width modulated (PWM)
	signal. Does not have an operator interface.

	Pwm2 - a generalization of the pulse-width modulation program
	that instantiates two tasks from the PWM class to generate two
	PWM signals simultaneously. Does not have an operator interface.

	Pwm3 - a generalization of the pulse-width modulation program
	that defines a generic class for PWM and uses it to generate two
	PWM signals simultaneously. Does not have an operator interface.

	Heater1 - a simulation of a temperature control problem. Uses an
	operator interface and defines a generic PID controller that can
	be used in other applications.

	Time_tst - a utility program that can be used to explore any
	preemption that the underlying operating system is doing. This
	will characterize execution delays that a real time proram will
	see independent of its own operation.

	Time_op - this is the program of time_tst modified so that it
	also has an operator interface. This allows the additional
	characterization of the computing time taken by the operator
	interface in a variety of environments.

1.1.1 Program File Structure
The user-written part of the program consists of the following file
set:

	- Tasks.hpp contains all of the class definitions for tasks and
	other objects, definitions (#define) for a number of operational
	mode indicators, declarations for global (extern) pointers to
	instantiations of task classes, and function prototypes.

	- Time_mod.hpp for specification of the timing and interrupt
	modes (note that this is new since the last version).

	- A file for each task class. This file contains the definitions
	for all member functions of the task. Multiple instantiations of
	the class would be unique tasks, but would all share the same
	code in this file.

	- A "main" file containing the instantiations for all of the
	tasks and task lists, intialization of any data that must be
	obtained from the user prior to real time operation, setup for
	real time devices such as interrupts, and the main program loop
	for all cooperatively dispatched tasks.

A program is constructed by adding the files from the common
directory to these files. The following files are used in all
programs:

	basetask.hpp, basetask.cpp
	clock.hpp, clock.cpp
	audit.hpp, audit.cpp

If an operator interface is used the following files must also be
used:

	base_obj.hpp, base_obj.cpp
	oper_int.hpp, oper_int.cpp
	compconv.hpp

Finally, if there is physical I/O to be done, an interface file for
the specific lab I/O board must be used.

(It is often easier just to copy all the files from the common
directory!)

The recommended application directory structure is to copy all needed
files to a directory that is unique to the application. Although it
is possible to access files from the common directory, it doesn't
seem to be worth the trouble or added complexity.

1.1.2 Setting Program Mode Switches
All of the switches used to control program mode are in tasks.hpp and
time_mod.hpp. These determine the type of timing used, whether
interrupt-based dispatching is used, and whether a simulation is
present.

To determine the type of timing:
Define (via #define) one or the other of
	TIMEINTERNAL
	TIMEFREE
	TIME_NT
TIMEINTERNAL uses an internally incremented variable to approximate
time. It can be used for simulations, where time is artificial, or in
real time problems where the accuracy requirements for timing are
low. If internal tining is used, the #define symbol TICKRATE gives
the default value of the time increment (note that this same variable
is used to set the interrupt rate when interrupt dispatching is
activated). The value used in calculations can be changed from the
program with the calls GetTickRate() and SetTickRate().

TIMEFREE determines time by referring to a free-running timer. On the
PC, that can be either timer #2 or timer #0. Timer #2 is preferred,
but some PCs use that timer for other purposes so timer #0 must be
used (this causes some complication if interrupt dispatching is used,
but that can be handled in software). Using a free running timer
gives a time granularity of a bit under 1µs. However, the time
calculation must be refreshed to avoid errors caused by timer
rollover. In order to have a margin so that rollover error can be
detected, the maximum refresh period is set at 30ms.

TIME_NT uses Windows-NT's internal, high precision timer. It is
internally maintained so does not need any refresh (in contrast to
TIMEFREE). The precision is abou 0.8 microsec.

ISR_SCHEDULING is defined to set up a timer interrupt routine to
dispatch tasks. The tasks that become part of the interrupt layer are
specified when the task lists are made up in the main function.

The define SIMULATION can be used to control conditional compilation
for simulation-related sections of code. It is not used in the base
software at all, so its use is at user option.

As much as possible, all of the sample programs come set for
SIMULATION, TIMEINTERNAL, and the compiler target is set for EasyWin.
This configuration will run almost anywhere!

1.1.3 Program Compilation (Borland C++ v5.0)
These compilation instructions are for use of the integrated
development environment (IDE) mode. Assuming that all of the relevant
files have been copied to the same directory, the project list is
made by adding all cpp files to the list.

The three standard targets are EasyWin (16-bit), WinNT console mode
(32-bit) or DOS (16-bit). In all cases, the fastest floating point
mode should be specified. If interrupt dispatching is used, the
fastest mode (which uses hardware floating point) is essential since
the software floating point library is not reliable in interrupt
mode. DOS is also the only possible target for interrupt mode
dispatching.

NOTE: Be extremely careful is setting the library, include and
temprorary directories in the IDE file that is in the archive file.
It was set using a computer with a fairly standard setup, but it
quite likely to be wrong for other computers!

The options to set (using the Options menu item) are:

	- Under Project/Directories add the source directory to the
	Include item (it will accept the notation "." for the current
	directory). It is also convenient to specify a temporary
	directory for the intermediate files to avoid clutter in the
	working directory.

	- Under Project/16-bit Compiler, set the memory model to "large".
	It also seems to help in some circumstances to turn on "far
	virtual tables" and "automatic far data." Setting the DS==SS
	switch to NEVER seems to help reliability also, particularly for
	DOS programs.

	- Under Project/Compiler, set the floating point mode to "fast
	floating point' (as noted above).

1.1.4 Program Execution
A detailed analysis of program performance is given in the chapter on
Performance. The summary is that predictable behavior is best when
running in "pure" DOS; predictability gets increasingly worse as
follows:

	DOS/Win (DOS session under windows), full screen,
	DOS/Win, window,
	EasyWin or NT-console mode, run from file manager
	EasyWin or NT-console mode, run from the IDE

On the other hand, debugging ease goes in essentially the reverse
order! The IDE debugger is usable only in EasyWin run from the IDE.

Codeguard, which is part of the Borland v5.0 compiler if purchased as
the "Development Suite," is an invaluable debugging aid since it
finds runtime errors that the compiler will not catch. It slows
execution considerably, however, so should normally be used in
simulation mode only (another good reason to make sure an do
simulations!)