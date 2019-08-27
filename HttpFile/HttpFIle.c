#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define FILE_NAME "test.txt"
#define FRAME_SIZE 1024

typedef struct{
	int file_size;
	char file_name[128];
}FILE_INFO, *P_FILE_INFO;

int Get_File_Info(P_FILE_INFO file_info);
int Send_Http_File(int file_size, char *file_name);

int main()
{
	P_FILE_INFO file_info;
	file_info = (P_FILE_INFO)malloc(sizeof(FILE_INFO));
	Get_File_Info(file_info);

	printf("\n***********************************************\n");
	printf("file name: %s\n", file_info->file_name);
	printf("file size: %d\n", file_info->file_size);
	printf("***********************************************\n");

	Send_Http_File(file_info->file_size, file_info->file_name);

	return 0;
}

int Send_Http_File(int file_size, char *file_name)
{
	char http_head_temp[256];

	char http_content_temp[256];

	char *http_head = "POST /upload HTTP/1.0\r\nHost: 127.0.0.1:5000\r\nContent-Length: %d\r\nContent-Type: multipart/form-data; boundary=----JamesChan\r\n\r\n";

	char *http_content = "------JamesChan\r\nContent-Disposition: form-data; name=\"file_name\"\r\n\r\n%s\r\n------JamesChan\r\nContent-Disposition: form-data; name=\"myFile\"\r\nContent-Type: text/plain\r\n\r\n";

	char *http_end = "\r\n------JamesChan--";

	int sockfd, ret, content_length;
	char buf[4096];
	struct sockaddr_in servaddr;

	/*create socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
	        printf("create socket error!\n");
	        exit(0);
	};
	printf("successfully create socket!\r\n");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5000);

    if (inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr) <= 0 ){
            printf("inet_pton error!\n");
            exit(0);
    };

	/*connect socket*/
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
            printf("connect error!\n");
            exit(0);
    }
    printf("successfully connect socket!\r\n\r\n");

	sprintf(http_content_temp, http_content, file_name);
	
	/*Send http head*/
	{
		content_length = strlen(http_content_temp) + file_size + strlen(http_end);

		sprintf(http_head_temp, http_head, content_length);

		printf("http_head_temp: %s\r\n", http_head_temp);

		/*WRITE*/
		if ((ret = write(sockfd, http_head_temp, strlen(http_head_temp))) < 0) {
		    printf("Send failed! ret:%d\r\n", ret);
		    return 1;
		}
	}
	
	/*Send http content*/
	{
		/*WRITE*/
		if ((ret = write(sockfd, http_content_temp, strlen(http_content_temp))) < 0) {
		    printf("Send failed! ret:%d\r\n", ret);
		    return 1;
		}
	}

	/*Send http content(data)*/
	{
		int file_fd, count;
		off_t offset;
		char read_buf[FRAME_SIZE];
		/*Open file*/
		if((file_fd = open(FILE_NAME, O_RDWR)) < 0){
			printf("[ERROR] open file failed!\r\n");
			return 0;	
		}
		
		do{
			/*Read file*/
			count = read(file_fd, read_buf, FRAME_SIZE - 1);
			read_buf[count] = '\0';

			/*Point to new offset*/
			offset += count;
			lseek(file_fd, offset, SEEK_SET);

			/*WRITE*/
			if ((ret = write(sockfd, read_buf, strlen(read_buf))) < 0) {
				printf("Send failed! ret:%d\r\n", ret);
				return 1;
			}
		}
		while(count == FRAME_SIZE - 1);
		/*Close file*/
		close(file_fd);
	}

	/*send http end*/
	{
		/*WRITE*/
		if ((ret = write(sockfd, http_end, strlen(http_end))) < 0) {
		    printf("Send failed! ret:%d\r\n", ret);
		    return 1;
		}
	}

	/*READ*/

	if((ret = read(sockfd, buf, 4096 - 1)) < 0){
		printf("recv failed! ret:%d\r\n", ret);	
		return 1;
	}
	buf[ret] = '\0';
	printf("%s\n", buf);

	//close socket
	close(sockfd);
}

int Get_File_Info(P_FILE_INFO file_info)
{
	int file_fd, file_size, count;
	off_t offset;
	char buf[FRAME_SIZE];

	/*Open file*/
	if((file_fd = open(FILE_NAME, O_RDWR)) < 0){
		printf("[ERROR] open file failed!\r\n");
		return 0;	
	}

	printf("[LOG] successfully open file! fd: %d\r\n\r\n", file_fd);
	
	do{
		/*Read file*/
		count = read(file_fd, buf, FRAME_SIZE - 1);
		buf[count] = '\0';

		/*calculate file size*/
		file_size += count;

		/*Point to new offset*/
		offset += count;
		lseek(file_fd, offset, SEEK_SET);

		//printf("%s", buf);
	}
	while(count == FRAME_SIZE - 1);

	/*Save file_info*/
	file_info->file_size = file_size;
	strcpy(file_info->file_name, FILE_NAME);

	/*Close file*/
	close(file_fd);
	return 1;
}

