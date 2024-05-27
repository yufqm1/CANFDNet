
#include "pch.h"
#include "CANFDNet.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define HEX 16

WSADATA wsaData;
SOCKET connectSocket = INVALID_SOCKET;
struct addrinfo* result = NULL,
	* ptr = NULL,
	hints;
int sendRes, recvRes;

SOCKET _connsocket = INVALID_SOCKET;

CANFDHead canHead = {};
CANFDResponse canResponse = {};

bool recvCANFDData(const SOCKET& socket);

class thread_guard
{
public:
	explicit thread_guard(std::shared_ptr<std::thread> t_) :t(t_) {}
	~thread_guard() {
		if (t->joinable())
		{
			t->join();
		}
	}

	thread_guard(thread_guard const&) = delete;
	thread_guard& operator=(thread_guard const&) = delete;

private:
	std::shared_ptr<std::thread> t;

};
std::string getHEXValue(char* fp,int offset)
{
	std::string res_str;
	unsigned char uchr ;
	for (int i = 0; i< offset; i++)
	{
		uchr = *fp;
		char id[3];
		sprintf(id, "%02x", uchr);
		res_str += std::string(id);
		*(fp++);
	}

	return res_str;
}

void getCharArrayHEXValue(UINT8* data,int offset)
{
	std::string res_str;
	unsigned char uchr;
	for (int i = 0; i < offset; i++)
	{
		uchr = *data;
		char id[3];
		sprintf(id, "%02x", uchr);
		res_str += std::string(id);
		*(data++);
	}
}

void headHEXValue(CANFDHead* head)
{
	char* fp = (char*)head;
	if (nullptr == fp)
	{
		return;
	}

	int offset = 0;
	head->startId = std::stoi(getHEXValue(fp, sizeof(head->startId)), nullptr, HEX); offset += sizeof(head->startId);
	head->type = std::stoi(getHEXValue(fp + offset, sizeof(head->type)), nullptr, HEX); offset += sizeof(head->type);
	head->para = std::stoi(getHEXValue(fp + offset, sizeof(head->para)), nullptr, HEX); offset += sizeof(head->para);
	head->save = std::stoi(getHEXValue(fp + offset, sizeof(head->save)), nullptr, HEX); offset += sizeof(head->save);
	head->length = std::stoi(getHEXValue(fp+ offset, sizeof(head->length)), nullptr, HEX);
}

void bodyHEXValue(CANFDResponse* response)
{
	char* fp = (char*)response;
	fp = fp + sizeof(CANFDHead);
	if (nullptr == fp)
	{
		return;
	}

	int offset = 0;
	std::cout << "--------- response->timestamp=" << sizeof(response->timestamp)<< std::endl;
	response->timestamp = std::stoll(getHEXValue(fp, sizeof(response->timestamp)), nullptr, HEX); offset += sizeof(response->timestamp);
	response->id = std::stoi(getHEXValue(fp + offset, sizeof(response->id)), nullptr, HEX); offset += sizeof(response->id);
	response->flag_res0 = std::stoi(getHEXValue(fp + offset, 1), nullptr, HEX); offset++;
	response->flag_echoflag = std::stoi(getHEXValue(fp + offset, 1), nullptr, HEX); offset++;
	response->flag_echo = std::stoi(getHEXValue(fp + offset, 1), nullptr, HEX); offset++;
	response->flag_fd = std::stoi(getHEXValue(fp + offset, 1), nullptr, HEX); offset++;
	response->flag_rtr = std::stoi(getHEXValue(fp + offset, 1), nullptr, HEX); offset++;
	response->flag_ext = std::stoi(getHEXValue(fp + offset, 1), nullptr, HEX); offset++;
	response->flag_err = std::stoi(getHEXValue(fp + offset, 1), nullptr, HEX); offset++;
	response->flag_brs = std::stoi(getHEXValue(fp + offset, 1), nullptr, HEX); offset++;
	response->flag_esi = std::stoi(getHEXValue(fp + offset, 1), nullptr, HEX); offset++;
	response->flag_sndDelay = std::stoi(getHEXValue(fp + offset, 1), nullptr, HEX); offset++;
	response->flag_store = std::stoi(getHEXValue(fp + offset, 1), nullptr, HEX); offset++;
	response->channel = std::stoi(getHEXValue(fp + offset, sizeof(response->timestamp)), nullptr, HEX); offset += sizeof(response->channel);
	response->length = std::stoi(getHEXValue(fp + offset, sizeof(response->timestamp)), nullptr, HEX); offset += sizeof(response->length);
	//response->data = std::stoi(getHEXValue(fp + offset, sizeof(response->data)), nullptr, HEX); 
	offset += sizeof(response->data);
	response->checkCode = std::stoi(getHEXValue(fp + offset, sizeof(response->checkCode)), nullptr, HEX);

}

