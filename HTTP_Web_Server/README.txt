Author: Gaurav Gandhi

This is Readme for the Second Programming assignment for the Network Systems Course

For this assignment I have one folder; containing server code, makefile, root folder and conf file
You can complie the code using the makefile.
Just type make in the respective folder and it will generate server executable.

How to run the code - You just have to run the web server in this assignment
In server folder
>make
>./server_TCP


Functionality of the server:
Server hadles GET and POST methods
It supports 2 versions of HTTP - 1.1 and 1.0
All the configurations are taken from the ws.conf file. It makes server flexible to changes without compiling it again
The default page for the server is index.html
Multiple Connections and requests are handeled by forking. New process is spawned for every new connection


Error Conditions hadling - 
Multiple error conditions are handled and proper response is given to the client in those circumstances

1. 400 Bad Request: Wrong method
2. 400 Bad Request: Wrong version
3. 404 Not Found: URL not found
4. 501 Not Implemented:  File format not supported
5. 500 Internal Server Error : Memory Allocation failed


Extra Credit Implemented:
1. Pipelining: Support for persistent connections and pipelining of client requests is provided. Setsockopt is used for timeout of the socket. Persistent connection is maintained using keep-alive string in the request. If keep alive is not mentioned in the request then the request is served and socket is closed immedietly.

2. POST method support: Client side will be able to see the POST data in the server’s response along with the requested URL’s contents.

