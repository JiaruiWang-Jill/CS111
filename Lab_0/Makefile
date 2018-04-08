.SILENT:

CC = gcc
CFLAGS  = -g -Wall -Wextra

SOURCEFILE = lab0.c README Makefile backtrace.png breakpoint.png

TARGET = lab0

default: all 

all: $(TARGET) 

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

check: default TestALL
	if [[ -s faillog.txt ]]; then \
	    cat faillog.txt; \
	    rm faillog.txt; \
	else \
	    echo "Passed all tests"; \
	fi

dist: 
	tar -czvf lab0-904801945.tar.gz $(SOURCEFILE)

clean:
	$(RM) $(TARGET) *.o *.txt *.tar.gz

TestALL: catch_test io_test bad_input_test bad_output_test unrecongize_test_1 unrecongize_test_2
 
catch_test:
	./lab0 --segfault --catch &> /dev/null; \
	if [[ $$? -ne 4 ]]; then \
	    echo "error: segfault catch doesn't work" >> faillog.txt; \
	else \
		echo "success: seg and catch" >> successlog.txt; \
	fi

io_test: 
	echo "testline1" > file_in.txt; \
	./lab0 --input=file_in.txt --output=file_out.txt &> /dev/null; \
	if [[ $$? -ne 0 ]]; then \
	    echo "error: program fail" >> faillog.txt; \
	else \
		echo "success: io_test" >> successlog.txt; \
	fi
	rm -r file_in.txt file_out.txt

bad_input_test: 
	echo "testline1" > no_exe_file_in.txt; \
	chmod -r no_exe_file_in.txt; \
	./lab0 --input=no_exe_file_in.txt --output=file_out.txt &> /dev/null; \
	if [[ $$? -ne 2 ]]; then \
	    echo "error: bad_input_test fail. Didn't detect unreadlbe file" >> faillog.txt; \
	else \
		echo "success: bad_input_test" >> successlog.txt; \
	fi
	rm -r no_exe_file_in.txt 

bad_output_test: 
	echo "testline1" > file_in.txt; \
	touch no_exe_file_out.txt; \
	chmod -w no_exe_file_out.txt; \
	./lab0 --input=file_in.txt --output=no_exe_file_out.txt &> /dev/null; \
	if [[ $$? -ne 3 ]]; then \
	    echo "error: bad_output_test fail. Didn't detect unreadlbe file" >> faillog.txt; \
	else \
		echo "success: bad_output_test" >> successlog.txt; \
	fi
	rm -rf no_exe_file_out.txt

unrecongize_test_1: 
	./lab0 --asa &> /dev/null; \
	if [[ $$? -ne 1 ]]; then \
	    echo "error: unrecognized argument didn't detech" >> faillog.txt; \
	else \
		echo "success: unrecongize_test_1" >> successlog.txt; \
	fi

unrecongize_test_2: 
	./lab0 -sa &> /dev/null; \
	if [[ $$? -ne 1 ]]; then \
	    echo "error: unrecognized argument didn't detech" >> faillog.txt; \
	else \
		echo "success: unrecongize_test_2" >> successlog.txt; \
	fi
	

