#! /usr/local/cs/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#
# output:
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#
#	Early in your implementation, you will not have data for all of the
#	tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","

# how many threads/iterations we can run without failure (w/o yielding)
set title "Scalability-1: Throughput of synchronized lists"
set xlabel "Threads"
set logscale x 2
set ylabel "Throughput (operation/sec)"
set logscale y 10
set output 'lab2b_1.png'

# grep out only single threaded, un-protected, non-yield results
plot \
     "< grep 'list-none-m,[0-9]\\+,1000,1,' lab2_list.csv" using ($2):(1000000000/($7)) \
	title 'list w/mutex' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]\\+,1000,1,' lab2_list.csv" using ($2):(1000000000/($7)) \
	title 'list w/spin-lock' with linespoints lc rgb 'green'


set title "Scalability-2: Per-operation Times for Mutex Protected List Operations"
set xlabel "Threads"
set logscale x 2
set ylabel "Mean time/operation (ns)"
set logscale y 10
set output 'lab2b_2.png'


plot \
     "< grep 'list-none-m,[0-9]\\+,1000,1,' lab2_list.csv" using ($2):($7) \
        title 'completion' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]\\+,1000,1,' lab2_list.csv" using ($2):($8) \
        title 'wait-for-lock time' with linespoints lc rgb 'blue'


set title "Scalability-3: Correct Syncornization of Partitioned Lists"
set xlabel "Threads"
set logscale x 2
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'


plot \
     "< grep 'list-id-none,[0-9]\\+,[0-9]\\+,4,' lab2_list.csv" using ($2):($3) \
        title 'unprotected' with points lc rgb 'red', \
     "< grep 'list-id-m,[0-9]\\+,[0-9]\\+,4,' lab2_list.csv" using ($2):($3) \
        title 'Mutex' with points lc rgb 'blue', \
     "< grep 'list-id-s,[0-9]\\+,[0-9]\\+,4,' lab2_list.csv" using ($2):($3) \
        title 'Spin-Lock' with points lc rgb 'green'

set title "Scalability-4: Throughput of Mutex-Synchronized Partitioned Lists"
set xlabel "Threads"
set logscale x 2
set ylabel "Throughput(opeartions/second)"
set logscale y 10
set output 'lab2b_4.png'


plot \
     "< grep 'list-none-m,[0-9][2]\\?,1000,1,' lab2_list.csv" using ($2):(100000000/($7)) \
        title 'list=1' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9][2]\\?,1000,4,' lab2_list.csv" using ($2):(100000000/($7)) \
        title 'list=4' with linespoints lc rgb 'yellow', \
     "< grep 'list-none-m,[0-9][2]\\?,1000,8,' lab2_list.csv" using ($2):(100000000/($7)) \
        title 'list=8' with linespoints lc rgb 'blue', \
     "< grep 'list-none-m,[0-9][2]\\?,1000,16,' lab2_list.csv" using ($2):(100000000/($7)) \
        title 'list=16' with linespoints lc rgb 'green'

set title "Scalability-5: Throughput of Spin-Lock Synchronized Partitioned Lists"
set xlabel "Threads"
set logscale x 2
set ylabel "Throughput(opeartions/second)"
set logscale y 10
set output 'lab2b_5.png'


plot \
     "< grep 'list-none-s,[0-9][2]\\?,1000,1,' lab2_list.csv" using ($2):(100000000/($7)) \
        title 'list=1' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9][2]\\?,1000,4,' lab2_list.csv" using ($2):(100000000/($7)) \
        title 'list=4' with linespoints lc rgb 'yellow', \
     "< grep 'list-none-s,[0-9][2]\\?,1000,8,' lab2_list.csv" using ($2):(100000000/($7)) \
        title 'list=8' with linespoints lc rgb 'blue', \
     "< grep 'list-none-s,[0-9][2]\\?,1000,16,' lab2_list.csv" using ($2):(100000000/($7)) \
	title 'list=16' with linespoints lc rgb 'green'