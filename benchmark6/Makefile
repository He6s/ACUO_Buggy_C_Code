CC := cc
CFLAGS := -g -O0 -Wall -Wextra -Wpedantic -Werror -std=c11
LDFLAGS :=
TARGET := metapack
OBJS := main.o parser.o router.o table.o format.o util.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
