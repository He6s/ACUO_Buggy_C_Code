CC ?= cc
CFLAGS := -std=c11 -g -O0 -Wall -Wextra -Wpedantic -Werror -fno-stack-protector
LDFLAGS :=
TARGET := stackroute
OBJS := main.o parser.o planner.o route.o util.o

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) trigger.txt --trace

clean:
	rm -f $(TARGET) $(OBJS)
