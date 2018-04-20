# Discussion 3 (4/20)
## Lab_1B
* mapping on the shell (do on server side makes much sense), but doing on the client side also works 
* Both server and client are running on the same server 
## Change Terminal Setting 
* Client side or Server side? 
* Server doesn't need to change the terminal setting.
## Socket 
* Tutorial copy/paste 
* On Tutorialspoint, search `unix socket` 
### Client Side 
* change `bzero` and `bcopy` to `memset` and `memcpy` 
    * `bzero` and `bcopy` are no longer supporetd nowadays
    * https://www.tutorialspoint.com/unix_sockets/socket_client_example.htm
### Server Side 
#### Port number 
* if the port number is used by others, the binding `bind()` function will fail 

## Compression 
* Copy/paste 
* `zlib` library
* Stream Data Structures 
    * Control of how compression and decompression works 
* `available_out` is zero -> no more buffer -> cannot do anymore compression, you need to move data in buffer to other place, then, you can keep compressing 
* `availabe_in` is zero -> no more data need to be compressed
* __USE a BUFFER that is LARGE enough__ 
    * so you don't need to check `avaiable_out` every time 
* `next_in` and `next_out` are next buffer address 
