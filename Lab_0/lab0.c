//
//                       _oo0oo_
//                      o8888888o
//                      88" . "88
//                      (| -_- |)
//                      0\  =  /0
//                    ___/`---'\___
//                  .' \\|     |// '.
//                 / \\|||  :  |||// \
//                / _||||| -:- |||||- \
//               |   | \\\  -  /// |   |
//               | \_|  ''\---/''  |_/ |
//               \  .-\__  '-'  ___/-. /
//             ___'. .'  /--.--\  `. .'___
//          ."" '<  `.___\_<|>_/___.' >' "".
//         | | :  `- \`.;`\ _ /`;.`/ - ` : | |
//         \  \ `_.   \_ __\ /__ _/   .-` /  /
//     =====`-.____`.___ \_____/___.-`___.-'=====
//                       `=---='
//
//
//     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//               佛祖保佑         永无BUG
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h> 
#include <getopt.h>
#include <fcntl.h> 
#include <signal.h>
#include <errno.h>
#include <string.h>

void read_to_write(int fd0, int fd1){
    char buffer; 
    int ret_val = 0;
    while(1){
        ret_val = read(fd0, &buffer, 1);
        if(ret_val == -1){
            exit(-1);
        }
        else if(ret_val == 0)
            break;
        write(fd1, &buffer, 1); 
    }
}

void seg_fault(){
    char *ptr = NULL; 
    *ptr = 'C'; 
}

void handler(int signum) {
    fprintf(stderr, "Segmentation fault: %d\n", signum);
    exit(4);
}

int main(int argc, char *argv[]){
    int option_index = 0;
    char *opt_in = NULL;
    char *opt_out = NULL; 
    int other = 0;  
    static struct option long_option[] = {
        {"input", required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'},
        {"segfault", no_argument, 0, 's'},
        {"catch", no_argument, 0 ,'c'},
        {0,0,0,0}
    };
    
    while(1){
        int c = getopt_long(argc, argv, "i:o:sc", long_option, &option_index);
        if(c == -1) //No more argument 
            break; 
        switch(c){
            case 'i':
                opt_in = optarg; 
                break;
            case 'o':
                opt_out = optarg;
                break;
            case 's':
                other = 1; 
                break; 
            case 'c':
                other = 2; 
                break; 
            default: 
                printf("Usage: lab0 --input=filename --output=filename <optional args.>\n");
                exit(2);
        };
    }

    //Unrecognized pattern 
    if(argv[optind-1] != NULL){
        printf("Unrecognized option '%s'\n", argv[optind-1]);
        printf("Usage: lab0 --input=filename --output=filename <optional args.>\n");
                exit(2);
    }
    
    if(other == 1)
        seg_fault(); 
    else if(other == 2)
        signal(SIGSEGV, handler);

    if(opt_in){
        int fd0 = open(opt_in, O_RDONLY);
        if(fd0 < 0){
            fprintf(stderr, "Unable to open the input file: %s\n", strerror(errno));
            exit(2);
        }
        else{
            close(0);
            dup(fd0);
            close(fd0);
        }                 
    }

    if(opt_out){
        int fd1 = creat(opt_out, 0666);
        if(fd1 < 0){
            fprintf(stderr, "Unable to open the output file: %s\n", strerror(errno));
            exit(3);
        }
        else{
            close(1);
            dup(fd1);
            close(fd1); 
        }
    }
    
    read_to_write(0,1);
    exit(0);

}
