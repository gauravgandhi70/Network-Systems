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
#include <openssl/md5.h>

#define BUFSIZE 1024
// Structure for packet
typedef struct{
		char command[20];
		char filename[30];
		char subfolder[30];
		char username_password[100];
		int file_slice;
		long int data_length;

}packet_t;


typedef struct{
		int serv_no;
		int total_files;
		int total_folders;
		char filename[100][30];
		char file_slice[100][2];
		char subfolder[100][30];
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
	char subfolder[100][30] = {0};
	int part_flag[100][5] = {0};	
	int file_counter = 0;
	int folder_counter = 0;

int tcp_socket[4] = {0};

unsigned char file_md5_counter(char *filename,size_t filesize, FILE *f);

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
	
  int recvlen,option = 1;
  unsigned char md5 = 3;	
  FILE *fptr;
  char inp[50],buf[BUFSIZE],rcv[BUFSIZE],err[50];
  char userinput[30] = {0}, ip[100]={0};
  char command[10] = {0}, filename[20] = {0};
  uint64_t ip_addr, port;
   int packet_combo[4][4][2] = {{{3,0},{0,1},{1,2},{2,3}},{{0,1},{1,2},{2,3},{3,0}},{{1,2},{2,3},{3,0},{0,1}},{{2,3},{3,0},{0,1},{1,2}}};     
 char subf[30]= {0},temp[30]={0};
  packet_t packet;
  int password_accepted = 0;

	struct timeval timeout = {1,0};

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
  	
/*
	  password_accepted = 0;
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
*/

	        
  while(1)
  {
	// Command Menu
    printf("\n Enter any of the Following Commands  \n 1. GET [file_name] \n 2. PUT [file_name] \n 3. LIST \n 4. MKDIR [foldername_name]\n");
    
    bzero(inp,50);
    bzero(subf,30);
    bzero(&packet,sizeof(packet));
  	// Recieve the command

    scanf (" %[^\n]%*c", inp);

				  for(int i=0;i<4;i++)
				  {
					  if((tcp_socket[i]=socket(AF_INET, SOCK_STREAM, 0))<0)
					  {
						perror("Socket Failed");
					  }

					 if (connect(tcp_socket[i], (struct sockaddr *) &dfs[i], sizeof(dfs[i]))<0) 
					{
					  perror("Problem in connecting to the server");
					}
					else
					{
					   char username[100] = {0};
					   setsockopt(tcp_socket[i], SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

				       
					   setsockopt(tcp_socket[i], SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,sizeof(struct timeval));

					  sprintf(username,"%s %s",conf.username,conf.password);
					  strcpy(packet.username_password,username);
					   sendto(tcp_socket[i],username, 100, 0, (struct sockaddr *)&dfs[i],sizeof(dfs[i]));
					  password_accepted = 0;
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
    
    sscanf(inp,"%s %s %s",packet.command,packet.filename,subf);
    if(strcmp(packet.command,"LIST")==0  || strcmp(packet.command,"MKDIR")==0)
    {
	if((packet.filename[strlen(packet.filename)-1]) != '/'  && strlen(packet.filename))  
	{
		packet.filename[strlen(packet.filename)] = '/';
	}
	sprintf(packet.subfolder,"%s",packet.filename);
    }  
    else
    {
	if((subf[strlen(subf)-1]) != '/' && strlen(subf))
	{
		subf[strlen(subf)] = '/';
	}

	    sprintf(packet.subfolder,"%s",subf);  
    }

	printf("Subfolder path %s\n",packet.subfolder);
	


	// Put the file into the server	
   	 if(((strcmp(packet.command,"PUT")==0) ) && (*(packet.filename) != '\0')) // Check if the filename is empty or not
 	  {
	     printf("command filename %s %s\n",packet.command,packet.filename);
		
		fptr = fopen(packet.filename,"rb");		// open the file to read
   	        
		
		if((fptr == NULL)) // Check if the filename was right or wrong
		{
			printf("Wrong File name \n");
		}
		else
		{
			fseek(fptr,0,SEEK_END);
  			size_t file_size=ftell(fptr);
  			fseek(fptr,0,SEEK_SET);
			
			md5 = file_md5_counter(packet.filename,file_size,fptr);	
				
			printf("File Size - %ld \n\n\n",file_size);
			
			float f = (file_size);
			f = f/4 ;
 			int fileSize_OneServer = round(f);
			//printf("float %f roundoff %d\n",f,fileSize_OneServer);

		     for(packet.file_slice = 0;packet.file_slice<4;packet.file_slice++)
	               {
			  	
	 	
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
				
			fread(buf,1,packet.data_length,fptr);
			
			printf("File Slice - %d Size -%ld\n",packet.file_slice,packet.data_length);

			    for(int serv=0;serv<2;serv++)
				
			    {
		         	
			    	int alive_server = send(tcp_socket[packet_combo[md5][packet.file_slice][serv]],&packet, sizeof(packet), MSG_NOSIGNAL);
	
							
				  password_accepted = 0;
				  recv(tcp_socket[packet_combo[md5][packet.file_slice][serv]],&password_accepted,sizeof(int), 0); 
				  if(password_accepted)
				  {
					printf("\nServer On\n");
				  }
				  else
				  {
				 	printf("Server Down \n");
					//continue;
					//exit(-1);
				  } 
			
		
				//recv(tcp_socket[packet_combo[md5][packet.file_slice][serv]],&alive_server ,sizeof(int), 0);	 

				//printf("Server%d sendto reply %d\n",packet_combo[md5][packet.file_slice][serv],alive_server); 
				if(serv == 0){  
				   for(long int j =0; j<packet.data_length ; j++)
					{	
						buf[j] ^= key;
					}
				    }
	 			    // Send the encrypted data
				    send(tcp_socket[packet_combo[md5][packet.file_slice][serv]], buf, packet.data_length , MSG_NOSIGNAL);
				timeout.tv_sec = 0;
				timeout.tv_usec = 10000;    
				setsockopt(tcp_socket[packet_combo[md5][packet.file_slice][serv]], SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,sizeof(struct timeval));

				recv(tcp_socket[packet_combo[md5][packet.file_slice][serv]],&alive_server ,sizeof(int), 0);	 
				timeout.tv_sec = 1;
				timeout.tv_usec = 0;    
				setsockopt(tcp_socket[packet_combo[md5][packet.file_slice][serv]], SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,sizeof(struct timeval));



   
	
				
			   }	
				/*char lfname[30] = {0};
				sprintf(lfname,"%s.%d",packet.filename,packet.file_slice+1);
				FILE *lfp = fopen(lfname,"w"),*rf;
				fwrite(buf,1,packet.data_length,lfp);
				fclose(lfp);*/
				
				/*lfp = fopen(lfname,"r");
				if(packet.file_slice==0){rf=fopen("temp","w");}
				char *temp = (char*)malloc(packet.data_length);
				fread(temp,1,packet.data_length,lfp);
				fwrite(temp,1,packet.data_length,rf);
				free(temp);
				if(packet.file_slice==3){fclose(rf);printf("Closed Temp");}*/


				free(buf);


		  }	

			
			printf("File Sent \n"); 
			
		      
       		     	      
		}

          }

   	 else if(((strcmp(packet.command,"LIST")==0) )) 
	 {	
		lister(packet);
		
		printf("\n\nPrinting the List of files on all the servers... \n\n");
		for(int i=0;i<file_counter;i++)
		{
			if(part_flag[i][0] == 4)
			{
				printf("%d) %s\n",i+1,files[i]);
			}
			else
			{
				printf("%d) %s Incomplete\n",i+1,files[i]);
			}
		}
		printf("\n\nPrinting the List of Sub-Folders on all the servers... \n\n");
		for(int i=0;i<folder_counter;i++)
		{
			printf("%d) %s\n",i+1,subfolder[i]);
		}

		

	 }

	
	  else if(strcmp(packet.command,"GET") == 0 && (*(packet.filename) != '\0'))
	  {
		packet_t p = {0};
		get_t getp={0};
		strcpy(p.command,"LIST");
		strcpy(p.subfolder,packet.subfolder);
		lister(p);
		int rcv_part_flag[5] = {0};
		long int rcv_size[4] = {0};

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
					for(int j=0;j<4;j++)
					{
						send(tcp_socket[j],&packet, sizeof(packet), MSG_NOSIGNAL);
	
					  password_accepted = 0;
					  recv(tcp_socket[j],&password_accepted,sizeof(int), 0); 

					  if(password_accepted)
					  {
						printf("\nServer On\n");
					  }
					  else
					  {
					 	printf("Server Down \n");
						//continue;
						//exit(-1);
					  } 

				

						recv(tcp_socket[j],&getp, sizeof(getp), MSG_NOSIGNAL);
						//printf("%s %d)%ld %d)%ld \n",files[i],getp.packet_number[0],getp.packet_sizes[0],getp.packet_number[1],getp.packet_sizes[1]);
						
						struct timeval timeout = {1,0};
						setsockopt(tcp_socket[j], SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,sizeof(struct timeval));

					     for(int x = 0;x<2;x++)
					     {

						if(rcv_part_flag[getp.packet_number[x]] == 0){
							filedata[getp.packet_number[x]-1] = (char*)malloc(getp.packet_sizes[x]);}

						long int total_bytes = 0;			
						while(total_bytes != getp.packet_sizes[x])
						{

				
				
							long int rcv = recv(tcp_socket[j],&filedata[getp.packet_number[x]-1][total_bytes], getp.packet_sizes[x], 0);
		
							
			
						    	if(rcv>0){		
							// Decrypt the data
							for(long int z =0; z<rcv ; z++)
							{	
									filedata[getp.packet_number[x]-1][z+total_bytes] ^= key;
							}
							//printf("rcv - %ld total bytes - %ld\n",rcv,total_bytes);
							total_bytes += rcv; 
							// Write the data to the file
						    	
							}
							else{perror("Not recieved");break;}
						}
							if(total_bytes>0){
								printf("Packet %d with Data Recieved %ld from server number - %d \n",getp.packet_number[x],total_bytes,j+1);
								rcv_part_flag[getp.packet_number[x]] = 1;
								rcv_size[getp.packet_number[x]-1] = total_bytes;

							}


				
						

					     }
						
						if(rcv_part_flag[1]+rcv_part_flag[2]+rcv_part_flag[3]+rcv_part_flag[4] == 4)
						{	
							printf("File Recieved\n");
							FILE *fpw = fopen(packet.filename,"w");
						
							for(int part=0;part<4;part++)
							{
								fwrite(filedata[part], 1,rcv_size[part],fpw);
								free(filedata[part]);								
							}
							fclose(fpw);
							break;
						}
					}


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
	  else if((strcmp(packet.command,"MKDIR")==0) && (*(packet.subfolder) != '\0'))
	  {
		
		
		for(int i =0;i<4;i++)
		{
			send(tcp_socket[i],&packet, sizeof(packet), MSG_NOSIGNAL);

				
					  password_accepted = 0;
					  recv(tcp_socket[i],&password_accepted,sizeof(int), 0); 
					  if(password_accepted)
					  {
						printf("\nServer On\n");
					  }
					  else
					  {
					 	printf("Server Down \n");
						//continue;
						//exit(-1);
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

	close(tcp_socket[0]);close(tcp_socket[1]);close(tcp_socket[2]);close(tcp_socket[3]);
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

				
					  int  password_accepted = 0;
					  recv(tcp_socket[i],&password_accepted,sizeof(int), 0); 
					  if(password_accepted)
					  {
						printf("\nServer On\n");
					  }
					  else
					  {
					 	printf("Server Down \n");
						//return;
					  } 

				

		

		
		long int rec;

		rec = recv(tcp_socket[i],&list[i], sizeof(list), 0);
		//printf("Files %d Recived list of %ld\n",list[i].total_files,rec);
		

		}
		
		bzero(files,300);
		bzero(subfolder,300);

		bzero(part_flag,sizeof(int)*500);

		file_counter = 0;
		folder_counter = 0;
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
						part_flag[file_counter][list[i].file_slice[j][0]-48] = 1;	
						part_flag[file_counter][list[i].file_slice[j][1]-48] = 1;
						file_counter++;
				
						break;
					}
	
				}

			}

			for(int j=0;j<list[i].total_folders;j++)
			{
				for(int k=0;k<folder_counter+1;k++)
				{
					if(strcmp(list[i].subfolder[j],subfolder[k]) == 0)
					{
						break;
					}
					else if(k == folder_counter)
					{
						strcpy(subfolder[folder_counter],list[i].subfolder[j]);
						folder_counter++;
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



unsigned char file_md5_counter(char *filename,size_t filesize, FILE *f)
{

  char *buf = NULL;
  unsigned char md5s[MD5_DIGEST_LENGTH] ={0}; //(char*)malloc(MD5_DIGEST_LENGTH); 

 
  buf = (char*)malloc(filesize);

  fread(buf,1, filesize, f);
  fseek(f,0,SEEK_SET);

  MD5(buf, filesize, md5s);
  printf("MD5 (%s) = ", filename);
  for (int i=0; i < MD5_DIGEST_LENGTH; i++)
  {
    printf("%x",  md5s[i]);

  }  
  printf("\nRemainder - %d  ",md5s[15]%4);
  free(buf);
  return (md5s[15]%4);

}
