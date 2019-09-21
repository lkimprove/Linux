//客户端

#include "udpsock.hpp"

#define CHECK_RET(q) if((q)  == false){ return -1; }

int main(int argc, char* argv[]){
    //判断传入参数个数
    if(argc != 3){
        cout << "传入参数有误" << endl;
    }

    //接收 IP 和 Port
    string ip = argv[1];
    uint16_t port = atoi(argv[2]);

    //创建服务端
    UdpSock sock;

    //创建套接字
    CHECK_RET(sock.Sock());

    //绑定地址信息
    //客户端自定义绑定地址信息
    //手动不绑定地址信息，则系统会自动绑定

    while(1){
        //客户端向固定的 IP 及 Port 发送数据

        //发送数据
        string buf;     //缓冲区
        cin >> buf;
        CHECK_RET(sock.Send(buf, ip, port));

        //接收数据
        buf.clear();    //清空buf
        CHECK_RET(sock.Recv(buf, ip, port));
        cout << "服务端发送的数据：" << buf << endl;
    }

    //关闭套接字
    sock.Close();

    return 0;
}
