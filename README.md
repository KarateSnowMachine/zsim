About this Fork
=====================
This zsim fork is intended to be a simplified and slightly modernized version
of zsim that is easier to get started with. Additionally, it may be a good
target for integrating with Pin3.x.

Features: 
-----------
* Intel Pin is the only external dependency. All other libraries are
  "header-only" and thus built internally. gcc >= 4.6 is still required for
  C++11 support (such as `auto` and such). There was an ABI break with gcc 5.x
  so while it may be possible to compile with newer versions of the compiler,
  it will probably result in subtle errors unless you really know what you're
  doing. If possible, my advice is to stick to gcc 4.x.

* SCons build system replaced by CMake. CMake is industry-standard these days.
* libconfig configuration replaced by JSON configuration. JSON is
  industry-standard these days.
* One possible use-case of this version is as a starting point for zsim
  integration with Pin3.x. Pin3.x requires all Pintools to be build against
  PinCRT and so each extra external library that is linked to zsim must also be
  built against PinCRT. It is my opinion that this represents a non-trivial
  complication in the Pin3.x integration and testing process.

Missing Features/Limitations:
--------------------
* Removing HDF5 dependency means no more HDF5 stats or tracing. 
* Removing gelf dependency means no more debug harness support. 
* Removing zlib dependency means no more address tracing support.
* Some unimplemented checks when copying input config to output config files.
* No more support for DRAMSim2.
* No built-in support for PGO builds.

Note that these removals are not necessarily permanent; they are merely a
temporary convenience to test integration with Pin3.x. Some of these features
can be added back in easily later. Others need testing with PinCRT in Pin3.x.

zsim
====

zsim is a fast x86-64 simulator. It was originally written to evaluate ZCache
(Sanchez and Kozyrakis, MICRO-44, Dec 2010), hence the name, but it has since
outgrown its purpose.
zsim's main goals are to be fast, simple, and accurate, with a focus on
simulating memory hierarchies and large, heterogeneous systems. It is parallel
and uses DBT extensively, resulting in speeds of hundreds of millions of
instructions/second in a modern multicore host. Unlike conventional simulators,
zsim is organized to scale well (almost linearly) with simulated core count.

You can find more details about zsim in our ISCA 2013 paper:
http://people.csail.mit.edu/sanchez/papers/2013.zsim.isca.pdf.


License & Copyright
-------------------

zsim is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation,
version 2.

zsim was originally written by Daniel Sanchez at Stanford University, and per
Stanford University policy, the copyright of this original code remains with
Stanford (specifically, the Board of Trustees of Leland Stanford Junior
University). Since then, zsim has been substantially modified and enhanced at
MIT by Daniel Sanchez, Nathan Beckmann, and Harshad Kasture. zsim also
incorporates contributions on main memory performance models from Krishna
Malladi, Makoto Takami, and Kenta Yasufuku.

zsim was also modified and enhanced while Daniel Sanchez was an intern at
Google. Google graciously agreed to share these modifications under a GPLv2
license. This code is (C) 2011 Google Inc. Files containing code developed at
Google have a different license header with the correct copyright attribution.

Additionally, if you use this software in your research, we request that you
reference the zsim paper ("ZSim: Fast and Accurate Microarchitectural
Simulation of Thousand-Core Systems", Sanchez and Kozyrakis, ISCA-40, June
2013) as the source of the simulator in any publications that use this
software, and that you send us a citation of your work.


Setup
-----

External dependencies: `gcc >=4.6, pin 2.14, cmake 3.13 or higher`

**Natively:** If you use a relatively recent Linux distribution:

1. Clone a fresh copy of the git zsim repository (`git clone <path to zsim repo>`).

2. Download Pin, http://www.pintool.org . Tested with Pin 2.14. Set `$PINPATH`
   to point to where you unpacked your Pin package. 

   NOTE: Because most modern Linux kernels fail the Pin 2.14 version check, the
   `-ifeellucky` flag has been permanently added in `pin_cmd.cpp` to skip the
   Pin version check. This may or may not backfire for your use-case. 

3. In some distributions you may need to make minor changes to the host
   configuration to support large shmem segments and ptrace. See the notes
   below for more details.

4. Compile zsim: `mkdir build; cd build; cmake ..; make`.

5. Launch a test run: `./zsim ../tests/simple.json`

NOTE: zsim uses C++11 features available in `gcc >=4.6` (such as range-based for
loops, strictly typed enums, lambdas, and type inference). Older version of gcc
will not work. zsim can also be built with `icc` (see the `SConstruct` file).

