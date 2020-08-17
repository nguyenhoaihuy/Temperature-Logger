#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <mraa.h>
#include <mraa/gpio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <ctype.h>

#define TEMP_SENSOR_PIN 1
#define BTN_PIN 60
#define MAX_SIZE 100

//global variables section
static volatile int run_flag = 1;
int id_opt = 0;
char* id_number;
int host_opt = 0;
char* host_name;
int per_opt = 0;
int time_period = 1;
int scl_opt = 0;
char scl_arg = 'F';
int log_opt = 0;
char* log_name; 
FILE *log_filed;
int port_number = 0;

//mraa context structure
mraa_aio_context temp_sensor;
mraa_gpio_context btn_signal;

//remote server connection
struct sockaddr_in svr_add;
struct hostent* svr_host;
int client_fd;

//routine declaration
void do_when_interrupted();
void initialize_context_struc();
void close_context_struc();
float read_and_convert(char scale);
int isID9Digit(const char* id_str);

int main(int argc, char* argv[]){
	float temp;
	char buf[MAX_SIZE];
	int stop = 0;
	int off = 0;
	time_t t;
	struct tm* tm;
	struct pollfd pfds[1];
	int i;
	int id = 0;
	char command[MAX_SIZE];
	
	static const struct option long_options[] = {
			{"id", required_argument, NULL, 1},
			{"host", required_argument, NULL, 2},
    		{"period", required_argument, NULL, 3},
    		{"scale", required_argument, NULL, 4},
    		{"log", required_argument, NULL, 5},
    		{0, 0, 0, 0}
  	};

  	while (1){
    		int op = getopt_long(argc, argv, "", long_options, NULL);
    		if (op == -1) break;

    		switch (op){
			case 1:
				id_opt = 1;
				id_number = optarg;			
				break;
			case 2:
				host_opt = 1;
				host_name = optarg;
				break;
    		case 3:
      			per_opt = 1;
      			time_period = atoi(optarg);
      			break;
    		case 4:
      			scl_opt = 1;
      			scl_arg = optarg[0];
				if (scl_arg != 'C' && scl_arg != 'F'){
					fprintf(stderr, "Scale should be C or F only\n");
					exit(1);
				}
      			break;
    		case 5:
      			log_opt = 1;
				log_name = optarg;
				log_filed = fopen(log_name, "w");
				if (log_filed == NULL){
					fprintf(stderr, "ERROR: Cannot create a log file description\n");
					exit(1);
				}
      			break;
    		default:
      			fprintf(stderr, "Usage: --id=<your ID> --host=<host_name> --period=<period> --scale=<scale> --log=<file_name>\n");
      			exit(1);
    		}
  	}
	
	if (id_opt==0){
		fprintf(stderr, "ERROR: Need id number\n");
		exit(1);
	}
	//fprintf(stdout,"%s\n",id_number);
	if (isID9Digit(id_number)==0){
		fprintf(stderr, "ERROR: ID must be 9 digits\n");
		exit(1);
	}
	if (host_opt==0){
		fprintf(stderr, "ERROR: Need host name\n");
		exit(1);
	}

	if (log_opt==0){
		fprintf(stderr, "ERROR: Need log file name\n");
	}
	
	if (optind >= argc){
		fprintf(stderr, "ERROR: Need a port number\n");
		exit(1);
	}	

	port_number = atoi(argv[optind]);
	if (port_number == 0){
		fprintf(stderr, "ERROR: Need a port number\n");
		exit(1);	
	}
	
	//create a socket for client
	client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (client_fd < 0){
		fprintf(stderr, "ERROR: Cannot create a socket on client\n");
		exit(1);
	}
	pfds[0].fd = STDIN_FILENO;;
    pfds[0].events = POLLIN;

	//setup connection
	svr_host = gethostbyname(host_name);
	if (!svr_host){
		fprintf(stderr, "ERROR: Invalid host name\n");
		exit(1);
	}		
	//fprintf(stdout,"%d",port_number);
	bzero((char*)&svr_add, sizeof(svr_add));
	svr_add.sin_family = AF_INET;
	bcopy((char*)(svr_host->h_addr), (char*)&svr_add.sin_addr.s_addr, svr_host->h_length);
	svr_add.sin_port = htons(port_number);
	
	//connect to the server
	if (connect(client_fd, (struct sockaddr*)&svr_add, sizeof(svr_add)) < 0){
		fprintf(stderr, "ERROR: Cannot connect to the server\n");
		exit(1);
	}

	//redirect fds
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	dup(client_fd);
	dup(client_fd);
	close(client_fd);

	//write ID to the server
	char id_str[15];
	sprintf(id_str, "ID=%s\n", id_number);
	if (write(STDOUT_FILENO, &id_str,strlen(id_str))<0){
		fprintf(stderr, "ERROR: Cannot write ID to the server\n");
		exit(1);
	} 

	if (log_opt == 1){
		fprintf(log_filed,"%s",id_str);	
	}

	initialize_context_struc();
	atexit(close_context_struc);

	while (run_flag){
		t = time(NULL);
				poll(pfds, 1, 100);
		if (pfds[0].revents & POLLIN){
			int n = read(STDIN_FILENO, buf, MAX_SIZE);
			if (n<0){
				fprintf(stderr, "ERROR: Cannot read from stdin");
				exit(1);
			}
			for (i=0; i<n; i++){
				char c = buf[i];
				if (c=='\n'){
					command[id++] = '\0';
					int err = 0;
					if (strncmp(command, "SCALE=F",7)==0){
						scl_arg = command[6];
					} else if (strncmp(command, "SCALE=C",7)==0){
						scl_arg = command[6];
					} else if (strncmp(command, "PERIOD=",7)==0){
						time_period = atoi(command+7);
					} else if (strncmp(command, "STOP", 4)==0){
						stop = 1;
					} else if (strncmp(command, "START", 5)==0){
						stop = 0;
					} else if (strncmp(command, "LOG",3)==0){
					} else if (strncmp(command, "OFF",3)==0){
						off = 1;
					} else {
						err = 1;
					}	
					if (err != 1 && log_opt==1){
						fprintf(log_filed,"%s\n",command);
					}
					id = 0;					
				} else {
					command[id++] = c;
				}
			}			
		}	
		if (off == 1){
			do_when_interrupted();
		}
		if (stop != 1){
			sleep(time_period);
			tm = localtime(&t);
			char result[MAX_SIZE];
			temp = read_and_convert(scl_arg);					
			sprintf(result,"%02d:%02d:%02d %.1f\n", tm->tm_hour, tm->tm_min, tm->tm_sec, temp);
			if (write(STDOUT_FILENO, result, strlen(result))<0){
				fprintf(stderr, "ERROR: Cannot write result to server\n");
				exit(1);
			}
			if (log_opt==1)
				fprintf(log_filed, "%02d:%02d:%02d %.1f\n", tm->tm_hour, tm->tm_min, tm->tm_sec, temp);
			
		}
	
	}
	return 0;
}

