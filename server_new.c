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
      
      // 2. bind: use the socket and associate it with the port number
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
     int i = 0;

	char * pointerToState = (char *) malloc(sizeof(char) *2);
	while(i < 100) {

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

	//if(sscanf(request, "GET / HTTP/1.1\nHost: 158.130.63.2:3001\nContent-Type: %s\nAccept: */*\nUser-Agent: //PebbleApp/2.1.1 CFNetwork/672.0.8 Darwin/14.0.0\nAccept-Language: en-us\nAccept-Encoding: gzip, deflate\nConnection: keep-//alive", state) != 1) {
		//printf("There was an error with the request");
	//}
	
	pointerToState = strchr(request, '$');
	*state = pointerToState[1]; //set parse to char after the $ sign


      printf("Here comes the message:\n");
      printf("%s\n", request);
      printf("Here comes the parse:\n");
      printf("%s\n", state); //put sent message from pebble into our state
      
      // this is the message that we'll send back
      /* it actually looks like this:
        {
           "name": "cit595"
        }
      */
     char* prefix = "{\n\"name\": \"";
     char* suffix = "\"\n}\n";
      char *reply = (char*) malloc(strlen(latestTemp) + strlen(prefix) + strlen(suffix) + 1);
      strcpy(reply, prefix);
      strcat(reply, latestTemp);
      strcat(reply, suffix);
      printf("%s\n",reply);
      // 6. send: send the message over the socket
      // note that the second argument is a char*, and the third is the number of chars
      send(fd, reply, strlen(reply), 0);
      //printf("Server sent message: %s\n", reply);

      // 7. close: close the socket connection
      close(fd);
	i++;
	}
      close(sock);
      printf("Server closed connection\n");
  
      return 0;
} 

void *fun(void *a) { //arduino thread
state = (char *)malloc(sizeof(char)*100); //state for arduino commands
  char * zero = "0"; //turn light off
  char * one = "1"; //change from celsius to farenheit
  char * two = "2"; //turn this on
  char * three = "3"; //get new reading
	//strcpy(state, two);
  int bytes_written;
	

  int fd = open("/dev/ttyUSB11", O_RDWR);
  if (fd == -1) {
    printf("Connecting to arduino did not work");
  	return NULL;
  }
  //make the error return actually do something
  struct termios options;
  tcgetattr(fd, &options);
  cfsetispeed(&options, 9600);
  cfsetospeed(&options, 9600);
  tcsetattr(fd, TCSANOW, &options);

  char buf[100];
  int bytes_read = read(fd, buf, 100);

  int j = 0; //buffer counter
  int i = 0; //string counter
  char updatedString[100];
char * lightMsg = "b";
char * lightMsg2 = "a";
int ip = 0;
  while(1) { 
	ip++;	
	//this is getting temperature information from arduino
	if (ip % 9999999 == 0) {
	printf("state in arduino loop: %s!!\n", state);
	}
	     bytes_read = read(fd, buf, 100);
	     for (i = 0; i < bytes_read; i++) {
	       updatedString[j] = buf[i];
	       j = j + 1;
	       if (buf[i] == '\n') {
		 updatedString[j -1] = '\0';
		 j = 0;
	       }
	     }
	     //printf("TEMP: %s\n",latestTemp);
		strcpy(latestTemp, updatedString);
	     //pthread_mutex_unlock(&input_lock);
	
	if (strcmp(state, zero) == 0) { //turn light off
		
		bytes_written = write(fd, lightMsg, strlen(lightMsg) + 1);//sends a string to arduino with value of b
	}
	else if (strcmp(state, one) == 0) { //change from farenheit to celsius and vice versa
		bytes_written = write(fd, state, strlen(state));//sends a string to arduino with value of 1
	}
	else if (strcmp(state, two) == 0) { //turn on
		printf("turn on\n");
		//light is off. want to turn on (a)
		
		bytes_written = write(fd, lightMsg2, strlen(lightMsg2) + 1);//sends a string to arduino with value of a
	}
	else if (strcmp(state, three) == 0) { //get new reading
		bytes_written = write(fd, state, strlen(state));//sends a string to arduino with value of 3
	}
	else { //error
		//keep looping here
	}


	/*char * lightMsg = "a";
		bytes_written = write(fd, lightMsg, strlen(lightMsg) + 1);//sends a string to arduino with value of b*/
     
  }
  close(fd);
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

