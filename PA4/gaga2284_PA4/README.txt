Author: Gaurav Gandhi

This is Readme for the Fourth Programming assignment for the Network Systems Course

For this assignment I have one folder; containing server code (webproxy.c), makefile, root folder and conf file containing the port number and cache expiration time
You can complie the code using the makefile.
Just type make in the respective folder and it will generate server executable.

How to run the code - You just have to run the proxy server in this assignment
In server folder
>make
>./webproxy


Functionality of the server:
Following Features are implemented in the proxy server

Multithreading - Multiple clients can access the different servers via my Proxy simultaneously. This is done via forking

Caching-
	Page cache - 
		It checks if a page exists in the proxy before retrieving a page from a remote
		server. If there is a valid cached copy of the page, that is returned to the
		client instead of creating a new server connection. Local file is created to store retrieved page based MD5sum of the url. New folder is created for 
		every new host.

	Expiration and Timeout setting- Timeout id taken from the conf file, and implemented using the time.h library and making entries in a file called cahceddata.txt
			Every time data in this file checked to see when was the last time a particular page was accessed

	Hostname’ IP address cache - If same hostname is requested again, my proxy server skips the DNS query to reduce DNS query time. Because host ip is cached into a database

	Block few websites - Proxy server will block client's access to the websites presennt in the "blocked.txt" file.


Error Conditions hadling - 
Multiple error conditions are handled and proper response is given to the client in those circumstances

1. 400 Bad Request: Wrong method
2. 400 Bad Request: Wrong version
3. 404 Not Found: URL not found
4. ERROR 403 Forbidden :  Blocked Website


