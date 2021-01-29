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

// 消息回传线程
void * send_msg(void *arg){
    uint64_t record_num = 0;
    SendMsg *msg = (SendMsg*)arg;
    //在创建线程前设置线程创建属性,设为分离态,效率高
    pthread_detach(pthread_self());
    cout << "回传线程启动成功" << chat_record_vector.size()<< endl;
    while(!msg->m_stop){
        if(record_num < chat_record_vector.size()){
            string str = "["+chat_record_vector[record_num].ip+"]:"+chat_record_vector[record_num].record;
            Data result;
            result.flags = 0x03;
            result.data_len = str.size();
            memcpy(result.data,str.c_str(),result.data_len);
            write(msg->client_socket, (char*)&result, 1024);
            ++record_num;
        }else{
            usleep(5);
        }
    }
    pthread_exit(NULL);
}

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
        Data *data = new Data();
        if (recv(client->client_socket, (char*)data, 1024, 0) == -1)
        {
            cout << "[" << client->client_info.ip << "]:连接出错:"<<
            errno << "(" << strerror(errno) <<")"<< endl;
            break;
        }
        string client_info_header = "[" + client->client_info.ip + "]:";
        cout << "原始数据:" <<  (char*)data <<" flags:"<<data->flags<<" data_len:" << data->data_len << endl;
        bool stop = false;
        if (data->flags == 0x01) {       // 登录
            string str = client_info_header + "登录成功!";
            cout << str +"(当前服务器人数:" << client_map.size() << ")" << endl;
            Data result;
            result.flags = 0x01;
            result.data_len = str.size();
            memcpy(result.data,str.c_str(),result.data_len);
            write(client->client_socket, (char*)&result, 1024);
            pthread_t tid;
            SendMsg *msg = new SendMsg();
            msg->flags = 0x01;
            msg->m_stop = &stop;
            msg->client_socket = client->client_socket;
            pthread_create(&tid,NULL,send_msg,(void*)msg);
        }
        else if (data->flags == 0x02) {  // 退出
            string str = client_info_header + "退出成功!";
            cout << "[" << client->client_info.ip << "]:退出服务器!(当前服务器人数:"<<client_map.size()<<")" << endl;
            Data result;
            result.flags = 0x02;
            result.data_len = str.size();
            memcpy(result.data,str.c_str(),result.data_len);
            write(client->client_socket, (char*)&result, 1024);
            stop = true;
            break;
        }
        else if (data->flags == 0x03) {  // 通讯
            string str(data->data, data->data_len);
            cout << "[" << client->client_info.ip << "]:"<< str << endl;
            ChatRecord chatRecord(client->client_info.ip,str);
            chat_record_vector.push_back(chatRecord);
        }
//        else if(data.flags == 0x04) {  // 请求消息记录
//            string str;
//            Data result;
//            result.flags = 0x05;
//            for(auto record:chat_record_vector){
//                str += ("["+record.ip+"]:"+record.record+"\r\n");
//            }
//            write(client->client_socket, str.c_str(), str.size());
//        }

        delete data;
        data = NULL;
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
