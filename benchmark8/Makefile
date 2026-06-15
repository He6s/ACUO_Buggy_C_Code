CC ?= cc
CFLAGS := -std=c11 -g -O0 -Wall -Wextra -Wpedantic -Werror
LDFLAGS :=
TARGET := listmux
OBJS := main.o parser.o scheduler.o task.o ilist.o report.o util.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
