#include <stdafx.h>



int _tmain(int argc, char **argv)
{
	WSADATA wsd;
	WSAStartup(MAKEWORD(2, 2), &wsd);

	//SOCKET s = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	//sockaddr_in addr_in;
	//memset(&addr_in,0,sizeof(addr_in));
	//addr_in.sin_family = AF_INET;
	//addr_in.sin_port = htons(9096);
	//addr_in.sin_addr.s_addr = inet_addr("192.168.1.47");

	//std::string sSend="POST /L/white.php?btnnum=7&code=788415 HTTP/1.1\r\n\
	//	x-requested-with: XMLHttpRequest\r\n\
	//	Accept-Language: zh-cn\r\n\
	//	Referer: http://192.168.1.47:9096/L/white.php\r\n\
	//	Accept: text/html, */*; q=0.01\r\n\
	//	Content-Type: application/x-www-form-urlencoded; charset=UTF-8\r\n\
	//	UA-CPU: AMD64\r\n\
	//	Accept-Encoding: gzip, deflate\r\n\
	//	User-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.1; Win64; x64; Trident/4.0; .NET CLR 2.0.50727; SLCC2; .NET CLR 3.5.30729; .NET CLR 3.0.30729; .NET4.0C; .NET4.0E)\r\n\
	//	Host: 192.168.1.47:9096\r\n\
	//	Connection: Keep-Alive\r\n\
	//	Cache-Control: no-cache\r\n";

	//connect(s,(struct sockaddr*)&addr_in,sizeof(addr_in));
	//send(s,sSend.c_str(),sSend.size(),0);

	DWORD dwPid=0;
	std::string sIp;
	
	RawSocket rawSock;
	std::cout<<"������Ҫץȡ�Ľ���id:";
	std::cin>>dwPid;
	std::cout<<"������Ҫץȡ��������ַ:";
	std::cin>>sIp;

	std::cout<<GetCurrentProcessId()<<std::endl;

	HandleRawData handleData(dwPid);
	rawSock.StartCatch(sIp.c_str(),&handleData);

	while(1) Sleep(1000);
	return 0;
}