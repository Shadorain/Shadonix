TARGET=drmlist
CSOURCES=$(shell echo *.c)
OBJECTS = $(CSOURCES:.c=_c.o)

LDFLAGS= -lshado -ldrm -L$(SHADONIX_PATH)/lib
BINDIR = $(PREFIX)/bin

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(FLAGS) -o $(TARGET) $(OBJECTS)

include ../base.mk
