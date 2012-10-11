CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=tosr.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=tosr

all: $(SOURCES) $(EXECUTABLE)
		
$(EXECUTABLE): $(OBJECTS) 
		$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
		$(CC) $(CFLAGS) $< -o $@

clean:
	rm tosr
	rm *.o

.phony : clean
