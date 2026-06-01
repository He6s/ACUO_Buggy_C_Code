# Benchmark 7: Integer Truncation Undersized Allocation

Multi file C debugging benchmark.

Bug type:
Integer truncation causes an undersized allocation that is later overrun by downstream writes.

Expected failure:
Deterministic segfault while storing generated tile records into the atlas table.

Build:

```bash
make
```

Run:

```bash
./tilepack trigger.txt --trace
```

Notes:

* Crash site is not the root cause
* Intended to require gdb inspection of parsed integer widths, allocation sizes, and later memory writes
* Evaluator notes are kept separately
