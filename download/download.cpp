#include "download.h"

HTTP_CODE process_httpcode(const char *http_respond)
{
	char *http;
	char code[4];
	char *get;
	int len = strlen(http_respond);
	http = new char[len];
	strcpy(http,http_respond);
	get = strstr(http," ");
	get++;
	cout<<get;
	int i =0;
	while(*get != ' ')
	{
		code[i++] = *get;
		get++;
	}
	code[3] = '\0';
	cout<<"code: "<<*code<<endl;
	if(strcmp(code,"200") == 0)
		return OK;
	else if(strcmp(code,"206") == 0)
		return PARTIAL_OK;
	else if(strcmp(code,"403") == 0)
		return FORBIDDEN;
	else if(strcmp(code , "406") == 0)
		return NOTFOUND;
	else
		return UNKNOWN;
}
void process_status_code(HTTP_CODE code)
{
	switch(code)
	{
		case (OK):
		{
			cout<<"资源已全部准备ok"<<endl;
			break;
		}
		case (PARTIAL_OK):
		{
			cout<<"资源已部分准备ok"<<endl;
			break;
		}
		case (FORBIDDEN):
		{
			cout<<"请求资源无权访问!"<<endl;
			exit(0);
		}		
		case (UNKNOWN):
		{
			cout<<"未找到该资源!"<<endl;
			exit(0);
		}		
		default :
		{	
			cout<<"未知状态码code : "<<code<<endl;
			exit(0);
		}	
	}
}
long int get_file_size(int fd)
{
	struct stat st;
	int ret = fstat(fd,&st);
	assert(ret != -1);
	return st.st_size;
}
client::~client()
{
	close(socket_fd);
	delete []address;
}

void client::mysocket()
{
	STATUS my_status;
	int len = strlen(address_buf);
	address = new char[len + 1];
	strcpy(address,address_buf);
	address[len] = '\0';
	
	cout<<"address : "<<address<<endl;
	my_status = process_address();
	cout<<"my_status : "<<my_status<<endl;
	
	server.sin_family = AF_INET;
	server.sin_port   = htons(port);
	server.sin_addr.s_addr = *(int *)host->h_addr_list[0];
	
	socket_fd = socket(AF_INET,SOCK_STREAM,0);
	assert(socket >= 0);
	/*创建连接*/
	int connects = connect(socket_fd,(struct sockaddr*)&server,sizeof(server));
	if(connects >= 0)
	{
		cout<<"创建连接成功 :"<<connects<<endl;
	}else
	{
		cout<<"创建连接失败 : "<<connects<<endl;
	}
	
	sprintf(http_request,"HEAD %s HTTP/1.1\r\nHost: %s\r\nConnection: Close\r\n\r\n",address_buf,fqdn);
	cout<<http_request;
	
	/* 发送HTTP请求报文 */
	len = write(socket_fd,http_request,sizeof(http_request));
	if(len <= 0)
	{
		cout<<"发送HTTP请求报文出错 len : "<<len<<endl;
		exit(0);
	}
	process_httphead();//解析HTTP响应头	
	thread_download();	//多线程下载
}

STATUS client::process_address()
{
	char *get;
	if(strstr(address,"https") != NULL)
	{
		return HTTPS;
	}
	get = address + 7; // 因为"http://"占7字节位置
	fqdn = get;
	get = strstr(get,"/");
	*get++ = '\0';
	host = gethostbyname(fqdn);
	
	
	cout<<get<<"   "<<fqdn<<endl;
	int len = strlen(get);
	myfile_information.absolute_path = new char[len + 2];
	sprintf(myfile_information.absolute_path,"/%s",get);
	myfile_information.absolute_path[len+1] = '\0';
	
	cout<<"absolute_path : "<<myfile_information.absolute_path<<endl;
	
	len = strlen(myfile_information.absolute_path);
	int i = len;
	for(i = len - 1;i >= 0;i--)
	{
		if(myfile_information.absolute_path[i] == '/')
		{
			get = &myfile_information.absolute_path[i + 1];
			break;
		}
	}
	len = strlen(get);
	stpcpy(myfile_information.file_name,get);
	cout<<"file_name : "<<myfile_information.file_name<<endl;
	
	len = strlen(myfile_information.file_name);
	for(int i = 0;i < len;i++)
	{
		myfile_information.file_name_temp[i] = myfile_information.file_name[i];
		if(myfile_information.file_name[i] == '.')
		{
			break;
		}
	}
	sprintf(myfile_information.file_name_temp,"%stemp",myfile_information.file_name_temp);
	cout<<"file_name_temp : "<<myfile_information.file_name_temp<<endl;
	
	return HTTP;
}

