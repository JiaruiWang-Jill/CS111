/*TODO:
  1. check shutdown flag
*/
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <mraa/aio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <poll.h>

#define C_SCALE 0
#define F_SCALE 1
#define STOPPED 1
#define SHUTDOWN 1
#define SIZE_READ 256

int debug = 0;
int period = 1;
int scale = F_SCALE;
FILE *log_fd = 0;
int is_shutted_down = !SHUTDOWN;
int is_stopped = !STOPPED;
int is_logging = 0; //log option is/isn't on

mraa_gpio_context button;
mraa_aio_context sensor;

void report_and_exit(char *msg)
{
    fprintf(stderr, "%s", msg);
    exit(1);
}

void signal_handler()
{
    is_shutted_down = SHUTDOWN;
}
void handle_time()
{
    //printing time information on stdout / log
    time_t timer;
    time(&timer);
    struct tm *time_info = localtime(&timer);
    char buf_time[10];
    strftime(buf_time, 10, "%H:%M:%S", time_info);
    fprintf(stdout, "%s ", buf_time);
    if (is_logging)
    {
        fprintf(log_fd, "%s ", buf_time);
        fflush(log_fd);
    }
}

void handle_shutdown()
{
    handle_time();
    fprintf(stdout, "SHUTDOWN\n");
    if (is_logging)
    {
        fprintf(log_fd, "SHUTDOWN\n");
        fflush(log_fd);
    }
    mraa_aio_close(sensor);
    mraa_gpio_close(button);
    exit(0);
}

//getting temperature data from the board
//converting based on F/C
//TODO: algorithm
double get_temperature()
{
    double raw = mraa_aio_read(sensor);
    //formula
    double R = (660.0 / raw - 1.0) * 100000.0;
    double temp = 1.0 / (log(R / 100000.0) / 4275 + 1 / 298.15) - 273.15;
    if (scale == C_SCALE)
    {
        return temp;
    }
    else
    {
        return temp * 9 / 5 + 32;
    }
}

void do_nothing()
{
}

//formmating
void report_sample()
{
    double temp = get_temperature();

    //check button position
    if (is_shutted_down == SHUTDOWN)
    {
        handle_shutdown();
    }
    else
    {
        handle_time();
        fprintf(stdout, "%.1f\n", temp);
        if (is_logging)
        {
            fprintf(log_fd, "%.1f\n", temp);
            fflush(log_fd);
        }
    }
}

//argument parsing
void argument_parsing(char *msg)
{
    if (msg[0] == 'S' && msg[1] == 'C' && msg[2] == 'A' && msg[3] == 'L' && msg[4] == 'E' && msg[5] == '=' && msg[6] == 'F')
    {
        scale = F_SCALE;
    }
    else if (msg[0] == 'S' && msg[1] == 'C' && msg[2] == 'A' && msg[3] == 'L' && msg[4] == 'E' && msg[5] == '=' && msg[6] == 'C')
    {
        scale = C_SCALE;
    }
    else if (msg[0] == 'L' && msg[1] == 'O' && msg[2] == 'G')
    {
        do_nothing();
    }
    else if (msg[0] == 'S' && msg[1] == 'T' && msg[2] == 'O' && msg[3] == 'P')
    {
        is_stopped = STOPPED;
    }
    else if (msg[0] == 'S' && msg[1] == 'T' && msg[2] == 'A' && msg[3] == 'R' && msg[4] == 'T')
    {
        is_stopped = !STOPPED;
    }
    else if (msg[0] == 'O' && msg[1] == 'F' && msg[2] == 'F')
    {
        is_shutted_down = SHUTDOWN;
    }
    else if (msg[0] == 'P' && msg[1] == 'E' && msg[2] == 'R' && msg[3] == 'I' && msg[4] == 'O' && msg[5] == 'D')
    {
        int period_temp = atoi(msg + 7);
        if (period_temp < 1)
        {
            report_and_exit("invalid period");
        }
        period = period_temp;
    }
    else
    {
        fprintf(stderr, "Unknown Instruction for %s\n", msg);
    }
    if (is_logging)
    {
        fprintf(log_fd, "%s", msg);
        fflush(log_fd);
    }
}

int main(int argc, char **argv)
{
    //initialize variables
    //reading from flags and inputs
    int c;
    char ch;
    char *file_name;
    const char *short_options = "p:l:s:";
    static struct option long_options[] =
        {
            {"period", required_argument, 0, 'p'},
            {"scale", required_argument, 0, 's'},
            {"log", required_argument, 0, 'l'},
            {0, 0, 0, 0}};
    while (1)
    {
        int option_index = 0;
        c = getopt_long(argc, argv, short_options, long_options, &option_index);
        if (c == -1)
        { //no more agruments
            break;
        }
        switch (c)
        {
        case 'p':
            period = atoi(optarg);
            if (period == 0)
            {
                report_and_exit("No conversion performed\n");
            }
            break;
        case 's':
            ch = optarg[0];
            if (ch == 'C' || ch == 'c')
            {
                scale = C_SCALE;
            }
            else
            {
                if (ch != 'F' && ch != 'f')
                {
                    report_and_exit("No such type of temperature\n");
                }
            }
            break;
        case 'l':
            is_logging = 1;
            file_name = optarg;
            break;
        case '?':
        default:
            report_and_exit("Usage: lab4b --log=# --scale=#\n");
        } //end switch
    }     //end while

    //open log file
    if (is_logging)
    {
        log_fd = fopen(file_name, "w");
    }
    struct pollfd ufds[2];
    ufds[0].fd = STDIN_FILENO;
    ufds[0].events = POLLIN;
    ufds[0].revents = POLLERR & POLLHUP;
    sensor = mraa_aio_init(1);
    button = mraa_gpio_init(60);
    mraa_gpio_dir(button, MRAA_GPIO_IN);
    mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &signal_handler, NULL);

    time_t current_time = time(NULL);
    time_t previous_time = 0;
    while (1)
    {
        int ret = poll(ufds, 1, 0);
        if (ret < 0)
        {
            report_and_exit("Polling Error\n");
        }
        current_time = time(NULL);
        if (is_stopped != STOPPED && (current_time - previous_time) >= period)
        {
            report_sample();
            previous_time = current_time;
        } //end if, print sample
        if (is_shutted_down == SHUTDOWN)
        {
            handle_shutdown();
        }

        if (ufds[0].revents & POLLIN)
        {
            char buffer[SIZE_READ];
            memset(buffer, 0, SIZE_READ);
            int ret = read(STDIN_FILENO, &buffer, SIZE_READ);
            if (ret == 0)
            {
                handle_shutdown();
                exit(0);
            }
            if (ret < 0)
            {
                report_and_exit("Read error\n");
            }
            argument_parsing(buffer);
        } //end pollin
    }     //end while
    return 0;
} //end main