**Using a virtual machine:** If you use another OS, can't make system-wide
configuration changes, or just want to test zsim without modifying your system,
you can run zsim on a Linux VM. We have included a vagrant configuration file
(http://vagrantup.com) that will provision an Ubuntu 12.04 VM to run zsim.
You can also follow this Vagrantfile to figure out how to setup zsim on an
Ubuntu system. Note that **zsim will be much slower on a VM** because it relies
on fast context-switching, so we don't recommend this for purposes other than
testing and development. Assuming you have vagrant installed (`sudo apt-get
install vagrant` on Ubuntu or Debian), follow these steps:

Copy the Vagrant file to the zsim root folder, boot up and provision the base VM
with all dependencies, then ssh into the VM.
```bash
cp misc/Vagrantfile .
vagrant up
vagrant ssh
```

Vagrant automatically [syncs](https://www.vagrantup.com/docs/synced-folders/)
the zsim root folder of your host machine to `/vagrant/` on the guest machine.
Now that you're in the VM, navigate to that synced folder, and simply build and
use zsim (steps 5 and 6 above).
```bash
cd /vagrant/
scons -j4
```

Notes
-----

**Accuracy & validation:** While we have validated zsim against a real system,
you should be aware that we sometimes sacrifice some accuracy for speed and
simplicity. The ISCA 2013 paper details the possible sources of inaccuracy.
Despite our validation efforts, if you are using zsim with workloads or
architectures that are significantly different from ours, you should not
blindly trust these results. Also, zsim can be configured with varying degrees
of accuracy, which may be OK in some cases but not others (e.g., longer bound
phases to reduce overheads are often OK if your application has little
communication, but not with fine-grained parallelism and synchronization).
Finally, in some cases you will need to modify the code to model what you want,
and for some purposes, zsim is just not the right tool. In any case, we
strongly recommend validating your baseline configuration and workloads against
a real machine.

In addition to the results in the zsim paper,
http://zsim.csail.mit.edu/validation has updated validation results.

**Memory Management:** zsim can simulate multiple processes, which introduces some
complexities in memory management. Each Pin process uses SysV IPC shared
memory to communicate through a global heap. Be aware that Pin processes have a
global and a process-local heap, and all simulator objects should be allocated
in the global heap. A global heap allocator is implemented (galloc.c and g\_heap
folder) using Doug Lea's malloc. The global heap allocator functions are as the
usual ones, with the gm\_ prefix (e.g. gm\_malloc, gm\_calloc, gm\_free). Objects
can be allocated in the global heap automatically by making them inherit from
GlobAlloc, which redefines the new and delete operators. STL classes use their
own internal allocators, so they cannot be members of globally visible objects.
To ease this, the g\_stl folder has template specializations of commonly used
STL classes that are changed to use our own STL-compliant allocator that
allocates from the global heap. Use these classes as drop-in replacements when
you need a globally visible STL class, e.g. substitute std::vector with
g\_vector, etc.

**Harness:** While most of zsim is implemented as a pintool (`libzsim.so`), a harness
process (`zsim`) is used to control the simulation: set up the shared memory
segment, launch pin processes, check for deadlock, and ensure termination of
the whole process tree when it is killed. In prior revisions of the simulator,
you could launch the pintool directly, but now you should use the harness.

**Transparency & I/O:** To maintain transparency w.r.t instrumented
applications, zsim does all logging through info/warn/panic methods. With the
sim.logToFile option, these dump to per-process log files instead of the
console. *You should never use cout/cerr or printf in simulator code* ---
simple applications will work, but more complex setups, e.g., anything that
uses pipes, will break.

**Interfacing with applications:** You can use special instruction sequences to
control the simulation from the application (e.g., fast-forward to the region
you want to simulate). `misc/hooks` has wrappers for C/C++, Fortran, and Java,
and extending this to other languages should be easy.

**Host Configuration:** The system configuration may need some tweaks to support
zsim. First, it needs to allow for large shared memory segments. Second, for
Pin to work, it must allow a process to attach to any other from the user, not
just to a child. Use sysctl to ensure that `kernel.shmmax=1073741824` (or larger)
and `kernel.yama.ptrace_scope=0`. zsim has mainly been used in
Ubuntu 11.10, 12.04, 12.10, 13.04, and 13.10, but it should work in other Linux
distributions. Using it in OSs other than Linux (e.g,, OS X, Windows) will be
non-trivial, since the user-level virtualization subsystem has deep ties into
the Linux syscall interface.

**Stats:** Only the plain text stats are now supported.

**Configuration & Getting Started:** A detailed use guide is out of the scope of
this README, because the simulator options change fairly often. In general,
*the documentation is the source code*. You should be willing to occasionally
read the source code to see how different zsim features work. To get familiar
with the way to configure the simulator, the following three steps usually work
well when getting started:

1. Check the examples in the `tests/` folder, play around with the settings, and
   launch a few runs. Config files have three sections, sys (configures the
   simulated system, e.g., core and cache parameters), sim (configures simulation
   parameters, e.g., how frequent are periodic stats output, phase length, etc.),
   and process{0, 1, 2, ...} entries (what processes to run). 

2. Most parameters have implicit defaults. zsim produces an out.json file that
   includes all the default choices (and we recommend that your analysis scripts
   automatically parse this file to check that what you are simulating makes
   sense). Inspecting the out.cfg file reveals more configuration options to play
   with, as well as their defaults.

3. Finally, check the source code for more info on options. The whole system is
   configured in the init.cpp (sys and sim sections) and process\_tree.cpp
   (processX sections) files, so there is no need to grok the whole simulator
   source to find out all the configuration options.

**Hacking & Style Guidelines:** zsim is mostly consistent with Google's C++ style
guide. You can use cpplint.py to check rule violations. We depart from these
guidelines on a couple of aspects:

- 4-space indentation instead of 2 spaces

- 120-character lines instead of 80-char (though you'll see a clear disregard
  for strict line length limits every now and then)

You can use cpplint.py (included in misc/ with slight modifications) to check
your changes. misc/ also has a script to tidy includes, which should be in
alphabetical order within each type (own, system, and project headers).

vim will indent the code just fine with the following options:
`set cindent shiftwidth=4 expandtab smarttab`

Finally, as Google's style guidelines say, please be consistent with the
current style used elsewhere. For example, the parts of code that deal with Pin
follow a style consistent with pintools.

Happy hacking, and hope you find zsim useful!

