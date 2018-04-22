#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h> 
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <poll.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//Flags 
int port_flag = 0;
int log_flag = 0;
int compress_flag = 0;
struct termios saved_attributes;
char* log_path = NULL;

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

void read_write_wrapper(int socket_fd){
    int logFD;
    if (log_flag) {
        logFD = creat(log_path, 0666);
    }
    struct pollfd pollfd_list[2];
    pollfd_list[0].fd = STDIN_FILENO;
    pollfd_list[0].events = POLLIN | POLLHUP | POLLERR; 
    pollfd_list[1].fd = socket_fd;
    pollfd_list[1].events = POLLIN | POLLHUP | POLLERR;

    while(1){
        int return_value = poll(pollfd_list, 2, 0); 
        if(return_value < 0){
            fprintf(stderr,"poll() failed! \n");
            exit(1);
        }
        if(return_value == 0) 
            continue; 

        // Keyboard has input to read POLLIN
        if(pollfd_list[0].revents & POLLIN){
            char buffer_loc[256];
            int bytes_read = read(STDIN_FILENO, buffer_loc, 256);
            read_write(buffer_loc,STDOUT_FILENO,bytes_read);
            if(log_flag){
                    write(logFD, "SENT 1 bytes: ", 14);
                    write(logFD, &buffer_loc, sizeof(char));
                    write(logFD, "\n", sizeof(char));
                }
            read_write(buffer_loc, socket_fd, bytes_read);
            
            continue;  
        }
        if(pollfd_list[0].revents & (POLLERR | POLLHUP)){
            fprintf(stderr,"pollin error keyboard \n");
            exit(1);
        }
        
        // Socket has output to read POLLIN 
        if(pollfd_list[1].revents & POLLIN){
            char buffer_loc[256];
            int bytes_read = read(pollfd_list[1].fd, buffer_loc, 256);
            if(log_flag){
                    char temp[20];
                    sprintf(temp, "RECEIVED %d bytes: ", bytes_read);
                    write(logFD, &temp, 20);
                    write(logFD, &buffer_loc, bytes_read);
                    write(logFD, "\n", sizeof(char));
                }
            read_write(buffer_loc,STDOUT_FILENO,bytes_read);
            
        }
        if(pollfd_list[1].revents & (POLLHUP | POLLERR))
            exit(0);
    }
}


int main(int argc, char *argv[]){
    
    int socket_fd; 
    int portno;
    int n;
    struct sockaddr_in serv_addr;
    struct hostent *server; 

    char buffer[256];

    

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
                port_flag = 1;
                portno = atoi(optarg);
                break;
            case 'l':
                log_flag = 1;
                log_path = optarg;
                break;
            case 'c':
                compress_flag = 0;
                break;
            default:
                break;
        };
    }

    if(!port_flag){
        fprintf(stderr, "ERROR--port is not specificed.\n");
        exit(1);
    }

    set_terminal_mode();

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(socket_fd < 0){
        fprintf(stderr, "ERROR opening socket.\n");
        exit(1);
    }

    server = gethostbyname("localhost");

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *) &serv_addr.sin_addr.s_addr, (char*) server->h_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    // Now connect to the server 
    if(connect(socket_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        fprintf(stderr,"ERROR connecting.\n");
    }

    read_write_wrapper(socket_fd);
    exit(0);

}