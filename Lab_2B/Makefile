#NAME:Liuyi Shi
#EMAIL:liuyi.shi@outlook.com
#ID:904801945

.SILENT:
CC =gcc
CFLAGS = -g -Wall -Wextra

SOURCEFILE = *.png lab2b_list.csv SortedList.c SortedList.h lab2_list.c Makefile README test_script.sh lab2_list.gp profile.out 

TARGET = lab2b

default: all

all: build

build:	
	$(CC) $(CFLAGS) -o lab2_list lab2_list.c SortedList.c -pthread

tests: build
	chmod +x test_script.sh
	./test_script.sh

graphs:
	gnuplot lab2_list.gp

dist: tests graphs 
	tar -czvf lab2b-904801945.tar.gz $(SOURCEFILE)

clean:
	rm -f *.tar.gz ./lab2_list *.o

lab2_list: clean lab2_list.o SortedList.o
	gcc -o $@ -pthread -g lab2_list.o SortedList.o

profile.gperf: lab2_list
	rm -f ./raw.gperf
	LD_PRELOAD=/u/eng/class/classihi/bin/lib/libprofiler.so.0 CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf >profile.out
	pprof --list=thread_function_to_run_test ./lab2_list ./raw.gperf >> profile.out
	rm -f ./raw.gperf

profile: profile.gperf

