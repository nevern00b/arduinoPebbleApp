/* 
This code primarily comes from 
http://www.prasannatech.net/2008/07/socket-programming-tutorial.html
and
http://www.binarii.com/files/papers/c_sockets.txt
*/

#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

char latestTemp[100];
pthread_t thread_id;
pthread_mutex_t lock;
char * state;
double average_temp;
double minTemp;
double maxTemp;
char min [100]; 
char max [100];
char avg [100];
int F = 0;
int S = 0;
int P = 0;
int arduinoFd;
int failedReadings = 0;
double TempCount = 0;
double TempTotal = 0;
double tempholder = -1;
pthread_mutex_t * fLock;
pthread_mutex_t * tLock;
void *fun(void *);
char * filename;

void toggleF() {
    pthread_mutex_lock(fLock);
    if (F) F = 0;
    else F = 1;
    pthread_mutex_unlock(fLock);
}

int getF() {
    int returnF;
    pthread_mutex_lock(fLock);
    returnF = F;
    pthread_mutex_unlock(fLock);
    return returnF;
}

double fToC(int f) {
  return (f - 32) / 1.8;
}

double cToF(int c) {
  return (c * 1.8) + 32;
}

int start_server(int PORT_NUMBER) {
	pthread_create(&thread_id, NULL, &fun, NULL);
	
	
	// structs to represent the server and client
	struct sockaddr_in server_addr,client_addr;    

	int sock; // socket descriptor

	// 1. socket: creates a socket descriptor that you later use to make other system calls
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	  	perror("Socket");
	  	exit(1);
	}
	int temp;
	if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&temp,sizeof(int)) == -1) {
	  	perror("Setsockopt");
	  	exit(1);
	}

	// configure the server
	server_addr.sin_port = htons(PORT_NUMBER); // specify port number
	server_addr.sin_family = AF_INET;         
	server_addr.sin_addr.s_addr = INADDR_ANY; 
	bzero(&(server_addr.sin_zero),8); 
	  
	// 2. bind: use the socket and associate it with the port numberthe user should be able to change the 7-segment display on the sensor to show the temperature either in Fahrenheit or Celsius. The user should be able to put the sensor into a "stand-by" mode in which it is not reporting temperature readings. When the sensor is in "stand-by" mode, the user should also be able to tell it to resume reporting the readings.
	if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
	  	perror("Unable to bind");
	  	exit(1);
	}

	// 3. listen: indicates that we want to listn to the port to which we bound; second arg is number of allowed connections
	if (listen(sock, 5) == -1) {
	  	perror("Listen");
	  	exit(1);
	}

	// once you get here, the server is set up and about to start listening
	printf("\nServer configured to listen on port %d\n", PORT_NUMBER);
	fflush(stdout);
	char * pointerToState = NULL;

	while(1) {
	  	int stopRunning = 0;
		// 4. accept: wait here until we get a connection on that port
	  	int sin_size = sizeof(struct sockaddr_in);
	  	int fd = accept(sock, (struct sockaddr *)&client_addr,(socklen_t *)&sin_size);
	  	printf("Server got a connection from (%s, %d)\n", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

		// buffer to read data into
	  	char request[1024];     

		// 5. recv: read incoming message into buffer
	  	int bytes_received = recv(fd,request,1024,0);
		// null-terminate the string
	  	request[bytes_received] = '\0';

		//parse request
		pointerToState = strchr(request, '$'); //parses request after $ to capture pebble output
		if (pointerToState == NULL) continue;
		//printf("%s ", pointerToState); //print out state
		
		char* prefix = "{\n\"name\": \"";
		char* suffix = "\"\n}\n";
		char* reply;
		
		//Checks state received
		int bytes_written = 0;
		struct stat buffer;   
		//int 
		//int fileExist = stat(filename, &buffer);//access(filename, F_OK);
		//FILE * fileCheck = fopen(filename,"r");
		if (failedReadings > 5000000 && !S) {
		    reply = "{\n\"name\": \"&Arduino disconnected\"\n}\n";
		}
		else {
		  switch (pointerToState[1]) {
			  case '0':
				if (S) {
					  reply = (char*) malloc(strlen(prefix) + strlen(suffix) + 27);
					  strcpy(reply, prefix);
					  strcat(reply, "&Server in standby");
					  strcat(reply, suffix);
				  }
				  //Sends actual temperature readings preceeded by F or C
				  else {
					  reply = (char*) malloc(strlen(latestTemp) + strlen(prefix) + strlen(suffix) + 2);
					  strcpy(reply, prefix);
					  if (F) strcat(reply,"F");
					  else strcat(reply,"C");
					  strcat(reply, latestTemp);
					  strcat(reply, suffix);
				  }
				  break;
			  case '1': //toggle F and C 
				  printf("writing to arduino");
				  bytes_written = write(arduinoFd, "f", sizeof(char)); 
				  if (getF()) {
				  reply = "{\n\"name\": \"Unit changed to C\"\n}\n";
				  toggleF();
				  }
				  else {
				    reply = "{\n\"name\": \"Unit changed to F\"\n}\n";
				    toggleF();
				  }
				  break;
			  case '2': //toggle standby mode
				  bytes_written = write(arduinoFd, "s", sizeof(char));
				  if (S) {
				    reply = "{\n\"name\": \"Standby mode OFF\"\n}\n";
				    S = 0;
				  }
				  else {
				    reply = "{\n\"name\": \"Standby mode ON\"\n}\n";
				    S = 1;
				  }
				  break;
			  case '3': //toggle party mode
				  bytes_written = write(arduinoFd, "p", sizeof(char));
				  if (P) {
				    reply = "{\n\"name\": \"Party mode OFF\"\n}\n";
				    P = 0;
				  }
				  else {
				    reply = "{\n\"name\": \"Party mode ON\"\n}\n";
				    P = 1;
				  }
				  break;
			  case '4': //returns average temperature readings
				  if (getF()) { 
					  pthread_mutex_lock(tLock);
					  average_temp = TempTotal/TempCount;
					  sprintf(avg, "%.2lf F\\n", cToF(average_temp));	
					  sprintf(min, "%.2lf F\\n", cToF(minTemp));	    
					  sprintf(max, "%.2lf F\\n", cToF(maxTemp));
					  pthread_mutex_unlock(tLock);
				  }
				  else {
					  pthread_mutex_lock(tLock);
					  average_temp = TempTotal/TempCount;
					  sprintf(avg, "%.2lf C\\n", average_temp);	
					  sprintf(min, "%.2lf C\\n", minTemp);	    
					  sprintf(max, "%.2lf C\\n", maxTemp);
					  pthread_mutex_unlock(tLock);
				  }
				  
				  reply = (char*) malloc(strlen(avg) + strlen(max) + strlen(min) + strlen(prefix) + strlen(suffix) + 16);
				  strcpy(reply, prefix);
				  strcat(reply, "min: ");
				  strcat(reply, min);
				  strcat(reply, "max: ");
				  strcat(reply, max);
				  strcat(reply, "avg: ");
				  strcat(reply, avg);
				  strcat(reply, suffix);
				  break;
			  case '5': //turns server off (irreversible)
				  stopRunning = 1;
				  break;
			  default:	
				  break;
		  }
		}
		if (bytes_written == -1) printf("Problem sending signal to arduino");
		// 6. send: send the message over the socket
		// note that the second argument is a char*, and the third is the number of chars
		send(fd, reply, strlen(reply), 0);
		//printf("Server sent message: %s\n", reply);
		if (pointerToState[1] == '0' && pointerToState[1] == '4')
		  free(reply);
		// 7. close: close the socket connection
		close(fd); //get new reading
		if (stopRunning) {
			break;
		}
	}
	close(sock);
	pthread_join(thread_id,NULL);
	printf("Server closed connection\n");
	return 0;
} 

