//
// Created by Clancy on 2021/1/26.
//

#ifndef CPP_CMD_CS_TEST_SERVER_MANAGEMENT_H
#define CPP_CMD_CS_TEST_SERVER_MANAGEMENT_H

#include <cstdio>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include "string"
#include <iostream>
#include <cerrno>
#include <vector>
#include <map>

using namespace std;

#define SOCKET int
#define SOCKADDR_IN sockaddr_in
#define SOCKADDR sockaddr
#define POST 1234

/* *********** 基础信息存放结构体 ************** */
// 数据信息
struct DataInfo {
    uint16_t flags;     // 标志位:0x01登录,0x02退出,0x03发送信息
    uint16_t len;       // 负载长度(小于1000)
    char data[1020];    // 负载
};

// socket信息
struct SocketInfo{
    SOCKET socket;
    SOCKADDR_IN sockaddrIn;
};

// 客户端详情
struct ClientInfo{
    uint64_t clientId;
    string clientName;
    string clientIp;
    uint64_t connectNum;
    ClientInfo(): connectNum(0){};
    bool operator==(const ClientInfo& clientInfo) const{
        return (clientInfo.clientId == this->clientId && clientInfo.clientName == this->clientName);
    }
};

// 聊天记录
struct ChatRecord{
    ClientInfo clientInfo;
    string record;
    ChatRecord(ClientInfo& clientInfo,const string& record):clientInfo(clientInfo),record(record){}
};

// SocketThread线程参数
struct SocketParam{
    SocketInfo socketInfo;
    ClientInfo clientInfo;
    vector<ChatRecord> *chatRecordVector;
    map<uint64_t ,ClientInfo> *clientMap;
};

// CallbackThread参数
struct CallbackParam{
    uint16_t *m_stop;
    SOCKET clientSocket;
    vector<ChatRecord> *chatRecordVector;
    CallbackParam(uint16_t* m_stop,SOCKET clientSocket,vector<ChatRecord> *chatRecordVector):
            m_stop(m_stop),clientSocket(clientSocket),chatRecordVector(chatRecordVector)
    {}
};

// 服务管理类,用于启动
bool Init(SocketInfo&);

class ServiceManagement {
public:
    ServiceManagement();
    ~ServiceManagement(){
        if(status_) close(receive_socket_);
    }
    bool status_; //激活状态
    SOCKET receive_socket_;
    SOCKADDR_IN receive_add_in_;
};


void * SocketThread(void *arg);





#endif //CPP_CMD_CS_TEST_SERVER_MANAGEMENT_H
