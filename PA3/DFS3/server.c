#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAXLINE 1024 /*max text line length*/
#define SERV_PORT 10003 /*port*/
#define LISTENQ 8 /*maximum number of client connections*/

typedef struct{
		char command[20];
		char filename[30];
		int file_slice;
		long int data_length;

}packet_t;


int main (int argc, char **argv)
{
 int listenfd, connfd, option =1 ;
 pid_t childpid;
  long long int n;
 socklen_t clilen;
 char buf[MAXLINE] = {0};
  char command[10]={0},filename[30]={0},username[100];
 struct sockaddr_in cliaddr, servaddr,remaddr;
 socklen_t addrlen = sizeof(servaddr); 
 char key=10;
  FILE *fptr;
  packet_t packet = {0};
  int recvlen;
 //Create a socket for the soclet
 //If sockfd<0 there was an error in the creation of the socket
 if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
  perror("Problem in creating the socket");
  exit(2);
 }


 //preparation of the socket address
 servaddr.sin_family = AF_INET;
 servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
 servaddr.sin_port = htons(SERV_PORT);

 //bind the socket
 bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

  // Set socket properties to reuse the port
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

 //listen to the socket by creating a connection queue, then wait for clients
 listen (listenfd, LISTENQ);

 printf("%s\n","Server running...waiting for connections.");

 for ( ; ; )
 {

  clilen = sizeof(cliaddr);
  //accept a connection
  connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);

  printf("%s\n","Received request...");

  if ( (childpid = fork ()) == 0 ) {//if it’s 0, it’s child process

  printf ("%s\n","Child created for dealing with client requests");
  char foldername[20] = {0},path[25]={0};

  //close listening socket
  close (listenfd);


  // Recieve Username and Password
  
  recv(connfd, username, 100,0);	
  puts(username);
  char filebuf[1000] = {0};
  FILE *f = fopen("dfs.conf","rb");
  fread(filebuf,1,1000,f);
  char *p = strstr(filebuf,username);
  bzero(username,100);
  if(p)
  {
	printf("Username Password Matched\n");
	// Making New Folder if it doesnt exist
	struct stat st = {0};
	sscanf(p,"%s",foldername);
	sprintf(path,"./%s",foldername);
	if (stat(path, &st) == -1) {
	    mkdir(path, 0700);
	    printf("Folder Created\n");
           }
  }
  else
  {
     printf("Username Password Not Mached, Closing Socket... \n");
     close(connfd);
     exit(0);
  }
 
  while ((n =recv(connfd,&packet ,sizeof(packet), 0))>0)   
  {
	
	//printf("command filename %s %s\n",packet.command,packet.filename);

          if(((strcmp(packet.command,"put")==0) ))
 	  {
			struct timeval timeout = {1,0};
			setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,sizeof(struct timeval));
			// Generate the name for the part number from the file slice number
			char partname[30] = {0};
			sprintf(partname,"%s/.%s.%d",path,packet.filename,packet.file_slice+1);
			printf("partname - %s\n",partname);
			// Open the new file with the part number
			fptr = fopen(partname,"w");
			char *recv_buf = NULL;
			 recv_buf = (char*)malloc(packet.data_length);
			// Recieve the data in the bufer
			long int total_bytes = 0;			
			while(total_bytes != packet.data_length)
			{
				struct timeval timeout = {2,0};
				setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,sizeof(struct timeval));
				 
				long int rcv = recv(connfd, recv_buf, packet.data_length, 0);
				total_bytes += rcv; 
			
			    	if(rcv>0){		
				// Decrypt the data
				for(long int j=0; j<rcv; j++)	
				{
					recv_buf[j] ^= key; 
				}
				// Write the data to the file
			    	fwrite(recv_buf, 1,rcv,fptr);
				}
				else{printf("eiflvlavasv\n");break;}
		
			}
				printf("Data Recieved %ld \n",total_bytes);
				fclose(fptr);	
			timeout.tv_sec = 0;
			setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,sizeof(struct timeval));
		

	}
	else
	{
		printf("Wrong Command\n");
		break;
	}
	
	

	    
		
    }


   	
  if (n < 0)
   printf("%s\n", "Read error");
  close(connfd);
  exit(0);
 }
 //close socket of the server
 
}
}

