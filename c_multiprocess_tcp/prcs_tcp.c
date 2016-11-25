#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/select.h>
#include <ctype.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>

#define PORT 8880		//端口号
#define BUFFER_SIZE 1024		//缓冲区大小
#define LISTEN_Q 20		//监听的队列数

char buf[BUFFER_SIZE+1];		//声明缓冲区数组

void handle_child(int signum)		//处理僵尸进程，与SIGCHLD信号绑定
{
	pid_t pid;
	int stat;
	while((pid = waitpid(-1, NULL, WNOHANG)) > 0)	//等待任何挂起子进程退出
	printf("Child %d terminated.\n",pid);
	return;
}

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
//	if (kb_ready == -1) {
//		perror("Select Wrong.");
//		return;
//	}
	if (kb_ready > 0) {		//如果有数据可读
		memset(buf, 0, BUFFER_SIZE+1); //buf指向内存空间全部初始化为指定值
		fgets(buf,BUFFER_SIZE,stdin);
	}
	return;
}

int main(int argc, char *argv[]) {
	struct sockaddr_in server_addr,client_addr;	//服务器和客户机的地址结构
	int listenfd,connfd;			//设置两个套接字，服务器的监听套接字和为客户机分配接收套接字
	socklen_t client_addr_len=sizeof(struct sockaddr);	//客户端地址长度，后面使用
	
	signal(SIGCHLD,handle_child);//注册回收僵尸进程的信号
	pid_t pid[BUFFER_SIZE];		//设置pid数组记录子进程号
	int pid_count = 0;
	
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
	
	//可重用
	int on = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	
	//绑定套接字与IP、端口号
	if (bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
		perror("Bind the IP and PORT");
		exit(1);
	}
	//监听套接字设置为非阻塞
	fcntl(listenfd, F_SETFL, O_NONBLOCK);
	//监听客户机请求
	listen(listenfd, LISTEN_Q);
	
	
	//循环接收客户端消息和发送键盘输入消息
	while (1) {
		handle_keyboard();		//非阻塞处理键盘输入
		if (strcmp(buf, "exit\n") == 0) {		//若为退出，服务器端退出，推出循环
			printf("You have input the end command.\n");
			break;
		}
		//设置子进程接收请求套接字
		connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_len);
		if (connfd < 0) {		//创建失败
//			perror("Accept the connection");
			continue;
		}
		
		pid[pid_count++] = fork();		//创建子进程并记录
		
		if (pid[pid_count-1] == -1) {	//子进程创建失败
			printf("Fork Failed.\n");
			exit(1);
		}
		else if (pid[pid_count-1] == 0) {		//在子进程中
			int n;						//用于存储接收的字节数
			close(listenfd);				//子进程中关闭父进程的监听套接字
			while (1) {
				
				memset(buf, 0, BUFFER_SIZE+1); //buf指向内存空间全部初始化为指定值
				n = recv(connfd, buf, BUFFER_SIZE, MSG_DONTWAIT);	//接受客户端发来的消息，返回值为消息字节数
				if (n <= 0) {
					continue;
				}
				if (strcmp(buf, "exit\n") == 0) {	//如果客户机要求关闭，退出循环
					break;
				}
				else {
					buf[n] = '\0';					//字符串尾设置为空字符
					printf("@Client says : [%s]",buf);	//打印客户机消息
					n = send(connfd, buf, strlen(buf), 0);	//将服务器的键盘输入发送到客户端
					if (n == -1) {						//发送失败
						perror("Reply sent");
						exit(1);
					}
				}
			}
			close(connfd);	//处理客户端的子进程关闭套接字并退出
			exit(0);
		}
		else if(pid[pid_count-1] > 0){	//如果在父进程中，关闭connfd连接套接字
			close(connfd);
			continue;
		}
		
	}
	
	if (close(listenfd) == -1) {			//关闭监听套接字
		perror("Close");
		exit(1);
	}
	
	//服务器关闭后kill所有子进程，回收资源
	for(pid_count = pid_count - 1;pid_count >= 0;pid_count--)
	{
		printf("Child %d terminated.\n",pid[pid_count]);	
		kill(pid[pid_count],SIGTERM);
	}
	puts("TCP Server has been closed");
	exit(0);
}
