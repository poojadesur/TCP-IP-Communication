#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<errno.h>

#define ll long long
#define PORT 8000
#define maxlim 100000

//find size of files
ll fileSize(char *file_name)
{
	struct stat st;
	if(stat(file_name,&st)==0)
		return st.st_size;
	else return -1;
}

//determines if file exists
ll fileExist(char *file_name)
{
	struct stat st;
	if(stat(file_name,&st) == 0) return 0;
	else return -1;
}

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;  
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)  // creates socket, SOCK_STREAM is for TCP. SOCK_DGRAM for UDP
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Creating Socket..\n");
    }
    

    // This is to lose the pesky "Address already in use" error message
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) // SOL_SOCKET is the socket layer itself
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;  // Address family. For IPv6, it's AF_INET6. 29 others exist like AF_UNIX etc. 
    address.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP address - listens from all interfaces.
    address.sin_port = htons( PORT );    // Server port to open. Htons converts to Big Endian - Left to Right. RTL is Little Endian

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Socket has been binded.\n");
    }
    


    // Port bind is done. You want to wait for incoming connections and handle them in some way.
    // The process is two step: first you listen(), then you accept()
    if (listen(server_fd, 3) < 0) // 3 is the maximum size of queue - connections you haven't accepted
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Listening..\n");
    }
    

    // returns a brand new socket file descriptor to use for this single accepted connection. Once done, use send and recv
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Connection accepted.\n\n");
    }
    


    while(1)
    {
        //reads either num of files to download or an exit
        char numFiles[50] = {0};
        bzero(numFiles,50);
        valread = recv(new_socket, numFiles, 50, 0); //ONE <-
        // if(valread != -1)printf("ack 1 recv | (buf) numFiles : %s valread %d\n",numFiles,valread);
        // if(valread == -1) printf("ack 1 recv error\n");

        if(strcmp(numFiles,"exit")==0)
        {
            printf("\nServer exiting.\n");
            exit(0);
        }

        //acknowledgment
        char ack2hello[50] = {0}; bzero(ack2hello,50);
        sprintf(ack2hello,"hello ack2 from server");
        valread = send(new_socket,ack2hello,50,0); //TWO ->
        // if(valread != -1) printf("ack two sent, hello server message (normal ack) valread %d\n",valread);
        // if(valread == -1) printf("ack 2 send error\n");

        int num = atoi(numFiles);
        int i = 0;
        while(i < num)
        {
            //receiving file name from client
            char fileName[150] = {0};
            bzero(fileName,150);
            valread = recv(new_socket, fileName, 150, 0); //THREE <-
            // if(valread != -1)printf("ack 3 recv (buf) filename : %s valread %d\n",fileName,valread);
            // if(valread == -1) printf("ack 3 recv error\n");

            printf("\nClient has requested for File %s\n",fileName);

            ll filesize;

            if(fileExist(fileName) == -1)
            {
                printf("File %s does not exist.\n",fileName);
                char minusone[50] = {0};
                bzero(minusone,50);
                sprintf(minusone,"-1");
                valread = send(new_socket,minusone,50,0); //FOUR ->
                // if(valread != -1)printf("ack 4 sent, sending minusone\n");
                // if(valread == -1) printf("ack 4 send error\n");
                
                char buf[50] = {0};
                bzero(buf,50);
                valread = recv(new_socket,buf,sizeof(buf),0); //FIVE a <-
                // if(valread != -1) printf("ack 5 received buf: %s valread %d\n",buf,valread);
                // if(valread == -1) printf("ack 5 recv error\n");

                char ack6send[50] = {0};
                bzero(ack6send,50);
                sprintf(ack6send,"server understands client understood");
                valread = send(new_socket,ack6send,50,0); //SIX a ->
                // if(valread != -1)printf("ack 6 sent, sending s u c u valread %d\n",50);
                // if(valread == -1) printf("ack 6 send error\n");
                // printf("\n\n");
                
                i++;
                continue; //cont goes to three
            }
            else
            {
                filesize = fileSize(fileName);
                char filesz[50] = {0};
                sprintf(filesz,"%lld",filesize);
                valread = send(new_socket,filesz,50,0); //FOUR ->
                // if(valread != -1)printf("ack 4 sent, sending filesize valread %d\n",valread);
                // if(valread == -1) printf("ack 4 send error\n");
            }

            //opening file in server directory
	        ll fd = open(fileName, O_RDONLY);
            if(fd == -1)
            {
                perror("Open() Error:");
                return 0;
            }

            ll filetrack  = 0;
            
             //receive acknowledgment ready to download
            char buf[50] = {0};
            bzero(buf,50);
            valread = recv(new_socket,buf,sizeof(buf),0); //FIVE b <-
            // if(valread != -1) printf("ack 5 received buf : %s valread %d\n",buf,valread);
            // if(valread == -1) printf("ack 5 recv error\n");

            printf("Sending File %s ...\n",fileName);
            while(filetrack < filesize)
            {
		        char *buffer = (char *) calloc(maxlim,sizeof(char)); 
                bzero(buffer,maxlim);
                ll numbytesread = read(fd,buffer,maxlim);
                // printf("num of bytes read from file on server side : %lld\n",numbytesread);
                valread = send(new_socket,buffer,maxlim,0); // SIX b ->
                // if(valread != -1)
                // {
                //     // printf("ack 6 sent, sending part of file valread %d\n",valread);
                //     printf("valread %d\n",valread);

                // }
                // if(valread == -1) printf("ack 6 send error\n");

                // //receive message it was sent successfully
                char buff[50] = {0};
                bzero(buff,50);
                valread = recv(new_socket,buff,50,0); // SEVEN <-
                // if(valread != -1) printf("ack 7 received buf : %s valread %d\n",buff,valread);
                // if(valread == -1) printf("ack 7 recv error\n");

                filetrack += maxlim;
            }

            
            char ack8send[50] = {0};
            bzero(ack8send,50);
            sprintf(ack8send,"download complete");
            valread = send(new_socket,ack8send,50,0); //EIGHT ->
            // printf("ack 8 send %d bytes\n",valread);
            // if(valread != -1) printf("ack 8 sent, sending that download is complete valread %d\n",valread);
            // if(valread == -1) printf("ack 8 send error\n");
            printf("File %s has been sent.\n",fileName);
            printf("\n\n");
    
            i++;
        }

        //CONTROL Z TILL HERE
    
    }

    // send(new_socket , hello , strlen(hello) , 0 );  // use sendto() and recvfrom() for DGRAM
    // printf("Hello message sent\n");
    // valread = read(new_socket , numFiles, 10);  // read infromation received into the numFiles
    // printf("%s\n",numFiles);
    
    return 0;
}
