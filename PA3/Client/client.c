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
		int serv_no;
		int total_files;
		char filename[100][30];
		char file_slice[100][2];
}list_t;

typedef struct{
		char packet_number[2];
		long int packet_sizes[2];
}get_t;
struct configure{
	 char root[50];
	 char server[4][100];
	 char port[4][10];
	 char username[20];
	 char password[100];
	
}conf = {0};

typedef enum{dfs1=0,dfs2,dfs3,dfs4}dfs_t;

	char files[100][30] = {0};
	int part_flag[100][5] = {0};	
	int file_counter = 0;

int tcp_socket[4] = {0};

void lister(packet_t packet);
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
	
  int recvlen,md5 = 3,option = 1;
  FILE *fptr;
  char inp[50],buf[BUFSIZE],rcv[BUFSIZE],err[50];
  char userinput[30] = {0}, ip[100]={0};
  char command[10] = {0}, filename[20] = {0};
  uint64_t ip_addr, port;
   int packet_combo[4][4][2] = {{{3,0},{0,1},{1,2},{2,3}},{{0,1},{1,2},{2,3},{3,0}},{{1,2},{2,3},{3,0},{0,1}},{{2,3},{3,0},{0,1},{1,2}}};     
 
  packet_t packet;


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
	  int password_accepted = 0;
	  recv(tcp_socket[i],&password_accepted,sizeof(int), 0); 
	  if(password_accepted)
	  {
		printf("\nLogin Successful\n");
	  }
	  else
          {
         	printf("Falied to Login, Password Not mactched \n");
		exit(-1);
	  } 
   

	 
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
		     printf("command filename %s %s\n",packet.command,packet.filename);
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
		         	
	
			    	int alive_server = send(tcp_socket[packet_combo[md5][packet.file_slice][serv]],&packet, sizeof(packet), MSG_NOSIGNAL);
				struct timeval timeout = {0,5000};
				setsockopt(tcp_socket[packet_combo[md5][packet.file_slice][serv]], SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,sizeof(struct timeval));
				//recv(tcp_socket[packet_combo[md5][packet.file_slice][serv]],&alive_server ,sizeof(int), 0);	 

				//printf("Server%d sendto reply %d\n",packet_combo[md5][packet.file_slice][serv],alive_server);   
				   for(long int j =0; j<packet.data_length ; j++)
					{	
						buf[j] ^= key;
					}
				    
	 			    // Send the encrypted data
				    send(tcp_socket[packet_combo[md5][packet.file_slice][serv]], buf, packet.data_length , MSG_NOSIGNAL);

				recv(tcp_socket[packet_combo[md5][packet.file_slice][serv]],&alive_server ,sizeof(int), 0);	 
				timeout.tv_usec = 0;    
				setsockopt(tcp_socket[packet_combo[md5][packet.file_slice][serv]], SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,sizeof(struct timeval));

   
	
				
			   }		
				free(buf);


		  }	
			
			printf("File Sent \n"); 
			
		      
       		     	      
		}

          }

   	 else if(((strcmp(packet.command,"list")==0) )) 
	 {	
		lister(packet);
		
		printf("\n\nPrinting the List of files on all the servers... \n\n");
		for(int i=0;i<file_counter;i++)
		{
			//printf("%d %s %d %d %d %d \n",i+1,files[i],part_flag[i][1],part_flag[i][2],part_flag[i][3],part_flag[i][4]);
			if(part_flag[i][0] == 4)
			{
				printf("%d) %s\n",i+1,files[i]);
			}
			else
			{
				printf("%d) %s Incomplete\n ",i+1,files[i]);
			}
		}

	 }

	
	  else if(strcmp(packet.command,"get") == 0 && (*(packet.filename) != '\0'))
	  {
		packet_t p = {0};
		get_t getp={0};
		strcpy(p.command,"list");
		lister(p);
		int rcv_part_flag[5] = {0};
		for(int i =0;i<file_counter;i++)
		{
			if(strcmp(files[i],packet.filename)==0)
			{
				if(part_flag[i][0] == 4)
				{
					printf("File Exists\n");
					//TODO Get the File here
					char *filedata[4] = {0};
					// recieve data size and packet number
					for(int j=0;j<1;j++)
					{
						send(tcp_socket[j],&packet, sizeof(packet), MSG_NOSIGNAL);
						recv(tcp_socket[j],&getp, sizeof(getp), MSG_NOSIGNAL);
						printf("%s %d)%ld %d)%ld \n",files[i],getp.packet_number[0],getp.packet_sizes[0],getp.packet_number[1],getp.packet_sizes[1]);
					
						struct timeval timeout = {2,0};
						setsockopt(tcp_socket[j], SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,sizeof(struct timeval));

					     for(int x = 0;x<2;x++)
					     {

						if(rcv_part_flag[getp.packet_number[x]] == 0){
							filedata[getp.packet_number[x]] = (char*)malloc(getp.packet_sizes[x]);}

						long int total_bytes = 0;			
						while(total_bytes != getp.packet_sizes[x])
						{

				
				
							long int rcv = recv(tcp_socket[j],filedata[getp.packet_number[x]], getp.packet_sizes[x], 0);
		
							total_bytes += rcv; 
			
						    	if(rcv>0){		
							// Decrypt the data
							//for(long int j=0; j<rcv; j++)	
							//{
							//	recv_buf[j] ^= key; 
							//}
							// Write the data to the file
						    	
							}
							else{perror("Not recieved");break;}
						}
							rcv_part_flag[getp.packet_number[x]] = 1;
							if(total_bytes>0)
								printf("Data Recieved %ld \n",total_bytes);
							


				
						

					     }
						timeout.tv_sec = 0;
						setsockopt(tcp_socket[j], SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,sizeof(struct timeval));
						
						if(rcv_part_flag[1]+rcv_part_flag[2]+rcv_part_flag[3]+rcv_part_flag[4] == 4){printf("File Recieved\n");break;}
					}


					// malloc that much memory
					// recieve the data
					//once entire file is recieved
					// write file 
				}
				else
				{
					printf(" File Exist But Incomplete :  Servers Down\n");
				}
				break;
			}
			else if(i == file_counter-1)
			{
				printf("Incorrect Filename, File Doesnt Exist\n");
			}
		}
 		
          }
	  else if(strcmp(packet.command,"exit")==0)
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





