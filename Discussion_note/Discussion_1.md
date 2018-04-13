# Disuccsion 1 4/6 

## Premission 
0644 	drw-r--r--   
0 to pass the sanity check 

## Read 
You request 1000 bit from a file of 10 bit   

*Always check the return value* to make sure you have read bits you want 

## Write 
**Caution check the return number**   
return 0 -> EOF 

## Exit 
What's the difference between `_exit` and `exit`?  

`_exit(2)` is a systemcall. It doesn't flush the buffer for you (Check stdout--linebuffer and blockbuffer) because `stdout` is a c libarary. It doesn't know the existence of c libarary functions. 

`exit(3)` is a c library function   

## Dup 
Try ur best to avoid `dup()`, use `dup2()` instead 
dup always picks the lowest fd number. You cannot determine the the return number.  

It's ok in single thread, but in multithreads, it will be a mess. A `5=dup(3)`, B `4=open()`. You assume A will `read(4, ...)`. It causes a race condition. 

## Signal 
Signal function is **not safe in multithreads**. You should use `siagaction(2)` instead. 

