********************************************************************************
*                            JRAPL README                                      *
* Kenan Liu Email: kliu20@binghamton.edu                                       *
* JRAPL - RAPL interfaces for Java			                       *
********************************************************************************


Introduction:
--------------------------------------------------------------

It works for three architectures -- sandy bridge, sandy bridge_ep(server) and ivy bridge.

sandy bridge/ivy bridge supports: gpu, cpu and package measurement.

sandy bridge_ep supports: dram, cpu and package measurement.

You need check which one is your CPU architecture before you use it.

reference: https://software.intel.com/en-us/articles/intel-power-governor

Prerequisites: 
--------------------------------------------------------------

This library uses the kernel msr module 

> sudo modprobe msr

To Build:
--------------------------------------------------------------

Generate the library for jni

>make

How to use:
--------------------------------------------------------------

Everytime you need root/sudo access to run the java code in order to access rapl msr.

EnergyCheckUtils.java is a sample drive code.

It includes the following methods, and needs to be declared like the following before call them in java:

--------------------------------------------------------------

> public native static int ProfileInit();	

Initialize jrapl. Just needs call once in the beginning.

Returns:
wraparound energy value.

--------------------------------------------------------------

> public native static String EnergyStatCheck();	

For EnergyStatCheck(), it would return a string value, which contains the format of content like: 

socket1_gpu/dram_energy#socket1_cpu_energy#socket1_package_energy@socket2_gpu/dram_energy#socket2_cpu_energy#socket2_package_energy....
character '#' to seperate the energy metric information wthin the socket. Character '@' to seperate different socket (if it only has one
socket, there is no '@'.

--------------------------------------------------------------

> public native static double[] GetPackagePowerSpec();

Get an array of rapl specifications.

Returns:
#1 TDP(Watt)
#2 Minimum power limit(Watt)
#3 Maximum power limit(Watt)
#4 Maximum time window(Second)

--------------------------------------------------------------

> public native static double[] GetDramPowerSpec();

Get an array of rapl specifications.

Returns:
#1 TDP(Watt)
#2 Minimum power limit(Watt)
#3 Maximum power limit(Watt)
#4 Maximum time window(Second)

--------------------------------------------------------------
	
> public native static void SetPackagePowerLimit(int socketId, int level, double customPower);

Set power limit for package.

Parameters:
socketId-the socket to be set
level-power limit level to be set
level key:
0-Minimum power limit from current machine specifications.
1-Maximum power limit from current machine specifications.
2-customized power limit. If it is out range of machine specifications. It would be discarded.
customPower-customized power. If level is not 2, the value would be discarded.

--------------------------------------------------------------

> public native static void SetDramPowerLimit(int socketId, int level, double costomPower);

Set power limit for dram.

Parameters:
socketId-the socket to be set
level-power limit level to be set
level key:
0-Minimum power limit from current machine specifications.
1-Maximum power limit from current machine specifications.
2-customized power limit. If it is out range of machine specifications. It would be discarded.
customPower-customized power. If level is not 2, the value would be discarded.

--------------------------------------------------------------

> public native static void SetPackageTimeWindowLimit(int socketId, int level, double costomTimeWin);

Set time window for package.

Parameters:
socketId-the socket to be set
level-time window level to be set
level key:
0-Minimum time window is not available
1-Maximum time window limit from current machine specifications.
2-customized time window limit. If it is out range of machine specifications. It would be discarded.
customTimeWin-customized time window. If level is not 2, the value would be discarded.

--------------------------------------------------------------

> public native static void SetDramTimeWindowLimit(int socketId, int level, double costomTimeWin);

Set time window for dram.

Parameters:
socketId-the socket to be set
level-time window level to be set
level key:
0-Minimum time window is not available
1-Maximum time window limit from current machine specifications.
2-customized time window limit. If it is out range of machine specifications. It would be discarded.
customTimeWin-customized time window. If level is not 2, the value would be discarded.

--------------------------------------------------------------

> public native static void ProfileDealloc();

Free the memory initialized in ProfileInit().


Limitations:
--------------------------------------------------------------

It only works for at most 2 sockets CPU. 


Contact Info:

Kenan Liu kliu20ATbinghamtonDOTedu
