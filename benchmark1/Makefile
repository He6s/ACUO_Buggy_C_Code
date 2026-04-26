CC := cc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Werror -g -O0
LDFLAGS :=
OBJ := main.o parser.o record.o store.o util.o
TARGET := cachebench

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)

main.o: main.c parser.h store.h util.h
parser.o: parser.c parser.h record.h util.h
record.o: record.c record.h util.h
store.o: store.c store.h util.h
util.o: util.c util.h

clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	./$(TARGET) trigger.txt --trace

.PHONY: all clean run
