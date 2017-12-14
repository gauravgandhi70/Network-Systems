#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netdb.h>
#include<time.h>
#include <openssl/md5.h>
#include <sys/stat.h>

#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 5000 /*port*/
#define LISTENQ 8 /*maximum number of client connections*/
#define BUFSIZE 1024
#define FORKING 1

char status[200];
char type[40];
char data[1024];
char length[40];
int listen_global;	
struct configure{
	 char root[50];
	 char content[10][100];
	 char type[10][100];
	 char default_web[20];
	 int port;
	 int feature_counter;
	 int cache_timeout;
	 int post_content_length;

}conf = {0};
int8_t * itoa(int32_t num, int8_t *str, int32_t base);      //Function definition for converting data from integer to ascii string
void reverse(int8_t *str, int32_t length);                             //Function to perform reverse of the string
void postdata_parser(char *buf,char *postdata);
void cleanup_routine(int l)
{
	printf("Closing the listenfd socket\n");
	close(listen_global);
	exit(0);
}
void find_case(char *met,char *url, char *ver);

int hostname_to_ip(char * hostname , char* ip);

void file_md5_counter(char *filename,unsigned char *md5)
{

 
 
  unsigned char md5s[MD5_DIGEST_LENGTH] = {0};
  MD5(filename, strlen(filename), md5s);
  
 // for (int i=0; i < MD5_DIGEST_LENGTH; i++)
  {
    sprintf(md5,"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",md5s[0],md5s[1],md5s[2],md5s[3],md5s[4],md5s[5],md5s[6],md5s[7],md5s[8],md5s[9],md5s[10],md5s[11],md5s[12],md5s[13],md5s[14],md5s[15]);

  }  
  //printf("md5sum for %s is %s\n",filename,md5);
  
}

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

		
		    lp = strstr(p,"Expiration");		
		    sscanf(lp,"%*s %*s %d",&conf.cache_timeout);

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
 char method[10] = {0},url[100] = {0}, version[100] = {0},ipaddr[20]={0};
 FILE *fptr = NULL;
 char root[50] = {0};
 char content[20] = {0};  
 char type[40] = {0};
 int met = 100;
 bool keep_alive= false;
 time_t rawtime;
 time(&rawtime);
 struct timeval timeout;
struct hostent* host;
 
 signal(SIGINT,cleanup_routine);
 system("rm -f IP_cache.txt");
 //Create a socket for the soclet
 //If sockfd<0 there was an error in the creation of the socket
  
 // Reading the conf file before opening the listening socket
 read_conf_file();
   
 // Open listening TCP socket 
 if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0)
 {
  perror("Problem in creating the socket");
  exit(2);
 }
  listen_global = listenfd;
  // Set socket properties to reuse the port
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

 for ( ; ; ) {
   
   clilen = sizeof(cliaddr);
  //accept a connection
  connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);
   
  