void *fun(void *a) { //arduino thread
	filename = "/dev/ttyUSB10";
	arduinoFd = open(filename, O_RDWR);
	if (arduinoFd == -1) {
		printf("Error connecting to Arduino.");
		failedReadings = 5000000;
	}
	minTemp = 1500;
	//configuration code
	struct termios options;
	tcgetattr(arduinoFd, &options);//get new reading
	cfsetispeed(&options, 9600);
	cfsetospeed(&options, 9600);
	tcsetattr(arduinoFd, TCSANOW, &options);
	
	char buf[100];
	int bytes_read = read(arduinoFd, buf, 100);
	int j = 0; //buffer counter
	int i = 0; //string counter
	int totalReadings = 0;
	char updatedString[100];
	while(1) { 
	//getting temperature from arduino constantly
		bytes_read = read(arduinoFd, buf, 100);
		for (i = 0; i < bytes_read; i++) {
			updatedString[j] = buf[i];
			j = j + 1;
			if (buf[i] == '\n') {
				updatedString[j -1] = '\0';
				j = 0;
			}
		}
		if (bytes_read > 0) {
		  strcpy(latestTemp, updatedString); //copying latest finished temp into latestTemp
		  sscanf(latestTemp, "%lf", &tempholder); //scan latest temp into temp holder
		  
		  if (totalReadings > 4) {
		    if (getF()) { //if temp is fahrenheit, convert to C, which is the unit we store int
			tempholder = fToC(tempholder);
		    }
		    pthread_mutex_lock(tLock);
		    if (tempholder < minTemp && tempholder != -1) {
			minTemp = tempholder;
		    }
		    else if (tempholder > maxTemp) {
			maxTemp = tempholder;
		    }
		    TempTotal += tempholder;
		    TempCount++;
		    pthread_mutex_unlock(tLock);
		    
		  }
		  totalReadings += 1;
		  printf("%d\n",failedReadings);
		  failedReadings = 0;
		}
		else {
		    failedReadings += 1;
		}
	}
	close(arduinoFd);
}



int main(int argc, char *argv[])
{
	fLock = malloc (sizeof(pthread_mutex_t));
	tLock = malloc (sizeof(pthread_mutex_t));
// check the number of arguments
	if (argc != 2)
	{
		printf("\nUsage: server [port_number]\n");
		exit(0);
	}
	pthread_mutex_init(fLock, NULL);
	int PORT_NUMBER = atoi(argv[1]);
	start_server(PORT_NUMBER);
}

