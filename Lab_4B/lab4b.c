//FIXME: Temp not convert correctly 
//FIXME: manually kill job during test script or it will freeze
// 妈的， 绝对这个SCRIPT 有问题 
#include <stdlib.h>
#include <stdio.h> 
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <mraa.h>
#include <time.h>
#include <poll.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>

#define Temp_Fahrenheit 1
#define Temp_Celsius 0 
#define IS_STOP 1

// Global Variables for MRAA 
mraa_aio_context temperature_sensor; 
mraa_gpio_context button; 

// Flags
sig_atomic_t volatile running_flag = 1; 
int temperature_scale = Temp_Fahrenheit;
int logging_flag = 0; 
int stop_flag = !IS_STOP;

// Other Global Variables
int sleep_period = 1; // 1/second
FILE *logfile_fd = 0; // log's file descriptor 

// Const for Tempeature Sensor Algorith
const int B = 4275;  // B value of therimistor
const int R0 = 100000;  //R0=100k

// Convert temperature from analog input to real number 
double raw_to_temp(double input){
    double R = (1023.0/input-1.0) * R0;
    float temp = 1.0/(log(R/R0)/B+1/298.15)-273.15; 
    if(temperature_scale){
        return (temp *9 / 5 +32); // Fahrenheit 
    }
    else{
        return temp; // Celsius 
    }
    
}

// Function for Log 
void log_change(){
    // Do nothing 
    // TODO: For Lab4C 
}

// Time printer
// Doesn't include '\n' at the end of stdout 
void print_time()
{
    time_t rawtime;
    struct tm *info;
    char buffer[80];

    time(&rawtime);
    info = localtime(&rawtime);
    strftime(buffer, 10, "%H:%M:%S", info); // Store time
    fprintf(stdout, "%s ", buffer);
    if(logging_flag){
        fprintf(logfile_fd, "%s ", buffer);
        fflush(logfile_fd);
    }
}

// Shutdown Process, close everything 
void shutdown_process(){
    print_time();
    fprintf(stdout, "SHUTDOWN\n");
    if (logging_flag)
    {
        fprintf(logfile_fd, "SHUTDOWN\n");
        fflush(logfile_fd);
    }
    mraa_aio_close(temperature_sensor);
    mraa_gpio_close(button);
    exit(EXIT_SUCCESS);
}

// Set running_flag to logical false 
void do_when_interrupt(int sig){
    if(sig == SIGINT)
        running_flag = 0;
}

void signal_wrapper()
{
    do_when_interrupt(SIGINT);
}

// Parsing function 
void parsing_arg(const char* buffer){
    //if (strcmp(buffer, "OFF")==0)
    if(buffer[0] == 'O' && buffer[1] == 'F' && buffer[2] == 'F')
    {
        running_flag = 0;
    }
    //else if (strcmp(buffer, "SCALE=F") == 0)
    else if (buffer[0] == 'S' && buffer[1] == 'C' && buffer[2] == 'A' && buffer[3] == 'L' && buffer[4] == 'E' && buffer[5] == '=' && buffer[6] == 'F')
    {
        temperature_scale = Temp_Fahrenheit;
    }
    //else if (strcmp(buffer, "SCALE=C") == 0)
    else if (buffer[0] == 'S' && buffer[1] == 'C' && buffer[2] == 'A' && buffer[3] == 'L' && buffer[4] == 'E' && buffer[5] == '=' && buffer[6] == 'C')
    {
        temperature_scale = Temp_Celsius;
    }
    //else if (strcmp(buffer, "STOP") == 0)
    else if (buffer[0] == 'S' && buffer[1] == 'T' && buffer[2] == 'O' && buffer[3] == 'P')
    {
        stop_flag = IS_STOP;
    }
    //else if (strcmp(buffer, "START") == 0)
    else if (buffer[0] == 'S' && buffer[1] == 'T' && buffer[2] == 'A' && buffer[3] == 'R' && buffer[4] == 'T')
    {
        stop_flag = !IS_STOP;
    }
    else if (buffer[0] == 'L' && buffer[1] == 'O' && buffer[2] == 'G' && buffer[3] == ' ')
    {
        log_change();
    }
    else if (buffer[0] == 'P' && buffer[1] == 'E' && buffer[2] == 'R' && buffer[3] == 'I' && buffer[4] == 'O' && buffer[5] == 'D' && buffer[6] == '='){
        int temp_period = atoi(buffer+7);
        if(temp_period < 1){
            fprintf(stderr, "ERROR; period invalid.\n");
            exit(EXIT_FAILURE);
        }
        sleep_period = temp_period;
    }
    else {
        fprintf(stderr, "ERROR; invalid arguments!\n");
    }
    fprintf(stdout, "%s",buffer);
    if (logging_flag)
    {
        fprintf(logfile_fd, "%s", buffer);
        fflush(logfile_fd);
    }
}


