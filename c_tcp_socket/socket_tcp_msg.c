#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#define PORT 8880		//端口号
#define BUFFER_SIZE 1024		//缓冲区大小
#define LISTEN_Q 20		//监听的队列数

int main(int argc, char *argv[]) {
	struct sockaddr_in server_addr,client_addr;	//服务器和客户机的地址结构
	int listenfd,connfd;			//设置两个套接字，服务器的监听套接字和为客户机分配接收套接字
	socklen_t client_addr_len=sizeof(struct sockaddr);	//客户端地址长度，后面使用
	char buf[BUFFER_SIZE+1];		//声明缓冲区数组
	int n;						//用于存储接收的字节数
	
	//设置监听套接字
	listenfd = socket(AF_INET, SOCK_STREAM, 0);//IPV4(AF_INET),TCP(SOCK_STREAM)
	if (listenfd == -1) {	//创建套接字失败
		perror("****Create TCP socket");
		exit(1);
	}
	
	//服务器地址结构清零及赋值
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
	server_addr.sin_port = htons(PORT);
	
	//绑定套接字与IP、端口号
	if (bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
		perror("Bind the IP and PORT");
		exit(1);
	}
	
	//监听客户机请求
	listen(listenfd, LISTEN_Q);
	
	//设置接收请求套接字
	connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_len);
	if (connfd < 0) {		//创建失败
		perror("Accept the connection");
		exit(1);
	}
	
	//循环接收客户端消息和发送键盘输入消息
	while (1) {
		memset(buf, 0, BUFFER_SIZE+1); //buf指向内存空间全部初始化为指定值
		
		n = recv(connfd, buf, BUFFER_SIZE, 0);	//接受客户端发来的消息，返回值为消息字节数
		if (n == -1) {
			perror("Receive ");
			exit(1);
		}
		if (strcmp(buf, "exit\n") == 0) {	//如果客户机要求关闭，退出循环
			break;
		}
		else {
			buf[n] = '\0';					//字符串尾设置为空字符
			printf("Client says : %s\n",buf);	//打印客户机消息
			
			printf("You would say :");			//接收服务器的键盘输入
			fgets(buf, sizeof(buf), stdin);		
			n = send(connfd, buf, strlen(buf), 0);	//将服务器的键盘输入发送到客户端
			if (n == -1) {						//发送失败
				perror("Reply sent");
				exit(1);
			}
			
		}
	}
	
	if (close(listenfd) == -1) {			//关闭监听套接字
		perror("Close");
		exit(1);
	}
	
	puts("TCP Server has been closed");
	return 0;
}
