//
// Created by Clancy on 2021/1/26.
//

#include "Management.h"

bool Init(SocketInfo& socketInfo){
    //创建TCP
    socketInfo.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socketInfo.socket == -1){
        cout << "创建Socket失败(1/3):错误代码[" << errno << "]("  << strerror(errno) << ")" << endl;
        return false;
    }else
        cout << "创建Socket成功(1/3)" << endl;

    //初始化接收信息
    memset(&socketInfo.sockaddrIn, 0, sizeof(socketInfo.sockaddrIn));
    socketInfo.sockaddrIn.sin_family = AF_INET;
    socketInfo.sockaddrIn.sin_addr.s_addr = INADDR_ANY;
    socketInfo.sockaddrIn.sin_port = htons(POST);

    //将接收信息与Socket绑定
    if(bind(socketInfo.socket, (struct sockaddr*)&socketInfo.sockaddrIn, sizeof(socketInfo.sockaddrIn)) == -1){
        cout << "绑定Socket失败(2/3):错误代码["<< errno <<"](" << strerror(errno) << ")" << endl;
        return false;
    }else
        cout << "绑定Socket成功(2/3)" << endl;

    //进入监听状态，等待用户发起请求
    if(listen(socketInfo.socket, 20) == -1){
        cout << "监听Socket失败(3/3):错误代码[" << errno << "](" << strerror(errno) << ")" << endl;
        return false;
    }else
        cout << "监听Socket成功(3/3) \n 初始化成功.." << endl;

    return true;
}

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


// 消息发送
void SendMsg(SOCKET socket,uint16_t flag,string data){
    DataInfo dataInfo;
    dataInfo.flags = flag;
    dataInfo.len = data.size();
    memcpy(dataInfo.data,data.c_str(),dataInfo.len);
    write(socket, (char*)&dataInfo, 1024);
}

// 消息回传线程
void * CallbackThread(void *arg){
    CallbackParam *pCallbackParam = (CallbackParam*)arg;
    uint64_t record_num = 0;
    //在创建线程前设置线程创建属性,设为分离态,效率高
    pthread_detach(pthread_self());
    cout << "回传线程启动成功" << pCallbackParam->chatRecordVector->size() << endl;
    SendMsg(pCallbackParam->clientSocket,0x03,"消息通道已建立");
    while(!*(pCallbackParam->m_stop)){
        if(record_num < pCallbackParam->chatRecordVector->size()){
            string str = "[" + (*(pCallbackParam->chatRecordVector))[record_num].clientInfo.clientIp + "]("+ (*(pCallbackParam->chatRecordVector))[record_num].clientInfo.clientName +"):" + (*(pCallbackParam->chatRecordVector))[record_num].record;
            if(!(*(pCallbackParam->chatRecordVector))[record_num].kill_process)
                SendMsg(pCallbackParam->clientSocket,0x03,str);
            else{
                SendMsg(pCallbackParam->clientSocket,0x04,str);
                cout << pCallbackParam->clientSocket << " " << str << endl;
            }
            ++record_num;
        }else{
            usleep(5);
        }
    }
    pthread_exit(NULL);
}

// Socket线程
// 说明：每个Socket对应一个线程
void * SocketThread(void *arg){
    // 参数转换
    SocketParam* pSocketParam = (SocketParam*)arg;

    // 检查客户端MAP中是否已存在该(目前肯定不存在)
    map<uint64_t,ClientInfo>::iterator it = pSocketParam->clientMap->find(pSocketParam->clientInfo.clientId);
    if(it != pSocketParam->clientMap->end()){
        it->second.connectNum++;
        pSocketParam->clientInfo = it->second;
    }
    else{
        pSocketParam->clientMap->insert(pair<uint64_t,ClientInfo>(pSocketParam->clientInfo.clientId, pSocketParam->clientInfo));
    }

    // 在创建线程前设置线程创建属性,设为分离态,效率高
    pthread_detach(pthread_self());

    // 线程开始运行
    while (true) {
        DataInfo *pDataInfo = new DataInfo();
        string strIp = "[" + pSocketParam->clientInfo.clientIp + "]:";
        if (recv(pSocketParam->socketInfo.socket, (char*)pDataInfo, 1024, 0) == -1)
        {
            cout << strIp << "连接出错:错误代码[" << errno << "](" << strerror(errno) << ")" << endl;
            break;
        }
        // Debug:
        cout << "flags:" << pDataInfo->flags << " len:" << pDataInfo->len << " 原始数据:" << (char*)pDataInfo << endl;

        // 子线程停止标志（0x00不停止。0x01停止）
        uint16_t stop = 0x00;

        if (pDataInfo->flags == 0x01) {       // 登录
            string str = strIp + "连接成功!";
            cout << str +"(当前服务器人数:" << pSocketParam->clientMap->size() << ")" << endl;
            pSocketParam->clientInfo.clientName.append(pDataInfo->data,pDataInfo->len);
            SendMsg(pSocketParam->socketInfo.socket, 0x01, str+pSocketParam->clientInfo.clientName);
            // 启动消息回调线程
            pthread_t tid;
            CallbackParam* callbackParam = new CallbackParam(&stop, pSocketParam->socketInfo.socket, pSocketParam->chatRecordVector);
            pthread_create(&tid, NULL, CallbackThread, (void *)callbackParam);
            ChatRecord chatRecord(pSocketParam->clientInfo, "连接成功!");
            pSocketParam->chatRecordVector->push_back(chatRecord);
        }
        else if (pDataInfo->flags == 0x02) {  // 退出
            string str = strIp + "退出成功!";
            cout << str <<  "(当前服务器人数:" << (pSocketParam->clientMap->size() - 1) << ")" << endl;
            SendMsg(pSocketParam->socketInfo.socket,0x02,str);
            stop = 0x01;
            ChatRecord chatRecord(pSocketParam->clientInfo, "退出成功!");
            pSocketParam->chatRecordVector->push_back(chatRecord);
            break;
        }
        else if (pDataInfo->flags == 0x03) {  // 通讯
            string str(pDataInfo->data, pDataInfo->len);
            cout << strIp << "(" << pSocketParam->clientInfo.clientName << "):" << str << endl;
            ChatRecord chatRecord(pSocketParam->clientInfo, str);
            pSocketParam->chatRecordVector->push_back(chatRecord);
        }
        else if(pDataInfo->flags == 0x04) { // 杀死进程信号
            string str(pDataInfo->data, pDataInfo->len);
            cout << strIp << "(" << pSocketParam->clientInfo.clientName << ")K:" << str << endl;
            ChatRecord chatRecord(pSocketParam->clientInfo, str,true);
            pSocketParam->chatRecordVector->push_back(chatRecord);
        }

        delete pDataInfo;
        pDataInfo = NULL;
    }

    map<uint64_t ,ClientInfo>::iterator its = pSocketParam->clientMap->find(pSocketParam->clientInfo.clientId);
    if(its != pSocketParam->clientMap->end()){
        pSocketParam->clientMap->erase(pSocketParam->clientInfo.clientId);
    }
    close(pSocketParam->socketInfo.socket);
    delete pSocketParam;
    pSocketParam = NULL;
    pthread_exit(NULL);
}
