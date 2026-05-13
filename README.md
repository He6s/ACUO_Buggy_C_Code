# Benchmark 1: Cross Module Ownership

Multi file C debugging benchmark.

Bug type:
Cross module ownership violation

Expected failure:
Deterministic segfault during cache resize

# Benchmark 2: Realloc Interior Pointer Invalidation

Multi file C debugging benchmark.

Bug type:
Realloc invalidates interior pointers kept by another module

Expected failure:
Deterministic segfault during index lookup after buffer growth

---------------------------------------------
Build:

```bash
make
```

Run:

```bash
./cachebench trigger.txt --trace
```

Notes:

* Crash site is not the root cause
* Intended to require gdb heap and lifetime inspection
* Evaluator notes are kept separately

