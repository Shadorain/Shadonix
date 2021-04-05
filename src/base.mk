SHELL ?= /bin/sh
CC ?= gcc
FLAGS ?= -nostdlib -I../include -fno-stack-protector -fPIC
SHADONIX_PATH ?= /home/shadow/Shadonix
DESTDIR ?= /home/shadow/Shadonix

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
