#include<sys/socket.h>
#include<sys/types.h>
#include<stdio.h>
#include<error.h>
#include<string.h>
#include <arpa/inet.h>
#include<stdlib.h>
#include  <fcntl.h>
#define BUFSIZE 1024

typedef struct{
		long int ID;
		char p[BUFSIZE];
		long int length;

}packet_t;


void main(int argc, char *argv[])
{

  int udp_socket, putf = 0, getf = 0, deletef = 0, option =1,port;
  long int ACK = 0;
  char filename[20], command[10];
  FILE *fptr;
  struct sockaddr_in udp_socketaddr, remaddr;
    socklen_t addrlen = sizeof(remaddr); 	
   int recvlen; /* # bytes received */
   unsigned char buf[BUFSIZE]; /* receive buffer */
   packet_t packet;
   char key = 10;
  struct timeval timeout = {0,500000};
  if(argc != 2)
  {
     printf(" Port Number Entered Incorrecly \n");
     return ;
  }
  else
  {
	port = atoi(argv[1]);
  }
  if((udp_socket=socket(AF_INET, SOCK_DGRAM, 0))<0)
  {
        perror("Socket Failed");
  }

  
  setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  
 
  

  udp_socketaddr.sin_family = AF_INET;
  udp_socketaddr.sin_addr.s_addr = INADDR_ANY;//inet_addr("192.168.3.131");
  udp_socketaddr.sin_port = htons(port);
  
  if((bind(udp_socket,(struct sockaddr*)&udp_socketaddr, sizeof(udp_socketaddr))<0))
  {
        perror("Binding Failed");
  }

  while(1)
  {
	  printf("\n waiting on port %d\n",port); 
	  
	  recvlen = recvfrom(udp_socket, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
          //printf("received %d bytes\n", recvlen); 
          if (recvlen > 0)
          { 
            buf[recvlen] = 0;
	    printf("received message: \"%s\"\n", buf);  
          }
		
	  memset(command,0,10);
          memset(filename,0,20);
	  sscanf(buf,"%s %s",command,filename);

 	  
          if(((strcmp(command,"put")==0) ))
 	  {
		
   	    if(*filename != '\0')
	    {
		long int num_packets = 0;
		// Recieve the number of packets of the file
		recvfrom(udp_socket, &num_packets, sizeof(num_packets), 0, (struct sockaddr *)&remaddr, &addrlen);
		//Server  ACK
		sendto(udp_socket, &num_packets, sizeof(num_packets), 0, (struct sockaddr *)&remaddr, addrlen);
		      
		printf("Packets - %ld \n",num_packets);	
		if(num_packets == 0)		// if 0 packets are recieved that means file can not be created
		{
			printf("Empty File - Not Created \n");
		}
		else if(num_packets > 0)
		{
			int total_bytes = 0;
			// Open the new file to write			
			fptr = fopen(filename,"w");
			// Write the number of packets to file
			for(long int i=0; i<num_packets;i++)
			{
			    // clear the recieving buffer before writing
			    memset(&packet,0,sizeof(packet));    
			   // recieve the packet
			    recvlen = recvfrom(udp_socket, &packet, sizeof(packet), 0, (struct sockaddr *)&remaddr, &addrlen);
			    
			    // send acknowledgement
			    sendto(udp_socket, &(packet.ID), sizeof(packet.ID), 0, (struct sockaddr *)&remaddr, addrlen);

			    if(packet.ID < i)	 // If repeated paxket is recived then discard it and reduce the for loop count of recieved packet
			    {
				i--;
				printf("Same Packet %ld Discarded \n",packet.ID);
			    }	
			    else
			    {
				// Decrypt the data
				for(long int j=0; j<packet.length; j++)
				{
					packet.p[j] ^= key; 
				}
				// Write the data to the file
			    	fwrite(packet.p, 1,packet.length,fptr);
				printf("Packet number = %ld Recieved Size- %ld  \n",packet.ID,packet.length);
				total_bytes += packet.length;
			    }
			    	
			    			
			}
			printf("Total bytes - %d\n",total_bytes);
			fclose(fptr);	
		}	
		
	    }	
	    else
	    {
		printf("Filename Not given \n");
				
	    }
		
          }

		// server
	  else if((strcmp(command,"get")==0))
	  {
		
		if(*filename != '\0')  // Check if the filename is empty or not
	     {
		long int num_packets = 0, dropped = 0;
		fptr = fopen(filename,"rb");	// open the file to read
   	        
		setsockopt(udp_socket,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));	// Set the timeout for Acknowlegment

		if((fptr == NULL))		// Check if the filename was right or wrong
		{
			printf("Wrong File name \n");
			// Send filesize as zero so that server will know that filename was wrong
			sendto(udp_socket, &num_packets, sizeof(num_packets), 0, (struct sockaddr *)&remaddr,sizeof(remaddr));
			recvfrom(udp_socket, &ACK, sizeof(ACK), 0, (struct sockaddr *)&remaddr, &addrlen); // Recieve ACK for number of packet
			
		}
		else
		{
			int counter = 0, timeout_flag = 0;
			fseek(fptr,0,SEEK_END);
  			size_t file_size=ftell(fptr);
  			fseek(fptr,0,SEEK_SET);
			memset(&packet,0,sizeof(packet));

			printf("File Size - %ld \n",file_size);
			num_packets = (file_size / BUFSIZE )+ 1;	// Number of packets to send
 
			// Send the number of packets to the reciever
			sendto(udp_socket,&(num_packets), sizeof(num_packets), 0, (struct sockaddr *)&remaddr,sizeof(remaddr));
			// wait for ack and if timeout then resend
			while(recvfrom(udp_socket, &ACK, sizeof(ACK), 0, (struct sockaddr *)&remaddr, &addrlen) < 0)
			{
				
				sendto(udp_socket,&(num_packets), sizeof(num_packets), 0, (struct sockaddr *)&remaddr,sizeof(remaddr));
				counter++;
				// Even after 100 tries if the reciever fails then give connection timeout
				if(counter == 100)
				{		
					timeout_flag = 1;
					printf("File Not Sent - Connection Timeout\n");
						break; 		
				}			
			}

			printf("packets - %ld \n",num_packets);
			// Send the data
			for(long int i=0; i<num_packets;i++)
			{
			    memset(&packet,0,sizeof(packet));    
			    packet.length = fread(packet.p,1,BUFSIZE, fptr), counter = 0;
			    ACK = 0;
			    packet.ID = i ;
			    // Encrypt the data by Xoring 
			    for(long int j=0; j<packet.length; j++)
				{
					packet.p[j] ^= key; 
				}
			    // Send the encrypted data
			    sendto(udp_socket, &packet, sizeof(packet) , 0, (struct sockaddr *)&remaddr,sizeof(remaddr));	
			    			
				
			     if(timeout_flag)
			 	{
					printf("File Not Sent - Connection Timeout\n"); 
					break;
				}
				// recieve ACK from the reciever
			       recvfrom(udp_socket, &ACK, sizeof(ACK), 0, (struct sockaddr *)&remaddr, &addrlen) ;
			    while((ACK != packet.ID))
			    {	
				// Keep re sending packets until ack for current packet is recieved
				sendto(udp_socket, &packet, sizeof(packet), 0, (struct sockaddr *)&remaddr,sizeof(remaddr));
				recvfrom(udp_socket, &ACK, sizeof(ACK), 0, (struct sockaddr *)&remaddr, &addrlen) ;
				printf("\t\tPacket no %ld	 dropped, Total - %ld \n",packet.ID,++dropped);  
				counter++;
				if(counter == 100)
				{
					timeout_flag = 1;
					break;
				}
  			    }
			
				   
			   printf("ACK for Packet, i = %ld ACK = %ld recieved \n",i,ACK);
			
				 if(i == num_packets - 1)
					printf("File Sent \n"); 

			}

			
			
		      
       		        	      
		}
		// Remove the timeout for the recvfrom
		struct timeval timeout = {0,0};
		setsockopt(udp_socket,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
 }
	    else
		{
			printf("Filename Not given \n");
		//sendto(udp_socket, "Filename Not Given", 18, 0, (struct sockaddr *)&remaddr, addrlen);		
			
		}
		
	  }

	  // server - delete command
	  else if((strcmp(command,"delete")==0))
	   {
		if(*filename == '\0')
		{
			printf("Filename Not given \n");
			//sendto(udp_socket, "Filename Not Given", 18, 0, (struct sockaddr *)&remaddr, addrlen);
		}
		else
		{
			char cmd[10] = "rm ";
			strcat(cmd,filename);
			int ret = system(cmd);
			if(ret)
			{
				printf("Error in Deleting\n");
				sendto(udp_socket, "Error in Deleting", 17, 0, (struct sockaddr *)&remaddr, addrlen);
			}
			else
			{
				printf("Successfully deleted %s\n",filename);
				sendto(udp_socket, "Successful", 18, 0, (struct sockaddr *)&remaddr, addrlen);
			}
					
		}

	   }
	  
 	  // get the list of files on the server side and print it
	  else if(strcmp(command,"ls")==0)
	  {
		FILE *list;
		char filelist[200] = {0};
		system("ls >> a.log");
		list = fopen("a.log","r");
		int rec = fread(filelist,1,200, list); 
		sendto(udp_socket, filelist, rec, 0, (struct sockaddr *)&remaddr, addrlen);
		 
		system("rm a.log");
          }	
	
	  // Get the md5sum for the file on the server side
	  else if(strcmp(command,"md")==0)
	  {
		if(*filename == '\0')
		{
			printf("Filename Not given \n");
			
		}
		else
		{
			char cmd[50] = "md5sum ";
			strcat(cmd,filename);
			printf("\n md5sum of %s on Server Side \n\n",filename);
			system(cmd);
       	  	}
	  }
	  else if((strcmp(command,"exit")==0))
          {
		//sendto(udp_socket, buf, strlen(buf), 0, (struct sockaddr *)&remaddr, addrlen);
		printf("Closing the socket\n");
		close(udp_socket);
		return;	
	  }	  
	
	// Error in the command or filename	  
	  else if(recvlen > 0)
	  {
				
		printf("Command Not Found \n");
     		//sendto(udp_socket, buf, strlen(buf), 0, (struct sockaddr *)&remaddr, addrlen);		
	  }
	  
  } 

}




