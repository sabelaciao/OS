# NO TOCAR / NOT MODIFIED ME ##
CC=gcc
FLAGS=-Wno-implicit-function-declaration
CFLAGS=-I.
###############################

# MODIFIED ME ##

OBJ = scripter.o mygrep.o

all: scripter mygrep

%.o: %.c 
	$(CC) $(FLAGS) -c -o $@ $< $(CFLAGS)

scripter: scripter.o
	$(CC) $(FLAGS) -L. -o $@ $< $(LIBS)

mygrep: mygrep.o
	$(CC) $(FLAGS) -L. -o $@ $< $(LIBS)

clean:
	rm -f ./scripter.o ./scripter ./mygrep.o ./mygrep