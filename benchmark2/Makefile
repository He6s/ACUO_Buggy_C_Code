CC ?= cc
CFLAGS ?= -std=c11 -g -O0 -Wall -Wextra -Wpedantic -Werror
LDFLAGS ?=

TARGET := sliceidx
OBJS := main.o parser.o database.o index.o textbuf.o util.o

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

main.o: main.c database.h parser.h util.h textbuf.h index.h
parser.o: parser.c parser.h util.h
database.o: database.c database.h parser.h textbuf.h index.h util.h
index.o: index.c index.h util.h
textbuf.o: textbuf.c textbuf.h util.h
util.o: util.c util.h

run: $(TARGET)
	./$(TARGET) trigger.txt --trace

clean:
	rm -f $(TARGET) $(OBJS)
