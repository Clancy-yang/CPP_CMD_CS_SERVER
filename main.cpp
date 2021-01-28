/* server */

#include "Management.h"
#include <csignal>


using namespace std;

bool app_stopped = false;

//void sigint_handler(int sig){
//    if(sig == SIGINT){
//        // ctrl+c退出时执行的代码
//        std::cout << "ctrl+c pressed!" << std::endl;
//        app_stopped = true;
//    }
//}

int main(){
    //signal(SIGINT, sigint_handler);
    vector<pthread_t> pid_v;

    ServiceManagement management;

    while (!app_stopped){
        pthread_t tid;
        Client *client = new Client();
        socklen_t client_addr_size = sizeof(client->client_addr);

        if((client->client_socket = accept(management.receive_socket_,(struct sockaddr*)&client->client_addr,&client_addr_size)) == -1){
            cout << "Accept Socket Failed! error:(" << errno << strerror(errno) << ")" << endl;
            close(client->client_socket);
        }else{
            client->client_info.ip = inet_ntoa(client->client_addr.sin_addr);
            client->client_info.connect_num++;
            cout << "[" << client->client_info.ip << "]:连接服务器!" << endl;
            pthread_create(&tid,NULL,run,(void*)client);
            pid_v.push_back(tid);
        }
    }

    cout << "程序结束" << endl;


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
//            Data data;
//            if (recv(serConn, (char*)&data, 1024, 0) < 0)
//            {
//                printf("Server Recieve Data Failed!\n");
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
//                string str(data.data, data.data_len);
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