void client::process_httphead()//解析HTTP响应头
{
	int k = 0;
	char ch[1];
	while(read(socket_fd,ch,1) != 0)
	{
		http_respond[k++] = ch[0];
		while(k >4 && http_respond[k] == '\n' && http_respond[k-1] == '\r' && http_respond[k-2] == '\n' && http_respond[k-3] == '\r')
		{
			break;
		}
	}
	http_respond[k] = '\0';
	cout<<http_respond<<endl;
	
	/* 分析 HTTP响应码 */
	HTTP_CODE code;
	code = process_httpcode(http_respond);
	cout<<"状态码code："<<code<<endl;
	process_status_code(code);
   
   /*解析出content-length:字段*/
    char *length;
    length = strstr(http_respond,"Content-Length:");
    if(length == NULL)
    {
        length = strstr(http_respond,"Content-length:");
        if(length == NULL)
        {
            length = strstr(http_respond, "content-Length:");
            if(length == NULL)
            {
                length = strstr(http_respond,"content-length:");
                if(length == NULL)
                {
                    cout << "NOT FOUND  Content-Length\n";
                    exit(0);
                }
            }
        }
    }
	char *get = strstr(length,"\r");
	*get = '\0';
	length = length + 16;
	myfile_information.file_length = atol(length);
	cout<<"下载文件长度 : "<<myfile_information.file_length<<endl;
	read(socket_fd,ch,1);  //读取报文最后一位数据'\0'
}
void* client::work(void *arg)
{
	char *buffer;
	struct thread_package *my = (struct thread_package *)arg;
	/*设置套接字*/
	struct sockaddr_in client;
	struct hostent *thread_host;
	thread_host = gethostbyname(my->fqdn);
	client.sin_family = AF_INET;
	client.sin_port = htons(80);
	client.sin_addr.s_addr = *(int*)thread_host->h_addr_list[0];
	
	my->sockfd = socket(AF_INET,SOCK_STREAM,0);
	assert(my->sockfd >= 0);
	int ret = connect(my->sockfd,(struct sockaddr*)&client,sizeof(client));
	assert(ret != -1);
	cout<<"thread download url : "<<my->url;
	
	char http_head_get[1000];
	sprintf(http_head_get,"GET %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\nRange: bytes=%ld-%ld\r\n\r\n",\
	my->url,my->fqdn,my->start,my->end);
	cout<<"线程HTTP请求报文："<<endl<<http_head_get;
	
	int sends = write(my->sockfd,http_head_get,sizeof(http_head_get));
	assert(sends>0);
    cout << "发送HTTP请求成功\n";
	char ch[1];
	char buf[2000];
	int k = 0;
	while(read(my->sockfd , ch, 1) != 0)
	{
		buf[k] = ch[0];
		if(k > 4 && buf[k] == '\n' && buf[k-1] == '\r' && buf[k-2] == '\n' && buf[k-3] == '\r')
		{
			break;
		}
		k++;
	}
	buf[k+1] = '\0';
	cout<<"线程响应报文："<<buf<<endl;
	HTTP_CODE code = process_httpcode(buf);
	process_status_code(code);
	int len = (my->end) - (my->start);
	buffer = new char[len];
	int fd = open(my->file_name, O_CREAT | O_WRONLY, S_IRWXG | S_IRWXO | S_IRWXU);
    assert(fd > 0);
    off_t offset;
    if((offset = lseek(fd, my->start, SEEK_SET)) < 0)
    {
        cout << "lseek is wrong!\n";
    }
	int ave = len;
	int r_ret = 0 , w_ret = 0;
	while((r_ret = read(my->sockfd,buffer,len)) > 0 && my->read_ret != ave)
	{
		my->read_ret = my->read_ret + r_ret;
		len = ave - my->read_ret;
		w_ret = write(fd,buffer,r_ret);
		my->write_ret += w_ret;
	}
	if(my->read_ret < 0)
	{
		cout<<"read from server happen error"<<endl;
	}
	delete []buffer;
	close(fd);
	close(my->sockfd);
}

void client::thread_download()
{
	long int average_bit;
	 struct thread_package *Thread_package;
	 Thread_package = new thread_package[thread_number];
	 long int start = 0;
	 int ave_bit;
	 if(access(myfile_information.file_name_temp,F_OK) == 0)	//表明该文件已存在，access函数作用是查看文件某一权限
	 {
		int fd = open(myfile_information.file_name_temp, O_CREAT | O_WRONLY, S_IRWXG | S_IRWXO | S_IRWXU); 
		long int file_size = get_file_size(fd);
		cout<<"已经下载的字节数目："<<file_size<<endl;
		close(fd);
		start = file_size;
		myfile_information.file_length = myfile_information.file_length - file_size;
		ave_bit = myfile_information.file_length / thread_number;	 
	 }
	 else{
		 ave_bit = myfile_information.file_length / thread_number;
	 }
    /*多线程下载*/
	int i;
    for(i=0; i<thread_number; i++)
    {
        Thread_package[i].read_ret = 0;//该线程已经从sockfd读取的字节数目
        Thread_package[i].write_ret = 0;//该线程已经写入文件的字节数目
        Thread_package[i].sockfd = -1;//该线程的socket
        Thread_package[i].start = start;//该线程读取文件内容的开始位置
        start = start + ave_bit;
        Thread_package[i].end = start;//该线程读取文件内容的结束位置
        Thread_package[i].fqdn = fqdn;//该线程存取访问的fqdn
        Thread_package[i].url = address_buf;//该线程存取下载地址
        strcpy(Thread_package[i].file_name, myfile_information.file_name_temp);//该线程存取文件名称CIF文件，以判断是否为断点下载
    }
    int Sum = 0;
    for(i=0; i<thread_number; i++)
    {
        /*pthread_create(&pid, NULL, work, &Thread_package[i]);
         pthread_join(pid, &statu);*/
        pthread_create(&Thread_package[i].pid, NULL, work, &Thread_package[i]);
        pthread_detach(Thread_package[i].pid);
    }	
	cout<<"打印进度条："<<endl;
	long int count = 0;
	while(1)
	{	
		count = 0;
		for(int i = 0;i < thread_number;i++)
		{
			count += Thread_package[i].write_ret;
		}
		double percent = (double)count / (double)myfile_information.file_length;
		//while(percent < 1)
		{
			printf("%-10d%\r",percent*100);
			usleep(10000);
		}
        if(count == myfile_information.file_length)
        {
            cout << "\n下载结束\n";
            break;
        }
	}
    if(count != myfile_information.file_length)
    {
        int r = remove(myfile_information.file_name_temp);
        if(r == 0)
        {
            cout << "下载失败!\n";
        }
        exit(0);
    }
    else{
        rename(myfile_information.file_name_temp, myfile_information.file_name);
        cout << "下载成功!\n";
    }	
}











