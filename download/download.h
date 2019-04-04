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




























#endif
