# Disuccsion 1 4/13
## Change to the test script 
search for `sight`   
add `-r` after `SEND "^C"`   
on seasnet server only   
check whether it is `-` or `\`

## Suggested reading for understadning this assignment 
*Is it a Terminal* to *Modes* in the GNU Manule 

## Canonical 
* `read(3)` won't read until `\n` 
* `^c` is never received by your program. SIGNAL 

## Noncanonical 
* one character at a time 
* __WRONG `read(0, buf, 1)`__ <--its not what 1 char at a time mean 
* once the user press one byte, your process should immediately receive it
* if the user type many bytes, process should also accpet it 
* '1'23 "123" you can read it one at a time, or all together 
* your terminal will not interpert the meaning of byte like `^C` `ERASE`, it's up to your prcoess 

## Fork 
* child is going to have it's own pid, everything else is going to be the same 
```c
int pid = fork() //pid return from child is 0
if(pid == 0) {
    //I'm the child process 
} else if (pid > 0) {
    //I'm the parent process 
}
printf("abc"); //both the parent and child will print 'abc' 
```
### what happend to descriptors?
child process will have a descriptor table will be the same as the parent process. Both tables will point to the same file 

## Pipe 
* __Always close the unsed file descriptors__



## Lab1a 
* For resetting the terminal, just use the professor's code. 
* no `SIGINT` sent to your process
* use `WEXITSTATUS` and `WTERMSIG` for this assignment, but one of them is just a trash value. 
