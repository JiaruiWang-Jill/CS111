#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include "SortedList.h"
#include <signal.h>

// Variables 
SortedList_t* list;
SortedListElement_t *elements;
int num_of_iterations = 1; 
int num_of_threads = 1;
int num_of_lists = 1;
int opt_yield = 0;
int my_spin_lock = 0;
int total_elements = 0;


pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER; 
typedef enum locks {
  NO_LOCK, MUTEX, SPIN_LOCK
} lock_type;
lock_type which_lock = NO_LOCK;
long long my_elapsed_time_in_ns = 0;
long long thread_lock_time[100] = {0};
long long total_lock_time = 0;


// void print_info(){
//   char* lock="";
//   if(which_lock == NO_LOCK){
//     lock = "NONE";
//   }else if(which_lock == MUTEX){
//     lock = "MUEX";
//   }else if(which_lock == SPIN_LOCK){
//     lock = "SPIN_LOCK";
//   }
 
//   char option_yield[] = "";
//   if(!opt_yield){
//       const char* temp = "none";
//       strcpy(option_yield, temp);
//     }
//     if (opt_yield & INSERT_YIELD){
//       const char* temp = "i";
//       strcpy(option_yield, temp);
//     }
//     if (opt_yield & DELETE_YIELD){
//         strcat(option_yield, "d");
//     }
//     if (opt_yield & LOOKUP_YIELD){
//         strcat(option_yield, "l");
//     }
//   fprintf(stderr, "Threads=%d; Iterations=%d; Lock=%s; Yield=%s \n", num_of_threads, num_of_iterations, lock, option_yield);
// }

void segfault_handler(){
    fprintf(stderr, "ERROR; caught segmentation fault\n");
    ////print_info();
    exit(2);
}

