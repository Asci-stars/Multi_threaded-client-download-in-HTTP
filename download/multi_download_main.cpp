#include "download.h"

int input_inform()
{
	int thread_num;
	string address,file_name;
	cout<<"请输入下载URL地址："<<" ";
	cin>>address;
	char *addr;
	char *fil;
	addr = new char[address.size()];
	address.copy(addr,address.size(),0);
	cout<<"adress: "<<addr<<endl;
	cout<<"请输入多线程数量：";
	cin >> thread_num;
	if(thread_num > 4 || thread_num <= 0)
	{
		cout<<"输入线程数量大于CPU内核数，使用默认线程数下载"<<endl;
		thread_num = 4;
	}
	cout << "multi_thread download num : "<<thread_num<<endl;
	cout<<"请输入下载文件名: ";
	cin>>file_name;
	fil = new char[file_name.size()];
	file_name.copy(fil,file_name.size(),0);
	cout<<"下载的文件名："<<fil<<endl;
	client my_client(thread_num,addr);
	my_client.mysocket();
	
	return 0;
}


int main(int argc,char *argv[])
{
	while(true)
	{
		input_inform();
	}
	return 0;	
}