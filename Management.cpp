//
// Created by Clancy on 2021/1/26.
//

#include "Management.h"

ServiceManagement::ServiceManagement(){
    //创建TCP
    receive_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(receive_socket_ == -1){
        status_ = false;
        cout << "Create a new socket failed! error:(" << errno << strerror(errno) << ")" << endl;
        return;
    }else{
        status_ = true;
        cout << "Create a new socket Success!" << endl;
    }
    //初始化接收信息
    memset(&receive_add_in_, 0, sizeof(receive_add_in_));
    receive_add_in_.sin_family = AF_INET;
    receive_add_in_.sin_addr.s_addr = INADDR_ANY;
    receive_add_in_.sin_port = htons(POST);

    //将接收信息与Socket绑定
    if(bind(receive_socket_, (struct sockaddr*)&receive_add_in_, sizeof(receive_add_in_)) == -1){
        cout << "Bind Socket Failed! error:(" << errno << strerror(errno) << ")" << endl;
        status_ = false;
        return;
    }else{
        cout << "Bind Socket Success!" << endl;
    }

    //进入监听状态，等待用户发起请求
    if(listen(receive_socket_, 20) == -1){
        cout << "Listen Socket Failed! error:(" << errno << strerror(errno) << ")" << endl;
        status_ = false;
        return;
    }else{
        cout << "Listen Socket Success!" << endl;
    }
}

bool ServiceManagement::run(Client &client){
    if(!status_) return false;

    while (true) {
        Data data;
        if (recv(client.client_socket, (char*)&data, 1024, 0) == -1)
        {
            cout << "Recieve Data Failed! error:(" << errno << strerror(errno) << ")" << endl;
            break;
        }

        if (data.flags == 0x01) {       //登录
            cout << "[" << inet_ntoa(client.client_addr.sin_addr) << "]:登录服务器!" << endl;
        }
        else if (data.flags == 0x02) {  //退出
            cout << "[" << inet_ntoa(client.client_addr.sin_addr) << "]:退出服务器!" << endl;
            break;
        }
        else if (data.flags == 0x03) {  //通讯
            string str(data.data, data.data_len);
            cout << "[" << inet_ntoa(client.client_addr.sin_addr) << "]:"<< str << endl;
        }
    }
    close(client.client_socket);

    return status_;
}

vector<ChatRecord> chat_record_vector;
map<string,ClientInfo> client_map;

void * run(void *arg){
    Client* client = (Client*)arg;
    map<string,ClientInfo>::iterator it = client_map.find(client->client_info.ip);

    if(it != client_map.end()){
        it->second.connect_num++;
    }else{
        client_map.insert(pair<string,ClientInfo>(client->client_info.ip,client->client_info));
    }
    //在创建线程前设置线程创建属性,设为分离态,效率高
    pthread_detach(pthread_self());

    while (true) {
        Data data;
        memset(&data,0,sizeof(data));
        if (recv(client->client_socket, (char*)&data, 1024, 0) == -1)
        {
            cout << "[" << client->client_info.ip << "]:连接出错:"<<
            errno << "(" << strerror(errno) <<")"<< endl;
            break;
        }

        if (data.flags == 0x01) {       //登录
            cout << "[" << client->client_info.ip << "]:登录成功!(当前服务器人数:"<<client_map.size()<<")" << endl;
        }
        else if (data.flags == 0x02) {  //退出
            cout << "[" << client->client_info.ip << "]:退出服务器!(当前服务器人数:"<<client_map.size()<<")" << endl;
            break;
        }
        else if (data.flags == 0x03) {  //通讯
            string str(data.data, data.data_len);
            cout << "[" << client->client_info.ip << "]:"<< str << endl;
            ChatRecord chatRecord(client->client_info.ip,str);
            chat_record_vector.push_back(chatRecord);
        }
    }
    map<string,ClientInfo>::iterator its = client_map.find(client->client_info.ip);
    if(its != client_map.end()){
        client_map.erase(client->client_info.ip);
    }
    close(client->client_socket);
    delete client;
    client = NULL;
    pthread_exit(NULL);
}
