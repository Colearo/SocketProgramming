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
	struct sockaddr_in server_addr;	//服务器的地址数据结构
	socklen_t server_addr_len=sizeof(struct sockaddr);//服务器地址结构大小
	int connfd;		//客户端的套接字
	char buf[BUFFER_SIZE+1];		//声明缓冲区数组
	int n;			//用以保存接收的字节数
	
	if (argc != 3) {		//判断参数个数是否为3，命令行运行形式应为./client 0.0.0.0 8880
		perror("Please use 3 args\n");
		exit(1);
	}
	
	//创建套接字，IPv4（AF_INET），UDP（SOCK_DGRAM）
	connfd = socket(AF_INET, SOCK_DGRAM, 0);
	
	//服务器地址结构清零及赋值
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	if (inet_aton(argv[1], &server_addr.sin_addr) == 0) {
		perror("inet_aton.");
		exit(1);
	}
	
	while (1) {
		memset(buf, 0, BUFFER_SIZE+1);	//缓冲区初始化赋值
		printf("You would say :");
		fgets(buf, sizeof(buf), stdin); //标准输入读入键盘输入的字符串
		
		n = sendto(connfd, buf, sizeof(buf), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));//发送给服务器
		if (n == -1) {
			perror("Send to Server");
			exit(1);
		}
		if (strcmp(buf, "exit\n") == 0) {//发出字符串为“exit”，退出循环
			break;
		}
		
		//接收服务器发送的消息
		n = recvfrom(connfd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&server_addr, &server_addr_len);
		if (n == -1) {		//接收错误，返回
			perror("Receive from Server");
			exit(1);
		}
		else {
			buf[n] = '\0';
			printf("Server send to you: %s\n",buf);//显示服务器发送的消息
		}
	}
	
	if (close(connfd) == -1) {		//关闭套接字
		perror("Close");
		exit(1);
	}
	
	puts("UDP Client has been closed");
	return 0;

}
