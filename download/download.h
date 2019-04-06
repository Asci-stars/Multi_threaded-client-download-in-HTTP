#ifndef _DOWNLOAD_H
#define _DOWNLOAD_H

#include<iostream>
#include<unistd.h>
#include<fcntl.h>
#include<pthread.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<sys/time.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<assert.h>
#include<stdlib.h>
using namespace std;

enum HTTP_CODE
{
	OK,
	FORBIDDEN,
	NOTFOUND,
	UNKNOWN,
	PARTIAL_OK
};
enum STATUS
{
	HTTP=0,
	HTTPS, 
	HOST_WRONG
};

struct file_imformation{
    char *absolute_path;//文件的绝对路径
    char file_name[1000];//文件解析出来的名称
	char rename_file[1000];
    char file_name_temp[1000];//建立.*td文件，判断是否为断点下载
    long int file_length;//文件的大小字节数目
};
struct thread_package{
    pthread_t pid;//线程号
    char *url;
    char *fqdn;		//网站域名
    int sockfd;//sockfd
    long int start;//文件下载起始位置
    long int end;//文件下载结束位置
    char file_name[1000];//文件名称
    int read_ret;//读取字节数目
    int write_ret;//写入字节数目
};

/* 客户端定义 */
class client{
private:
    int socket_fd;//套接字
    int port;//端口号
    int thread_number;//开辟的线程数量
    char *address_buf;//存储URL地址
	char *real_name;     //存储文件自定义名称
	char *address;
    char *fqdn;//FQDN解析
    char http_request[1000];//http请求头填写
    char http_respond[1000];//http响应头接收
    struct sockaddr_in server;//服务器套接字地址
    struct hostent *host;//通过解析下载地址，获取IP地址
    struct thread_package Thread_package;//线程包
    struct file_imformation myfile_information;//文件信息
    STATUS status;
public:
    client(int thread_num, char *addr,char *name) : thread_number(thread_num), address_buf(addr),real_name(name){
        socket_fd = -1;
        port = 80;//默认端口为80
        fqdn = NULL;
        status = HTTP;
        memset(http_request, 0, 1000);
        bzero(&server,sizeof(server));
        bzero(&Thread_package,sizeof(Thread_package));
        bzero(&host,sizeof(host));
        bzero(&myfile_information,sizeof(myfile_information));
    }
    
    ~client();
    STATUS process_address();//解析下载地址
    void process_httphead();//解析HTTP响应头
    void thread_download();//多线程下载
    void mysocket();
private:
    static void *work(void *arg);
};



























#endif
