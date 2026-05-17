# Benchmark 3: Parser Length Mismatch

Multi file C debugging benchmark.

Bug type:
Parser length mismatch across encoded and raw payload bytes

Expected failure:
Deterministic segfault during parent path analysis after a malformed node appears in the catalog

Build:

```bash
make
```

Run:

```bash
./framewalk trigger.txt --trace
```

Notes:

* Crash site is not the root cause
* Intended to require gdb inspection of raw input bytes, parsed length fields, cursor advancement, and downstream catalog usage
* Evaluator notes are kept separately
