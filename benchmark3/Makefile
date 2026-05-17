CC ?= cc
CFLAGS := -g -O0 -Wall -Wextra -Wpedantic -Werror -std=c11
LDFLAGS :=

TARGET := framewalk
OBJS := main.o util.o parser.o catalog.o analyzer.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.c util.h parser.h catalog.h analyzer.h
util.o: util.c util.h
parser.o: parser.c parser.h util.h
catalog.o: catalog.c catalog.h parser.h util.h
analyzer.o: analyzer.c analyzer.h catalog.h

clean:
	rm -f $(TARGET) $(OBJS)
