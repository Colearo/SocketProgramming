#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define BUFFER_SIZE 1024		//缓冲区大小

int main(int argc, char *argv[]) {
	struct sockaddr_in server_addr;		//存储服务器数据结构
	int connfd;				//连接套接字描述符
	char buf[BUFFER_SIZE+1];	//缓冲区数组
	int n;			//读取字节数变量
	
	if (argc != 3) {		//参数个数不为3（包括第一个文件名参数），返回错误
		perror("Please use 3 args**\n");
		exit(1);
	}
	
	connfd = socket(AF_INET, SOCK_STREAM, 0);//TCP,IPv4
	
	//服务器地址结构清零及赋值,设置端口号和IP地址
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	if (inet_aton(argv[1], &server_addr.sin_addr) == 0) {
		perror("inet_aton.");
		exit(1);
	}
	
	//连接到服务器
	if (connect(connfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		perror("Connecting");
		exit(1);
	}
	
	fd_set sets;//可读文件描述符集合
	int maxfd = connfd;	//最大文件描述符为connfd
	
	printf("@Please enter your message:\n");
	while (1) {
		FD_ZERO(&sets);	//sets集合清零
		FD_SET(fileno(stdin),&sets);//标准输入加入监听
		FD_SET(connfd,&sets);	//与服务器的连接也加入监听
		
		memset(buf, 0, BUFFER_SIZE+1);//分配内存，空间初始化为0
		
		if (select(maxfd + 1, &sets, NULL, NULL, NULL) < 0) {//select函数，maxfd需加1
			perror("Select Error.\n");
			continue;
		}
		
		if (FD_ISSET(connfd,&sets)) {		//如果connfd即客户端有数据可读，则进入下一步读取
			n = recv(connfd, buf, BUFFER_SIZE, 0);//从服务器接收数据，读入buf，最大字节数BUFFER_SIZE
			if (n == 0) {					//n为0表示无数据可读，客户端关闭
				perror("***Server closed.\n");
				exit(1);
			}
			if (n == -1) {					//n为-1表示读取错误
				perror("Receive from Server");
				exit(1);
			}
			else {							
				buf[n] = '\0';				//字符串末尾加入结束标记'\0'
				printf("@[Server] send back to you:\n%s",buf);		//输出服务器回显的信息
			}
		}
		
		if (FD_ISSET(fileno(stdin), &sets)) {	//如果键盘输入可读，则读取入缓冲区中
			//printf("Having input your message.\n");
			if(fgets(buf, sizeof(buf), stdin) == NULL){
				continue;	
			}
			
			n = send(connfd, buf, strlen(buf), 0);	//发送键盘输入的字符串
			if (n == -1) {		//发送失败
				perror("Send to Server");
				exit(1);
			}
			if (strcmp(buf, "exit\n") == 0) {	//检测客户端键盘输入是否为‘exit’，是则退出
				break;
			}
		}
		
	}
	
	if (close(connfd) == -1) {		//关闭客户端的套接字
		perror("Close");
		exit(1);
	}
	
	puts("TCP Client has been closed");	//显示TCP客户端连接关闭
	return 0;

}