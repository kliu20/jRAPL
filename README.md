Introduction
--------------------------------------------------------------

For a general explanation of energy measurement, check our [webpage](http://kliu20.github.io/jRAPL/).

jRAPL works for Intel microarchitectures of Skylake, Haswell, Sandy Bridge, Sandy Bridge_ep (Server) and Ivy Bridge. Each architecture has different RAPL support.

- Sandy Bridge/Ivy Bridge: uncore GPU, CPU and package support.

- Skylake, Haswell, Sandy Bridge_ep: DRAM, CPU and package support.

You need check which one is your CPU architecture before you use jRAPL.

For more details about RAPL, check the main [reference](https://software.intel.com/en-us/articles/intel-power-governor)

Pre-requisites for energy measurement
--------------------------------------------------------------

JRAPL only supports Linux system which should be directly installed on your machine. So you cannot measure the energy consumption as well as the hardware counters information on virtual machine since jRAPL needs to access the real respective registers. It currently supports energy measurement and hardware counters measurements (Cache hits, cache references, TLB hits, TLB misses, page faults, and etc.). For more information about the hardware counters measurements, check [reference](https://perf.wiki.kernel.org/index.php/Tutorial#Counting_with_perf_stat). Instead of `tool perf` on Linux which supports application level measurements. We supply APIs to measure those counters information with finer granularity in Java program.

1) Energy check library uses the Linux kernel `msr` module. To use, type

```
sudo modprobe msr
```
2) Perf counters check library depends on Libpfm4 (http://perfmon2.sourceforge.net/docs_v4.html) library. Therefore, you should go to libpfm with command 
```
cd libpfm/ 
sudo make install 
```
to install necessary libraries and header files on your system.

Build
--------------------------------------------------------------

To generate energy measurement library for JNI, type

```
make lib_shared_CPUScaler
```
To generate hardware counter measurement library for JNI, type

```
make lib_shared_perfChecker
```
To generate both libraries for JNI, type
```
make all
```
How to use
--------------------------------------------------------------

Everytime you need root/sudo access to run the Java code in order to access RAPL MSR.

jRAPL includes the following methods, and it needs to be declared like the following before usage:

- `public native static int ProfileInit();` Initializes jRAPL. Just needs call once in the beginning.

- `public native static String EnergyStatCheck();` It returns a string value, which contains the format of content like: `socket1_gpu/dram_energy#socket1_cpu_energy#socket1_package_energy@socket2_gpu/dram_energy#socket2_cpu_energy#socket2_package_energy`. The character '#' seperates the energy information per the socket. Character '@' seperates different sockets. If it only has one socket, there is no '@'.

- `public native static void ProfileDealloc();`: Free the memory initialized in `ProfileInit()`.

The `EnergyCheckUtils.java` class provide a working sample on how to use jRAPL.


For Perf counter measurement, you don't need root/sudo access to run the library.
It has a seperate set of APIs which include:

- `public native static void perfInit(int numEvents, int isSet);`
- `public native static void singlePerfEventCheck(String eventNames);`
- `public native static void groupPerfEventsCheck(String eventNames);`
- `public native static void perfEnable();`
- `public native static void perfDisable();`
- `public native static void perfSingleRead(int id, long[] buffer);`
- `public native static void perfMultRead(long[] buffer);`
- `public native static long processSingleValue(long[] buffer);`
- `public native static long[] processMultiValue(long[] buffer);`

Energy check and perf counter check can be easily used by accessing helper classes: `EnergyCheckUtils` and `PerfCheckUtils` respectively.

Updates
--------------------------------------------------------------
Support microarchitectures of Skylake and Haswell. (Feb 13 2017)

Support perf hardware counter check. (May 01 2017)



Known limitations
--------------------------------------------------------------

Energy check only works for at most 2 sockets CPU.


Contributions
--------------------------------------------------------------

Want to contribute with code, documentation or bug report? That's great, check out the Issues page.


Contact
--------------------------------------------------------------

- Kenan Liu {kliu20@binghamton.edu}
