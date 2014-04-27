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

char latestTemp[100];
pthread_t thread_id;
pthread_mutex_t lock;
char * state;
double average_temp;
double maxF;
double minF;
double maxC;
double minC;
char min [100]; 
char max [100];
char avg [100];
int F = 0;
int arduinoFd;
  double CTempCount = 0;
  double FTempCount = 0;
  double CTempTotal = 0;
  double FTempTotal = 0;
  double tempholder = 0;

void *fun(void *);

int start_server(int PORT_NUMBER)
{
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

	char * pointerToState = (char *) malloc(sizeof(char) *2); //malloc space for pointer to state
	if (pointerToState == NULL) {
	    printf("Not enough memory for pointerToState");
	    return 0;
	}
	
	while(1) {

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
      *state = pointerToState[1]; //set parse to char after the $ sign
	printf("%s ", state); //print out state
	char * six = "6"; //temp stats state
	
	int bytes_written = 0;
	switch (state[0]) {
	  case '0':
	    F = 0;
	    bytes_written = write(arduinoFd, "f", sizeof(char));
	    break;
	  case '1':
	    F = 1;
	    bytes_written = write(arduinoFd, "f", sizeof(char));
	    break;
	  case '2':
	    bytes_written = write(arduinoFd, "s", sizeof(char));
	    break;
	  case '3':
	    bytes_written = write(arduinoFd, "s", sizeof(char));
	    break;
	  case '4':
	    bytes_written = write(arduinoFd, "p", sizeof(char));
	    break;
	  case '5':
	    bytes_written = write(arduinoFd, "p", sizeof(char));
	    break;
	  case '6':
	    if (F) { 
	      average_temp = FTempTotal/FTempCount;
	      sprintf(avg, "%lf\n", average_temp);	
	      sprintf(min, "%lf\n", minF);	    
	      sprintf(max, "%lf\n", maxF);	  
	    }
	    else {
	      average_temp = CTempTotal/CTempCount;
	      sprintf(avg, "%lf\n", average_temp);	
	      sprintf(min, "%lf\n", minC);	    
	      sprintf(max, "%lf\n", maxC);	 
	    }
	    break;
	}
	
	if (bytes_written == -1) printf("Problem sending signal to arduino");
	
      // this is the message that we'll send back
      /* it actually looks like this:
        {
           "name": "cit595"
        }
	*/
	char* prefix = "{\n\"name\": \"";
	char* suffix = "\"\n}\n";
	  char* reply;
	  if (strcmp(state, six) == 0) {
		reply = (char*) malloc(strlen(avg) + strlen(max) + strlen(min) + strlen(prefix) + strlen(suffix) + 1);
		strcpy(reply, prefix);
		strcat(reply, min);
		strcat(reply, max);
		strcat(reply, avg);
		strcat(reply, suffix);

	  }
	  else {
		reply = (char*) malloc(strlen(latestTemp) + strlen(prefix) + strlen(suffix) + 1);
		strcpy(reply, prefix);
		strcat(reply, latestTemp);
		strcat(reply, suffix);
	  }
	
	// 6. send: send the message over the socket
	// note that the second argument is a char*, and the third is the number of chars
	send(fd, reply, strlen(reply), 0);
	//printf("Server sent message: %s\n", reply);

	// 7. close: close the socket connection
	close(fd); //get new reading
	}
      close(sock);
      printf("Server closed connection\n");
  
      return 0;
} 

void *fun(void *a) { //arduino thread
  state = (char *)malloc(sizeof(char)*100); //state for arduino commands
    if (state == NULL) {
      printf("Not enough memory for state");
      return NULL;
    }

  arduinoFd = open("/dev/ttyUSB11", O_RDWR);
  if (arduinoFd == -1) {
	printf("Error connecting to Arduino.");
  	return NULL;
  }
  
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
	strcpy(latestTemp, updatedString); //copying latest finished temp into latestTemp
	sscanf(latestTemp, "%lf", &tempholder); //scan latest temp into temp holder

	if (F) { //temp is farenheit so add this to the farenheight total
	  if (FTempTotal == 0) {
		minF = tempholder;
		maxF = tempholder;
	  }
	  else if (tempholder < minF) {
		minF = tempholder;
	  } //update min
	  else if (tempholder > maxF) {
		maxF = tempholder;
	  } //update max
	  FTempTotal += tempholder;
	  FTempCount++;
	}
	else { //temp is celsius so add to CTempTotal
	  if (CTempTotal == 0) {
		minC = tempholder;
		maxC = tempholder;
	  }
	  else if (tempholder < minC) {
		minC = tempholder;
	  } //update min
	  else if (tempholder > maxC) {
		maxC = tempholder;
	  } //update max
	  CTempTotal += tempholder;
	  CTempCount++;
	}
  }
  close(arduinoFd);
}



int main(int argc, char *argv[])
{

  // check the number of arguments
  if (argc != 2)
    {
      printf("\nUsage: server [port_number]\n");
      exit(0);
    }

  int PORT_NUMBER = atoi(argv[1]);
  start_server(PORT_NUMBER);
}

