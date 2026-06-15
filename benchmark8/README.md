# Benchmark 8: Intrusive List Metadata Corruption

Multi file C debugging benchmark.

Bug type:
Linked list metadata corruption in an intrusive queue node.

Expected failure:
Deterministic segfault while moving a task between scheduler queues.

Build:

```bash
make
```

Run:

```bash
./listmux trigger.txt --trace
```

Notes:

* Crash site is not the root cause
* Intended to require gdb watchpoints to identify where `next` or `prev` pointers are first corrupted
* Evaluator notes are kept separately
