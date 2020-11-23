# OSN Assignment 6

## Server Client Communication through TCP/IP
#### Pooja Desur 2019101112 

------------------------

#### To Run files
Execute the server.c and client.c files in two different terminals.
In one terminal, run server executable first.
```sh
$ ./server
```
After running server executable, run client executable.
```sh
$ ./client
```
This will bring you to a CLI on the client side. Use the following command in order to download one or multiple files that exist on the server directory. 
```sh
client-> get <file_name1> <file_name2>
```
If a file does not exist in the server directory, an error message will be displayed on both server and client side.
If a file is requested with the get command and already exists in the client directory, it will be deleted and replaced by the new file that will be downloaded from the server directory.
In ordder to exit the CLI and close the connection between client and server run the following command on the client side CLI.
```sh
client-> exit
```
### Setting up the Sockets

##### Server side
- socket() with parameter SOCK_STREAM creates the socket as a TCP connection
- bind() attaches the created socket to port 8000
- listen() waits for a client to approach the server to make a connection
- accept() extracts the first connection requesst and creates a new connected socket. Now connection is established between client and server.

##### Client side
- socket() with parameter SOCK_STREAM creates the socket as a TCP connection
- inet_pton(AF_INET) Converts an IP address in numbers-and-dots notation into struct in_addr
- connect() connects the socket referred to by the file descriptor sockfd to the address specified by addr.

### Transfer of Data

Client and Server use functions send() and recv() in order to read and write a specified number of bytes from the socket stream respectively.
Since files of size 1 GB and more can not be read all at once, the server sends chunks of size 10^5 bytes of the file at a time to the client. 
The client ensures that it receives all the data using recv() by reading from the same send more than once, and up until the full size meant to be recv() has been read by client. 

##### Code Flow
In order to ensure no data is lost while both client and server and sending and receiving, in each file, send() and recv() calls are made alternately. recv() will wait until a send message from the other end arrives. This way it can be ensured that the full message is received on one end before the other end sends another message.
Sending of data and acknowledgements have all been implemented through 8 send() recv() pairs on either side of client/server, which continue in a loop.

1. Client first sends number of files requested to be downloaded from one get command, which server recv() as numFiles.
2. Server sends an acknowledgment to Client that it has received the number of files.
3. Client sends the name of the file requested to server which retreives it with recv().
4. Server sends back the size of the file if it exists or a "-1" if the file does not exist.
If the file does not exist, a series of acknowledgements are sent between the client and server which ensures each has alternating send() and recv() and when the loop continues, it starts again at point 3.
5. Client sends a message to server that it is ready to begin download.
6. Server sends a part of the file to the client. For larger bytes of data being sent, in order to ensure the full message is read from the socket stream, the client calls recv() in a loop, until the entire number of bytes has been read.
7. Client sends an acknowledgment that the part of the file has been received successfully. 
8. Server sends a message that the entire file has been sent.



