// NAME: Liuyi Shi
// EMAIL: liuyi.shi@outlook.com
// ID: 904801945

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
#include <zlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CR '\015' //carriage return
#define LF '\012' //line feed

z_stream client_to_server;
z_stream server_to_client; 
int sockfd, newsockfd, portno;

char crlf[2] = {CR, LF};
//Flags 
int port_flag = 0;
int compress_flag = 0;
int shell_flag = 1;

int debug_mod = 0;
int to_child_pipe[2]; 
int from_child_pipe[2];
pid_t child_pid = -1; 

void exit_shell(){
    int status;
    waitpid(child_pid, &status, 0);
    fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WIFSIGNALED(status), WEXITSTATUS(status));
}

void read_write(char* buf, int write_fd, int nbytes){
    //if(debug_mod){printf("DEBUG__in__read_write\n");}
    int i; 
    for(i=0; i < nbytes; i++){
        switch(*(buf+i)){
            case 0x04: //^D
	            close(to_child_pipe[1]);
                break;
            case 0x03: //^C
                kill(child_pid, SIGINT);
                break;
            case CR:
            case LF:
                if(write_fd == STDOUT_FILENO){
                    //char temp[2] = {'\r','\n'};
                    write(write_fd, crlf, 2);
                } else {
                    //char temp[1] = {'\n'};
                    char lf = LF;
                    write(write_fd, &lf, 1);
                }           
                break;
            default:
                write(write_fd, buf+i, 1); //Check the size value !!
        }
    }
}

void read_write_shell_wrapper(int socketfd){
    //if(debug_mod){printf("DEBUG__read_write_shell_wrapper\n");}
    struct pollfd pollfd_list[2];
    pollfd_list[0].fd = socketfd;
    pollfd_list[0].events = POLLIN | POLLHUP | POLLERR; 
    pollfd_list[1].fd = from_child_pipe[0];
    pollfd_list[1].events = POLLIN | POLLHUP | POLLERR;
    int status; 
    while(1){
        if(waitpid(child_pid, &status, WNOHANG) != 0){
            close(sockfd);
            close(newsockfd);
            close(from_child_pipe[0]);
            close(to_child_pipe[1]);
            fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WIFSIGNALED(status), WEXITSTATUS(status));
            exit(0);
        }
        int return_value = poll(pollfd_list, 2, 0); 
        if(return_value < 0){
            fprintf(stderr,"poll() failed!\n");
            exit(1);
        }
        if(return_value == 0) continue; 
        //socketfd POLLIN
        if(pollfd_list[0].revents & POLLIN){
            char buffer_loc[2048];
            int bytes_read = read(socketfd, buffer_loc, 2048);
            if(compress_flag){
                char buffer_comp[2048];
                client_to_server.avail_in = bytes_read;
                client_to_server.next_in = (unsigned char *) buffer_loc;
                client_to_server.avail_out = 2048;
                client_to_server.next_out = (unsigned char *) buffer_comp;
                do{
                    inflate(&client_to_server, Z_SYNC_FLUSH);
                }while(client_to_server.avail_in > 0);
                //read_write(buffer_comp, STDOUT_FILENO, 2048-client_to_server.avail_out);
                read_write(buffer_comp, to_child_pipe[1], 2048-client_to_server.avail_out);
            }else{
                //read_write(buffer_loc,STDOUT_FILENO,bytes_read);
                read_write(buffer_loc, to_child_pipe[1], bytes_read);
            }
            
        }
        if(pollfd_list[0].revents & POLLERR){
            fprintf(stderr,"pollin error keyboard\n");
            exit(1);
        }
        if(pollfd_list[0].revents & POLLHUP){
            fprintf(stderr,"pollin pollhup keyboard\n");
            exit(1);
        }
        //shell POLLIN
        if(pollfd_list[1].revents & POLLIN){
            char buffer_loc[2048];
            int bytes_read = read(pollfd_list[1].fd, buffer_loc, 2048);
            
            if(compress_flag){
                char buffer_comp[2048];
                server_to_client.avail_in = bytes_read;
                server_to_client.next_in = (unsigned char *) buffer_loc;
                server_to_client.avail_out = 2048;
                server_to_client.next_out = (unsigned char *) buffer_comp;
                do{
                    deflate(&server_to_client, Z_SYNC_FLUSH);
                }while(server_to_client.avail_in > 0);
                read_write(buffer_comp, socketfd, 2048-server_to_client.avail_out);
            }else{
                read_write(buffer_loc,socketfd,bytes_read);
            }
        }
        if(pollfd_list[1].revents & (POLLHUP | POLLERR)){
	  //printf("Location pipe pohllhup or pollerr\n");
            break;
        }
    }
}

void signal_handler(int sig){
    if(sig == SIGPIPE){
        close(from_child_pipe[0]);
        close(to_child_pipe[1]);
        kill(child_pid,SIGKILL);
        exit_shell();
        exit(0);
    }
}

int main(int argc, char *argv[]){
    
    
    

    //char* port_num = NULL;

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
                port_flag = 1;
                portno = atoi(optarg);
                break;
            case 'c':
                compress_flag = 1;
                server_to_client.zalloc = Z_NULL;
                server_to_client.zfree = Z_NULL;
                server_to_client.opaque = Z_NULL;
                if(deflateInit(&server_to_client, Z_DEFAULT_COMPRESSION) != Z_OK){
                    fprintf(stderr, "ERROR- Failure to deflate server message on server \n");
                    exit(1);
                }
                client_to_server.zalloc = Z_NULL;
                client_to_server.zfree = Z_NULL;
                client_to_server.opaque = Z_NULL;
                if(inflateInit(&client_to_server) != Z_OK ){
                    fprintf(stderr, "ERROR- Failure to inflate client message on server\n");
                    exit(1);
                }
                break;
            case 'd':
                debug_mod = 1;
                break;
            default: 
                fprintf(stderr, "Error in arguments.\n");
                exit(1);
                break;
        };
    }

    if(!port_flag){
        fprintf(stderr, "ERROR--port is not specificed.\n");
        exit(1);
    }

    // Socket
    unsigned int clilen;
    //char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    //int n; 
    // First call to socket() function 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        fprintf(stderr, "ERROR opening socket.\n");
        exit(1);
    }

    // Initialize socket structure 
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; 
    serv_addr.sin_port = htons(portno);

    // Now bind the host address using bind() 
    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) <0 ){
        fprintf(stderr, "ERROR on binding.\n");
        exit(1);
    }

    listen(sockfd, 8); //8 or 5 ??
    clilen = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

    if(newsockfd < 0){
        fprintf(stderr, "ERROR on accept.\n");
        exit(1);
    }
    
    // Shell
    
        
        
        if(pipe(to_child_pipe) == -1){
            fprintf(stderr, "pipe() failed!\n");
            exit(1);
        }
        if(pipe(from_child_pipe) == -1){
            fprintf(stderr, "pipe() failed!\n");
            exit(1);
        }
    signal(SIGPIPE, signal_handler);
    
    child_pid = fork(); 

        if(child_pid >0){ //parent process 
            close(to_child_pipe[0]);
            close(from_child_pipe[1]);
            read_write_shell_wrapper(newsockfd);
            close(sockfd);
            close(newsockfd);
            close(from_child_pipe[0]);
            close(to_child_pipe[1]);
            exit_shell();
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
    
    if(compress_flag){
        inflateEnd(&client_to_server);
        deflateEnd(&server_to_client);
    }


    if(debug_mod){printf("DEBUG__end of main function\n");}
    exit(0); 

}
