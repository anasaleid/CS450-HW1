#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netdb.h>

//I used this website as a reference in learning how to use connect. Mainly for line in which I call connect itself. 
//https://stackoverflow.com/questions/27014955/socket-connect-vs-bind

int main(int argc, char** argv)
{

	char command[4096];
	int i = 1;
	while(argc - i > 0)
	{	
		memset(command, 0, 4096);
		strcpy(command, "host -t MX ");
        	char* nextFile = strtok(argv[i], " \n");
		printf("Starting File: %s\n", nextFile);
		
		FILE* newFile = fopen(nextFile, "r");
                char newLine[4096];
                memset(newLine, 0, 4096);
                fgets(newLine, 4096, newFile);
                fgets(newLine, 4096, newFile);
                
		//get part after @
		char * address = strstr(newLine, "@");
		address = strtok(address, ">");	
		strcat(command, ++address);
				
		FILE* requestFile = popen(command, "r");
		fgets(newLine, 4096, requestFile);
		address = strrchr(address, ' ');
		address++;
		address[strlen(address) - 2] = ' ';
							
		int retval = -1;
		FILE* hostFile = popen("host mailvm.cs.uic.edu", "r");
		fgets(newLine, 4096, hostFile);
		char* IPAdress = strstr(newLine, "address");
		IPAdress = strstr(IPAdress, " ");
		IPAdress++;
				
                struct sockaddr_in addr;   // internet socket address data structure
   	 	addr.sin_family = AF_INET;
    		addr.sin_port = htons(25); // byte order is significant
		addr.sin_addr.s_addr = inet_addr(IPAdress); // listen to all interfaces
		
		int client_sock = socket(AF_INET, SOCK_STREAM, 0);
		if(client_sock < 0) {
			perror("Creating socket failed");
			exit(1);
		}

		socklen_t addrSize = sizeof(addr);
		if(connect(client_sock, (struct sockaddr *) &addr, addrSize) == -1)
		{
			perror("Connection failed");
			exit(1);
		}
		

		char recvMessage[4096];
		memset(recvMessage, 0, 4096);
		recv(client_sock, &recvMessage, 4096, 0); 
		printf("%s\n", recvMessage);

		char welcomeMessage[4096] = "HELO anas \r\n";
		send(client_sock, welcomeMessage, strlen(welcomeMessage), 0);
		
		int recieved = 1;
		recieved = recv(client_sock, &recvMessage, 4096, 0);
		printf("%s", recvMessage);
		                
		fseek(newFile, 0, SEEK_SET);
		char nextLine[4096];
		char mailFrom[4096] = "MAIL FROM:";
		char rcptTo[4096] = "RCPT TO:";
		memset(nextLine, 0, 4069);
		int j = 1;
		while(fgets(nextLine, 4096, newFile) != NULL)
		{
			if(j == 1)
			{
				char* emailAddress = strchr(nextLine, '<');	
				strcat(mailFrom, emailAddress);
				send(client_sock, mailFrom, strlen(mailFrom), 0);
				printf("%s\n", mailFrom);
				j++;
				recv(client_sock, &recvMessage, 4096, 0);
                                printf("%s",recvMessage);
			}
			else if(j == 2)
			{
				char* emailAddressTo = strchr(nextLine, '<');
				strcat(rcptTo, emailAddressTo);
				send(client_sock, rcptTo, strlen(rcptTo), 0);
                                printf("%s\n", rcptTo);
                                j++;
				recv(client_sock, &recvMessage, 4096, 0);
                                printf("%s",recvMessage);
			}		
			else if(j == 3)
			{
				send(client_sock, "data \r\n", strlen("data \r\n"), 0);
                                printf("data\n");
				j++;
				recv(client_sock, &recvMessage, 4096, 0);
                                printf("%s",recvMessage);
				printf("%s\n", nextLine);
                                send(client_sock, nextLine, strlen(nextLine), 0);
			}
			else if(j > 3)
			{
				j++;
				printf("%s\n", nextLine);
				send(client_sock, nextLine, strlen(nextLine), 0);
			}
			if(strstr(recvMessage, "450") != NULL)
			{
				sleep(61000);		
			}
		}
		send(client_sock, "\r\n.\r\n", strlen("\r\n.\r\n"), 0);
		recv(client_sock, &recvMessage, 4096, 0);
		printf("%s", recvMessage);
		send(client_sock, "QUIT \r\n", strlen("QUIT \r\n"), 0);
		recv(client_sock, &recvMessage, 4096, 0);
		printf("%s", recvMessage);

		fclose(requestFile);
		fclose(newFile);
		fclose(hostFile);
		i++;
		printf("Finished file: %s\n", nextFile);
	}
	return 0;
}


