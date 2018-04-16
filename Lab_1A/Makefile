.SILENT:

CC = gcc
CFLAGS  = -g -Wall -Wextra

SOURCEFILE = lab1a.c README Makefile 

TARGET = lab1a

default: all 

all: $(TARGET) 

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c
dist: 
	tar -czvf lab1a-904801945.tar.gz $(SOURCEFILE)

clean:
	$(RM) $(TARGET) *.o *.txt *.tar.gz

