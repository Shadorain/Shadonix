TARGET=init
CSOURCES=$(shell echo *.c)

OBJECTS = $(CSOURCES:.c=_c.o)
LDFLAGS= -lshado -L$(SHADONIX_PATH)/lib

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(FLAGS) -o $(TARGET) $(OBJECTS)

include ../base.mk
