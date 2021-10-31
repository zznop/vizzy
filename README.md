# vizzy

```
> ./build/vizzytrace /tmp/heapinfo.trace /bin/find /home/zznop -name vizzy

 _  _  ____  ____  ____  _  _
( \/ )(_  _)(_   )(_   )( \/ )
 \  /  _)(_  / /_  / /_  \  /
  \/  (____)(____)(____) (__)

[*] Dropping libvizzy to disk at /tmp/libvizzy.so
[*] Child process started (pid=13849)
/home/zznop/projects/vizzy
[*] Child process exited with a return code of 0
> python3 ./script/vizzyreport.py /tmp/heapinfo.trace 
HEAP SUMMARY:
    in use at exit   : 11060 bytes in 58 blocks
    total heap usage : 422216 allocs, 422157 frees, 792452324 bytes allocated
```

## Description

**Vizzy is a suite of dynamic analysis tools for profiling heap usage and memory management.** Vizzy consists of a
tracer application (`vizzytrace`) that injects (`LD_PRELOAD`'s) a shared object into a process to hook libc allocation
and free APIs. These hooks log timestamped information on each allocation and free to a trace file for post-processing.
Vizzy contains a script (`vizzyreport.py`) that can processes vizzy trace files to generate reports and visualizations.

## Sample Visualizations

![Alt text](screens/memusage.png "Vizzy Plot")

# Build

To build `vizzytrace` and `libvizzy` install SCons from the package manager and run:

```
scons
```

`vizzyreport.py` requires the Python 3 Bokeh package. This can be installed using pip.

```
pip install bokeh
```
