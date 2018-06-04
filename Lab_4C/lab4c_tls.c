// TODO: Change logflag to mandatory 
// TODO: Remove Button 
// TODO: Change all stdout to server 
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
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define Temp_Fahrenheit 1
#define Temp_Celsius 0 
#define IS_STOP 1

// Global Variables for MRAA 
mraa_aio_context temperature_sensor; 
mraa_gpio_context button; 

// Flags
int running_flag = 1; 
int temperature_scale = Temp_Fahrenheit;
int logging_flag = 0; 
int stop_flag = !IS_STOP;

// Other Global Variables
int sleep_period = 1; // 1/second
FILE *logfile_fd = 0; // log's file descriptor 
int port_num = 0; // Port number 
int id_num = 000000000; // ID number 
char* host_addr = "NULL"; // Host address 
int socket_fd = 0; // Socket File Descriptor  

// Const for Tempeature Sensor Algorith
const int B = 4275;  // B value of therimistor
const double R0 = 100000.0;  //R0=100k

// SSL structures
const SSL_METHOD *sslmethod;
SSL_CTX *sslctx;
SSL *ssl;

// Convert temperature from analog input to real number 
double raw_to_temp(double input){
    double R = (1023.0/input-1.0);
    R = R * R0;
    double temp = 1.0/(log(R/R0)/B+1/298.15)-273.15; 
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
    // Logging is acomplished by the calling function 
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
    char buffer_time[80];
	memset(buffer_time, 0, 80);
    sprintf(buffer_time, "%s ", buffer);
    SSL_write(ssl, buffer_time, strlen(buffer_time));
    
    if(logging_flag){
        fprintf(logfile_fd, "%s ", buffer);
        fflush(logfile_fd);
    }
}

// Shutdown Process, close everything 
void shutdown_process(){
    print_time();
    char buffer_shutdown[50];
	memset(buffer_shutdown, 0, 50);
    sprintf(buffer_shutdown, "SHUTDOWN\n");
    SSL_write(ssl, buffer_shutdown, strlen(buffer_shutdown));
    if (logging_flag)
    {
        fprintf(logfile_fd, "SHUTDOWN\n");
        fflush(logfile_fd);
    }
    mraa_aio_close(temperature_sensor);
    mraa_gpio_close(button);
    close(socket_fd);
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
    if (strcmp(buffer, "OFF")==0)
    {
        running_flag = 0;
    }
    else if (strcmp(buffer, "SCALE=F") == 0)
    {
        temperature_scale = Temp_Fahrenheit;
    }
    else if (strcmp(buffer, "SCALE=C") == 0)
    {
        temperature_scale = Temp_Celsius;
    }
    else if (strcmp(buffer, "STOP") == 0)
    {
        stop_flag = IS_STOP;
    }
    else if (strcmp(buffer, "START") == 0)
    {
        stop_flag = !IS_STOP;
    }
    else if (buffer[0] == 'L' && buffer[1] == 'O' && buffer[2] == 'G')
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
    
    if (logging_flag)
    {
        fprintf(logfile_fd, "%s\n", buffer);
        fflush(logfile_fd);
    }
}


