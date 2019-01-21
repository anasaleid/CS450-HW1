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

	char command[999];
	int i = 1;
	while(argc - i > 0)
	{	
		memset(command, 0, 999);
		strcpy(command, "host -t MX ");
        	
		char* nextFile = strtok(argv[i], " \n");
		printf("Starting File: %s\n", nextFile);
				
		FILE* newFile = fopen(nextFile, "r");
                char newLine[999];
                memset(newLine, 0, 999);
               
		fgets(newLine, 999, newFile);
                fgets(newLine, 999, newFile);
		
		//get part after @
		char * address = strstr(newLine, "@");
		address = strtok(address, ">");	
		strcat(command, ++address);
		
				
		FILE* requestFile = popen(command, "r");
		fgets(newLine, 999, requestFile);
		address = strrchr(address, ' ');
		address++;
		address[strlen(address) - 2] = ' ';
		
		char command2[999] = "host ";
		strcat(command2, address);
		FILE* hostFile = popen(command2, "r");
		fgets(newLine, 999, hostFile);
		char* IPAdress = strstr(newLine, "address");
		IPAdress = strstr(IPAdress, " ");
		IPAdress++;
				
                struct sockaddr_in addr;   // internet socket address data structure
   	 	addr.sin_family = AF_INET;
    		addr.sin_port = htons(25); // byte order is significant
		addr.sin_addr.s_addr = inet_addr(IPAdress); 
		
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
		

		char recvMessage[999];
		memset(recvMessage, 0, sizeof recvMessage);
		recv(client_sock, &recvMessage, 999, 0); 
		printf("%s\n", recvMessage);

		char welcomeMessage[999] = "HELO anas \r\n";
		send(client_sock, welcomeMessage, strlen(welcomeMessage), 0);
		printf("%s", welcomeMessage);
		
		memset(recvMessage, 0, sizeof recvMessage);
		recv(client_sock, &recvMessage, 999, 0);
		printf("%s", recvMessage);
		
		if(strstr(recvMessage, "501") != NULL)
                {
                        printf("Invalid domain. Try again\n");
                	exit(1);
                } 
		
		fseek(newFile, 0, SEEK_SET);
		char nextLine[999];
		char mailFrom[999] = "MAIL FROM:";
		char rcptTo[999] = "RCPT TO:";
		char saveForLater[999];
		int error = 0;
		memset(nextLine, 0, 999);
		int j = 1;
		while(fgets(nextLine, 999, newFile) != NULL)
		{
			if(j == 1)
			{
				//Send the TO line
				strcpy(saveForLater, nextLine);
				char* emailAddress = strchr(nextLine, '<');	
				strcat(mailFrom, emailAddress);
				printf("%s", mailFrom);
				send(client_sock, mailFrom, strlen(mailFrom), 0);
				send(client_sock, "\r\n", strlen("\r\n"), 0);
				j++;
				recv(client_sock, &recvMessage, 999, 0);
                                printf("%s",recvMessage);
			}
			else if(j == 2)
			{
				//SEND the FROM line
				char* emailAddressTo = strchr(nextLine, '<');
				strcat(rcptTo, emailAddressTo);
				printf("%s", rcptTo);
				send(client_sock, rcptTo, strlen(rcptTo), 0);
                                send(client_sock, "\r\n", strlen("\r\n"), 0);
				j++;
				recv(client_sock, &recvMessage, 999, 0);
                                printf("%s",recvMessage);
			}		
			else if(j == 3)
			{
				//SEND the DATA and FROM Header again
				printf("data\n");
                                send(client_sock, "data \r\n", strlen("data \r\n"), 0);
                                j++;
				recv(client_sock, &recvMessage, 999, 0);
                                printf("%s",recvMessage);
				printf("%s", saveForLater);
				send(client_sock, saveForLater, strlen(saveForLater), 0);
                                send(client_sock, "\r\n", strlen("\r\n"), 0);
                                strcpy(saveForLater, nextLine);
			}
			else if(j == 4)
			{
				//SEND the subject line
				printf("%s", saveForLater);
				send(client_sock, saveForLater, strlen(saveForLater), 0);
                                send(client_sock, "\r\n", strlen("\r\n"), 0);	
				j++;
			}
			else
			{
				//SEND the rest of the email
				printf("%s", nextLine);
                                send(client_sock, nextLine, strlen(nextLine), 0);
			}
			if(strstr(recvMessage, "450") != NULL)
			{
				printf("Address was graylisted. Try again later.\n");
				error = 1;
				break;		
			}
			else if(strstr(recvMessage, "550") != NULL || strstr(recvMessage, "421") != NULL)
			{
				printf("An error has occurred.\n");
                                error = 1;
                                break;
			}
			memset(nextLine, 0, sizeof nextLine);
		}
		if(error == 1)
		{
			pclose(requestFile);
                	fclose(newFile);
                	pclose(hostFile);
                	close(client_sock);
                	i++;
                	printf("File: %s had an error. Continuing to the next.\n", nextFile);
			continue;
		}
		printf("\r\n.\r\n");
		send(client_sock, "\r\n.\r\n", strlen("\r\n.\r\n"), 0);
		memset(recvMessage, 0, sizeof recvMessage);
		recv(client_sock, &recvMessage, 999, 0);
		printf("%s", recvMessage);
		printf("QUIT \r\n");
		send(client_sock, "QUIT \r\n", strlen("QUIT \r\n"), 0);
		memset(recvMessage, 0, sizeof recvMessage);
		recv(client_sock, &recvMessage, 999, 0);
		printf("%s", recvMessage);

		pclose(requestFile);
		fclose(newFile);
		pclose(hostFile);
		close(client_sock);
		i++;
		printf("Finished file: %s\n", nextFile);
	}
	return 0;
}


