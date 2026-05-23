CC := gcc
CFLAGS := -g -O0 -Wall -Wextra -Wpedantic -Werror -std=c11
LDFLAGS :=
TARGET := eventmesh
OBJS := main.o input.o message.o rules.o dispatcher.o plugin.o util.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