int main(int argc, char** argv){
    // RAW data 
    double temp_input_raw = 0;

    // Clean data 
    double temp_pro = 0;

    //Other Variables:
    char fc_indicator;
 
    // Conditions
    int option_index = 0;
    static struct option long_option[] = {
        {"period", required_argument, 0, 'p'},
        {"scale", required_argument, 0, 's'},
        {"log", required_argument, 0, 'l'},
        {0, 0, 0, 0}};
    while (1)
    {
        int c = getopt_long(argc, argv, "p:s:l:", long_option, &option_index);
        if (c == -1) //No more argument
            break;
        switch (c)
        {
        case 'p':
            sleep_period = atoi(optarg);
            if(sleep_period <= 0){
                fprintf(stderr, "ERROR; period unvalid.\n");
                exit(EXIT_FAILURE);
            }
            break;
        case 's':
            fc_indicator = tolower(optarg[0]);
            if(fc_indicator == 'c'){
                temperature_scale = Temp_Celsius;
            } else {
                temperature_scale = Temp_Fahrenheit;
            }
            break;
        case 'l':
            logging_flag = 1;
            logfile_fd = fopen(optarg, "w"); 
            // FIXME: check if its' writable ?? 
            break;
        default:
            //INVALID ARGUMENT(S)
            exit(EXIT_FAILURE);
            break;
        };
    }

    // Initialize temperature Sensor and Button 
    temperature_sensor = mraa_aio_init(1);
    if (temperature_sensor == NULL)
    {
        fprintf(stderr, "ERROR; Failed to initialize TEMPERATURE\n");
        mraa_deinit();
        return EXIT_FAILURE;
    }

    button = mraa_gpio_init(60);
    if (button == NULL)
    {
        fprintf(stderr, "ERROR; Failed to initialize GPIO\n");
        mraa_deinit();
        return EXIT_FAILURE;
    }

    // poll structure 
    struct pollfd pf_array[1];
    pf_array[0].fd = STDIN_FILENO; // polls from stdin
    pf_array[0].events = POLLIN | POLLHUP | POLLERR;

    signal(SIGINT, do_when_interrupt);
    time_t last_cycle_time = 0;
    while(1){
        // Time stamp (to enforce the period)
        time_t current_cycle_time = time(NULL);
        

        // Poll 
        int ret_value = poll(pf_array, 1, 0);
        if (ret_value < 0){
            fprintf(stderr, "ERROR; polling error.\n");
            exit(EXIT_FAILURE);
        }

        // Get temeprature 
        temp_input_raw = mraa_aio_read_float(temperature_sensor);
        temp_pro = raw_to_temp(temp_input_raw);
        mraa_gpio_dir(button, MRAA_GPIO_IN);
        mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &signal_wrapper, NULL);

        // Period
        if (stop_flag != IS_STOP && ((current_cycle_time- last_cycle_time) >= sleep_period))
        {
            // Print ime
            print_time();
            // Print Temperature
            fprintf(stdout, "%.1f\n", temp_pro);
            // Log temperature
            if (logging_flag)
            {
                fprintf(logfile_fd, "%.1f\n", temp_pro);
                fflush(logfile_fd);
            }
            last_cycle_time = current_cycle_time;
        } // END-if 

        // Shutdown 
        if(running_flag == 0){
            shutdown_process();
        }

        // Pollin 
        if(pf_array[0].revents & POLLIN){
            char buffer[50];
            //memset(buffer, 0, 256);
            //int ret_value = read(STDIN_FILENO, &buffer, 256);
            fgets(buffer, 50, stdin);
            //scanf("%s", buffer);
            if(ret_value == 0){
                shutdown_process();
                exit(EXIT_SUCCESS);
            }
            else if (ret_value < 0){
                fprintf(stderr, "ERROR; fail to read.\n");
                exit(EXIT_FAILURE);
            }

            parsing_arg(buffer);
            
        } // END-if 
        
    } // END-while  
    return 0;
}// END-main()






