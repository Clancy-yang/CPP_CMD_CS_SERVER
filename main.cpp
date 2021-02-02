/* server */

#include "Management.h"
#include <csignal>


using namespace std;

SocketInfo receive;

void sigint_handler(int sig){
    if(sig == SIGINT){
        // ctrl+c退出时执行的代码
        cout << "程序结束中.." << endl;
        close(receive.socket);
        cout << "程序结束" << endl;
        exit(0);
    }
}

int main(){
    signal(SIGINT, sigint_handler);

    // 初始化变量
    map<uint64_t ,ClientInfo> clientMap;
    vector<ChatRecord> chatRecordVector;

    //程序初始化
    if(!Init(receive)){
        exit(0);
    }

    //主线程采用阻塞模式监听端口
    uint64_t threadNumber = 0;
    while(true){
        pthread_t pthread;
        SocketParam *param = new SocketParam();
        socklen_t socklen = sizeof(param->socketInfo.sockaddrIn);

        if((param->socketInfo.socket = accept(receive.socket,(struct sockaddr*)&param->socketInfo.sockaddrIn,&socklen)) == -1){
            cout << "接收Socket失败:错误代码[" << errno << "](" << strerror(errno) << ")" << endl;
            delete param;
            param = NULL;
            break;
        }else{
            param->chatRecordVector = &chatRecordVector;
            param->clientInfo.clientId = ++threadNumber;
            param->clientInfo.clientIp = inet_ntoa(param->socketInfo.sockaddrIn.sin_addr);
            param->clientMap = &clientMap;
            param->clientInfo.connectNum++;
            cout << "[" << param->clientInfo.clientIp << "]:连接服务器!" << endl;
            //为该连接分配单独线程
            pthread_create(&pthread,NULL,SocketThread,(void*)param);
        }
    }





//    ServiceManagement management;
//
//
//    while (!app_stopped){
//        pthread_t tid;
//        SocketParam *client = new SocketParam();
//        socklen_t client_addr_size = sizeof(client->socketInfo.sockaddrIn);
//
//        if((client->socketInfo.socket = accept(management.receive_socket_,(struct sockaddr*)&client->socketInfo.sockaddrIn,&client_addr_size)) == -1){
//            cout << "Accept Socket Failed! error:(" << errno << strerror(errno) << ")" << endl;
//            close(client->socketInfo.socket);
//            break;
//        }else{
//            client->chatRecordVector = &chatRecordVector;
//            client->clientInfo.clientIp = inet_ntoa(client->socketInfo.sockaddrIn.sin_addr);
//            client->clientInfo.connectNum++;
//            cout << "[" << client->clientInfo.clientIp << "]:连接服务器!" << endl;
//            pthread_create(&tid,NULL,SocketThread,(void*)client);
//
//        }
//    }




//    //创建套接字
//    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//
//    //初始化socket元素
//    struct sockaddr_in serv_addr{};
//    memset(&serv_addr, 0, sizeof(serv_addr));
//    serv_addr.sin_family = AF_INET;
//    serv_addr.sin_addr.s_addr = INADDR_ANY;
//    serv_addr.sin_port = htons(1234);
//
//    //绑定文件描述符和服务器的ip和端口号
//    bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
//
//    //进入监听状态，等待用户发起请求
//    listen(serv_sock, 20);
//    //接受客户端请求
//    //定义客户端的套接字，这里返回一个新的套接字，后面通信时，就用这个clnt_sock进行通信
//    struct sockaddr_in clientsocket;
//    socklen_t clnt_addr_size = sizeof(clientsocket);
//
//    //
//    while(true){
//        int serConn = accept(serv_sock, (struct sockaddr*)&clientsocket, &clnt_addr_size);
//        if (serConn < 0) {
//            printf("Server Accept Failed!\n");
//            break;
//        }
//        while (true) {
//            //char sendBuf[100];
//            //sprintf(sendBuf, "welcome %s to bejing", inet_ntoa(clientsocket.sin_addr));//找对对应的IP并且将这行字打印到那里
//            //发送信息
//            //send(serConn, sendBuf, strlen(sendBuf) + 1, 0);
//
//            DataInfo data;
//            if (recv(serConn, (char*)&data, 1024, 0) < 0)
//            {
//                printf("Server Recieve DataInfo Failed!\n");
//                break;
//            }
//
//            if (data.flags == 0x01) {       //登录
//                cout << "[" << inet_ntoa(clientsocket.sin_addr) << "]:登录服务器!" << endl;
//            }
//            else if (data.flags == 0x02) {  //退出
//                cout << "[" << inet_ntoa(clientsocket.sin_addr) << "]:退出服务器!" << endl;
//                break;
//            }
//            else if (data.flags == 0x03) {  //通讯
//                string str(data.data, data.len);
//                cout << "[" << inet_ntoa(clientsocket.sin_addr) << "]:"<< str << endl;
//            }
//        }
//
//        close(serConn);//关闭
//    }
////    //接收客户端数据，并相应
////    char str[256];
////    read(clnt_sock, str, sizeof(str));
////    printf("client send: %s\n",str);
////    strcat(str, "+ACK");
////    write(clnt_sock, str, sizeof(str));
////
////    //关闭套接字
////    close(clnt_sock);
//    close(serv_sock);

    return 0;
}