# Name: Liuyi Shi
# EMAIL: liuyi.shi@outlook.com
# ID: 904801945

.SILENT:

CC = gcc
CFLAGS  = -g -Wall -Wextra -lmraa -lm

SOURCEFILE = lab4b.c README Makefile 

TARGET = lab4b

default: all 

all: $(TARGET) 

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c
dist: 
	tar -czvf lab4b-904801945.tar.gz $(SOURCEFILE)

clean:
	$(RM) $(TARGET) *.o *.tar.gz