void endianSwap(UINT8* pData, int startIndex, int length)
{
	int i, cnt, end, start;
	cnt = length / 2;
	start = startIndex;
	end = startIndex + length - 1;
	UINT8 tmp;
	for (i = 0; i < cnt; i++)
	{
		tmp = pData[start + i];
		pData[start + i] = pData[end - i];
		pData[end - i] = tmp;
	}
}

void endianSwap2(void* ptr, size_t size)
{
	char* p = reinterpret_cast<char*>(ptr);
	std::reverse(p, p + size);
}

bool headEndianExchange(CANFDHead* pData, int nLen)
{
	int offset[5] = { 1,1,1,1,2};

	for (int i = 0; i < sizeof(offset) / sizeof(int); i++)
	{
		endianSwap2(pData, offset[i]);
		pData += offset[i];
	}

	return true;
}

bool endianExchange(UINT8* pData, int nLen)
{
	CANFDRequest csize;
	int offset[] =
	{
		sizeof(csize.head.startId),
		sizeof(csize.head.type),
		sizeof(csize.head.para),
		sizeof(csize.head.save),
		sizeof(csize.head.length),

		sizeof(csize.timestamp),
		sizeof(csize.id),
		1,1,1,1,1,1,1,1,1,1,1,
		sizeof(csize.channel),
		sizeof(csize.length),
		sizeof(csize.data),
		sizeof(csize.checkCode),
	};

	for (int i = 0; i < sizeof(offset) / sizeof(int); i++)
	{
		endianSwap2(pData, offset[i]);
		pData += offset[i];
	}

	return false;
}

bool endianExchange2(CANFDRequest* pData, int nLen)
{
	int offset[] = 
	{ 
		sizeof(pData->head.startId),
		sizeof(pData->head.type),
		sizeof(pData->head.para),
		sizeof(pData->head.save),
		sizeof(pData->head.length),

		sizeof(pData->timestamp),
		sizeof(pData->id),
		1,1,1,1,1,1,1,1,1,1,1,
		sizeof(pData->channel),
		sizeof(pData->length),
		sizeof(pData->data),
		sizeof(pData->checkCode),
	};

	for (int i = 0;i< sizeof(offset) / sizeof(int);i++)
	{
		std::cout << i << " " << offset[i] << std::endl;
	}

	uint8_t* fd = (uint8_t*)&pData;
	for (int i = 0; i < sizeof(offset) / sizeof(int); i++)
	{
		endianSwap2(fd, offset[i]);

		std::cout << "---" << getHEXValue((char*)fd, offset[i]) << std::endl;

		fd += offset[i];
	}

	

	return false;
}