void lister(packet_t packet)
{
			list_t list[4] = {0};
		printf("Lister Called\n");
		for(int i=0;i<4;i++)
		{
			
		char parts[20][4]={0};
		send(tcp_socket[i],&packet, sizeof(packet), MSG_NOSIGNAL);

		//struct timeval timeout = {0,5000};
		//setsockopt(tcp_socket[0], SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,sizeof(struct timeval));

		

		
		long int rec;
		rec = recv(tcp_socket[i],&list[i], sizeof(list), 0);
		//printf("Files %d Recived list of %ld\n",list[i].total_files,rec);
		

		}
		
		bzero(files,300);
		bzero(part_flag,sizeof(int)*500);
		file_counter = 0;
		for(int i=0;i<4;i++)
		{
			for(int j=0;j<list[i].total_files;j++)
			{
				for(int k=0;k<file_counter+1;k++)
				{
					if(strcmp(list[i].filename[j],files[k]) == 0)
					{
						part_flag[k][list[i].file_slice[j][0]-48] = 1;	
						part_flag[k][list[i].file_slice[j][1]-48] = 1;
						break;
						
					}
					else if(k == file_counter)
					{
						strcpy(files[file_counter],list[i].filename[j]);
						//printf("%d %d \n",list[i].file_slice[j][0]-48,list[i].file_slice[j][1]-48);
						part_flag[file_counter][list[i].file_slice[j][0]-48] = 1;	
						part_flag[file_counter][list[i].file_slice[j][1]-48] = 1;
						file_counter++;
				
						break;
					}
	
				}

			}
					
			
		}


	   for(int i=0;i<file_counter;i++)
	   {
			part_flag[i][0] = 0;
			part_flag[i][0] = (part_flag[i][1]+part_flag[i][2]+part_flag[i][3]+part_flag[i][4]);
           }
	
			 
}
