Introduction
--------------------------------------------------------------

For a general explanation, check our [webpage](http://kliu20.github.io/jRAPL/).

jRAPL works for three Intel architectures -- Sandy Bridge, Sandy Bridge_ep (Server) and Ivy Bridge. Each architecture has different RAPL support.

- Sandy Bridge/Ivy Bridge: GPU, CPU and package support.

- Sandy Bridge_ep: DRAM, CPU and package support.

You need check which one is your CPU architecture before you use jRAPL.

For more details about RAPL, check the main [reference](https://software.intel.com/en-us/articles/intel-power-governor)

Pre-requisites
--------------------------------------------------------------

This library uses the kernel `msr` module. To use, type

```
sudo modprobe msr
```

Build
--------------------------------------------------------------

To generate the library for JNI, type

```
make
```

How to use
--------------------------------------------------------------

Everytime you need root/sudo access to run the Java code in order to access RAPL MSR.

jRAPL includes the following methods, and it needs to be declared like the following before usage:

- `public native static int ProfileInit();` Initializes jRAPL. Just needs call once in the beginning.

- `public native static String EnergyStatCheck();` It returns a string value, which contains the format of content like: `socket1_gpu/dram_energy#socket1_cpu_energy#socket1_package_energy@socket2_gpu/dram_energy#socket2_cpu_energy#socket2_package_energy`. The character '#' seperates the energy information per the socket. Character '@' seperates different sockets. If it only has one socket, there is no '@'.

- `public native static void ProfileDealloc();`: Free the memory initialized in `ProfileInit()`.

The `EnergyCheckUtils.java` class provide a working sample on how to use jRAPL.


Known limitations
--------------------------------------------------------------

It only works for at most 2 sockets CPU.


Contributions
--------------------------------------------------------------

Want to contribute with code, documentation or bug report? That's great, check out the Issues page.


Contact
--------------------------------------------------------------

- Kenan Liu {kliu20@binghamton.edu}
