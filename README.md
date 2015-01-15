********************************************************************************
*                            JRAPL README                                      *
* Kenan Liu Email: kliu20ATbinghamton.edu                                       *
* JRAPL - RAPL interfaces for Java			                       *
********************************************************************************


Introduction:
--------------------------------------------------------------

It works for three architectures -- sandy bridge, sandy bridge_ep (server) and ivy bridge.

sandy bridge/ivy bridge supports: gpu, cpu and package measurement.

sandy bridge_ep supports: dram, cpu and package measurement.

You need check which one is your CPU architecture before you use it.

Main reference: https://software.intel.com/en-us/articles/intel-power-governor

Prerequisites: 
--------------------------------------------------------------

This library uses the kernel msr module. To use, type

`sudo modprobe msr`

Build
--------------------------------------------------------------

Generate the library for JNI

`make`

How to use
--------------------------------------------------------------

Everytime you need root/sudo access to run the java code in order to access rapl msr.

`EnergyCheckUtils.java` is a sample drive code.

It includes the following methods, and it needs to be declared like the following before call them in java:

--------------------------------------------------------------

`public native static int ProfileInit();`

Initialize jRAPL. Just needs call once in the beginning.

Expected return:

wraparound energy value.

--------------------------------------------------------------

`public native static String EnergyStatCheck();`

For `EnergyStatCheck()`, it would return a string value, which contains the format of content like: 

socket1_gpu/dram_energy#socket1_cpu_energy#socket1_package_energy@socket2_gpu/dram_energy#socket2_cpu_energy#socket2_package_energy....

The character '#' to seperate the energy metric information wthin the socket. Character '@' to seperate different socket. If it only has one socket, there is no '@'.

--------------------------------------------------------------

`public native static void ProfileDealloc();`

Free the memory initialized in ProfileInit().


Known limitations:
--------------------------------------------------------------

It only works for at most 2 sockets CPU. 


Contributions
--------------------------------------------------------------

If you find any bug, please open an issue explain what happened.