void* thread_function_to_run_test(void * index){
    struct timespec start,end;
    // Sortedlist_insert 
    int thread_id = *((int*) index)/num_of_iterations;
    for(int i = *((int*) index); i < *((int*) index)+ num_of_iterations; i++){
        switch(which_lock){
            case NO_LOCK:
            {
                SortedList_insert(list, &elements[i]);
                break;
            }
            case MUTEX:
            {
                if(clock_gettime(CLOCK_MONOTONIC,&start)<0){
                    fprintf(stderr,"ERROR; fail to get time\n");
                    exit(1);
                }
                pthread_mutex_lock(&my_mutex);
                if(clock_gettime(CLOCK_MONOTONIC,&end)<0){
                    fprintf(stderr,"ERROR; fail to get time\n");
                    exit(1);
                }
                
                thread_lock_time[thread_id]+=(end.tv_sec - start.tv_sec) * 1000000000;
                thread_lock_time[thread_id]+=end.tv_nsec;
                thread_lock_time[thread_id]-=start.tv_nsec;
                SortedList_insert(list, &elements[i]);
                pthread_mutex_unlock(&my_mutex);
                break; 
            }
            case SPIN_LOCK: 
            {
                if(clock_gettime(CLOCK_MONOTONIC,&start)<0){
                    fprintf(stderr,"ERROR; fail to get time\n");
                    exit(1);
                }
                while(__sync_lock_test_and_set(&my_spin_lock, 1));
                if(clock_gettime(CLOCK_MONOTONIC,&end)<0){
                    fprintf(stderr,"ERROR; fail to get time\n");
                    exit(1);
                }
                
                thread_lock_time[thread_id]+=(end.tv_sec - start.tv_sec) * 1000000000;
                thread_lock_time[thread_id]+=end.tv_nsec;
                thread_lock_time[thread_id]-=start.tv_nsec;
                SortedList_insert(list, &elements[i]);
                __sync_lock_release(&my_spin_lock);
                break;
            }
        }
    }

    // Get length 
    int list_length = 0; 
    switch(which_lock){
        case NO_LOCK:
        {
            list_length = SortedList_length(list);
            break;
        }
        case MUTEX:
        {
            if(clock_gettime(CLOCK_MONOTONIC,&start)<0){
                fprintf(stderr,"ERROR; fail to get time\n");
                exit(1);
            }
            pthread_mutex_lock(&my_mutex);
            if(clock_gettime(CLOCK_MONOTONIC,&end)<0){
                fprintf(stderr,"ERROR; fail to get time\n");
                exit(1);
            }
                
            thread_lock_time[thread_id]+=(end.tv_sec - start.tv_sec) * 1000000000;
            thread_lock_time[thread_id]+=end.tv_nsec;
            thread_lock_time[thread_id]-=start.tv_nsec;
            list_length = SortedList_length(list);
            pthread_mutex_unlock(&my_mutex);
            break; 
        }
        case SPIN_LOCK: 
        {
            if(clock_gettime(CLOCK_MONOTONIC,&start)<0){
                fprintf(stderr,"ERROR; fail to get time\n");
                exit(1);
            }
            while(__sync_lock_test_and_set(&my_spin_lock, 1));
            if(clock_gettime(CLOCK_MONOTONIC,&end)<0){
                fprintf(stderr,"ERROR; fail to get time\n");
                exit(1);
            }
                
            thread_lock_time[thread_id]+=(end.tv_sec - start.tv_sec) * 1000000000;
            thread_lock_time[thread_id]+=end.tv_nsec;
            thread_lock_time[thread_id]-=start.tv_nsec;
            list_length = SortedList_length(list);
            __sync_lock_release(&my_spin_lock);
            break;
        }
    }
    if (list_length == -1) {
        fprintf(stderr, "ERROR; failed to get length of list\n");
	    //print_info();
        exit(2);
    }

    // Looks up and delete
    SortedListElement_t *new = NULL;
    for(int i = *((int*) index); i < *((int*) index)+ num_of_iterations; i++){
        switch(which_lock){
            case NO_LOCK:
            {
                new = SortedList_lookup(list, elements[i].key);
                if(new == NULL){
                    fprintf(stderr, "ERROR; fail to find the element in the list\n");
		    //print_info();
                    exit(2);
                }
                if(SortedList_delete(new)){
                    fprintf(stderr, "ERROR; fail to delete the element in the list\n");
		    //print_info();
                    exit(2);
                }
                break;
            }
            case MUTEX:
            {
                if(clock_gettime(CLOCK_MONOTONIC,&start)<0){
                    fprintf(stderr,"ERROR; fail to get time\n");
                    exit(1);
                }
                pthread_mutex_lock(&my_mutex);
                if(clock_gettime(CLOCK_MONOTONIC,&end)<0){
                    fprintf(stderr,"ERROR; fail to get time\n");
                    exit(1);
                }
                
                thread_lock_time[thread_id]+=(end.tv_sec - start.tv_sec) * 1000000000;
                thread_lock_time[thread_id]+=end.tv_nsec;
                thread_lock_time[thread_id]-=start.tv_nsec;
                SortedList_insert(list, &elements[i]);
                new = SortedList_lookup(list, elements[i].key);
                if(new == NULL){
                    fprintf(stderr, "ERROR; fail to find the element in the list\n");
		    //print_info();
                    exit(2);
                }
                if(SortedList_delete(new)){
                    fprintf(stderr, "ERROR; fail to delete the element in the list\n");
		    //print_info();
                    exit(2);
                }
                pthread_mutex_unlock(&my_mutex);
                break; 
            }
            case SPIN_LOCK: 
            {
                if(clock_gettime(CLOCK_MONOTONIC,&start)<0){
                    fprintf(stderr,"ERROR; fail to get time\n");
                    exit(1);
                }
                while(__sync_lock_test_and_set(&my_spin_lock, 1));
                if(clock_gettime(CLOCK_MONOTONIC,&end)<0){
                    fprintf(stderr,"ERROR; fail to get time\n");
                    exit(1);
                }
                
                thread_lock_time[thread_id]+=(end.tv_sec - start.tv_sec) * 1000000000;
                thread_lock_time[thread_id]+=end.tv_nsec;
                thread_lock_time[thread_id]-=start.tv_nsec;
                new = SortedList_lookup(list, elements[i].key);
                if(new == NULL){
                    fprintf(stderr, "ERROR; fail to find the element in the list\n");
		    //print_info();
                    exit(2);
                }
                if(SortedList_delete(new)){
                    fprintf(stderr, "ERROR; fail to delete the element in the list\n");
		    //print_info();
                    exit(2);
                }
                __sync_lock_release(&my_spin_lock);
                break;
            }
        }
    }
    return NULL; 
}

