# Benchmark 1, cross module ownership bug

This project builds a small key value cache that reads commands from a text file.

## Build

```bash
make
```

## Reproduce the crash

```bash
./cachebench trigger.txt --trace
```

The crash is deterministic with the provided trigger file.

## Command format

- `ADD <key> <value> <ttl>`
- `FIND <key>`
- `DEL <key>`
- `DUMP`
- `HELP`

## Notes for benchmark runners

- Compile with debug info already enabled by the Makefile.
- The trace mode prints pointer values that are useful during debugging.
- The provided trigger is intended to crash during a resize path, not during initial parsing.

The evaluator notes are intentionally separated into `evaluator_notes.md`.
