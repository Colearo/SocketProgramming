#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#define BUFFER_SIZE 1024		//缓冲区大小

int main(int argc, char *argv[]) {
	struct sockaddr_in server_addr,client_addr;	//客户机和服务器的地址数据结构
	int serverfd;	//服务器的套接字
	socklen_t client_addr_len=sizeof(struct sockaddr);	//客户机地址结构大小
	char buf[BUFFER_SIZE+1];		//声明缓冲区数组
	int n;		//用以保存接收的字节数
	
	if (argc != 3) {		//判断参数个数是否为3，命令行运行形式应为./client 0.0.0.0 8880
		perror("Please use 3 args\n");
		exit(1);
	}
	
	serverfd = socket(AF_INET, SOCK_DGRAM, 0);		//创建套接字，IPv4（AF_INET），UDP（SOCK_DGRAM）
	if (serverfd == -1) {		//创建错误返回
		perror("****Create UDP socket");
		exit(1);
	}
	
   //服务器地址结构清零及赋值
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	if (inet_aton(argv[1], &server_addr.sin_addr) == 0) {
		perror("Host wrong.");
	}
	
	//绑定服务器IP、PORT等
	if (bind(serverfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
		perror("Bind the IP and PORT");
		exit(1);
	}
	
	//消息交互循环
	while (1) {
		memset(buf, 0, BUFFER_SIZE+1);	//缓冲区初始化赋值
		
		//接收客户机发送的消息，并把客户机信息存入client_addr数据结构
		n = recvfrom(serverfd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_addr_len);
		if (n == -1) {			//接收错误，返回
			perror("Receive ");
			exit(1);
		}
		if (strcmp(buf, "exit\n") == 0) {	//接收字符串为“exit”，退出循环
			break;
		}
		else {
			buf[n] = '\0';
			printf("Client says : %s\n",buf);	//显示客户机发出的消息
			
			printf("You would say :");
			fgets(buf, sizeof(buf), stdin);		//标准输入读入键盘输入的字符串
			n = sendto(serverfd, buf, strlen(buf), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));				//发送给之前存下client_addr的客户端
			if (n == -1) {
				perror("Reply sent");
				exit(1);
			}
			
		}
	}
	
	if (close(serverfd) == -1) {		//关闭套接字
		perror("Close");
		exit(1);
	}
	
	puts("UDP Server has been closed");
	return 0;
}
