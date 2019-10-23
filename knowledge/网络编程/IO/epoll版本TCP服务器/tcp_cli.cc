#include "tcpsocket.h"

int main(int argc, char* argv[]){
    //判断传入的参数个数
    if(argc != 3){
        cout << "传入参数个数有误" << endl;
        return -1;
    }

    string ip = argv[1];
    uint16_t port = atoi(argv[2]);

    //创建客户端
    TcpSocket sock;

    //创建套接字
    CHECK_RET(sock.Sock());

    //向服务端发起连接请求
    CHECK_RET(sock.Connect(ip, port));

    while(1){
        //发送数据
        string buf;
        cin >> buf;
        int ret = sock.Send(buf);
        if(ret == false){
            break;
        }

        //接收数据
        buf.clear();
        ret = sock.Recv(buf, 0);
        if(ret == false){
            break;
        }
        cout << "服务端发送的数据为：" <<  buf << endl;

    }
    
    sock.Close();
    return 0;
}
