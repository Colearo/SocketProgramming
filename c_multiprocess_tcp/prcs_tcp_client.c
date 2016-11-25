#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <ctype.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024		//缓冲区大小

void handle_keyboard(void)		//非阻塞处理键盘输入
{
	fd_set initsets;		//读数据监听集合
	int kb_ready;
	struct timeval t;	//超时结构体
	
	FD_ZERO(&initsets);		//清零
	FD_SET(0,&initsets);		//将标准输入加入initsets
	t.tv_sec = 0;			//设置超时时延
	t.tv_usec = 10;
	
	kb_ready = select(1, &initsets, NULL, NULL, &t);		//select函数

	if (kb_ready > 0) {		//如果有数据可读
		memset(buf, 0, BUFFER_SIZE+1); //初始化缓冲区内存
		fgets(buf,BUFFER_SIZE,stdin);
	}
	return;
}

int main(int argc, char *argv[]) {
	struct sockaddr_in server_addr;		//服务器地址数据结构
	int connfd;		//客户端的连接套接字
	char buf[BUFFER_SIZE+1];		//缓冲区数组
	int n;		//用于存放接收数据流的字节数
	
	if (argc != 3) {			//如果参数个数不为3，即未在终端中输入IP和端口号
		perror("Please use 3 args\n");
		exit(1);
	}
	
	connfd = socket(AF_INET, SOCK_STREAM, 0);	//创建套接字
	
	//服务器地址结构清零及赋值
	bzero(&server_addr, sizeof(server_addr));		
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	if (inet_aton(argv[1], &server_addr.sin_addr) == 0) {
		perror("inet_aton.");
		exit(1);
	}
	
	//连接服务器
	if (connect(connfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		perror("Connecting");
		exit(1);
	}
	
	while (1) {
		memset(buf, 0, BUFFER_SIZE+1);		//初始化缓冲区内存
		printf("You would puts: ");
		fgets(buf, sizeof(buf), stdin);
		
		n = send(connfd, buf, strlen(buf), 0);	//发送消息至服务器
		if (n == -1) {
			perror("Send to Server");
			exit(1);
		}
		if (strcmp(buf, "exit\n") == 0) {		//如果消息是“exit”，即退出循环
			break;
		}
		
		n = recv(connfd, buf, BUFFER_SIZE, 0);	//接收服务器发来的消息
		if (n == -1) {
			perror("Receive from Server");
			exit(1);
		}
		if (strcmp(buf, "exit\n") == 0) {		//如果消息是“exit”，即退出循环
			break;
		}
		if (n == 0) {
			perror("Note: Sorry,server closed.\n");
			exit(1);
		}
		else {						
			buf[n] = '\0';
			printf("Server send to you: %s",buf);		//打印服务器的消息
		}
	}
	
	if (close(connfd) == -1) {				//关闭套接字
		perror("Close");
		exit(1);
	}
	
	puts("TCP Client has been closed");
	return 0;

}