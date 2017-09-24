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

#define BUFSIZE 1024
// Structure for packet
typedef struct{
		long int ID;
		char p[BUFSIZE];
		long int length;

}packet_t;

// Function to convert website name to ip address
int hostname_to_ip(char * hostname , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
         
    if ( (he = gethostbyname( hostname ) ) == NULL) 
    {
        // get the host info
        herror("gethostbyname");
        return 1;
    }
 
    addr_list = (struct in_addr **) he->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++) 
    {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }
     
    return 1;
}


void main(int argc, char *argv[])
{
	
  int udp_socket, recvlen;
  FILE *fptr;
  char inp[50],buf[BUFSIZE],rcv[BUFSIZE],err[50];
  char userinput[30] = {0}, ip[100]={0};
  char command[10] = {0}, filename[20] = {0};
  uint64_t ip_addr, port;
   
  packet_t packet;
  char key = 10;	
  memset(&packet,0,sizeof(packet));
  long int ACK = 0;
  struct timeval timeout = {0,50000};
	
 // Check for correct user input
  if(argc != 3)
  {
     printf(" IP and Port Number Entered Incorrecly");
     return ;
  }
  else
  {
	// Convert the user entered ipaddress string into integer
         
    hostname_to_ip("elra-01.cs.colorado.edu",ip);
    printf("IP address server %s",ip);

    int dotcounter = 0,i=0,ip[4];
         char temp_ip[3];
 
    while(dotcounter < 4)
    {
       
        if(*(argv[1])!= '.' && *(argv[1]) != '\0')
        {
          temp_ip[i] = *(argv[1]);
          i++;
          argv[1]++; 
        }
        else
        {
                             
                 ip[dotcounter] = atoi(temp_ip); 
                 memset(temp_ip,0,3);
                 dotcounter++;
                 i=0;
                 argv[1]++;
                 
        }
    }
    
      ip_addr = 16777216*ip[0] + 65536*ip[1] + 256*ip[2] + ip[3];
       port = atoi(argv[2]);  // Get the port number from the user input

  } 

  // Open a UDP socket
  if((udp_socket=socket(AF_INET, SOCK_DGRAM, 0))<0)
  {
	perror("Socket Failed");
  }
	
   
   // Connect to the server address using parameters in sockaddr_in structure 
   struct sockaddr_in serveraddr;
   memset( &serveraddr, 0, sizeof(serveraddr) );
   socklen_t addrlen = sizeof(serveraddr);
   serveraddr.sin_family = AF_INET;
   serveraddr.sin_port = htons( port );              
   serveraddr.sin_addr.s_addr = htonl( ip_addr );
  
  // Connect/Bind our socket to the server
  connect(udp_socket, (struct sockaddr *)&serveraddr, sizeof(serveraddr)); 
  		        
  while(1)
  {
	// Command Menu
    printf("\n Enter any of the Following Commands  \n 1. get [file_name] \n 2. put [file_name] \n 3. delete [file_name] \n 4. ls \n 5. md \n 5. exit \n");
    
    memset(inp,0,50);
    memset(command,0,10);
    memset(filename,0,20);
	// Recieve the command
    scanf ("%[^\n]%*c", inp);
    sendto(udp_socket, inp, strlen(inp), 0, (struct sockaddr *)&serveraddr,sizeof(serveraddr));
    sscanf(inp,"%s %s",command,filename);
	// Put the file into the server	
   	 if(((strcmp(command,"put")==0) ) && (*filename != '\0')) // Check if the filename is empty or not
 	  {
		long int num_packets = 0, dropped = 0;
		fptr = fopen(filename,"rb");		// open the file to read
   	        
		setsockopt(udp_socket,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));  // Set the timeout for Acknowlegment 

		if((fptr == NULL)) // Check if the filename was right or wrong
		{
			printf("Wrong File name \n");
			// Send filesize as zero so that server will know that filename was wrong
			sendto(udp_socket, &num_packets, sizeof(num_packets), 0, (struct sockaddr *)&serveraddr,sizeof(serveraddr));
			recvfrom(udp_socket, &num_packets, sizeof(num_packets), 0, (struct sockaddr *)&serveraddr, &addrlen);  // Recieve ACK for number of packets
		}
		else
		{
		
			int counter = 0, timeout_flag = 0;
			fseek(fptr,0,SEEK_END);
  			size_t file_size=ftell(fptr);
  			fseek(fptr,0,SEEK_SET);
			memset(&packet,0,sizeof(packet));
			
			printf("File Size - %ld \n",file_size);
			num_packets = (file_size / BUFSIZE )+ 1; // Number of packets to send
 
			// Send the number of packets to the reciever
			sendto(udp_socket,&(num_packets), sizeof(num_packets), 0, (struct sockaddr *)&serveraddr,sizeof(serveraddr));
			// wait for ack and if timeout then resend
			while(recvfrom(udp_socket, &ACK, sizeof(ACK), 0, (struct sockaddr *)&serveraddr, &addrlen) < 0)
			{
			
				sendto(udp_socket,&(num_packets), sizeof(num_packets), 0, (struct sockaddr *)&serveraddr,sizeof(serveraddr));
				counter++;
				// Even after 100 tries if the server fails then give connection timeout
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
			   for(long int j =0; j<packet.length ; j++)
				{	
					packet.p[j] ^= key;
				}
			    	
			    // Send the encrypted data
			    sendto(udp_socket, &packet, sizeof(packet) , 0, (struct sockaddr *)&serveraddr,sizeof(serveraddr));	
			    			
				// If timeout flag is sent then give connection timeout
			     if(timeout_flag)
			 	{
					printf("File Not Sent - Connection Timeout\n"); 
					break;
				}
				// recieve ACK from the reciever
			       recvfrom(udp_socket, &ACK, sizeof(ACK), 0, (struct sockaddr *)&serveraddr, &addrlen) ;
				// Keep re sending packets until ack for current packet is recieved
			    while((ACK != packet.ID))
			    {	
				sendto(udp_socket, &packet, sizeof(packet), 0, (struct sockaddr *)&serveraddr,sizeof(serveraddr));
				recvfrom(udp_socket, &ACK, sizeof(ACK), 0, (struct sockaddr *)&serveraddr, &addrlen) ;
				printf("\t\tPacket no %ld dropped, Total - %ld \n",packet.ID,++dropped);  
				counter++;
				if(counter == 100)
				{timeout_flag = 1;
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



	  // client
	  else if((strcmp(command,"get")==0)  && (*filename != '\0'))
	  {
		
	    
	      
		long int num_packets = 0;
		// Recieve the number of packets of the file
		recvfrom(udp_socket, &num_packets, sizeof(num_packets), 0, (struct sockaddr *)&serveraddr, &addrlen);
		//Server  ACK
		sendto(udp_socket, &num_packets, sizeof(num_packets), 0, (struct sockaddr *)&serveraddr, addrlen);
		
		printf("Packets expected - %ld \n",num_packets);

		if(num_packets == 0)
		{
			printf("Empty File - Not Created \n");// if 0 packets are recieved that means file can not be created
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
			    recvlen = recvfrom(udp_socket, &packet, sizeof(packet), 0, (struct sockaddr *)&serveraddr, &addrlen);
			    
			    // send acknowledgement
			    sendto(udp_socket, &(packet.ID), sizeof(packet.ID), 0, (struct sockaddr *)&serveraddr, addrlen);
			    
			    if(packet.ID < i)  // If repeated paxket is recived then discard it and reduce the for loop count of recieved packet
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
				printf("Packet number = %ld Recieved Size %ld  \n",packet.ID,packet.length);
				total_bytes += packet.length;
			    }
			    	
			    			
			}
			for(int k=0;k<100;k++)
			{
				struct timeval tout = {0,800};
				setsockopt(udp_socket,SOL_SOCKET,SO_RCVTIMEO,(char*)&tout,sizeof(struct timeval));

				recvfrom(udp_socket, &packet, sizeof(packet), 0, (struct sockaddr *)&serveraddr, &addrlen);
				
				tout.tv_sec = 0;
				tout.tv_usec = 0;
				setsockopt(udp_socket,SOL_SOCKET,SO_RCVTIMEO,(char*)&tout,sizeof(struct timeval));

				
			}
			printf("Total bytes - %d\n",total_bytes);
			fclose(fptr);	
		}	
		 
	  }

	// client - delete command
	  else if((strcmp(command,"delete")==0) && (*filename != '\0')) 
	  {
		  char ret[50];
		  memset(ret,0,50);
		  // Print the server reply for delete command
		  recvfrom(udp_socket, ret, 50, 0, (struct sockaddr *)&serveraddr, &addrlen);
		  printf(" %s \n",ret);
   	  }
	// Get the md5sum for the file on the client side
	 else if((strcmp(command,"md")==0) && (*filename != '\0'))
	 {
	      	char cmd[50] = "md5sum ";
			strcat(cmd,filename);
			printf("\n md5sum of %s on Client Side \n\n\t",filename);
			system(cmd);
	 }
	// get the list of files on the server side and print it
	  else if(strcmp(command,"ls")==0)
	  {
		char filelist[200] = {0};
		memset(filelist,0,200);
		recvfrom(udp_socket, filelist, 200, 0, (struct sockaddr *)&serveraddr, &addrlen);
		printf("\n \nList of files is: \n%s \n",filelist);
		
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
