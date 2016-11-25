#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>

#define PORT 8880	//端口号
#define BUFFER_SIZE 1024		//缓冲区大小
#define LISTEN_Q 20			//监听队列个数

int main(int argc, char *argv[]) {
	struct sockaddr_in server_addr,client_addr;	//服务器和客户机地址结构
	int listenfd,connfd,sockfd;	//分别为监听套接字、请求套接字和之后存放监听客户端的套接字中间变量
	socklen_t client_addr_len=sizeof(struct sockaddr);	//客户机地址结构长度变量
	char buf[BUFFER_SIZE+1],sbuf[BUFFER_SIZE];	//缓冲区数组
	int n,i,maxi;	//中间变量
	int se_ready;	//存放select返回值
	int client[FD_SETSIZE];	//客户端套接字数组
	fd_set sets,initsets;	//读套接字集合和初始集合
	int maxfd;				//最大套接字
//	struct timeval durT = {}
	
	//设置监听套接字
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd == -1) {
		perror("****Create TCP socket");
		exit(1);
	}
	
	//服务器地址结构清零及赋值,设置端口号和IP地址
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
	server_addr.sin_port = htons(PORT);
	
	//将套接字和本地地址绑定
	if (bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
		perror("Bind the IP and PORT");
		exit(1);
	}
	
	//监听listenfd，设置队列个数
	listen(listenfd, LISTEN_Q);
	
	//客户端套接字数组初始化
	for(i = 0;i < FD_SETSIZE;i++)
		client[i] = -1;
	
	FD_ZERO(&initsets);	//初始套接字集合清零
	FD_SET(0,&initsets);		//将标准输入加入select监听
	FD_SET(listenfd,&initsets);	//将监听套接字listenfd加入select监听
	maxfd = listenfd;	//初始最大套接字暂为listenfd
	maxi = -1;		//记录最大客户端套接字数组的下标
	
	
	//循环接收客户端消息和发送键盘输入消息
	while (1) {
		
		sets = initsets;		//每一次循环中读集合重置为初始集合
		se_ready = select(maxfd + 1, &sets, NULL, NULL, NULL);	//select函数
		if (se_ready <= 0) {
			perror("Select Error.\n");
			continue;
		}
		
		if (FD_ISSET(0,&sets)) {		//如果标准输入可读
			memset(buf, 0, BUFFER_SIZE+1);	//分配内存并初始化
			fgets(buf, sizeof(buf), stdin);	//读入buf
			if (strcmp(buf, "exit\n") == 0) {	//若为‘exit’，服务器退出
				break;
			}
		}
		
		if (FD_ISSET(listenfd, &sets)) {		//监听套接字可用
			//设置接收请求套接字
			connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_len);
			if (connfd < 0) {
				perror("Accept the connection");
				continue;
			}
			
			sprintf(sbuf,"Accept from client %s:%d\n",inet_ntoa(client_addr.sin_addr),client_addr.sin_port);	 //输出连接客户端的信息（IP+PORT）
			printf(sbuf,"");
			
			for (i = 0;i < FD_SETSIZE;i++) {		//找到未用的数组下标元素
				if (client[i] < 0) {
					client[i] = connfd;
					FD_SET(client[i],&initsets);		//加入initsets
					break;
				}
			}
			if (connfd > maxfd) {		//如果该套接字大于maxfd，最将maxfd赋值为该connfd
				maxfd = connfd;
			}
			if (i > maxi) {		//最大使用的数组下标
				maxi = i;
			}
		}
		for(i = 0;i <= maxi;i++)		//查看所有连接的客户端
		{
			sockfd = client[i];		
			if (sockfd < 0) {
				continue;
			}
			
			if (FD_ISSET(sockfd,&sets)) { //若sockfd有数据可读
				memset(buf, 0, BUFFER_SIZE+1);		//内存分配初始化
				n = recv(sockfd, buf, BUFFER_SIZE, 0);	//接收客户机发来的数据
				if (n == -1) {
					perror("Receive ");
					exit(1);
				}
				if (strcmp(buf, "exit\n") == 0) {	//如果客户机发来‘exit’，则关闭该客户机的套接字并置为未使用，同时从initsets中移除
					close(sockfd);
					FD_CLR(sockfd, &initsets);
					client[i] = -1;
					break;
				}
				else {
					buf[n] = '\0';		
					printf("Client 【%d】 says : %s",i,buf);	//显示客户端发来的消息
					n = send(sockfd, buf, strlen(buf), 0);	//并且回显给客户端
					if (n == -1) {
						perror("Reply sent");
						exit(1);
					}
				}
			}
		}
		
	}
	
	if (close(listenfd) == -1) {		//关闭监听套接字，服务器关闭
		perror("Close");
		exit(1);
	}
	puts("You have input the end command.\n");
	puts("TCP Server has been closed");
	return 0;
}