float read_and_convert(char scale){
	int a = mraa_aio_read(temp_sensor);
	float R = 1023.0/a-1.0;
	R = 100000*R;
	float temperature = 1.0/(log(R/100000)/4275+1/298.15)-273.15;
	if (scale == 'F'){
		temperature = temperature * (9.0 / 5.0) + 32;
	}
	return temperature;
}

void do_when_interrupted(){
	time_t t;
        struct tm* tm;
	t = time(NULL);
        tm = localtime(&t);
	
	run_flag = 0;
	fprintf(stdout,"%02d:%02d:%02d SHUTDOWN\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
        if (log_opt==1)
        	fprintf(log_filed,"%02d:%02d:%02d SHUTDOWN\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
	exit(0);
}

void initialize_context_struc(){

	temp_sensor = mraa_aio_init(TEMP_SENSOR_PIN);
	if (temp_sensor == NULL){
		fprintf(stderr, "ERROR: Failed to initialize temp AIO\n");
		mraa_deinit();
		exit(1);
	}
	btn_signal = mraa_gpio_init(BTN_PIN);
	if (btn_signal == NULL){
		fprintf(stderr, "ERROR: Failed to initialize btn PGIO\n");
		mraa_deinit();
		exit(1);
	}	
	
	if (mraa_gpio_dir(btn_signal, MRAA_GPIO_IN) != MRAA_SUCCESS){
		fprintf(stderr, "ERROR: Failled to direct the button signal\n");
		exit(1);
	}
	if (mraa_gpio_isr(btn_signal, MRAA_GPIO_EDGE_RISING, &do_when_interrupted, NULL) != MRAA_SUCCESS){
		fprintf(stderr, "ERROR: Failed to set interruption to the button signal\n");
		exit(1);
	} 	
}

void close_context_struc(){
	mraa_aio_close(temp_sensor);
	mraa_gpio_close(btn_signal);
}

int isID9Digit(const char* id_str){
	if (strlen(id_str)!=9) return 0;
	int i;
	for (i=0;i<9;i++){
		if (isdigit(id_str[i])==0) return 0;
	}
	return 1;
}
