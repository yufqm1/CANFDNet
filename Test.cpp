
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "CANFDNet.h"
#include <thread>
#include <Windows.h>

#include <chrono>

using namespace std;

void toHEX(CANFDRequest *request)
{
    int i;
	char* fp = (char*)request;

    unsigned char uchr;

    int count = sizeof(CANFDRequest) - sizeof(UINT8);
	for (int i = 0;i<count;i++)
	{
        uchr = *(fp + i);
        printf("%02x\n",uchr);
	}
}

void headtoHEX(CANFDHead* request)
{
	int i;
	char* fp = (char*)request;

	unsigned char uchr;

	int count = sizeof(CANFDHead);
	for (int i = 0; i < count; i++)
	{
		uchr = *(fp + i);
		printf("%02x\n", uchr);
	}
}

void toHEXbyStream()
{
    
}

uint8_t bbc(uint8_t* b, int len) //异或校验
{
	uint8_t r = b[0];
	// 求xor校验注意：XOR运算第二元素始
	for (int i = 1; i < len; i++)
	{
		r ^= b[i];
	}
	return r;
}

int main()
{
    connectDev("192.168.33.130","8000");
    //connectDev("127.0.0.1", "2400");
    CANFDRequest pkg;

    pkg.timestamp = 0;
    pkg.id = 0;

    pkg.flag_res0 = 2;
    pkg.flag_echoflag = 1;
    pkg.flag_echo = 1;
    pkg.flag_fd = 1;
    pkg.flag_rtr = 0;
    pkg.flag_ext = 0;
    pkg.flag_err = 0;
    pkg.flag_brs = 0;
    pkg.flag_esi = 0;
    pkg.flag_sndDelay = 0;
    pkg.flag_store = 0;

    pkg.channel = 0;
    pkg.length = 8;
    
    for (int i =0;i<64;i++)
    {
        pkg.data[i] = 0;
    }

    uint8_t* fd = (uint8_t*)&pkg;
    pkg.checkCode = bbc(fd, sizeof(CANFDRequest)-1);

    cout << "========= checkCode = " << pkg.checkCode << endl;

    cout << sizeof(CANFDHead) << endl;
    cout << sizeof(CANFDRequest) << endl;

    // toHEX(&pkg);

    //endianExchange(fd, sizeof(CANFDRequest));

	sendCANFD(pkg,sizeof(CANFDRequest));


	CANFDResponse resp;
	recvCANFDInfo(&resp, sizeof(CANFDRequest));
	std::cout << "<-->>>>>>>>>>>>>>>>" << std::endl;
	std::cout << resp.timestamp << std::endl;
	std::cout << resp.id << std::endl;
	std::cout << resp.length << std::endl;
	std::cout << "--------------" << std::endl;

	std::cout << resp.head.length << std::endl;
	std::cout << resp.head.para << std::endl;
	std::cout << resp.head.save << std::endl;
	std::cout << resp.head.startId << std::endl;
	std::cout << resp.head.type << std::endl;

    system("pause");
}
