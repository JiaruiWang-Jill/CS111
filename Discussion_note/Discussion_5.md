# Discussion 5 (5/04)
## Lab_5
`pthread` 
### Wall time vs. CPU time 
* wall time - time in real life 
* CPU time - CPU running time 

**GNUPLOT Version 5** 
* make sure you use v5 instead of v4.6
* You probably need to modify the script 
    * Correct the GNUPLOT Location 
* the `basic add` function is not atmoic 
    * final value is not zero. 
* Run on SEASNET server. It has 16/12 cores 
* It's not possible for two students to have exact graph
* `pthread` use system call to create new threads. It's complicated and time comsuming. For our code, it's very easy. Before the second thread starts, the first thread has already finished. Therefore, multithreads will work in our code with limited number of threads without a proper lock mechanism. 
* when you turn on `yield`, the time each iteration takes will become longer, which result in a lower success rate. 
* time for `yield` will be longer than the total running time(wall time). b/c of the multiple threading. 