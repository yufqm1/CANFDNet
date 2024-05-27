#ifndef __CANFDNET_H__
#define __CANFDNET_H__
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#ifndef DLL_IMPLEMENT
#define DLL_API _declspec(dllexport)
#else
#define DLL_API _declspec(dllimport)
#endif

#include <basetsd.h>

#define DEFAULT_BUFFLEN 1024
#pragma pack(push,1)

enum MsgID
{
    UNKNOWN,
    DEV_CONNECT,
    DEV_START,
    DEV_SEND_CANFDPKG,
    DEV_SEND_TIMEPKG,
    DEV_SEND_UTILIZATIONINDIC,   //UtilizationIndication
};

enum WorkMode
{
    Client,
    Service,
};

enum Direction
{
    No,
    Rx,
    Tx,
};


enum ProtocolType
{
    CAN,
    CANFD,
    CANFDAccelerate,
};

// ��
struct StartRequest
{
    ProtocolType proType;
    WorkMode workMode;
    bool isAccelerate;
    char ip[16];
    char localPort[10];
    char workPort[10];
};

// �ر�
struct CloseRequest
{
    bool toClose;
    char ip[16];
    char port[10];
    char deviceName[125];
};

// �޸�
struct ModifyRequest
{
    UINT32 speed;
    char ip[16];
    char port[10];
    char macAddr[125];
};

struct CANFDHead
{
	UINT8 startId;
	UINT8 type;
	UINT8 para;
	UINT8 save;
	UINT16 length;
};

// ���ͱ���
struct CANFDRequest
{
	// CANFDHEAD canfdHead;
    CANFDRequest() 
    {
        head.startId = 0x55;
        head.type = 0x01;
        head.para = 0;
        head.save = 0;
        head.length = 80; // 20480
    }

    CANFDHead head;
	UINT64 timestamp; // ʱ���
	UINT32 id;  // ����ID

	UINT8 flag_res0 : 2; // ���ı�ʶ
	UINT8 flag_echoflag : 1; // ���ı�ʶ
	UINT8 flag_echo : 1; // ���ı�ʶ
	UINT8 flag_fd : 1; // ���ı�ʶ
	UINT8 flag_rtr : 1; // ���ı�ʶ
	UINT8 flag_ext : 1; // ���ı�ʶ
	UINT8 flag_err : 1; // ���ı�ʶ
	UINT8 flag_brs : 1; // ���ı�ʶ
	UINT8 flag_esi : 1; // ���ı�ʶ
	UINT8 flag_sndDelay : 1; // ���ı�ʶ
	UINT8 flag_store : 5; // ���ı�ʶ

	UINT8 channel;
	UINT8 length;
	UINT8 data[64];

	UINT8 checkCode;
};

// ���ձ���
struct CANFDResponse
{
    CANFDHead head;

	UINT64 timestamp = 0; // ʱ���
	UINT32 id;  // ����ID

	UINT8 flag_res0 : 2; // ���ı�ʶ
	UINT8 flag_echoflag : 1; // ���ı�ʶ
	UINT8 flag_echo : 1; // ���ı�ʶ
	UINT8 flag_fd : 1; // ���ı�ʶ
	UINT8 flag_rtr : 1; // ���ı�ʶ
	UINT8 flag_ext : 1; // ���ı�ʶ
	UINT8 flag_err : 1; // ���ı�ʶ
	UINT8 flag_brs : 1; // ���ı�ʶ
	UINT8 flag_esi : 1; // ���ı�ʶ
	UINT8 flag_sndDelay : 1; // ���ı�ʶ
	UINT8 flag_store : 5; // ���ı�ʶ

	UINT8 channel;
	UINT8 length;
	UINT8 data[64];

    UINT8 checkCode;
};


struct TimeSendPkgRequest
{
    UINT8 id;
    UINT8 enable;
    UINT16 periodic;
    UINT16 sendCount;
    UINT16 mark;
    char data[DEFAULT_BUFFLEN];
};

struct BusUtilizationIndicationPkgRequest
{
    UINT8 channelId;
    UINT8 reserve;
    UINT16 busRate; // ����������
    UINT32 sendandRecvCount;
    UINT64 beginTimestamp;
    UINT64 endTimestamp;
};

struct RecvDataResponse
{
    char data[DEFAULT_BUFFLEN];
};
 
RecvDataResponse dataResponse;

#ifdef __cplusplus
extern "C" {
#endif
    DLL_API bool connectDev(const char* ip,const char* port);
    DLL_API bool sendCANFD(const CANFDRequest& req,int maxLen);
    DLL_API bool sendTimeSendPkg(const TimeSendPkgRequest* timeSendReq);
    DLL_API bool sendCanBusRate(const BusUtilizationIndicationPkgRequest* busReq);
	DLL_API int  recvCANFDInfo(CANFDResponse* canResponse, int maxLen);
	DLL_API bool setDevInfo(const ModifyRequest* modifyReq, int maxLen);
	DLL_API bool getDevInfo(const ModifyRequest* modifyReq, int maxLen);
    DLL_API bool closeDev();
    //DLL_API bool endianExchange(UINT8* pData, int nLen);
#ifdef __cplusplus
}
#endif

#pragma pack(pop)

#endif