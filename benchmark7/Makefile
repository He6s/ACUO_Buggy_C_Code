CC ?= gcc
CFLAGS := -D_DEFAULT_SOURCE -std=c11 -g -O0 -Wall -Wextra -Wpedantic -Werror
LDFLAGS :=

PROG := tilepack
OBJS := main.o util.o parser.o arena.o table.o planner.o encoder.o report.o

.PHONY: all clean

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(PROG) $(OBJS)
