# Benchmark 6: Varargs Type Confusion

Multi file C debugging benchmark.

Bug type:
Varargs misuse causes an unsigned integer field to be consumed as a string pointer by a downstream packet formatter.

Expected failure:
Deterministic segfault during packet emission when `strlen` dereferences a numeric field value as `char *`.

Build:

```bash
make
```

Run:

```bash
./metapack trigger.txt --trace
```

Notes:

* Crash site is not the root cause
* Intended to require gdb inspection of call arguments, argument registers, `va_list` behavior, and incorrect type interpretation
* Evaluator notes are kept separately
