SHELL ?= /bin/sh
CC ?= gcc
FLAGS ?= -nostdlib -I../include
SHADONIX_PATH ?= ~/Shadonix
DESTDIR ?= ~/Shadonix

PREFIX ?= $(DESTDIR)
BINDIR ?= $(PREFIX)/sbin

OBJECTS += ../crt0_s.o

%_c.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

%_s.o: %.S
	$(CC) $(FLAGS) -c $< -o $@

install:
	cp $(TARGET) $(BINDIR)/

clean:
	rm -r *.o $(TARGET)
