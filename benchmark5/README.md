# Benchmark 5: Callback Signature Mismatch

Multi file C debugging benchmark.

Bug type:
Callback prototype mismatch caused by registering a plugin handler as a dispatcher callback

Expected failure:
Deterministic segfault during filtered message dispatch

Build:

```bash
make
```

Run:

```bash
./eventmesh trigger.txt --trace
```

Notes:

* Crash site is not the root cause
* Intended to require gdb inspection of argument registers, call frames, and callee expectations
* Evaluator notes are kept separately