/********************************************
*	����ӿ�
*********************************************/
std::shared_ptr<thread_guard> pGuard = nullptr;
bool connectDev(const char* ip, const char* port)
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		return false;
	}
	_connsocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_connsocket == INVALID_SOCKET)
	{
		printf("invalid socket!\n");
		return false;
	}

	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(atoi(port));
	serAddr.sin_addr.S_un.S_addr = inet_addr(ip);
	if (connect(_connsocket, (sockaddr*)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{  //����ʧ�� 
		printf("connect error !\n");
		closesocket(_connsocket);
		return false;
	}

	pGuard = std::make_shared<thread_guard>(std::make_shared<std::thread>(recvCANFDData, _connsocket));

	return true;
}

bool sendCANFD(const CANFDRequest& req, int maxLen)
{
	uint8_t* fd = (uint8_t*)&req;
	endianExchange(fd, sizeof(CANFDRequest));

	while (true)
	{
		std::cout << "------------- enter send data:" << std::endl;
		int res = send(_connsocket, (char*)&req, sizeof(req), 0);
		if (res == sizeof(req))
		{
			std::cout << "=============== send success... res = " << res << std::endl;
		}

		Sleep(3000);
		break;
	}

	return true;
}

bool sendTimeSendPkg(const TimeSendPkgRequest* timeSendReq)
{
    sendRes = send(connectSocket, (char*)timeSendReq, sizeof(*timeSendReq), 0);

    if (sendRes == SOCKET_ERROR) {
        //printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return false;
    }

    return true;
}

bool sendCanBusRate(const BusUtilizationIndicationPkgRequest* busReq)
{
    return false;
}

int recvCANFDInfo(CANFDResponse* respose, int maxLen)
{
	std::cout << "recvCANFDInfo: start to recvinfo" << std::endl;

	respose->head = canHead;
	respose->timestamp = canResponse.timestamp;
	respose->id = canResponse.id;
	respose->flag_res0 = canResponse.flag_res0;
	respose->flag_echoflag = canResponse.flag_echoflag;
	respose->flag_echo = canResponse.flag_echo;
	respose->flag_fd = canResponse.flag_fd;
	respose->flag_rtr = canResponse.flag_rtr;
	respose->flag_ext = canResponse.flag_ext;
	respose->flag_err = canResponse.flag_err;
	respose->flag_brs = canResponse.flag_brs;
	respose->flag_esi = canResponse.flag_esi;
	respose->flag_sndDelay = canResponse.flag_sndDelay;
	respose->flag_store = canResponse.flag_store;
	respose->channel = canResponse.channel;
	respose->length = canResponse.length;
	for (int i = 0;i<sizeof(respose->data)/sizeof(UINT8);i++)
	{
		respose->data[i] = canResponse.data[i];
	}
	respose->checkCode = canResponse.checkCode;

	memset(&canHead, 0, sizeof(canHead));
	memset(&canResponse, 0, sizeof(canResponse));

	return true;
}

bool setDevInfo(const ModifyRequest* modifyReq, int maxLen)
{
    return false;
}

bool getDevInfo(const ModifyRequest* modifyReq, int maxLen)
{
    return false;
}

bool modifyDevInfo(const ModifyRequest* modifyReq)
{
    sendRes = send(connectSocket, (char*)modifyReq, sizeof(*modifyReq), 0);

    if (sendRes == SOCKET_ERROR) {
        closesocket(connectSocket);
        WSACleanup();
        return false;
    }

    return true;
}

bool closeDev()
{
	closesocket(_connsocket);
	WSACleanup();
	return true;
}

bool recvCANFDData(const SOCKET& socket)
{
	while(true)
	{
		Sleep(100);
		std::cout << "RECV -----> start to recv:" << std::endl;
		int nLen = recv(_connsocket, (char*)&canHead, sizeof(CANFDHead), 0);
		std::cout << "+++++++++++++++" << std::endl;
		if (nLen <= 0)
		{
			std::cout << "blocking reception..." << nLen << std::endl;
			break;
		}

		headEndianExchange(&canHead, sizeof(CANFDHead));
		headHEXValue(&canHead);

		std::cout << "--------- print head:" << std::endl;

		std::cout << "startId=" << canHead.startId << " " << (int)canHead.startId << std::endl;
		std::cout << "type=" << canHead.type << " " << (int)canHead.type << std::endl;
		std::cout << "para=" << canHead.para << " " << (int)canHead.para << std::endl;
		std::cout << "save=" << canHead.save << " " << (int)canHead.save << std::endl;
		std::cout << "length=" << canHead.length << " " << (int)canHead.length << std::endl;

		std::cout << "--------- start to recv body:" << std::endl;
		// ������
		// CANFDResponse response = {};
		int bodyLen = recv(_connsocket, (char*)&canResponse + sizeof(CANFDHead), sizeof(CANFDResponse) - sizeof(CANFDHead), 0);
		if (bodyLen == sizeof(CANFDResponse) - sizeof(CANFDHead))
		{
			// TODO  ��ʼ�������岿��
			bodyHEXValue(&canResponse);

			std::cout << "recv bodyLen = " << bodyLen << std::endl;
			std::cout << "--------- print body:" << std::endl;
			std::cout << canResponse.timestamp << std::endl;
			std::cout << canResponse.id << std::endl;
			std::cout << canResponse.flag_res0 << std::endl;
			std::cout << canResponse.flag_echoflag << std::endl;
			std::cout << canResponse.flag_echo << std::endl;
			std::cout << canResponse.flag_fd << std::endl;
			std::cout << canResponse.flag_rtr << std::endl;
			std::cout << canResponse.flag_ext << std::endl;
			std::cout << canResponse.flag_err << std::endl;
			std::cout << canResponse.flag_brs << std::endl;
			std::cout << canResponse.flag_esi << std::endl;
			std::cout << canResponse.flag_sndDelay << std::endl;
			std::cout << canResponse.flag_store << std::endl;
			std::cout << canResponse.channel << std::endl;
			std::cout << canResponse.length << std::endl;
			std::cout << canResponse.data << std::endl;
			std::cout << canResponse.checkCode << std::endl;
		}
		else
		{
			std::cout << "recv data lose ..." << std::endl;
		}

		break;
	}

    return true;
}


