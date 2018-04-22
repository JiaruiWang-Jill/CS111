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
#include <assert.h> 
#include <zlib.h>

//Flags 
int port_flag = 0;
int log_flag = 0;
int compress_flag = 0;
struct termios saved_attributes;
char* log_path = NULL;
int logFD;
int socket_fd;

z_stream client_to_server;
z_stream server_to_client; 

void reset_terminal_mode(){
    close(logFD);
    close(socket_fd);
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
    
    if (log_flag) {
        if( (logFD = creat(log_path, 0666)) == -1){
            fprintf(stderr, "Failure to create/write to file. \n");
        }
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
            char buffer_loc[2048];
            int bytes_read = read(STDIN_FILENO, buffer_loc, 2048);
            read_write(buffer_loc,STDOUT_FILENO,bytes_read);
            if(compress_flag){
                char buffer_comp[2048];
                client_to_server.avail_in = bytes_read;
                client_to_server.next_in = ( unsigned char *) buffer_loc;
                client_to_server.avail_out = 2048;
                client_to_server.next_out = ( unsigned char *)buffer_comp; 

                do{
                    deflate(&client_to_server, Z_SYNC_FLUSH);
                }while(client_to_server.avail_in > 0);

                read_write(buffer_comp, socket_fd, 2048 - client_to_server.avail_out);
                if(log_flag){
                    char temp_string[20];
                    sprintf(temp_string, "SENT %d bytes: ", 2048-client_to_server.avail_out);
                    write(logFD, &temp_string, 20);
                    write(logFD, &buffer_comp, 2048-client_to_server.avail_out);
                    write(logFD, "\n", sizeof(char));
                }
            }
            else {
                if(log_flag){
                    write(logFD, "SENT 1 bytes: ", 14);
                    write(logFD, &buffer_loc, sizeof(char));
                    write(logFD, "\n", sizeof(char));
                }
                read_write(buffer_loc, socket_fd, bytes_read);
            }
            
            
    
        }
        if(pollfd_list[0].revents & (POLLERR | POLLHUP)){
            fprintf(stderr,"pollin error keyboard \n");
            exit(1);
        }
        
        // Socket has output to read POLLIN 
        if(pollfd_list[1].revents & POLLIN){
            char buffer_loc[2048];
            int bytes_read = read(pollfd_list[1].fd, buffer_loc, 2048);
            if(bytes_read == 0){
                break;
            }
            if(compress_flag){
                char buffer_comp[2048];
                server_to_client.avail_in = bytes_read;
                server_to_client.next_in = (unsigned char *) buffer_loc;
                server_to_client.avail_out = 2048;
                server_to_client.next_out = (unsigned char *) buffer_comp;

                do{
                    inflate(&server_to_client, Z_SYNC_FLUSH);
                } while(server_to_client.avail_in > 0);

                read_write(buffer_comp, STDOUT_FILENO,2048-server_to_client.avail_out);
                
            } else {
                read_write(buffer_loc,STDOUT_FILENO,bytes_read);
            }
            if(log_flag){
                char temp[20];
                sprintf(temp, "RECEIVED %d bytes: ", bytes_read);
                write(logFD, &temp, 20);
                write(logFD, &buffer_loc, bytes_read);
                write(logFD, "\n", sizeof(char));
            }
            
            
            
        }
        if(pollfd_list[1].revents & (POLLHUP | POLLERR))
            exit(0);
    }
}


int main(int argc, char *argv[]){
    
     
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
                compress_flag = 1;
                client_to_server.zalloc = Z_NULL;
                client_to_server.zfree = Z_NULL;
                client_to_server.opaque = Z_NULL;
                if(deflateInit(&client_to_server, Z_DEFAULT_COMPRESSION) != Z_OK){
                    fprintf(stderr, "ERROR- Failure to deflate client message on client \n");
                    exit(1);
                }
                server_to_client.zalloc = Z_NULL;
                server_to_client.zfree = Z_NULL;
                server_to_client.opaque = Z_NULL;
                if(inflateInit(&server_to_client) != Z_OK ){
                    fprintf(stderr, "ERROR- Failure to inflate server message on client\n");
                    exit(1);
                }
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
    if(compress_flag){
        inflateEnd(&server_to_client);
        deflateEnd(&client_to_server);
    }
    exit(0);

}