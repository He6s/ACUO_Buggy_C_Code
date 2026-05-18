# Benchmark 1: Cross Module Ownership

Bug type:
Cross module ownership violation

Expected failure:
Deterministic segfault during cache resize

# Benchmark 2: Realloc Interior Pointer Invalidation

Bug type:
Realloc invalidates interior pointers kept by another module

Expected failure:
Deterministic segfault during index lookup after buffer growth

# Benchmark 3: Parser Length Mismatch

Bug type:
Parser advances cursor using decoded payload length instead of declared raw input length

Expected failure:
Deterministic segfault during downstream parent analysis

# Benchmark 4: Stackframe Corruption

Bug type:
Local stack buffer overwrite corrupts nearby stack frame fields

Expected failure:
Deterministic crash after corrupted stack metadata is used in a later module

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

