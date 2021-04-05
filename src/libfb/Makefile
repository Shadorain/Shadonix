LDFLAGS = -shared
TARGET = libfb.so
CSOURCES = $(shell echo *.c)

OBJECTS = $(CSOURCES:.c=_c.o)

BINDIR = $(PREFIX)/lib

all: $(TARGET)

$(TARGET): $(OBJECTS)
	ld $(FLAGS) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

include ../base.mk
