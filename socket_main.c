#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int sockfd, ret, port;
	char address[16], buf[4096], type[7], para[128], message[256];
	struct sockaddr_in servaddr;
	fd_set   t_set1;
	struct timeval  tv;

	if(argc <= 3){
		printf("api <address> <port> <GET/POST/PUT/DELETE> <PARA>\r\n");
		return 0;
	}
	else if(argc == 4){
		/*GET/DELETE*/
		strcpy(address, argv[1]);
		port = atoi(argv[2]);
		strcpy(type, argv[3]); 
	}
	else if(argc == 5)
	{
		/*POST/PUT/DELETE*/
		strcpy(address, argv[1]);
		port = atoi(argv[2]);
		strcpy(type, argv[3]); 
		strcpy(para, argv[4]);
		printf("para: %s\r\n", para);
	}
	

	/*create socket*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		printf("create socket error!\n");
		exit(0);
	};
	printf("successfully create socket!\r\n");

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);

	if (inet_pton(AF_INET, address, &servaddr.sin_addr) <= 0 ){
	    printf("inet_pton error!\n");
	    exit(0);
	};

	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
	    printf("connect error!\n");
	    exit(0);
	}
	printf("successfully connect socket!\r\n");

	
	/*GET*/
	if(strcmp(type, "GET") == 0){
		sprintf(message, "GET /user/test HTTP/1.0\r\nHost: %s:%d\r\nConnection:Close\r\n\r\n", address, port);
	
	}
	/*POST*/
	else if(strcmp(type, "POST") == 0){
		sprintf(message, "POST /user/test HTTP/1.0\r\nHost: %s:%d\r\nConnection:Close\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s", address, port, strlen(para), para);
	}
	/*PUT*/
	else if(strcmp(type, "PUT") == 0){
		sprintf(message, "PUT /user/test HTTP/1.0\r\nHost: %s:%d\r\nConnection:Close\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s", address, port, strlen(para), para);
	}
	/*DELETE*/
	else if(strcmp(type, "DELETE") == 0){
		sprintf(message, "DELETE /user/test HTTP/1.0\r\nHost: %s:%d\r\nConnection:Close\r\n\r\n", address, port);
	}

	/*WRITE*/
	if (ret = write(sockfd, message, sizeof(message)) < 0) {
	printf("Send failed! ret:%d\r\n", ret);
	return 1;
	}

	/*READ*/
	ret = read(sockfd, buf, 4096 - 1);
	if(ret < 0){
		printf("recv failed! ret:%d\r\n", ret);	
		return 1;
	}
	printf("%s\n", buf);

	//close socket
	close(sockfd);
}
