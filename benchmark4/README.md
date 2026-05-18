# Benchmark 4: Stack Frame Corruption

Multi file C debugging benchmark.

Bug type:
Stack resident parser frame is overwritten by an overlong route name, corrupting nearby parsed fields.

Expected failure:
Deterministic crash after corrupted stack metadata is copied into the route and later used by the planner.

Build:

```bash
make
```

Run:

```bash
./stackroute trigger.txt --trace
```

Notes:

* Crash site is not the root cause
* Intended to require gdb inspection of local stack layout, corrupted fields, and memory around `rsp` or `rbp`
* Evaluator notes are kept separately
