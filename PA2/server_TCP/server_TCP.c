#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 5000 /*port*/
#define LISTENQ 8 /*maximum number of client connections*/
#define BUFSIZE 1024
#define FORKING 1

char status[200];
char type[40];
char data[1024];
char length[40];
	
struct configure{
	 char root[50];
	 char content[10][100];
	 char type[10][100];
	 char default_web[20];
	 int port;
	 int feature_counter;
	 int alive_timeout;
}conf = {0};
int8_t * itoa(int32_t num, int8_t *str, int32_t base);      //Function definition for converting data from integer to ascii string
void reverse(int8_t *str, int32_t length);                             //Function to perform reverse of the string


void find_case(char *met,char *url, char *ver);

void read_conf_file(void)
{
	FILE *f;
	f = fopen("ws.conf","rb");
	char buf[1000] = {0},*p = buf;
	fread(buf,1,1000,f);
	char garbage[8];
	
	char *lp = strstr(p,"Listen");
		    sscanf(lp,"%*s %d",&conf.port);
		    
		
		    lp = strstr(p,"DocumentRoot");
		    sscanf(lp,"%*s %s",conf.root);
		    
			
		    lp = strstr(p,"DirectoryIndex");
		    sscanf(lp,"%*s %s",conf.default_web);

		
		    lp = strstr(p,"Keep-Alive");		
		    sscanf(lp,"%*s %*s %d",&conf.alive_timeout);

                    lp = strstr(p,"#Content-Type which the server handles");
		    p = lp;
		   
	while(*p != EOF && *(p+1) != '#')
	{	  
		if(*p == '.')
		{
		  sscanf(p,"%s %s",conf.content[conf.feature_counter],conf.type[conf.feature_counter]);
		  conf.feature_counter++;
		}
		
		 p++;   	  		
		
	}	
}

int main (int argc, char **argv)
{
 int listenfd, connfd, n,option=1;
 pid_t childpid;
 socklen_t clilen;
 char buf[MAXLINE];
 struct sockaddr_in cliaddr, servaddr;
 char method[10] = {0},url[100] = {0}, version[100] = {0};
 FILE *fptr = NULL;
 char root[50] = {0};
 char content[20] = {0};  
 char type[40] = {0};
 int met = 0;
 bool keep_alive= false;
 time_t rawtime;
 time(&rawtime);
 struct timeval timeout;

 //Create a socket for the soclet
 //If sockfd<0 there was an error in the creation of the socket
  
 read_conf_file();
   
  
 if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0)
 {
  perror("Problem in creating the socket");
  exit(2);
 }

  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  
 //preparation of the socket address					
 servaddr.sin_family = AF_INET;
 servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
 servaddr.sin_port = htons(conf.port);
 
 //bind the socket
 bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

 //listen to the socket by creating a connection queue, then wait for clients
 listen (listenfd, LISTENQ);

 printf("%s\n","Server running...waiting for connections.");
int trial = 0;
 for ( ; ; ) {

	 clilen = sizeof(cliaddr);
  //accept a connection
  connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);
  
  
  printf("%s\n","Received request...");
