#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>

// Global variables 
long long counter = 0; 
long long my_elapsed_time_in_ns = 0;
int num_of_iterations = 1; 
int num_of_threads = 1;
int  my_spin_lock = 0;
int opt_yield = 0;
pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER; 
typedef enum locks {
  NO_LOCK, MUTEX, SPIN_LOCK, COMPARE_AND_SWAP
} lock_type;
lock_type which_lock = NO_LOCK;

void add(long long *pointer, long long value){
    long long sum = *pointer + value;
    if (opt_yield)
        sched_yield();
    *pointer = sum;
}

void* thread_function_to_run_test(){
    // first perform num_of_iterations times +1
    for(int i = 0; i < num_of_iterations; i++){
        switch(which_lock){
            case NO_LOCK:
            {
                add(&counter, 1);
                break;
            }
            case MUTEX:
            {
                pthread_mutex_lock(&my_mutex);
                add(&counter, 1);
                pthread_mutex_unlock(&my_mutex);
                break; 
            }
            case SPIN_LOCK: 
            {
                while(__sync_lock_test_and_set(&my_spin_lock, 1));
                add(&counter, 1);
                __sync_lock_release(&my_spin_lock);
                break;
            }
            case COMPARE_AND_SWAP:
            {
                long long old, new;
                do {
                    if (opt_yield)
                        sched_yield();
                    old = counter;
                    new = old + 1;
                } while ( __sync_val_compare_and_swap(&counter, old, new) != old);
                break; 
            }
        }
    }
    // seond perform num_of_iterations times -1 
    for (int i = 0; i < num_of_iterations; i++){
        switch(which_lock){
            case NO_LOCK:
            {
                add(&counter, -1);
                break;
            }
            case MUTEX:
            {
                pthread_mutex_lock(&my_mutex);
                add(&counter, -1);
                pthread_mutex_unlock(&my_mutex);
                break; 
            }
            case SPIN_LOCK: 
            {
                while(__sync_lock_test_and_set(&my_spin_lock, 1));
                add(&counter, -1);
                __sync_lock_release(&my_spin_lock);
                break;
            }
            case COMPARE_AND_SWAP:
            {
                long long old, new;
                do {
                    if (opt_yield)
                        sched_yield();
                    old = counter;
                    new = old - 1;
                } while ( __sync_val_compare_and_swap(&counter, old, new) != old);
                break; 
            }
        }
    }

    return NULL; 
}

void print_result(){
    char* print_lock;
    char* option_yield = "";
    if(opt_yield){
        option_yield = "yield-";
    }
    switch(which_lock){
        case NO_LOCK:
            print_lock = "none";
            break;
        case MUTEX:
            print_lock = "m";
            break; 
        case SPIN_LOCK: 
            print_lock = "s";
            break; 
        case COMPARE_AND_SWAP: 
            print_lock = "c";
            break;
    }
    int total_op = num_of_threads * num_of_iterations * 2; 
    long long average_time_per_op = my_elapsed_time_in_ns/total_op;
    printf("add-%s%s,%d,%d,%d,%lld,%lld,%lld\n", option_yield,print_lock, num_of_threads, 
        num_of_iterations, total_op, my_elapsed_time_in_ns, average_time_per_op, counter);
}

int main(int argc, char ** argv){
    // variables
    

    // getting args
    int option_index = 0;
    static struct option long_option[] = {
        {"threads", required_argument, 0, 't'},
        {"iterations", required_argument, 0, 'i'},
        {"yield", no_argument, 0, 'y'},
        {"sync", required_argument, 0, 's'},
        {0,0,0,0}
    };   
    while(1){
        int c = getopt_long(argc, argv, "i:t:ys:", long_option, &option_index);
        if(c == -1) //No more argument 
            break; 
        switch(c){
            case 't':
                num_of_threads = atoi(optarg);
                break; 
            case 'i':
                num_of_iterations = atoi(optarg);
                break;
            case 'y':
                opt_yield = 1;
                break; 
            case 's':{
                char option = optarg[0];
                switch(option){
                    case 's':
                        which_lock = SPIN_LOCK;
                        break; 
                    case 'm':
                        which_lock = MUTEX;
                        break; 
                    case 'c':
                        which_lock = COMPARE_AND_SWAP;
                        break; 
                    default:
                        exit(1);
                }
                break;
            } 
            default: 
                exit(1);
        };
    }
   
    pthread_t threads[num_of_threads];

    // collect start time 
    struct timespec my_start_time;
    clock_gettime(CLOCK_MONOTONIC, &my_start_time);

    // start threads
    for(int i = 0; i< num_of_threads; i++){
        int ret = pthread_create(&threads[i], NULL, thread_function_to_run_test, NULL);  // TODO check attr & para
        if (ret){
          fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", ret);
          exit(2);
       }
    }

    // wait for all threads to exit 
    for(int i = 0; i< num_of_threads; i++){
        int ret = pthread_join(threads[i], NULL); // TODO check teh attr
        if (ret){
          fprintf(stderr, "ERROR; return code from pthread_join() is %d\n", ret);
          exit(2);
       } 
    }

    // collect end time 
    struct timespec my_end_time;
    clock_gettime(CLOCK_MONOTONIC, &my_end_time);

    // calculate the elapsed time 
    my_elapsed_time_in_ns = (my_end_time.tv_sec - my_start_time.tv_sec) * 1000000000;
    my_elapsed_time_in_ns += my_end_time.tv_nsec;
    my_elapsed_time_in_ns -= my_start_time.tv_nsec; 

    // report data 
    print_result();
    // exit 
    exit(0);
}