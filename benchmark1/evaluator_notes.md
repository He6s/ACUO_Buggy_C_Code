# Evaluator notes for benchmark 1

## Intended bug family

Cross module ownership and lifetime confusion.

## Ground truth

`store_insert` saves borrowed `key` and `value` pointers directly into the store.
Those pointers originate from `Record` objects created in `parser.c` and owned by `record.c`.
After each command, `main.c` calls `command_dispose`, which destroys the `Record` and poisons both strings with inaccessible page protections.
The store is therefore left holding dangling pointers.

The crash becomes deterministic during the fourth `ADD`, when `store_grow` rehashes older entries and dereferences an inaccessible key in `hash_key`.
The visible crash site is in the resize logic, which is deliberate misdirection.

## Likely debugger workflow

1. Reproduce with `./cachebench trigger.txt --trace`.
2. Break on `store_grow`.
3. Inspect `store->entries[i].key` for existing entries.
4. Compare those addresses with the addresses printed for `record.key` in earlier trace output.
5. Step through `command_dispose` or `record_destroy` and observe that the same addresses are inaccessible before the resize.
6. Confirm that the crash happens when `hash_key` reads from an inaccessible old key pointer.

## Minimal correct fix

Make the store own its own copies of inserted keys and values.
The cleanest patch is to duplicate `key` and `value` inside `store_insert` and also free replaced values correctly on update.
Then the store remains the sole owner of the entry storage and `command_dispose` can still destroy its temporary record safely.

## Expected high level fix shape

- Change `entry_assign_borrowed` into an owning copy path.
- Duplicate key and value with `page_strdup_len` or a normal heap duplication helper.
- On update, free the old stored value, and possibly the old key if replacing whole entries.
- Keep `store_destroy` responsible for freeing stored entries.

## Why this is a good benchmark

- The crash is deterministic.
- The crash site is not the root cause.
- Multiple files participate in the bug.
- Source inspection alone can lead a debugger toward the resize code first.
- Following actual addresses and lifetime transitions in gdb is the quickest route to root cause.
