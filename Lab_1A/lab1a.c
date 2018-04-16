#include <termios.h> 
#include <unistd.h>
#include <sys/types.h> 
#include <sys/wait.h> 
#include <string.h> 
#include <stdio.h>
#include <stdlib.h> 
#include <errno.h> 
#include <getopt.h>
#include <poll.h> 
#include <signal.h> 

int shell_flag = 0;
int to_child_pipe[2]; 
int from_child_pipe[2];
pid_t child_pid = -1; 
struct termios saved_attributes;


void set_terminal_mode(){
    struct termios tattr; 
    char *name; 
    //Make sure stdin is a terminal
    if(!isatty (STDIN_FILENO)){
        fprintf(stderr, "Not a terminal. \n");
        exit(EXIT_FAILURE);
    }
    //Save the terminal attributes 
    tcgetattr(STDIN_FILENO, &saved_attributes);
    atexit(reset_terminal_mode); //DO WE NEED THIS ??

    tcgetattr(STDIN_FILENO, &tattr);
    tattr.c_iflag=ISTRIP;
    tattr.c_oflag=0;
    tattr.c_lflag=0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr); //TCSAFLUSH or TCSANOW ??
}

void reset_terminal_mode(){
    tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);
    if(shell_flag){
        int status = 0; 
        if(waitpid(child_pid, &status, 0) = -1){
            fprintf(stderr, "waitpid() failed!\n")
            exit(1); 
        }
        if(WIFEXITED(status)){
            int exit_status = WEXITSTATUS(status);
            int num_signal = WTERMSIG(status);
            fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d", num_signal, exit_status);
            exit(0);
        }
    }
}

void read_write_shell_wrapper(){
    struct pollfd pollfd_list[2];
    pollfd_list[0].fd = STDIN_FILENO;
    pollfd_list[0].events = POLLIN | POLLHUP | POLLERR; 
    pollfd_list[1].fd = from_child_pipe[0];
    pollfd_list[1].events = POLLIN | POLLHUP | POLLERR;
    while(1){
        int return_value = poll(pollfd_list, 2, 0); 
        if(return_value < 0){
            fprintf(stderr,"poll() failed!\n")
            exit(1);
        }
        //KEYBOARD POLLIN
        if(pollfd_list[0].revents & POLLIN){
            char buffer_loc[256];
            int bytes_read = read(STDIN_FILENO, buffer_loc, 256);
            read_write(buffer_loc,STDOUT_FILENO,bytes_read);
            read_write(buffer_loc, to_child_pipe[1], bytes_read);
        }
        //PIPE POLLIN
        if(pollfd_list[1].revents & POLLIN){
            char buffer_loc[256];
            int bytes_read = read(pollfd_list[1].fd, buffer_loc, 256);
            read_write(buffer_loc,STDOUT_FILENO,bytes_read);
        }
        if(pollfd_list[1].revents & (POLLHUP | POLLERR)){
            close(from_child_pipe[0]);
            reset_terminal_mode();
            exit(0);
        }
    }
}


void read_write(int buf, int write_fd, int nbytes){
    int i; 
    for(i=0; i < nbytes; i++){
        switch(*(buf+i)){
            case 0x04: //^D
                if(shell_flag){
                    //reset_terminal_mode(); ??
                    close(to_child_pipe[1]);
                }
                else {
                    exit(0);
                }
                break;
            case 0x03: //^C 
                kill(child_pid, SIGINT);
                break;
            case '\r':
            case '\n':
                if(write_fd == STDOUT_FILENO){
                    char temp[2] = {'\r','\n'};
                    write(write_fd, temp, 2);
                } else {
                    char temp[1] = {'\n'};
                    wrtie(write_fd, tmp, 1);
                }           
                break;
            default:
                write(write_fd, buffer+i, 1); //Check the size value !!
        }
    }
}

int main(int argc, char **argv){
    
    int option_index = 0;
    static struct option long_option[] = {
        {"shell", no_argument, 0, 's'},
        {0,0,0,0}
    };
    while(1){
        int c = getopt_long(argc, argv, "s", long_option, &option_index);
        if(c == -1) //No more argument 
            break; 
        switch(c){
            case 's':
                shell_flag =1; 
                break; 
            default: 
                //INVALID ARGUMENT(S)
               
                exit(1);
                break;
        };
    }
    
    set_terminal_mode(); 

    if(!shell_flag){
        char buffer[2048];
        ssize_t nbytes = read(read_fd,buffer,2048);
        if(nbytes < 0){
            fprintf(stderr, "read_write() error!\n")
            exit(1);
        }
        read_write(buffer, STDOUT_FILENO, nbytes);
        reset_terminal_mode();
        exit(0); 
    }

    if(shell_flag){
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
            read_write_shell_wrapper();
            
            /*
            char buffer[2048];
            int count = 0;
            count = read(STDIN_FILENO, buffer, 2048);
            write(to_child_pipe[1], buffer, count);
            count = read(from_child_pipe[0], buffer, 2048);
            write(STDOUT_FILENO, buffer, count);
            */
        } else if (child_pid == 0) { //child process
            close(to_child_pipe[1]);
            close(from_child_pipe[0]);
            dup2(to_child_pipe[0], STDIN_FILENO);
            dup2(from_child_pipe[1], STDOUT_FILENO);
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

    reset_terminal_mode();
    exit(0); 

}