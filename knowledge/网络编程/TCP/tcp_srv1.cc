//服务端(多线程版)

#include "tcpsock.h"
#include <pthread.h>

//线程入口函数
void* thr_start(void* arg){
    //获取参数
    TcpSock* newsock = (TcpSock*)arg;

    while(1){
        //接收数据
        string buf;
        bool ret = newsock->Recv(buf);
        if(ret == false){
            newsock->Close();
            delete newsock;
            return NULL;   
        }
        cout << "客户端发送的数据：" << buf << endl;

        //发送数据
        buf.clear();    //清空buf
        cin >> buf;
        ret = newsock->Send(buf);
        if(ret == false){
            newsock->Close();
            delete newsock;
            return NULL;
        }
    }
    
    newsock->Close();
    delete newsock;
    return NULL;
}

int main(int argc, char* argv[]){
    //判断传入的参数个数
    if(argc != 3){
        cout << "传入的参数有误" << endl;
        return -1;
    }

    //获取ip和port
    string ip = argv[1];
    uint16_t port = atoi(argv[2]);

    //创建客户端
    TcpSock sock;

    //创建套接字
    CHECK_RET(sock.Sock());

    //绑定地址信息
    CHECK_RET(sock.Bind(ip, port));

    //服务端开始监听
    CHECK_RET(sock.Listen());

    while(1){
        //为了避免被释放，用于与客户端通信的sock在堆上创建
        TcpSock* newsock = new TcpSock ();
        
        //获取已连接客户端
        bool ret = sock.Accept(*newsock);
        if(ret == false){
            delete newsock;
            continue;
        }
        
        //创建于客户端通信的线程
        pthread_t tid;
        int ret_t = pthread_create(&tid, NULL, thr_start, (void*)newsock);
        
        //分离线程
        pthread_detach(tid);
    }

    //关闭套接字
    sock.Close();

    return 0;
}