#if FORKING
  if ( (childpid = fork ()) == 0 ) {//if it’s 0, it’s child process
#endif
  
  //close (listenfd);
  printf ("%s\n","Child created for dealing with client requests");

   //gettimeoftheday(&timestart,NULL);
   timeout.tv_sec = conf.alive_timeout;
   setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,sizeof(struct timeval));

  while ( (n = recv(connfd, buf, MAXLINE,0)) > 0 ) 
  {
   //printf("Connfd - %d\n",connfd);
   char *keepa = strstr(buf,"Connection: keep-alive");
   if(keepa)
   {
	timeout.tv_sec = conf.alive_timeout;
        setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,sizeof(struct timeval));

   }
   else
   {
	timeout.tv_sec = 0;
   	setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,sizeof(struct timeval));
   }
   bzero(length,sizeof(length));
   bzero(method,sizeof(method));
   bzero(url,sizeof(url));
   bzero(root,sizeof(root));

   int i =1;
   sscanf(buf,"%s %s %s",method,url,version); 
   //printf("URL - %s\n",url);
   if(strcmp(version,"HTTP/1.0") && strcmp(version,"HTTP/1.1"))
   {
	write(connfd, "HTTP/1.1 400 Bad Request\n", strlen("HTTP/1.1 400 Bad Request"));
	strcpy(length,"Content-length: ");
	itoa (strlen("<html><body>400 Bad Request Reason: Invalid Method :<<request method>></body></html>"), status, 10 );
	strcat(length,status);
	write(connfd, length,strlen(length));
	write(connfd, "Content-Type: html\n\n", 20); 
	write(connfd,"<html><body>400 Bad Request Reason: Invalid HTTP Version :<<request method>></body></html>",strlen("<html><body>400 Bad Request Reason: Invalid Method :<<request method>></body></html>"));

	continue;
   } 
   strcpy(root,"./www");
   	
   if(strcmp(url,"/") == 0)
   {
   	strcat(root,"/index.html");
        fptr = fopen(root,"rb");
    }
    else
    {
	strcat(root,url);
        fptr = fopen(root,"rb");
	
    }
    
    if(!fptr)
    {
	//printf("File Can Not be Opened in Process - %d\n",getpid());
	strcat(version," 404 Not Found\n");	
	write(connfd, version, strlen(version));
	strcpy(length,"Content-length: ");
	itoa (strlen("<html><body>404 Not Found Reason URL does not exist :<<requested url>></body></html>"), status, 10 );
	strcat(length,status);
	//write(connfd, length,strlen(length));
	write(connfd, "Content-Type: html\n\n", 20); 
	write(connfd,"<html><body>404 Not Found Reason URL does not exist :<<requested url>></body></html>",strlen("<html><body>404 Not Found Reason URL does not exist :<<requested url>></body></html>"));

        continue;
    }	
    else
	printf("%s Opened IN process %d\n",root,getpid());		


   	char *lp = url;
	while(*lp != '\0')
        {
	     lp++;
	}
	while(*lp != '.')
	{
	     lp--;	
	}
	
        strcpy(content,lp);
	
   	if(strcmp(method,"GET") == 0)
	{
	
		for(int i=0;i<conf.feature_counter;i++)
		{
			if(strcmp(content,conf.content[i]) == 0)
			{
				met = i;
				break;
			}
			if(i == conf.feature_counter-1)
			{		
				printf("501 Not Implemented\n");
				strcat(version," 501 Not Implemented\n");	
				write(connfd, version, strlen(version));	
				//write(connfd, "HTTP/1.1 501 Not Implemented\n", strlen("HTTP/1.1 501 Not Implemented"));
				strcpy(length,"Content-length: ");
				itoa (strlen("<html><body>501 Not Implemented <<error type>>: <<requested data>></body></html>"), status, 10 );
				strcat(length,status);
				
				write(connfd, "Content-Type: html\n\n", 20); 
				write(connfd,"<html><body>501 Not Implemented <<error type>>: <<requested data>></body></html>",strlen("<html><body>501 Not Implemented <<error type>>: <<requested data>></body></html>"));

		
				continue;			
			}
			
		}
	       
		
		fseek(fptr,0,SEEK_END);
		size_t file_size=ftell(fptr);
		fseek(fptr,0,SEEK_SET);
		strcpy(length,"Content-length: ");
		itoa (file_size, status, 10 );
		strcat(length,status);
		strcat(length,"\n");
		write(connfd, "HTTP/1.1 200 Document Follows\n\n", strlen("HTTP/1.1 200 Document Follows\n"));
		printf("HTTP/1.1 200 Document Follows\n");
		write(connfd, length,strlen(length));
		printf("%s",length);
		strcpy(type,"Content-Type: ");
		strcat(type,conf.type[met]);
		strcat(type,"\n\n");
		write(connfd,type,strlen(type)); 
		printf("%s",type);
		
		
		char c[1000000] = {0};
		fread(c,1,1000000, fptr);
		write(connfd,c,file_size);
		
	}
	else	
	{
		printf("400 Bad Request \n");
		strcat(version," 400 Bad Request\n");	
		write(connfd, version, strlen(version));	
		//write(connfd, "HTTP/1.1 400 Bad Request\n", strlen("HTTP/1.1 400 Bad Request"));
		strcpy(length,"Content-length: ");
		itoa (strlen("<html><body>400 Bad Request Reason: Invalid Method :<<request method>></body></html>"), status, 10 );
		strcat(length,status);
		//write(connfd, length,strlen(length));
		write(connfd, "Content-Type: html\n\n", 20); 
		write(connfd,"<html><body>400 Bad Request Reason: Invalid Method :<<request method>></body></html>",strlen("<html><body>400 Bad Request Reason: Invalid Method :<<request method>></body></html>"));
	}			
		
  }
  
	
  
  
  

   
    printf("%s for %d\n", "Timeout, Connection Closed",getpid());
   	
    exit(0);
#if FORKING
  }
	#endif
 close(connfd);
 }
 close (listenfd);
}


int8_t * itoa(int32_t num, int8_t *str, int32_t base)      //Function definition for converting data from integer to ascii string
{
    int32_t i = 0;
    int32_t neg=0;

                                                          // Handle 0 explicitely, otherwise empty string is printed for 0
    //printf("the number is:%d\n",num);
    if (num ==0)
    {
        *str='0';
        i++;
		*(str+i)='\0';
        //printf("The string is:%s",str);
        return str;
    }

    if(num<0 && base==16)
    {
        sprintf(str,"%X",num);                                           // convert decimal to hexadecimal
        //printf("converting %d to hexadecimal notation %s\n",num,str);    //shows the hex output for signed integer
        return str;
    }

    if(num<0 && base==8)
    {
        sprintf(str,"%o",num);                                         //convert decimal to octal
        //printf("converting %d to octal notation %s\n",num,str);        //shows the octal output of signed integer
        return str;
    }

    if (num < 0 && (base == 10 || base==2))
    {
        neg = 1;                                                       //Set neg varibale to 1 if the number is negative
        num = -num;                                                    //Consider only unsigned number initially for conversion
    }

                                                                       // Process individual digits
    while (num != 0)
    {
        int32_t rem = num % base;
        *(str+i)= (rem > 9)?(rem-10) + 'a' : rem + '0';               //Converting integer to ascii string
        i++;
        num = num/base;
    }

    if (neg==1)                                                        //If number is negative, append '-'
    {
        *(str+i)= '-';
        i++;
    }

                                                        // Append string terminator
    reverse(str, i);                                                   // Reverse the string
    *(str+i)= '\0';
    //printf("The string is:%s",str);                                    //Print the ascii string

    return str;
}

void reverse(int8_t *str, int32_t length)                             //Function to perform reverse of the string
{
    int32_t start = 0;
    int32_t end = length -1;
    while (start < end)                                               //Swap the string
    {
        int8_t temp= *(str+start);
        *(str+start)= *(str+end);
        *(str+end)=temp;
        start++;
        end--;
    }
}