void print_result(){
    char* print_lock;
    char option_yield[20] = "";
    if(!opt_yield){
      const char* temp = "none";
      strcpy(option_yield, temp);
    }
    if (opt_yield & INSERT_YIELD){
      const char* temp = "i";
      strcpy(option_yield, temp);
    } 
    if (opt_yield & DELETE_YIELD){
        strcat(option_yield, "d");
    }
    if (opt_yield & LOOKUP_YIELD){
        strcat(option_yield, "l");
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
    }
    int total_op = num_of_threads * num_of_iterations * 3; 
    long long average_time_per_op = my_elapsed_time_in_ns/total_op;
    int total_lock_op = num_of_threads *(num_of_iterations*2 +1);
    long long average_wait_for_lock = total_lock_time/total_lock_op;
    printf("list-%s-%s,%d,%d,%d,%d,%lld,%lld,%lld\n", option_yield,print_lock, num_of_threads,num_of_iterations,num_of_lists ,total_op, my_elapsed_time_in_ns, average_time_per_op,average_wait_for_lock);
}

int main(int argc, char ** argv){
    
    int option_index = 0;
    static struct option long_option[] = {
        {"threads", required_argument, 0, 't'},
        {"iterations", required_argument, 0, 'i'},
        {"yield", required_argument, 0, 'y'},
        {"sync", required_argument, 0, 's'},
        {"list",required_argument, 0, 'l'},
        {0,0,0,0}
    };
    while(1){
        int c = getopt_long(argc, argv, "i:t:y:s:l:", long_option, &option_index);
        if (c == -1) //No more argument 
            break;
        switch(c){
            case 't':
                num_of_threads = atoi(optarg);
                break; 
            case 'i':
                num_of_iterations = atoi(optarg);
                break;
            case 'y':
                for(size_t i =0; i < strlen(optarg); i++){
                    if(optarg[i] == 'i')
                        opt_yield |= INSERT_YIELD;
                    else if (optarg[i] == 'd')
                        opt_yield |= DELETE_YIELD;
                    else if (optarg[i] == 'l')
                        opt_yield |= LOOKUP_YIELD; 
                }
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
                    default:
                        exit(1);
                }
                break; 
            }
            case 'l':{
                num_of_lists = atoi(optarg);
                break;
            }


        } 
    }
    signal(SIGSEGV, segfault_handler);

    // initialize a list 
    list = malloc(sizeof(SortedList_t));
    list->key = NULL;
    list->next = list;
    list->prev = list;


    // create and initialize list's elements 
    total_elements = num_of_iterations * num_of_threads;
    elements = malloc(total_elements * sizeof(SortedListElement_t));
 
    /* Intializes random number generator */
    srand(time(NULL));
    for(int i = 0; i < total_elements; i++){
        int random_number = rand() % 26; //Bound to a-z 
        char* random_key = malloc(2 * sizeof(char)); // 1 char key + null byte
        random_key[0] = 'a' + random_number; // turn randNumber into character
        random_key[1] = '\0';

        elements[i].key = random_key;
    }

    pthread_t threads[num_of_threads];

    // collect start time 
    struct timespec my_start_time;
    clock_gettime(CLOCK_MONOTONIC, &my_start_time);

    int index[num_of_threads] ;
    for(int i = 0;i<num_of_threads;i++)
        index[i] = i*num_of_iterations;

    // start threads
    for(int i = 0; i< num_of_threads; i++){
        int ret = pthread_create(&threads[i], NULL, thread_function_to_run_test, (void*)&index[i]);  
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

    // check the length of the list 
    if(SortedList_length(list) != 0){
        fprintf(stderr, "Error; length of the list is not zero  ");
	//print_info();
        exit(2);
    }
    
    // calculate the elapsed time 
    for(int i = 0; i< num_of_threads; i++){
        total_lock_time += thread_lock_time[i];
    }
    my_elapsed_time_in_ns = (my_end_time.tv_sec - my_start_time.tv_sec) * 1000000000;
    my_elapsed_time_in_ns += my_end_time.tv_nsec;
    my_elapsed_time_in_ns -= my_start_time.tv_nsec; 

    free(list);
    free(elements);

    // report data 
    print_result();

    // exit 
    exit(0);

}
