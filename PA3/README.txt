This is Readme for the first Programming assignment for the Network Systems Course

Author - Gaurav Gandhi

For this assignment I have created 2 folders; one for client and one for server.
You can complie the codes using the makefile in the respective folders. 
Just type make in the respective folder and it will generate client and server executables.

How to run the code
In client folder
>make
>./client <conf file> 

In server folder
>make
>./server </root_folder> <port number>
>./server </root_folder> <port number>
>./server </root_folder> <port number>
>./server </root_folder> <port number>


Work Done on the Client side:

Implemented a menu of commands -
1. GET - Gets the file parts from 4 servers(under that user subfolder); Combines them together and creates new file. If file is not compplete because servers were down then appropriate error message is displayed
	 Extra Credit - Data Traffic optimization is done. Once the file is completely recieved then the client stops the transfer so it doesnt recive data twice the size of file
	 GET filename
2. PUT - PUT command uploads file(into user subfolder) onto DFS using md5sum%4 value of file. File parts are uploaded according to the table given in the requirement document.
	PUT filename
3. LIST - LIST command inquires what file in stored on DFS servers, and print file names stored under Username on DFS servers.
	LIST command also identifies if file pieces on DFSs are enough to reconstruct the original file. If pieces are not enough then “[incomplete]” will be added to the end of the
	file. 
	LIST

4. MKDIR - Implemented an extra command “MKDIR” with in DFC, so that you can make subfolders on DFS under user's folder. Upgraded LIST GET PUT commands so that they can access,
	download and upload files in sub folders of that user. 
	MKDIR subfolder
	LIST subfolder/
	PUT 1.txt subfolder/
	GET 1.txt subfolder/


All possible error conditions, corner cases are handled so that code doesnt go in unknown state. 
Client waits for server to respond for one second if it doesnt then it timesout

Each command is sent with the username and password info to server in clear text to be identified. Server checks these credentials and serves requests only if username and password matches as per the dfs.conf file available with the server.

Workdone on the server side:

State machine is made to handle the commands from the client side. Server always responds to the commands recieved from the client.
If a wrong type of command is recieved then the server handles it gracefully. After responding to clients request the server again goes into wait mode.

Each DFS server has its own directory named DFS1/ to DFS4/ respectively. 
After start each DFS server reads dfs.conf so that it knows all available users and their password.

When a valid user request come in DFS server always checks if there is a folder named after the username under the DFS’s directory. if there is not, creates one and uses this to handle all file pieces of this user.

Encryption is also implemented.. For encryption I have used XOR technique. If both the sides are not aware about the encryprion key then the data 
cant be read. So the data is protected. Data can be decrypted only when both sides are aware about the encryption key.