int main(int argc, char** argv){
    // TCP 
    struct sockaddr_in serv_addr;
    struct hostent *server; 

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
        {"id", required_argument, 0 , 'i'},
        {"host", required_argument, 0, 'h'},
        {0, 0, 0, 0}};
    while (1)
    {
        int c = getopt_long(argc, argv, "p:s:l:i:h", long_option, &option_index);
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
            break;
        case 'i':
            id_num = atoi(optarg); 
            // TODO: Check if it's a valid id number 
            break;
        case 'h':
            host_addr = optarg; 
            break;
        default:
            //INVALID ARGUMENT(S)
            exit(EXIT_FAILURE);
            break;
        };
    }

    // Check the mandatory commands 
    if(logging_flag != 1 && id_num == 000000000 && host_addr == NULL)
        exit(EXIT_ARG);

    // Getting port number 
    port_num = atoi(argv[optind]);

    // Initialize SSL and context structure
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	if (SSL_library_init() < 0) {
	    fprintf(stderr, "ERROR; error in initializing OpenSSL library\n");
	    exit(EXIT_ARG);
	}
	sslmethod = SSLv23_client_sslmethod();
	if ((sslctx = SSL_CTX_new(sslmethod)) == NULL) {
	    fprintf(stderr, "ERROR; error in creating a new SSL context structure\n");
	    exit(EXIT_ARG);
	}

	// create new SSL based on context structure
	ssl = SSL_new(sslctx);

    // Setting up Socket 
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0){
        fprintf{stderr, "ERROR; Cannot open socket.\n"};
    }

    server = gethostbyname(host_addr);
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *) &serv_addr.sin_addr.s_addr, (char*) server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port_num);

    // Connect to the server 
    if(connect(socket_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        fprintf(stderr,"ERROR; Cannot connect to server.\n");
    }

    // initialize SSL connection
	SSL_set_fd(ssl, socket_fd);
	if (SSL_connect(ssl) != 1) {
	    fprintf(stderr, "ERROR; error in building a SSL/TLS session\n");
	    exit(1);
	}

    // Sending ID 
    char buffer_id[50];
	memset(buffer_id, 0, 50);
    sprintf(buffer_id, "ID=%d\n", id_num);
    SSL_write(ssl, buffer_id, strlen(buffer_id));

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
    pf_array[0].fd = socket_fd; // polls from socket 
    pf_array[0].events = POLLIN | POLLHUP | POLLERR;

    signal(SIGINT, do_when_interrupt);
    time_t last_cycle_time = 0;
    time_t current_cycle_time = time(NULL);
    mraa_gpio_dir(button, MRAA_GPIO_IN);
    mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &signal_wrapper, NULL);

    while(1){
        // Time stamp (to enforce the period)
        current_cycle_time = time(NULL);
        

        // Poll 
        int ret_value = poll(pf_array, 1, 0);
        if (ret_value < 0){
            fprintf(stderr, "ERROR; polling error.\n");
            exit(EXIT_FAILURE);
        }

        // Get temeprature 
        temp_input_raw = mraa_aio_read(temperature_sensor);
        temp_pro = raw_to_temp(temp_input_raw);
        
        // Period
        if (stop_flag != IS_STOP && ((current_cycle_time- last_cycle_time) >= sleep_period))
        {
            if (running_flag == 0)
            {
                shutdown_process();
            }
            else {
                // Print ime
                print_time();
                // Print Temperature
                char buffer_temp[50];
	            memset(buffer_temp, 0, 50);
                sprintf(buffer_temp, "%.1f\n", temp_pro);
                SSL_write(ssl, buffer_temp, strlen(buffer_temp));                
                // Log temperature
                if (logging_flag)
                {
                    fprintf(logfile_fd, "%.1f\n", temp_pro);
                    fflush(logfile_fd);
                }
            }
            
            last_cycle_time = current_cycle_time;
        } // END-if 

        // Shutdown 
        if(running_flag == 0){
            shutdown_process();
        }

        // Pollin 
        if(pf_array[0].revents & POLLIN){
            char buffer[256];
            memset(buffer, 0, 256);
            int ret_value = read(socket_fd, &buffer, 256);
            if (ret_value == 0)
            {
                shutdown_process();
                exit(EXIT_SUCCESS);
            }
            else if (ret_value < 0)
            {
                fprintf(stderr, "ERROR; fail to read.\n");
                exit(EXIT_FAILURE);
            }
            char* comm = strtok(buffer, "\n");		//breaks buff into strings when newline read
	        //process commands until no commands left
	        while (comm != NULL && ret_value> 0){
	        	//process command
	        	parsing_arg(comm);
	            comm = strtok(NULL, "\n");		//keep reading from buff, if no input left set to NULL
	        }          
        } // END-if 
        
    } // END-while  
    return 0;
}// END-main()