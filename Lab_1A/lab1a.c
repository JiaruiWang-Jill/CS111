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
int debug_mod = 0;
int to_child_pipe[2]; 
int from_child_pipe[2];
pid_t child_pid = -1; 
struct termios saved_attributes;

void reset_terminal_mode(){
    if(debug_mod){printf("DEBUG__in__Resetting terminal mode\n");}
    tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);
    //printf("DEBUG_child pid = %d\n", child_pid);
    if(shell_flag){
        int status = 0; 
        if(waitpid(child_pid, &status, 0) == -1){
	  fprintf(stderr, "waitpid() failed! %s\n", strerror(errno));
            exit(1); 
        }
	//else {printf("Wait pid sucess status= %d \n", status);}
        if(WIFEXITED(status)){
	  printf("IN WIFEXITED\n");
            int exit_status = WEXITSTATUS(status);
            int num_signal = WTERMSIG(status);
            fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d \n", num_signal, exit_status);
            exit(0);
        }
	if(WIFSIGNALED(status)){
	  int exit_status = WEXITSTATUS(status);
	  int num_signal = WTERMSIG(status);
	  fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d \n", num_signal, exit_status);
	  exit(0);
	}
    }
}

void set_terminal_mode(){
    if(debug_mod){printf("DEBUG__in__set_terminal_mode\n");}
    struct termios tattr; 
    //char *name; 
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
    tcsetattr(STDIN_FILENO, TCSANOW, &tattr); //TCSAFLUSH or TCSANOW ??
    if(debug_mod){printf("DEBUG__end__set_terminal_mode\n");}
}

void read_write(char* buf, int write_fd, int nbytes){
    if(debug_mod){printf("DEBUG__in__read_write\n");}
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

void read_write_shell_wrapper(){
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
        //KEYBOARD POLLIN
        if(pollfd_list[0].revents & POLLIN){
            char buffer_loc[256];
            int bytes_read = read(STDIN_FILENO, buffer_loc, 256);
            read_write(buffer_loc,STDOUT_FILENO,bytes_read);
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
        //PIPE POLLIN
        if(pollfd_list[1].revents & POLLIN){
            char buffer_loc[256];
            int bytes_read = read(pollfd_list[1].fd, buffer_loc, 256);
            read_write(buffer_loc,STDOUT_FILENO,bytes_read);
        }
        if(pollfd_list[1].revents & (POLLHUP | POLLERR)){
	  //printf("Location pipe pohllhup or pollerr\n");
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
    
    int option_index = 0;
    static struct option long_option[] = {
        {"shell", no_argument, 0, 's'},
        {"debug", no_argument, 0, 'd'},
        {0,0,0,0}
    };
    while(1){
        int c = getopt_long(argc, argv, "sd", long_option, &option_index);
        if(c == -1) //No more argument 
            break; 
        switch(c){
            case 's':
                shell_flag = 1; 
                break; 
            case 'd':
                debug_mod = 1;
                break;
            default: 
                //INVALID ARGUMENT(S)
               
                exit(1);
                break;
        };
    }

    if(debug_mod) printf("DEBUG__shell_flag = %d\n", shell_flag);
    if(debug_mod) printf("DEBUG__debug_flag = %d\n", debug_mod);

    set_terminal_mode(); 

    if(!shell_flag){
        if(debug_mod){printf("DEBUG__in_nonshell_mod\n");}
        char buffer[2048];
        ssize_t nbytes = read(STDIN_FILENO,buffer,2048);
        if(nbytes < 0){
            fprintf(stderr, "read_write() error!\n");
            exit(1);
        }
        while(nbytes){
            read_write(buffer, STDOUT_FILENO, nbytes);
            nbytes = read(STDIN_FILENO,buffer,2048);
        }
        //if(debug_mod){printf("DEBUG__goint to rest terminal mode\n");}
        //reset_terminal_mode();
        if(debug_mod){printf("DEBUG__out__nonshell_mod\n");}
        //exit(0); 
    }

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
    //reset_terminal_mode();
    exit(0); 

}
