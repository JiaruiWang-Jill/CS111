#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h> 
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <poll.h>
#include<sys/socket.h>
#include<netdb.h>
#include<netinet/in.h>


//Flags 
int port_flag = 0;
int compress_flag = 0;
int shell_flag = 1;

int debug_mod = 0;
int to_child_pipe[2]; 
int from_child_pipe[2];
pid_t child_pid = -1; 

void read_write(char* buf, int write_fd, int nbytes){
    if(debug_mod){printf("DEBUG__in__read_write\n");}
    int i; 
    for(i=0; i < nbytes; i++){
        switch(*(buf+i)){
            case 0x04: //^D
                if(shell_flag){
                    close(to_child_pipe[1]);
                }
                else {
                    exit(0);
                }
                break;
            case 0x03: //^C
	      if(shell_flag){
                kill(child_pid, SIGINT);}
                break;
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
                write(write_fd, buf+i, 1); //Check the size value !!
        }
    }
}

void read_write_shell_wrapper(int socketfd){
    if(debug_mod){printf("DEBUG__read_write_shell_wrapper\n");}
    struct pollfd pollfd_list[2];
    pollfd_list[0].fd = STDIN_FILENO;
    pollfd_list[0].events = POLLIN | POLLHUP | POLLERR; 
    pollfd_list[1].fd = from_child_pipe[0];
    pollfd_list[1].events = POLLIN | POLLHUP | POLLERR;
    while(1){
        int return_value = poll(pollfd_list, 2, 0); 
        if(return_value < 0){
            fprintf(stderr,"poll() failed!\n");
            exit(1);
        }
        if(return_value == 0) continue; 
        //socketfd POLLIN
        if(pollfd_list[0].revents & POLLIN){
            char buffer_loc[256];
            int bytes_read = read(socketfd, buffer_loc, 256);
            //read_write(buffer_loc,STDOUT_FILENO,bytes_read);
            read_write(buffer_loc, to_child_pipe[1], bytes_read);
        }
        if(pollfd_list[0].revents & POLLERR){
            fprintf(stderr,"pollin error keyboard\n");
            exit(1);
        }
        if(pollfd_list[0].revents & POLLHUP){
            fprintf(stderr,"pollin error keyboard\n");
            exit(1);
        }
        //shell POLLIN
        if(pollfd_list[1].revents & POLLIN){
            char buffer_loc[256];
            int bytes_read = read(pollfd_list[1].fd, buffer_loc, 256);
            read_write(buffer_loc,socketfd,bytes_read);
        }
        if(pollfd_list[1].revents & (POLLHUP | POLLERR)){
	  //printf("Location pipe pohllhup or pollerr\n");
            fprintf(stderr, "error: received POLLHUP|POLLERR.\n"); // Check this ??
            close(from_child_pipe[0]);
            //reset_terminal_mode();
            exit(0);
        }
    }
}

void signal_handler(int sig){
    if(debug_mod){printf("DEBUG__in signal handler\n");}
    if(sig == SIGPIPE){
        if(debug_mod){printf("DEBUG__SIGPIPE caught\n");}
        exit(0);
    }
}

int main(int argc, char *argv[]){
    
    int sockfd, newsockfd, portno, clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n; 

    char* port_num = NULL;

    int option_index = 0;
    static struct option long_option[] = {
        {"port", required_argument, 0, 'p'},
        {"compress", no_argument, 0, 'c'},
        {"debug", no_argument, 0, 'd'},
        {0,0,0,0}
    };
    while(1){
        int c = getopt_long(argc, argv, "p:cd", long_option, &option_index);
        if(c == -1) //No more argument 
            break; 
        switch(c){
            case 'p':
                port_flag = 0;
                portno = atoi(optarg);
                break;
            case 'c':
                compress_flag = 0;
                break;
            case 'd':
                debug_mod = 1;
                break;
            default: 
                exit(1);
                break;
        };
    }

    if(!port_flag){
        fprintf(stderr, "ERROR--port is not specificed.\n");
        exit(1);
    }

    // Socket
    // First call to socket() function 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0){
        frpintf(stderr, "ERROR opening socket.\n");
        exit(1);
    }

    // Initialize socket structure 
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_adder = INADDR_ANY; 
    serv_addr.sin_port = htons(portno);

    // Now bind the host address using bind() 
    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) <0 ){
        frpintf(stderr, "ERROR on binding.\n");
        exit(1);
    }

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

    if(newsockfd < 0){
        frpintf(stderr, "ERROR on accept.\n");
        exit(1);
    }
    
    // Shell
    if(shell_flag){
        if(debug_mod){printf("DEBUG__in_shell_mod\n");}
        signal(SIGPIPE, signal_handler);
        if(pipe(to_child_pipe) == -1){
            fprintf(stderr, "pipe() failed!\n");
            exit(1);
        }
        if(pipe(from_child_pipe) == -1){
            fprintf(stderr, "pipe() failed!\n");
            exit(1);
        }
        child_pid = fork(); 

        if(child_pid >0){ //parent process 
            close(to_child_pipe[0]);
            close(from_child_pipe[1]);
            read_write_shell_wrapper(newsockfd);
        } else if (child_pid == 0) { //child process
            close(to_child_pipe[1]);
            close(from_child_pipe[0]);
            dup2(to_child_pipe[0], STDIN_FILENO);
            dup2(from_child_pipe[1], STDOUT_FILENO);
            dup2(from_child_pipe[1], STDERR_FILENO);
            close(to_child_pipe[0]);
            close(from_child_pipe[1]);

            char *execvp_argv[2];
            char execvp_filename[] = "/bin/bash"; 
            execvp_argv[0] = execvp_filename;
            execvp_argv[1] = NULL; 
            if(execvp(execvp_filename, execvp_argv) == -1){
                fprintf(stderr, "execvp() failed!\n");
                exit(1);
            }
        } else { //fork() failed! 
            fprintf(stderr, "fork() failed!\n");
            exit(1); 
        }
    }


    if(debug_mod){printf("DEBUG__end of main function\n");}
    exit(0); 

}