# *JDigi* AX.25/APRS Smart Digipeater v0.99-prerelease
Copyright 2018 Jason Woodford, VO1JWW.
Uses the *AtomThreads* scheduler, Copyright 2010 Kelvin Lawson.
*GNU Time* module is Copyright 2012 Michael Duane Rice.
*APRS* is a trademark of Bob Bruninga, WB4APR.
All rights reserved.

## Introduction
The JDigi project is an attempt to put an entire amateur radio AX.25 stack
with both digipeating and *APRS* functions into an embedded platform, such
as the AVR series of microcontrollers. Many of these platforms (AVR in particular)
have very limited RAM, so JDigi has been written to be very efficient with memory.

JDigi is essentially a group of applications that run on top of the
*AtomThreads* scheduler - a multi-threading platform that can run on several
embedded architectures. This allows JDigi to be a multi-platform project that
only requires architecture dependent modules to deal with low-level functions.
The core of *AtomThreads* is located in __kernel/__.

JDigi has been split into two trees: a platform independent Core, and the
platform dependent modules which provide the functions required by Core.
The core of JDigi is located in __jdigi/__. Please note that many of these
modules still presume the use of a separate Program Memory area (PROGMEM) to
reduce RAM usage when space is limited. _This may reduce the platform
independent design of this code._

All architecture dependent modules (both AtomThreads and JDigi) are located
in __ports/XXX/__, where __XXX__ is the AtomThreads provided sub-folder that
contains it's architecture dependent stuff. Currently, the only supported
platform is the ATMega2560 due to it's "large" RAM amount. However, an earlier
prototype used the ATMega32 with success. Therefore, it is possible to create
a stripped-down version that can operate in smaller RAM space. Note that
devices with 32K of Program Memory will need to omit many RAM and PROGMEM hungry
Core modules.

## Building JDigi
This uses the same build structure as *AtomThreads*. Each __ports/XXX/__ folder
contains it's own Makefile for building *JDigi*. Each Makefile should be
customized for the architecture and programmer being used.

For example, the currently supported platform for AVR is the ATMega2560 *Arduino
Mega*, using a parallel port *Pony STK200* programmer. To build the project,
change to the __ports/avr__ folder and:

1. Edit the various header files to suit your particular setup.
2. Edit the local Makefile for the programming interface and part ID.
3. Build the code: `make`.
4. Document the code: `make doxygen`.
5. Upload the code: `make progdude app=jdigi`.
6. Verify the upload: `make progdudetest app=jdigi`.
7. Upload EEPROM code using `make progdudeee app=jdigi`.



