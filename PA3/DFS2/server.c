#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>
#define MAXLINE 1024 /*max text line length*/
#define SERV_PORT 10002 /*port*/
#define LISTENQ 8 /*maximum number of client connections*/

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

list_t list = {0};
int file_counter=-1;
void file_lister(char *path, char *user,int conndf);

int main (int argc, char **argv)
{
 int listenfd, connfd, option =1 ;
 const int serv_num = 1;
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
  int password_accepted =0;
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
	password_accepted = 1;
	send(connfd,&password_accepted,sizeof(int),0);
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
    password_accepted = 0;
	send(connfd,&password_accepted,sizeof(int),0);
     printf("Username Password Not Mached, Closing Socket... \n");
     close(connfd);
     exit(0);
  }
 
  while ((n =recv(connfd,&packet ,sizeof(packet), 0))>0)   
  {
	//send(connfd,&serv_num, sizeof(int), MSG_NOSIGNAL);
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
	else if(((strcmp(packet.command,"list")==0) ))
	{
		printf("Lister called\n");
		file_lister(path,foldername,connfd);
		
	}
	else if(((strcmp(packet.command,"get")==0) ))
	{
		get_t getp = {0};
		char partname[2][30] = {0};
		FILE *f1,*f2;

		for(int i=0;i<file_counter+1;i++)
		{	

			if(strcmp(list.filename[i],packet.filename)==0)
			{
				//printf(" %s Partname %d %d\n",list.filename[i],list.file_slice[i][0],list.file_slice[i][1]);
				getp.packet_number[0] = list.file_slice[i][0] - 48 ; getp.packet_number[1] = list.file_slice[i][1] - 48; 
				// Make the names of file
				sprintf(partname[0],"./%s/.%s.%d",foldername,list.filename[i],getp.packet_number[0]);
				sprintf(partname[1],"./%s/.%s.%d",foldername,list.filename[i],getp.packet_number[1]);

				break;
			}	
		
		}

		f1 = fopen(partname[0],"r");
		f2 = fopen(partname[1],"r");

		if(f1){
		fseek(f1,0,SEEK_END);
		getp.packet_sizes[0]=ftell(f1);
		fseek(f1,0,SEEK_SET);}
	

		if(f2){
		fseek(f2,0,SEEK_END);
		getp.packet_sizes[1]=ftell(f2);
		fseek(f2,0,SEEK_SET);}


		printf("1)%s %ld 2)%s %ld \n",partname[0],getp.packet_sizes[0],partname[1],getp.packet_sizes[1]);
		send(connfd,&getp, sizeof(getp),0);
		
		

		
		
	}
	else
	{
		printf("Wrong Command\n");
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



void file_lister(char *path,char *user,int connfd)
{
	FILE *f_list;
	bzero(&list,sizeof(list));
	char command[100]= {0},filepath[100] = {0},filelist[1024] = {0},*c = filelist;
	char *line;
	sprintf(command,"ls -a %s/ >> %s_filelist.log",path,user);
	system(command);
	sprintf(filepath,"%s_filelist.log",path);
	f_list = fopen(filepath,"r");
	int rec = fread(filelist,1,1024, f_list); 
	//printf("%s\n",filelist);
	int i=0;
	c = c+5;
/*	while(c[i] != EOF && i < 1024)
	{
		if(c[i]=='\n')
		{
			list.total_files++;
		}	
		
		i++;
	}
	float f = list.total_files;
	f = f/2;
	list.total_files = round(f);i=0;
	printf("files - %d\n",list.total_files);*/
	
	//send(connfd,&list.total_files, sizeof(int), MSG_NOSIGNAL);

	
	
	while(*c != EOF && i < 1024)
	{	int j=0,dotcounter = 0;
		char line[30] = {0};		
		while(*c != '\n')
		{
			line[j] = *c;
			if(*c == '.')
			{
				dotcounter++; 
			}
			c++;i++;j++;
		}
	
		if(dotcounter == 2)
		{
			char *lp = line,local_filename[30]= {0};
			lp++;
			int char_counter = 0;
			
			while(*lp != '.')
			{
				local_filename[char_counter] = *lp;
				lp++;char_counter++;
			}

			lp++;
	
			if(strcmp(list.filename[file_counter],local_filename)==0)
			{
				list.file_slice[file_counter][1] = *lp; 
			}
			else
			{
				file_counter++;
				strcpy(list.filename[file_counter],local_filename);
				list.file_slice[file_counter][0] = *lp; 

			}
			
			//printf("filename - %s File - %d Slice - %s\n",list.filename[file_counter],file_counter,list.file_slice[file_counter]);
			lp++;
			
			
			

			
			
		}

		else if(dotcounter == 3)
		{
			char *lp = line,local_filename[30]= {0};
			lp++;
			int char_counter = 0,local_dot_counter = 0;
			while(1)
			{
				if(*lp == '.')
				{
					local_dot_counter++;
					if(local_dot_counter == 2)
					{
						break;
					}
				}	
				local_filename[char_counter] = *lp;
				lp++;char_counter++;
				
			}
			lp++;
			if(strcmp(list.filename[file_counter],local_filename)==0)
			{
				list.file_slice[file_counter][1] = *lp; 
			}
			else
			{
				file_counter++;
				strcpy(list.filename[file_counter],local_filename);
				list.file_slice[file_counter][0] = *lp; 

			}
			
			//printf(" filename - %s File - %d Slice - %s\n",list.filename[file_counter],file_counter,list.file_slice[file_counter]);

			
		}
		
		c++;i++;
		
			
	}
	list.total_files = file_counter + 1;
	int sendsize;
	sendsize = send(connfd,&list, sizeof(list), MSG_NOSIGNAL);
	printf("sendsize %d\n",sendsize);


	bzero(command,100);
	sprintf(command,"rm -f %s_filelist.log",user);
	system(command); 
	


}


