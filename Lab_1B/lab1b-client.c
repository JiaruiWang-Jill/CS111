#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h> 
#include <getopt.h>
#include <fcntl.h> 
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <poll.h>
#include<sys/socket.h>  

//Flags 
int port_flag = 0;
int log_flag = 0;
int compress_flag = 0;
int socket_fd;
struct termios saved_attributes;

void reset_terminal_mode(){
    tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);
    exit(0);
}

void set_terminal_mode(){
    struct termios tattr; 
    // Make sure stdin is a terminal
    if(!isatty (STDIN_FILENO)){
        fprintf(stderr, "Not a terminal. \n");
        exit(EXIT_FAILURE);
    }
    // Save the terminal attributes 
    tcgetattr(STDIN_FILENO, &saved_attributes);
    atexit(reset_terminal_mode); 
    // Set terminal mode
    tcgetattr(STDIN_FILENO, &tattr);
    tattr.c_iflag=ISTRIP;
    tattr.c_oflag=0;
    tattr.c_lflag=0;
    tcsetattr(STDIN_FILENO, TCSANOW, &tattr); 
}

void read_write(char* buf, int write_fd, int nbytes){
    int i; 
    for(i=0; i < nbytes; i++){
        switch(*(buf+i)){
            case '\r':
            case '\n':
                if(write_fd == STDOUT_FILENO){
                    char temp[2] = {'\r','\n'};
                    write(write_fd, temp, 2);
                } else {
                    char temp[1] = {'\n'};
                    write(write_fd, temp, 1);
                }           
                break;
            default:
                write(write_fd, buf+i, 1); 
        }
    }
}

void read_write_wrapper(){

    struct pollfd pollfd_list[2];
    pollfd_list[0].fd = STDIN_FILENO;
    pollfd_list[0].events = POLLIN | POLLHUP | POLLERR; 
    pollfd_list[1].fd = socket_fd;
    pollfd_list[1].events = POLLIN | POLLHUP | POLLERR;

    while(1){
        int return_value = poll(pollfd_list, 2, 0); 
        if(return_value < 0){
            fprintf(stderr,"poll() failed!-- %s\n", sterror(errno));
            exit(1);
        }
        if(return_value == 0) 
            continue; 

        // Keyboard has input to read POLLIN
        if(pollfd_list[0].revents & POLLIN){
            char buffer_loc[256];
            int bytes_read = read(STDIN_FILENO, buffer_loc, 256);
            read_write(buffer_loc,STDOUT_FILENO,bytes_read);
            read_write(buffer_loc, socket_fd, bytes_read);
            continue; // Check this !! 
        }
        if(pollfd_list[0].revents & (POLLERR | POLLHUP){
            fprintf(stderr,"pollin error keyboard --%s \n", sterror(errno));
            exit(1);
        }
        
        // Socket has output to read POLLIN 
        if(pollfd_list[1].revents & POLLIN){
            char buffer_loc[256];
            int bytes_read = read(pollfd_list[1].fd, buffer_loc, 256);
            read_write(buffer_loc,STDOUT_FILENO,bytes_read);
        }
        if(pollfd_list[1].revents & (POLLHUP | POLLERR))
            exit(0);
    }
}

int main(int argc, char *argv[]){
    
    char* log_path = NULL;
    char* port_num = NULL;

    int option_index = 0;
    static struct option long_option[] = {
        {"port", required_argument, 0, 'p'},
        {"log", required_argument, 0, 'l'},
        {"compress", no_argument, 0, 'c'},
        {0,0,0,0}
    };
    while(1){
        int c = getopt_long(argc, argv, "p:l:c", long_option, &option_index);
        if(c == -1) //No more argument 
            break; 
        switch(c){
            case 'p':
                port_flag = 0;
                port_num = atoi(optarg);
            case 'l':
                log_flag = 0;
                log_path = optarg;
                break;
            case 'c':
                compress_flag = 0;
                break;
            default:
                break;
        };
    }
}