# my-OnStep
## The OnStep Project

This is only a private repository to get my things organized. For you own project refer to the original projekt site. Source Code hosted here is taken from OnStep Projekt: https://onstep.groups.io and its owner:
 
 * Copyright (C) 2016 to 2021 Howard Dutton
 * Author: Howard Dutton
 * http://www.stellarjourney.com
 * hjd1964@gmail.com


## My private OnStep source code repo

* Branch 3.16 (used and proven to work with MKS GEN L v1.0, TMC 5160 and 2130 (Waterott)
* Branch 4.24 (used and proben to work with MKS GEN L v 2.0, TMC 5160 (Bigtreetek V 1.2)

## Hardware
### MKS GEN L V 2.0
The best is: plug the Bigtreetech TMC5160 in without any cables. Finding the correct values for current and goto (!) microsteps can be a bit tricky.

### Stepper
* Nema 17 Bipolar 0.9deg 36Ncm (51oz.in) 0.9A 5.4V 42x42x40mm 4 Wires 
* 17HM15-0904S
* ca 30,- €

### OnStep App (Handy)

Android and iPhone
1. Set location once
2. Set Time and date
3. start tracking (unpark only, if parked previously)

iPhone App crashes when using GoTo.

### SkySafari

Direct connection via WiFi possible
* 192.168.0.1 (webserver on WeMos D1 mini)
* Port: 9999

## OnStep Configuration (Config.h)
#### Serial 
    #define SERIAL_C_BAUD_DEFAULT  9600  //  must be set to 9600 if TX2/RX2 are used (which should you do)


#### Motors
    #define TRACK_BACKLASH_RATE    10    //  good experences with both MKS V1 and v2

    #define SLEW_RATE_BASE_DESIRED        2.5   // giv es me a good mix between speed and robustness
    #define SLEW_RATE_MEMORY              OFF
    #define SLEW_ACCELERATION_DIST        7.0   // more than default - not necessary to put the mount into stress
    #define SLEW_RAPID_STOP_DIST          4.0   // more than default - don't like to rapid stops
    
    #define PIER_SIDE_SYNC_CHANGE_SIDES   ON    // I am still testing
    #define PIER_SIDE_PREFERRED_DEFAULT  BEST

#### Next is for Astrophysics CNC 400 frp both ax1 and ax2
    #define AXIS1_STEPS_PER_DEGREE    20480.0
    #define AXIS1_DRIVER_MODEL  TMC5160_QUIET   // Bigtreetech 5160 V 1.2 (without any cables), all jumpers left in place
    #define AXIS1_DRIVER_MICROSTEPS       32
    #define AXIS1_DRIVER_MICROSTEPS_GOTO  4     // try
    #define AXIS1_DRIVER_IRUN             400
    #define AXIS1_DRIVER_IGOTO           1800   // try
    #define AXIS1_DRIVER_STATUS      TMC_SPI
    #define AXIS1_LIMIT_MIN              -90    // I don't want the mount go further
    #define AXIS1_LIMIT_MAX               90    // I don't want the mount go further









