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












