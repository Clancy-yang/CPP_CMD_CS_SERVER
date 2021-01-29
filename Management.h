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
#define POST 1234

//数据结构体:用于数据交互
struct Data {
    uint16_t flags;     // 标志位:0x01登录,0x02退出,0x03发送信息
    uint16_t data_len;  // 负载长度(小于1000)
    char data[1020];    // 负载
};

struct SendMsg{
    uint16_t flags;     // 标志位:
    bool *m_stop;        //
    SOCKET client_socket;
};

struct ClientInfo{
    string ip;
    uint64_t connect_num;
    ClientInfo():connect_num(0){};
};

struct Client{
    SOCKET client_socket;
    SOCKADDR_IN client_addr;
    ClientInfo client_info;
};

struct ChatRecord{
    string ip;
    string record;
    ChatRecord(const string& ip,const string& record):ip(ip),record(record){}
};

//服务管理类
class ServiceManagement {
public:
    ServiceManagement();
    ~ServiceManagement(){
        if(status_) close(receive_socket_);
    }
    bool run(Client &client);

    bool status_; //激活状态
    SOCKET receive_socket_;
    SOCKADDR_IN receive_add_in_;
};


void * run(void *arg);



#endif //CPP_CMD_CS_TEST_SERVER_MANAGEMENT_H
