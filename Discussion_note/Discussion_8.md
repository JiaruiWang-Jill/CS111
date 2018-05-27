# Discussion 8 (5/25)
## Lab_3A
* The only goal is to produce __Same output__ 
    * If something is wrong, it's fine 
    * Do not need to handle all corner cases 
* Test Script -> Time out 
    * If you want to just check the functionality, you can chagne the time out parameter in the script 
    * To spped up your program, __Scan through the file one time__ 
        * Begining to the Superblock; one time 
    * In test script, `project url ` download it one by one if you want to test it 
### Spec 
* Copy the file to beaglebone (LINUX System) 
* __You can fully ignore the first step__ 
* `debugfs` you cannot understand the output. __LMAO__ 
#### Part 2 
* The order of each line doesn't matter 
* At the very end of your CSV file. __`\n` is very important__ 
* Do not have __EXTRA space, EXTRA newline__
* ***Follow the spec/rules***
### EXT2 
* Table 3.1 
* http://www.nongnu.org/ext2-doc/ext2.html
### Pread
* `char *a = new char[128]` is on the heap, buffer need to be freed manually (USE this one for this project, write to the same buffer)
* `char a[128]` system deallocated. 
* `offset` start point, 
* `count` how many bytes you want to read
### Header File 
* type casting (conver any time of pointer to another one)  
### File system 
* divide blocks into different groups 
* boot locateds on the first group (super block) 
* `1024` is always the number of the super block 
* __13 - print out the pointer information__ 





