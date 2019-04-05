#include "download.h"

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












