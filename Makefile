
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra

TARGET = c2048
OBJECTS = 2048.o main.o

PREFIX ?= /usr/local

all : $(TARGET)

$(TARGET): $(OBJECTS)	
	$(CC) $(CFLAGS) -o $@ $^

debug:
	$(CC) $(CFLAGS) -g -o $(TARGET) $@ $^

clean:
	rm -rf *.out *.o *.dSYM $(TARGET)

install: $(TARGET)
	cp -f $(TARGET) $(PREFIX)/bin/

uninstall:
	rm -f $(PREFIX)/bin/$(TARGET)

.PHONY: debug clean install uninstall
