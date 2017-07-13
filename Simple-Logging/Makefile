CC		= gcc
XX		= g++

CFLAGS		= -Wall -O3 -g -c  
INCLUDES	= -I.
LDFLAGS		= 

SOURCES		= $(wildcard *.c *.cpp)
OBJECTS		= $(SOURCES:.c=.o)
# OBJECTS		= $(patsubst %.c,%.o,$(patsubst,%.cpp,%.o))

TARGET		= a.out

all: $(SOURCES) $(TARGET) 

 

$(TARGET): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o: %.c
	$(XX) $(CFLAGS) $(INCLUDES) $< -o $@

.cpp.o:
	$(XX) $(CFLAGS) $(INCLUDES) $< -o $@

.PHONY: clean cleanall run
clean:
	rm -rf $(OBJECTS)

cleanall:
	rm -rf $(OBJECTS) $(TARGET)
run:
	./$(TARGET)