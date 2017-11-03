#include<sys/socket.h>
#include<sys/types.h>
#include<stdio.h>
#include<error.h>
#include<string.h>
#include <arpa/inet.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>
#include <netdb.h>
#include <math.h>
#define BUFSIZE 1024
// Structure for packet
typedef struct{
		char command[20];
		char filename[30];
		int file_slice;
		long int data_length;

}packet_t;

typedef struct{
		int total_files;
		char filename[30];
		int file_slice[2];
}list_t;
struct configure{
	 char root[50];
	 char server[4][100];
	 char port[4][10];
	 char username[20];
	 char password[100];
	
}conf = {0};

typedef enum{dfs1=0,dfs2,dfs3,dfs4}dfs_t;

void parse_config(char *filename)
{
	FILE *f;
	f = fopen(filename,"rb");
	if(f)
	{
		char buffer[1000],cmp[20],*c;
	        fread(buffer,1,1000, f);
		for(int i=0;i<4;i++)
		{
			bzero(cmp,sizeof(cmp));
			sprintf(cmp,"%s%d","DFS",i+1);
			c = strstr(buffer,cmp);
			sscanf(c,"%*s %s %s",conf.server[i],conf.port[i]);
		}
		bzero(cmp,sizeof(cmp));
		c = strstr(buffer,"Username_Pass");
		sscanf(c,"%*s %s %s",conf.username,conf.password);
			
	}
	else
	{
		printf("Config Filename Wrong\n");
		exit(0);
	}
}


void main(int argc, char *argv[])
{
	
  int tcp_socket[5], recvlen,md5 = 3,option = 1;
  FILE *fptr;
  char inp[50],buf[BUFSIZE],rcv[BUFSIZE],err[50];
  char userinput[30] = {0}, ip[100]={0};
  char command[10] = {0}, filename[20] = {0};
  uint64_t ip_addr, port;
   int packet_combo[4][4][2] = {{{3,0},{0,1},{1,2},{2,3}},{{0,1},{1,2},{2,3},{3,0}},{{1,2},{2,3},{3,0},{0,1}},{{2,3},{3,0},{0,1},{1,2}}};     
 
  packet_t packet;
  list_t
  char key = 10;	
  memset(&packet,0,sizeof(packet));
  long int ACK = 0;
  
  if(argc != 2)
  {
	printf("Configure file not given properly");
	exit(0);
  }
  else
  {
	parse_config(argv[1]);
  }

  // Open a TCP socket
     for(int i = 0;i<4;i++)
     {
	  if((tcp_socket[i]=socket(AF_INET, SOCK_STREAM, 0))<0)
	  {
		perror("Socket Failed");
	  }
	  //else{printf("Socket Created %d\n",tcp_socket[i]);}
     }	
   



   // Connect to the server address using parameters in sockaddr_in structure 
 
  struct sockaddr_in dfs[4];
  for(int i = 0 ; i<4;i++)
  {
   memset( &dfs[i], 0, sizeof(dfs[i]) );
   dfs[i].sin_family = AF_INET;
   //printf("Port %d\n",atoi(conf.port[i]));
   dfs[i].sin_port = htons(atoi(conf.port[i]));              
   dfs[i].sin_addr.s_addr =  inet_addr(conf.server[i]);
  }

  // Connect our socket to the server
  for(int i=0;i<4;i++)
  {
	 if (connect(tcp_socket[i], (struct sockaddr *) &dfs[i], sizeof(dfs[i]))<0) 
	{
	  perror("Problem in connecting to the server");
	}
	else
	{
	   char username[100] = {0};
	   setsockopt(tcp_socket[i], SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	  sprintf(username,"%s %s",conf.username,conf.password);
	  
	  sendto(tcp_socket[i],username, 100, 0, (struct sockaddr *)&dfs[i],sizeof(dfs[i]));
	 
	 
	}
  }    
  	


	        
  while(1)
  {
	// Command Menu
    printf("\n Enter any of the Following Commands  \n 1. GET [file_name] \n 2. PUT [file_name] \n 3. LIST \n");
    
    bzero(inp,50);
    bzero(&packet,sizeof(packet));
  	// Recieve the command

    scanf (" %[^\n]%*c", inp);

    sscanf(inp,"%s %s",packet.command,packet.filename);

	// Put the file into the server	
   	 if(((strcmp(packet.command,"put")==0) ) && (*(packet.filename) != '\0')) // Check if the filename is empty or not
 	  {
		
		fptr = fopen(packet.filename,"rb");		// open the file to read
   	        
		
		if((fptr == NULL)) // Check if the filename was right or wrong
		{
			printf("Wrong File name \n");
		}
		else
		{
		     for(packet.file_slice = 0;packet.file_slice<4;packet.file_slice++)
	               {
			  
			fseek(fptr,0,SEEK_END);
  			size_t file_size=ftell(fptr);
  			fseek(fptr,0,SEEK_SET);
					
			printf("File Size - %ld \n",file_size);
			
			float f = (file_size);
			f = f/4 ;
 			int fileSize_OneServer = round(f);
			printf("float %f roundoff %d\n",f,fileSize_OneServer);
			
			
	 	
			char *buf = NULL;

			if(packet.file_slice == 3)
			{
				packet.data_length = (file_size - fileSize_OneServer*3);
				buf = (char*)malloc(packet.data_length);
			}
			else
			{
				packet.data_length = (fileSize_OneServer);
				buf = (char*)malloc(packet.data_length);
			}
				
			printf("File Slice - %d Size -%ld \n",packet.file_slice,packet.data_length);

			    for(int serv=0;serv<2;serv++)
				
			    {
		         	printf("command filename %s %s\n",packet.command,packet.filename);
	
			    	send(tcp_socket[packet_combo[md5][packet.file_slice][serv]],&packet, sizeof(packet), MSG_NOSIGNAL);
				   
				   for(long int j =0; j<packet.data_length ; j++)
					{	
						buf[j] ^= key;
					}
				    
	 			    // Send the encrypted data
				    send(tcp_socket[packet_combo[md5][packet.file_slice][serv]], buf, packet.data_length , MSG_NOSIGNAL);

				struct timeval timeout = {0,5000};int alive_server;
				setsockopt(tcp_socket[packet_combo[md5][packet.file_slice][serv]], SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,sizeof(struct timeval));
				recv(tcp_socket[packet_combo[md5][packet.file_slice][serv]],&alive_server ,sizeof(int), 0);	 
				timeout.tv_usec = 0;    
				setsockopt(tcp_socket[packet_combo[md5][packet.file_slice][serv]], SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,sizeof(struct timeval));
   
	
				
			   }		
				free(buf);


		  }	
			
			printf("File Sent \n"); 
			
		      
       		     	      
		}

          }



	  else if(strcmp(command,"exit")==0)
	  {
		return;
	  }
	// Error in the command or filename 
	  else
	  {
		
		printf("Error (Wrong Command or Wrong filename) \n");	
		  
	  }
  } 


}
