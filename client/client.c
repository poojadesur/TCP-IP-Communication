// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<errno.h>
#include <strings.h>
#include <string.h>

#define forn(i) for(int i=0;i<150;i++)
#define ll long long
#define maxlim 100000

#define PORT 8000

//determines if file exists
ll fileExist(char *file_name)
{
	struct stat st;
	if(stat(file_name,&st) == 0) return 0;
	else return -1;
}

//find size of files
ll fileSz(char *file_name)
{
	struct stat st;
	if(stat(file_name,&st)==0)
		return st.st_size;
	else return -1;
}

int main(int argc, char const *argv[])
{
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "ceepcoop";
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '\0', sizeof(serv_addr)); // to make sure the struct is empty. Essentially sets sin_zero as 0
                                                // which is meant to be, and rest is defined below

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Converts an IP address in numbers-and-dots notation into either a 
    // struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)  // connect to the server address
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    while(1)
    {
        //reading input
        printf("\033[1;36mclient->");
		char *line = (char*)malloc(150*sizeof(char)); forn(i) line[i] = '\0';
        long int len = 0;
        long int char_read = getline(&line,&len,stdin); 
        int length = (int)strlen(line);
		line[length-1] = '\0';

        //tokenizing client input
        char *input[150] = {0};
        bzero(input,150);
        int i = 0;
        char *sptr;

        char *token = (char*)malloc(150*sizeof(char)); forn(i) token[i] = '\0';
        token = strtok_r(line," \t",&sptr);
        line = NULL;

        input[i] = token;
        int numOfFiles = 0;

        //exit
        if(strcmp(input[0],"exit") == 0)
        {
            send(sock,input[i],strlen(input[i]),0); //ONE <-
            exit(0);
        }

        //ensure first input is "get"
        if(strcmp(input[0],"get") != 0)
        {
            printf("Invalid input.\n");
            continue;
        }
        
        for(int j=0; ;j++)
        {
            token = strtok_r(line," \t",&sptr);
            if(token == NULL) break;
            ++i;
            numOfFiles++;
            input[i] = token;
            // printf("input[i] %s\n",input[i]);
        }

        char num[50];
        bzero(num,50);
        sprintf(num,"%d",numOfFiles);
        // printf("\n\n");
        //sending number of files to download
        valread = send(sock,num,50,0); //ONE <-
        // if(valread != -1)printf("ack 1 sent, sent num files to download valread %d\n",valread);
        // if(valread == -1) printf("ack 1 send error\n");

        //receive acknowledgment 
        char ack2recv[50] = {0};
        bzero(ack2recv,50);
        valread = recv(sock,ack2recv,50,0); //TWO ->
        // if(valread != -1) printf("ack two received | buf: %s valread %d\n",ack2recv,valread);
        // if(valread == -1) printf("ack 2 recv error\n");


        //go through each file
        i = 1;
        while(input[i] != NULL)
        {
            // send file name 
            valread = send(sock, input[i], strlen(input[i]), 0); //THREE <- 
            // if(valread != -1)printf("ack 3 sent, sending file name valread %d\n",valread);
            // if(valread == -1) printf("ack 3 send error\n");

            // printf("%s filename sent.\n",input[i]);
            
            //receive size of file
            char filesz[50] = {0};
            bzero(filesz,50);
            valread = recv(sock, filesz, 50, 0); //FOUR ->
            // if(valread != -1)printf("ack 4 recv (buf) filesz : %s valread %d\n",filesz,valread);
            // if(valread == -1) printf("ack 4 recv error\n");
            ll fileSize = atoll(filesz);
            ll filetrack = 0;
            
            //if file doesnt exist in server directory
            if(fileSize == -1)
            {
                printf("\nFile %s doesn't exist in the server directory.\n\n",input[i]);
                char ack5send[50] = {0};
                bzero(ack5send,50);
                sprintf(ack5send,"client understands file dont exist");
                valread = send(sock,ack5send,50,0); //FIVE a <-
                // if(valread != -1) printf("ack 5 sent, client to server ack file does not exist valread %d\n",valread);
                // if(valread == -1) printf("ack 5 send error\n");
                
                char buf[50] = {0};
                // memset(buf,'\0',sizeof(buf));
                bzero(buf,50);
                valread = recv(sock,buf,sizeof(buf),0); //SIX a ->
                // if(valread != -1) printf("ack 6 recv buf : %s valread %d\n",buf,valread);
                // if(valread == -1) printf("ack 6 recv error\n");
                // printf("%s,%d\n",__func__,__LINE__);
                // printf("\n\n");

                i++;
                continue; //cont goes to three
            }

            //if file already exists in client directory, remove it and redownload
            int exist = fileExist(input[i]);
	        if(exist == 0)  remove(input[i]);

            //create new file in client directory
	        ll fd = open(input[i], O_CREAT | O_RDONLY | O_WRONLY , 0666);
            if(fd == -1)
            {
                perror("Open() Error:");
                return 0;
            }

            //send message to server that client ready to download
            char ack5bsend[50] = {0};
            bzero(ack5bsend,50);
            sprintf(ack5bsend,"pls begin download");
            valread = send(sock,ack5bsend,50,0); //FIVE b <-
            // if(valread != -1) printf("ack 5 sent, client tells server to start download valread %d\n",valread);
            // if(valread == -1) printf("ack 5 send error\n");

            printf("\nBeginning download of File %s\n\n",input[i]);

            //receive part of the file
            while(filetrack < fileSize)
            {
                //printing percentage of file downloaded
                char percentage[14];
                double x = (double)( ((double)fileSz(input[i]) / (double)(fileSize)) * 100);
                sprintf(percentage,"%.2f%%",x);
                write(1,percentage,7);
                // printf("\n");

                //reading from server file and writing into client file
                // memset(buffer, '\0', sizeof(buffer));
                char buffer[maxlim] = {0};
		        // char *buffer = (char *) calloc(maxlim,sizeof(char)); int k; forn(k) buffer[k] = '\0';
                bzero(buffer,maxlim);
                int read_so_far = 0;
                while(read_so_far < maxlim)
                {
                    valread = recv(sock, buffer + read_so_far, maxlim - read_so_far, 0); //SIX b ->
                    // if(valread != -1) printf("ack 6 received, got part of file | buffer : %s valread : %d\n",buffer,valread);
                    // if(valread != -1)
                    // {
                    //     printf("ack 8 recv buf : %s valread : %d\n",ack8buf,valread);
                    //     printf("valread in read so far loop %d\n",valread);
                    // }
                    // if(valread == -1) printf("ack 6 recv error\n");
                    read_so_far += valread;
                }
                // printf("read so far final %d\n",read_so_far);
                // printf("\nwhat was read: %s\n",buffer);
                write(fd,buffer,sizeof(buffer));

                // memset(buffer, '\0', sizeof(buffer));
                bzero(buffer,maxlim);

                //send message to server message was received successfully and has been written to new file
                char ack7send[50] = {0};
                bzero(ack7send,50);
                sprintf(ack7send,"client sends ack 7");
                valread = send(sock,ack7send,50,0); // SEVEN <-
                // if(valread != -1) printf("ack 7 sent, sending that part of file has been written valread : %d\n",valread);
                // if(valread == -1) printf("ack 7 send error\n");

                //removing current percentage of file written from stdout for next update
                char blank[10] = "\b\b\b\b\b\b\b";
                write(1,blank,7);  

                // char cut[2] = "\r";
                // write(1,cut,1);
                // fflush(stdout);

                filetrack += maxlim;
                // printf("filetrack %d",filetrack);

            }

            //acknowledgement that everything is over
            char ack8buf[50] = {0};
            bzero(ack8buf,50);
            valread = recv(sock,ack8buf,50,0); // EIGHT ->
            // if(valread != -1)
            // {
            //     printf("ack 8 recv buf : %s valread : %d\n",ack8buf,valread);
            //     printf("valread %d\n",valread);
            // }
            // if(valread == -1) printf("ack 8 recv error\n");
            // printf("size of ack 8 buf : %ld valread : %d\n",strlen(buf),valread);

            printf("File %s has been downloaded.\n\n",input[i]);
            i++;
        }
    
    }

    // send(sock , hello , strlen(hello) , 0 );  // send the message.
    // printf("Hello message sent\n");
    // valread = read( sock , buffer, 10);  // receive message back from server, into the buffer
    // printf("%s\n",buffer);
    return 0;
}
