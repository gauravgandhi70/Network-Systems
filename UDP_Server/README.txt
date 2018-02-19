This is Readme for the first Programming assignment for the Network Systems Course

For this assignment I have created 2 folders; one for client and one for server.
You can complie the codes using the makefile in the respective folders. 
Just type make in the respective folder and it will generate client and server executables.

How to run the code
In client folder
>make
>./client <server IP> <port> 

In server folder
>make
>./server <port>


Work Done on the Client side:

Implemented a menu of commands -
1. Get - get the file from the server; if the file doesnt exist in server then display a error message. Exeution - get 'filename'
2. Put - put the file from the cclient side to the server side. Execution - put 'filename'
3. ls - list all the files in the current server directory. Execution - ls
4. delete - delete specified filename; and show if it was successful or not. Execution
5. exit - Terminate the server as well as client. Execution - exit
6. md   - Additional command which will show you md5um of same file on client side as well as server side. Execution md 'filename'


All possible error conditions, corner cases are handled so that code doesnt go in unknown state. 
Timeout is implemented in PUT command for ACK so that the packets are reliably transferred 



Workdone on the server side:

State machine is made to handle the commands from the client side. Server always responds to the commands recieved from the client.
If a wrong type of command is recieved then the server handles it gracefully. After responding to clients request the server again goes into wait mode.
When the server send files to client then I have implemented timeout for the ACK so that reliable transfer is done.


Files upto 100 mb has been successfuly sent using this code with remote server as well as local server.

Encryption is also implemented for extra credit. For encryption I have used XOR technique. If both the sides are not aware about the encryprion key then the data 
cant be read. So the data is protected. Data can be decrypted only when both sides are aware about the encryption key.