#if FORKING
  if ( (childpid = fork ()) == 0 ) {//if it’s 0, it’s child process
#endif

struct sockaddr_in host_addr;
int portf=0,servsock,n,port=0,i,sockfd1;
char buf[1000],method[300],url[300],version[10];
unsigned char md5s[MD5_DIGEST_LENGTH] = {0};
char* temp=NULL;
bzero((char*)buf,500);
recv(connfd,buf,500,0);
sscanf(buf,"%s %s %s",method,url,version);
file_md5_counter(url,md5s);

if(((strncmp(method,"GET",3)==0))&&((strncmp(version,"HTTP/1.1",8)==0)||(strncmp(version,"HTTP/1.0",8)==0))&&(strncmp(url,"http://",7)==0))
{
	strcpy(method,url);
	portf=0;
	for(i=7;i<strlen(url);i++)
	{
		if(url[i]==':')
		{
		portf=1;
		break;
		}
	}

	temp=strtok(url,"//");

	if(portf==0)
	{
		port=80;
		temp=strtok(NULL,"/");
	}
	else
	{
		temp=strtok(NULL,":");
	}

	sprintf(url,"%s",temp);
	
	int blocked = hostname_to_ip(url,ipaddr);
	if(blocked == -2)
	{
	
		char err[200] = {0};
		printf("ERROR 403 Forbidden: Wrong method %s\n",method);
		strcat(version," 400 Bad Request\n");	
		write(connfd, version, strlen(version));	
		sprintf(err,"<html><body><font size = 6><b>ERROR 403 Forbidden: %s <font></body></html>",url);
		strcpy(length,"Content-length: ");
		itoa (strlen(err), status, 10 );
		strcat(length,status);
		strcat(length,"\n");
		write(connfd, length, strlen(length));
		write(connfd, "Content-Type: html\n\n", 20); 
		write(connfd,err,strlen(err));

		close(connfd);
		close(listenfd);
		_exit(0);  

	}

	  time_t rawtime;
	  struct tm * timeinfo;

	  time ( &rawtime );
	  timeinfo = localtime ( &rawtime );

	
	FILE *fptr = fopen("cachedata.txt","a+");

	fseek(fptr,0,SEEK_END);
	size_t file_size=ftell(fptr);
	fseek(fptr,0,SEEK_SET);
	
	char *cdata = (char*)malloc(file_size);
	fread(cdata,1,file_size,fptr);
	char *founddata = strstr(cdata,method);
	
	char usecached = 0;

	if(founddata)
	{	int min,sec,diff;
		//printf("FoundData\n");
		sscanf(founddata,"%*s %d %d",&min,&sec);
		diff = (method,timeinfo->tm_min*60 +timeinfo->tm_sec - min*60 - sec);
		diff  = abs(diff);
		if(diff < conf.cache_timeout)
		{
			usecached = 1;	
			
			free(cdata);
		}
		else
		{
			fclose(fptr);
			
			fptr = fopen("cachedata.txt","w+");
			
			char c[200] = {0};
			sprintf(c,"%s %02d %02d\n",method,timeinfo->tm_min,timeinfo->tm_sec);
			int i=0;
			while(c[i])
			{
				*founddata = c[i];
				i++;founddata++;
			}
			
			fwrite(cdata, 1,file_size,fptr);
			fclose(fptr);
			free(cdata);
		}
	}
	else
	{
		printf("Data Not found; Created Entry\n");
		fprintf(fptr,"%s %02d %02d\n",method,timeinfo->tm_min,timeinfo->tm_sec);
		fclose(fptr);
		free(cdata);
	}
	
	char filepath[100] = {0};
	sprintf(filepath,"./%s/%s",url,md5s);
	
	FILE *f2 = fopen(filepath,"r");

	
	if(usecached && f2)
	{
		printf("Using Cached Data\n");

		fseek(f2,0,SEEK_END);
		size_t file_size=ftell(f2);
		fseek(f2,0,SEEK_SET);

		char *data = (char*)malloc(file_size);
		int n = fread(data,1,file_size,f2);
		send(connfd,data,n,0);
		
	}	
	else
	{
		printf("Cache Timeout; Getting Data From Server\n");
		if(portf==1)
		{
			temp=strtok(NULL,"/");
			port=atoi(temp);
		}
		//strcat(method,"^]");
		temp=strtok(method,"//");
		temp=strtok(NULL,"/");

		if(temp!=NULL)
			temp=strtok(NULL,"\n");

		//printf("\npath = %s\nPort = %d\n",temp,port);

		bzero(&host_addr,sizeof(host_addr));
		host_addr.sin_port=htons(port);
		host_addr.sin_family=AF_INET;
		host_addr.sin_addr.s_addr =  inet_addr(ipaddr);

		sockfd1=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		servsock=connect(sockfd1,(struct sockaddr*)&host_addr,sizeof(struct sockaddr));

	
		if(servsock<0)
			perror("Connection Problem");
		//printf("\n%s\n",buf);
		bzero(buf,sizeof(buf));


		if(temp!=NULL)
			sprintf(buf,"GET /%s %s\r\nHost: %s\r\nConnection: close\r\n\r\n",temp,version,url);
		else
			sprintf(buf,"GET / %s\r\nHost: %s\r\nConnection: close\r\n\r\n",version,url);

		char folder[100] = {0},delete[100]= {0};


		//sprintf(folder,"%s %s","mkdir",url);
		struct stat st = {0};
		if (stat(url, &st) == -1) {
		    mkdir(url, 0700);
		    printf("Folder Created\n");
        	   }


		sprintf(delete,"%s -f %s","rm",filepath);
		system(delete);
		
		

		n=send(sockfd1,buf,strlen(buf),0);
	
		//printf("%s Created File for path %s\n",md5s,temp);
		FILE *f1 = fopen(filepath,"w");

		if(n<0)
			perror("Cant Write to socket");
		else
		{
			do
			{
			bzero(buf,sizeof(buf));
			n=recv(sockfd1,buf,500,0);
			fwrite(buf, 1,n,f1);
			if(!(n<=0))
			send(connfd,buf,n,0);
			}while(n>0);
		
		}
		fclose(f1);
	
	}
}
else
{
send(connfd,"400 : BAD REQUEST\nONLY HTTP REQUESTS ALLOWED",18,0);
}
close(sockfd1);
close(connfd);
close(listenfd);
_exit(0);  

    
#if FORKING
  }
	#endif
 
 }
 

}
// Function to parse the post request, it returns the post data from the request
void postdata_parser(char *buf,char *postdata)
{
	char *c = strstr(buf,"Content-Length:");
	sscanf(c,"%*s %d",&conf.post_content_length);
	int n =0;
	while(n<2)
	{
		if(*c == '\n')
		{
		  n++;
		}
		c++;
	}
	strcpy(postdata,c);
	
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
int hostname_to_ip(char * hostname , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
	time_t rawtime;
  struct tm * timeinfo;
	 time ( &rawtime );
  timeinfo = localtime ( &rawtime );
		char *blocked = NULL,*c = NULL;
		size_t file_size = 0;

		FILE *f = fopen("blocked.txt","r");
		if(f)
		{
		fseek(f,0,SEEK_END);
		file_size=ftell(f);
		fseek(f,0,SEEK_SET);
	
		 c = (char*)malloc(file_size);
		fread(c,1,file_size,f);

		blocked = strstr(c,hostname);
		fclose(f);

		}
		
	


	if(blocked)
	{
		printf("Blocked \n");
		free(c);
		return -2;
	}

    	
        FILE *fptr = fopen("IP_cache.txt","a+");

	fseek(fptr,0,SEEK_END);
	file_size=ftell(fptr);
	fseek(fptr,0,SEEK_SET);
	
	char *filedata = (char*)malloc(file_size);

	fread(filedata,1,file_size,fptr);
	
	char *found_ip = strstr(filedata,hostname);
	if(found_ip)
	{
		
		
	sscanf(found_ip,"%*s %s",ip);
	free(filedata);
		return 0;
		
	}
	else
	{
	  if ( (he = gethostbyname( hostname ) ) == NULL) 
	    {
		// get the host info
		perror("gethostbyname");
		free(filedata);
		return 1;
	    }
	 
	    addr_list = (struct in_addr **) he->h_addr_list;
	     
	    for(i = 0; addr_list[i] != NULL; i++) 
	    {
		//Return the first one;
		strcpy(ip , inet_ntoa(*addr_list[i]) );
		fprintf(fptr,"%s %s %d\n",hostname,ip,timeinfo->tm_min);
		fclose(fptr);
		free(filedata);
		return 0;
	    }

		
	}

    
    return 1;
}

